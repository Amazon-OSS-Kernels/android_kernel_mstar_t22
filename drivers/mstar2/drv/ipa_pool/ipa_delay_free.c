/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   ipa_delay_free.c
/// @delay free ipa pool allocation
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/interval_tree_generic.h>
#include <mdrv_ipa_pool.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include "ipa_pool.h"

#define BUFFER_SIZE 32
#define KBYTE_SHIFT 10
#define KBYTE_MASK  ((1<<10) - 1)

#define IPA_PROC_DELAY_FREE_ENABLE "delay_free_enable"
#define IPA_PROC_DELAY_FREE_SIZELIMIT "delay_free_sizelimit"
#define IPA_PROC_DELAY_FREE_TIMELIMIT "delay_free_timelimit"
#define IPA_PROC_DELAY_FREE_FORCEFREE "delay_free_forcefree"

#define IPA_DELAY_FREE_SLEEP_TIMER            5000     //delay thread sleep interval 5s
#define IPA_DELAY_FREE_DEFAULT_TIMEOUT   30000   //default timeout 30s
#define IPA_DELAY_FREE_DEFAULT_SIZELIMIT 0         //defalut size limit 0M, that is all buffer will be added to delay pool

#define IPA_DELAY_FREE_POOL_NAME "delay_free_pool"

#define IPA_DEBUG
#define IPA_ERROR

