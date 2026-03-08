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
#ifndef PUTK
#define print_err(Err)
#endif

#ifndef BL2_OFFSET
#define BL2_OFFSET	32*1024
#endif
#define CHUNK_HEADER_BINARYID 0x0000B007
#define CHUNK_HEADER_BINARYID_POS	0x7

#define NAND_HASH_END_BLK                   255

#if defined(CONFIG_TEE_LOADER)
PAIRED_PAGE_MAP_t ga_tPairedPageMap[512];
#else
static PAIRED_PAGE_MAP_t *ga_tPairedPageMap = (PAIRED_PAGE_MAP_t *)(CONFIG_BOOTRAM_LOADADDR - 0x4000);
#endif

#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
#if defined(CONFIG_TEE_LOADER)
UNFD_ALIGN0 U8 gpu8_SpareBuf[4096]; UNFD_ALIGN1// 4K
#else
static U8* gpu8_SpareBuf = (U8*)(CONFIG_BOOTRAM_LOADADDR - 0x5000); // 4K
#endif
#endif

#if defined(CONFIG_MSTAR_NS_UBOOT)
static U8* gpu8_PageDataBuf = (U8*)(CONFIG_BOOTRAM_LOADADDR - 0x9000); // 16K
static U8* gpu8_PartInfo = (U8*)(CONFIG_BOOTRAM_LOADADDR - 0xA000); // 4K
static U8 gu8_PniInit = 0;
#elif defined(CONFIG_TEE_LOADER)
UNFD_ALIGN0 U8 gpu8_PageDataBuf[32768]; UNFD_ALIGN1; // 32K
UNFD_ALIGN0 U8 gpu8_PartInfo[1024]; UNFD_ALIGN1; // 1K
U8 gu8_PniInit = 0;
#endif

#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
static U8 gu8_ReadId = 0;
#endif

U32 drvNAND_LoadHash0(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size);
U32 drvNAND_LoadBLOffset(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size);
U32 drvNAND_Load2ndBL(U8* pu8DMAAddr, U32 u32_Size);
U32 drvNAND_LoaduBoot(U8* pu8DMAAddr, U32 u32_BLSize);
void * nand_memcpy (void *destaddr, void const *srcaddr, unsigned int len);
U32 drvNAND_IsGoodBlk(U16 u16_PBA);
U8 drvNAND_CountBits(U32 u32_x);
PARTITION_RECORD_t * drvNAND_SearchPartition(PARTITION_RECORD_t *pRecord, U16 u16_PartType);
U32 drvNAND_ReadBlk(U8 *pu8_DestAddr, U8 u8_PartType, U16 u16_PBA,
							U16 u16_LBA, U32 u32_StartSector, U32 u32_SectorCnt);
U32 drvNAND_ReadPartition(U16 u16_PartType, U32 u32_StartPhySector,
						   U32 u32_SrcPhyAddr, U32 u32_SectorCnt );
U32 nand_set_partinfo (U8 *pu8_PartInfo);

