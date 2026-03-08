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

    .text
    .global     BootRom_InitClock

BootRom_InitClock:

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && defined(CONFIG_MBOOT_IN_SPI_FLASH)
    mov         r8, lr
#else
    push        {lr}
#endif

#if defined(CONFIG_MSTAR_HAPS)
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101d00 << 1))
    ldr         r0, =0x01
    strb        r0, [r1, #(0xf8 << 1)]                  //Set CPU clock MUX to ARMPLL clk
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101800 << 1))
    ldr         r0, =0x80
    strb        r0, [r1, #(0x82 << 1)]
    ldr         r0, =0x01
    strb        r0, [r1, #(0x84 << 1)]
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101d00 << 1))
    ldr         r0, =0x03
    strb        r0, [r1, #(0x02 << 1)]
#else
    //------------------------------------------------------------------------------------
    // IP: CPU
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x101d00 << 1))
    ldr         r0, =0x01
    strb        r0, [r1, #(0xf8 << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_mcu, uart, spi
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000e00 << 1))
    ldr         r0, =0x8438
    str         r0, [r1, #(0x40 << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100b00 << 1))
    ldr         r0, =0x00
    strb        r0, [r1, #((0x20 << 1) + 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #(0x20 << 1)]

    ldr         r0, =0x0c
    strb        r0, [r1, #((0x26 << 1) + 1)]

    ldr         r0, =0x0c
    strb        r0, [r1, #(0x28 << 1)]

    ldr         r0, =0x0c
    strb        r0, [r1, #((0x28 << 1) + 1)]

    ldr         r0, =0x0c
    strb        r0, [r1, #(0x32 << 1)]

    ldr         r0, =0x0c
    strb        r0, [r1, #((0x32 << 1) + 1)]

    ldr         r0, =0x04
    strb        r0, [r1, #(0x2c << 1)]

    ldr         r0, =0x44
    strb        r0, [r1, #(0x2c << 1)]

    ldr         r0, =0x04
    strb        r0, [r1, #(0x2e << 1)]

    ldr         r0, =0x24
    strb        r0, [r1, #(0x2e << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_miu/clk_miu_256bus
    //------------------------------------------------------------------------------------
    ldr         r0, =0x00
    strb        r0, [r1, #(0x3e << 1)]

    ldr         r0, =0x0c08
    str         r0, [r1, #(0x3a << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_ddr
    //------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------------
    // IP: clk_vd
    //------------------------------------------------------------------------------------
    ldr         r0, =0x00
    strb        r0, [r1, #(0x40 << 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #((0x40 << 1) + 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_vdmcu
    //------------------------------------------------------------------------------------
    ldr         r0, =0x11
    strb        r0, [r1, #(0x42 << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_vd
    //------------------------------------------------------------------------------------
    ldr         r0, =0x01
    strb        r0, [r1, #(0x44 << 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #((0x44 << 1) + 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #(0x46 << 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #((0x46 << 1) + 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_tsp
    //------------------------------------------------------------------------------------
    ldr         r0, =0x11
    strb        r0, [r1, #(0x54 << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_vd_mheg5
    //------------------------------------------------------------------------------------
    ldr         r0, =0x09
    strb        r0, [r1, #(0x60 << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_jpd
    //------------------------------------------------------------------------------------
    ldr         r0, =0x01
    strb        r0, [r1, #((0x6a << 1) + 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_gop
    //------------------------------------------------------------------------------------
    ldr         r0, =0x00
    strb        r0, [r1, #(0x80 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0x80 << 1) + 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #(0x82 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #(0xfe << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0x86 << 1) + 1)]

    ldr         r0, =0x0000
    str         r0, [r1, #(0x8a << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_ge
    //------------------------------------------------------------------------------------
    ldr         r0, =0x00
    strb        r0, [r1, #(0x90 << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_dc
    //------------------------------------------------------------------------------------
    ldr         r0, =0x01
    strb        r0, [r1, #(0x98 << 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #((0x98 << 1) + 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #(0x9a << 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #((0x9a << 1) + 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_sc
    //------------------------------------------------------------------------------------
    ldr         r0, =0x0000
    str         r0, [r1, #(0xa4 << 1)]

    ldr         r0, =0x4000
    str         r0, [r1, #(0xa2 << 1)]

    ldr         r0, =0x0004
    str         r0, [r1, #(0xa6 << 1)]

    ldr         r0, =0x0000
    str         r0, [r1, #(0xaa << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_emac
    //------------------------------------------------------------------------------------
    ldr         r0, =0x00
    strb        r0, [r1, #(0xc0 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0xc0 << 1) + 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #(0xc2 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0xc2 << 1) + 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #(0xc4 << 1)]

    //------------------------------------------------------------------------------------
    // IP: clk_aesdma
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100a00 << 1))
    ldr         r0, =0x00
    strb        r0, [r1, #((0x32 << 1) + 1)]

    ldr         r0, =0x10
    strb        r0, [r1, #((0x32 << 1) + 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x113d00 << 1))
    ldr         r0, =0x00
    strb        r0, [r1, #((0xbe << 1) + 1)]

#if !defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH) && \
    !defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    //------------------------------------------------------------------------------------
    // IP: spi
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000a00 << 1))
    ldr         r0, =0xff
    strb        r0, [r1, #(0xe2 << 1)]

    ldr         r0, =0x0f
    strb        r0, [r1, #((0xe2 << 1) + 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #(0xe4 << 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000e00 << 1))
    ldr         r0, =0x04
    strb        r0, [r1, #((0x40 << 1) + 1)]

    ldr         r0, =0x44
    strb        r0, [r1, #((0x40 << 1) + 1)]

    ldr         r0, =0x80
    strb        r0, [r1, #(0x40 << 1)]

#endif
    //----------------------------------------------------------
    // CLK RTC
    //----------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x000e00 << 1))
    ldr         r0, =0x04
    strb        r0, [r1, #(0x44 << 1)]

#if defined(CONFIG_MSTAR_GPU_MANUAL)
    bl          BootRom_SetGPUClock
#endif
#endif

#if defined(CONFIG_MSTAR_TITANIA_BD_FPGA) && defined(CONFIG_MBOOT_IN_SPI_FLASH)
    mov         lr, r8
#else
    pop         {lr}
#endif
    bx          lr

#if defined(CONFIG_MSTAR_GPU_MANUAL)
@//-------------------------------------------------------------------------------------------------
@// BootRom_SetGPUClock
@// @param
@// @return None
@// @note
@//-------------------------------------------------------------------------------------------------
BootRom_SetGPUClock:
    @//Set GPU Clock
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110800 << 1))
    ldr         r0, =CONFIG_GPU_CLOCK
    ldr         r0, [r1, #(0x8c << 1)]
    ldr         r0, [r1, #(0xd2 << 1)]
    orr         r0, r0, #0x01
    str         r0, [r1, #(0xd2 << 1)]

    bx          lr
#endif
    .end
