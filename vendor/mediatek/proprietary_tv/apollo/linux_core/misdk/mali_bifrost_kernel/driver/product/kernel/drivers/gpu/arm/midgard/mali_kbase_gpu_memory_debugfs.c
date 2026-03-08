/*
 *
 * (C) COPYRIGHT 2012-2017 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 */

#include <mali_kbase.h>

#ifdef CONFIG_DEBUG_FS
/** Show callback for the @c gpu_memory debugfs file.
 *
 * This function is called to get the contents of the @c gpu_memory debugfs
 * file. This is a report of current gpu memory usage.
 *
 * @param sfile The debugfs entry
 * @param data Data associated with the entry
 *
 * @return 0 if successfully prints data in debugfs entry file
 *         -1 if it encountered an error
 */

static int kbasep_gpu_memory_seq_show(struct seq_file *sfile, void *data)
{
	struct list_head *entry;
	const struct list_head *kbdev_list;

	kbdev_list = kbase_dev_list_get();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;
		struct kbasep_kctx_list_element *element;

		kbdev = list_entry(entry, struct kbase_device, entry);
		/* output the total memory usage and cap for this device */
#ifdef MSTAR_MEMORY_USAGE
		seq_printf(sfile, "%-16s  %10u %10u\n",
				kbdev->devname,
				atomic_read(&(kbdev->memdev.used_pages)),
				kbdev->memdev.max_used_pages);
#else
		seq_printf(sfile, "%-16s  %10u\n",
				kbdev->devname,
				atomic_read(&(kbdev->memdev.used_pages)));
#endif
		mutex_lock(&kbdev->kctx_list_lock);
		list_for_each_entry(element, &kbdev->kctx_list, link) {
			/* output the memory usage and cap for each kctx
			* opened on this device */
#ifdef MSTAR_MEMORY_USAGE
			seq_printf(sfile, "  %s-0x%p (%4u) %10u %10u\n",
				"kctx",
				element->kctx,
				element->kctx->tgid,
				atomic_read(&(element->kctx->used_pages)),
				element->kctx->max_used_pages);
#else
			seq_printf(sfile, "  %s-0x%p %10u\n",
				"kctx",
				element->kctx,
				atomic_read(&(element->kctx->used_pages)));
#endif
		}
		mutex_unlock(&kbdev->kctx_list_lock);
	}
	kbase_dev_list_put(kbdev_list);
	return 0;
}

/*
 *  File operations related to debugfs entry for gpu_memory
 */
static int kbasep_gpu_memory_debugfs_open(struct inode *in, struct file *file)
{
	return single_open(file, kbasep_gpu_memory_seq_show, NULL);
}