static U32 drvNAND_LoadBL(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size, U8 u8_IsDMA)
{
	#if (defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE)  || defined(ENABLE_ROM_DMA_FUNCTION) && ENABLE_ROM_DMA_FUNCTION
	PAIRED_PAGE_MAP_t *ga_tPairedPageMapSRAM = (PAIRED_PAGE_MAP_t *) (ROM_PPM_ADDR);		//rom code 's pair-page map
	#endif
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	U32	u32_Err = UNFD_ST_SUCCESS;
	U32	u32_SectorCnt;
	U32	u32_i;
	U32	u32_j;
	U32	u32_TmpSectorCnt = 0;
	U32	u32_Row;
	U32	u32_SectorIdx;
	U8	*pu8_PBA;
    #if !(defined(NC_SEL_FCIE5) && NC_SEL_FCIE5)
	U8	au8_SpareData[2];
    #endif
	U8	*pu8_SpareData;
	U8	u8_BootStageId;

	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD100);

	#ifdef REMAP_PPM_ADDR
	if(u8_IsDMA == 0)
		nand_remap_ppm_addr(&ga_tPairedPageMapSRAM);
	#endif

	u32_SectorCnt = (u32_Size) >> pNandDrv->u8_SectorByteCntBits;
	if(u32_Size & (pNandDrv->u16_SectorByteCnt - 1))
		u32_SectorCnt += 1;

	if(pNandDrv->u8_BL0PBA)
	{
		pu8_PBA = &pNandDrv->u8_BL0PBA;
		u8_BootStageId = BOOT_NONSECURE;
	}
	else
	{
		pu8_PBA = &pNandDrv->u8_HashPBA[1][0];
		u8_BootStageId = BOOT_HASH1;
	}

    #if defined(ENABLE_ROM_DMA_FUNCTION) && ENABLE_ROM_DMA_FUNCTION
    if(u8_IsDMA == 0)
        pNandDrv->pReadSectors = (ReadSectorsPtr)drvNAND_get_ReadSectors_address();
    #endif
	while(1)
	{
		if(u8_IsDMA)
		{
			//prepare dram dma address for spare of FCIE5
			#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
			pu8_SpareData = gpu8_SpareBuf;
            #else
            pu8_SpareData = au8_SpareData;
			#endif
			u32_Err = NC_ReadSectors((*pu8_PBA) << pNandDrv->u8_BlkPageCntBits,
									0,
									pu8DMAAddr,
									pu8_SpareData,
									1);
		}
		#if defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE
		else
		{
			#if defined(SPARE_BUFFER_ADDR) 	//for fcie5 only
			pu8_SpareData = (U8 *)SPARE_BUFFER_ADDR;
            #else
            pu8_SpareData = au8_SpareData;
			#endif
			u32_Err = NC_ReadSector_RIUMode((*pu8_PBA) << pNandDrv->u8_BlkPageCntBits,
											0,
											pu8DMAAddr,
											pu8_SpareData);
		}
		#elif defined(ENABLE_ROM_DMA_FUNCTION) && ENABLE_ROM_DMA_FUNCTION
		else
		{
			//prepare sram dma address for spare of FCIE5
			#if defined(SPARE_BUFFER_ADDR) //for fcie5 only
			pu8_SpareData = (U8 *)SPARE_BUFFER_ADDR;
            #else
            pu8_SpareData = au8_SpareData;
			#endif
			nand_switch_sram((U32)pu8DMAAddr - CONFIG_SRAM_BASE_ADDRESS);
			u32_Err = pNandDrv->pReadSectors((*pu8_PBA) << pNandDrv->u8_BlkPageCntBits,
											0,
											pu8DMAAddr,
											pu8_SpareData, 1);
		}
		#endif
		if(u32_Err==UNFD_ST_SUCCESS && pu8_SpareData[0]==0xFF && pu8_SpareData[1]==u8_BootStageId)
		{
			for(u32_i = 0; u32_i < u32_SectorCnt; )
			{
				u32_SectorIdx = (((u32_Offset >> pNandDrv->u8_SectorByteCntBits) + u32_i) & (pNandDrv->u16_PageSectorCnt - 1));

				//calculate page index in a block for pair page map
				u32_j = (u32_i + (u32_Offset >> pNandDrv->u8_SectorByteCntBits)) >> pNandDrv->u8_PageSectorCntBits;

				u32_Row = (*pu8_PBA) << pNandDrv->u8_BlkPageCntBits;
				if(u8_IsDMA)
				{
					u32_TmpSectorCnt = ((u32_i + pNandDrv->u16_PageSectorCnt) < u32_SectorCnt)
                                                ? (pNandDrv->u16_PageSectorCnt-u32_SectorIdx) : (u32_SectorCnt - u32_i);
					u32_Row += ga_tPairedPageMap[u32_j].u16_LSB;
					//prepare dram dma address for spare of FCIE5
					#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
					pu8_SpareData = gpu8_SpareBuf;
					u32_Err = NC_ReadSectors(u32_Row,
											u32_SectorIdx,
											pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
											pu8_SpareData,
											u32_TmpSectorCnt);

					#else
					u32_Err = NC_ReadSectors(u32_Row,
											u32_SectorIdx,
											pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
											NULL,
											u32_TmpSectorCnt);
					#endif
					u32_i += u32_TmpSectorCnt;
				}
				#if defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE
				else
				{
					u32_Row += ga_tPairedPageMapSRAM[u32_j].u16_LSB;
					#if defined(SPARE_BUFFER_ADDR)  //for fcie5 only
					pu8_SpareData = (U8 *)SPARE_BUFFER_ADDR;
					u32_Err = NC_ReadSector_RIUMode(u32_Row,
													u32_SectorIdx,
													pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
													pu8_SpareData);

					#else	//for fcie3
					u32_Err = NC_ReadSector_RIUMode(u32_Row,
													u32_SectorIdx,
													pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
													NULL);
					#endif
					u32_i++;
				}
				#elif defined(ENABLE_ROM_DMA_FUNCTION) && ENABLE_ROM_DMA_FUNCTION
				else
				{
					u32_Row += ga_tPairedPageMapSRAM[u32_j].u16_LSB;

					nand_switch_sram((U32)pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits)  - CONFIG_SRAM_BASE_ADDRESS);

					u32_TmpSectorCnt = ((u32_i + pNandDrv->u16_PageSectorCnt) < u32_SectorCnt)
				                   				? (pNandDrv->u16_PageSectorCnt-u32_SectorIdx) : (u32_SectorCnt - u32_i);

					#if defined(SPARE_BUFFER_ADDR)  //for fcie5 only
					pu8_SpareData = (U8 *)SPARE_BUFFER_ADDR;
                    #else
                    pu8_SpareData = au8_SpareData;
					#endif

					u32_Err = pNandDrv->pReadSectors(u32_Row,
													u32_SectorIdx,
													pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
													pu8_SpareData, u32_TmpSectorCnt);
					u32_i += u32_TmpSectorCnt;
				}
				#endif
				#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
				if( (u32_Err != UNFD_ST_SUCCESS) || (REG(NC_CTRL)&BIT_NC_ALLONE_FLAG))
				#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
				if( (u32_Err != UNFD_ST_SUCCESS) || (REG(NC_STAT_CHK)&BIT_NC_ALLONE_FLAG))
				#endif
					goto RETRY_READ_BL;
			}

			goto EXIT_BL;
		}

