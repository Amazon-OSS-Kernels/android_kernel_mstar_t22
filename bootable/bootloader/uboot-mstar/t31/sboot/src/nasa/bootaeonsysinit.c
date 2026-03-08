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
/*------------------------------------------------------------------------------*/
/*  INCLUDE                                                                     */
/*------------------------------------------------------------------------------*/
#include "datatype.h"
#include "hwreg.h"
#include "drvGlobal.h"
#include "drvAeonUART.h"
#include "risc32_spr.h"
#include "drvPadConf.h"
#include "Board.h"

/*------------------------------------------------------------------------------*/
/*  VARAIBLES DECLARED IN LINKER SCRIPT                                         */
/*------------------------------------------------------------------------------*/
U16 *risc32_regbase;

extern unsigned char _text_length[];
extern unsigned char loader_2nd_lma[];
extern unsigned char loader_2nd_vma[];

extern unsigned char loader_2nd_len[];

#define BEON_MEM_ADR            0x200000
#define chunk_header_addr       0x20000
#define chunk_header_len        0x80

#define MHAL_DCACHE_SIZE        4096//(8192*4)
#ifdef __AEONR2__
#define MHAL_DCACHE_WAYS        2 // 4
#else
#define MHAL_DCACHE_WAYS        2
#endif
#define MHAL_DCACHE_LINE_SIZE   16

/*------------------------------------------------------------------------------*/
/*  LOCAL FUNCTIONS DECLARATION                                                 */
/*------------------------------------------------------------------------------*/
void load2ndLoader(U32 dst, U32 src, U32 size);

/*------------------------------------------------------------------------------*/
/*  LOCAL FUNCTIONS IMPLEMENTATION                                              */
/*------------------------------------------------------------------------------*/
void SetBdmaSrcDstDevice( U8 SrcDEv,U8 SrcDataWidth, U8 DstDEv , U8 DstDataWidth);
void SetBdmaSrcDstDevice( U8 SrcDEv,U8 SrcDataWidth, U8 DstDEv , U8 DstDataWidth)
{
   MDrv_Write2Byte( (BDMA_REG_BASE+0x04) ,  ( (DstDEv|DstDataWidth)  <<8) |(SrcDataWidth|SrcDEv) );

   //Both with 1 byte data width for FPGA test
}

/*------------------------------------------------------------------------------*/
/*  Copy binary image from SPI to DRAM                                          */
/*------------------------------------------------------------------------------*/
//#define REG8(a)       RIU8[((a) * 2) - ((a) & 1)]
void CopyFLH2MIU(U32 dst, U32 src, U32 size);
void CopyFLH2MIU(U32 dst, U32 src, U32 size)
{
    //printf("SPI %lx -> MIU %lx  length: %lx\n", spi_addr, miu_addr, length);

    while ( RIU8[(BDMA_REG_BASE + 0x02)*2]  & 0x01);   ///queue ==0?
    MDrv_Write4Byte((BDMA_REG_BASE + 0x08 ) , src  );
    MDrv_Write4Byte((BDMA_REG_BASE + 0x0c ),  dst  );
    MDrv_Write4Byte((BDMA_REG_BASE + 0x10 ), (size + 0x0f) & ~0x0fuL);
    MDrv_WriteByte((BDMA_REG_BASE + 0x06),0x00);   //increasing
    MDrv_WriteByte((BDMA_REG_BASE + 0x07),0x02);   //Set Dummy write count
    SetBdmaSrcDstDevice( SPI_FLASH , DW_8BYTE, DRAM_MIU_0, DW_8BYTE);
    MDrv_WriteByte( (BDMA_REG_BASE + 0x00), 0x01 );
    while(1) //for(k=0;k<12000;k++)
    {
        if ( (RIU8[(BDMA_REG_BASE+0x02)*2] & 0x08) == 0x08 )
        {
            return;        //pass
        }
    }
}

void BootFromDRAM(U32 u32BootAddr);
void BootFromDRAM(U32 u32BootAddr)
{
    //set up reset vector base
    MDrv_Write2Byte(0x1002B4, (U16)(u32BootAddr >> 16));
    MDrv_Write2Byte(0x002E54, 0x829f);  // set wtd_mcu_rst passwd
    MDrv_Write2Byte(0x002E52, MDrv_Read2Byte(0x002E52)&(~BIT8));  // non-fire wtd_mcu_rst
    //wdt reset to dram: bellow 2 lines must in the same cache line 256bytes
    MDrv_Write2Byte(0x100280, MDrv_Read2Byte(0x100280)|(0x002F));  // set sdr_boot=1, release rstz_miu
    MDrv_Write2Byte(0x002E52, MDrv_Read2Byte(0x002E52)|(BIT8));  // fire wtd_mcu_rst
}

