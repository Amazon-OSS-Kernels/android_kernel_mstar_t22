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

#ifndef _BOARD_H_
#include "Board.h"
#endif

#ifndef _DRV_MIU_INIT_H_
#include "drv_miu_init.h"
#endif

#ifndef _MIUSEL_M7642_2MIU_H_
#include "MIUSEL_M7642_2MIU.h"
#endif
#ifdef CONFIG_ONEBIN_ENABLE
#include "drvLoadTbl.h"
#endif

    .text

MIU_MESSAGE_BIST0:      .short   'B', 'I', 'S', 'T', '0', '-', 0x00
MIU_MESSAGE_BIST1:      .short   'B', 'I', 'S', 'T', '1', '-', 0x00

MIU_MESSAGE_OK:         .short   'O', 'K', '\n', '\r', 0x00
MIU_MESSAGE_FAIL:       .short   'F', 'A', 'I', 'L', '\n', '\r', 0x00


    .align      4
    .global     BootRom_InitMiu
BootRom_InitMiu:

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && defined(CONFIG_MBOOT_IN_SPI_FLASH)
    mov         r8, lr
#else
    push        {lr}
#endif

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && !defined(CONFIG_MSTAR_HAPS)

    //-------------------------------------------------------
    // MIU Channel Config
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x152b26 << 1)), 0x0d06
    WREG        (MS_BUS_RIU_ADDR + (0x1012dc << 1)), 0x000c
    WREG        (MS_BUS_RIU_ADDR + (0x152b36 << 1)), 0x00c0
    WREG        (MS_BUS_RIU_ADDR + (0x152b28 << 1)), 0x1400
    WREG        (MS_BUS_RIU_ADDR + (0x152c28 << 1)), 0x0400

    //-------------------------------------------------------
    // MIU DDRPLL Setting
    //-------------------------------------------------------
    // MIU reset & mask all
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x0c01
    WREG        (MS_BUS_RIU_ADDR + (0x1615e6 << 1)), 0xfffe

    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x0c01
    WREG        (MS_BUS_RIU_ADDR + (0x1622e6 << 1)), 0xfffe

    //----------------------
    // Wait PLL Stable!!
    //----------------------
    DELAYUS     10000 // delay 10ms

    //-------------------------------------------------------
    // MIU Table Setting
    //-------------------------------------------------------
    // MIU0
    WREG        (MS_BUS_RIU_ADDR + (0x101202 << 1)), 0x0800
    WREG        (MS_BUS_RIU_ADDR + (0x101204 << 1)), 0x01c5
    WREG        (MS_BUS_RIU_ADDR + (0x101206 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x101228 << 1)), 0x0100
    WREG        (MS_BUS_RIU_ADDR + (0x152b20 << 1)), 0x1200
    WREG        (MS_BUS_RIU_ADDR + (0x152b22 << 1)), 0x1101
    WREG        (MS_BUS_RIU_ADDR + (0x152b40 << 1)), 0x0f0f
    WREG        (MS_BUS_RIU_ADDR + (0x152b42 << 1)), 0x081f
    WREG        (MS_BUS_RIU_ADDR + (0x152b44 << 1)), 0x2f08
    WREG        (MS_BUS_RIU_ADDR + (0x152b46 << 1)), 0x0f01
    WREG        (MS_BUS_RIU_ADDR + (0x152b48 << 1)), 0x070f
    WREG        (MS_BUS_RIU_ADDR + (0x152b4a << 1)), 0x0102
    WREG        (MS_BUS_RIU_ADDR + (0x152b4e << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x152b50 << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x152b54 << 1)), 0x0f09
    WREG        (MS_BUS_RIU_ADDR + (0x152b5e << 1)), 0x2400
    WREG        (MS_BUS_RIU_ADDR + (0x152b60 << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x152b80 << 1)), 0x8010
    WREG        (MS_BUS_RIU_ADDR + (0x152b86 << 1)), 0x0000

    //MIU1
    WREG        (MS_BUS_RIU_ADDR + (0x100602 << 1)), 0x0800
    WREG        (MS_BUS_RIU_ADDR + (0x100604 << 1)), 0x01c5
    WREG        (MS_BUS_RIU_ADDR + (0x100606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x100628 << 1)), 0x0100
    WREG        (MS_BUS_RIU_ADDR + (0x152c20 << 1)), 0x1200
    WREG        (MS_BUS_RIU_ADDR + (0x152c22 << 1)), 0x1101
    WREG        (MS_BUS_RIU_ADDR + (0x152c40 << 1)), 0x0f0f
    WREG        (MS_BUS_RIU_ADDR + (0x152c42 << 1)), 0x081f
    WREG        (MS_BUS_RIU_ADDR + (0x152c44 << 1)), 0x2f08
    WREG        (MS_BUS_RIU_ADDR + (0x152c46 << 1)), 0x0f01
    WREG        (MS_BUS_RIU_ADDR + (0x152c48 << 1)), 0x070f
    WREG        (MS_BUS_RIU_ADDR + (0x152c4a << 1)), 0x0102
    WREG        (MS_BUS_RIU_ADDR + (0x152c4e << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x152c50 << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x152c54 << 1)), 0x0f09
    WREG        (MS_BUS_RIU_ADDR + (0x152c5e << 1)), 0x2400
    WREG        (MS_BUS_RIU_ADDR + (0x152c60 << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x152c80 << 1)), 0x8010
    WREG        (MS_BUS_RIU_ADDR + (0x152c86 << 1)), 0x0000

    //-------------------------------------------------------
    // miu_pre_inivec_dig setting start MIU0
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x1615e4 << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x1622e4 << 1)), 0x000e

    //program dig general setting
    WREG        (MS_BUS_RIU_ADDR + (0x152b2a << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x152bdc << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x161580 << 1)), 0x0090
    WREG        (MS_BUS_RIU_ADDR + (0x152b8a << 1)), 0xff10
    WREG        (MS_BUS_RIU_ADDR + (0x152b80 << 1)), 0x80f0
    WREG        (MS_BUS_RIU_ADDR + (0x152bde << 1)), 0x8002
    WREG        (MS_BUS_RIU_ADDR + (0x1012fc << 1)), 0x951a

    //-------------------------------------------------------
    // miu_pre_inivec_dig setting start MIU1
    //-------------------------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x1615e4 << 1)), 0x000e
    WREG        (MS_BUS_RIU_ADDR + (0x1622e4 << 1)), 0x000e

    //program dig general setting
    WREG        (MS_BUS_RIU_ADDR + (0x152c2a << 1)), 0x0031
    WREG        (MS_BUS_RIU_ADDR + (0x152cdc << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x162280 << 1)), 0x0090
    WREG        (MS_BUS_RIU_ADDR + (0x152c8a << 1)), 0xff90
    WREG        (MS_BUS_RIU_ADDR + (0x152c80 << 1)), 0x80f0
    WREG        (MS_BUS_RIU_ADDR + (0x152cde << 1)), 0x8002
    WREG        (MS_BUS_RIU_ADDR + (0x1006fc << 1)), 0x951a

    // MIU SW reset
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x8c00
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x8c00

    //--------------------------------------
    // MIU DRAM init
    //--------------------------------------
    // begin init DRAM
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x0000
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000c
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x000e
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x100600 << 1)), 0x001f
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x0000
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x0008
    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000c
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x000e
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x101200 << 1)), 0x001f
    DELAYUS     1000

    DELAYUS     10000

    //--------------------------------------
    // Arbitration setting
    //--------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x310212 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310312 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310412 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310512 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310612 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310712 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310812 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310912 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310a12 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310230 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310200 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310330 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310300 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310430 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310400 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310530 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310500 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310630 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310600 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310730 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310700 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310830 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310800 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310930 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310900 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310a30 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310a00 << 1)), 0x0303

    //--------------------------------------
    //CTRL Arbiter setting
    //--------------------------------------
    WREG        (MS_BUS_RIU_ADDR + (0x161534 << 1)), 0x03fe
    WREG        (MS_BUS_RIU_ADDR + (0x161504 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161506 << 1)), 0x7ffe
    WREG        (MS_BUS_RIU_ADDR + (0x16150a << 1)), 0xffff

