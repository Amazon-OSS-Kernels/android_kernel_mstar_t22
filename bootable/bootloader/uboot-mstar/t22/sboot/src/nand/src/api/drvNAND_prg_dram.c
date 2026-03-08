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
#include "../../inc/common/drvNAND.h"
//#include "drvNAND_utl.h"
#define BL2_OFFSET	32*1024
#define CHUNK_HEADER_BINARYID 0x0000B007
#define CHUNK_HEADER_BINARYID_POS	0x7

#define NAND_HASH_END_BLK                   255

extern U32 nand_flush_cache_dram(U32 u32_DMAAddr, U32 u32_ByteCnt);

U32 drvNAND_LoaduBoot_dram(U8* pu8DMAAddr, U32 u32_BLSize);

static PAIRED_PAGE_MAP_t *ga_tPairedPageMap = (PAIRED_PAGE_MAP_t *)(CONFIG_BOOTRAM_LOADADDR - 0x4000);


#ifndef __ATTR_DRAM_CODE__
	#define __ATTR_DRAM_CODE__ 
#endif
U32 __ATTR_DRAM_CODE__  drvNAND_LoaduBoot_dram(U8* pu8DMAAddr, U32 u32_BLSize)
{
	NAND_DRIVER *pNandDrv = (NAND_DRIVER*)(CONFIG_SRAM_BASE_ADDRESS + CONFIG_SRAM_FCIE_OFFSET);
	U32         u32_Err = UNFD_ST_SUCCESS;
	U32         u32_SectorCnt;
	U32         u32_i;
	U32         u32_j;
	U32         u32_TmpSectorCnt;
	U32         u32_Row;
	U8	        u8_PBA;
	U16         u16_BlkPageCnt;
	U8	        au8_SpareData[2];
	U8	*pu8_SpareData = au8_SpareData;
	
	#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	pu8_SpareData = (U8 *)gpu8_SpareBuf;
	#endif

	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD000);
	u16_BlkPageCnt = pNandDrv->u16_BlkPageCnt >> pNandDrv->u8_CellType;	
	u32_SectorCnt = (u32_BLSize) >> pNandDrv->u8_SectorByteCntBits;
	if(u32_BLSize & (pNandDrv->u16_SectorByteCnt - 1))
		u32_SectorCnt += 1;
	
	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD100);
	
	//1st uboot locate at the next good block of BL1PBA
	
	u8_PBA = pNandDrv->u8_BL0PBA != 0 ? pNandDrv->u8_BL0PBA : pNandDrv->u8_HashPBA[2][0]-1;

	RETRY_READ_UBOOT2:
		
	for(u32_i = 0; u32_i < u32_SectorCnt; u32_i += pNandDrv->u16_PageSectorCnt)
	{
		u32_TmpSectorCnt = ((u32_i + pNandDrv->u16_PageSectorCnt) < u32_SectorCnt) ?
							pNandDrv->u16_PageSectorCnt : (u32_SectorCnt - u32_i);
		
		u32_j = (u32_i & (pNandDrv->u16_PageSectorCnt * u16_BlkPageCnt - 1)) >> pNandDrv->u8_PageSectorCntBits;
		
		if(u32_j == 0 )
		{	
			search_next_block:
			u8_PBA ++;
			// Read 1st sector for checking good block
			u32_Row = u8_PBA << pNandDrv->u8_BlkPageCntBits;

			#if defined(ENABLE_CACHE) && ENABLE_CACHE
			//only invalidate first page of Mboot for checking header.
			if(u32_i == 0)
			{
				nand_flush_cache_dram(
					(U32)pu8DMAAddr, pNandDrv->u16_SectorByteCnt*u32_TmpSectorCnt);
				
				#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
				nand_flush_cache_dram( (U32)gpu8_SpareBuf, pNandDrv->u16_SpareByteCnt);
				#endif
				
			}
			#endif

			u32_Err = NC_ReadSectors_dram(u32_Row, 0, pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
			                         pu8_SpareData, u32_TmpSectorCnt);

			// Check the bad block infomation
			if( pu8_SpareData[0] != 0xFF  || u32_Err != UNFD_ST_SUCCESS)
			{
				goto search_next_block;
			}
			else
			{
				// Check chunk header binary ID 0x0000B007
				if(u32_i == 0)
				{
					if( ((U32*)(pu8DMAAddr))[CHUNK_HEADER_BINARYID_POS] != CHUNK_HEADER_BINARYID)
					{
						goto search_next_block;
					}
				}
			}
		}
		else
		{
			u32_Row = u8_PBA << pNandDrv->u8_BlkPageCntBits;
			u32_Row += ga_tPairedPageMap[u32_j].u16_LSB;

			//always need spare address for dma in FCIE5
			#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
			u32_Err = NC_ReadSectors_dram(u32_Row, 0, pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
										pu8_SpareData, u32_TmpSectorCnt);
			#else
			u32_Err = NC_ReadSectors_dram(u32_Row, 0, pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
										NULL, u32_TmpSectorCnt);
			#endif
		}

		#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
		if((u32_Err != UNFD_ST_SUCCESS) || (REG(NC_CTRL)&BIT_NC_ALLONE_FLAG))
		#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
		if( (u32_Err != UNFD_ST_SUCCESS) || (REG(NC_STAT_CHK)&BIT_NC_ALLONE_FLAG))
		#endif
		{
			if(u8_PBA==NAND_HASH_END_BLK)
				return UNFD_ST_ERR_NO_BL;
			
			goto RETRY_READ_UBOOT2;
		}

	}
	
	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD200);
	
	return u32_Err;
}