RETRY_READ_BL:
		if((*pu8_PBA)==NAND_HASH_END_BLK)
		{
			//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD110);
			return UNFD_ST_ERR_NO_BL;
		}

		(*pu8_PBA)++;
	}

EXIT_BL:
	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD200);
	return u32_Err;
}

#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
U32 drvNAND_Check_BBM_WA(void)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	U8 u8_i, u8_j;

	if(gu8_ReadId == 0)
	{
		pNandDrv->u8_bbm_wa = 0;

		if(NC_ReadID() == UNFD_ST_SUCCESS)
		{
			//{5, {0xEC, 0xDA, 0x10, 0x95, 0x46}},
			//{5, {0xEC, 0xDC, 0x10, 0x95, 0x56}},
			//{5, {0xEC, 0xD3, 0x51, 0x95, 0x5A}},
			if(pNandDrv->au8_ID[0] == 0xEC)
			{
				if((pNandDrv->au8_ID[1] == 0xDA && pNandDrv->au8_ID[2] == 0x10 && pNandDrv->au8_ID[3] == 0x95 && pNandDrv->au8_ID[4] == 0x46)
				|| (pNandDrv->au8_ID[1] == 0xDC && pNandDrv->au8_ID[2] == 0x10 && pNandDrv->au8_ID[3] == 0x95 && pNandDrv->au8_ID[4] == 0x56)
				|| (pNandDrv->au8_ID[1] == 0xD3 && pNandDrv->au8_ID[2] == 0x51 && pNandDrv->au8_ID[3] == 0x95 && pNandDrv->au8_ID[4] == 0x5A))
				   pNandDrv->u8_bbm_wa = 0xBB;
			}
		}
		gu8_ReadId = 1;
	}

	return UNFD_ST_SUCCESS;
}
#endif

/*
 * This api loads the 2nd BL in the same block of the BL0_PBA or BL1_PBA from offset 32KB
 *
 */

#if (defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE) || defined(ENABLE_ROM_DMA_FUNCTION) && ENABLE_ROM_DMA_FUNCTION
U32 drvNAND_LoadBLOffset(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size)
{
	#if (defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE)

	#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
	REG_CLR_BITS_UINT16(NC_REG_PAD_SWITCH, BIT3);
	#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	REG_WRITE_UINT16(NC_BOOT_MODE, 0);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_BOOT_MODE);
	#endif

	#elif defined(ENABLE_ROM_DMA_FUNCTION) && ENABLE_ROM_DMA_FUNCTION

	#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
	REG_SET_BITS_UINT16(NC_REG_PAD_SWITCH, BIT3);
	#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	REG_WRITE_UINT16(NC_BOOT_MODE, BIT_NAND_BOOT_EN);
	REG_SET_BITS_UINT16(NC_FUN_CTL, BIT_BOOT_MODE);
	#endif

	#endif

	#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
	drvNAND_Check_BBM_WA();
	#endif

	return(drvNAND_LoadBL(pu8DMAAddr, u32_Offset, u32_Size, 0));
}
#endif

/*
 * This api loads the 2nd BL in the same block of the BL0_PBA or BL1_PBA from offset 32KB
 *
 */

U32 drvNAND_Load2ndBL(U8* pu8DMAAddr, U32 u32_Size)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	U32 u32_Err;
	U32 u32_i;

	#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
	REG_CLR_BITS_UINT16(NC_REG_PAD_SWITCH, BIT3);
	#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	REG_WRITE_UINT16(NC_BOOT_MODE, 0);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_BOOT_MODE);
	#endif

	#ifdef CONFIG_NAND_SAMSUNG_BBM_WA
	drvNAND_Check_BBM_WA();
	#endif

	// Load paired page table first
	#if defined(SUPPORT_3D_NAND) && SUPPORT_3D_NAND
	if(pNandDrv->u8_CellType && pNandDrv->u8_SlcMode == 0)
	#else
	if(pNandDrv->u8_CellType)
	#endif
	{
		#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
		u32_Err = NC_ReadSectors(pNandDrv->u32_CISRow + pNandDrv->u8_PairPageMapLoc, 0, (U8*)ga_tPairedPageMap, NULL, 2);
		#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
		u32_Err = NC_ReadSectors(pNandDrv->u32_CISRow + pNandDrv->u8_PairPageMapLoc, 0, (U8*)ga_tPairedPageMap, gpu8_SpareBuf, 2);
		#endif
		if(u32_Err != UNFD_ST_SUCCESS)
		{
			//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD010);
			print_err(UNFD_ST_ERR_NO_CIS);
			return UNFD_ST_ERR_NO_CIS;
		}
	}
	else
	{
		for(u32_i = 0; u32_i < pNandDrv->u16_BlkPageCnt; u32_i ++)
		{
			ga_tPairedPageMap[u32_i].u16_LSB = u32_i;
		}
	}

	u32_Err = drvNAND_LoadBL(pu8DMAAddr, BL2_OFFSET, u32_Size, 1);
	if(u32_Err)
	{
		print_err(u32_Err);
	}
	return u32_Err;
}

/*
 * drvNAND_LoadBLOffsetDma must be used after drvNAND_Load2ndBL
 */
U32 drvNAND_LoadBLOffsetDma(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size)
{
	return(drvNAND_LoadBL(pu8DMAAddr, u32_Offset, u32_Size, 1));
}

