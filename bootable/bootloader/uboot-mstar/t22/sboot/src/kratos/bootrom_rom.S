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
    .globl      BOOTROM_Entry
BOOTROM_Entry:
# Clear Timer0 for STR to prevent stuck by DELAYUS
    WREG  0xBF006048, 0xFFFF
    WREG  0xBF00604C, 0xFFFF
    WREG  0xBF006040, 0x0000
    WREG  0xBF006040, 0x0000

# Switch fast mode
    bal         CHIP_SWITCH_SPI_FAST_MODE
    nop
    
# Dummy jump
    bal         BOOTROM_JUMP_DUMMY
    nop


    DBGREG      0x0002        
    #//Configure PAD top for MUX and GPIO (e.g. DDR power)
    la           a3, MDrv_WriteRegMaskTbl
    lui          a0, %hi(padInitTbl + 10)
    ori          a0, %lo(padInitTbl + 10)
    jal          a3
    nop

    #// Enable 8051 to access MIU for MStar TV Tool
    bal         BootRomEnableAccessMiu
    nop

    DBGREG      0x0003

    #// Pre-Trimming Sequence
    bal         BootRomPreTrimSequence
    nop

    DBGREG      0x0004

    #// Power-On Sequence
    bal         BootRomPowerOnSequence
    nop

    DBGREG      0x0005

    #// Post-Trimming Sequence
    bal         BootRomPostTrimSequence
    nop

    DBGREG      0x0006

    # set stack pointer
    la          sp,  _stack_pointer     #//Set stack pointer

    DBGREG      0x0007

	la			a0, CONFIG_BOOTRAM_LOADADDR
	la			a1, 0x8000
	bal			drvNAND_Load2ndBL
    nop

	DBGREG      0x0008

	#PUTK        'M'
    #PUTK        'S'
    #PUTK        't'
    #PUTK        'a'
    #PUTK        'r'
    #PUTK        '\r'
    #PUTK        '\n'

    #//Jump to the entry of BOOTRAM
    la          ra, BOOTRAM_ROM_Entry
    nop

#-------------------------------------------------------------------------------------------------
# BOOTROM_CopyData
# @param  None
# @return None
# @note   Power On Sequence for Chip
#-------------------------------------------------------------------------------------------------
BOOTROM_CopyData:
#//-------------------------------------------------------------------------------------------------
#// Copy Data Section to DRAM
#//-------------------------------------------------------------------------------------------------
    la	        t3, _end_address_text_section
    la	        t4, _start_address_data_section
    la	        t5, _size_data_section
    beqz        t5, _CopyDataToSramEnd

_CopyDataToSram:
    lw          t6, 0(t3)
    sw          t6, 0(t4)

    add         t3, t3, 4
    add         t4, t4, 4
    add         t5, t5, -4

    bnez        t5, _CopyDataToSram
    nop

_CopyDataToSramEnd:
    jr          ra
    nop

#-------------------------------------------------------------------------------------------------
# BOOTROM_InitBss
# @param  None
# @return None
# @note   Power On Sequence for Chip
#-------------------------------------------------------------------------------------------------
BOOTROM_InitBss:
#//-------------------------------------------------------------------------------------------------
#// Clear BSS Section in DRAM
#//-------------------------------------------------------------------------------------------------
    la	        t3, _start_address_bss_section
    la	        t4, _size_bss_section
    li	        t5, 0
    beqz        t4, _InitBssLoopEnd

_InitBssLoop:
    sw          t5, 0(t3)

    add         t3, t3, 4
    add         t4, t4, -4

    bnez        t5, _InitBssLoop
    nop

_InitBssLoopEnd:
    jr          ra
    nop

#-------------------------------------------------------------------------------------------------
# BootRomPowerOnSequence
# @param  None
# @return None
# @note   Power On Sequence for Chip
#-------------------------------------------------------------------------------------------------
BootRomPowerOnSequence:

    move        k1, ra

    #//Initial releated settings of PLL
    bal         BootRomInitPll
    nop

    DBGREG      0x0030

    #//Initial related settings of Clock
    bal         BootRomInitClock
    nop

    DBGREG      0x0031

    DELAYUS     100

    #//Initial related settings of UART
    bal         BootInitUart
    nop

    DBGREG      0x0032

    #//Initial related settings of cache
    bal         BootRomInitCache
    nop

	DBGREG      0x0033

    #//Disable ADV before MIU initiation
    bal     	BootRomDisableAvd
    nop

    DBGREG      0x0034
    
    #//Initial related settings of MIU0
    bal     	BootInitMiu
    nop

    DBGREG      0x0035
    
#ifdef CONFIG_MSTAR_32BIT_RIU_MASK_ENABLE
    bal     	BootInitRiuBitMask
    nop
#endif
    bal         BootRomInitFpu
    nop


#ifndef CONFIG_MSTAR_KRATOS_BD_FPGA
    #//Initial related settings of SPI flash clock
    bal         BootRomInitSpiFlashClock
    nop

    DBGREG      0x0036

    bal         BootRomChangeSpiFlashClock
    nop

    DBGREG      0x0037
