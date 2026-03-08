/* drivers/misc/lowmemorykiller.c
 *
 * The lowmemorykiller driver lets user-space specify a set of memory thresholds
 * where processes with a range of oom_score_adj values will get killed. Specify
 * the minimum oom_score_adj values in
 * /sys/module/lowmemorykiller/parameters/adj and the number of free pages in
 * /sys/module/lowmemorykiller/parameters/minfree. Both files take a comma
 * separated list of numbers in ascending order.
 *
 * For example, write "0,8" to /sys/module/lowmemorykiller/parameters/adj and
 * "1024,4096" to /sys/module/lowmemorykiller/parameters/minfree to kill
 * processes with a oom_score_adj value of 8 or higher when the free memory
 * drops below 4096 pages and kill processes with a oom_score_adj value of 0 or
 * higher when the free memory drops below 1024 pages.
 *
 * The driver considers memory used for caches to be free, but if a large
 * percentage of the cached memory is locked this can be very inaccurate
 * and processes may not get killed until the normal oom killer is triggered.
 *
 * Copyright (C) 2007-2008 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/oom.h>
#include <linux/sched.h>
#include <linux/swap.h>
#include <linux/rcupdate.h>
#include <linux/profile.h>
#include <linux/notifier.h>

#ifdef CONFIG_AMZ_MISC
/* ACOS_MOD_BEGIN {fwk_crash_log_collection} */
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kallsyms.h>
#include <linux/slab.h>

#ifdef CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES
#define REVERT_ADJ(x)  (x * (-OOM_DISABLE + 1) / OOM_SCORE_ADJ_MAX)
#else
#define REVERT_ADJ(x) (x)
#endif /* CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES */
/* ACOS_MOD_END {fwk_crash_log_collection} */
#endif

#define CREATE_TRACE_POINTS
#include "trace/lowmemorykiller.h"

static u32 lowmem_debug_level = 1;
static short lowmem_adj[6] = {
	0,
	1,
	6,
	12,
};