// in order to reduce nand code size in sram, we define a section in dram
// __ATTR_DRAM_CODE__ is defined in drvNAND_xxx_sboot.h
// section is added in sboot.lds.S
#ifndef __ATTR_DRAM_CODE__
	#define __ATTR_DRAM_CODE__
#endif
U32 __ATTR_DRAM_CODE__  drvNAND_LoaduBoot(U8* pu8DMAAddr, U32 u32_BLSize)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	U32         u32_Err = UNFD_ST_SUCCESS;
	U32         u32_SectorCnt;
	U32         u32_i;
	U32         u32_j;
	U32         u32_TmpSectorCnt;
	U32         u32_Row;
	U8	        u8_PBA;
	U16         u16_BlkPageCnt;
	U8          *pu8_SpareData;

	#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	pu8_SpareData = (U8 *)gpu8_SpareBuf;
    #else
   	U8	        au8_SpareData[2];
    pu8_SpareData = au8_SpareData;
	#endif

	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD000);
	u16_BlkPageCnt = pNandDrv->u16_BlkPageCnt >> pNandDrv->u8_CellType;
	#if defined(SUPPORT_3D_NAND) && SUPPORT_3D_NAND
	if(pNandDrv->u8_CellType && pNandDrv->u8_SlcMode == 1)
		u16_BlkPageCnt = pNandDrv->u16_BlkPageCnt;
	#endif
	u32_SectorCnt = (u32_BLSize) >> pNandDrv->u8_SectorByteCntBits;
	if(u32_BLSize & (pNandDrv->u16_SectorByteCnt - 1))
		u32_SectorCnt += 1;

	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD100);

	//1st uboot locate at the next good block of BL1PBA

	u8_PBA = pNandDrv->u8_BL0PBA != 0 ? pNandDrv->u8_BL0PBA : pNandDrv->u8_HashPBA[2][0]-1;
    #if defined(UBOOT_RECOVERY)
    REG_WRITE_UINT16(UBOOT_RECOVERY, 0);
    #endif
	RETRY_READ_UBOOT2:

	for(u32_i = 0; u32_i < u32_SectorCnt; u32_i += pNandDrv->u16_PageSectorCnt)
	{
		u32_TmpSectorCnt = ((u32_i + pNandDrv->u16_PageSectorCnt) < u32_SectorCnt) ?
							pNandDrv->u16_PageSectorCnt : (u32_SectorCnt - u32_i);

		u32_j = (u32_i & (pNandDrv->u16_PageSectorCnt * u16_BlkPageCnt - 1)) >> pNandDrv->u8_PageSectorCntBits;

		if(u32_j == 0 )
		{
			search_next_block:
			if(u8_PBA==NAND_HASH_END_BLK)
			{
				print_err(UNFD_ST_ERR_NO_BL);
				return UNFD_ST_ERR_NO_BL;
			}
			u8_PBA ++;
			// Read 1st sector for checking good block
			u32_Row = u8_PBA << pNandDrv->u8_BlkPageCntBits;

			#if defined(ENABLE_CACHE) && ENABLE_CACHE
			//only invalidate first page of Mboot for checking header.
			if(u32_i == 0)
			{
				nand_flush_cache(
					(U32)pu8DMAAddr, pNandDrv->u16_SectorByteCnt*u32_TmpSectorCnt);
			}
			#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
			nand_flush_cache( (U32)gpu8_SpareBuf, pNandDrv->u16_SpareByteCnt);
			#endif
			#endif

			u32_Err = NC_ReadSectors(u32_Row, 0, pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
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
			u32_Err = NC_ReadSectors(u32_Row, 0, pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
										pu8_SpareData, u32_TmpSectorCnt);
			#else
			u32_Err = NC_ReadSectors(u32_Row, 0, pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
										NULL, u32_TmpSectorCnt);
			#endif
		}

		#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
		if((u32_Err != UNFD_ST_SUCCESS) || (REG(NC_CTRL)&BIT_NC_ALLONE_FLAG))
		#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
		if( (u32_Err != UNFD_ST_SUCCESS) || (REG(NC_STAT_CHK)&BIT_NC_ALLONE_FLAG))
		#endif
		{
            #if defined(UBOOT_RECOVERY)
            REG_WRITE_UINT16(UBOOT_RECOVERY, 0x1);
            #endif
			goto RETRY_READ_UBOOT2;
		}

	}

	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD200);

	return u32_Err;
}

#if defined(CONFIG_MSTAR_NS_UBOOT) || defined(CONFIG_TEE_LOADER)
/*************************************************************************
This new APIs are created for OPTEE
*************************************************************************/
#define memcpy(destaddr, srcaddr, len) nand_memcpy(destaddr, srcaddr, len)
void * __ATTR_DRAM_CODE__ nand_memcpy (void *destaddr, void const *srcaddr, unsigned int len)
{
	char *dest = destaddr;
	char const *src = srcaddr;

	while (len-- > 0)
		*dest++ = *src++;
	return destaddr;
}