#endif

    #//Set clock ratio of OCP
    #bal     	BootRomOcpClockRatio
    #nop

    move        ra, k1
    jr          ra
    nop

#-------------------------------------------------------------------------------------------------
# BOOT_SramToTsp  Set TSP Sram to TSP
# @param  None
# @return None
# @note   do TSP Sram init
#-------------------------------------------------------------------------------------------------
BOOT_SramToTsp:

    RREG        t9, 0xBF2039C0 #//Select tsp sram to TSP
    or          t9, t9, 0x0001
    REG_C2M     t9, 0xBF2039C0

    jr          ra
    nop

#-------------------------------------------------------------------------------------------------
# BootRomInitSpiFlashClock SPI Clock init
# @param  None
# @return None
# @note   do SPI Clock init
#-------------------------------------------------------------------------------------------------
    BootRomInitSpiFlashClock:

        WREG_B      0xBF0015C4, 0xFF        #//Set CSZ deselect time to 16 SPI clock cycles
                                        #//Set CSZ setup time to 16 SPI clock cycles
        WREG_B      0xBF0015C5, 0x0F        #//Set CSZ hold time to 16 SPI clock cycles
        WREG_B      0xBF0015C8, 0x01        #//Enable fast read mode

        jr          ra
        nop

#-------------------------------------------------------------------------------------------------
# BootRomChangeSpiFlashClock
# @param  None
# @return None
# @note   change SPI clock
#-------------------------------------------------------------------------------------------------
    BootRomChangeSpiFlashClock:

        WREG_B      0xBF001C81, 0x04        #//Set clock of clk_spi to 86MHz
        WREG_B      0xBF001C81, 0x44        #//Switch clk_spi between clk_mcu_p and clk_ext_xtali_buf
        WREG_B      0xBF001C80, 0x80        #//Set clock of MCU to 170MHz

        jr          ra
        nop


#-------------------------------------------------------------------------------------------------
# BootRomOcpClockRatio
# @param  None
# @return None
# @note   do MIPS OCP clock ratio setting
#-------------------------------------------------------------------------------------------------
    BootRomOcpClockRatio:

        WREG_B      0xBF203A3C, (MIPS_OCP_CLOCK_RATIO | 0x00)   #//Set clock ratio to 2:1
        WREG_B      0xBF203A3C, (MIPS_OCP_CLOCK_RATIO | 0x01)   #//Set clock ratio change pulse

        WREG_B      0xBF201645, 0x80      #//Switch to CLK_MIPS to mipspll_vco

        jr          ra
        nop

#-------------------------------------------------------------------------------------------------
# BootRomEnableAccessMiu
# @param  None
# @return None
# @note   Enable to access MIU
#-------------------------------------------------------------------------------------------------
LEAF(BootRomEnableAccessMiu)

        WREG        0xBF005788, 0x0004      #//Set XDATA2MIU read Priority
        WREG        0xBF00578C, 0x3c11      #//Set the low bound address of MCU XDATA mapping to MIU (Unit: 1K-Byte)
        WREG        0xBF005790, 0x0010      #//Set the low byte address to access xdata from MIU. The granularity is 64k bytes
        WREG        0xBF005794, 0x403c      #//Set the low bound address of MCU XDATA mapping to MIU (Unit: 1K-Byte)
        WREG        0xBF005798, 0x0101      #//Set the low byte address to access xdata from MIU. The granularity is 4k bytes.

        jr          ra
        nop

END(BootRomEnableAccessMiu)

#-------------------------------------------------------------------------------------------------
# BootInitUart
# @param  None
# @return None
# @note   Init UART
#-------------------------------------------------------------------------------------------------
    LEAF(BootInitUart)

#ifdef CONFIG_MSTAR_KRATOS_BD_FPGA

    WREG        0xbf001c24 0x0800
    WREG        0xbf203d4c 0x2104
    WREG        0xbf201318 0x0083
    WREG        0xbf201300 0x0013
    WREG        0xbf201308 0x0000
    WREG        0xbf201310 0x0000
    WREG        0xbf201318 0x0003
    WREG        0xbf201310 0x0000
    WREG        0xbf201310 0x0007
    WREG        0xbf006010 0x0000
    WREG        0xbf006014 0x0000

    PUTK        'U'
    PUTK        'A'
    PUTK        'R'
    PUTK        'T'
    PUTK        '_'
    PUTK        '3'
    PUTK        '8'
    PUTK        '4'
    PUTK        '0'
    PUTK        '0'

    PUTK        '\n'
    PUTK        '\r'

#else

    WREG        0xbf20164c, 0x0C01      #//Set CLK_UART0 to 123Mhz

.if (BOOTINFO_UART0_ENABLE == 1)
    RREG        t3, 0xbf001c24          #//Enable UART Rx via PAD_DDCA for dig_mux
    ori         t3, t3, 0x0800
    sw          t3, 0xbf001c24

    RREG        t3, 0xbf203d4c          #//Select UART0 source to PIU UART0
    or          t3, t3, 0x0004
    sw          t3, 0xbf203d4c

