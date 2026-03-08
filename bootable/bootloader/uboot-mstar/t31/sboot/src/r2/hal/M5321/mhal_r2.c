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
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "Types.h"
#include "Bits.h"

#include "mhal_r2.h"
#include "mhal_r2_reg.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
#if 0 // Fix coveriry un-used function
static U8 MHal_R2_Read_Reg8(VIRT virtBase, U32 u32Reg)
{
    return *((volatile U8 *)(virtBase + (u32Reg << 1)) - (u32Reg & 1));
}
#endif

static U16 MHal_R2_Read_Reg16(VIRT virtBase, U32 u32Reg)
{
    return *((volatile U16 *)(virtBase + (u32Reg << 1)));
}

static void MHal_R2_Write_Reg8(VIRT virtBase, U32 u32Reg, U8 u8Val)
{
    *((volatile U8 *)(virtBase + (u32Reg << 1) - (u32Reg & 1))) = u8Val;
}

static void MHal_R2_Write_Reg16(VIRT virtBase, U32 u32Reg, U16 u16Val)
{
    *((volatile U16 *)(virtBase + (u32Reg << 1))) = u16Val;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void MHal_SEC_R2_Disable(void)
{
    MHal_R2_Write_Reg8(BASE_NON_PM, REG_SEC_R2_STOP, 0x00);
}

void MHal_SEC_R2_Enable(PHY phyAddr)
{
    //(1) Disable R2 -> Stop(Reset)
    MHal_SEC_R2_Disable();

    //(2) Set R2 clock
    MHal_R2_Write_Reg8(BASE_NON_PM, REG_NPM_CLKGEN0_SEC_R2, 0x00);

    //(3) Set CPU reset base(vector)
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_SEC_R2_RST_BASE, phyAddr >> 16); // reset vector address 0x0(64K alignment)

    //(4)Set MAU Mapping
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_MAU_LV1, 0x8400UL);

    //(5) Set RIU   base
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_SEC_R2_RIU_BASE, 0xFA00UL); // RIU  base address

    //(6) Set UART base
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_SEC_R2_IO1_BASE, 0xF800UL); // UART base address
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_SEC_R2_SPI_BASE, 0xF900UL);
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_SEC_R2_SPI_BASE1, 0xF900UL);
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_SEC_R2_DQMEM_BASE, 0xFB00UL);
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_SEC_R2_QMEM_MASK_HIGH, 0xFF00UL);

    //(7) Set IO space enable (UART, RIU) with QMEM space disabled
    MHal_R2_Write_Reg16(BASE_NON_PM, REG_SEC_R2_SPACE_EN, 0x0003UL); // io_space_en[3:0]: UART,RIU, qmem_space_en[4] : disable

    //(8) CPU SW reset R2
    // reg_cpux_sw_rstz =  aeon sw rstz(low)
    //*(volatile U32*)(0xBF000000+0x5CA4UL) &= ~BIT13;
    MHal_R2_Write_Reg16(BASE_PM, REG_PM_CPUX_SW_RSTZ, MHal_R2_Read_Reg16(BASE_PM, REG_PM_CPUX_SW_RSTZ) &(~BIT13));
    MHal_R2_Write_Reg16(BASE_PM, REG_PM_CPUX_SW_RSTZ, MHal_R2_Read_Reg16(BASE_PM, REG_PM_CPUX_SW_RSTZ) | (BIT13));

    //(9) Without QMEM

    //(10) Enable R2 -> Start
    MHal_R2_Write_Reg8(BASE_NON_PM, REG_SEC_R2_STOP, 0x24UL); // miu_sw_rst and sdram_boot = 1
    MHal_R2_Write_Reg8(BASE_NON_PM, REG_SEC_R2_STOP, 0x27UL); // sdram_boot and (miu/r2/r2_rst) =1
}
