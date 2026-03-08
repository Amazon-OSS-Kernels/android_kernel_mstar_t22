/*
 * drivers/staging/android/ion/ion_mem_pool.c
 *
 * Copyright (C) 2011 Google, Inc.
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

#include <linux/debugfs.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include "ion_priv.h"
#include "ion.h"

#ifdef CONFIG_ION_FB_CACHE
#include <linux/module.h>

/*
 * Two level minfree thresholds for low order (2 and 0) cache pools.
 *
 * The top threshold is between launcher BG kill and cache app kill.
 * It makes system keep EGL cache over cache applications
 * EGL cache becomes reclaimable before reaching to launcher BG kill.
 * When launcher trims EGL memory, memory is moved to EGL cache
 * rather than being freed. Hence it also helps on speeding up EGL
 * memory allocation when switch back to launcher.
 * This design highly depends on launcher to free EGL memory at
 * background on memory pressure.
 *
 * The bottom threshold is lightly below launcher BG kill.
 * When launcher is killed at background, system is really short on free
 * memory. The EGL cache mainained below bottom minfree helps on bringing
 * out flyouts like QS Alexa Serach. And it doesn't have an impact to FG 
 * kill as the cache becomes reclaimable before FG kill.
 */

/* keep 96MB egl cache when no memory pressue */
int egl_cache_count_top = 24576;
/* keep 24MB egl cache before launcher is killed at BG */
int egl_cache_count_bottom = 6144;

/* default 160 MB, an ideal size should be a little bit greater than Netflix BG kill */
int egl_cache_minfree_top = 40960;
/* default 96MB MB, an ideal size should be a little bit smaller than Launcher BG kill */
int egl_cache_minfree_bottom = 24576;

#endif

#if CONFIG_MP_MMA_UMA_WITH_NARROW
static void *ion_page_pool_alloc_pages(struct ion_page_pool *pool,unsigned long flags)
{
	struct page *page;
	gfp_t gfp_mask = pool->gfp_mask;

	if(flags & ION_FLAG_DMAZONE){
		gfp_mask &= ~__GFP_HIGHMEM;
#ifdef CONFIG_DMA_CMA
		gfp_mask |= GFP_DMA;
#endif
	}
	if(!(flags & ION_FLAG_IOMMU_FLUSH))
		gfp_mask |= __GFP_ZERO;

	page = alloc_pages(gfp_mask,pool->order);

	if (!page)
		return NULL;

	if (!pool->cached && !(flags & ION_FLAG_IOMMU_FLUSH))
		ion_pages_sync_for_device(NULL, page, PAGE_SIZE << pool->order,
					  DMA_BIDIRECTIONAL);

	return page;
}
#else
static void *ion_page_pool_alloc_pages(struct ion_page_pool *pool)
{
	struct page *page = alloc_pages(pool->gfp_mask, pool->order);

	if (!page)
		return NULL;
	if (!pool->cached)
		ion_pages_sync_for_device(NULL, page, PAGE_SIZE << pool->order,
					  DMA_BIDIRECTIONAL);
	return page;
}

#endif
static void ion_page_pool_free_pages(struct ion_page_pool *pool,
				     struct page *page)
{
	__free_pages(page, pool->order);
}
#if CONFIG_MP_MMA_UMA_WITH_NARROW
static int ion_page_pool_add(struct ion_page_pool *pool, struct page *page)
{
	mutex_lock(&pool->mutex);
	if (ZONE_NORMAL == page_zonenum(page) || PageHighMem(page)) {
		list_add_tail(&page->lru, &pool->high_items);
		pool->high_count++;
	} else {
		list_add_tail(&page->lru, &pool->low_items);
		pool->low_count++;
	}

	mutex_unlock(&pool->mutex);
	return 0;
}

#else
static int ion_page_pool_add(struct ion_page_pool *pool, struct page *page)
{
	mutex_lock(&pool->mutex);
	if (PageHighMem(page)) {
		list_add_tail(&page->lru, &pool->high_items);
		pool->high_count++;
	} else {
		list_add_tail(&page->lru, &pool->low_items);
		pool->low_count++;
	}
	mutex_unlock(&pool->mutex);
	return 0;
}
#endif
static struct page *ion_page_pool_remove(struct ion_page_pool *pool, bool high)
{
	struct page *page;

	if (high) {
		BUG_ON(!pool->high_count);
		page = list_first_entry(&pool->high_items, struct page, lru);
		pool->high_count--;
	} else {
		BUG_ON(!pool->low_count);
		page = list_first_entry(&pool->low_items, struct page, lru);
		pool->low_count--;
	}

	list_del(&page->lru);
	return page;
}
#if CONFIG_MP_MMA_UMA_WITH_NARROW
struct page *ion_page_pool_alloc(struct ion_page_pool *pool,unsigned long flags, bool new)
{
	struct page *page = NULL;
	BUG_ON(!pool);

