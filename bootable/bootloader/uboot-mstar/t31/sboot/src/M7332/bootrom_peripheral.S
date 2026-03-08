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


#ifndef _BOOT_INC_
#include "boot.inc"
#endif

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

    .text

    .global     BootRom_InitPeripheral
BootRom_InitPeripheral:

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && defined(CONFIG_MBOOT_IN_SPI_FLASH)
    mov         r8, lr
#else
    push        {lr}
#endif

//================================================================================================
    //Initial related settings of UART
    bl          BootRom_InitUart

//================================================================================================
    //Initial related settings of RIU
    bl          BootRom_InitRiu

#ifdef CONFIG_MSTAR_EFUSE_ENABLE
    //Initial Trimming Setting in eFuse
    bl          BootRom_InitEfuse
#endif

//================================================================================================
    //ADC PLL settng
    bl          BootRom_ADCPLL_set

#if !defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH) && \
    !defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)

    //checkpoint for autotest boottime, plz dont remove it
    DPUTK       '['
    DPUTK       'A'
    DPUTK       'T'
    DPUTK       ']'
    DPUTK       '['
    DPUTK       'M'
    DPUTK       'B'
    DPUTK       ']'
    DPUTK       '['
    DPUTK       's'
    DPUTK       't'
    DPUTK       'a'
    DPUTK       'r'
    DPUTK       't'
    DPUTK       ' '
    DPUTK       's'
    DPUTK       'b'
    DPUTK       ']'
    DPUTK       '['
    DPUTK       '0'
    DPUTK       ']'

#endif

#ifdef REG_USB30_GPIO_CONTROL
    bl          BootRom_InitUsb30Gpio
#endif

#if (defined ENABLE_FCIE_GPIO_CONTROL) && ENABLE_FCIE_GPIO_CONTROL
    bl          BootRom_InitFCIEGpio
#endif
#if (defined ENABLE_SDIO_GPIO_CONTROL) && ENABLE_SDIO_GPIO_CONTROL
    bl          BootRom_InitSDIOGpio
#endif

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && defined(CONFIG_MBOOT_IN_SPI_FLASH)
    mov         lr, r8
#else
    pop         {lr}
#endif
    bx          lr

//-------------------------------------------------------------------------------------------------
// BootRom_InitRiu
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitRiu
BootRom_InitRiu:

    //Enable 32-bit RIU Write Data
    RREG        r3, (MS_BUS_RIU_ADDR + (0x101882 << 1))
    ldr         r4, =0x00008000
    bic         r3, r3, r4
    REG_C2M     r3, (MS_BUS_RIU_ADDR + (0x101882 << 1))

#ifdef CONFIG_MSTAR_32BIT_RIU_MASK_ENABLE
    RREG        r3, (MS_BUS_RIU_ADDR + (0x10012E << 1))
    ldr         r4, =0x00000001
    orr         r3, r3, r4
    REG_C2M     r3, (MS_BUS_RIU_ADDR + (0x10012E << 1))

    RREG        r3, (MS_BUS_RIU_ADDR + (0x101882 << 1))
    ldr         r4, =0x00008000
    orr         r3, r3, r4
    REG_C2M     r3, (MS_BUS_RIU_ADDR + (0x101882 << 1))

    RREG        r3, (MS_BUS_RIU_ADDR + (0x00012E << 1))
    ldr         r4, =0x00000001
    orr         r3, r3, r4
    REG_C2M     r3, (MS_BUS_RIU_ADDR + (0x00012E << 1))
#endif

    bx          lr

//-------------------------------------------------------------------------------------------------
// BootRom_ADCPLL_set
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_ADCPLL_set
BootRom_ADCPLL_set:

    //Get Trim Value of Video_atop(ADC) in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0148
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2148
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_VideoAtop_ADC_Loop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_VideoAtop_ADC_Loop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //Set ADCPLL setting by efuse value
    and         r4, r0, #(0x03 << 1)
    lsr         r4, r4, #1

    cmp         r4, #1
    beq         BootRom_ReadEfuse_VideoAtop_ADC_01_Set

    cmp         r4, #2
    beq         BootRom_ReadEfuse_VideoAtop_ADC_10_Set

    b           BootRom_ReadEfuse_VideoAtop_ADC_Exit

