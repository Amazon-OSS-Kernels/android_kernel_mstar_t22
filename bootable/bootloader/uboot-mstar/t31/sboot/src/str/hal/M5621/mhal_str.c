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

#include "mhal_str.h"
#include "mhal_str_reg.h"

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
static U8 MHal_STR_Read_Reg8(VIRT virtBase, U32 u32Reg)
{
    return *((volatile U8 *)(virtBase + (u32Reg << 1)) - (u32Reg & 1));
}

static U16 MHal_STR_Read_Reg16(VIRT virtBase, U32 u32Reg)
{
    return *((volatile U16 *)(virtBase + (u32Reg << 1)));
}

static void MHal_STR_Write_Reg8(VIRT virtBase, U32 u32Reg, U8 u8Val)
{
    *((volatile U8 *)(virtBase + (u32Reg << 1) - (u32Reg & 1))) = u8Val;
}

static void MHal_STR_Write_Reg16(VIRT virtBase, U32 u32Reg, U16 u16Val)
{
    *((volatile U16 *)(virtBase + (u32Reg << 1))) = u16Val;
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void MHal_SEC_STR_FastStrMiuRegResume(void)
{
    U16 *pu8mMiuReg=(U16*)(0x201f0018);
    MHal_STR_Write_Reg16(BASE_PM, 0x1012F0, (U16)pu8mMiuReg[0]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1012F2, (U16)pu8mMiuReg[2]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1012F4, (U16)pu8mMiuReg[4]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1012F6, (U16)pu8mMiuReg[6]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1012F8, (U16)pu8mMiuReg[8]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1012FA, (U16)pu8mMiuReg[10]);
    MHal_STR_Write_Reg16(BASE_PM, 0x137F20, (U16)pu8mMiuReg[12]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1006F0, (U16)pu8mMiuReg[14]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1006F2, (U16)pu8mMiuReg[16]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1006F4, (U16)pu8mMiuReg[18]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1006F6, (U16)pu8mMiuReg[20]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1006F8, (U16)pu8mMiuReg[22]);
    MHal_STR_Write_Reg16(BASE_PM, 0x1006FA, (U16)pu8mMiuReg[24]);
    MHal_STR_Write_Reg16(BASE_PM, 0x137F22, (U16)pu8mMiuReg[26]);
}

void MHal_SEC_STR_FastStrConsoleInit(void)
{
    //bit 12 UART Enable, bit 11 UART RX Enable
    *(volatile U32*)(BASE_PM+(0x0E12<<1)) &= ~0x1800;    //don't enable uart (decided by mboot env)

    // Switch UART0 to PIU UART 0
    // Clear UART_SEL0
    *(volatile U32*)(BASE_PM+(0x101EA6<<1)) &= ~0x000F;

    // bit 11 UART RX Enable
    *(volatile U32*)(BASE_PM+(0x0E12<<1)) |= 0x0800;

    // UART_SEL0 --> PIU UART0

#ifdef CONFIG_UART2_SEL
    *(volatile U32*)(BASE_PM+(0x101EA6<<1)) |= 0x0500;

#endif
    *(volatile U32*)(BASE_PM+(0x101EA6<<1)) = 0x0654;

    *(volatile U32*)(BASE_PM+(0x101EA8<<1)) = 0x0000;

    // UART mode
    //enable reg_uart_rx_enable;
    *(volatile U32*)(BASE_PM+(0x101EA2<<1)) |= (1<<10);
}
