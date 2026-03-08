/*
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

#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "ion.h"
#include "ion_priv.h"
#include "compat_ion.h"

#if (MP_ION_PATCH_CACHE_FLUSH_MOD==1)
#include <asm/cacheflush.h>
#include <asm/outercache.h>
#endif

union ion_ioctl_arg {
#if (MP_ION_PATCH_CACHE_FLUSH_MOD==1)
	struct ion_cache_flush_data cache_flush;
#endif
	struct ion_fd_data fd;
	struct ion_allocation_data allocation;
	struct ion_handle_data handle;
	struct ion_custom_data custom;
#ifdef CONFIG_MP_ION_PATCH_MSTAR
	struct ion_user_data bus_addr_info; // for keeping bus_addr to msos layer, msos layer will change to miu/offset
	struct ion_cust_allocation_data cust_allocation;
#endif
	struct ion_heap_query query;
};

#if (MP_ION_PATCH_CACHE_FLUSH_MOD==1)
static int ion_cache_flush(unsigned long start, unsigned long end)
{
	struct mm_struct *mm = current->active_mm;
	struct vm_area_struct *vma;
	int ret=0;

	if (end < start)
		return -EINVAL;

	down_read(&mm->mmap_sem);
	vma = find_vma(mm, start);
	if (vma && vma->vm_start < end) {
		if (start < vma->vm_start)
			start = vma->vm_start;
		if (end > vma->vm_end)
			end = vma->vm_end;

		dmac_flush_range((void*)(start& PAGE_MASK), (void*)PAGE_ALIGN(end));
		outer_flush_range(start, end);
#ifndef CONFIG_OUTER_CACHE
		{
			extern void Chip_Flush_Miu_Pipe(void);
			Chip_Flush_Miu_Pipe();
		}
#endif
		up_read(&mm->mmap_sem);
		return ret;
	}

	up_read(&mm->mmap_sem);
	return -EINVAL;
}
#endif

/* Must hold the client lock */
static void user_ion_handle_get(struct ion_handle *handle)
{
	if (handle->user_ref_count++ == 0)
		kref_get(&handle->ref);
}

/* Must hold the client lock */
static struct ion_handle *user_ion_handle_get_check_overflow(
	struct ion_handle *handle)
{
	if (handle->user_ref_count + 1 == 0)
		return ERR_PTR(-EOVERFLOW);
	user_ion_handle_get(handle);
	return handle;
}

/* passes a kref to the user ref count.
 * We know we're holding a kref to the object before and
 * after this call, so no need to reverify handle.
 */
static struct ion_handle *pass_to_user(struct ion_handle *handle)
{
	struct ion_client *client = handle->client;
	struct ion_handle *ret;

	mutex_lock(&client->lock);
	ret = user_ion_handle_get_check_overflow(handle);
	ion_handle_put_nolock(handle);
	mutex_unlock(&client->lock);
	return ret;
}

/* Must hold the client lock */
static int user_ion_handle_put_nolock(struct ion_handle *handle)
{
	int ret;

	if (--handle->user_ref_count == 0)
		ret = ion_handle_put_nolock(handle);

	return ret;
}

static void user_ion_free_nolock(struct ion_client *client,
				 struct ion_handle *handle)
{
	bool valid_handle;

	WARN_ON(client != handle->client);

	valid_handle = ion_handle_validate(client, handle);
	if (!valid_handle) {
		WARN(1, "%s: invalid handle passed to free.\n", __func__);
		return;
	}
	if (handle->user_ref_count == 0) {
		WARN(1, "%s: User does not have access!\n", __func__);
		return;
	}
	user_ion_handle_put_nolock(handle);
}

static int validate_ioctl_arg(unsigned int cmd, union ion_ioctl_arg *arg)
{
	int ret = 0;

	switch (cmd) {
	case ION_IOC_HEAP_QUERY:
		ret = arg->query.reserved0 != 0;
		ret |= arg->query.reserved1 != 0;
		ret |= arg->query.reserved2 != 0;
		break;
	default:
		break;
	}

	return ret ? -EINVAL : 0;
}

/* fix up the cases where the ioctl direction bits are incorrect */
static unsigned int ion_ioctl_dir(unsigned int cmd)
{
	switch (cmd) {
	case ION_IOC_SYNC:
	case ION_IOC_FREE:
	case ION_IOC_CUSTOM:
		return _IOC_WRITE;
	default:
		return _IOC_DIR(cmd);
	}
}

