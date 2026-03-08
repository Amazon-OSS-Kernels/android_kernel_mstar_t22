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
#define _DRV_SPINAND_H_
#include "halSPINAND.h"


#define MID_GD          0xC8
#define MID_MICRON      0x2C
#define MID_WIN         0xEF

typedef enum _SPINAND_ERROR_NUM
{
    ERR_SPINAND_SUCCESS,
    ERR_SPINAND_UNKNOWN_ID,
    ERR_SPINAND_TIMEOUT,
    ERR_SPINAND_ECC_ERROR,
    ERR_SPINAND_BAD_BLK,
    ERR_SPINAND_E_FAIL,
    ERR_SPINAND_W_FAIL,
    ERR_SPINAND_INVALID
} SPINAND_FLASH_ERRNO_e;

typedef struct
{
    U8   u8_IDByteCnt; 
    U8   au8_ID[15];
    U16  u16_SpareByteCnt;
    U16  u16_PageByteCnt;
    U16  u16_BlkPageCnt;
    U16  u16_BlkCnt;
    U16  u16_SectorByteCnt;
    U8   u8PlaneCnt;
    U8   u8WrapConfig;
    BOOL bRIURead;
    U8   u8_MaxClk;
    U8   u8_UBOOTPBA;
    U8   u8_BL0PBA;
    U8   u8_BL1PBA;
    U8   u8_HashPBA[3][2];
//    U8   u8ReadMode;
//    U8   u8WriteMode;
} SPINAND_FLASH_INFO_t;

#define _gtSpinandInfo    ((SPINAND_FLASH_INFO_t *)CONFIG_SRAM_SPINAND_OFFSET)

#define SPARE_SIZE  _gtSpinandInfo->u16_SpareByteCnt
#define BLOCK_SIZE  _gtSpinandInfo->u16_BlkPageCnt
#define SECTOR_SIZE _gtSpinandInfo->u16_SectorByteCnt
#define PAGE_SIZE   _gtSpinandInfo->u16_PageByteCnt
#define PLANE       _gtSpinandInfo->u8PlaneCnt
#define RIU_FLAG    _gtSpinandInfo->bRIURead
#define CLKCFG      _gtSpinandInfo->u8_MaxClk
//#define READ_MODE   _gtSpinandInfo->u8ReadMode
//#define WRITE_MODE  _gtSpinandInfo->u8WriteMode

U32 MDrv_SPINAND_Read(U32 u32_BlkIdx, U32 u32_PageIdx, U8 u8_SectorInPage, U32 u32_SectorCnt, U8 *u8Data, U8 *pu8_SpareBuf);

#endif

