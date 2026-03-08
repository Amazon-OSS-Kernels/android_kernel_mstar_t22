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
//#include <stdio.h>
//#include <assert.h>
//#include "datatype.h"
//#include "mhal_arch.h"
//###################################################

#if defined (__arm__) || defined (__aarch64__)

#include <string.h>
#define NONCACHABLE(_x_)    ((_x_) | 0x00000000)

#define RIUBASE   0x1F000000
#define PERIBASE  0x16000000

  #if (ENABLE_MSTAR_MACAN)
    #define MIPS_CLOCK_FREQ     1008000000
    #define MIU0_BUS_BASE       0x20000000
    #define MIU1_BUS_BASE       0xA0000000
    #define MIU0_MAP_LEN        0x20000000
    #define MIU1_MAP_LEN        0x40000000
    #define MIU1_ENABLE         1

  #elif (ENABLE_MSTAR_M3822)
    #define MIPS_CLOCK_FREQ     1008000000
    #define MIU0_BUS_BASE       0x20000000
    #define MIU0_MAP_LEN        0x10000000
    #define MIU1_ENABLE         0
  #endif

#define CREATE_MASK(msb, lsb)               (((1U << ((msb) - (lsb) + 1)) - 1) << (lsb))
#define MASK_BITS(arg, msb, lsb)            ((arg) & CREATE_MASK(msb, lsb))
#define EXTRACT_BITS(arg, msb, lsb)         (MASK_BITS(arg, msb, lsb) >> (lsb))
#define INSERT_BITS(arg, msb, lsb, value) \
    ((arg) = ((arg) & ~CREATE_MASK(msb, lsb)) | (((value) << (lsb)) & CREATE_MASK(msb, lsb)))

#define MASK_FIELD(arg, field)              MASK_BITS(arg, field##_MSB, field##_LSB)
#define EXTRACT_FIELD(arg, field)           EXTRACT_BITS(arg, field##_MSB, field##_LSB)
#define INSERT_FIELD(arg, field, value)     INSERT_BITS(arg, field##_MSB, field##_LSB, value)

#define SET_BIT(arg, bit)                   ((arg) |= (1 << (bit)))
#define CLEAR_BIT(arg, bit)                 ((arg) &= ~(1 << (bit)))
#define TEST_BIT(arg, bit)                  ((arg) & (1 << (bit)))

typedef struct
{
    unsigned int va;
    unsigned int pa;
    unsigned int size;
    int         access;
    int         attributes;
}
_cpu_memory_t;

#define MMU_AP_NONE         0
#define MMU_AP_PRIVILEGED   1
#define MMU_AP_READ         2
#define MMU_AP_FULL         3

#define MMU_BUFFERED        1
#define MMU_CACHED          2

//extern void v7_dma_flush_range(U32 u32Start, U32 u32End);

void _cpu_mmu_map_memory (_cpu_memory_t *entry);
void mhal_dcache_enable(void);
void mhal_dcache_disable(void);
void mhal_L2Cache_Flush(void);
void mhal_L2Cache_Read(void);
void mhal_init_mmu(void);

void mhal_dcache_enable(void)
{
    unsigned long control;

    /* Enable data cache */
    asm volatile ("mrc p15, 0, %0, c1, c0, 0\n"
                  "orr %0, %0, #0x04\n"
                  "mcr p15, 0, %0, c1, c0, 0\n"
                  : "=r" (control)
                  :
                  : "memory");
}

void mhal_dcache_disable(void)
{
    unsigned long control;

    /* Disable data cache */
    asm volatile ("mrc p15, 0, %0, c1, c0, 0 \n"
                  "bic %0, %0, #0x04 \n"
                  "mcr p15, 0, %0, c1, c0, 0 \n"
                  : "=r" (control)
                  :
                  : "memory");
}

/* MIU pipe flush */
#if 0
extern void HAL_MMIO_FlushMemory(void);

void mhal_dcache_flush(U32 u32Base, U32 u32Size)
{
    v7_dma_flush_range(u32Base, u32Base + u32Size);

    HAL_MMIO_FlushMemory();
}
#endif

#define LEVEL_1_ADDRESS(va,ttb) (MASK_BITS(ttb, 31, 14) | (EXTRACT_BITS(va, 31, 20) << 2))


/*! \brief  Reads the translation table base address
 *
 *  \return Translation table base address
 *
 */
static unsigned int __cpu_mmu_ttb_read (void)
{
    unsigned int ttb;

    asm volatile ("mrc p15, 0, %0, c2, c0, 0"
                   : "=r" (ttb)
                   :
                   : "memory");

    return ttb;
}

static unsigned int __cpu_mmu_ttb_write (unsigned int ttbr0)
{
    asm volatile ("mcr  p15, 0, %[ttbr0], c2, c0, 0"
            : : [ttbr0] "r" (ttbr0)
    );

    return ttbr0;
}

/*! \brief  Writes the domain register
 *
 *  \param  domains The domain access control register data
 *
 */
static void __cpu_mmu_domains_write (unsigned int domains)
{
    asm volatile ("mcr p15, 0, %0, c3, c0, 0"
                   :
                   : "r" (domains)
                   : "memory");
}
/*! \brief  Creates a section translation table entry.
 *
 * Taking the virtual section address, physical address, access permissions and attributes
 * a level 1 section entry is defined in the translation table.
 *
 *  \param  va          Virtual Address
 *  \param  pa          Physical Address
 *  \param  access      Access Permissions
 *  \param  attributes  Cacheable / Bufferable
 *
 */
