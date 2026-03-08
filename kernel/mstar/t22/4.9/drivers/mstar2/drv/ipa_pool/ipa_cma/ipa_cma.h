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
/// @file   ipa_cma.h
/// @brief  cma buffer and miu protect management
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __IPA_CMA_H__
#define __IPA_CMA_H__

#define size_1M (1<<20)

//FIXME: may be not 4 by different chip
#define MIU_BLOCK_NUM 4

#ifdef CONFIG_MSTAR_MIUSLITS
#define SLIT_NUM   (256)
#define CMA_HEAP_LENGTH_ALIGNMENT ((pageblock_nr_pages)*(PAGE_SIZE))
#endif

typedef enum
{
    MIU_BLOCK_IDLE = 0,
    MIU_BLOCK_BUSY
}MIU_PROTECT_BLOCK_STATUS;

struct cma_buffer{
    bool freed;             //freed by upper layer, but because miu hardware limitation, this buffer temply is hold by driver
    struct page * page;     //first page to buffer
    pid_t pid;              //who has this buffer

    unsigned long start_pa; //cpu bus address of this buffer
    unsigned long length;
    struct list_head list;
};

//maintain the cma buffer list, all is allocated from system(may has hole)
struct cma_alloc_list{
    unsigned long min_start;    //min of all start of cma buffers
    unsigned long max_end;      //max of all end of cma buffers
    unsigned long using_count;  //using buffer count
    unsigned long freed_count;  //freed buffer count: freed buffer must located at middle of list (hardware limitation, hold by driver temporary)
    struct list_head list_head; //all buffers in this list
};

//one miu protect entry, which protect a chunk of memory range
typedef struct
{
    unsigned char miuBlockIndex;    //index of miu protcet block
    unsigned long start_pa;         //kernel memory start cpu address
    unsigned long length;           //kernel memory length
    struct list_head list_node;
}MIU_ProtectRange;

//miu protcet blocks using status
struct MIU_ProtectRanges
{
    unsigned char miu;              //which miu
    MIU_PROTECT_BLOCK_STATUS miuBlockStatus[MIU_BLOCK_NUM]; //miu protect block status

    unsigned int krange_num;        //using count, totally 4 block
    struct list_head list_head;     //list of used
    struct mutex lock;              //different heaps may point to one miu, different processes access one struct. this for different heaps in one miu
};

//all info about cma heap & miu protect status
//it as parameters register with IPA pool
struct ipa_cma_para {
    int miu;
    char *name;
    struct device *dev;
    int ipa_id;

    struct MIU_ProtectRanges *miu_pro_sta;  //this miu kernel protect using status
    struct cma_alloc_list cma_buf_list;     //this ipa cma heap  buffer allocation status

    struct mutex ipa_cma_mutex;             //miu_pro_sta & cma_buf_list mutual access and consistency for one heap
};

typedef enum
{
    CMA_ALLOC = 0,
    CMA_FREE
}BUFF_OPS;


#define PHYSICAL_START_INIT     UL(-1)
#define PHYSICAL_END_INIT       0
#define INVALID_MIU             0xFF
#define INVALID_PID             0
#define MAX_ALLOC_TRY           30
#define MIU_PROTECT_ENABLE      1
#define MIU_PROTECT_DISABLE     0
#define IPA_CMA_KERNEL_PROT_DUMP    1
#ifdef CONFIG_MSTAR_MIUSLITS
//for miuslit,IPA POOL should 1MB alignment.
#define MIU_PROT_ALIGN_SIZE     1048576  //miuslit limitation
#else
#define MIU_PROT_ALIGN_SIZE     8192  //miu protect limitation
#endif

#endif