#ifdef IPA_DEBUG
#define IPA_DELAY_FREE_DEBUG(fmt, args...) \
    printk(KERN_DEBUG "%s %d: " fmt, __FUNCTION__, __LINE__, ## args)    
#else
#define IPA_DELAY_FREE_DEBUG(fmt, args...)
#endif

#ifdef IPA_ERROR
#define IPA_DELAY_FREE_ERROR(fmt, args...) \
    printk(KERN_ERR "%s %d: " fmt, __FUNCTION__, __LINE__, ## args)
#else
#define IPA_DELAY_FREE_ERROR(fmt, args...) 
#endif

extern int lowmem_minfree[6];
extern int lowmem_minfree_size;
extern struct IPA_space_heap_man IPASpcHeapMan;
extern struct IPAPool_device IPAPoolDev;

static int delay_free_lowmem_minfree = 17 * 1024;
static int delay_free_heap_count = 0;
static struct proc_dir_entry *delay_free_root = NULL;
static struct task_struct *delay_free_task = NULL;

static int ipa_heap_delay_free_force_free(struct IPA_heap *heap);
static int ipa_heap_delay_free_thread(void* arg);

static int node_range_size_test(void *param1, void *param2)
{
    struct IPA_heap *heap = (struct IPA_heap *)param1;
    struct pool_range_node *node = (struct pool_range_node *)param2;

    if(node->length >= heap->delay_free_sizelimit)
        return 1;
    return 0;
}

static unsigned long delay_free_pool_size(struct IPA_pool *pool)
{
    struct pool_range_node *node, *next;
    struct list_head head;
    unsigned long totalsize=0;
    int ret;
    
    INIT_LIST_HEAD(&head);
    ret = ipa_pool_search_valid_range(pool, NULL, 0, pool->len, &head, NULL);
    if(ret)
        goto out;
    list_for_each_entry_safe(node, next, &head, list) {
        totalsize += node->length;
        list_del(&node->list);
        kfree(node);
    }
    
out:
    return totalsize; 
}

static int delay_free_shrink(struct shrinker *s, struct shrink_control *sc)
{
    struct IPA_heap *heap = NULL;
    int other_free = global_page_state(NR_FREE_PAGES) - totalreserve_pages;
    int other_file = global_page_state(NR_FILE_PAGES) - global_page_state(NR_SHMEM);
    int free_cma = 0;
    int to_scan = sc->nr_to_scan;
    unsigned long total = 0;

    if(to_scan == 0)
        goto out;

#ifdef CONFIG_CMA
    if (allocflags_to_migratetype(sc->gfp_mask) != MIGRATE_MOVABLE)
        free_cma = global_page_state(NR_FREE_CMA_PAGES);
#endif

    if(delay_free_lowmem_minfree < lowmem_minfree[lowmem_minfree_size-1])
        delay_free_lowmem_minfree = lowmem_minfree[lowmem_minfree_size-1];

    if ((other_free - free_cma) < delay_free_lowmem_minfree && other_file < delay_free_lowmem_minfree) {
        mutex_lock(&IPASpcHeapMan.lock);
        list_for_each_entry(heap, &IPASpcHeapMan.heaps, node) {
            mutex_lock(&heap->delay_free_mutex);
            if(heap->delay_free_pool) {
                ipa_heap_delay_free_force_free(heap);
            }
            mutex_unlock(&heap->delay_free_mutex);
        }
        mutex_unlock(&IPASpcHeapMan.lock);
    }

out:
    mutex_lock(&IPASpcHeapMan.lock);
    list_for_each_entry(heap, &IPASpcHeapMan.heaps, node) {
        mutex_lock(&heap->delay_free_mutex);
        if(heap->delay_free_pool) {
            total += delay_free_pool_size(heap->delay_free_pool);
        }
        mutex_unlock(&heap->delay_free_mutex);
    }
    mutex_unlock(&IPASpcHeapMan.lock);

    return total / PAGE_SIZE;
}

static struct shrinker delay_free_shrinker = {
    .shrink = delay_free_shrink,
    .seeks = DEFAULT_SEEKS * 16
};

int ipa_heap_delay_free_enable(struct IPA_heap *heap)
{
    struct IPA_pool *pool;
    int ret = 0;
    char pool_name[BUFFER_SIZE] = {0};

    //mutex lock outside
    //Already enabled
    if(heap->delay_free_pool) {
        return ret;
    }

    heap->delay_free_timelimit = IPA_DELAY_FREE_DEFAULT_TIMEOUT;
    heap->delay_free_sizelimit = IPA_DELAY_FREE_DEFAULT_SIZELIMIT;

    sprintf(pool_name, "%s_hid_%d", IPA_DELAY_FREE_POOL_NAME, heap->hid);

    mutex_lock(&IPAPoolDev.apilock);
    ret = _IPAPool_create_pool(heap->hid, IPA_POOL_FLAG_NO_CONFLICT, 0, 
        heap->len, pool_name, &pool);
    mutex_unlock(&IPAPoolDev.apilock);

    if(!pool) {
        IPA_DELAY_FREE_ERROR("Delay free pool failed to create for heap%d\n", heap->hid);
        BUG();
    }
    
    pool->flags |= IPA_POOL_FLAG_NO_CONFLICT;
    heap->delay_free_pool = pool;

    if(!delay_free_task) {
        delay_free_task = kthread_create(ipa_heap_delay_free_thread, NULL, "IPA Delay Free Thread");
        if(delay_free_task) {
            wake_up_process(delay_free_task);
        }
    }

    delay_free_heap_count++;
    if(delay_free_heap_count == 1)
        register_shrinker(&delay_free_shrinker);

    return ret;    
}

int ipa_heap_delay_free_disable(struct IPA_heap *heap)
{
    struct IPA_pool *pool = heap->delay_free_pool;
    int ret;
    
    //mutex lock outside
    //Already disabled
    if(!pool)
        return 0;

    ret = pool_free(pool, NULL, 0, pool->len);
    if(ret)
        goto out;

    mutex_lock(&IPAPoolDev.apilock);
    ret = _IPAPool_release_pool(pool);
    mutex_unlock(&IPAPoolDev.apilock);
    if(ret)
        goto out;

    heap->delay_free_pool = NULL;

    delay_free_heap_count--;
    if(delay_free_heap_count < 0)
        BUG();
    if(delay_free_heap_count ==0)
        unregister_shrinker(&delay_free_shrinker);

out:
    return ret;    
}


int IPA_pool_delay_free(int pool_id, unsigned long pool_offset, unsigned long length)
{
    struct IPA_heap *heap = NULL;
    struct IPA_pool *pool = NULL;
    struct IPA_pool_ref *pr = NULL;
    struct file *fp = NULL;
    struct list_head head;
    struct pool_range_node *node, *next;
    int ret = 0;

    INIT_LIST_HEAD(&head);

    ret=_IPA_pool_entry(pool_id, pool_offset, length, &fp, &pr, &pool);
    if(ret < 0){
        IPA_DELAY_FREE_ERROR("Invalid para\n");
        return ret;
    }
    
    mutex_lock(&IPASpcHeapMan.lock);
    heap = _IPASpcMan_find_heap(pool->hid);
    if(!heap) {
        IPA_DELAY_FREE_ERROR("Invalid heap\n");
        BUG();
    }
    mutex_unlock(&IPASpcHeapMan.lock);

    mutex_lock(&heap->delay_free_mutex);
    if(!ret && heap->delay_free_pool) {
        ret = ipa_pool_search_valid_range(pool, fp, pool->heap_off + pool_offset, length, &head, NULL);
        if(ret)
            goto out;

        ret = ipa_pool_alloc_from_range_list(heap->delay_free_pool, NULL, &head, 
                node_range_size_test);
        if(ret)
            goto out;
    }

out:
    list_for_each_entry_safe(node, next, &head, list) {
        list_del(&node->list);
        kfree(node);
    }
    mutex_unlock(&heap->delay_free_mutex);
    return ret;
}

static int ipa_heap_delay_free_force_free(struct IPA_heap *heap)
{
    struct IPA_pool *pool = heap->delay_free_pool;
    int ret;

    //Mutex lock outside
    if(!pool)
        return 0;
    IPA_DELAY_FREE_ERROR("delay free pool size=%llx\n", delay_free_pool_size(heap->delay_free_pool));
    ret = pool_free(pool, NULL, 0, heap->len);
    return ret;
}

static int ipa_heap_delay_free_thread(void* arg)
{
    struct list_head head;
    struct IPA_heap *heap;
    struct pool_range_node *node, *next;
    int ret;
    
    while(1) {
        //mutex_lock(&IPASpcHeapMan.lock);
        list_for_each_entry(heap, &IPASpcHeapMan.heaps, node) {
            INIT_LIST_HEAD(&head);
            mutex_lock(&heap->delay_free_mutex);
            if(!heap->delay_free_pool) {
                mutex_unlock(&heap->delay_free_mutex);
                continue;
            }
            
            if(heap->delay_free_pool) {
                ret = ipa_pool_search_valid_range(heap->delay_free_pool, NULL, 0, heap->len, 
                    &head, pool_allocation_timeout_test);
                if(ret)
                    IPA_DELAY_FREE_DEBUG("Incomplete node\n");

                list_for_each_entry_safe(node, next, &head, list) {
                   IPA_DELAY_FREE_DEBUG("timeout pool: %s, start %lx, length %lx\n", 
                        heap->delay_free_pool->name, node->start, node->length);
                    ret = pool_free(heap->delay_free_pool, NULL, node->start, node->length);
                    BUG_ON(ret);
                    list_del(&node->list);
                    kfree(node);
                }
            }
            mutex_unlock(&heap->delay_free_mutex);
        }
        //mutex_unlock(&IPASpcHeapMan.lock);
        schedule_timeout_interruptible(IPA_DELAY_FREE_SLEEP_TIMER);
    }
}

/*
 *delay free enable proc interface
 */
static int ipa_delay_free_enable_show(struct seq_file *s, void *v)
{
    struct IPA_heap *heap = s->private;
    int ret;
    
    mutex_lock(&heap->delay_free_mutex);
    if(heap->delay_free_pool)
        ret = seq_printf(s, "delay free enable: %d\n", 1);
    else
        ret = seq_printf(s, "delay free enable: %d\n", 0);
    mutex_unlock(&heap->delay_free_mutex);

    return ret;
}
static ssize_t ipa_delay_free_enable_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    struct IPA_heap *heap = ((struct seq_file *)(file->private_data))->private;
    char kbuf[BUFFER_SIZE] = {0};
    int enable, ret;
    
    copy_from_user(kbuf, buf, size);
    sscanf(kbuf, "%d", &enable);

    mutex_lock(&heap->delay_free_mutex);
    if(enable)
        ret = ipa_heap_delay_free_enable(heap);
    else
        ret = ipa_heap_delay_free_disable(heap);
    mutex_unlock(&heap->delay_free_mutex);

    return ret ? ret : size;
}
static int ipa_delay_free_enable_open(struct inode *inode, struct file *file)
{
    struct IPA_heap *heap = PDE_DATA(inode);

    if(!heap)
        return -EINVAL;

    return single_open(file, ipa_delay_free_enable_show, heap);
}
static const struct file_operations ipa_delay_free_fops_enable = {
    .owner = THIS_MODULE,
    .read = seq_read,
    .write = ipa_delay_free_enable_write,
    .open = ipa_delay_free_enable_open,
    .release = seq_release,
};

/*
 *time limit proc interface
 */
static int ipa_delay_free_timelimit_show(struct seq_file *s, void *v)
{
    struct IPA_heap *heap = s->private;
    int ret;
    
    mutex_lock(&heap->delay_free_mutex);
    ret = seq_printf(s, "%d (%d seconds)\n", heap->delay_free_timelimit, heap->delay_free_timelimit / 1000);
    mutex_unlock(&heap->delay_free_mutex);

    return ret;
}
static ssize_t ipa_delay_free_timelimit_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    struct IPA_heap *heap = ((struct seq_file *)(file->private_data))->private;
    char kbuf[BUFFER_SIZE] = {0};
    int timelimit;
    
    copy_from_user(kbuf, buf, size);
    sscanf(kbuf, "%d", &timelimit);
    mutex_lock(&heap->delay_free_mutex);
    heap->delay_free_timelimit = timelimit;
    mutex_unlock(&heap->delay_free_mutex);

    return size;
}
static int ipa_delay_free_timelimit_open(struct inode *inode, struct file *file)
{
    struct IPA_heap *heap = PDE_DATA(inode);

    if(!heap)
        return -EINVAL;

    return single_open(file, ipa_delay_free_timelimit_show, heap);
}
static const struct file_operations ipa_delay_free_fops_timelimit = {
    .owner = THIS_MODULE,
    .read = seq_read,
    .write = ipa_delay_free_timelimit_write,
    .open = ipa_delay_free_timelimit_open,
    .release = seq_release,
};