	mutex_lock(&pool->mutex);

	if (pool->high_count && (!(flags & ION_FLAG_DMAZONE)))
		page = ion_page_pool_remove(pool, true);
	else if (pool->low_count)
		page = ion_page_pool_remove(pool, false);
	mutex_unlock(&pool->mutex);

	if (!page && new)
		page = ion_page_pool_alloc_pages(pool,flags);

	return page;
}

#else
struct page *ion_page_pool_alloc(struct ion_page_pool *pool)
{
	struct page *page = NULL;

	BUG_ON(!pool);

	mutex_lock(&pool->mutex);
	if (pool->high_count)
		page = ion_page_pool_remove(pool, true);
	else if (pool->low_count)
		page = ion_page_pool_remove(pool, false);
	mutex_unlock(&pool->mutex);

	if (!page)
		page = ion_page_pool_alloc_pages(pool);

	return page;
}
#endif

void ion_page_pool_free(struct ion_page_pool *pool, struct page *page)
{
	int ret;

	BUG_ON(pool->order != compound_order(page));

#ifdef CONFIG_MP_CMA_PATCH_ION_LOW_ORDER_ALLOC
	ion_page_pool_free_pages(pool, page);
#else
	ret = ion_page_pool_add(pool, page);
	if (ret)
		ion_page_pool_free_pages(pool, page);
#endif
}

#ifdef CONFIG_ION_FB_CACHE
static int get_min_cache(int nr_to_scan)
{
	int min_cache = egl_cache_count_top;
	int minfree = 0;
	int other_file = global_node_page_state(NR_FILE_PAGES) -
				global_node_page_state(NR_SHMEM) -
				global_node_page_state(NR_UNEVICTABLE) -
				total_swapcache_pages();

	if (other_file < egl_cache_minfree_bottom) {
		min_cache = 0; /* all cache pool can be freed now*/
		minfree = egl_cache_minfree_bottom;
	}
	else if (other_file < egl_cache_minfree_top) {
		min_cache = egl_cache_count_bottom; /* keep half cache */
		minfree = egl_cache_minfree_top;
	}

	if (min_cache != egl_cache_count_top) {
		long size = nr_to_scan * (long)(PAGE_SIZE / 1024);
		long cache_size = other_file * (long)(PAGE_SIZE / 1024);
		long cache_limit = minfree * (long)(PAGE_SIZE / 1024);
#if 0
		pr_info_ratelimited("ion: shrink from reserved pool on behalf of '%s' (%d) because cache %ldkB is below limit %ldkB, nr_to_scan: %ldkB\n",
			     current->comm, current->pid,
			     cache_size, cache_limit, size);
		pr_info_ratelimited("ion: top_minfree is %ldkB, bottom minfree is %ldkB\n",
			     egl_cache_minfree_top * (long)(PAGE_SIZE / 1024),
			     egl_cache_minfree_bottom * (long)(PAGE_SIZE / 1024));
#endif
	}

	return min_cache;
}
#endif

static int ion_page_pool_total(struct ion_page_pool *pool, bool high)
{
	int count = pool->low_count;

	if (high)
		count += pool->high_count;


#ifdef CONFIG_ION_FB_CACHE
	if ((pool->order == 8) &&
		(pool->cached == ION_FB_CACHE_CACHED)) {
		count -= ION_ORDER_8_CACHE_CNT;
		if (count < 0)
			count = 0;
	} else if ((pool->order == 4) &&
		(pool->cached == ION_FB_CACHE_CACHED)) {
		count -= ION_ORDER_4_CACHE_CNT;
		if (count < 0)
			count = 0;
	}
#endif

	return count << pool->order;
}