static int lowmem_adj_size = 4;
#ifdef CONFIG_MP_CMA_PATCH_CMA_DYNAMIC_STRATEGY
int lowmem_minfree[6] = {
#else
int lowmem_minfree[6] = {
#endif
	3 * 512,	/* 6MB */
	2 * 1024,	/* 8MB */
	4 * 1024,	/* 16MB */
	16 * 1024,	/* 64MB */
};

#ifdef CONFIG_MP_CMA_PATCH_DELAY_FREE
int lowmem_minfree_size = 4;
#else
static int lowmem_minfree_size = 4;
#endif

static unsigned long lowmem_deathpending_timeout;
static pid_t lowmem_deathpending_tgid;
static unsigned long lowmem_kill_timeout;

/* adjust killing level based on page thrashing status and swap status in
   addtion to file page status */
#ifdef CONFIG_LMKD_POLICY
/* tune-able parameters */
static int thrashing_limit = 50;
static int oom_score_adj_perceptible = 200;
static int thrashing_limit_critical = -1; /* -1 means FG kill policy disabled */
static int oom_score_adj_fg = 0;
static int swap_low_pct_threshold = 5; /* ref: sunstone */

/* avoid to renew thrashing baseline too frequently */
static unsigned long thrashing_renew_timeout = 0;

static int base_file_lru;
static int init_ws_refault;
static bool in_reclaim;
static DEFINE_SPINLOCK(thrashing_lock);

static inline bool is_direct_reclaim(void)
{
	return !current_is_kswapd();
}
static inline bool swap_is_low(void)
{
	long free_swap_pages = get_nr_swap_pages();

	if (total_swap_pages == 0)
		return false;

	return free_swap_pages <
		(total_swap_pages * swap_low_pct_threshold / 100);
}
static inline int swap_pages_pct(void)
{
	long free_swap_pages = get_nr_swap_pages();
	if (total_swap_pages == 0)
		return 0;

	return (int)((total_swap_pages - free_swap_pages) * 100 / total_swap_pages);
}

static inline void update_thrashing_baseline(void) {
	if (time_before_eq(jiffies, thrashing_renew_timeout)) {
		/* do nothing */
	}
	else {
		unsigned long flags = 0;
		spin_lock_irqsave(&thrashing_lock, flags);

		/* for skipping renew thrashing baseline in next 100ms */
		thrashing_renew_timeout = jiffies + HZ / 10;

		/* update thrashing baseline when lmk skips kill */
		base_file_lru = global_node_page_state(NR_INACTIVE_FILE) +
						global_node_page_state(NR_ACTIVE_FILE);
		init_ws_refault = global_node_page_state(WORKINGSET_REFAULT);
		spin_unlock_irqrestore(&thrashing_lock, flags);
	}

}
#endif

#ifdef CONFIG_AMZ_MISC
/* ACOS_MOD_BEGIN {fwk_crash_log_collection} */

/* Constants */
static int BUFFER_SIZE = 16*1024;
static int ELEMENT_SIZE = 256;

/* Variables */
static char *lmk_log_buffer;
static char *buffer_end;
static char *head;
static char *kill_msg_index;
static char *previous_crash;
static int buffer_remaining;
static int foreground_kill;

void lmk_add_to_buffer(const char *fmt, ...)
{
	if (lmk_log_buffer) {
		if (head >= buffer_end) {
			/* Don't add more logs buffer is full */
			return;
		}
		if (buffer_remaining > 0) {
			va_list args;
			int added_size = 0;
			va_start(args, fmt);
			/* If the end of the buffer is reached and the added
			 * value is truncated then vsnprintf will return the
			 * original length of the value instead of the
			 * truncated length - this is intended by design. */
			added_size = vsnprintf(head, buffer_remaining, fmt, args);
			va_end(args);
			if (added_size > 0) {
				/* Add 1 for null terminator */
				added_size = added_size + 1;
				buffer_remaining = buffer_remaining - added_size;
				head = head + added_size;
			}
		}
	}
}

EXPORT_SYMBOL(lmk_add_to_buffer);

#define lowmem_print(level, x...)			\
	do {						\
		if (lowmem_debug_level >= (level))	\
			pr_info(x);			\
		if (foreground_kill)			\
			lmk_add_to_buffer(x);		\
	} while (0)
/* ACOS_MOD_END {fwk_crash_log_collection} */
#else
#define lowmem_print(level, x...)			\
	do {						\
		if (lowmem_debug_level >= (level))	\
			pr_info(x);			\
	} while (0)
#endif

static unsigned long lowmem_count(struct shrinker *s,
				  struct shrink_control *sc)
{
	return global_node_page_state(NR_ACTIVE_ANON) +
		global_node_page_state(NR_ACTIVE_FILE) +
		global_node_page_state(NR_INACTIVE_ANON) +
		global_node_page_state(NR_INACTIVE_FILE);
}

#ifdef CONFIG_MP_DEBUG_TOOL_MEMORY_USAGE_MONITOR
extern db_time_table time_cnt_table[DB_MAX_CNT];
#endif

#ifdef CONFIG_MP_CMA_PATCH_DELAY_FREE
extern void set_delay_free_min_mem(int min_mem);
#endif

#ifdef CONFIG_MP_MMA_ENABLE
extern int total_ion_system_pages(void);
#endif

static unsigned long lowmem_scan(struct shrinker *s, struct shrink_control *sc)
{
#ifdef CONFIG_MP_DEBUG_TOOL_MEMORY_USAGE_MONITOR
	unsigned long time_start = jiffies;
#endif
#ifdef CONFIG_LMKD_POLICY
	int thrashing;
#endif
	struct task_struct *tsk;
	struct task_struct *selected = NULL;
	unsigned long rem = 0;
	int tasksize;
	int i;
	short min_score_adj = OOM_SCORE_ADJ_MAX + 1;
	int minfree = 0;
	int selected_tasksize = 0;
	short selected_oom_score_adj;
	int array_size = ARRAY_SIZE(lowmem_adj);
	int other_free = global_page_state(NR_FREE_PAGES) - totalreserve_pages;
	int other_file = global_node_page_state(NR_FILE_PAGES) -
				global_node_page_state(NR_SHMEM) -
				global_node_page_state(NR_UNEVICTABLE) -
				total_swapcache_pages();
#ifdef CONFIG_MP_Android_MSTAR_ADJUST_LOW_MEM_KILLER_POLICY
	int active_file = global_node_page_state(NR_ACTIVE_FILE);
	int inactive_file = global_node_page_state(NR_INACTIVE_FILE);
#endif

	int free_cma = 0;
#ifdef CONFIG_MP_Android_MSTAR_ADJUST_LOW_MEM_KILLER_POLICY
	int total_free = 0;
#endif

	/* Avoid to have too many parallel executions from direct reclaim when
       memory pressure is really critical. The cost of going through task
       list to find one to kill is too high when allow parallel execution */
	if (time_before_eq(jiffies, lowmem_kill_timeout) && (!current_is_kswapd())) {
		lowmem_print(5, "skip kill for direct reclaim within kill timeout\n");
		return 0;
	}

#ifdef CONFIG_CMA
	if (gfpflags_to_migratetype(sc->gfp_mask) != MIGRATE_MOVABLE) {
		free_cma = global_page_state(NR_FREE_CMA_PAGES);
		other_free -= free_cma;
	}
#ifdef CONFIG_MP_CMA_PATCH_AGRESSIVE_KILL_PROCESS_TO_FREE_CMA_PAGE
	if(lowmem_adj[4])
		set_early_kill_oom_adj_threshold(lowmem_adj[4]);
#endif

#ifdef CONFIG_MP_CMA_PATCH_DELAY_FREE
	if(lowmem_minfree[5])
		set_delay_free_min_mem(lowmem_minfree[5]);
#endif

#endif

#ifdef CONFIG_MP_Android_MSTAR_ADJUST_LOW_MEM_KILLER_POLICY
	// the file cache can be writted as "other_file - mmaped"(unmapped) or "other_file - active"(inactive)
	total_free = other_free - free_cma + (other_file - active_file);
#ifdef CONFIG_MP_MMA_ENABLE
	total_free += total_ion_system_pages();
#endif
	//total_free = other_free - free_cma + other_file - global_node_page_state(NR_FILE_MAPPED);
#endif

	if (lowmem_adj_size < array_size)
		array_size = lowmem_adj_size;
	if (lowmem_minfree_size < array_size)
		array_size = lowmem_minfree_size;
	for (i = 0; i < array_size; i++) {
#ifdef CONFIG_MP_Android_MSTAR_ADJUST_LOW_MEM_KILLER_POLICY
		int tmp_free;

		minfree = lowmem_minfree[i];
		tmp_free = !lowmem_adj[i] ? total_free + active_file : total_free;

		if (tmp_free < minfree) {
			min_score_adj = lowmem_adj[i];
			break;
		}
#else
		minfree = lowmem_minfree[i];
		if (other_free < minfree && other_file < minfree) {
			min_score_adj = lowmem_adj[i];
			break;
		}
#endif
	}

	lowmem_print(3, "lowmem_scan %lu, %x, ofree %d %d, ma %hd\n",
		     sc->nr_to_scan, sc->gfp_mask, other_free,
		     other_file, min_score_adj);

	if (min_score_adj == OOM_SCORE_ADJ_MAX + 1) {
		lowmem_print(5, "lowmem_scan %lu, %x, return 0\n",
			     sc->nr_to_scan, sc->gfp_mask);
#ifdef CONFIG_MP_Android_MSTAR_ADJUST_LOW_MEM_KILLER_POLICY
		lowmem_print(5, "total_free is %d, other_file is %d, free_cma is %d\n", total_free, other_file, free_cma);
		lowmem_print(5, "active_file is %d, inactive_file is %d\n", active_file, inactive_file);
#if 0
		int print_index = 0;
		for(print_index = 0; print_index < array_size; print_index++)
		{
			printk("\033[35mlowmem_adj: %d, lowmem_minfree: %d\033[m\n", lowmem_adj[print_index], lowmem_minfree[print_index]); // page_count
		}
#endif
#endif
#ifdef CONFIG_MP_DEBUG_TOOL_MEMORY_USAGE_MONITOR
		atomic_add((jiffies-time_start), &time_cnt_table[lowmem_scan_count].lone_time);
		atomic_inc(&time_cnt_table[lowmem_scan_count].do_cnt);
#endif

#ifdef CONFIG_LMKD_POLICY
		update_thrashing_baseline();
#endif
		return 0;
	}

#ifdef CONFIG_LMKD_POLICY

	/* check thrashing when lmk starts to kill */
	thrashing = (global_node_page_state(WORKINGSET_REFAULT) - init_ws_refault)
				 * 100 / base_file_lru;

	if ( (thrashing_limit_critical > 0) &&
		(thrashing >= thrashing_limit_critical) &&
		(swap_is_low() || is_direct_reclaim())) {
		/* kill FG apps, FOS8 lmkd policy */
		if (min_score_adj > oom_score_adj_fg) {
			min_score_adj = oom_score_adj_fg;
			/* use ratelimited version of print */
			pr_info_ratelimited("Adjust min_score_adj to FG (%d) because"
				 " thrashing pct (%d) is above threshhold (%d),"
				 " direct reclaim (%s), swap pages pct is (%d)\n",
				 min_score_adj, thrashing, thrashing_limit_critical,
				  is_direct_reclaim() ? "yes" : "no", swap_pages_pct());
		}
	} else if ((thrashing >= thrashing_limit) &&
		(swap_is_low() || is_direct_reclaim())) {
		/* increase killing level to PERCEPTIBLE_APP_ADJ + 1, FOS8 lmkd policy*/
		if (min_score_adj > oom_score_adj_perceptible) {
			min_score_adj = oom_score_adj_perceptible + 1;
			/* by default disable this print */
			lowmem_print(2, "Adjust min_score_adj to PERCEPTIBLE+1 (%d) because"
				 " thrashing pct (%d) is above threshhold (%d),"
				 " direct reclaim (%s), swap pages pct is (%d)\n",
				 min_score_adj, thrashing, thrashing_limit,
				 is_direct_reclaim() ? "yes" : "no", swap_pages_pct());

		}
	}
#endif

	selected_oom_score_adj = min_score_adj;

	rcu_read_lock();
	for_each_process(tsk) {
		struct task_struct *p;
		short oom_score_adj;

		if (tsk->flags & PF_KTHREAD)
			continue;

		p = find_lock_task_mm(tsk);
		if (!p)
			continue;

		if ((task_lmk_waiting(p) || (lowmem_deathpending_tgid == task_tgid_nr(p))) &&
		    time_before_eq(jiffies, lowmem_deathpending_timeout)) {
			task_unlock(p);
			rcu_read_unlock();
#ifdef CONFIG_MP_DEBUG_TOOL_MEMORY_USAGE_MONITOR
			atomic_add((jiffies-time_start), &time_cnt_table[lowmem_scan_count].lone_time);
			atomic_inc(&time_cnt_table[lowmem_scan_count].do_cnt);
#endif
			return 0;
		}
		oom_score_adj = p->signal->oom_score_adj;
		if (oom_score_adj < min_score_adj) {
			task_unlock(p);
			continue;
		}
		tasksize = get_mm_rss(p->mm);
		task_unlock(p);
		if (tasksize <= 0)
			continue;
		if (selected) {
			if (oom_score_adj < selected_oom_score_adj)
				continue;
			if (oom_score_adj == selected_oom_score_adj &&
			    tasksize <= selected_tasksize)
				continue;
		}
		selected = p;
		selected_tasksize = tasksize;
		selected_oom_score_adj = oom_score_adj;
		lowmem_print(2, "select '%s' (%d), adj %hd, size %d, to kill\n",
			     p->comm, p->pid, oom_score_adj, tasksize);
	}
#ifdef CONFIG_AMZ_MISC
	/* ACOS_MOD_BEGIN {fwk_crash_log_collection} */
	if (lmk_log_buffer && selected && selected_oom_score_adj == 0) {
		foreground_kill = 1;
		head = lmk_log_buffer;
		buffer_remaining = BUFFER_SIZE;
		if (kill_msg_index && previous_crash)
			strncpy(previous_crash, kill_msg_index, ELEMENT_SIZE);
		lowmem_print(1, "======low memory killer=====\n");
		lowmem_print(1, "Free memory other_free: %d, other_file:%d pages\n", other_free, other_file);
		if (gfp_zone(sc->gfp_mask) == ZONE_NORMAL)
			lowmem_print(1, "ZONE_NORMAL\n");
		else
			lowmem_print(1, "ZONE_HIGHMEM\n");

		for_each_process(tsk) {
			struct task_struct *p2;
			short oom_score_adj2;

			if (tsk->flags & PF_KTHREAD)
				continue;

			p2 = find_lock_task_mm(tsk);
			if (!p2)
				continue;

			oom_score_adj2 = p2->signal->oom_score_adj;
#ifdef CONFIG_ZRAM
			lowmem_print(1, "Candidate %d (%s), adj %d, score_adj %d, rss %lu, rswap %lu, to kill\n",
				p2->pid, p2->comm, REVERT_ADJ(oom_score_adj2),
				oom_score_adj2, get_mm_rss(p2->mm), get_mm_counter(p2->mm, MM_SWAPENTS));
#else /* CONFIG_ZRAM */
			lowmem_print(1, "Candidate %d (%s), adj %d, score_adj %d, rss %lu, to kill\n",
				p2->pid, p2->comm, REVERT_ADJ(oom_score_adj2),
				oom_score_adj2, get_mm_rss(p2->mm));
#endif /* CONFIG_ZRAM */
			task_unlock(p2);
		}
		kill_msg_index = head;
	}
	/* ACOS_MOD_END {fwk_crash_log_collection} */
#endif
	if (selected) {
		long cache_size = other_file * (long)(PAGE_SIZE / 1024);
		long cache_limit = minfree * (long)(PAGE_SIZE / 1024);
		long free = other_free * (long)(PAGE_SIZE / 1024);

		task_lock(selected);
		lowmem_deathpending_tgid = task_tgid_nr(selected);
		send_sig(SIGKILL, selected, 0);
		if (selected->mm)
			task_set_lmk_waiting(selected);
		task_unlock(selected);
		trace_lowmemory_kill(selected, cache_size, cache_limit, free);
		lowmem_print(1, "Killing '%s' (%d) (tgid %d), adj %hd,\n"
				 "   to free %ldkB on behalf of '%s' (%d) because\n"
				 "   cache %ldkB is below limit %ldkB for oom_score_adj %hd\n"
				 "   Free memory is %ldkB above reserved\n",
			     selected->comm, selected->pid, selected->tgid,
			     selected_oom_score_adj,
			     selected_tasksize * (long)(PAGE_SIZE / 1024),
			     current->comm, current->pid,
			     cache_size, cache_limit,
			     min_score_adj,
			     free);
#ifdef CONFIG_LMKD_POLICY
		if (thrashing > thrashing_limit)
			lowmem_print(1, "Thrashing policy is used,"
				 " thrashing pct (%d) is above threshhold (%d),"
				 " direct reclaim (%s), swap pages pct is (%d)",
				 thrashing, thrashing_limit, is_direct_reclaim() ? "yes" : "no",
				 swap_pages_pct());
#endif

#ifdef CONFIG_MP_Android_MSTAR_ADJUST_LOW_MEM_KILLER_POLICY
		printk("   Total_free = %ldkB, free_cma=%ldkB, Totalreserve_pages = %ldkB, MAPPED = %ldkB\n",
			total_free * (long)(PAGE_SIZE / 1024),
			free_cma * (long)(PAGE_SIZE / 1024),
			totalreserve_pages * (long)(PAGE_SIZE / 1024),
			global_node_page_state(NR_FILE_MAPPED) * (long)(PAGE_SIZE / 1024));
		lowmem_print(1, "total_free is %d, other_free is %d, free_cma is %d\n", total_free, other_free, free_cma);
		lowmem_print(1, "other_file is %d, active_file is %d, inactive_file is %d\n\n\n", other_file, active_file, inactive_file);
#endif
		lowmem_deathpending_timeout = jiffies + HZ;
		/* for skipping scan from direct reclaim in next 100ms*/
		lowmem_kill_timeout = jiffies + HZ/10;
		rem += selected_tasksize;
	}

	lowmem_print(4, "lowmem_scan %lu, %x, return %lu\n",
		     sc->nr_to_scan, sc->gfp_mask, rem);
	rcu_read_unlock();
#ifdef CONFIG_MP_DEBUG_TOOL_MEMORY_USAGE_MONITOR
	atomic_add((jiffies-time_start), &time_cnt_table[lowmem_scan_count].lone_time);
	atomic_inc(&time_cnt_table[lowmem_scan_count].do_cnt);
#endif
	return rem;
}

static struct shrinker lowmem_shrinker = {
	.scan_objects = lowmem_scan,
	.count_objects = lowmem_count,
	.seeks = DEFAULT_SEEKS * 16
};

#ifdef CONFIG_AMZ_MISC
/* ACOS_MOD_BEGIN {fwk_crash_log_collection} */
static int lowmem_proc_show(struct seq_file *m, void *v)
{
	char *ptr;
	if (!lmk_log_buffer) {
		seq_printf(m, "lmk_logs are not functioning - something went wrong during init");
		return 0;
	}
	ptr = lmk_log_buffer;
	while (ptr < head) {
		int cur_line_len = strlen(ptr);
		seq_printf(m, ptr, "\n");
		if (cur_line_len <= 0)
			break;
		/* add 1 to skip the null terminator for C Strings */
		ptr = ptr + cur_line_len + 1;
	}
	if (previous_crash && previous_crash[0] != '\0') {
		seq_printf(m, "previous crash:\n");
		seq_printf(m, previous_crash, "\n");
	}
	return 0;
}

static int lowmem_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, lowmem_proc_show, NULL);
}

