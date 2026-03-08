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

#include <mhal_miu.h>
#include <MsTypes.h>
#include <drvMIU.h>

static void _MIU_Write2Bytes(U32 u32RegOffset, U16 u16Val)
{
    ((volatile U16*)(MIUREG_BASE))[u32RegOffset] = u16Val;
}

U8 HAL_MIU_SelectMIU_UBoot(U8 bMIU1, U16 *au16SelMiu, U8 u8GpNum)
{
    return TRUE;
}

static MS_U8 u8MIUProtectkernel_ID[16] = {
// comment enums which are over u8 size
//    MIU_CLIENT_CPU_R,
//    MIU_CLIENT_CPU_W,
};

static MS_U32 u32MIUProtectkernel_ID[16] = {
    MIU_CLIENT_CPU_R,   //0
    MIU_CLIENT_CPU_W,
    MIU_CLIENT_GPU_R,
    MIU_CLIENT_GPU_W,
    MIU_CLIENT_USB_UHC0_RW,
    MIU_CLIENT_USB_UHC1_RW,
    MIU_CLIENT_USB_UHC2_RW,
    MIU_CLIENT_SDIO_RW,
    MIU_CLIENT_SC_DIP_0_W,
    MIU_CLIENT_FCIE_RW,
    MIU_CLIENT_EMAC_RW,
};

MS_U8* HAL_MIU_GetKernelProtectClients(void)
{
    return u8MIUProtectkernel_ID;
}

MS_U32* HAL_MIU_GetKernelProtectClientsEx(void)
{
    return u32MIUProtectkernel_ID;
}

MS_U64 HAL_MIU_PA2BA(MS_U64 u64PhyAddr)
{
    MS_U64 u64BusAddr = 0x0;
    if (u64PhyAddr>=CONFIG_SYS_MIU2_INTERVAL) //PA 0xC --> BA 0xE
    {
        u64BusAddr = u64PhyAddr + 0x20000000;
    }
    else
    {
        u64BusAddr =u64PhyAddr + CONFIG_SYS_MIU0_BUS;
    }
    return u64BusAddr;
}

MS_U64 HAL_MIU_BA2PA(MS_U64 u64BusAddr)
{
    if((u64BusAddr >= 0) && u64BusAddr<CONFIG_SYS_MIU1_BUS)
    {
        return (u64BusAddr - CONFIG_SYS_MIU0_BUS);
    }
    else if (u64BusAddr >= CONFIG_SYS_MIU1_BUS)
    {
        return (u64BusAddr - CONFIG_SYS_MIU1_BUS+CONFIG_SYS_MIU_INTERVAL);
    }
    else
    {
        return 0;
    }
}

void HAL_MIU_ProtectDummyDRAM(void)
{
    return;
}
