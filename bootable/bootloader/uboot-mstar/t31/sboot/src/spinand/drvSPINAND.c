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

#ifndef _DRV_SPINAND_H_
#include "drvSPICMD.h"
#include "halSPINAND.h"
#include "halSPINAND_boot.h"
#endif

//==========================================================================
// Define
//==========================================================================


//==========================================================================
// Global Variable
//==========================================================================

U16 MDrv_DIV_To_RShift(U16 u16Size)
{
  /*u16Size should be 2^x*/
  U16 u16shift = 0;
  while(u16Size)
  {
     u16shift++;
     u16Size=u16Size >> 1;
  }
  return (u16shift - 1);
}

U32 MDrv_SPINAND_Read(U32 u32_BlkIdx, U32 u32_PageIdx, U8 u8_SectorInPage, U32 u32_SectorCnt, U8 *u8Data, U8 *pu8_SpareBuf)
{
    U32  u32DataSize;
    U32  u32Addr;
    U16  u16NumofSector;
    U8   u8Status;
    SPINAND_FLASH_ERRNO_e eRet = ERR_SPINAND_SUCCESS;
    //calculate page address
    u32Addr = (u32_BlkIdx * BLOCK_SIZE) + u32_PageIdx;

    //read data to cache first
    HAL_SPINAND_RFC(u32Addr, &u8Status);

    if(u8Status & ECC_STATUS_ERR)
    {
        eRet = ERR_SPINAND_ECC_ERROR;
    }

    if(PLANE)
        HAL_SPINAND_PLANE_HANDLER(u32Addr);
    //HAL_SPINAND_SetCKG(CLKCFG);
    // calculate data size
    u32DataSize = u32_SectorCnt * SECTOR_SIZE;
    // calculate read start address
    u32Addr = u8_SectorInPage * SECTOR_SIZE;

    // Read SPINand Data
    HAL_SPINAND_Read (u32Addr, u32DataSize, u8Data);

    // calculate number of sector per page
    //u16NumofSector = PAGE_SIZE / SECTOR_SIZE;
    u16NumofSector = PAGE_SIZE >> MDrv_DIV_To_RShift(SECTOR_SIZE);
    // update start address to spare start address

    u32Addr = PAGE_SIZE + ((SPARE_SIZE >> (MDrv_DIV_To_RShift(u16NumofSector)))*(u8_SectorInPage));

    // update size to SPARE space size
    u32DataSize = ((SPARE_SIZE >> (MDrv_DIV_To_RShift(u16NumofSector)))*u32_SectorCnt);

    // Read SPINand Spare Data
    HAL_SPINAND_Read(u32Addr, u32DataSize, pu8_SpareBuf);
    return eRet;
}