int ion_page_pool_shrink(struct ion_page_pool *pool, gfp_t gfp_mask,
			 int nr_to_scan)
{
	int freed = 0;
	bool high;

	if (current_is_kswapd())
		high = true;
	else
		high = !!(gfp_mask & __GFP_HIGHMEM);

	if (nr_to_scan == 0)
		return ion_page_pool_total(pool, high);

#ifdef CONFIG_ION_FB_CACHE
	mutex_lock(&pool->mutex);
	if ((pool->order == 8) &&
		(pool->cached == ION_FB_CACHE_CACHED)) {
		int available_cache = (pool->low_count + pool->high_count) -
			ION_ORDER_8_CACHE_CNT;
		if (available_cache <= 0) {
			mutex_unlock(&pool->mutex);
			return 0;
		}
		if (nr_to_scan > (available_cache << 8))
			nr_to_scan = available_cache << 8;
	}
	else if ((pool->order == 4) &&
		(pool->cached == ION_FB_CACHE_CACHED)) {
		int available_cache = (pool->low_count + pool->high_count) -
			ION_ORDER_4_CACHE_CNT;
		if (available_cache <= 0) {
			mutex_unlock(&pool->mutex);
			return 0;
		}

		if (nr_to_scan > (available_cache << 4))
			nr_to_scan = available_cache << 4;
	}
	else if (pool->cached == ION_FB_CACHE_CACHED) {
		int available_cache = ((pool->low_count + pool->high_count) << pool->order) -
			get_min_cache(nr_to_scan) / NR_OF_LOW_ORDER_CACHE_POOL;
		if (available_cache <= 0) {
			mutex_unlock(&pool->mutex);
			return 0;
		}
		if (nr_to_scan > available_cache)
			nr_to_scan = available_cache;
	}
	if ((pool->low_count + pool->high_count) > 0)
		pr_info_ratelimited("ion_pool: shrink %d pages in order %d pool. %d pages in pool\n",
			nr_to_scan, pool->order, ((pool->low_count + pool->high_count) << pool->order));

	mutex_unlock(&pool->mutex);
#endif

	while (freed < nr_to_scan) {
		struct page *page;

		mutex_lock(&pool->mutex);
		if (pool->low_count) {
			page = ion_page_pool_remove(pool, false);
		} else if (high && pool->high_count) {
			page = ion_page_pool_remove(pool, true);
		} else {
			mutex_unlock(&pool->mutex);
			break;
		}
		mutex_unlock(&pool->mutex);
		ion_page_pool_free_pages(pool, page);
		freed += (1 << pool->order);
	}

	return freed;
}

struct ion_page_pool *ion_page_pool_create(gfp_t gfp_mask, unsigned int order,
					   bool cached)
{
	struct ion_page_pool *pool = kmalloc(sizeof(*pool), GFP_KERNEL);

	if (!pool)
		return NULL;
	pool->high_count = 0;
	pool->low_count = 0;
	INIT_LIST_HEAD(&pool->low_items);
	INIT_LIST_HEAD(&pool->high_items);
	pool->gfp_mask = gfp_mask | __GFP_COMP;
	pool->order = order;
	mutex_init(&pool->mutex);
	plist_node_init(&pool->list, order);
	pool->cached = cached;

#ifdef CONFIG_ION_FB_CACHE
	if ((order == 8) &&
		(cached == ION_FB_CACHE_CACHED)) {
		int i;
		struct page *page = NULL;

		/* use low order flag to make sure allocation won't fail */
		pool->gfp_mask = (GFP_HIGHUSER | __GFP_ZERO|__GFP_COMP);
		for (i = 0; i < ION_ORDER_8_CACHE_CNT; i++) {
#if CONFIG_MP_MMA_UMA_WITH_NARROW
			page = ion_page_pool_alloc_pages(pool, ION_FLAG_DMAZONE);
#else
			page = ion_page_pool_alloc_pages(pool);
#endif
			if (page) {
				ion_page_pool_add(pool, page);
			}
			page = NULL;
		}
		pool->gfp_mask = gfp_mask | __GFP_COMP;
	} else if ((order == 4) &&
		(cached == ION_FB_CACHE_CACHED)) {
		int i;
		struct page *page = NULL;

		/* use low order flag to make sure allocation won't fail */
		pool->gfp_mask = (GFP_HIGHUSER | __GFP_ZERO|__GFP_COMP);
		for (i = 0; i < ION_ORDER_4_CACHE_CNT; i++) {
#if CONFIG_MP_MMA_UMA_WITH_NARROW
			page = ion_page_pool_alloc_pages(pool, ION_FLAG_DMAZONE);
#else
			page = ion_page_pool_alloc_pages(pool);
#endif
			if (page) {
				ion_page_pool_add(pool, page);
			}
			page = NULL;
		}
		pool->gfp_mask = gfp_mask | __GFP_COMP;
	}
#endif

	return pool;
}

void ion_page_pool_destroy(struct ion_page_pool *pool)
{
	kfree(pool);
}

static int __init ion_page_pool_init(void)
{
	return 0;
}

device_initcall(ion_page_pool_init);

#ifdef CONFIG_ION_FB_CACHE
MODULE_PARM_DESC(cache_count_top, "cached EGL pages, reclaimable when file cache is below top threshold");
module_param_named(cache_count_top, egl_cache_count_top, int, 0664);
MODULE_PARM_DESC(cache_count_bottom, "cached EGL pages, reclaimable when file cache is below bottom threshold");
module_param_named(cache_count_bottom, egl_cache_count_bottom, int, 0664);
MODULE_PARM_DESC(minfree_top, "top threshold for file cache");
module_param_named(minfree_top, egl_cache_minfree_top, int, 0664);
MODULE_PARM_DESC(minfree_bottom, "bottom threshold for file cache");
module_param_named(minfree_bottom, egl_cache_minfree_bottom, int, 0664);
#endif