BootRom_ReadEfuse_VideoAtop_ADC_01_Set:

    // Set BK1026_70h[2:1] = 2'b01
    ldr         r0, =(MS_BUS_RIU_ADDR + (0x102600 << 1))
    ldr         r1, [r0, #(0xE0<< 1)]
    ldr         r2, =0x0002
    orr         r1, r1, r2
    str         r1, [r0, #(0xE0 << 1)]
    b           BootRom_ReadEfuse_VideoAtop_ADC_Exit

BootRom_ReadEfuse_VideoAtop_ADC_10_Set:

    // Set BK1026_70h[2:1] = 2'b11
    ldr         r0, =(MS_BUS_RIU_ADDR + (0x102600 << 1))
    ldr         r1, [r0, #(0xE0<< 1)]
    ldr         r2, =0x0006
    orr         r1, r1, r2
    str         r1, [r0, #(0xE0 << 1)]

    //Set BK1128_4Fh[7] = 1'b1
    ldr         r0, =(MS_BUS_RIU_ADDR + (0x112800 << 1))
    ldr         r1, [r0, #(0x9E<< 1)]
    ldr         r2, =0x0080
    orr         r1, r1, r2
    str         r1, [r0, #(0x9E << 1)]

BootRom_ReadEfuse_VideoAtop_ADC_Exit:

    bx          lr

//-------------------------------------------------------------------------------------------------
// BootRom_InitUart
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitUart
BootRom_InitUart:

#ifdef CONFIG_MSTAR_TITANIA_BD_FPGA
    WREG        (MS_BUS_RIU_ADDR + (0x000E12 << 1)), 0x0800
    WREG        (MS_BUS_RIU_ADDR + (0x101EA6 << 1)), 0x2104
    WREG        (MS_BUS_RIU_ADDR + (0x10098C << 1)), 0x0083
    WREG        (MS_BUS_RIU_ADDR + (0x100980 << 1)), 0x0013
    WREG        (MS_BUS_RIU_ADDR + (0x100984 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x100988 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x10098C << 1)), 0x0003
    WREG        (MS_BUS_RIU_ADDR + (0x100988 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x100988 << 1)), 0x0007
    WREG        (MS_BUS_RIU_ADDR + (0x003008 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x00300A << 1)), 0x0000

    DPUTK       'U'
    DPUTK       'A'
    DPUTK       'R'
    DPUTK       'T'
    DPUTK       '_'
    DPUTK       '3'
    DPUTK       '8'
    DPUTK       '4'
    DPUTK       '0'
    DPUTK       '0'

    DPUTK       '\n'
    DPUTK       '\r'

#else
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100b00 << 1))
    ldr         r0, =0x0c01
    str         r0, [r1, #(0x26 << 1)]                      //Set CLK_UART0 to 123Mhz

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r0, =0x0000
    str         r0, [r1, #(0xa6 << 1)]
    ldr         r0, =0x0100
    str         r0, [r1, #(0xa8 << 1)]

.if (BOOTINFO_UART0_ENABLE == 1)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000e00 << 1))
    ldr         r0, [r1, #(0x12 << 1)]

    bic         r0, r0, #0x1000                             //switch UART to ARM
    mov         r2, r0                                      //Backup rx en/dis setting in r2
    bic         r0, r0, #0x0800                             //Keep Rx disable during UART0 init
    str         r0, [r1, #(0x12 << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r0, [r1, #(0xa6 << 1)]
    orr         r0, r0, #0x0004
    str         r0, [r1, #(0xa6 << 1)]                      //Select UART0 source to PIU UART0

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100900 << 1))
    ldr         r0, [r1, #(0x8c << 1)]
    orr         r0, r0, #0x0080
    str         r0, [r1, #(0x8c << 1)]                      //Enable Divisor Latch Access

    ldr         r0, =UART_DIVISOR_L
    str         r0, [r1, #(0x80 << 1)]                      //Set Divisor Latch LSB

    ldr         r0, =UART_DIVISOR_H
    str         r0, [r1, #(0x84 << 1)]                      //Set Divisor Latch MSB

    ldr         r0, [r1, #(0x8c << 1)]
    bic         r0, r0, #0x0080
    str         r0, [r1, #(0x8c << 1)]                      //Disable Divisor Latch Access

    ldr         r0, =0x0000
    str         r0, [r1, #(0x88 << 1)]
    ldr         r0, =0x0007
    str         r0, [r1, #(0x88 << 1)]                      //Enable FIFO and Clear TX/RX FIFO

    ldr         r0, =0x0000
    str         r0, [r1, #(0x8c << 1)]
    ldr         r0, =0x0003
    str         r0, [r1, #(0x8c << 1)]                      //8-bit Data / 1-bit Stop / No Parity Bit

    ldr         r0, =0x0000
    str         r0, [r1, #(0x90 << 1)]                      //Set DTS/RTS to 1

    ldr         r0, =0x0000
    str         r0, [r1, #(0x84 << 1)]                      //Disable Interrupt

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000e00 << 1))
    str         r2, [r1, #(0x12 << 1)]                      //Write back rx en/dis setting from r2
.endif

.if (BOOTINFO_UART1_ENABLE == 1)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100b00 << 1))
    ldr         r0, =0x0014                                 //0x000C -> 0x0014
    str         r0, [r1, #(0x28 << 1)]                      //Set CLK_UART1 to 24Mhz

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r0, [r1, #(0xa6 << 1)]
    orr         r0, r0, #0x0050
    str         r0, [r1, #(0xa6 << 1)]                      //Select UART1 source to PIU UART1

    ldr         r0, [r1, #(0x04 << 1)]
    orr         r0, r0, #0x0300
    str         r0, [r1, #(0x04 << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110600 << 1))
    ldr         r0, [r1, #(0x0c << 1)]
    orr         r0, r0, #0x0080
    str         r0, [r1, #(0x0c << 1)]                      //Enable Divisor Latch Access

    ldr         r0, =EX_UART_DIVISOR_L
    str         r0, [r1, #(0x00 << 1)]                      //Set Divisor Latch LSB

    ldr         r0, =EX_UART_DIVISOR_H
    str         r0, [r1, #(0x04 << 1)]                      //Set Divisor Latch MSB

    ldr         r0, [r1, #(0x0c << 1)]
    bic         r0, r0, #0x0080
    str         r0, [r1, #(0x0c << 1)]                      //Disable Divisor Latch Access

    ldr         r0, =0x0000
    str         r0, [r1, #(0x08 << 1)]
    ldr         r0, =0x0007
    str         r0, [r1, #(0x08 << 1)]                      //Enable FIFO and Clear TX/RX FIFO

    ldr         r0, =0x0000
    str         r0, [r1, #(0x0c << 1)]
    ldr         r0, =0x0003
    str         r0, [r1, #(0x0c << 1)]                      //8-bit Data / 1-bit Stop / No Parity Bit

    ldr         r0, =0x0000
    str         r0, [r1, #(0x10 << 1)]                      //Set DTS/RTS to 1

    ldr         r0, =0x0000
    str         r0, [r1, #(0x04 << 1)]                      //Disable Interrupt
.endif

.if (BOOTINFO_UART2_ENABLE == 1)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r0, [r1, #(0xa6 << 1)]
    orr         r0, r0, #0x0500
    str         r0, [r1, #(0xa6 << 1)]                      //Select UART2 source to PIU UART1
.endif

    DPUTK       'U'
    DPUTK       'A'
    DPUTK       'R'
    DPUTK       'T'
    DPUTK       '_'
    DPUTK       '1'
    DPUTK       '1'
    DPUTK       '5'
    DPUTK       '2'
    DPUTK       '0'
    DPUTK       '0'

    DPUTK       '\n'
    DPUTK       '\r'

#ifdef CONFIG_FAST_UART_RTSCTS
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100b00 << 1))    //Set CLK_FUART to 123Mhz
    ldr         r0, =0x000c
    str         r0, [r1, #(0x2a << 1)]

    RREG        r0, (MS_BUS_RIU_ADDR + (0x11068C << 1))     //Enable Divisor Latch Access
    orr         r0, r0, #0x0080
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x11068C << 1))

    WREG        (MS_BUS_RIU_ADDR + (0x110680 << 1)), UART_DIVISOR_L   //Set Divisor Latch LSB
    WREG        (MS_BUS_RIU_ADDR + (0x110684 << 1)), UART_DIVISOR_H   //Set Divisor Latch MSB

    RREG        r0, (MS_BUS_RIU_ADDR + (0x11068C << 1))     //Disable Divisor Latch Access
    ldr         r1, =0xFF7F
    and         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x11068C << 1))

    WREG        (MS_BUS_RIU_ADDR + (0x110688 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110688 << 1)), 0x0007 //Enable FIFO and Clear TX/RX FIFO

    WREG        (MS_BUS_RIU_ADDR + (0x11068C << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x11068C << 1)), 0x0003 //8-bit Data / 1-bit Stop / No Parity Bit
    WREG        (MS_BUS_RIU_ADDR + (0x110690 << 1)), 0x0000 //Set DTS/RTS to 1
    WREG        (MS_BUS_RIU_ADDR + (0x110684 << 1)), 0x0000 //Disable Interrupt
#endif

#endif

    bx          lr

//-------------------------------------------------------------------------------------------------
// BootRom_EnableMcuAccess
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_EnableMcuAccess
BootRom_EnableMcuAccess:

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002b00 << 1))
    ldr         r0, =0x0004
    str         r0, [r1, #(0xc4 << 1)]
    ldr         r0, =0x3c11
    str         r0, [r1, #(0xc6 << 1)]
    ldr         r0, =0x0010
    str         r0, [r1, #(0xc8 << 1)]
    ldr         r0, =0x403c
    str         r0, [r1, #(0xca << 1)]
    ldr         r0, =0x0101
    str         r0, [r1, #(0xcc << 1)]

    bx          lr

#ifdef REG_USB30_GPIO_CONTROL
//-------------------------------------------------------------------------------------------------
// BootRom_InitUsb30Gpio
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
BootRom_InitUsb30Gpio:

    bx          lr
#endif


// different project might has different fcie/sdio bank address
#define RIU_BANK_FCIE0      0x1113
#define RIU_BANK_SDIO0      0x120F

#if (defined ENABLE_FCIE_GPIO_CONTROL) && ENABLE_FCIE_GPIO_CONTROL
    BootRom_InitFCIEGpio:
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x14<<2) ), SD_SIGNATURE
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x15<<2) ), FCIE_OPT
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x0C<<2) ), 0x0080 // enable reg_chk_cmd
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x23<<2) ), FCIE_CD_BANK
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x24<<2) ), ((FCIE_CD_OFFSET<<8)|(FCIE_CD_BIT&0x07)|((FCIE_PAD_CONFIG<<3)&0x78))
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x25<<2) ), FCIE_PC_BANK
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x26<<2) ), ((FCIE_PC_OFFSET<<8)|(FCIE_PC_BIT&0x07))
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x27<<2) ), FCIE_WP_BANK
        WREG_B  ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x28<<2) ), (FCIE_WP_BIT&0x07)
        WREG_B  ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x0E<<2) + 1), FCIE_WP_OFFSET
        WREG    ( MS_BUS_RIU_ADDR + (RIU_BANK_FCIE0<<9) + (0x0C<<2) ), 0x0000 // disable reg_chk_cmd

        bx      lr
#endif

#if (defined ENABLE_SDIO_GPIO_CONTROL) && ENABLE_SDIO_GPIO_CONTROL
    BootRom_InitSDIOGpio:
   
		WREG	(MS_BUS_RIU_ADDR + ((0x120F28)<<1)),SD_SIGNATURE
		WREG	(MS_BUS_RIU_ADDR + ((0x120F2A)<<1)),SDIO_OPT
		WREG	(MS_BUS_RIU_ADDR + ((0x120F18)<<1)),0x0080 // enable reg_chk_cmd
		WREG	(MS_BUS_RIU_ADDR + ((0x120F46)<<1)),SDIO_CD_BANK
		WREG	(MS_BUS_RIU_ADDR + ((0x120F48)<<1)),((SDIO_CD_OFFSET<<8)|(SDIO_CD_BIT&0xFF))
		WREG	(MS_BUS_RIU_ADDR + ((0x120F4A)<<1)),SDIO_PC_BANK
		WREG	(MS_BUS_RIU_ADDR + ((0x120F4C)<<1)),((SDIO_PC_OFFSET<<8)|(SDIO_PC_BIT&0xFF))
		WREG	(MS_BUS_RIU_ADDR + ((0x120F4E)<<1)),SDIO_WP_BANK
		WREG	(MS_BUS_RIU_ADDR + ((0x120F50)<<1)),(SDIO_WP_BIT&0xFF)
		WREG	(MS_BUS_RIU_ADDR + ((0x120F1C)<<1)),(SDIO_WP_OFFSET<<8)

        bx      lr
#endif

#ifdef CONFIG_MSTAR_VID_ENABLE
//-------------------------------------------------------------------------------------------------
// BootRom_InitVid
// @param  None
// @return None
// @note None
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitVid
BootRom_InitVid:

    //Disable All-Pad-In
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101e00 << 1))
    ldr         r0, [r1, #(0xa0 << 1)]
    bic         r0, r0, #0x8000
    str         r0, [r1, #(0xa0 << 1)]

//============================================================================
    //Set Specific Label of VID
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, =0x2330
    str         r0, [r1, #(0x00 << 1)]  //cluster 0

    //Set Initial Value of CPU Clock Register
    ldr         r0, =CONFIG_MSTAR_DVFS_INIT_CPU_CLOCK
    str         r0, [r1, #(0x02 << 1)]  //cluster 0

    //Set Initial Value of Running State
    ldr         r0, =0x0
    str         r0, [r1, #(0x04 << 1)]  //cluster 0

    //Set Initial Value of CPU Temperature Register
    ldr         r0, [r1, #(0x04 << 1)]
    orr         r0, #(0xFF << 0x08)
    str         r0, [r1, #(0x04 << 1)]  //cluster 0

    //Set Initial Value of Special Clock Register
    ldr         r0, =0x2454
    str         r0, [r1, #(0x06 << 1)]  //cluster 0

//============================================================================

    // efuse auto detect corner type
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0194
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2194
    str         r0, [r1, #(0x50 << 1)]

BootRom_InitVid_AutoDetect:
    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_InitVid_AutoDetect

    ldr         r7, [r1, #(0x5A << 1)]
    bic         r7, r7, #(0xFE)
    and         r7, r7, #(0x1)
    cmp         r7, #0
    beq         BootRom_InitVid_OldMode

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0194
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2194
    str         r0, [r1, #(0x50 << 1)]

BootRom_InitVid_AutoDetect_Core_Corner:
    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_InitVid_AutoDetect_Core_Corner

    ldr         r7, [r1, #(0x5A << 1)]
    bic         r7, r7, #(0xFB)
    and         r7, r7, #(0x4)
    cmp         r7, #0
    beq         BootRom_InitVid_SetCoreVoltage_SS
    b           BootRom_InitVid_SetCoreVoltage_FF

BootRom_InitVid_OldMode:

    //Get CPU+CORE SIDD Value        -> r2: bank6d[21:12]
    //Get CPU OSC Value              -> r3: bank6d[11:2]
    //Get CPU SIDD Value             -> r4: bank53[9:0]
    //Get GPU OSC Value              -> r5: bank53[29:20]
    //Get HI_CODEC OSC Value         -> r9: bank53[19:10]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x01b4
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x21b4
    str         r0, [r1, #(0x50 << 1)]

BootRom_InitVid_ReadEfuse_0:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_InitVid_ReadEfuse_0

    ldr         r6, [r1, #(0x58 << 1)]
    ldr         r7, [r1, #(0x5a << 1)]
    mov         r7, r7, lsl #16
    orr         r0, r6, r7

    ldr         r9, =0x3ff

    // r2: CPU+CORE SIDD Value
    lsr         r2, r0, #12
    and         r2, r2, r9

    // r3: CPU OSC Value
    lsr         r3, r0, #2
    and         r3, r3, r9

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x014c
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x214c
    str         r0, [r1, #(0x50 << 1)]

BootRom_InitVid_ReadEfuse_1:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_InitVid_ReadEfuse_1

    ldr         r6, [r1, #(0x58 << 1)]
    ldr         r7, [r1, #(0x5a << 1)]
    mov         r7, r7, lsl #16
    orr         r0, r6, r7

    // r4: CPU SIDD Value
    and         r4, r0, r9

    // r5: GPU OSC Value
    lsr         r5, r0, #20
    and         r5, r5, r9

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x014c
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x214c
    str         r0, [r1, #(0x50 << 1)]

BootRom_InitVid_ReadEfuse_2:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_InitVid_ReadEfuse_2

    ldr         r6, [r1, #(0x58 << 1)]
    lsr         r6, r6, #10
    ldr         r7, [r1, #(0x5a << 1)]
    and         r7, r7, #0x000F
    lsl         r7, r7, #6

    // r9: HI_CODEC OSC
    orr         r7, r7, r6
    mov         r9, r7

BootRom_InitVid_Normal_Package:
    //Set Chip Package
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, [r1, #(0x02 << 1)]
    bic         r0, r0, #(0x07 << 12)
    str         r0, [r1, #(0x02 << 1)]

BootRom_InitVid_Package_Exit:

    cmp         r2, #0
    beq         BootRom_InitVid_Exit                    //If no SIDD existed in eFuse, bypass VID flow

    cmp         r3, #0
    beq         BootRom_InitVid_Exit                    //If no SIDD existed in eFuse, bypass VID flow

    cmp         r4, #0
    beq         BootRom_InitVid_Exit                    //If no SIDD existed in eFuse, bypass VID flow

    cmp         r5, #0
    beq         BootRom_InitVid_Exit                    //If no SIDD existed in eFuse, bypass VID flow

    cmp         r9, #0
    beq         BootRom_InitVid_Exit                    //If no SIDD existed in eFuse, bypass VID flow

//============================================================================
    ldr         r6, =CONFIG_OSC_THRESHOLD_GPU
    cmp         r5, r6
    bls         BootRom_InitVid_SetCoreVoltage_SS       //If OSC <= The Value of Threshold 0, Set Corresponding Voltage 0

    // efuse auto detect sidd by ic
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0194
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2194
    str         r0, [r1, #(0x50 << 1)]

BootRom_AutoDetect_Sidd:
    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_AutoDetect_Sidd

    ldr         r7, [r1, #(0x58 << 1)]
    bic         r7, r7, #(0xFE)
    and         r7, r7, #0x1
    cmp         r7, #0
    beq         POR_IC
    b           CIP_IC

POR_IC:
    ldr         r6, =CONFIG_SIDD_POR_THRESHOLD
    b           SIDD_EXIT
CIP_IC:
    ldr         r6, =CONFIG_SIDD_CIP_THRESHOLD

SIDD_EXIT:
    cmp         r2, r6
    bls         BootRom_InitVid_SetCoreVoltage_SS       //If SIDD <= The Value of Threshold 0, Set Corresponding Voltage 0

    ldr         r6, =CONFIG_OSC_HI_CODEC_THRESHOLD
    cmp         r9, r6
    bls         BootRom_InitVid_SetCoreVoltage_SS       //If HI_CODEC_OSC <= The Value of Threshold 0, Set Corresponding Voltage 0

BootRom_InitVid_SetCoreVoltage_FF:
    //Set core Power Type for FF Corner Chip
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, [r1, #(0x04 << 1)]
    bic         r0, r0, #(0x03 << 5)
    orr         r0, r0, #(0x2 << 5)
    str         r0, [r1, #(0x04 << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002e00 << 1)) @//set VID1 input, VID0 input
    ldr         r0, =0x0202
    str         r0, [r1, #(0x84 << 1)]

    b           BootRom_InitVid_SetCoreVoltage_Exit

BootRom_InitVid_SetCoreVoltage_SS:
    //Set core Power Type for SS Corner Chip
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, [r1, #(0x04 << 1)]
    bic         r0, r0, #(0x03 << 5)
    str         r0, [r1, #(0x04 << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002e00 << 1)) @//set VID1 input, VID0 output low
    ldr         r0, =0x0200
    str         r0, [r1, #(0x84 << 1)]

BootRom_InitVid_SetCoreVoltage_Exit:

    // efuse auto detect corner type
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0194
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2194
    str         r0, [r1, #(0x50 << 1)]

BootRom_InitVid_AutoDetect_Cpu:
    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_InitVid_AutoDetect_Cpu

    ldr         r7, [r1, #(0x5A << 1)]
    bic         r7, r7, #(0xFE)
    and         r7, r7, #(0x1)
    cmp         r7, #0
    beq           BootRom_InitVidOldwModeCpu

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0194
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2194
    str         r0, [r1, #(0x50 << 1)]

BootRom_InitVid_AutoDetect_Cpu_Corner:
    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_InitVid_AutoDetect_Cpu_Corner

    ldr         r7, [r1, #(0x5A << 1)]
    bic         r7, r7, #(0xFD)
    and         r7, r7, #(0x2)
    cmp         r7, #0x0
    beq         BootRom_InitVid_SetCpuVoltage_SS
    b           BootRom_InitVid_SetCpuVoltage_FF

//============================================================================
BootRom_InitVidOldwModeCpu:

    ldr         r6, =CONFIG_OSC_THRESHOLD_CPU
    cmp         r3, r6
    bls         BootRom_InitVid_SetCpuVoltage_SS       //If OSC <= The Value of Threshold 0, Set Corresponding Voltage 0

    ldr         r6, =CONFIG_SIDD_THRESHOLD_CPU
    cmp         r4, r6
    bls         BootRom_InitVid_SetCpuVoltage_SS       //If SIDD <= The Value of Threshold 0, Set Corresponding Voltage 0

BootRom_InitVid_SetCpuVoltage_FF:
    //Set CPU Power Type for FF Corner Chip
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, [r1, #(0x04 << 1)]
    bic         r0, r0, #(0x03 << 3)
    orr         r0, r0, #(0x2 << 3)
    str         r0, [r1, #(0x04 << 1)]

    b           BootRom_InitVid_SetCpuVoltage_Exit

BootRom_InitVid_SetCpuVoltage_SS:
    //Set CPU Power Type for SS Corner Chip
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, [r1, #(0x04 << 1)]
    bic         r0, r0, #(0x03 << 3)
    str         r0, [r1, #(0x04 << 1)]

BootRom_InitVid_SetCpuVoltage_Exit:


    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000e00 << 1))
    ldr         r0, [r1, #(0x38 << 1)]
    ldr         r2, =(0x01 << 10)
    orr         r0, r0, r2
    str         r0, [r1, #(0x38 << 1)]

BootRom_InitVid_Exit:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_DVFS_ENABLE
//-------------------------------------------------------------------------------------------------
// BootRom_InitDvfs
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitDvfs
BootRom_InitDvfs:

//============================================================================
    //Set Specific Label of DVFS
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, =0x3697
    str         r0, [r1, #(0x00 << 1)]  //cluster 0

//============================================================================
    //Set Control Type of CPU power
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, [r1, #(0x08 << 1)]
    bic         r0, r0, #(0x03)
    #ifdef CONFIG_MSTAR_DVFS_CPU_GPIO
    ldr         r2, =0x1
    orr         r0, r2
    #elif defined(CONFIG_MSTAR_DVFS_CPU_PWM)
    ldr         r2, =0x2
    orr         r0, r2
    #endif
    str         r0, [r1, #(0x08 << 1)]
//============================================================================
    //Enable Temperature Sensor
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000e00 << 1))
    ldr         r0, [r1, #(0x5e << 1)]
    orr         r0, r0, #0x04
    str         r0, [r1, #(0x5e << 1)]

    //Switch CPU Temperature Sensor
    ldr         r0, [r1, #(0xc8 << 1)]
    ldr         r2, =(0x01 << 10)
    orr         r0, r0, r2
    str         r0, [r1, #(0xc8 << 1)]

//============================================================================

BootRom_InitDvfs_Exit:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_MOBF_ENABLE
//-------------------------------------------------------------------------------------------------
// BootRom_InitMobf
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitMobf
BootRom_InitMobf:

    bx          lr
#endif

//-------------------------------------------------------------------------------------------------
// BootRom_InitTimer
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitTimer
BootRom_InitTimer:

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x003000 << 1))
    ldr         r0, =0xffff
    str         r0, [r1, #(0x24 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x44 << 1)]
    ldr         r0, =0xffff
    str         r0, [r1, #(0x46 << 1)]
    ldr         r0, =0x0000
    str         r0, [r1, #(0x20 << 1)]
    ldr         r0, =0x0001
    str         r0, [r1, #(0x20 << 1)]
    ldr         r0, =0x0000
    str         r0, [r1, #(0x40 << 1)]
    ldr         r0, =0x0001
    str         r0, [r1, #(0x40 << 1)]

    bx          lr

#ifdef CONFIG_MSTAR_EFUSE_ENABLE
//-------------------------------------------------------------------------------------------------
// BootRom_InitEfuse
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitEfuse
BootRom_InitEfuse:

    mov         r12, lr

    //Set eFuse Redundancy bit
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0004
    str         r0, [r1, #(0x00 << 1)]
    str         r0, [r1, #(0x26 << 1)]

#ifdef CONFIG_MSTAR_EFUSE_VIDEO_ATOP
    //Initial Video ATOP Trimming Control Setting
    bl          BootRom_InitVideoAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_MHL_ECBUS_ATOP
    //Initial MHL E-/C-Bus ATOP Trimming Control Setting
    bl          BootRom_InitMhlEcbusAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_MOD_ATOP
    //Initial MOD Trimming Control Setting
    bl          BootRom_InitModAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_ETH_ATOP
    //Initial ETHERNET ATOP Trimming Control Setting
    bl          BootRom_InitEthAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_USB_XCVR
    //Initial USB XCVR Trimming Control Setting
    bl          BootRom_InitUsbXcvrTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_USB3_XCVR
    //Initial USB XCVR Trimming Control Setting
    bl          BootRom_InitUsb3XcvrTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_PM_SAR_ATOP
    //Initial PM SAR ATOP Trimming Control Setting
    bl          BootRom_InitPmSarAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_CBUS
    //Initial CBUS Trimming Control Setting
    bl          BootRom_InitCbusTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_MHL_COMBO_ATOP
    //Initial MHL Combo ATOP Trimming Control Setting
    bl          BootRom_InitMhlComboAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_HDMI2RX_ATOP
    //Initial HDMI2RX Trimming Control Setting
    bl          BootRom_InitHDMI2RXTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_EARC_TX_ATOP
    //Initial EARC Tx ATOP Trimming Control Setting
    bl          BootRom_InitEarcTxAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_HPD
    //Initial HPD Trimming Control Setting
    bl          BootRom_InitHpdTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_CEC
    //Initial CEC Trimming Control Setting
    bl          BootRom_InitCecTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_DEMOD_ATOP
    //Initial Demod ATOP Trimming Control Setting
    bl          BootRom_InitDemodAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_AUSDM_ATOP
    //Initial Demod ATOP Trimming Control Setting
    bl          BootRom_InitAusdmAtopTrimmingControl
#endif

#ifdef CONFIG_MSTAR_EFUSE_ARC_ATOP
    //Initial ARC ATOP Trimming Control Setting
    bl          BootRom_InitArcAtopTrimmingControl
#endif

    mov         lr, r12
    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_VIDEO_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitVideoAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitVideoAtopTrimmingControl
BootRom_InitVideoAtopTrimmingControl:

    //Get Trim Value of Video ATOP in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0134
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2134
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_VideoAtopLoop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_VideoAtopLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //Bandgap voltage trimming
    //Activateefuse
    and         r2, r0, #(0x1 << 7)
    cmp         r2, #0
    beq         BootRom_InitVideoAtopTrimmingControl_BandgapVoltageTrimming_Exit

    and         r4, r0, #(0x1F << 0)
    lsl         r4, r4, #8

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x102500 << 1))
    ldr         r2, [r1, #(0xB6 << 1)]
    ldr         r3, =(0x1F << 8)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0xB6 << 1)]  //0x1025_5B[12:8]

    orr         r2, r2, #(0x01 << 13)
    str         r2, [r1, #(0xB6 << 1)]  //0x1025_5B[13] =1'b1

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103D00 << 1))
    ldr         r2, [r1, #(0x20 << 1)]
    orr         r2, r2, #(0x01 << 10)
    str         r2, [r1, #(0x20 << 1)]  //0x103D_10[10]=1'b1

BootRom_InitVideoAtopTrimmingControl_BandgapVoltageTrimming_Exit:

#ifdef CONFIG_MSTAR_EFUSE_VIDEO_ATOP_FULL_CURRENT

    //IDAC_cur_full
    //Activateefuse
    and         r2, r0, #(0x1 << 15)
    cmp         r2, #0
    beq         BootRom_InitVideoAtopTrimmingControlExit_Full_current

    and         r4, r0, #(0x7F << 8)
    lsr         r4, r4, #8

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103D00 << 1))
    ldr         r2, [r1, #(0x10 << 1)]
    ldr         r3, =(0x0F << 12)
    bic         r2, r2, r3              //0x103D_08[15:12] = 4'b0000

    ldr         r3, =(0x7F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x10 << 1)]  //0x103D_08[6:0]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103D00 << 1))
    ldr         r2, [r1, #(0x20 << 1)]
    ldr         r3, =(0x01 << 10)
    bic         r2, r2, r3
    str         r2, [r1, #(0x20 << 1)]  //0x103D_10[10]=1'b0

BootRom_InitVideoAtopTrimmingControlExit_Full_current:

#endif

#ifdef CONFIG_MSTAR_EFUSE_VIDEO_ATOP_ONE_HALF_CURRENT

    //IDAC_cur_half
    //Activateefuse
    and         r2, r0, #(0x1 << 23)
    cmp         r2, #0
    beq         BootRom_InitVideoAtopTrimmingControlExit_Half_Current

    and         r4, r0, #(0x7F << 16)
    lsr         r4, r4, #16

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103D00 << 1))
    ldr         r2, [r1, #(0x10 << 1)]
    ldr         r3, =(0x0F << 12)
    bic         r2, r2, r3
    ldr         r3, =(0x02 << 12)
    orr         r2, r2, r3              //0x103D_08[15:12] = 4'b0010

    ldr         r3, =(0x7F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x10 << 1)]  //0x103D_08[6:0]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103D00 << 1))
    ldr         r2, [r1, #(0x20 << 1)]
    ldr         r3, =(0x01 << 10)
    bic         r2, r2, r3
    str         r2, [r1, #(0x20 << 1)]  //0x103D_10[10]=1'b0

BootRom_InitVideoAtopTrimmingControlExit_Half_Current:

#endif

#ifdef CONFIG_MSTAR_EFUSE_VIDEO_ATOP_ONE_QUARTER_CURRENT

    //IDAC_cur_quar
    //Activateefuse
    and         r2, r0, #(0x1 << 31)
    cmp         r2, #0
    beq         BootRom_InitVideoAtopTrimmingControlExit_Quarter_Current

    and         r4, r0, #(0x7F << 24)
    lsr         r4, r4, #24

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103D00 << 1))
    ldr         r2, [r1, #(0x10 << 1)]
    ldr         r3, =(0x0F << 12)
    bic         r2, r2, r3
    ldr         r3, =(0x02 << 12)
    orr         r2, r2, r3              //0x103D_08[15:12] = 4'b0010

    ldr         r3, =(0x7F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x10 << 1)]  //0x103D_08[6:0]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103D00 << 1))
    ldr         r2, [r1, #(0x20 << 1)]
    ldr         r3, =(0x01 << 10)
    bic         r2, r2, r3
    str         r2, [r1, #(0x20 << 1)]  0x103D_10[10]=1'b0

BootRom_InitVideoAtopTrimmingControlExit_Quarter_Current:

#endif

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_DEMOD_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitDemodAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitDemodAtopTrimmingControl
BootRom_InitDemodAtopTrimmingControl:

    //Get Trim Value of Demod ATOP in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0138
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2138
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_DemodAtopLoop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_DemodAtopLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //DEMOD_vol_bgap voltage trimming
    //Activateefuse
    and         r2, r0, #(0x1 << 15)
    cmp         r2, #0
    beq         BootRom_InitDemodAtopTrimmingControl_BgapVoltageTrimming_Exit

    and         r4, r0, #(0x1F << 8)
    lsr         r4, r4, #8

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x112800 << 1))
    ldr         r2, [r1, #(0x8C << 1)]
    ldr         r3, =(0x1F)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x8C << 1)]  //0x1128_46[4:0]

BootRom_InitDemodAtopTrimmingControl_BgapVoltageTrimming_Exit:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_AUSDM_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitDemodAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitAusdmAtopTrimmingControl
BootRom_InitAusdmAtopTrimmingControl:

    //Get Trim Value of AUSDM ATOP in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0130
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2130
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_AusdmAtopLoop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_AusdmAtopLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //AUSDM_vol_ref voltage trimming
    //Activateefuse
    and         r2, r0, #(0x1 << 6)
    cmp         r2, #0
    beq         BootRom_InitAusdmAtopTrimmingControl_RefVoltageTrimming_Exit

    and         r4, r0, #(0x3F << 0)
    lsl         r4, r4, #8

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x112C00 << 1))
    ldr         r2, [r1, #(0xF6 << 1)]
    ldr         r3, =(0x3F << 8)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0xF6 << 1)]  //0x112c_7B[13:8]

BootRom_InitAusdmAtopTrimmingControl_RefVoltageTrimming_Exit:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_MHL_COMBO_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitMhlComboAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitMhlComboAtopTrimmingControl
BootRom_InitMhlComboAtopTrimmingControl:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_MHL_ECBUS_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitMhlEcbusAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitMhlEcbusAtopTrimmingControl
BootRom_InitMhlEcbusAtopTrimmingControl:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_MOD_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitModAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitModAtopTrimmingControl
BootRom_InitModAtopTrimmingControl:

    //Get Trim Value of MOD ATOP in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0174
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2174
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_ModAtopLoop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_ModAtopLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //MOD_vx1
    //Activateefuse
    and         r2, r0, #(0x1 << 6)
    cmp         r2, #0
    beq         BootRom_InitModAtopTrimmingControlExit_Vx1

    and         r2, r0, #(0x3F << 0)
    lsl         r1, r2, #8
    orr         r4, r2, r1

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x360200 << 1))
    ldr         r2, [r1, #(0x20 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x20 << 1)]  //CH0/CH1 RCON: 0x3602_10[13:8][5:0]

    ldr         r2, [r1, #(0x22 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x22 << 1)]  //CH2/CH3 RCON: 0x3602_11[13:8][5:0]

    ldr         r2, [r1, #(0x24 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x24 << 1)]  //CH4/CH5 RCON: 0x3602_12[13:8][5:0]

    ldr         r2, [r1, #(0x26 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x26 << 1)]  //CH6/CH7 RCON: 0x3602_13[13:8][5:0]

    ldr         r2, [r1, #(0x28 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x28 << 1)]  //CH8/CH9 RCON: 0x3602_14[13:8][5:0]

    ldr         r2, [r1, #(0x2A << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x2A << 1)]  //CH10/CH11 RCON: 0x3602_15[13:8][5:0]

    ldr         r2, [r1, #(0x2C << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x2C << 1)]  //CH12/CH13 RCON: 0x3602_16[13:8][5:0]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x111E00 << 1))
    ldr         r2, [r1, #(0x20 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x20 << 1)]  //CH0/CH1 RCON: 0x111E_10[13:8][5:0]

    ldr         r2, [r1, #(0x22 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x22 << 1)]  //CH2/CH3 RCON: 0x111E_11[13:8][5:0]

    ldr         r2, [r1, #(0x24 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x24 << 1)]  //CH4/CH5 RCON: 0x111E_12[13:8][5:0]

    ldr         r2, [r1, #(0x26 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x26 << 1)]  //CH6/CH7 RCON: 0x111E_13[13:8][5:0]

    ldr         r2, [r1, #(0x28 << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x28 << 1)]  //CH8/CH9 RCON: 0x111E_14[13:8][5:0]

    ldr         r2, [r1, #(0x2A << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x2A << 1)]  //CH10/CH11 RCON: 0x111E_15[13:8][5:0]

    ldr         r2, [r1, #(0x2C << 1)]
    ldr         r3, =(0x3F3F << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x2C << 1)]  //CH12/CH13 RCON: 0x111E_16[13:8][5:0]

BootRom_InitModAtopTrimmingControlExit_Vx1:

    //MOD_Cur (for Single term)
    //Activateefuse
    and         r2, r0, #(0x1 << 14)
    cmp         r2, #0
    beq         BootRom_InitModAtopTrimmingControl_DualTerm

    and         r4, r0, #(0x7F << 7)
    lsr         r4, r4, #7
    b           BootRom_InitModAtopTrimmingControl_Set

BootRom_InitModAtopTrimmingControl_DualTerm:

    //MOD_Cur (for Dual term)
    //Activateefuse
    and         r2, r0, #(0x1 << 22)
    cmp         r2, #0
    beq         BootRom_InitModAtopTrimmingControlExit_Cur

    and         r4, r0, #(0x7F << 15)
    lsr         r4, r4, #7

BootRom_InitModAtopTrimmingControl_Set:

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x111E00 << 1))
    ldr         r2, [r1, #(0x6E << 1)]
    bic         r2, r2, #(0x007F << 0)
    orr         r2, r2, r4
    str         r2, [r1, #(0x6E << 1)]  // 0x111E_37[6:0]

BootRom_InitModAtopTrimmingControlExit_Cur:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_USB_XCVR
//-------------------------------------------------------------------------------------------------
// BootRom_InitUsbXcvrTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitUsbXcvrTrimmingControl
BootRom_InitUsbXcvrTrimmingControl:

    //Get Trim Value of USB XCVR in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0140
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2140
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_UsbXcvrLoop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_UsbXcvrLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //Activateefuse
    and         r2, r0, #(0x1 << 4)
    cmp         r2, #0
    beq         BootRom_InitUsbXcvrTrimmingControlExit0

    //I0:P0
    and         r4, r0, #(0x0F << 0)
    lsl         r4, r4, #5

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103A00 << 1))
    ldr         r2, [r1, #(0xA8 << 1)]
    bic         r2, r2, #(0x0F << 5)
    orr         r2, r2, r4
    str         r2, [r1, #(0xA8 << 1)]  //0x103A_54[8:5]

    //I1:P0
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103A00 << 1))
    ldr         r2, [r1, #(0x28 << 1)]
    bic         r2, r2, #(0x0F << 5)
    orr         r2, r2, r4
    str         r2, [r1, #(0x28 << 1)]  //0x103A_14[8:5]

    //I2:P0
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x011400 << 1))
    ldr         r2, [r1, #(0xA8 << 1)]
    bic         r2, r2, #(0x0F << 5)
    orr         r2, r2, r4
    str         r2, [r1, #(0xA8 << 1)]  //0x0114_54[8:5]

BootRom_InitUsbXcvrTrimmingControlExit0:

    //Activateefuse
    and         r2, r0, #(0x1 << 11)
    cmp         r2, #0
    beq         BootRom_InitUsbXcvrTrimmingControlExit1

    //I0:P0
    and         r4, r0, #(0x03 << 5)
    lsr         r4, r4, #5
    lsl         r4, r4, #8
    
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103A00 << 1))
    ldr         r2, [r1, #(0xAA << 1)]
    bic         r2, r2, #(0x03 << 8)
    orr         r2, r2, r4
    str         r2, [r1, #(0xAA << 1)]  //0x103A_55[9:8]

    //I1:P0
    and         r4, r0, #(0x03 << 7)
    lsr         r4, r4, #7
    lsl         r4, r4, #8

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x103A00 << 1))
    ldr         r2, [r1, #(0x2A << 1)]
    bic         r2, r2, #(0x03 << 8)
    orr         r2, r2, r4
    str         r2, [r1, #(0x2A << 1)]  //0x103A_15[9:8]
    
    //I2:P0
    and         r4, r0, #(0x03 << 9)
    lsr         r4, r4, #9
    lsl         r4, r4, #8

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x011400 << 1))
    ldr         r2, [r1, #(0xAA << 1)]
    bic         r2, r2, #(0x03 << 8)
    orr         r2, r2, r4
    str         r2, [r1, #(0xAA << 1)]  //0x0114_55[9:8]

BootRom_InitUsbXcvrTrimmingControlExit1:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_USB3_XCVR
//-------------------------------------------------------------------------------------------------
// BootRom_InitUsb3XcvrTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitUsb3XcvrTrimmingControl
BootRom_InitUsb3XcvrTrimmingControl:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_PM_SAR_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitPmSarAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitPmSarAtopTrimmingControl
BootRom_InitPmSarAtopTrimmingControl:

    //Get PM SAR Trim Value of T_Sensor in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0144
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2144
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_PmSarAtopLoop:
    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_PmSarAtopLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //Activateefuse
    and         r4, r0, #(0x1 << 16)
    cmp         r4, #0
    beq         BootRom_InitPmSarAtopTrimmingControlExit

    //PM_vol_bgap
    and         r4, r0, #(0x3F << 0)
    lsl         r4, r4, #1

    //I0:P0
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000E00 << 1))
    ldr         r2, [r1, #(0xC8 << 1)]
    bic         r2, r2, #(0x003F << 1)
    orr         r2, r2, r4
    str         r2, [r1, #(0xC8 << 1)]  //0x000e_64[6:1]

BootRom_InitPmSarAtopTrimmingControlExit:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_ETH_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitEthAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitEthAtopTrimmingControl
BootRom_InitEthAtopTrimmingControl:

    //Get Trim Value of ETH_ATOP in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0168
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2168
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_EthAtopLoop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_EthAtopLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //Activateefuse
    and         r2, r0, #(0x1 << 18)
    cmp         r2, #0
    beq         BootRom_InitEthAtopTrimmingControlExit

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x003400 << 1))
    ldr         r2, [r1, #(0x60 << 1)]
    orr         r2, r2, #(0x01 << 2)
    str         r2, [r1, #(0x60 << 1)]      //0x0034_30[2]=1

    ldr         r2, [r1, #(0x68<< 1)]
    orr         r2, r2, #(0x01 << 15)
    str         r2, [r1, #(0x68 << 1)]      //0x0034_34[15]=1

    //ETH_cur_100t
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x003400 << 1))
    ldr         r2, [r1, #(0x68 << 1)]
    bic         r2, r2, #(0x1F << 0)
    and         r4, r0, #(0x1F << 0)
    orr         r2, r2, r4
    str         r2, [r1, #(0x68 << 1)]      //0x0034_34[4:0]

    //ETH_cur_10t
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x003400 << 1))
    ldr         r2, [r1, #(0x68 << 1)]
    bic         r2, r2, #(0x1F << 5)
    and         r4, r0, #(0x1F << 5)
    orr         r2, r2, r4
    str         r2, [r1, #(0x68 << 1)]      //0x0034_34[9:5]

    //ETH_imp
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x003400 << 1))
    ldr         r2, [r1, #(0x68 << 1)]
    ldr         r3, =(0x0F << 10)
    bic         r2, r2, #(0x0F << 10)
    and         r4, r0, #(0x0F << 10)
    orr         r2, r2, r4
    str         r2, [r1, #(0x68 << 1)]      //0x0034_34[13:10]

    //ETH_lpf
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x003400 << 1))
    ldr         r2, [r1, #(0x60 << 1)]
    bic         r2, r2, #(0x0F << 7)
    and         r4, r0, #(0x0F << 14)
    lsr         r4, r4, #14
    lsl         r4, r4, #7
    orr         r2, r2, r4
    str         r2, [r1, #(0x60 << 1)]      //0x0034_30[10:7]

BootRom_InitEthAtopTrimmingControlExit:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_CBUS
//-------------------------------------------------------------------------------------------------
// BootRom_InitCbusTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitCbusTrimmingControl
BootRom_InitCbusTrimmingControl:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_HDMI2RX_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitHDMI2TXTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitHDMI2RXTrimmingControl
BootRom_InitHDMI2RXTrimmingControl:

   //Get Trim Value of MHL Combo ATOP in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x013C
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x213C
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_HDMI2RXLoop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_HDMI2RXLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

#if !defined(CONFIG_HDMI2RX_EFUSE_45OHM)

    //MHL_rterm
    //Activateefuse (50Ohm)
    and         r2, r0, #(0x01 << 4)
    cmp         r2, #0
    beq         BootRom_InitHDMI2RXTrimmingControlExit

    and         r2, r0, #(0x0F << 0)
    lsl         r3, r2, #8
    lsl         r1, r2, #4
    orr         r4, r3, r1

    //I0:P0/P1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x010700 << 1))
    ldr         r2, [r1, #(0x80 << 1)]
    ldr         r3, =(0x0F << 8)
    ldr         r3, =(0x0FF0 << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x80 << 1)]  //0x0107_40[11:8][7:4]

    ldr         r2, [r1, #(0x80 << 1)]
    orr         r2, r2, #(0x03 << 14)
    str         r2, [r1, #(0x80 << 1)]  //0x0107_40[15][14]=11

    //I1:P0/P1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x010800 << 1))
    ldr         r2, [r1, #(0x80 << 1)]
    ldr         r3, =(0x0F << 8)
    ldr         r3, =(0x0FF0 << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x80 << 1)]  //0x0108_40[11:8][7:4]

    ldr         r2, [r1, #(0x80 << 1)]
    orr         r2, r2, #(0x03 << 14)
    str         r2, [r1, #(0x80 << 1)]  //0x0108_40[15][14]=11

    //I2:P0/P1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x010900 << 1))
    ldr         r2, [r1, #(0x80 << 1)]
    ldr         r3, =(0x0F << 8)
    ldr         r3, =(0x0FF0 << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x80 << 1)]  //0x0109_40[11:8][7:4]

    ldr         r2, [r1, #(0x80 << 1)]
    orr         r2, r2, #(0x03 << 14)
    str         r2, [r1, #(0x80 << 1)]  //0x0109_40[15][14]=11

    //I3:P0/P1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x010A00 << 1))
    ldr         r2, [r1, #(0x80 << 1)]
    ldr         r3, =(0x0F << 8)
    ldr         r3, =(0x0FF0 << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x80 << 1)]  //0x010a_40[11:8][7:4]

    ldr         r2, [r1, #(0x80 << 1)]
    orr         r2, r2, #(0x03 << 14)
    str         r2, [r1, #(0x80 << 1)]  //0x010a_40[15][14]=11

#else
    //Activateefuse (45Ohm)
    and         r2, r0, #(0x01 << 9)
    cmp         r2, #0
    beq         BootRom_InitHDMI2RXTrimmingControlExit

    and         r2, r0, #(0x0F << 5)
    lsr         r2, r2, #5
    lsl         r3, r2, #8
    lsl         r1, r2, #4
    orr         r4, r3, r1

    //I0:P0/P1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x010700 << 1))
    ldr         r2, [r1, #(0x80 << 1)]
    ldr         r3, =(0x0F << 8)
    ldr         r3, =(0x0FF0 << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x80 << 1)]  //0x0107_40[11:8][7:4]

    ldr         r2, [r1, #(0x80 << 1)]
    orr         r2, r2, #(0x03 << 14)
    str         r2, [r1, #(0x80 << 1)]  //0x0107_40[15][14]=11

    //I1:P0/P1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x010800 << 1))
    ldr         r2, [r1, #(0x80 << 1)]
    ldr         r3, =(0x0F << 8)
    ldr         r3, =(0x0FF0 << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x80 << 1)]  //0x0108_40[11:8][7:4]

    ldr         r2, [r1, #(0x80 << 1)]
    orr         r2, r2, #(0x03 << 14)
    str         r2, [r1, #(0x80 << 1)]  //0x0108_40[15][14]=11

    //I2:P0/P1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x010900 << 1))
    ldr         r2, [r1, #(0x80 << 1)]
    ldr         r3, =(0x0F << 8)
    ldr         r3, =(0x0FF0 << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x80 << 1)]  //0x0109_40[11:8][7:4]

    ldr         r2, [r1, #(0x80 << 1)]
    orr         r2, r2, #(0x03 << 14)
    str         r2, [r1, #(0x80 << 1)]  //0x0109_40[15][14]=11

    //I3:P0/P1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x010A00 << 1))
    ldr         r2, [r1, #(0x80 << 1)]
    ldr         r3, =(0x0F << 8)
    ldr         r3, =(0x0FF0 << 0)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0x80 << 1)]  //0x010a_40[11:8][7:4]

    ldr         r2, [r1, #(0x80 << 1)]
    orr         r2, r2, #(0x03 << 14)
    str         r2, [r1, #(0x80 << 1)]  //0x010a_40[15][14]=11
#endif

BootRom_InitHDMI2RXTrimmingControlExit:

    bx          lr
#endif


#ifdef CONFIG_MSTAR_EFUSE_EARC_TX_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitEarcTxAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitEarcTxAtopTrimmingControl
BootRom_InitEarcTxAtopTrimmingControl:

    bx          lr

#endif

#ifdef CONFIG_MSTAR_EFUSE_ARC_ATOP
//-------------------------------------------------------------------------------------------------
// BootRom_InitArcAtopTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitArcAtopTrimmingControl
BootRom_InitArcAtopTrimmingControl:

    //Get Trim Value of ARC ATOP in eFuse
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0178
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2178
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_ArcAtopLoop:

    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_ArcAtopLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //Trim ARC output voltage trimming
    //Activateefuse
    and         r2, r0, #(0x1 << 10)
    cmp         r2, #0
    beq         BootRom_InitArcAtopTrimmingControl_ArcOutputVoltageTrimming_Exit

    and         r4, r0, #(0x0F << 6)
    lsr         r4, r4, #6
    lsl         r4, r4, #4

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x322e00 << 1))
    ldr         r2, [r1, #(0xF0 << 1)]
    ldr         r3, =(0x0F << 4)
    bic         r2, r2, r3
    orr         r2, r2, r4
    str         r2, [r1, #(0xF0 << 1)]  //0x322e_78[7:4]

BootRom_InitArcAtopTrimmingControl_ArcOutputVoltageTrimming_Exit:

    bx          lr

#endif

#ifdef CONFIG_MSTAR_EFUSE_HPD
//-------------------------------------------------------------------------------------------------
// BootRom_InitHpdTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitHpdTrimmingControl
BootRom_InitHpdTrimmingControl:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0178
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2178
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_HpdLoop:
    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_HpdLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //Activateefuse
    and         r4, r0, #(0x01 << 2)
    cmp         r4, #0
    beq         BootRom_InitHpdTrimmingControlExit

    //HPD en
    and         r4, r0, #(0x01 << 0)
    cmp         r4, #0
    bne         BootRom_InitHpdTrimmingControlHpdSetup
    
    //TRIM_EN =0 ==> default value
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002e00 << 1))
    ldr         r2, [r1, #(0xf8 << 1)]
    bic         r2, r2, #(0x0F << 0)
    str         r2, [r1, #(0xf8 << 1)]

    b           BootRom_InitHpdTrimmingControlExit

BootRom_InitHpdTrimmingControlHpdSetup:
    
    //TRIM_EN=1 
    //TRIM_SEL=0 ==> tirm low
    //TRIM_SEL=1 ==> tirm high
    
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002e00 << 1))
    ldr         r2, [r1, #(0xf8 << 1)]
    orr         r2, r2, #(0x0F << 0)

    bic         r2, r2, #(0x0F << 4)
    and         r4, r0, #(0x01 << 1)
    cmp         r4, #0
    beq         BootRom_InitHpdTrimmingControlHpdSelLow

    orr         r2, r2, #(0x0F << 4)

BootRom_InitHpdTrimmingControlHpdSelLow:

    str         r2, [r1, #(0xf8 << 1)]

BootRom_InitHpdTrimmingControlExit:

    bx          lr
#endif

#ifdef CONFIG_MSTAR_EFUSE_CEC
//-------------------------------------------------------------------------------------------------
// BootRom_InitCecTrimmingControl
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitCecTrimmingControl
BootRom_InitCecTrimmingControl:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002000 << 1))
    ldr         r0, =0x0178
    str         r0, [r1, #(0x50 << 1)]
    ldr         r0, =0x2178
    str         r0, [r1, #(0x50 << 1)]

BootRom_ReadEfuse_CecLoop:
    ldr         r0, [r1, #(0x50 << 1)]
    and         r0, r0, #(0x0001 << 13)
    cmp         r0, #0
    bne         BootRom_ReadEfuse_CecLoop

    ldr         r2, [r1, #(0x58 << 1)]
    ldr         r3, [r1, #(0x5a << 1)]
    mov         r3, r3, lsl #16
    orr         r0, r2, r3

    //Activateefuse
    and         r4, r0, #(0x01 << 5)
    cmp         r4, #0
    beq         BootRom_InitCecTrimmingControlExit

    //CEC en
    and         r4, r0, #(0x01 << 3)
    cmp         r4, #0
    bne         BootRom_InitCecTrimmingControlCecSetup

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002e00 << 1))
    ldr         r2, [r1, #(0xf6 << 1)]
    bic         r2, r2, #(0x01 << 0)
    str         r2, [r1, #(0xf6 << 1)]

    b           BootRom_InitCecTrimmingControlExit

BootRom_InitCecTrimmingControlCecSetup:

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002e00 << 1))
    ldr         r2, [r1, #(0xf6 << 1)]
    orr         r2, r2, #(0x01 << 0)

    bic         r2, r2, #(0x01 << 4)
    and         r4, r0, #(0x01 << 4)
    cmp         r4, #0
    beq         BootRom_InitCecTrimmingControlCecSelLow

    orr         r2, r2, #(0x01 << 4)

BootRom_InitCecTrimmingControlCecSelLow:

    str         r2, [r1, #(0xf6 << 1)]

BootRom_InitCecTrimmingControlExit:

    bx          lr

#endif

    .global     BootRom_SaveHash1Version
BootRom_SaveHash1Version:
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x0600 << 1))
    ldr         r11, =HASH1_VERSION
    str         r11, [r12, #(0x00 << 1)]
    bx          lr