static const struct file_operations kbasep_gpu_memory_debugfs_fops = {
	.open = kbasep_gpu_memory_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

/*
 *  Initialize debugfs entry for gpu_memory
 */
void kbasep_gpu_memory_debugfs_init(struct kbase_device *kbdev)
{
	debugfs_create_file("gpu_memory", S_IRUGO,
			kbdev->mali_debugfs_directory, NULL,
			&kbasep_gpu_memory_debugfs_fops);
	return;
}

#else
/*
 * Stub functions for when debugfs is disabled
 */
void kbasep_gpu_memory_debugfs_init(struct kbase_device *kbdev)
{
	return;
}
#endif

/* ACOS_MOD_BEGIN {fwk_crash_log_collection} */
void lmk_add_to_buffer(const char *fmt, ...);

void kbasep_gpu_memory_lmk(void)
{
	struct list_head *entry;
	const struct list_head *kbdev_list;

	kbdev_list = kbase_dev_list_get();
	list_for_each(entry, kbdev_list) {
		struct kbase_device *kbdev = NULL;
		struct kbasep_kctx_list_element *element;

		kbdev = list_entry(entry, struct kbase_device, entry);
		/* output the total memory usage and cap for this device */
		lmk_add_to_buffer("device name   : %s\n", kbdev->devname);
		lmk_add_to_buffer("used pages    : %u\n", atomic_read(&(kbdev->memdev.used_pages)) * PAGE_SIZE);
#ifdef MSTAR_MEMORY_USAGE
		lmk_add_to_buffer("max used pages: %u\n", kbdev->memdev.max_used_pages * PAGE_SIZE);
		lmk_add_to_buffer("%-25s  %-10s  %-10s  %-10s  %-10s  %-10s  %-10s %-10s  %-10s  %-10s\n", "kbase_context", "pid", "gpu_mem", "max_gpu_mem",
							"native_mem", "umm_mem", "ext_mem", "alias_mem", "raw_mem");
		lmk_add_to_buffer("============================================================================\n");
#else
		lmk_add_to_buffer("%-25s  %-10s  %-10s  %-10s  %-10s  %-10s %-10s  %-10s  %-10s\n", "kbase_context", "pid", "gpu_mem",
							"native_mem", "umm_mem", "ext_mem", "alias_mem", "raw_mem";
		lmk_add_to_buffer("============================================================\n");
#endif

		mutex_lock(&kbdev->kctx_list_lock);
		list_for_each_entry(element, &kbdev->kctx_list, link) {
			int bit = 0;
			struct kbase_context *kctx = element->kctx;
			unsigned long native_pages = 0;
			unsigned long umm_pages = 0;
			unsigned long ext_pages = 0;
			unsigned long alias_pages = 0;
			unsigned long raw_pages = 0;
			for (bit = 0 ; bit < BITS_PER_LONG ; bit++){
				struct kbase_va_region *reg = kctx->pending_regions[bit];

				if (reg == NULL)
					continue;
				if (reg->gpu_alloc == NULL)
					continue;

				switch (reg->gpu_alloc->type) {
				case KBASE_MEM_TYPE_NATIVE:
					native_pages += reg->nr_pages;
					break;
				case KBASE_MEM_TYPE_IMPORTED_UMM:
					umm_pages += reg->nr_pages;
					break;
				case KBASE_MEM_TYPE_IMPORTED_USER_BUF:
					ext_pages += reg->nr_pages;
				case KBASE_MEM_TYPE_ALIAS:
					alias_pages += reg->nr_pages;
					break;
				case KBASE_MEM_TYPE_RAW:
					raw_pages += reg->nr_pages;
					break;
				}
			}

#ifdef MSTAR_MEMORY_USAGE
			lmk_add_to_buffer("  %s-0x%p  %-10u  %-10u  %-10u %-10u  %-10u %-10u %-10u  %-10u\n",
				"kctx",
				element->kctx,
				element->kctx->tgid,
				atomic_read(&(element->kctx->used_pages)) * PAGE_SIZE,
				element->kctx->max_used_pages * PAGE_SIZE,
				native_pages  * PAGE_SIZE,
				umm_pages * PAGE_SIZE,
				ext_pages * PAGE_SIZE,
				alias_pages * PAGE_SIZE,
				raw_pages * PAGE_SIZE);
#else
			lmk_add_to_buffer("  %s-0x%p  %-10u  %-10u  %-10u  %-10u  %-10u %-10u %-10u  %-10u\n",
				"kctx",
				element->kctx,
				element->kctx->tgid,
				atomic_read(&(element->kctx->used_pages)) * PAGE_SIZE,
				native_pages  * PAGE_SIZE,
				umm_pages * PAGE_SIZE,
				ext_pages * PAGE_SIZE,
				alias_pages * PAGE_SIZE,
				raw_pages * PAGE_SIZE);
#endif
		}
		mutex_unlock(&kbdev->kctx_list_lock);
	}
	kbase_dev_list_put(kbdev_list);
}
EXPORT_SYMBOL(kbasep_gpu_memory_lmk);
/* ACOS_MOD_END {fwk_crash_log_collection} */
