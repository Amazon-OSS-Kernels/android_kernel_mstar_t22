/*
 *
 * (C) COPYRIGHT 2014-2017 MStar Semiconductor, Inc. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */

/**
 * @file mali_kbase_mem_block_export.c
 */

#include <mali_kbase_mem_block_export.h>

#include <linux/dma-buf.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/version.h>

#include "mstar/mstar_chip.h"

#define PHYS_TO_BUS_ADDRESS_ADJUST_MIU0 MSTAR_MIU0_BUS_BASE

#if defined(MSTAR_MIU1_BUS_BASE) && defined(ARM_MIU1_BASE_ADDR)
#define MSTAR_MIU1_PHYS_BASE ARM_MIU1_BASE_ADDR
#define PHYS_TO_BUS_ADDRESS_ADJUST_MIU1 (MSTAR_MIU1_BUS_BASE - MSTAR_MIU1_PHYS_BASE)
#endif

#if defined(MSTAR_MIU2_BUS_BASE) && defined(ARM_MIU2_BASE_ADDR)
#define MSTAR_MIU2_PHYS_BASE ARM_MIU2_BASE_ADDR
#define PHYS_TO_BUS_ADDRESS_ADJUST_MIU2 (MSTAR_MIU2_BUS_BASE - MSTAR_MIU2_PHYS_BASE)
#endif

extern ulong  mtk_miu0_length;

typedef enum {
    MAP_TYPE_KMAP = 0,
    MAP_TYPE_IOREMAP
} map_type;

struct mem_block_data
{
    phys_addr_t base;
    map_type mapped;
};

static struct sg_table* mem_block_map(struct dma_buf_attachment* attach,
                                      enum dma_data_direction direction)
{
    struct mem_block_data* data = attach->dmabuf->priv;
    unsigned long pfn = PFN_DOWN(data->base);
    struct page* page = pfn_to_page(pfn);
    struct sg_table* table;
    int ret;

    table = kzalloc(sizeof(*table), GFP_KERNEL);

    if (!table)
    {
        return ERR_PTR(-ENOMEM);
    }

    ret = sg_alloc_table(table, 1, GFP_KERNEL);

    if (ret < 0)
    {
        goto err;
    }

    sg_set_page(table->sgl, page, attach->dmabuf->size, 0);
    /* XXX: in sparse memory model, it's possible that pfn_to_page(page_to_pfn(page)) != page) */
    sg_dma_address(table->sgl) = data->base;
    /* sg_dma_address(table->sgl) = sg_phys(table->sgl); */

    return table;

err:
    kfree(table);
    return ERR_PTR(ret);
}

static void mem_block_unmap(struct dma_buf_attachment* attach,
                            struct sg_table* table,
                            enum dma_data_direction direction)
{
    sg_free_table(table);
    kfree(table);
}

static void mem_block_release(struct dma_buf* buf)
{
    struct mem_block_data* data = buf->priv;
    kfree(data);
}