void loadAP(void);
void BL_Init(void);

//#define QMEM     ((unsigned  short volatile *) 0xc0000000ul)    //T2
#define QMEM     ((unsigned  short volatile *) 0xB0000000ul)    //T3

void start(void);
void start(void)
{
    //(1) disable watch dog
    MDrv_Write2Byte(0x3008, 0x0000);
    MDrv_Write2Byte(0x300A, 0x0000);

    //(2) boot initialization
    BL_Init();

    //(3)Pad Init
#if ((MS_BOARD_TYPE_SEL != BD_XL3_MEGA_NASA))
    MDrv_Pad_Init();
#endif

    //(4) copy 2nd loader here
    CopyFLH2MIU( (U32)loader_2nd_vma, (U32)loader_2nd_lma, (U32)loader_2nd_len );

    MDrv_Write2Byte(0x1002AC, 0xA100);  //SPI
    MDrv_Write2Byte(0x100290, 0xA100);  //SPI

    //(5) load and execute application
    BootFromDRAM((U32)loadAP);

    while(1);

}

#define __loader_start __attribute__ ((__section__ (".loader_2nd_start.text")))
#define __loader_2nd __attribute__ ((__section__ (".loader_2nd.text")))
extern void __loader_2nd DoMSDecompression(void);
//########################################
//#
//# Especially use  __loader_start specified section to
//# let it run at address 64Kx N+0x100
//#   Ex:  0x1000000+0x100 (see sboot.lds.S) => N=0x100
//########################################
void __loader_start loadAP(void)
{
	// Enable I-Cache to speed up the load code process
    __asm__ __volatile__ (
        /* Invalidate I-Cache */
        "\tl.addi  r13,r0,0\n"
        "1:\n"
        "\tl.mtspr %2,r13,0\n"
        "\tl.sfne  r13,%3\n"
        #ifdef __AEONR2__
        "\tl.addi  r13,r13,%4\n"
        "\tl.bf    1b\n"
        #else
        "\tl.bf    1b\n"
        "\tl.addi  r13,r13,%4\n"
        #endif

        /* Enable I-Cache */
        "\tl.mfspr r13,%0,0\n"
        "\tl.ori   r13,r13,%1\n"
        "\tl.mtspr %0,r13,0\n"

        /* Flush instructions out of instruction buffer */
        "\tl.nop\n"
        "\tl.nop\n"
        "\tl.nop\n"
        "\tl.nop\n"
        "\tl.nop\n"
        :
        : "r" (SPR_SR), "i" (SPR_SR_ICE), "r" (SPR_ICBIR),
          "r" (8192*2), "i" (16)
        : "r13");

	// Enable D-Cache to speed up decompress
    __asm__ __volatile__ (
        /* Invalidate DC    */
        "\tl.addi  r13,r0,0\n"
        "1:\n"

        "\tl.invalidate_line  0(r13), 0\n"
        "\tl.invalidate_line  0(r13), 1\n"

        "\tl.sfne  r13,%2\n"
        #ifdef __AEONR2__
        "\tl.addi  r13,r13,%3\n"
        "\tl.bf    1b\n"
        #else
        "\tl.bf    1b\n"
        "\tl.addi  r13,r13,%3\n"
        #endif

        /* Enable D-Cache */
        "\tl.mfspr r13,%0,0\n"
        "\tl.ori   r13,r13,%1\n"
        "\tl.mtspr %0,r13,0\n"
        :
        : "r" (SPR_SR), "i" (SPR_SR_DCE),
          "r" ((MHAL_DCACHE_SIZE/MHAL_DCACHE_WAYS) - MHAL_DCACHE_LINE_SIZE),
          "i" (MHAL_DCACHE_LINE_SIZE)
        : "r13");

    //start to do decompress task
    DoMSDecompression();

    #ifdef __AEONR2__
    while(1);
    #else
    __asm__ __volatile__ (
        "\tl.syncwritebuffer\n"
        "\tl.jr     %0\n"
        "\tl.syncwritebuffer\n"
        : : "r" (0x100));
    #endif
}