.endif

.if (BOOTINFO_UART2_ENABLE == 1)
    RREG        t3, 0xbf203d4c          #//Select UART2 source to PIU UART0
    ori         t3, t3, 0x0400
    sw          t3, 0xbf203d4c

    WREG        0xbf203d50 0x0000       #//Set UART4 source to None

    RREG        t3, 0xbf203c08          #//Set third UART mode: RX2/TX2 uses PAD_UART_RX2/PAD_UART_TX2
    ori         t3, t3, 0x0800
    sw          t3, 0xbf203c08
.endif

    RREG        t3, 0xbf201318          #//Enable Divisor Latch Access
    ori         t3, t3, 0x0080
    sw          t3, 0xbf201318

    WREG        0xbf201300 UART_DIVISOR_L   #//Set Divisor Latch LSB
    WREG        0xbf201308 UART_DIVISOR_H   #//Set Divisor Latch MSB

    RREG        t3, 0xbf201318          #//Disable Divisor Latch Access
    and         t3, t3, 0xFF7F
    sw          t3, 0xbf201318

    WREG        0xbf201310 0x0000
    WREG        0xbf201310 0x0007       #//Enable FIFO and Clear TX/RX FIFO

    WREG        0xbf201318 0x0000
    WREG        0xbf201318 0x0003       #//8-bit Data / 1-bit Stop / No Parity Bit
    WREG        0xbf201320 0x0000       #//Set DTS/RTS to 1
    WREG        0xbf201308 0x0000       #//Disable Interrupt

    PUTK        'R'
    PUTK        'O'
    PUTK        'M'

    PUTK        '\n'
    PUTK        '\r'

    PUTK        'U'
    PUTK        'A'
    PUTK        'R'
    PUTK        'T'
    PUTK        '_'
    PUTK        '1'
    PUTK        '1'
    PUTK        '5'
    PUTK        '2'
    PUTK        '0'
    PUTK        '0'

    PUTK        '\n'
    PUTK        '\r'
#endif

    jr          ra
    nop

    END(BootInitUart)

#-------------------------------------------------------------------------------------------------
# BootInitRiuBitMask
# @param  None
# @return None
# @note   RIU bitmask
#-------------------------------------------------------------------------------------------------
    BootInitRiuBitMask:

        lw      t1, 0xBF20310C
        or      t1, t1, (0x01 << 5)
        sw      t1, 0xBF20310C

        lw      t1, 0xBF00025C
        or      t1, t1, (0x01 << 0)
        sw      t1, 0xBF00025C

        lw      t1, 0xBF20025C
        or      t1, t1, (0x01 << 0)
        sw      t1, 0xBF20025C

        jr      ra
        nop

    LEAF(BootRomInitFpu)

        mfc0    t1, C0_Config1
        nop
        and     t1, t1, 0x01            #//Check Status of FPU Implemented
        beq     t1, zero, BootRomInitFpuExit
        nop

        mfc0    t0, C0_Status
        nop
        or      t0, t0, (0x01 << 29)    #//Enable Co-Processor 1
        or      t0, t0, (0x01 << 26)    #//Set floating point registers can contain any datatype
        mtc0    t0, C0_Status
        nop

        move    t0, zero                #//FS = FO = FN = 0: IEEE-compliant mode
        ctc1    t0, C1_FCSR
        nop

    BootRomInitFpuExit:
        jr          ra
        nop

    END(BootRomInitFpu)

#-------------------------------------------------------------------------------------------------
# BootRomDisableAvd
# @param  None
# @return None
# @note   Disable AVD before MIU init
#-------------------------------------------------------------------------------------------------
LEAF(BootRomDisableAvd)

    RREG        t9, 0xBF206C54
    or          t9, t9, 0x0010
    REG_C2M     t9, 0xBF206C54

    jr          ra
    nop

END(BootRomDisableAvd)

#-------------------------------------------------------------------------------------------------
# CHIP_SWITCH_SPI_FAST_MODE
# @param  None
# @return None
# @note   CHIP_SWITCH_SPI_FAST_MODE
#-------------------------------------------------------------------------------------------------
LEAF(CHIP_SWITCH_SPI_FAST_MODE)

    WREG        (0xBF000000 + (0x0017E2 << 1)), 0x0FFF      # CSZ clock adjust
    WREG        (0xBF000000 + (0x0017E4 << 1)), 0x0001      # enable fast Mode
    jr          ra
    nop

END(CHIP_SWITCH_SPI_FAST_MODE)

#-------------------------------------------------------------------------------------------------
# BOOTROM_JUMP_DUMMY
# @param  None
# @return None
# @note   BOOTROM_JUMP_DUMMY
#-------------------------------------------------------------------------------------------------
LEAF(BOOTROM_JUMP_DUMMY)
    nop
    jr          ra
    nop

END(BOOTROM_JUMP_DUMMY)