static const struct file_operations lowmem_proc_fops = {
	.open       = lowmem_proc_open,
	.read       = seq_read,
	.release    = single_release
};
/* ACOS_MOD_END {fwk_crash_log_collection} */
#endif

static int __init lowmem_init(void)
{
	register_shrinker(&lowmem_shrinker);

#ifdef CONFIG_AMZ_MISC
	/* ACOS_MOD_BEGIN {fwk_crash_log_collection} */
	proc_create("lmk_logs", 0, NULL, &lowmem_proc_fops);
	lmk_log_buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
	if (lmk_log_buffer) {
		buffer_end = lmk_log_buffer + BUFFER_SIZE;
		head = lmk_log_buffer;
		buffer_remaining = BUFFER_SIZE;
		foreground_kill = 0;
		kill_msg_index = NULL;
		previous_crash = kzalloc(ELEMENT_SIZE, GFP_KERNEL);
		if (!previous_crash)
			printk(KERN_ALERT "unable to allocate previous_crash for /proc/lmk_logs - previous_crash will not be logged");
	} else {
		printk(KERN_ALERT "unable to allocate buffer for /proc/lmk_logs - feature will be disabled");
	}
	/* ACOS_MOD_END {fwk_crash_log_collection} */
#endif

	return 0;
}
device_initcall(lowmem_init);