U32 __ATTR_DRAM_CODE__ drvNAND_IsGoodBlk(U16 u16_PBA)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	U8 *au8_DataBuf = gpu8_PageDataBuf;
	#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	U8	*au8_SpareBuf = gpu8_SpareBuf;
	#else
	U8	au8_SpareData[2];
	U8	*au8_SpareBuf = au8_SpareData;
	#endif
	U32 u32_PageIdx;
	U32 u32_Err;
	//U32 i;

	// old way checking
	//for(i=0; i<1; i++)
	{
		u32_PageIdx = u16_PBA << pNandDrv->u8_BlkPageCntBits;

        #if 0
	    switch(i)
	    {
			case 0:
			    break;
			case 1: u32_PageIdx++;
			    break;
			case 2: u32_PageIdx += pNandDrv->u16_BlkPageCnt-1;
			    break;
			case 3: u32_PageIdx += pNandDrv->u16_BlkPageCnt-2;
			    break;
	    }
        #endif
	    u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_DataBuf, au8_SpareBuf, 1);

	    if (au8_SpareBuf[0] != 0xFF)
		    return 0; // bad blk
		else if(u32_Err != UNFD_ST_SUCCESS && u32_Err != UNFD_ST_ERR_ECC_FAIL)
            return 0;  
	}

	return 1; // good blk
}

U8 __ATTR_DRAM_CODE__ drvNAND_CountBits(U32 u32_x)
{
	U8 u8_i = 0;

    if(u32_x==0) return u8_i;

	while (u32_x) {
		u8_i++;
		u32_x >>= 1;
	}

	return u8_i-1;
}

/* Search for partition of type @u16_PartType, begin from @pRecord */
PARTITION_RECORD_t * __ATTR_DRAM_CODE__ drvNAND_SearchPartition(PARTITION_RECORD_t *pRecord,
											U16 u16_PartType)
{
	PARTITION_INFO_t *pPartInfo = (PARTITION_INFO_t *)gpu8_PartInfo;

	while (pRecord - pPartInfo->records < pPartInfo->u16_PartCnt) {

		if (pRecord->u16_PartType == u16_PartType)
			return pRecord;

		pRecord++;
	}

	return (void*)0;
}

U32 __ATTR_DRAM_CODE__ drvNAND_ReadBlk(U8 *pu8_DestAddr, U8 u8_PartType, U16 u16_PBA,
							U16 u16_LBA, U32 u32_StartSector, U32 u32_SectorCnt)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	U8 *au8_PageBuf = gpu8_PageDataBuf;
	#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	U8	*au8_SpareBuf = gpu8_SpareBuf;
	#else
	U8	*au8_SpareBuf = NULL;
	#endif

	U32 u32_Err;
	//BLK_INFO_t *pBlkInfo = (BLK_INFO_t*)au8_SpareBuf;
	U16 u16_PageSectorCnt;
	U8 u8_PageSectorCntBits;
	U16 u16_PageSectorCntMask;
	U32 u32_PageIdx;
	U32 u32_PageIdxInBlk;
	U16 u16_PageCnt;
	U16 u16_SectorCntRead;
	U16 u16_SectorIdxInPage;

	//u8_PartType = u8_PartType;
	//u16_LBA = u16_LBA;

	/* sector size from AP is 512 Bytes, not NAND sector size */
	u16_PageSectorCnt = pNandDrv->u16_PageByteCnt >> 9;
	u8_PageSectorCntBits = drvNAND_CountBits(u16_PageSectorCnt);
	u16_PageSectorCntMask = (1<<u8_PageSectorCntBits)-1;

	/* Read unaligned sectors first */
	if( u32_StartSector & u16_PageSectorCntMask )
	{
		u16_SectorIdxInPage = u32_StartSector & u16_PageSectorCntMask;
		u16_SectorCntRead = u32_SectorCnt > ((U32)u16_PageSectorCnt-(U32)u16_SectorIdxInPage) ? ((U32)u16_PageSectorCnt-(U32)u16_SectorIdxInPage) : (u32_SectorCnt);
		u32_PageIdxInBlk = (u32_StartSector>>u8_PageSectorCntBits);
		u32_PageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u32_PageIdxInBlk].u16_LSB;
		#if defined(ENABLE_CACHE) && ENABLE_CACHE
		//invalidate cache from memcpy
		nand_flush_cache((U32)au8_PageBuf, pNandDrv->u16_PageByteCnt);
		#endif
		//u32_Err = NC_ReadPages(u32_PageIdx, au8_PageBuf, (U8*)au8_SpareBuf, 1);
		u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_PageBuf, au8_SpareBuf, pNandDrv->u16_PageSectorCnt);
		if (u32_Err != UNFD_ST_SUCCESS) {
			return u32_Err;
		}
		u16_SectorIdxInPage = u32_StartSector & u16_PageSectorCntMask;
		u16_SectorCntRead = u32_SectorCnt > ((U32)u16_PageSectorCnt-(U32)u16_SectorIdxInPage) ? ((U32)u16_PageSectorCnt-(U32)u16_SectorIdxInPage) : (u32_SectorCnt);

		memcpy(pu8_DestAddr, (au8_PageBuf+(u16_SectorIdxInPage<<9)), (u16_SectorCntRead<<9));
		#if defined(ENABLE_CACHE) && ENABLE_CACHE
		//flush memcpy data from cache to dram 
		nand_flush_cache((U32)pu8_DestAddr, (u16_SectorCntRead<<9));
		#endif
		pu8_DestAddr += (u16_SectorCntRead<<9);
		u32_StartSector += u16_SectorCntRead;
		u32_SectorCnt -= u16_SectorCntRead;
	}

	/* Read aligned sectors then */
	u32_PageIdxInBlk = (u32_StartSector>>u8_PageSectorCntBits);
	u32_PageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) +ga_tPairedPageMap[u32_PageIdxInBlk].u16_LSB;
	u16_PageCnt = (u32_SectorCnt>>u8_PageSectorCntBits);

	while(u16_PageCnt>0)
	{
		u32_PageIdxInBlk = (u32_StartSector>>u8_PageSectorCntBits);
		u32_PageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u32_PageIdxInBlk].u16_LSB;

		//u32_Err = NC_ReadPages(u32_PageIdx, pu8_DestAddr, (U8*)au8_SpareBuf, 1);
		u32_Err = NC_ReadSectors(u32_PageIdx, 0, pu8_DestAddr, au8_SpareBuf, pNandDrv->u16_PageSectorCnt);
		if (u32_Err != UNFD_ST_SUCCESS) {
			return u32_Err;
		}
		#if 0
		pu8_DestAddr += (pNandDrv->u16_PageByteCnt*u16_PageCnt);
		u32_PageIdx += u16_PageCnt;
		u32_StartSector += (u16_PageSectorCnt*u16_PageCnt);
		u32_SectorCnt -= (u16_PageSectorCnt*u16_PageCnt);
		#else
		pu8_DestAddr += pNandDrv->u16_PageByteCnt;
		u32_StartSector += u16_PageSectorCnt;
		u32_SectorCnt -= u16_PageSectorCnt;
		u16_PageCnt --;
		#endif

	}

	/* Read remaining unaligned sectors finally */
	if( u32_SectorCnt )
	{
		u32_PageIdxInBlk = (u32_StartSector>>u8_PageSectorCntBits);
		u32_PageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u32_PageIdxInBlk].u16_LSB;
		#if defined(ENABLE_CACHE) && ENABLE_CACHE
		//invalidate cache from memcpy
		nand_flush_cache((U32)au8_PageBuf, pNandDrv->u16_PageByteCnt);
		#endif
		//u32_Err = NC_ReadPages(u32_PageIdx, au8_PageBuf, (U8*)au8_SpareBuf, 1);
		u32_Err = NC_ReadSectors(u32_PageIdx, 0, au8_PageBuf, au8_SpareBuf, pNandDrv->u16_PageSectorCnt);
		if (u32_Err != UNFD_ST_SUCCESS) {
			return u32_Err;
		}
		memcpy(pu8_DestAddr, au8_PageBuf, (u32_SectorCnt<<9));
		#if defined(ENABLE_CACHE) && ENABLE_CACHE
		//flush memcpy data from cache to dram 
		nand_flush_cache((U32)pu8_DestAddr, (u32_SectorCnt<<9));
		#endif
	}

	return UNFD_ST_SUCCESS;
}