_CheckDramData:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, [r1]
    and         r0, r0, #0x8000
    cmp         r0, #0
    beq         _CheckDramData

    //unmask client
    WREG        (MS_BUS_RIU_ADDR + (0x310206 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310306 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310406 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310506 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310706 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310806 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310906 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310a06 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f0 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af0 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f2 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af2 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f4 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af4 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x3102f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3103f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3104f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3105f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3106f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3107f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3108f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x3109f6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310af6 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x31023e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31033e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31043e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31053e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31063e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31073e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31083e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x31093e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x310a3e << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x1615e6 << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x1622e6 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x1012fe << 1)), 0xa000

    bl          Miu0_BistTest

    //sw init done release
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x8c08
    WREG        (MS_BUS_RIU_ADDR + (0x10061e << 1)), 0x8c08
#else
    DPUTK_STR   MiuBoardType
    DPUTK_STR   MiuDate
#if !defined(CONFIG_MSTAR_HAPS)
#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r0, =MIU0_Ver
    bl          MDrv_WriteRegTbl32
#endif
#if !defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r0, =MIU1_Ver
    bl          MDrv_WriteRegTbl32
#endif
#endif

    //Disable the function that PM51 is blocked from accessing DIE domain riu & miu
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x3900 << 1))
    ldr         r0, [r1, #(0x64 << 1)]
    orr         r0, #0x01
    str         r0, [r1, #(0x64 << 1)]

    //PM 51 can access nonPM RIU
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x123900 << 1))
    ldr         r0, [r1, #(0x60 << 1)]
    orr         r0, #(0x01 << 2)
    str         r0, [r1, #(0x60 << 1)]

    bl          MIU_Arbiter_Reset_Block

#if defined(CONFIG_MSTAR_HAPS)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0002
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
    ldr         r0, =0x5000
    str         r0, [r1, #(0x28 << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
    ldr         r0, =0x5001
    str         r0, [r1, #(0x28 << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, =0x03ff
    str         r0, [r1, #(0xac << 1)]
#else
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0002
    str         r0, [r1, #(0x26 << 1)]
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
    ldr         r0, =0x1000
    str         r0, [r1, #(0x28 << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0031
    str         r0, [r1, #(0x2a << 1)]
    ldr         r0, =0x1001
    str         r0, [r1, #(0x28 << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, =0x01ff
    str         r0, [r1, #(0xac << 1)]
#endif
    ldr         r0, =MIU_PreInit
    bl          MDrv_WriteRegTbl32

    DELAYUS     4000

#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110d00 << 1))

#ifdef CONFIG_ONEBIN_ENABLE
    bl          Get_Packge_CFG @put result to r1
    ldr         r0, =MIUBuffer
    ldr         r2, =MIU0_Pre
    bl          drv_LoadTbl
    ldr         r0, =MIUBuffer
#else
    ldr         r0, =MIU0_DDR_PreInit
#endif
    bl          MDrv_WriteRegTbl32

    DELAYUS     1000

#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161600 << 1))

#ifdef CONFIG_ONEBIN_ENABLE
    bl          Get_Packge_CFG @put result to r1
    ldr         r0, =MIUBuffer
    ldr         r2, =MIU1_Pre
    bl          drv_LoadTbl
    ldr         r0, =MIUBuffer
#else
    ldr         r0, =MIU1_DDR_PreInit
#endif
    bl          MDrv_WriteRegTbl32

    DELAYUS     1000
#endif

    DELAYUS     10000

#if !defined(CONFIG_MIU0_DRAM_NONE)

#ifdef CONFIG_ONEBIN_ENABLE
    bl          Get_Packge_CFG @put result to r1
    ldr         r0, =MIUBuffer
    ldr         r2, =MIU0
    bl          drv_LoadTbl
    ldr         r0, =MIUBuffer
#else
    ldr         r0, =MIU0_DDR_Init
#endif
    bl          MDrv_WriteRegTbl32

#endif /* CONFIG_MIU0_DRAM_NONE */

#if !defined(CONFIG_MIU1_DRAM_NONE)

#ifdef CONFIG_ONEBIN_ENABLE
    bl          Get_Packge_CFG @put result to r1
    ldr         r0, =MIUBuffer
    ldr         r2, =MIU1
    bl          drv_LoadTbl
    ldr         r0, =MIUBuffer
#else
    ldr         r0, =MIU1_DDR_Init
#endif
    bl          MDrv_WriteRegTbl32

#endif /* CONFIG_MIU1_DRAM_NONE */

    bl          MIU_Secure_setting
    bl          MIU_Arbiter_Setting
    bl          Miu_Improve_Efficiency
    bl          Release_Sw_Reset
    bl          MIU_Post_inivec

    DELAYUS     200                         //delay 200us

#if defined(CONFIG_MSTAR_HAPS)
    WREG        (MS_BUS_RIU_ADDR + (0x152b14 << 1)), 0x0001
    WREG        (MS_BUS_RIU_ADDR + (0x152b36 << 1)), 0x0011
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    //MIU1 INIT Cycle
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    ldr         r0, =0x0000                 //DRAM reset
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     200                         //delay 200us

    ldr         r0, =0x0008                 //DRAM reset
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     1                           //delay 1us

    ldr         r0, =0x000c                 //DRAM reset+CS
    str         r0, [r1, #(0x00 << 1)]

#if defined(CONFIG_MSTAR_HAPS)
    DELAYUS     1000
#else
    DELAYUS     500
#endif

    ldr         r0, =0x000e                 //DRAM reset+CS+CKE
    str         r0, [r1, #(0x00 << 1)]

#if defined(CONFIG_MSTAR_HAPS)
    DELAYUS     100                           //delay 1us
#else
    DELAYUS     1                           //delay 1us
#endif

    ldr         r0, =0x000e                 //DRAM reset+CS+CKE
    str         r0, [r1, #(0x00 << 1)]

#if defined(CONFIG_MSTAR_HAPS)
    DELAYUS     100                           //delay 1us
#else
    DELAYUS     1                           //delay 1us
#endif

    ldr         r0, =0x001f                 //DRAM reset+CS+CKE+auto cycle+ODE
    str         r0, [r1, #(0x00 << 1)]
    DELAYUS     1000                           //delay 1us

#endif /* CONFIG_MIU1_DRAM_NONE */

    WREG        (MS_BUS_RIU_ADDR + (0x152b14 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x152b14 << 1)), 0x0001
    WREG        (MS_BUS_RIU_ADDR + (0x152b36 << 1)), 0x0020

#if !defined(CONFIG_MIU0_DRAM_NONE)
    //MIU0 INIT Cycle
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, =0x0000                 //DRAM reset
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     200                         //delay 200us

    ldr         r0, =0x0008                 //DRAM reset
    str         r0, [r1, #(0x00 << 1)]

    DELAYUS     1                           //delay 1us

    ldr         r0, =0x000c                 //DRAM reset+CS
    str         r0, [r1, #(0x00 << 1)]

#if defined(CONFIG_MSTAR_HAPS)
    DELAYUS     1000
#else
    DELAYUS     500
#endif

    ldr         r0, =0x000e                 //DRAM reset+CS+CKE
    str         r0, [r1, #(0x00 << 1)]

#if defined(CONFIG_MSTAR_HAPS)
    DELAYUS     100                           //delay 1us
#else
    DELAYUS     1                           //delay 1us
#endif

    ldr         r0, =0x001f                 //DRAM reset+CS+CKE+auto cycle+ODE
    str         r0, [r1, #(0x00 << 1)]
    DELAYUS     1000

#endif /* CONFIG_MIU0_DRAM_NONE */

#if defined(CONFIG_MSTAR_HAPS)
    WREG        (MS_BUS_RIU_ADDR + (0x152b14 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x152b36 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d14 << 1)), 0x0e00
    WREG        (MS_BUS_RIU_ADDR + (0x161614 << 1)), 0x0e00
#endif
    DELAYUS     10000

#if !defined(CONFIG_MIU0_DRAM_NONE)
    // wait for miu0 init done only, for UMA mode
CheckMiu0Init:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, [r1, #(0x00 << 1)]
    and         r0, r0, #0x8000
    cmp         r0, #0
    beq         CheckMiu0Init

#if defined(CONFIG_MSTAR_HAPS)
    WREG        (MS_BUS_RIU_ADDR + (0x152b36 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d14 << 1)), 0x0e00
#if !defined(CONFIG_MIU1_DRAM_NONE)
    WREG        (MS_BUS_RIU_ADDR + (0x152c36 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161614 << 1)), 0x0e00
#endif
#endif
#endif

#if defined(CONFIG_ENABLE_MIU0_AUTO_DQS)
#if !defined(CONFIG_MIU0_DRAM_NONE)

    //Scan MIU0 DQS
    ldr         r0, =0
    bl          BootRom_MiuDdrDqs

#endif //CONFIG_MIU0_DRAM_NONE
#endif

    DELAYUS     800                                         //delay 800us

    bl          MIU_Unmask_Bist

#if !defined(CONFIG_MIU0_DRAM_NONE)
    //BIST test
    bl          Miu0_BistTest
#endif

#if defined(CONFIG_MSTAR_HAPS)
    //sw init done release
    WREG        (MS_BUS_RIU_ADDR + (0x10121e << 1)), 0x8008
#endif

    //Disable BIST
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, [r12, #(0xfe << 1)]
    orr         r0, r0, #(0x01 << 11)
    str         r0, [r12, #(0xfe << 1)]

    ldr         r0, =MIU_PostInit
    bl          MDrv_WriteRegTbl32
#endif

    // Set DRAM size for OPTEE tsp used
#if !defined(CONFIG_MIU0_DRAM_NONE)
    WREG        (MS_BUS_RIU_ADDR + (0x1012d2 << 1)), 0xB000
#endif

#ifdef CONFIG_MSTAR_MOBF_ENABLE
    bl          BootRom_InitMobf_Miu
#endif

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && defined(CONFIG_MBOOT_IN_SPI_FLASH)
    mov         lr, r8
#else
    pop         {lr}
#endif
    bx          lr

//-------------------------------------------------------------------------------------------------
// MIU_Arbiter_Reset_Block
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Arbiter_Reset_Block:
    //BLOCK ARBITER RESET & CLK_ENABLE
    WREG        (MS_BUS_RIU_ADDR + (0x3102ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3103ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3104ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3105ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3106ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3107ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3108ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3109ae << 1)), 0xd000
    WREG        (MS_BUS_RIU_ADDR + (0x3110ae << 1)), 0xd000

    WREG        (MS_BUS_RIU_ADDR + (0x10129c << 1)), 0x0001

    WREG        (MS_BUS_RIU_ADDR + (0x10124c << 1)), 0x03ff
    WREG        (MS_BUS_RIU_ADDR + (0x10124e << 1)), 0x03ff
    WREG        (MS_BUS_RIU_ADDR + (0x101250 << 1)), 0x03ff

    bx          lr

//-------------------------------------------------------------------------------------------------
// MIU_Arbiter_Setting
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Arbiter_Setting:

    //BLOCK ARBITER Release Reset
    WREG        (MS_BUS_RIU_ADDR + (0x10124e << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x101250 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x10124c << 1)), 0x0000

#if defined(CONFIG_BLOCK_MCM_ENABLE)
    WREG        (MS_BUS_RIU_ADDR + (0x3102ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3103ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3104ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3105ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3106ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3107ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3108ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3109ae << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x3110ae << 1)), 0xc000
#endif

    //Block Arbiter setting
    WREG        (MS_BUS_RIU_ADDR + (0x310212 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310312 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310412 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310512 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310612 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310712 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310812 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310912 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310a12 << 1)), 0xffff
    WREG        (MS_BUS_RIU_ADDR + (0x310230 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310200 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310330 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310300 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310430 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310400 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310530 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310500 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310630 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310600 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310730 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310700 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310830 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310800 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310930 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310900 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310a30 << 1)), 0x0011
    WREG        (MS_BUS_RIU_ADDR + (0x310a00 << 1)), 0x0303
    WREG        (MS_BUS_RIU_ADDR + (0x310844 << 1)), 0x0003

    //CTRL Arbiter setting
    WREG        (MS_BUS_RIU_ADDR + (0x161504 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161506 << 1)), 0x7ffe
    WREG        (MS_BUS_RIU_ADDR + (0x16150a << 1)), 0xffff

    bx          lr

//-------------------------------------------------------------------------------------------------
// Release_Sw_Reset
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
Release_Sw_Reset:

#if !defined(CONFIG_MIU1_DRAM_NONE)
    //MIU1
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    ldr         r0, =0x8c00
    str         r0, [r1, #(0x1e << 1)]
#endif

#if !defined(CONFIG_MIU0_DRAM_NONE)
    //MIU0
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, =0x8c00
    str         r0, [r1, #(0x1e << 1)]
#endif
    bx          lr

//-------------------------------------------------------------------------------------------------
// MIU_Secure_setting
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Secure_setting:

#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0000
    str         r0, [r1, #(0xc8 << 1)]
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0000
    str         r0, [r1, #(0xc8 << 1)]
#endif

#if defined(CONFIG_MIU0_DRAM_NONE) || defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113b00 << 1))
    ldr         r0, =0xc000
    str         r0, [r1, #(0xec << 1)]
#endif

#if !defined(CONFIG_MIU0_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x0011
    str         r0, [r1, #(0xc8 << 1)]
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x0011
    str         r0, [r1, #(0xc8 << 1)]
#endif
    bx          lr
    
//-------------------------------------------------------------------------------------------------
// MIU_Post_inivec
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Post_inivec:

#if !defined(CONFIG_MIU0_DRAM_NONE)

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110d00 << 1))
    ldr         r0, [r1, #(0x00 << 1)]
    ldr         r2, =0xFFF8             // BIT[2:0] (mapping setting need keep original value)
    bic         r0, r0, r2
    ldr         r2, =0x2010
    orr         r3, r0, r2
    str         r3, [r1, #(0x00 << 1)]
    ldr         r2, =0x0000
    orr         r3, r0, r2
    str         r3, [r1, #(0x00 << 1)]

    WREG        (MS_BUS_RIU_ADDR + (0x110d18 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d06 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x110d7c << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x110d54 << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x110d1e << 1)), 0x0005

#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161600 << 1))
    ldr         r0, [r1, #(0x00 << 1)]
    ldr         r2, =0xFFF8             // BIT[2:0] (mapping setting need keep original value)
    bic         r0, r0, r2
    ldr         r2, =0x2010
    orr         r3, r0, r2
    str         r3, [r1, #(0x00 << 1)]
    ldr         r2, =0x0000
    orr         r3, r0, r2
    str         r3, [r1, #(0x00 << 1)]

    WREG        (MS_BUS_RIU_ADDR + (0x161618 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x161606 << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x16167c << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x161654 << 1)), 0xc000
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0005
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x000f
    WREG        (MS_BUS_RIU_ADDR + (0x16161e << 1)), 0x0005

#endif
    bx          lr

//-------------------------------------------------------------------------------------------------
// MIU_Unmask_Bist
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
MIU_Unmask_Bist:

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161500 << 1))
    ldr         r0, =0x7ffe
    str         r0, [r1, #(0xe6 << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110d00 << 1))
    ldr         r0, =0xc114
    str         r0, [r1, #(0x2c << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r0, =0xa000
    str         r0, [r1, #(0xfe << 1)]
//-------------------------------------------------------------------------------------------------
// Miu_Improve_Efficiency
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
Miu_Improve_Efficiency:

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101800 << 1))
    ldr         r0, =0x00
    strb        r0, [r1, #((0xc0 << 1) + 1)]

    //------------------
    // PRE_DTOP SETTING
    //------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x161500 << 1))
    ldr         r0, =0x1001
    str         r0, [r1, #(0xfe << 1)]

    ldr         r0, =0x2848
    str         r0, [r1, #(0x96 << 1)]

    ldr         r0, =0x0005
    str         r0, [r1, #(0x9a << 1)]

    ldr         r0, =0x7ffe
    str         r0, [r1, #(0x98 << 1)]

#if !defined(CONFIG_MIU0_DRAM_NONE)
    //------------------
    // DTOP CH0 SETTING
    //------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152b00 << 1))
    ldr         r0, =0x1800
    str         r0, [r1, #(0x88 << 1)]

    ldr         r0, =0xff12
    str         r0, [r1, #(0x8a << 1)]

    ldr         r0, =0x000a
    str         r0, [r1, #(0xdc << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    //ldr         r0, =0x000f
    //str         r0, [r1, #(0x2e << 1)]

    ldr         r0, =0xc804
    str         r0, [r1, #(0x30 << 1)]

    ldr         r0, =0xce52
    str         r0, [r1, #(0xaa << 1)]

    ldr         r0, =0x0007
    str         r0, [r1, #(0xac << 1)]
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    //------------------
    // DTOP CH1 SETTING
    //------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x152c00 << 1))
    ldr         r0, =0x1800
    str         r0, [r1, #(0x88 << 1)]

    ldr         r0, =0xff12
    str         r0, [r1, #(0x8a << 1)]

    ldr         r0, =0x000a
    str         r0, [r1, #(0xdc << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    //ldr         r0, =0x000f
    //str         r0, [r1, #(0x2e << 1)]

    ldr         r0, =0xc804
    str         r0, [r1, #(0x30 << 1)]

    ldr         r0, =0xce52
    str         r0, [r1, #(0xaa << 1)]

    ldr         r0, =0x0007
    str         r0, [r1, #(0xac << 1)]
#endif
    bx          lr
//-------------------------------------------------------------------------------------------------
// MiuBistTest
// param
// return None
// note
//-------------------------------------------------------------------------------------------------
#if !defined(CONFIG_MIU1_DRAM_NONE)
    .global     Miu1_BistTest
Miu1_BistTest:

    DPUTK_STR   MIU_MESSAGE_BIST1
    // disable miu protect
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x301b00 << 1))
    ldr         r11, =0x0000
    str         r11, [r12]
    str         r11, [r12, #(0x02 << 1)]

    //unmask miu request
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    ldr         r11, =0x7FFE
    str         r11, [r12, #(0xe6 << 1)]

    //reset bist test
    ldr         r11, =0x0000
    str         r11, [r12, #(0xe0 << 1)]

    b           Miu_BistTest
#endif

    .global     Miu0_BistTest
Miu0_BistTest:

    DPUTK_STR   MIU_MESSAGE_BIST0

    //disable MIU protect
    ldr         r12, =(MS_BUS_RIU_ADDR + (0x301a00 << 1))
    ldr         r11, =0x0000
    str         r11, [r12]
    str         r11, [r12, #(0x02 << 1)]

    ldr         r12, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r11, =0x7FFE
    str         r11, [r12, #(0xe6 << 1)]

    //reset bist test
    ldr         r11, =0x0000
    str         r11, [r12, #(0xe0 << 1)]

    .global     Miu_BistTest
Miu_BistTest:

    ldr         r11, =0x0000
    str         r11, [r12, #(0xe2 << 1)]    //start address base unit: 16k bytes

    // unit: 32k bytes
    ldr         r0, =0xFFFF
    ldr         r11, =BIST_LEN
    and         r11, r11, r0
    str         r11, [r12, #(0xe4 << 1)]
    ldr         r11, =BIST_LEN
    lsr         r11, #16
    and         r11, r11, r0
    str         r11, [r12, #(0xe6 << 1)]

    ldr         r11, =0x5aa5
    str         r11, [r12, #(0xe8 << 1)]    //bist test pattern

    ldr         r11, =0x0003                //bist test mode:3
    str         r11, [r12, #(0xe0 << 1)]

Miu_CheckBistLoop:
    ldr         r1, [r12, #(0xe0 << 1)]
    and         r0, r1, #0x8000
    cmp         r0, #0
    beq         Miu_CheckBistLoop

    and         r0, r1, #0x6000
    cmp         r0, #0
    beq         Miu_Bist_OK

Miu_Bist_Fail:
    DPUTK_STR   MIU_MESSAGE_FAIL

Miu_Bist_FailLoop:
    b           Miu_Bist_FailLoop

Miu_Bist_OK:
    DPUTK_STR   MIU_MESSAGE_OK

Miu_Bist_Exit:

    bx          lr

#ifdef CONFIG_MSTAR_MOBF_ENABLE
//-------------------------------------------------------------------------------------------------
// BootRom_InitMobf_Miu
// @param
// @return None
// @note
//-------------------------------------------------------------------------------------------------
    .global     BootRom_InitMobf_Miu
BootRom_InitMobf_Miu:

Obf_CheckTRNGLoop:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113D00 << 1))
    ldr         r2, [r1, #(0x06 << 1)]
    and         r2, r2, #0x0002
    cmp         r0, #0
    beq         Obf_CheckTRNGLoop

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113B00 << 1))
    ldr         r2, [r1, #(0xEC << 1)]
    bic         r2, r2, #0x0060
    str         r2, [r1, #(0xEC << 1)]

    ldr         r3, =0xC000
    orr         r2, r2, r3
    str         r2, [r1, #(0xEC << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113B00 << 1))
    ldr         r2, [r1, #(0xEC << 1)]
    ldr         r3, =0x1000
    orr         r2, r2, r3
    str         r2, [r1, #(0xEC << 1)]

    // Lock
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113B00 << 1))
    ldr         r2, [r1, #(0xEC << 1)]
    ldr         r3, =0x0400
    orr         r2, r2, r3
    str         r2, [r1, #(0xEC << 1)]

Obf_CheckMiu0Loop:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101200 << 1))
    ldr         r2, [r1, #(0x2A << 1)]
    and         r2, r2, #0x8000
    cmp         r0, #0
    beq         Obf_CheckMiu0Loop

Obf_CheckMiu1Loop:
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100600 << 1))
    ldr         r2, [r1, #(0x2A << 1)]
    and         r2, r2, #0x8000
    cmp         r0, #0
    beq         Obf_CheckMiu1Loop

    bx          lr
#endif

    .ltorg
    .size       BootRom_InitMiu,.-BootRom_InitMiu
