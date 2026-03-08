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


#include "boot.inc"

#ifndef _ArchDefs_h_
#include "ArchDefs.h"
#endif

    .text
    .set        noreorder

#-------------------------------------------------------------------------------------------------
# BOOTRAM_Entry:
# @param  None
# @return None
# @note   Entry point of BootRam code
#-------------------------------------------------------------------------------------------------
    .globl      BOOTRAM_Entry
BOOTRAM_Entry:

    PUTC        '_'

#//================================================================================================
#// Clear data in BSS section

    la          t0, _ld_bootbss_start
    la          t1, _ld_bootbss_end

    sub         t0, 4
1:
    addi        t0, 4
    
    sw          zero, 0(t0)
    
    bltl        t0, t1, 1b
    nop

#//================================================================================================
#// Initialize system stack pointer for unzip subroutine

    li          sp, ADDR_STACK_POINTER

#//================================================================================================
#// Copy Kernel Image and jump to kernel entry

.if (SBOOT_TO_KERNEL)
    PUTK        'K'
# Load ROM_START, RAM_START, RAM_END
    li      t0,  ADDR_BIN_ROM_START
    li      t1,  ADDR_BIN_RAM_START
    li      t2,  ADDR_BIN_RAM_END

# copy kernel to ram
2:
    ld      t4, 0(t0)      #ld=lw lw to save time
    ld      t6, 8(t0)
    sd      t4, 0(t1)
    sd      t6, 8(t1)
    addu    t0, t0, 16
    addu    t1, t1, 16
    bltu    t1, t2, 2b
    nop

	jal	    BootRamFlushMemory
	nop

    la      t5,KERNEL_ENTRY
    PUTK    'E'
    PUTK    '\n'
    j       t5
    nop
.endif
#//================================================================================================
     PUTK        'U'

# Load ROM_START, RAM_START, RAM_END
    li      t0,  0x80020400
    li      t1,  CONFIG_UBOOT_LOADADDR				#// UBOOT_START ADDRESS in DRAM (0x80000000 + 0x20000 + 0x400)
    li      t2,  (CONFIG_UBOOT_LOADADDR+0x100000)


# copy uboot to ram
2:
    ld      t4, 0(t0)      #ld=lw lw to save time
    ld      t6, 8(t0)
    sd      t4, 0(t1)
    sd      t6, 8(t1)
    addu    t0, t0, 16
    addu    t1, t1, 16
    bltu    t1, t2, 2b
    nop

	jal		    BootRamFlushMemory
	nop

    la      t5,0x871F0188
    PUTK    'E'
    PUTK    '\n'
    j       t5
    nop

#//================================================================================================
BOOT_POINT:

    PUTC        'U'

    li          t0, 0xbfc20000
    lw          t1, 0x10(t0)
    nop

    jr          t1          #//Jump to The Entry Point of UBoot
    nop

#//================================================================================================
    .org        BOOT_POINT + 0x0100
    .align      4
BootRamFlushMemory:

	li		    t0, 0xa3f00000
	sw		    t0, 0(t0)
	sw		    t0, 32(t0)
	sw		    t0, 64(t0)
	sw		    t0, 96(t0)
	sw		    t0, 128(t0)

	jr		    ra
	nop

#-------------------------------------------------------------------------------------------------
# BOOTRAM_ExceptionHandler
# @param  None
# @return None
# @note   Exception handler for general exception or decompression failure at bootram code
#-------------------------------------------------------------------------------------------------
BOOTRAM_ExceptionHandler:

    la      ra, BOOT_ExceptionHandler
    j       ra
    nop

#-------------------------------------------------------------------------------------------------
# BOOTRAM_Pad
# @param  None
# @return None
# @note   Pad with zeros (Reserve SIZE_BOOTRAM_RESERVED Bytes for bootram)
#-------------------------------------------------------------------------------------------------
    .org        (SIZE_BOOTRAM_RESERVED-4)
    .word       0