U32 __ATTR_DRAM_CODE__ drvNAND_ReadPartition(U16 u16_PartType, U32 u32_StartPhySector,
						   U32 u32_SrcPhyAddr, U32 u32_SectorCnt )
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	PARTITION_INFO_t *pPartInfo = (PARTITION_INFO_t *)gpu8_PartInfo;
	PARTITION_RECORD_t *pRecord = pPartInfo->records;

	U32 u32_Err = UNFD_ST_SUCCESS;
	U16 u16_BlkIdx;
	U16 u16_i;
	U16 u16_LBA;
	U32 u32_StartSector;
	U32 u32_SectorCntRead;
	U16 u16_PageSectorCnt;
	U8 u8_PageSectorCntBits;
	//U8 u8_BlkSectorCntBits;
	U16 u16_BlkSectorCnt;
	//U16 u16_BlkSectorCntMask;
	U8 u8_BlkSectorCntBits;
	U16 u16_BlkPageCnt;

	if(gu8_PniInit == 0)
	{
		u32_Err = UNFD_ST_ERR_INVALID_PART;
		goto READPARTITION_END;
	}

	u16_BlkPageCnt = pNandDrv->u16_BlkPageCnt>>(pNandDrv->u8_CellType);

	pRecord = drvNAND_SearchPartition(pRecord, u16_PartType);
	if (!pRecord)
	{
		u32_Err = UNFD_ST_ERR_INVALID_PART;
		goto READPARTITION_END;
	}

	if((u32_StartPhySector+u32_SectorCnt) >
		(((((U32)pRecord->u16_BlkCnt+(U32)pRecord->u16_BackupBlkCnt)*(U32)u16_BlkPageCnt)<<(U32)pNandDrv->u8_PageByteCntBits)>>9))
	{
		u32_Err = UNFD_ST_ERR_INVALID_PARAM;
		goto READPARTITION_END;
	}

	/* sector size from AP is 512 Bytes, not NAND sector size */
	u16_PageSectorCnt = pNandDrv->u16_PageByteCnt >> 9;
	u8_PageSectorCntBits = drvNAND_CountBits(u16_PageSectorCnt);
	//u16_PageSectorCntMask = (1<<u8_PageSectorCntBits)-1;
	u8_BlkSectorCntBits = (pNandDrv->u8_BlkPageCntBits + u8_PageSectorCntBits)>>pNandDrv->u8_CellType;
	u16_BlkSectorCnt = (1<<u8_PageSectorCntBits) * u16_BlkPageCnt;

	u16_LBA = u32_StartPhySector>>u8_BlkSectorCntBits;
	/* Find physical BlkIdx for LBA */
	u16_BlkIdx = pRecord->u16_StartBlk;

	for(u16_i=0 ; u16_i<=u16_LBA; u16_i++ )
	{
		while (!drvNAND_IsGoodBlk(u16_BlkIdx))
		{
			if((++u16_BlkIdx)>=(pRecord->u16_StartBlk+pRecord->u16_BlkCnt+pRecord->u16_BackupBlkCnt))
			{
				u32_Err = UNFD_ST_ERR_LACK_BLK;
				goto READPARTITION_END;
			}
		}
		u16_BlkIdx++;
	}

	u16_BlkIdx--;

	while(u32_SectorCnt>0)
	{
		while (!drvNAND_IsGoodBlk(u16_BlkIdx) )
		{
			if((++u16_BlkIdx)>=(pRecord->u16_StartBlk + pRecord->u16_BlkCnt + pRecord->u16_BackupBlkCnt))
			{
				u32_Err = UNFD_ST_ERR_LACK_BLK;
				goto READPARTITION_END;
			}
		}

		u16_LBA = u32_StartPhySector>>u8_BlkSectorCntBits;
		u32_StartSector = u32_StartPhySector&(u16_BlkSectorCnt-1);
		u32_SectorCntRead = u32_SectorCnt > (u16_BlkSectorCnt - u32_StartSector) ?
							(u16_BlkSectorCnt - u32_StartSector) : u32_SectorCnt;

		u32_Err = drvNAND_ReadBlk((U8*)u32_SrcPhyAddr, pRecord->u16_PartType,
								   u16_BlkIdx, u16_LBA, u32_StartSector,
								   u32_SectorCntRead);

		if (u32_Err != UNFD_ST_SUCCESS)
		{
			goto READPARTITION_END;
		}

		u32_SectorCnt -= u32_SectorCntRead;
		u32_StartPhySector += u32_SectorCntRead;
		u32_SrcPhyAddr += (u32_SectorCntRead<<9);
		u16_BlkIdx++;
	}

