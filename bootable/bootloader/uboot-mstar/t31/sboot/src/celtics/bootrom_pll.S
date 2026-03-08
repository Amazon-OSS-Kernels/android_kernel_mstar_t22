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
    #//adc_atop
    WREG_B      (MS_BUS_RIU_ADDR + (0x10250E << 1)), 0x3f       //Release Soft-Reset for ADCA/PLLA/ADCB/PLLB/ATOP/Phase DAC
    WREG_B      (MS_BUS_RIU_ADDR + (0x10250E << 1)), 0x00

    WREG        (MS_BUS_RIU_ADDR + (0x1025FC << 1)), 0x0000     //Power-On LDO
    WREG        (MS_BUS_RIU_ADDR + (0x1025FE << 1)), 0x0000

    WREG        (MS_BUS_RIU_ADDR + (0x102508 << 1)), 0x0000     //Power-On releated blocks in ATOP
    WREG        (MS_BUS_RIU_ADDR + (0x10250A << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x10250C << 1)), 0x0000
    WREG        (MS_BUS_RIU_ADDR + (0x10257E << 1)), 0x0030     //Enable SOG MUX

    WREG_B      (MS_BUS_RIU_ADDR + (0x102590 << 1)), 0x08       //Enable IDAC (Ch1)
    WREG_B      (MS_BUS_RIU_ADDR + (0x102592 << 1)), 0x08       //Enable IDAC (Ch1)
    WREG_B      (MS_BUS_RIU_ADDR + (0x1025A0 << 1)), 0x00       //Enable CVBSO1
    WREG_B      (MS_BUS_RIU_ADDR + (0x1025A4 << 1)), 0x00       //Enable CVBSO2

#//================================================================================================
                                                                //initial MPLL
    WREG        (MS_BUS_RIU_ADDR + (0x101E38 << 1)), 0x0000     //Power-on for PD_3P3_1, PD_3P3_0 and PD_3P3_USB1

    WREG_B      (MS_BUS_RIU_ADDR + (0x110C02 << 1)+1), 0x00     //Module: ana_misc (0x110C)
                                                                //reg_mpll_clk_adc216m_pd = 0
                                                                //reg_mpll_clk_dp432m_pd = 0
    WREG_B      (MS_BUS_RIU_ADDR + (0x110C00 << 1)), 0x00       //reg_mpll_test[4] = 0
    
#//================================================================================================
                                                                //initial ARMPLL
#ifdef CONFIG_MSTAR_DVFS_ENABLE
    WREG        (MS_BUS_RIU_ADDR + (0x110CC0 << 1)), (CONFIG_DVFS_CPU_CLOCK_INIT & 0xFFFF)
    WREG        (MS_BUS_RIU_ADDR + (0x110CC2 << 1)), ((CONFIG_DVFS_CPU_CLOCK_INIT >> 16) & 0xFFFF)
#else
    WREG        (MS_BUS_RIU_ADDR + (0x110CC0 << 1)), (CONFIG_CPU_CLOCK & 0xFFFF)
    WREG        (MS_BUS_RIU_ADDR + (0x110CC2 << 1)), ((CONFIG_CPU_CLOCK >> 16) & 0xFFFF)
#endif
    WREG        (MS_BUS_RIU_ADDR + (0x110CC4 << 1)), 0x0001
    WREG_B      (MS_BUS_RIU_ADDR + (0x110C22 << 1)+1), 0x00     //reg_mipspll_pd = 0

    WREG        (MS_BUS_RIU_ADDR + (0x10051E << 1)), CPU_CLOCK

#//================================================================================================
                                                                //initial MIU128PLL & MIU256PLL
    WREG_B      (MS_BUS_RIU_ADDR + (0x110C66 << 1)), 0x14       //MIUPLL select 480MHz                                                     
    WREG_B      (MS_BUS_RIU_ADDR + (0x110C68 << 1)), 0x11       
    WREG_B      (MS_BUS_RIU_ADDR + (0x110C62 << 1)+1), 0x00     //MIU128BUSPLL power on
    WREG_B      (MS_BUS_RIU_ADDR + (0x110C76 << 1)), 0x14       //MIUPLL select 480MHz
    WREG_B      (MS_BUS_RIU_ADDR + (0x110C78 << 1)), 0x11       
    WREG_B      (MS_BUS_RIU_ADDR + (0x110C72 << 1)+1), 0x00     //MIU256BUSPLL power on   

#//================================================================================================
                                                                //initial EVDPLL
    WREG_B      (MS_BUS_RIU_ADDR + (0x110B84 << 1)+1), 0x01     
    WREG_B      (MS_BUS_RIU_ADDR + (0x110B86 << 1)+1), 0x00     
    WREG_B      (MS_BUS_RIU_ADDR + (0x110B88 << 1)), 0x11     
    WREG_B      (MS_BUS_RIU_ADDR + (0x110B82 << 1)+1), 0x00     

