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
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
#include "../mmc/inc/common/eMMC.h"
#include "../part_emmc.h"
#endif

#include "../../include/autoconf.h"
#include "../../../u-boot-2011.06/MstarApp/include/MsTrustZone.h"
#include "../MMAPInfo.h"
#define EMMC_BLOCK_SIZE (512)

#if defined (CONFIG_TEE_LOADER)
#define UBOOTOFFSET 0x2400
#else
#define UBOOTOFFSET 0x400
#endif

int ns_find_mbootbak(void)
{
    U8* part_name1 ="MBOOTBAK";
    U8* part_name2 ="MBOOT";
    U8 u8_i           = 0;
    U8 u8_j           = 0;
    U8 u8found_part   = 0;
    U32 u32RAM_START  = 0 ;
    U32 u32RAM_END    = 0;
    U32 u32RAM_SIZE   = 0;
    U8  u8len_name    = 7; //length of string MBOOTBAK
    emmc_partition_t mpart;

    // iterate to find  MBOOTBAK partition
    for(u8_i=0; u8_i<= EMMC_RESERVED_FOR_MAP_V2;u8_i++)//CustomizeID=0
    {
        if(eMMC_ReadData_MIU((U8*)&mpart, EMMC_BLOCK_SIZE, u8_i))
            break; // emmc read fail

        u8found_part = 1;

        for (u8_j=0; u8_j<u8len_name; u8_j++)
        {
            if (mpart.name[u8_j]!=part_name1[u8_j])
            {
                u8found_part = 0;
                break;
            }
        }

        if (u8found_part)
        {
            //Load chunker size=0x400, 0x2400 is chunkder+customer keybank
            eMMC_ReadData_MIU(CONFIG_UBOOT_LOADADDR-UBOOTOFFSET, UBOOTOFFSET, mpart.start_block);

            //Authenticate Chunk header
            if(MDrv_AESDMA_SecureMain(CONFIG_UBOOT_LOADADDR-UBOOTOFFSET , 0x2F0, CONFIG_UBOOT_LOADADDR-UBOOTOFFSET+0x300, 0x0) == 0)
            {
                //*(volatile unsigned int*)(0x1F201300) = 'C';
                return 0;
            }

            u32RAM_START = *((U32 *)(CONFIG_UBOOT_LOADADDR-UBOOTOFFSET+4));
            u32RAM_END   = *((U32 *)(CONFIG_UBOOT_LOADADDR-UBOOTOFFSET+8));
            u32RAM_SIZE  = u32RAM_END - u32RAM_START;

            //Load uboot nad SECURITY INFO Size 0x110
#if defined (CONFIG_TEE_LOADER)
            eMMC_ReadData_MIU(CONFIG_UBOOT_LOADADDR, u32RAM_SIZE+0x110, mpart.start_block+0x12);
#else
            eMMC_ReadData_MIU(CONFIG_UBOOT_LOADADDR, u32RAM_SIZE+0x110, mpart.start_block+0x2);
#endif

            //Authenticate Chunk header
            if(MDrv_AESDMA_SecureMain(CONFIG_UBOOT_LOADADDR, u32RAM_SIZE, CONFIG_UBOOT_LOADADDR+u32RAM_SIZE+0x10, 0x0) == 0)
            {
                //*(volatile unsigned int*)(0x1F201300) = 'M';
                return 0;
            }

            break;
        }
    }

    if (u8found_part) //try find MBOOT partition and copy from MBOOTBAK to MBOOT
    {
        u8len_name = 4; // length of string MBOOT

        // iterate to find specific partition
        for(u8_i=0; u8_i<= EMMC_RESERVED_FOR_MAP_V2; u8_i++)//CustomizeID=0
        {
            if(eMMC_ReadData_MIU((U8*)&mpart, EMMC_BLOCK_SIZE, u8_i))
                break; // emmc read fail

            u8found_part = 1;

            for (u8_j=0; u8_j<u8len_name; u8_j++)
            {
                if (mpart.name[u8_j]!=part_name2[u8_j])
                {
                    u8found_part = 0;
                    break;
                }
            }

            if (u8found_part)
            {
                //*(volatile unsigned int*)(0x1F201300) = 'F';
                eMMC_WriteData_MIU(CONFIG_UBOOT_LOADADDR-UBOOTOFFSET, u32RAM_SIZE+0x110+UBOOTOFFSET, mpart.start_block);
                return 1;
            }
        }
    }

    return 0;
}