#ifdef CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES
static short lowmem_oom_adj_to_oom_score_adj(short oom_adj)
{
	if (oom_adj == OOM_ADJUST_MAX)
		return OOM_SCORE_ADJ_MAX;
	else
		return (oom_adj * OOM_SCORE_ADJ_MAX) / -OOM_DISABLE;
}

static void lowmem_autodetect_oom_adj_values(void)
{
	int i;
	short oom_adj;
	short oom_score_adj;
	int array_size = ARRAY_SIZE(lowmem_adj);

	if (lowmem_adj_size < array_size)
		array_size = lowmem_adj_size;

	if (array_size <= 0)
		return;

	oom_adj = lowmem_adj[array_size - 1];
	if (oom_adj > OOM_ADJUST_MAX)
		return;

	oom_score_adj = lowmem_oom_adj_to_oom_score_adj(oom_adj);
	if (oom_score_adj <= OOM_ADJUST_MAX)
		return;

	lowmem_print(1, "lowmem_shrink: convert oom_adj to oom_score_adj:\n");
	for (i = 0; i < array_size; i++) {
		oom_adj = lowmem_adj[i];
		oom_score_adj = lowmem_oom_adj_to_oom_score_adj(oom_adj);
		lowmem_adj[i] = oom_score_adj;
		lowmem_print(1, "oom_adj %d => oom_score_adj %d, with lowmem_minfree is %dkB\n",
			     oom_adj, oom_score_adj, lowmem_minfree[i]);
	}
}