/*
 *size limit proc interface
 */
static int ipa_delay_free_sizelimit_show(struct seq_file *s, void *v)
{
    struct IPA_heap *heap = s->private;
    int mbytes, kbytes, bytes, sizelimit;
    int ret;

    mutex_lock(&heap->delay_free_mutex);
    sizelimit = heap->delay_free_sizelimit;
    bytes = sizelimit & (KBYTE_MASK);
    sizelimit = sizelimit >> KBYTE_SHIFT;
    kbytes = sizelimit & (KBYTE_MASK);
    sizelimit = sizelimit >> KBYTE_SHIFT;
    mbytes = sizelimit & (KBYTE_MASK);
    ret = seq_printf(s, "%d(%dM%dK%dbytes)\n", heap->delay_free_sizelimit, mbytes, kbytes, bytes);
    mutex_unlock(&heap->delay_free_mutex);

    return ret;
}
static ssize_t ipa_delay_free_sizelimit_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    struct IPA_heap *heap = ((struct seq_file *)(file->private_data))->private;
    char kbuf[BUFFER_SIZE] = {0};
    int sizelimit;
    
    copy_from_user(kbuf, buf, size);
    sscanf(kbuf, "%d", &sizelimit);

    mutex_lock(&heap->delay_free_mutex);
    heap->delay_free_sizelimit = sizelimit;
    mutex_unlock(&heap->delay_free_mutex);

    return size;
}
static int ipa_delay_free_sizelimit_open(struct inode *inode, struct file *file)
{
    struct IPA_heap *heap = PDE_DATA(inode);

    if(!heap)
        return -EINVAL;

    return single_open(file, ipa_delay_free_sizelimit_show, heap);
}
static const struct file_operations ipa_delay_free_fops_sizelimit = {
    .owner = THIS_MODULE,
    .read = seq_read,
    .write = ipa_delay_free_sizelimit_write,
    .open = ipa_delay_free_sizelimit_open,
    .release = seq_release,
};

