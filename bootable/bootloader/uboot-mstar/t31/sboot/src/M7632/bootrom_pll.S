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
    .global     BootRom_InitPll

BootRom_InitPll:

    //------------------------------------------------------------------------------------
    // IP: ADC
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x102500 << 1))

    ldr         r0, =0x0000
    str         r0, [r1, #(0x0c << 1)]

    //------------------------------------------------------------------------------------
    // IP: DMPLL(replace MPLL)
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x002e00 << 1))
    ldr         r0, =0x00
    strb        r0, [r1, #(0xe8 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0xe8 << 1) + 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x112800 << 1))
    ldr         r0, =0x01
    strb        r0, [r1, #(0x66 << 1)]

    ldr         r0, =0x12
    strb        r0, [r1, #((0x66 << 1) + 1)]

    ldr         r0, =0x80
    strb        r0, [r1, #(0x60 << 1)]

    ldr         r0, =0x24
    strb        r0, [r1, #((0x60 << 1) + 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #(0x86 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0x86 << 1) + 1)]

    ldr         r0, =0x03
    strb        r0, [r1, #(0x6a << 1)]

    ldr         r0, =0x06
    strb        r0, [r1, #((0x6a << 1) + 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #(0x40 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0x40 << 1) + 1)]

    //------------------------------------------------------------------------------------
    // IP: MIU128BUSPLL
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110c00 << 1))
    ldr         r0, =0x1a
    strb        r0, [r1, #(0x66 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0x62 << 1) + 1)]

    //------------------------------------------------------------------------------------
    // IP: MIU256BUSPLL
    //------------------------------------------------------------------------------------
    ldr         r0, =0x00
    strb        r0, [r1, #((0x82 << 1) + 1)]

    //------------------------------------------------------------------------------------
    // IP: SCPLL
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110b00 << 1))
    ldr         r0, =0xf3
    strb        r0, [r1, #(0xca << 1)]

    ldr         r0, =0x97
    strb        r0, [r1, #((0xca << 1) + 1)]

    ldr         r0, =0x2c
    strb        r0, [r1, #(0xcc << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0xcc << 1) + 1)]

    ldr         r0, =0xb4
    strb        r0, [r1, #(0xce << 1)]

    ldr         r0, =0xe5
    strb        r0, [r1, #(0xd2 << 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #((0xd2 << 1) + 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #(0xda << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0xda << 1) + 1)]

    ldr         r0, =0x01
    strb        r0, [r1, #(0xe8 << 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0xe8 << 1) + 1)]

    ldr         r0, =0x00
    strb        r0, [r1, #((0xc2 << 1) + 1)]

    //------------------------------------------------------------------------------------
    // IP: ARMPLL
    //------------------------------------------------------------------------------------
    ldr         r1, =(MS_BUS_RIU_ADDR + (0x110c00 << 1))

#ifdef CONFIG_MSTAR_DVFS_ENABLE
    ldr         r0, =(CONFIG_DVFS_CPU_CLOCK_INIT & 0xFFFF)
    str         r0, [r1, #(0xc0 << 1)]
    ldr         r0, =((CONFIG_DVFS_CPU_CLOCK_INIT >> 16) & 0xFFFF)
    str         r0, [r1, #(0xc2 << 1)]
#else
    ldr         r0, =(CONFIG_CPU_CLOCK & 0xFFFF)
    str         r0, [r1, #(0xc0 << 1)]
    ldr         r0, =((CONFIG_CPU_CLOCK >> 16) & 0xFFFF)
    str         r0, [r1, #(0xc2 << 1)]
#endif

    ldr         r0, =0x01
    strb        r0, [r1, #(0xc4 << 1)]

    ldr         r0, =0x02
    strb        r0, [r1, #((0x22 << 1) + 1)]

    ldr         r1, =(MS_BUS_RIU_ADDR + (0x100500 << 1))
    ldr         r0, =CPU_CLOCK
    str         r0, [r1, #(0x1e << 1)]

#ifndef CONFIG_BOOT_SEQUENCE_FORCE_USB_UPGRADE_LOADER
    //----------------------------------------------------------
    // UPLL0
    //----------------------------------------------------------
    //----------------------------------------------------------
    // UPLL0 clock flow
    //----------------------------------------------------------
    RREG        r0, (MS_BUS_RIU_ADDR + (0x100800 << 1))
    ldr         r1, =(0x1 << 1)                                //UPLL0_0x00[1] <- 0 (enable
    bic         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x100800 << 1))

    //----------------------------------------------------------
    // UPLL0 sub-CLKs for other IPs
    //----------------------------------------------------------
    RREG        r0, (MS_BUS_RIU_ADDR + (0x100800 << 1))
    ldr         r1, =0x0000
    orr         r1, r1, #(0x1 << 4)                            //UPLL0_0x00[4] <- 0 (enable 20MHz clock)
    orr         r1, r1, #(0x1 << 5)                            //UPLL0_0x00[5] <- 0 (enable 320MHz clock)
    bic         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x100800 << 1))

    RREG        r0, (MS_BUS_RIU_ADDR + (0x10080E << 1))
    ldr         r1, =0x0000
    orr         r1, r1, #(0x1 << 0)                             //UPLL0_0x0e[0] <- 1 (enable 384MHz clock)
    orr         r1, r1, #(0x1 << 1)                             //UPLL0_0x0e[1] <- 1 (enable UPLL_CLK_ANA2)
    orr         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x10080E << 1))

    //----------------------------------------------------------
    // UPLL1
    //----------------------------------------------------------
    //----------------------------------------------------------
    // UPLL1 clock flow
    //----------------------------------------------------------
    RREG        r0, (MS_BUS_RIU_ADDR + (0x011100 << 1))
    ldr         r1, =(0x1 << 1)                                //UPLL1_0x00[1] <- 0 (enable
    bic         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x011100 << 1))

    RREG        r0, (MS_BUS_RIU_ADDR + (0x01110E << 1))
    ldr         r1, =(0x1 << 1)                                //UPLL1_0x0e[1] <- 0 (disable UPLL_CLK_ANA2)
    bic         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x01110E << 1))

//================================================================================================
    WREG        (MS_BUS_RIU_ADDR + (0x103A88 << 1)), 0x040F     // UTMI0
    WREG        (MS_BUS_RIU_ADDR + (0x103A80 << 1)), 0x7F05
    WREG_B      (MS_BUS_RIU_ADDR + (0x103AD2 << 1)), 0x00       //overwrite off

    WREG        (MS_BUS_RIU_ADDR + (0x103A08 << 1)), 0x040F     // UTMI1
    WREG        (MS_BUS_RIU_ADDR + (0x103A00 << 1)), 0x7F05
    WREG_B      (MS_BUS_RIU_ADDR + (0x103A52 << 1)), 0x00       //overwrite off

    WREG        (MS_BUS_RIU_ADDR + (0x011488 << 1)), 0x040F	// UTMI2
    WREG        (MS_BUS_RIU_ADDR + (0x011480 << 1)), 0x7F05
    WREG_B      (MS_BUS_RIU_ADDR + (0x0114D2 << 1)), 0x00       //overwrite off

//================================================================================================                                                                //initial USBPLL
                                                                //Module: usb0 (0x100700)
    WREG_B      (MS_BUS_RIU_ADDR + (0x100700 << 1)), 0x0a       //Disable MAC initial suspend, Reset UHC
    WREG_B      (MS_BUS_RIU_ADDR + (0x100700 << 1)), 0x28       //Release UHC reset, enable UHC and OTG XIU function
                                                                //Module: utmi0 (0x103A80)
    WREG        (MS_BUS_RIU_ADDR + (0x103AA2 << 1)), 0x2088     //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    WREG        (MS_BUS_RIU_ADDR + (0x103AA0 << 1)), 0x8051     //PLL_TEST[15]: Bypass 480MHz clock divider
    WREG        (MS_BUS_RIU_ADDR + (0x103A82 << 1)), 0x2084     //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    WREG        (MS_BUS_RIU_ADDR + (0x103A88 << 1)), 0x0426     //bit<7>: Power down UTMI port-0 bandgap current

    WREG        (MS_BUS_RIU_ADDR + (0x103A80 << 1)), 0x6bc3     //reg_pdn: bit<15>, bit <2> ref_pdn # Turn on reference voltage and regulator
    WREG_B      (MS_BUS_RIU_ADDR + (0x103AD2 << 1)), 0x3f       //overwrite on
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x103A80 << 1)), 0x69c3     //Turn on UPLL, reg_pdn: bit<9>
    WREG_B      (MS_BUS_RIU_ADDR + (0x103AD2 << 1)), 0x3f       //overwrite on
    DELAYUS     2000

    WREG        (MS_BUS_RIU_ADDR + (0x103A80 << 1)), 0x0001     //Disable dp/dm pull-down override mode
    WREG_B      (MS_BUS_RIU_ADDR + (0x103AD2 << 1)), 0x00       //overwrite off

//================================================================================================
                                                                //Module: usb1 (0x100780)
    WREG_B      (MS_BUS_RIU_ADDR + (0x100780 << 1)), 0x0a       //Disable MAC initial suspend, Reset UHC
    WREG_B      (MS_BUS_RIU_ADDR + (0x100780 << 1)), 0x28       //Release UHC reset, enable UHC and OTG XIU function
                                                                //Module: utmi1 (0x103A00)
    WREG        (MS_BUS_RIU_ADDR + (0x103A22 << 1)), 0x2088     //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    WREG        (MS_BUS_RIU_ADDR + (0x103A20 << 1)), 0x8051     //PLL_TEST[15]: Bypass 480MHz clock divider
    WREG        (MS_BUS_RIU_ADDR + (0x103A02 << 1)), 0x2084     //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    WREG        (MS_BUS_RIU_ADDR + (0x103A08 << 1)), 0x0426     //bit<7>: Power down UTMI port-0 bandgap current

    WREG        (MS_BUS_RIU_ADDR + (0x103A00 << 1)), 0x6bc3     //reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    WREG_B      (MS_BUS_RIU_ADDR + (0x103A52 << 1)), 0x3f       //overwrite on
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x103A00 << 1)), 0x69c3     //Turn on UPLL, reg_pdn: bit<9>
    WREG_B      (MS_BUS_RIU_ADDR + (0x103A52 << 1)), 0x3f       //overwrite on
    DELAYUS     2000

    WREG        (MS_BUS_RIU_ADDR + (0x103A00 << 1)), 0x0001     //Disable dp/dm pull-down override mode
    WREG_B      (MS_BUS_RIU_ADDR + (0x103A52 << 1)), 0x00       //overwrite off

//================================================================================================
// non-die domain port, no need to reset UHC
                                                                //Module: usb2 (0x011200)
//    WREG_B      (MS_BUS_RIU_ADDR + (0x011200 << 1)), 0x0a       //Disable MAC initial suspend, Reset UHC
//    WREG_B      (MS_BUS_RIU_ADDR + (0x011200 << 1)), 0x28       //Release UHC reset, enable UHC and OTG XIU function
                                                                //Module: utmi2 (0x011480)
    WREG        (MS_BUS_RIU_ADDR + (0x0114A2 << 1)), 0x2088     //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    WREG        (MS_BUS_RIU_ADDR + (0x0114A0 << 1)), 0x8051     //PLL_TEST[15]: Bypass 480MHz clock divider
    WREG        (MS_BUS_RIU_ADDR + (0x011482 << 1)), 0x2084     //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    WREG        (MS_BUS_RIU_ADDR + (0x011488 << 1)), 0x0426     //bit<7>: Power down UTMI port-0 bandgap current

    WREG        (MS_BUS_RIU_ADDR + (0x011480 << 1)), 0x6bc3     //reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    WREG_B      (MS_BUS_RIU_ADDR + (0x0114D2 << 1)), 0x3f       //overwrite on
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x011480 << 1)), 0x69c3     //Turn on UPLL, reg_pdn: bit<9>
    WREG_B      (MS_BUS_RIU_ADDR + (0x0114D2 << 1)), 0x3f       //overwrite on
    DELAYUS     2000

    WREG        (MS_BUS_RIU_ADDR + (0x011480 << 1)), 0x0001     //Turn dp/dm pull-down use override mode
    WREG_B      (MS_BUS_RIU_ADDR + (0x0114D2 << 1)), 0x00       //overwrite off

//================================================================================================
    RREG        r0, (MS_BUS_RIU_ADDR + (0x102440 << 1))
    ldr         r1, =0x0000
    orr         r1, r1, #(0x1 << 3)                             //USB0, Set INT_POLARITY to Active HIGH
    orr         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x102440 << 1))

    RREG        r0, (MS_BUS_RIU_ADDR + (0x100D40 << 1))
    ldr         r1, =0x0000
    orr         r1, r1, #(0x1 << 3)                             //USB1, Set INT_POLARITY to Active HIGH
    orr         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x100D40 << 1))

    RREG        r0, (MS_BUS_RIU_ADDR + (0x11340 << 1))
    ldr         r1, =0x0000
    orr         r1, r1, #(0x1 << 3)                             //USB2, Set INT_POLARITY to Active HIGH
    orr         r0, r0, r1
    REG_C2M     r0, (MS_BUS_RIU_ADDR + (0x11340 << 1))
//================================================================================================
#endif
    bx          lr

    .size       BootRom_InitPll,.-BootRom_InitPll

    .end