static void* mem_block_do_kmap(struct dma_buf* buf, unsigned long pgoffset, bool atomic)
{
    struct mem_block_data* data = buf->priv;
    unsigned long pfn = PFN_DOWN(data->base) + pgoffset;
    struct page* page = pfn_to_page(pfn);
    phys_addr_t vaddr;

    if(!pfn_valid(pfn))
    {
        unsigned long offset;
        offset = pgoffset << PAGE_SHIFT;
        vaddr = (phys_addr_t )ioremap((data->base + offset), buf->size);

        data->mapped = MAP_TYPE_IOREMAP;
        return (void *)vaddr;
    }
    else
    {
        data->mapped = MAP_TYPE_KMAP;
        if (atomic)
        {
            return kmap_atomic(page);
        }
        else
        {
            return kmap(page);
        }

    }
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void* mem_block_kmap_atomic(struct dma_buf* buf, unsigned long pgoffset)
{
    return mem_block_do_kmap(buf, pgoffset, true);
}

static void mem_block_kunmap_atomic(struct dma_buf* buf, unsigned long pgoffset, void* vaddr)
{
    struct mem_block_data* data = buf->priv;
    if (data->mapped == MAP_TYPE_KMAP)
    {
        kunmap_atomic(vaddr);
    }
    else if (data->mapped == MAP_TYPE_IOREMAP)
    {
        iounmap(vaddr);
    }
}
#endif

static void* mem_block_kmap(struct dma_buf* buf, unsigned long pgoffset)
{
    return mem_block_do_kmap(buf, pgoffset, false);
}

static void mem_block_kunmap(struct dma_buf* buf, unsigned long pgoffset, void* vaddr)
{
    struct mem_block_data* data = buf->priv;
    if (data->mapped == MAP_TYPE_KMAP)
    {
        kunmap(vaddr);
    }
    else if (data->mapped == MAP_TYPE_IOREMAP)
    {
        iounmap(vaddr);
    }

}

static int mem_block_mmap(struct dma_buf* buf, struct vm_area_struct* vma)
{
    struct mem_block_data* data = buf->priv;

    return remap_pfn_range(vma, vma->vm_start,
                           PFN_DOWN(data->base),
                           vma->vm_end - vma->vm_start,
                           vma->vm_page_prot);
}

static struct dma_buf_ops mem_block_ops =
{
    .map_dma_buf    = mem_block_map,
    .unmap_dma_buf  = mem_block_unmap,
    .release        = mem_block_release,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,0)
    .map            = mem_block_kmap,
    .unmap          = mem_block_kunmap,
#else
    .kmap_atomic    = mem_block_kmap_atomic,
    .kunmap_atomic  = mem_block_kunmap_atomic,
    .kmap           = mem_block_kmap,
    .kunmap         = mem_block_kunmap,
#endif
    .mmap           = mem_block_mmap,
};

#define MB (1024ULL * 1024ULL)
int mem_block_export_dma_buf(phys_addr_t base, u32 size, u8 is_bus_address)
{
    struct mem_block_data* data;
    struct dma_buf* buf;
    int fd = 0;
    u64 miu0_len_bytes = (u64)mtk_miu0_length * MB;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
    DEFINE_DMA_BUF_EXPORT_INFO(export_info);
#endif

    if (PAGE_ALIGN(base) != base || PAGE_ALIGN(size) != size)
    {
        return -EINVAL;
    }

    if (!is_bus_address)
    {
        if (base < miu0_len_bytes)
        {
            base += PHYS_TO_BUS_ADDRESS_ADJUST_MIU0;
        }
#if defined(MSTAR_MIU2_BUS_BASE) && defined(ARM_MIU2_BASE_ADDR)
        else if (base >= MSTAR_MIU2_PHYS_BASE)
        {
            base += PHYS_TO_BUS_ADDRESS_ADJUST_MIU2;
        }
#endif
#if defined(MSTAR_MIU1_BUS_BASE) && defined(ARM_MIU1_BASE_ADDR)
        else
        {
            base += PHYS_TO_BUS_ADDRESS_ADJUST_MIU1;
        }
#endif
    }

    data = kzalloc(sizeof(*data), GFP_KERNEL);

    if (!data)
    {
        return -ENOMEM;
    }

    data->base = base;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
    export_info.ops = &mem_block_ops;
    export_info.size = size;
    export_info.flags = O_RDWR;
    export_info.priv = data;
    buf = dma_buf_export(&export_info);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
    buf = dma_buf_export(data, &mem_block_ops, size, O_RDWR, NULL);
#else
    buf = dma_buf_export(data, &mem_block_ops, size, O_RDWR);
#endif

    if (IS_ERR(buf))
    {
        kfree(data);
        return PTR_ERR(buf);
    }

    fd = dma_buf_fd(buf, O_CLOEXEC);

    if (fd < 0)
    {
        dma_buf_put(buf);
    }

    return fd;
}