/*
 *force free proc interface
 */
static ssize_t ipa_delay_free_forcefree_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
    struct IPA_heap *heap = ((struct seq_file *)(file->private_data))->private;
    char kbuf[BUFFER_SIZE] = {0};
    int forcefree, ret = 0;
    
    copy_from_user(kbuf, buf, size);
    sscanf(kbuf, "%d", &forcefree);

    if(forcefree) {
        mutex_lock(&heap->delay_free_mutex);
        ret = ipa_heap_delay_free_force_free(heap);
        mutex_unlock(&heap->delay_free_mutex);
    }

    return ret ? ret : size;
}
static int ipa_delay_free_forcefree_open(struct inode *inode, struct file *file)
{
    struct IPA_heap *heap = PDE_DATA(inode);

    if(!heap)
        return -EINVAL;

    return single_open(file, NULL, heap);
}
static const struct file_operations ipa_delay_free_fops_forcefree = {
    .owner = THIS_MODULE,
    .write = ipa_delay_free_forcefree_write,
    .open = ipa_delay_free_forcefree_open,
    .release = seq_release,
};

int ipa_heap_delay_free_init(struct IPA_heap *heap)
{
    char name[BUFFER_SIZE] = {0};

    if(!heap)
        return 0;

    heap->delay_free_timelimit = 0;
    heap->delay_free_sizelimit = 0;

    mutex_init(&heap->delay_free_mutex);

    mutex_lock(&heap->delay_free_mutex);
    //Create root ipa heap directory
    if(!delay_free_root) {
        sprintf(name, "ipa-heap");
        delay_free_root = proc_mkdir(name, NULL);  
        if (!delay_free_root)  {  
            IPA_DELAY_FREE_ERROR("Create dir /proc/%s error!\n", name);
            mutex_unlock(&heap->delay_free_mutex);
            return -EINVAL;  
        }
    }
    
    //Create directory for specified heap
    sprintf(name, "heap%d", heap->hid);
    heap->proc_root = proc_mkdir(name, delay_free_root);  
    if (!heap->proc_root)  {  
        IPA_DELAY_FREE_ERROR("Create dir /proc/%s error!\n", name);
        mutex_unlock(&heap->delay_free_mutex);
        return -EINVAL;  
    }    
    
    proc_create_data(IPA_PROC_DELAY_FREE_SIZELIMIT, 0644, heap->proc_root, 
        &ipa_delay_free_fops_sizelimit, heap);
    proc_create_data(IPA_PROC_DELAY_FREE_TIMELIMIT, 0644, heap->proc_root, 
        &ipa_delay_free_fops_timelimit, heap);
    proc_create_data(IPA_PROC_DELAY_FREE_FORCEFREE, 0644, heap->proc_root, 
        &ipa_delay_free_fops_forcefree, heap);
    proc_create_data(IPA_PROC_DELAY_FREE_ENABLE, 0644, heap->proc_root, 
        &ipa_delay_free_fops_enable, heap);
    
    mutex_unlock(&heap->delay_free_mutex);
    return 0;
}
