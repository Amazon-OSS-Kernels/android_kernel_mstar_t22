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

#include "drvSPINAND.h"
#include "halSPINAND.h"
#include "drvSPINAND_boot.h"
#include "halSPINAND_boot.h"
#if (CONFIG_MSTAR_CHIP_REVISION_U01 == 1)
#define BL2_OFFSET  56*1024
#else
#define BL2_OFFSET  24*1024
#endif
#define CHUNK_HEADER_BINARYID 0x0000B007
#define CHUNK_HEADER_BINARYID_POS  0x7

#define SPINAND_HASH_START_BLK              10
#define SPINAND_HASH_END_BLK                255

#define BOOT_NONSECURE                      0x00
#define BOOT_HASH0                          0x80
#define BOOT_HASH1                          0x81
#define BOOT_HASH2                          0x82
#define LOAD_2K_PAGE_WITH_VERIFY_PATH       1

typedef struct _SPINAND_DRIVER
{
  SPINAND_FLASH_INFO_t tSpinandInfo;

  U8  u8_CISBlk;
  U8  u8_BL0PBA;
  U8  u8_BL1PBA;
  U8  u8_UBOOTPBA;
  U8  u8_HashPBA[3][2];
/*** Please append new member here ***/

} SPINAND_DRIVER, *P_SPINAND_DRIVER;


static U32 CheckBlank(U8 *pu8_Buf, U32 u32_Size)
{
  U16 u16_i;

  for(u16_i = 0 ; u16_i<u32_Size ; u16_i++)
  {
    if(pu8_Buf[u16_i] != 0xFF)
    return 0;
  }
  return 1;
}

static U32 drvNAND_LoadBL(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size, U8 u8_IsDMA)
{
   SPINAND_DRIVER *pSpiNandDrv = (SPINAND_DRIVER *)CONFIG_SRAM_SPINAND_OFFSET;

   U8   u8_SpareData[256];
   U8   *pu8_SpareData = (U8 *)u8_SpareData;
   U32   u32_Err = ERR_SPINAND_SUCCESS;
   U32   u32_i;
   U32   u32_j;

#if LOAD_2K_PAGE_WITH_VERIFY_PATH
   U8  u8_PageSectorCnt;
   U8  u8PageOffset;
   U8  u8PageByteCnt;
   U8  u8PageByteCntTmp;
   U8  u8_BLA;

   u8PageByteCntTmp = MDrv_DIV_To_RShift(pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
   u8_PageSectorCnt = PAGE_SECTOR_CNT;
   u8PageByteCnt = ((u32_Size)>>u8PageByteCntTmp);
   u8PageOffset = ((u32_Offset)>>u8PageByteCntTmp);

   if(u32_Size & (pSpiNandDrv->tSpinandInfo.u16_PageByteCnt - 1))
     u8PageByteCnt += 1;

   if(u32_Offset & (pSpiNandDrv->tSpinandInfo.u16_PageByteCnt - 1))
     u8PageOffset += 1;

   u8_BLA=pSpiNandDrv->tSpinandInfo.u8_HashPBA[1][0];
  // REG_WRITE_UINT16(DEBUG_REG_SYS_L3, u8PageByteCnt);
 //  REG_WRITE_UINT16(DEBUG_REG_SYS_L4, u8_BLA);
   for(u32_i = 0; u32_i < u8PageByteCnt;u32_i++)
   {
       u32_j = u8PageOffset + u32_i;
       u32_Err = MDrv_SPINAND_Read(u8_BLA, u32_j, 0, u8_PageSectorCnt, pu8DMAAddr + (u32_i*PAGE_SIZE), pu8_SpareData);
       if(u32_Err != ERR_SPINAND_SUCCESS)
       return ERR_SPINAND_INVALID;
   }
#else
   U32  u32_SectorCnt;
   U32  u32_TmpSectorCnt = 0;
   U32  u32_SectorIdx;
   U8   *pu8_PBA;
   U8   u8_BootStageId;
   U16  u16SecByteCntTmp;

   u16SecByteCntTmp = MDrv_DIV_To_RShift(pSpiNandDrv->tSpinandInfo.u16_SectorByteCnt);
   u8_PageSectorCnt = (pSpiNandDrv->tSpinandInfo.u16_PageByteCnt)>>u16SecByteCntTmp;
   REG_WRITE_UINT16(DEBUG_REG_SYS_L4, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);

   u32_SectorCnt = (u32_Size)>>u16SecByteCntTmp;

   if(u32_Size & (pSpiNandDrv->tSpinandInfo.u16_SectorByteCnt - 1))
     u32_SectorCnt += 1;

   if(pSpiNandDrv->u8_BL0PBA)
   {
      pu8_PBA = &pSpiNandDrv->u8_BL0PBA;
      u8_BootStageId = BOOT_NONSECURE ;
   }
   else
   {
      pu8_PBA = &pSpiNandDrv->u8_HashPBA[1][0];
      u8_BootStageId = BOOT_HASH1;
   }

   while(1)
   {
      if(u8_IsDMA)
      {
         u32_Err = MDrv_SPINAND_Read((*pu8_PBA), 0, 0, 1, pu8DMAAddr, pu8_SpareData);
      }
      #if defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE
      else
      {
      // TODO
      }
      #endif
      if(u32_Err==ERR_SPINAND_SUCCESS && pu8_SpareData[0]==0xFF && pu8_SpareData[4]==u8_BootStageId)
      {
          for(u32_i = 0; u32_i < u32_SectorCnt; )
          {
              u32_SectorIdx = (((u32_Offset >> u16SecByteCntTmp) + u32_i)& (u8_PageSectorCnt - 1));
              //calculate page index in a block for pair page map
              u32_j = (u32_i + (u32_Offset >> u16SecByteCntTmp))>>u16SecByteCntTmp;
              if(u8_IsDMA)
              {
                  u32_TmpSectorCnt = ((u32_i + u8_PageSectorCnt) < u32_SectorCnt)?(u8_PageSectorCnt-u32_SectorIdx) : (u32_SectorCnt - u32_i);
                  u32_Err = MDrv_SPINAND_Read((*pu8_PBA), u32_j, u32_SectorIdx, u32_TmpSectorCnt,pu8DMAAddr + (u32_i * pSpiNandDrv->tSpinandInfo.u16_SectorByteCnt), pu8_SpareData);
                  u32_i += u32_TmpSectorCnt;
              }
#if defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE
              else
             {
                  // TODO
             }
#endif
             if(u32_Err != ERR_SPINAND_SUCCESS)
               goto RETRY_READ_BL;
          }
          goto EXIT_BL;
       }
       RETRY_READ_BL:
       if((*pu8_PBA)==SPINAND_HASH_END_BLK)
       {
           //REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD110);
           return ERR_SPINAND_INVALID;
       }
       (*pu8_PBA)++;
    }
    EXIT_BL:
    REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD200);
#endif
    return u32_Err;
}

