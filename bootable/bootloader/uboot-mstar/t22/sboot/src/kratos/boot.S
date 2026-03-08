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
# BOOT_Entry:
# @param  None
# @return None
# @note   Entry point of Boot code
#-------------------------------------------------------------------------------------------------
    .globl      BOOT_Entry

BOOT_Entry:
    #// for NAND programming tool bin usage, overwirte the chip config to (1) boot from NAND 3'b011 (2) boot from SPI-NAND 3'b010
#if (ENABLE_MSTAR_TOOL_ROM_PROGRAM_NAND_BIN==1)
    #if (ENABLE_NAND_FLASH==1)
    WREG 0xBF001C7C, 0x007F
    #elif (ENABLE_SPI_NAND_FLASH==1)
    WREG 0xBF001C7C, 0x006F
    #endif
#endif

    #// 8051 instruction: jump 0xb00
    #.BYTE       0x02, 0x0b, 0x00, 0x00

	#//disable vdd2low_rst function  and change chip_top_password to non-default value
	WREG_B      0xBF001CA4, 0x3F

    # initalize the PIU timer1 to measure boot time
    WREG  0xBF006088, 0xFFFF
    WREG  0xBF00608C, 0xFFFF
    WREG  0xBF006080, 0x0000
    WREG  0xBF006080, 0x0001
    #clean PIU timer0
    WREG  0xBF006048, 0x0000
    WREG  0xBF00604C, 0x0000
    WREG  0xBF006040, 0x0000
    WREG  0xBF006040, 0x0000

ROM_Start:

    #//Clear status in cause register
    li          t1, M_CauseIV
    mtc0        t1, C0_Cause
    nop

    #//Set WDT period maximum value to 0 to disable watchdog timer
    sw          zero, (0xbf000000 + (0x3008 << 1))
    sw          zero, (0xbf000000 + (0x300A << 1))

    #//Initialize CPU Registers
    bal         BootInitCpuRegister
    nop

    DBGREG      0x0001

    #//Jump to Entry of BOOTROM
    la          ra, BOOTROM_Entry
    jr          ra
    nop

#-------------------------------------------------------------------------------------------------
# BootInitCpuRegister
# @param  None
# @return None
# @note   Init CPU Registers
#-------------------------------------------------------------------------------------------------
BootInitCpuRegister:

	#//Initial setting in watch register
	li          t0, 0x07
	mtc0        t0, C0_WatchLo
    nop

	mtc0	    zero, C0_WatchHi
	nop

	#//Disable interrupts and KSU field (power up issue)
	mfc0        t0, C0_Status
	nop

    #//Check status of Non-Maskable Interrupt
    and         t1, t0, M_StatusNMI
    bnez        t1, 1f
    nop

    #//Disable interrupt and set base mode to kernel mode
	and	        t1, t0, ~(M_StatusIE | M_StatusKSU)
	mtc0	    t1, C0_Status
	nop

#if 0
	#//Don't touch cause if we got here from an NMI
	mfc0        t0, C0_Status
	nop

    and         t1, t0, M_StatusNMI
    bnez        t1, 1f
    nop
#endif

	#//Clear status in cause register
	#//IV = 1: Use the special interrupt vector
	li	        t1, M_CauseIV
	mtc0	    t1, C0_Cause

1:
	nop

#//================================================================================================
    mtc0        zero, C0_Count
    nop

    mtc0        zero, C0_Compare
    nop

#//================================================================================================

	#//Set attribute in config register
	mfc0        t0, C0_Config
	nop

	and	        t0, t0, CONFIG0_MIPS32_64_MSK
	or          t0, t0, ((K_CacheAttrCN << S_ConfigK23) | (K_CacheAttrCN << S_ConfigKU) | (M_ConfigMM))

    #//Set merging allowed
    #//Set the cacheability of kuseg, usegm kseg2 and kseg3 to "Cacheable, noncoherent, write-back, write allocate"
	mtc0        t0, C0_Config
	nop

    #//Enable Externalize Sync
    mfc0        t0, C0_Config, 7
    nop

    or          t0, t0, 0x0100

    mtc0        t0, C0_Config, 7
    nop

#//================================================================================================

    #//Set Status Register
	mfc0        t0, C0_Status
	nop

	and         t0, t0, STATUS_MIPS32_64_MSK
	or          t0, t0, STATUS_MIPS34K

	mtc0	    t0, C0_Status                   #//Disable CorExtend block?
	nop

#//================================================================================================
    jr          ra
    nop

#-------------------------------------------------------------------------------------------------
# BOOT_ExceptionHandler
# @param  None
# @return None
# @note   Exception handler for general exception or decompression failure at boot code
#-------------------------------------------------------------------------------------------------

    #//0xBFC00380 if StatusBEV=1 StatusEXL=0 CauseIV=0
    .org        0x0380
    .globl      BOOT_ExceptionHandler
BOOT_ExceptionHandler:

    .set        noat

.if (SUPPORT_POWER_SAVING)

    #//Check status of power saving in standby mode
    #//Related flag was definied in power saving driver
    lw          t1, REG_TOP_POWER_STATUS    #//I2S_GPIO_OEN[11:0]
    nop

    li          t0, 0x03
    and         t1, t1, t0
    bne         t1, t0, _ExceptionHandlerReset
    nop

    #//Set the location of exception vectors to Normal
    mfc0        t0, C0_Status
    nop

    li          t1, ~M_StatusBEV
    and         t0, t0, t1

    mtc0        t0, C0_Status
    nop

    eret
    nop

_ExceptionHandlerReset:

.endif  #//SUPPORT_POWER_SAVING

.if !(BOOT_DEBUG_VERSION)

    la          t0, 0xBF001C00

    li          t1, 0xB8D3
    sw          t1, 0xB0(t0)                #//Set password of MIPS to 0xB8D3

    lw          t1, 0xA4(t0)
    nop

    and         t1, t1, 0xFBFF              #//Trigger MIPS Software Reset (Active High)
    sw          t1, 0xA4(t0)

    or          t1, t1, 0x0400              #//Trigger MIPS Software Reset (Active High)
    sw          t1, 0xA4(t0)

.endif  #//BOOT_DEBUG_VERSION

_ExceptionHandlerLoop:
    j           _ExceptionHandlerLoop
    nop

    .set        at


.if (SUPPORT_POWER_SAVING)
#-------------------------------------------------------------------------------------------------
# BOOT_CPU_DRAM_Standby
# @param  None
# @return None
# @note   Let DRAM & CPU enter standby mode
#-------------------------------------------------------------------------------------------------
    .org        0xa80      # Cannot change it, _MDrv_Power_EnterPowerSavingMode will call it.
BOOT_CPU_DRAM_Standby:

    #//Enable Interrupt
    mfc0        t0, C0_Status
    nop

    or          t0, t0, M_StatusIE

    mtc0        t0, C0_Status
    nop

    #//Enter MIPS power-down state
    wait
    nop

    jr          ra
    nop

    .set        at

.endif  #//SUPPORT_POWER_SAVING


#-------------------------------------------------------------------------------------------------
# BOOT_Pad
# @param  None
# @return None
# @note   Pad with zeros (Reserve SIZE_BOOT_RESERVED Bytes for boot)
#-------------------------------------------------------------------------------------------------
    .org        (SIZE_BOOT_RESERVED - 4)
    .word       0