static int lowmem_adj_array_set(const char *val, const struct kernel_param *kp)
{
	int ret;

	ret = param_array_ops.set(val, kp);

	/* HACK: Autodetect oom_adj values in lowmem_adj array */
	lowmem_autodetect_oom_adj_values();

	return ret;
}

static int lowmem_adj_array_get(char *buffer, const struct kernel_param *kp)
{
	return param_array_ops.get(buffer, kp);
}

static void lowmem_adj_array_free(void *arg)
{
	param_array_ops.free(arg);
}

static struct kernel_param_ops lowmem_adj_array_ops = {
	.set = lowmem_adj_array_set,
	.get = lowmem_adj_array_get,
	.free = lowmem_adj_array_free,
};

static const struct kparam_array __param_arr_adj = {
	.max = ARRAY_SIZE(lowmem_adj),
	.num = &lowmem_adj_size,
	.ops = &param_ops_short,
	.elemsize = sizeof(lowmem_adj[0]),
	.elem = lowmem_adj,
};
#endif

#ifdef CONFIG_LMKD_POLICY
module_param_named(thrashing_limit, thrashing_limit, int, 0664);
module_param_named(thrashing_limit_critical, thrashing_limit_critical, int, 0664);
module_param_named(oom_score_adj_fg, oom_score_adj_fg, int, 0664);
module_param_named(oom_score_adj_perceptible, oom_score_adj_perceptible, int, 0664);
module_param_named(swap_low_pct_threshold, swap_low_pct_threshold, int, 0664);
#endif

/*
 * not really modular, but the easiest way to keep compat with existing
 * bootargs behaviour is to continue using module_param here.
 */
module_param_named(cost, lowmem_shrinker.seeks, int, 0644);
#ifdef CONFIG_ANDROID_LOW_MEMORY_KILLER_AUTODETECT_OOM_ADJ_VALUES
module_param_cb(adj, &lowmem_adj_array_ops,
		.arr = &__param_arr_adj,
		0644);
__MODULE_PARM_TYPE(adj, "array of short");
#else
module_param_array_named(adj, lowmem_adj, short, &lowmem_adj_size, 0644);
#endif
module_param_array_named(minfree, lowmem_minfree, uint, &lowmem_minfree_size,
			 0644);
module_param_named(debug_level, lowmem_debug_level, uint, 0644);