#// initial UPLL ==================================================================================

    WREG_B      (MS_BUS_RIU_ADDR + (0x100800 << 1)), 0xc0       //enable UPLL0 clock
    WREG_B      (MS_BUS_RIU_ADDR + (0x10080E << 1)), 0x01       //enable 384MHz clk from UPLL0
    WREG        (MS_BUS_RIU_ADDR + (0x100804 << 1)), 0x0110     //UPLL0     

#//================================================================================================
                                                                //initial UPLL1 & UTMISS0 & UTMISS1
    WREG_B      (MS_BUS_RIU_ADDR + (0x100820 << 1)), 0xc0       //enable UPLL1 clock
    WREG        (MS_BUS_RIU_ADDR + (0x100824 << 1)), 0x0110     //UPLL1     
    
    WREG        (MS_BUS_RIU_ADDR + (0x103A88 << 1)), 0x040F     // UTMI0
    WREG        (MS_BUS_RIU_ADDR + (0x103A80 << 1)), 0x7F05
    WREG        (MS_BUS_RIU_ADDR + (0x103A08 << 1)), 0x040F	// UTMI1
    WREG        (MS_BUS_RIU_ADDR + (0x103A00 << 1)), 0x7F05
    WREG        (MS_BUS_RIU_ADDR + (0x103908 << 1)), 0x040F	// UTMI2
    WREG        (MS_BUS_RIU_ADDR + (0x103900 << 1)), 0x7F05
    WREG        (MS_BUS_RIU_ADDR + (0x122308 << 1)), 0x040F    	// UTMI-SS0
    WREG        (MS_BUS_RIU_ADDR + (0x122300 << 1)), 0x7F05
    WREG        (MS_BUS_RIU_ADDR + (0x122388 << 1)), 0x040F	// UTMI-SS1
    WREG        (MS_BUS_RIU_ADDR + (0x122380 << 1)), 0x7F05

#//enable miu arbiter ===============================================================================
    WREG_B      (MS_BUS_RIU_ADDR + (0x122521 << 1) - 1), 0x01   //Enable MIU arbiter

#//initial USBPLL ===================================================================================
                                                                //Module: usb0 (0x100700)
    WREG_B      (MS_BUS_RIU_ADDR + (0x100700 << 1)), 0x0a       //Disable MAC initial suspend, Reset UHC
    WREG_B      (MS_BUS_RIU_ADDR + (0x100700 << 1)), 0x28       //Release UHC reset, enable UHC and OTG XIU function

                                                                //Module: utmi0 (0x103A80)
    WREG        (MS_BUS_RIU_ADDR + (0x103AA2 << 1)), 0x2088     //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    WREG        (MS_BUS_RIU_ADDR + (0x103AA0 << 1)), 0x8051     //PLL_TEST[15]: Bypass 480MHz clock divider
    WREG        (MS_BUS_RIU_ADDR + (0x103A82 << 1)), 0x2084     //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    WREG        (MS_BUS_RIU_ADDR + (0x103A88 << 1)), 0x0426     //bit<7>: Power down UTMI port-0 bandgap current

    WREG        (MS_BUS_RIU_ADDR + (0x103A80 << 1)), 0x6bc3     //reg_pdn: bit<15>, bit <2> ref_pdn # Turn on reference voltage and regulator
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x103A80 << 1)), 0x69c3     //Turn on UPLL, reg_pdn: bit<9>
    DELAYUS     2000

    WREG        (MS_BUS_RIU_ADDR + (0x103A80 << 1)), 0x0001     //Turn all (including hs_current) use override mode
    
#//================================================================================================
    
                                                                //Module: usb1 (0x100780)
    WREG_B      (MS_BUS_RIU_ADDR + (0x100780 << 1)), 0x0a       //Disable MAC initial suspend, Reset UHC
    WREG_B      (MS_BUS_RIU_ADDR + (0x100780 << 1)), 0x28       //Release UHC reset, enable UHC and OTG XIU function

                                                                //Module: utmi1 (0x103A00)
    WREG        (MS_BUS_RIU_ADDR + (0x103A22 << 1)), 0x2088     //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    WREG        (MS_BUS_RIU_ADDR + (0x103A20 << 1)), 0x8051     //PLL_TEST[15]: Bypass 480MHz clock divider
    WREG        (MS_BUS_RIU_ADDR + (0x103A02 << 1)), 0x2084     //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    WREG        (MS_BUS_RIU_ADDR + (0x103A08 << 1)), 0x0426     //bit<7>: Power down UTMI port-0 bandgap current

    WREG        (MS_BUS_RIU_ADDR + (0x103A00 << 1)), 0x6bc3     //reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x103A00 << 1)), 0x69c3     //Turn on UPLL, reg_pdn: bit<9>
    DELAYUS     2000

    WREG        (MS_BUS_RIU_ADDR + (0x103A00 << 1)), 0x0001     //Turn all (including hs_current) use override mode