static void __cpu_mmu_create_section (unsigned int va, unsigned int pa, int access, int attributes)
{
    unsigned int ttb;
    unsigned int *ptr;

    ttb  = __cpu_mmu_ttb_read ();
    ptr  = (unsigned int *)LEVEL_1_ADDRESS(va,ttb);
    *ptr = pa | (access << 10) | (attributes << 2) | 2;
}

/*
** TO BE IMPLEMENTED
*/
/*
static void __cpu_mmu_create_large_page (unsigned int va, unsigned int pa, int access, int attributes)
{
    va = va;
    pa = pa;
    access = access;
    attributes = attributes;
}
*/
/*
** TO BE IMPLEMENTED
*/
/*
static void __cpu_mmu_create_small_page (unsigned int va, unsigned int pa, int access, int attributes)
{
    va = va;
    pa = pa;
    access = access;
    attributes = attributes;
}
*/
/*! \brief  Creates a MMU table entry
 *
 * Takes a pointer to a memory descriptor (size, virtual address, physical address,
 * access permissions and attributes) and creates the required translation table
 * entries.
 *
 *  \param  entry   Pointer to the table entry
 *
 */
void _cpu_mmu_map_memory (_cpu_memory_t *entry)
{
    unsigned int size;
    unsigned int va;
    unsigned int pa;
    int access;
    int attributes;

    size        = entry->size;
    va          = entry->va;
    pa          = entry->pa;
    access      = entry->access;
    attributes  = entry->attributes;

#if 1
    /*
    ** Round up to MB so that we only use sections
    */
    size += 0x000FFFFF;
    size &= ~0x000FFFFF;
#endif

    while (size > 0)
    {   // Generate the page tables
        int temp = 0;

        __cpu_mmu_create_section (va, pa, access, attributes);
        temp = 0x00100000;

        va += temp;
        pa += temp;
        size -= temp;

    }
}

/*! \brief  Initializes the MMU
 *
 * Initializes the MMU, generating a background 4GB flat mapped full access,
 * non cacheable non bufferable page table then overlay that with cached and
 * buffered SDRAM entries.
 *
 */

//#####################################
// Set 16K alignment for CPU HW limitation
//#####################################
unsigned int MMUBaseTable[0x4000] __attribute__ ((aligned (16 * 1024)));

void mhal_init_mmu(void)
{
    _cpu_memory_t entry;

    //#####################################
    // Set MMU table base first for following MMU mapping
    //#####################################
    memset((void*)MMUBaseTable, 0, 0x4000);
    __cpu_mmu_ttb_write((unsigned int) (MMUBaseTable)|(unsigned int) 0x6a);


    entry.pa            = 0x1F000000;
    entry.va            = 0x1F000000;
    entry.size          = 0x01000000; // RIU(16M)
    entry.access        = MMU_AP_FULL;
    entry.attributes    = 0;

    _cpu_mmu_map_memory (&entry);

    entry.pa            = 0x15000000;
    entry.va            = 0x15000000;
    entry.size          = 0x00100000; // ARM RIU(16M)
    entry.access        = MMU_AP_FULL;
    entry.attributes    = 0;

    _cpu_mmu_map_memory (&entry);

    entry.pa            = 0x16000000;
    entry.va            = 0x16000000;
    entry.size          = 0x00100000; // ARM RIU(16M)
    entry.access        = MMU_AP_FULL;
    entry.attributes    = 0;

    _cpu_mmu_map_memory (&entry);

    entry.pa            = 0x14000000;
    entry.va            = 0x14000000;
    entry.size          = 0x01000000; // XIU SPI flash(16M)
    entry.access        = MMU_AP_FULL;
    entry.attributes    = MMU_CACHED | MMU_BUFFERED;

    _cpu_mmu_map_memory (&entry);

    entry.pa            = MIU0_BUS_BASE;
    entry.va            = MIU0_BUS_BASE;
    entry.size          = MIU0_MAP_LEN; // MIU0 - cached
    entry.access        = MMU_AP_FULL;
    entry.attributes    = MMU_CACHED | MMU_BUFFERED;

    _cpu_mmu_map_memory (&entry);

    entry.pa            = MIU0_BUS_BASE;
    entry.va            = MIU0_BUS_BASE+0x20000000;
    entry.size          = MIU0_MAP_LEN; // MIU0 - uncached
    entry.access        = MMU_AP_FULL;
    entry.attributes    = MMU_BUFFERED;

    _cpu_mmu_map_memory (&entry);

#if( MIU1_ENABLE ) // MIU1
    entry.pa            = MIU1_BUS_BASE;
    entry.va            = MIU1_BUS_BASE;
    entry.size          = 0x20000000; // MIU1 - cached
    entry.access        = MMU_AP_FULL;
    entry.attributes    = MMU_CACHED | MMU_BUFFERED;

    _cpu_mmu_map_memory (&entry);

    entry.pa            = MIU1_BUS_BASE;
    entry.va            = MIU1_BUS_BASE + 0x20000000;
    entry.size          = MIU1_MAP_LEN; // MIU1 - uncached
    entry.access        = MMU_AP_FULL;
    entry.attributes    = MMU_BUFFERED;

    _cpu_mmu_map_memory (&entry);
#endif

#if 1
    entry.pa            = 0x0;
    entry.va            = 0x0;
    entry.size          = 0x00800000;
    entry.access        = MMU_AP_FULL;
    entry.attributes    = MMU_CACHED | MMU_BUFFERED;

    _cpu_mmu_map_memory (&entry);
#endif

    /*
    ** We are only using domain 0.
    */
    __cpu_mmu_domains_write (1);
}

void mhal_L2Cache_Flush(void)
{
}

void mhal_L2Cache_Read(void)
{
}
#endif //defined (__arm__) || defined (__aarch64__)

