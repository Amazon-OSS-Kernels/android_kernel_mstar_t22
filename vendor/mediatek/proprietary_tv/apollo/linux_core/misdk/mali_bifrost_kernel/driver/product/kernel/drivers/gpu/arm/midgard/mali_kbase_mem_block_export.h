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
 * @file mali_kbase_mem_block_export.h
 */

#ifndef _KBASE_MEM_BLOCK_EXPORT_H_
#define _KBASE_MEM_BLOCK_EXPORT_H_

#include <linux/types.h>

/**
 * @brief Exmport a memory block as a dma-buf
 *
 * the base address and size of the memory block must be page-aligned.
 *
 * @param[in] base              the base address of the memory block
 * @param[in] size              the size of the memory block
 * @param[in] is_bus_address    is the address a bus address
 *
 * @return file descriptor on success of -errno on failure
 */
int mem_block_export_dma_buf(phys_addr_t base, u32 size, u8 is_bus_address);

#endif /* _KBASE_MEM_BLOCK_EXPORT_H_ */