/*
* This api loads the 2nd BL in the same block of the BL0_PBA or BL1_PBA from offset 32KB
*
*/

#if defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE
U32 drvNAND_LoadBLOffset(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size)
{
    return(drvNAND_LoadBL(pu8DMAAddr, u32_Offset, u32_Size, 0));
}
#endif

/*
* This api loads the 2nd BL in the same block of the BL0_PBA or BL1_PBA from offset 32KB
*
*/

U32 drvNAND_Load2ndBL(U8* pu8DMAAddr, U32 u32_Size)
{
   return(drvNAND_LoadBL(pu8DMAAddr, BL2_OFFSET, u32_Size, 1));
}

// in order to reduce nand code size in sram, we define a section in dram
// __ATTR_DRAM_CODE__ is defined in drvNAND_xxx_sboot.h
// section is added in sboot.lds.S
#ifndef __ATTR_DRAM_CODE__
#define __ATTR_DRAM_CODE__
#endif
U32 __ATTR_DRAM_CODE__  drvNAND_LoaduBoot(U8* pu8DMAAddr, U32 u32_BLSize)
{
   SPINAND_DRIVER *pSpiNandDrv = (SPINAND_DRIVER *)CONFIG_SRAM_SPINAND_OFFSET;
   U8  u8_SpareData[256];
   U8 *pu8_SpareData = (U8 *)u8_SpareData;
   U32 u32_Err = ERR_SPINAND_SUCCESS;
   U32 u32_i;
   U8  u8_PageSectorCnt;

   REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD000);