long ion_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct ion_client *client = filp->private_data;
	struct ion_device *dev = client->dev;
	struct ion_handle *cleanup_handle = NULL;
	int ret = 0;
	unsigned int dir;
	union ion_ioctl_arg data;

	dir = ion_ioctl_dir(cmd);

	if (_IOC_SIZE(cmd) > sizeof(data))
		return -EINVAL;

	/*
	 * The copy_from_user is unconditional here for both read and write
	 * to do the validate. If there is no write for the ioctl, the
	 * buffer is cleared
	 */
	if (copy_from_user(&data, (void __user *)arg, _IOC_SIZE(cmd)))
		return -EFAULT;

	ret = validate_ioctl_arg(cmd, &data);
	if (ret) {
		pr_warn_once("%s: ioctl validate failed\n", __func__);
		return ret;
	}

	if (!(dir & _IOC_WRITE))
		memset(&data, 0, sizeof(data));

	switch (cmd) {
	case ION_IOC_ALLOC:
	{
		struct ion_handle *handle;

#if (MP_ION_PATCH_MSTAR == 1)
		if(data.allocation.heap_id_mask == 0xFFFFFFFF)  // for codec2.0, the libion will use all heaps
			data.allocation.heap_id_mask = 1 << 0;  // system_heap id is 0
#endif

		handle = __ion_alloc(client, data.allocation.len,
				     data.allocation.align,
				     data.allocation.heap_id_mask,
				     data.allocation.flags, true);
		if (IS_ERR(handle))
			return PTR_ERR(handle);
		data.allocation.handle = handle->id;
		cleanup_handle = handle;
		pass_to_user(handle);
		break;
	}
	case ION_IOC_FREE:
	{
		struct ion_handle *handle;

		mutex_lock(&client->lock);
		handle = ion_handle_get_by_id_nolock(client, data.handle.handle);
		if (IS_ERR(handle)) {
			mutex_unlock(&client->lock);
			return PTR_ERR(handle);
		}
		user_ion_free_nolock(client, handle);
		ion_handle_put_nolock(handle);
		mutex_unlock(&client->lock);
		break;
	}
	case ION_IOC_SHARE:
	case ION_IOC_MAP:
	{
		struct ion_handle *handle;

		handle = ion_handle_get_by_id(client, data.handle.handle);
		if (IS_ERR(handle))
			return PTR_ERR(handle);
		data.fd.fd = ion_share_dma_buf_fd(client, handle);
		ion_handle_put(handle);
		if (data.fd.fd < 0)
			ret = data.fd.fd;
		break;
	}
	case ION_IOC_IMPORT:
	{
		struct ion_handle *handle;

		handle = ion_import_dma_buf_fd(client, data.fd.fd);
		if (IS_ERR(handle)) {
			ret = PTR_ERR(handle);
		} else {
			data.handle.handle = handle->id;
			handle = pass_to_user(handle);
			if (IS_ERR(handle)) {
				ret = PTR_ERR(handle);
				data.handle.handle = 0;
			}
		}
		break;
	}
	case ION_IOC_SYNC:
	{
		ret = ion_sync_for_device(client, data.fd.fd);
		break;
	}
	case ION_IOC_CUSTOM:
	{
		if (!dev->custom_ioctl)
			return -ENOTTY;
		ret = dev->custom_ioctl(client, data.custom.cmd,
						data.custom.arg);
		break;
	}
#ifdef CONFIG_MP_ION_PATCH_MSTAR
	case ION_IOC_GET_CMA_BUFFER_INFO:
	{
		/* add this to keep bus_addr info to msos layer */
		struct ion_handle *handle;
		struct ion_heap *heap;

		ion_phys_addr_t bus_addr;
		size_t buffer_len;

		handle = ion_handle_get_by_id(client, data.bus_addr_info.handle);
		if (IS_ERR(handle))
			return PTR_ERR(handle);

		heap = handle->buffer->heap;
		heap->ops->phys(heap, handle->buffer, &bus_addr, &buffer_len);
		data.bus_addr_info.bus_addr = (unsigned long)bus_addr;
		ion_handle_put(handle);
		break;
	}
	case ION_IOC_CUST_ALLOC:
	{
		struct ion_handle *handle;

		handle = ion_cust_alloc(client, data.cust_allocation.start,
					data.cust_allocation.len,
					data.cust_allocation.align,
					data.cust_allocation.heap_id_mask,
					data.cust_allocation.flags,
					&data.cust_allocation.miu,
					&data.cust_allocation.miu_offset, true);

		if (IS_ERR(handle))
			return PTR_ERR(handle);

		data.cust_allocation.handle = handle->id;

		cleanup_handle = handle;
		pass_to_user(handle);
		break;
	}
#endif
#if (MP_ION_PATCH_CACHE_FLUSH_MOD==1)
	case ION_IOC_CACHE_FLUSH:
	{
		ion_cache_flush(data.cache_flush.start, data.cache_flush.start+data.cache_flush.len);
		break;
	}
#endif
	case ION_IOC_HEAP_QUERY:
		ret = ion_query_heaps(client, &data.query);
		break;
	default:
		return -ENOTTY;
	}

	if (dir & _IOC_READ) {
		if (copy_to_user((void __user *)arg, &data, _IOC_SIZE(cmd))) {
			if (cleanup_handle) {
				mutex_lock(&client->lock);
				user_ion_free_nolock(client, cleanup_handle);
				ion_handle_put_nolock(cleanup_handle);
				mutex_unlock(&client->lock);
			}
			return -EFAULT;
		}
	}
	if (cleanup_handle)
		ion_handle_put(cleanup_handle);
	return ret;
}