READPARTITION_END:
	return u32_Err;
}

U32 __ATTR_DRAM_CODE__ nand_set_partinfo (U8 *pu8_PartInfo)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	PARTITION_INFO_t *pPartInfo;
	U32 u32_ChkSum = 0;
	U32 u32_Err;
	U32 u32_i;
	#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	U8	*au8_SpareBuf = gpu8_SpareBuf;
	#else
	U8	*au8_SpareBuf = NULL;
	#endif

	#if defined(CONFIG_TEE_LOADER)
	// Load paired page table first
	#if defined(SUPPORT_3D_NAND) && SUPPORT_3D_NAND
	if(pNandDrv->u8_CellType && pNandDrv->u8_SlcMode == 0)
	#else
	if(pNandDrv->u8_CellType)
	#endif
	{
		#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
		u32_Err = NC_ReadSectors(pNandDrv->u32_CISRow + pNandDrv->u8_PairPageMapLoc, 0, (U8*)ga_tPairedPageMap, NULL, 2);
		#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
		u32_Err = NC_ReadSectors(pNandDrv->u32_CISRow + pNandDrv->u8_PairPageMapLoc, 0, (U8*)ga_tPairedPageMap, gpu8_SpareBuf, 2);
		#endif
		if(u32_Err != UNFD_ST_SUCCESS)
		{
			//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD010);
			print_err(UNFD_ST_ERR_NO_CIS);
			return UNFD_ST_ERR_NO_CIS;
		}
	}
	else
	{
		for(u32_i = 0; u32_i < pNandDrv->u16_BlkPageCnt; u32_i ++)
		{
			ga_tPairedPageMap[u32_i].u16_LSB = u32_i;
		}
	}
	#endif

	if(pu8_PartInfo)
		memcpy(gpu8_PartInfo, pu8_PartInfo, NAND_PARTITAION_BYTE_CNT);
	else
	{
		u32_Err = NC_ReadSectors(pNandDrv->u32_CISRow + ga_tPairedPageMap[1].u16_LSB, 0, (U8*)gpu8_PartInfo, au8_SpareBuf, 1);
		if(u32_Err != UNFD_ST_SUCCESS)
		{
			return UNFD_ST_ERR_CIS_PART_ERR;
		}
	}
	pPartInfo = (PARTITION_INFO_t *)gpu8_PartInfo;
	for(u32_i=0 ; u32_i<(0x200-0x04) ; u32_i++)
		u32_ChkSum += gpu8_PartInfo[0x04+u32_i];
	if (u32_ChkSum != pPartInfo->u32_ChkSum)
	{
		return UNFD_ST_ERR_CIS_PART_ERR;
	}

	gu8_PniInit = 1;
    return UNFD_ST_SUCCESS;
}
#endif