#if LOAD_2K_PAGE_WITH_VERIFY_PATH
   U16  u16PageByteCnt;
   U8  u8PageByteCntTmp;
   U8  u8_BLA;
   U8  u8BlockPageIdx;

   u8PageByteCntTmp = MDrv_DIV_To_RShift(pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
   u8_PageSectorCnt = PAGE_SECTOR_CNT;
   u16PageByteCnt = ((u32_BLSize)>>u8PageByteCntTmp);

   if(u32_BLSize&(pSpiNandDrv->tSpinandInfo.u16_PageByteCnt - 1))
   u16PageByteCnt += 1;

   u8_BLA = u8_BLA=pSpiNandDrv->tSpinandInfo.u8_HashPBA[2][0];

search_next_block:
   for(u32_i = 0; u32_i < u16PageByteCnt;u32_i++)
   {
     u8BlockPageIdx = u32_i & (BLOCK_SIZE - 1);
     if((u32_i)&&(u8BlockPageIdx==0))
     {
       u8_BLA = u8_BLA + 1;
     }
     if(u32_i == 0)
     {
         u32_Err = MDrv_SPINAND_Read(u8_BLA, 0 , 0, u8_PageSectorCnt, pu8DMAAddr + (u32_i*PAGE_SIZE), pu8_SpareData);
         if( ((U32*)(pu8DMAAddr))[CHUNK_HEADER_BINARYID_POS] != CHUNK_HEADER_BINARYID)
         {
            u8_BLA = u8_BLA + 1;
            if(u8_BLA==SPINAND_HASH_END_BLK)
            {
                return ERR_SPINAND_INVALID;
            }
            goto search_next_block;
         }
     }
     else
     {
        u32_Err = MDrv_SPINAND_Read(u8_BLA, u8BlockPageIdx, 0, u8_PageSectorCnt, pu8DMAAddr + (u32_i*PAGE_SIZE), pu8_SpareData);
     }

     if(u32_Err != ERR_SPINAND_SUCCESS)
     {
        REG_WRITE_UINT16(DEBUG_REG, 0xD109);
        return ERR_SPINAND_INVALID;
     }
   }
#else
    U32 u32_j;
    U32 u32_SectorCnt;
    U32 u32_TmpSectorCnt;
    U8  u8_PBA;
    U16 u16_BlkPageCnt;
    U16 u16SecByteCntTmp;

    u16SecByteCntTmp = MDrv_DIV_To_RShift(pSpiNandDrv->tSpinandInfo.u16_SectorByteCnt);
    u8_PageSectorCnt = (pSpiNandDrv->tSpinandInfo.u16_PageByteCnt)>>u16SecByteCntTmp;
    u16_BlkPageCnt = pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    u32_SectorCnt = (u32_BLSize)>>u16SecByteCntTmp;

    if(u32_BLSize & (pSpiNandDrv->tSpinandInfo.u16_SectorByteCnt - 1))
      u32_SectorCnt += 1;

    REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD100);

    //1st uboot locate at the next good block of BL1PBA
    u8_PBA = pSpiNandDrv->u8_BL0PBA != 0 ? pSpiNandDrv->u8_BL0PBA : pSpiNandDrv->u8_HashPBA[2][0]-1;

    RETRY_READ_UBOOT2:
    for(u32_i = 0; u32_i < u32_SectorCnt; u32_i += u8_PageSectorCnt)
    {
       u32_TmpSectorCnt = ((u32_i + u8_PageSectorCnt) < u32_SectorCnt)?u8_PageSectorCnt : (u32_SectorCnt - u32_i);

       u32_j = (u32_i & (u8_PageSectorCnt * u16_BlkPageCnt - 1))>>(MDrv_DIV_To_RShift(u8_PageSectorCnt));

       if(u32_j == 0 )
       {
           search_next_block:
           u8_PBA ++;
           // Read 1st sector for checking good block
           u32_Err = MDrv_SPINAND_Read(u8_PBA, 0, 0, u32_TmpSectorCnt, pu8DMAAddr + (u32_i * pSpiNandDrv->tSpinandInfo.u16_SectorByteCnt), pu8_SpareData);
           // Check the bad block infomation
           if( pu8_SpareData[0] != 0xFF  || u32_Err != ERR_SPINAND_SUCCESS)
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
           u32_Err = MDrv_SPINAND_Read(u8_PBA, u32_j, 0, u32_TmpSectorCnt, pu8DMAAddr + (u32_i * pSpiNandDrv->tSpinandInfo.u16_SectorByteCnt), pu8_SpareData);
       }

       if(u32_Err != ERR_SPINAND_SUCCESS)
       {
           if(u8_PBA==SPINAND_HASH_END_BLK)
             return ERR_SPINAND_INVALID;

           goto RETRY_READ_UBOOT2;
       }
    }
    #endif
    REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xD200);
    return u32_Err;
}