#//================================================================================================
                                                                //Module: usb2 (0x113800)
    WREG_B      (MS_BUS_RIU_ADDR + (0x113800 << 1)), 0x0a       //Disable MAC initial suspend, Reset UHC
    WREG_B      (MS_BUS_RIU_ADDR + (0x113800 << 1)), 0x28       //Release UHC reset, enable UHC and OTG XIU function

                                                                //Module: utmi2 (0x103900)
    WREG        (MS_BUS_RIU_ADDR + (0x103922 << 1)), 0x2088     //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    WREG        (MS_BUS_RIU_ADDR + (0x103920 << 1)), 0x8051     //PLL_TEST[15]: Bypass 480MHz clock divider
    WREG        (MS_BUS_RIU_ADDR + (0x103902 << 1)), 0x2084     //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    WREG        (MS_BUS_RIU_ADDR + (0x103908 << 1)), 0x0426     //bit<7>: Power down UTMI port-0 bandgap current

    WREG        (MS_BUS_RIU_ADDR + (0x103900 << 1)), 0x6bc3     //reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x103900 << 1)), 0x69c3     //Turn on UPLL, reg_pdn: bit<9>
    DELAYUS     2000

    WREG        (MS_BUS_RIU_ADDR + (0x103900 << 1)), 0x0001     //Turn all (including hs_current) use override mode

#//================================================================================================
                                                                //Module: usb3-ss0 (0x153200)                                                                
    WREG_B      (MS_BUS_RIU_ADDR + (0x153200 << 1)), 0x0a       //Disable MAC initial suspend, Reset UHC
    WREG_B      (MS_BUS_RIU_ADDR + (0x153200 << 1)), 0x28       //Release UHC reset, enable UHC and OTG XIU function
                                                                
                                                                //Module: utmi-ss0 (0x122300)
    WREG        (MS_BUS_RIU_ADDR + (0x122322 << 1)), 0x2088     //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    WREG        (MS_BUS_RIU_ADDR + (0x122320 << 1)), 0x8051     //PLL_TEST[15]: Bypass 480MHz clock divid
    WREG        (MS_BUS_RIU_ADDR + (0x122302 << 1)), 0x2084     //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    WREG        (MS_BUS_RIU_ADDR + (0x122308 << 1)), 0x0426     //bit<7>: Power down UTMI port-0 bandgap current

    WREG        (MS_BUS_RIU_ADDR + (0x122300 << 1)), 0x6bc3     //reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x122300 << 1)), 0x69c3     //Turn on UPLL, reg_pdn: bit<9>
    DELAYUS     2000

    WREG        (MS_BUS_RIU_ADDR + (0x122300 << 1)), 0x0001     //Turn all (including hs_current) use override mode

#//================================================================================================                                                                
                                                                //Module: usb3-ss1 (0x153280)                                                                
    WREG_B      (MS_BUS_RIU_ADDR + (0x153280 << 1)), 0x0a       //Disable MAC initial suspend, Reset UHC
    WREG_B      (MS_BUS_RIU_ADDR + (0x153280 << 1)), 0x28       //Release UHC reset, enable UHC and OTG XIU function                                                            
                                                                
                                                                //Module: utmi-ss1 (0x122380)
    WREG        (MS_BUS_RIU_ADDR + (0x1223A2 << 1)), 0x2088     //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    WREG        (MS_BUS_RIU_ADDR + (0x1223A0 << 1)), 0x8051     //PLL_TEST[15]: Bypass 480MHz clock divid
    WREG        (MS_BUS_RIU_ADDR + (0x122382 << 1)), 0x2084     //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    WREG        (MS_BUS_RIU_ADDR + (0x122388 << 1)), 0x0426     //bit<7>: Power down UTMI port-0 bandgap current

    WREG        (MS_BUS_RIU_ADDR + (0x122380 << 1)), 0x6bc3     //reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    DELAYUS     1000

    WREG        (MS_BUS_RIU_ADDR + (0x122380 << 1)), 0x69c3     //Turn on UPLL, reg_pdn: bit<9>
    DELAYUS     2000

    WREG        (MS_BUS_RIU_ADDR + (0x122380 << 1)), 0x0001     //Turn all (including hs_current) use override mode
 
#//================================================================================================
                                                                //initial G3DPLL
    WREG        (MS_BUS_RIU_ADDR + (0x160F04 << 1)), 0x0000     
    WREG        (MS_BUS_RIU_ADDR + (0x160F06 << 1)), 0x0080     
    WREG        (MS_BUS_RIU_ADDR + (0x160F08 << 1)), 0x1015     
    WREG        (MS_BUS_RIU_ADDR + (0x160F0A << 1)), 0x0001

#//================================================================================================

    ret
    .size       BootRom_InitPll,.-BootRom_InitPll

    .end