/*************************************************************************
This new API is created for certain chip and cannot modify drvNAND_LoadBL
to achieve functionality becasue not enough code space in some chips
*************************************************************************/
U32 drvNAND_LoadHash0(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size)
{
	NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
	U32	u32_Err = UNFD_ST_SUCCESS;
	U32	u32_SectorCnt;
	U32	u32_i;
	U32	u32_j;
	U32	u32_TmpSectorCnt = 0;
	U32	u32_Row;
	U32	u32_SectorIdx;
	U8	*pu8_PBA;
    #if !(defined(NC_SEL_FCIE5) && NC_SEL_FCIE5)
	U8	au8_SpareData[2];
    #endif
	U8	*pu8_SpareData;
	U8	u8_BootStageId = BOOT_HASH0;

	if(pNandDrv->u8_BL0PBA)
	    return UNFD_ST_ERR_INVALID_PARAM;

	#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
	REG_CLR_BITS_UINT16(NC_REG_PAD_SWITCH, BIT3);
	#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
	REG_WRITE_UINT16(NC_BOOT_MODE, 0);
	REG_CLR_BITS_UINT16(NC_FUN_CTL, BIT_BOOT_MODE);
	#endif

	// Load paired page table first
	#if defined(SUPPORT_3D_NAND) && SUPPORT_3D_NAND
	if(pNandDrv->u8_CellType && pNandDrv->u8_SlcMode == 0)
	#else
	if(pNandDrv->u8_CellType)
	#endif
	{
		#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
		u32_Err = NC_ReadSectors(pNandDrv->u32_CISRow + pNandDrv->u8_PairPageMapLoc, 0, (U8*)ga_tPairedPageMap, NULL, 2);
		#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
		u32_Err = NC_ReadSectors(pNandDrv->u32_CISRow + pNandDrv->u8_PairPageMapLoc, 0, (U8*)ga_tPairedPageMap, gpu8_SpareBuf, 2);
		#endif
		if(u32_Err != UNFD_ST_SUCCESS)
		{
			//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD010);
			return UNFD_ST_ERR_NO_CIS;
		}
	}
	else
	{
		for(u32_i = 0; u32_i < pNandDrv->u16_BlkPageCnt; u32_i ++)
		{
			ga_tPairedPageMap[u32_i].u16_LSB = u32_i;
		}
	}

	pu8_PBA = &pNandDrv->u8_HashPBA[0][0];

	u32_SectorCnt = (u32_Size) >> pNandDrv->u8_SectorByteCntBits;
	if(u32_Size & (pNandDrv->u16_SectorByteCnt - 1))
		u32_SectorCnt += 1;

	while(1)
	{

		//prepare dram dma address for spare of FCIE5
		#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
		pu8_SpareData = gpu8_SpareBuf;
        #else
        pu8_SpareData = au8_SpareData;
		#endif
		u32_Err = NC_ReadSectors((*pu8_PBA) << pNandDrv->u8_BlkPageCntBits,
								0,
								pu8DMAAddr,
								pu8_SpareData,
								1);

		if(u32_Err==UNFD_ST_SUCCESS && pu8_SpareData[0]==0xFF && pu8_SpareData[1]==u8_BootStageId)
		{
			for(u32_i = 0; u32_i < u32_SectorCnt; )
			{
				u32_SectorIdx = (((u32_Offset >> pNandDrv->u8_SectorByteCntBits) + u32_i) & (pNandDrv->u16_PageSectorCnt - 1));

				//calculate page index in a block for pair page map
				u32_j = (u32_i + (u32_Offset >> pNandDrv->u8_SectorByteCntBits)) >> pNandDrv->u8_PageSectorCntBits;

				u32_Row = (*pu8_PBA) << pNandDrv->u8_BlkPageCntBits;

				u32_TmpSectorCnt = ((u32_i + pNandDrv->u16_PageSectorCnt) < u32_SectorCnt)
                                            ? (pNandDrv->u16_PageSectorCnt-u32_SectorIdx) : (u32_SectorCnt - u32_i);
				u32_Row += ga_tPairedPageMap[u32_j].u16_LSB;
				//prepare dram dma address for spare of FCIE5
				#if defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
				pu8_SpareData = gpu8_SpareBuf;
				u32_Err = NC_ReadSectors(u32_Row,
										u32_SectorIdx,
										pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
										pu8_SpareData,
										u32_TmpSectorCnt);

				#else
				u32_Err = NC_ReadSectors(u32_Row,
										u32_SectorIdx,
										pu8DMAAddr + (u32_i << pNandDrv->u8_SectorByteCntBits),
										NULL,
										u32_TmpSectorCnt);
				#endif
				u32_i += u32_TmpSectorCnt;

				#if defined(NC_SEL_FCIE3) && NC_SEL_FCIE3
				if( (u32_Err != UNFD_ST_SUCCESS) || (REG(NC_CTRL)&BIT_NC_ALLONE_FLAG))
				#elif defined(NC_SEL_FCIE5) && NC_SEL_FCIE5
				if( (u32_Err != UNFD_ST_SUCCESS) || (REG(NC_STAT_CHK)&BIT_NC_ALLONE_FLAG))
				#endif
					goto RETRY_READ;
			}

			goto EXIT;
		}

RETRY_READ:
		if((*pu8_PBA)==NAND_HASH_END_BLK)
		{
			//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD110);
			return UNFD_ST_ERR_NO_BL;
		}

		(*pu8_PBA)++;
	}

EXIT:
	//REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD200);
	return u32_Err;
}
