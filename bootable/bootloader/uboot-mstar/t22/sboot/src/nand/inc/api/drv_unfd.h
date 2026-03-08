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

/**
* @version
* @brief
*
*/

#ifndef __DRV_UNFD_H__
#define __DRV_UNFD_H__

/*=============================================================*/
// Include files
/*=============================================================*/

/*=============================================================*/
// Extern definition
/*=============================================================*/

/*=============================================================*/
// Macro definition
/*=============================================================*/

/*=============================================================*/
// Data type definition
/*=============================================================*/
typedef struct _NAND_INFO
{
	U8	au8_Tag[16];
	U8	u8_IDByteCnt;
	U8	au8_ID[15];
	U32	u32_ChkSum;
	U16	u16_SpareByteCnt;
	U16	u16_PageByteCnt;
	U16	u16_BlkPageCnt;
	U16	u16_BlkCnt;
	U32	u32_Config;
	U16	u16_ECCType;
	U16	u16_SeqAccessTime;
	U8	padding[12];
	U8	u8_Vendor[16];
	U8	u8_PartNumber[16];

	U16	u16_ECCCodeByteCnt;
	U16	u16_PageSectorCnt;
	U8	u8_WordMode;

} NAND_INFO_t;

typedef struct _CIS {

	U8	au8_NandInfo[512];
	U8	au8_PartInfo[512];
	U8	au8_PairedPageMap[2048];

} CIS_t;
/*=============================================================*/
// Variable definition
/*=============================================================*/

/*=============================================================*/
// Global function definition
/*=============================================================*/
extern U32 drvNAND_Init(void);
extern U32 drvNAND_IPVerify_Main(void);
extern U32 drvNAND_ChkRdy(U32 u32_PartNo);
extern int drv_unfd_init(void);
extern U32 drvNAND_InitProgrammer(void);
extern U32 drvNAND_GetNandID(U8 *pu8IDByteCnt, U8 *pu8ID);
extern U32 drvNAND_CheckCIS(CIS_t *ptCISData);
extern U32 drvNAND_WriteCIS(CIS_t *ptCISData);
extern U32 drvNAND_ReadCIS(CIS_t *ptCISData);
extern U32 drvNAND_EraseAllPartitions(void);
extern U32 drvNAND_FormatFTL(void);
extern U32 drvNAND_GetPhyCapacity(void);
extern U32 drvNAND_GetNandInfo(NAND_INFO_t *pNandInfo);
extern U32 drvNAND_EraseAllPhyBlk(void);
extern U32 drvNAND_EraseAllGoodPhyBlk(U16 *pu16_GoodBlkCnt, U16 *pu16_BadBlkIdxList, U16 *pu16_BadBlkCnt);
extern U32 drvNAND_ErasePhysicalBlk(U16 u16_BlkIdx);
extern U32 drvNAND_ReadPhysicalPage(U32 u32_PageIdx, U8 *pu8_Data, U8 *pu8_Spare);
extern U32 drvNAND_WritePhysicalPage(U32 u32_PageIdx, U8 *pu8_Data, U8 *pu8_Spare);
extern U32 drvNAND_CheckGoodBlk(U16 u16_BlkIdx);
extern U32 drvNAND_ResetNandFlash(void);
extern U32 drvNAND_FlushCache(void);

#endif //__DRV_UNFD_H__

