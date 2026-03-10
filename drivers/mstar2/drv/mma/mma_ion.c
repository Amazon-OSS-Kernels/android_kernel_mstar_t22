#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/ioctl.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ion.h>
#include <linux/mm.h>

#include "mma_core.h"
#include "mma_of.h"
#include "mma_ion.h"
#ifdef CONFIG_MP_MMA_CMA_ENABLE
extern int ion_mma_cma_miu0_heap_id;
extern int ion_mma_cma_miu1_heap_id;
extern int ion_mma_cma_miu2_heap_id;
#endif

int mma_ion_open(struct mma_device* mma_dev)
{
	if(mma_dev->file_ion == NULL) {
		struct file* filep = filp_open(ION_DEV_NAME, O_RDWR, 0);
        if (IS_ERR(filep)) {
            printk("mma_ion_open failed, err %ld", PTR_ERR(filep));
            mma_dev->file_ion = NULL;
			goto error;
        }
		mma_dev->file_ion = filep;
	}
	return 0;

error:
	return -ENODEV;
}

int mma_ion_close(struct mma_device* mma_dev)
{
	if(!mma_dev->file_ion){
		int ret = filp_close(mma_dev->file_ion, NULL);
		if (ret < 0)
			goto error;

		mma_dev->file_ion = NULL;
	}
	return 0;

error:
	return -EBUSY;
}

static int mma_ion_ioctl(struct mma_device* mma_dev, uint req, void *arg)
{
    int ret;
    mm_segment_t old_fs;

    if(mma_dev->file_ion == NULL) {
        ret = mma_ion_open(mma_dev);
        CHECK_RETURN(ret);
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = file_ioctl(mma_dev->file_ion, req, (unsigned long )arg);
    set_fs(old_fs);
    return ret;
}

int mma_ion_alloc(struct mma_device* mma_dev, size_t size, unsigned int heap_mask,
                  int flag, int *dmabuf_fd)
{
    int ret;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)


#else
    struct ion_allocation_data alloc_data = {0};
	struct ion_fd_data fd_data = {0};
	struct ion_handle_data handle_data = {0};

	CHECK_POINTER(dmabuf_fd, -EINVAL)

	alloc_data.len = size;
    alloc_data.align = MMA_ALIGN;
    alloc_data.heap_id_mask = heap_mask;
    alloc_data.flags = flag;
    ret = mma_ion_ioctl(mma_dev, ION_IOC_ALLOC, &alloc_data);
	CHECK_RETURN(ret);


	fd_data.handle = alloc_data.handle;
    handle_data.handle = alloc_data.handle;

    ret = mma_ion_ioctl(mma_dev, ION_IOC_SHARE, &fd_data);
	CHECK_RETURN(ret);

    if (fd_data.fd < 0) {
        mma_ion_ioctl(mma_dev, ION_IOC_FREE, &handle_data);
		PRINT_AND_RETURN(-EINVAL);
    }
    *dmabuf_fd = fd_data.fd;

    ret = mma_ion_ioctl(mma_dev, ION_IOC_FREE, &handle_data); //dec reference count
	CHECK_RETURN(ret);
#endif

    return 0;
}

int mma_ion_query_heap(struct mma_device* mma_dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,12,0)
	
		
	
#endif
	return 0;
}


int  mma_ion_get_id_flag(int heap_type, int miu,int zone_flag, bool secure, int* heap_mask, int* ion_flag)
{
    int flag = 0;
    if(miu > 2)
        return -EINVAL;
    //heap_type = HEAP_TYPE_CMA;
    if(heap_type == HEAP_TYPE_IOMMU) {
        *heap_mask = 1 << ION_HEAP_TYPE_SYSTEM;//ION_HEAP_SYSTEM_MASK;
        if(miu == 0)
            flag = ION_FLAG_MIU0;
        else if(miu == 1)
            flag = ION_FLAG_MIU1;
        else
            flag = ION_FLAG_MIU2;
    } else if (heap_type == HEAP_TYPE_CMA ||heap_type == HEAP_TYPE_CMA_IOMMU) {
#ifdef CONFIG_MP_MMA_CMA_ENABLE
        if(miu == 0 && !ion_mma_cma_miu0_heap_id) {
            *heap_mask = 1<<ion_mma_cma_miu0_heap_id;
        } else if (miu == 1 && !ion_mma_cma_miu1_heap_id) {
            *heap_mask = 1<<ion_mma_cma_miu1_heap_id;
        } else if (miu == 2 && !ion_mma_cma_miu2_heap_id){
            *heap_mask = 1<<ion_mma_cma_miu2_heap_id;
        }else{
            printk("mma_ion_get_id_flag miu=%d,miu0 heap id=%d,miu1 heap id=%d,miu2 heap id=%d \n",
                miu,ion_mma_cma_miu0_heap_id,ion_mma_cma_miu1_heap_id,ion_mma_cma_miu2_heap_id);
            return -EINVAL;
        }
        flag = ION_FLAG_CONTIGUOUS;
#else
        printk("MMA CMA not support\n");
        return -EINVAL;
#endif
    }

    if(!zone_flag)
        flag |= ION_FLAG_DMAZONE;

    if(secure)
        flag |= ION_FLAG_SECURE;
    *ion_flag = flag;
	return 0;
}
