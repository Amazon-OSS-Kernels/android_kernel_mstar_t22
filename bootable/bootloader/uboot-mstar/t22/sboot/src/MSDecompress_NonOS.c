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

#if (ENABLE_NON_OS)

//---------------------------------------------------------------------------
// Fix undefined warning
#ifndef ENABLE_MSTAR_MARIA10
    #define ENABLE_MSTAR_MARIA10    0
#endif
#ifndef ENABLE_MSTAR_MACAW12
    #define ENABLE_MSTAR_MACAW12    0
#endif
#ifndef ENABLE_MSTAR_EDEN
    #define ENABLE_MSTAR_EDEN       0
#endif
#ifndef ENABLE_MSTAR_EULER
    #define ENABLE_MSTAR_EULER      0
#endif
#ifndef ENABLE_MSTAR_NASA
    #define ENABLE_MSTAR_NASA       0
#endif

#ifndef ENABLE_MSTAR_EMERALD
    #define ENABLE_MSTAR_EMERALD    0
#endif

//---------------------------------------------------------------------------
#include "zlib.h"
#include "MSDecompress_NonOS_priv.h"
//#include "version.h"

//---------------------------------------------------------------------------
#if (ENABLE_MSTAR_MARIA10)
    #include "maria10/hwreg.h"
#elif (ENABLE_MSTAR_MACAW12)
    #include "macaw12/hwreg.h"
#elif (ENABLE_MSTAR_EDEN)
    #include "eden/hwreg.h"
#elif (ENABLE_MSTAR_MAYA)
    #include "maya/hwreg.h"
#elif (ENABLE_MSTAR_EULER)
    #include "euler/hwreg.h"
#elif (ENABLE_MSTAR_WHISKY)
    #include "whisky/hwreg.h"
#elif (ENABLE_MSTAR_MELODY)
    #include "melody/hwreg.h"
#elif (ENABLE_MSTAR_NASA)
    #include "nasa/hwreg.h"

#elif (ENABLE_MSTAR_MILAN)
    #include "milan/include/hwreg_MILAN.h"
#elif (ENABLE_MSTAR_MARLON)
    #include "marlon/include/hwreg_MARLON.h"
#elif (ENABLE_MSTAR_MACAN)
    #include "macan/include/hwreg_macan.h"
#elif (ENABLE_MSTAR_MAZDA)
    #include "mazda/include/hwreg_mazda.h"
#elif (ENABLE_MSTAR_M3822)
    #include "M3822/include/hwreg_M3822.h"
#else

#endif

//---------------------------------------------------------------------------
// Type define
typedef unsigned char   U8;
typedef unsigned short  U16;
typedef unsigned int    U32;

/// data type unsigned char, data length 1 byte
typedef unsigned char               MS_U8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              MS_U16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned long               MS_U32;                             // 4 bytes

#ifndef BOOL
typedef unsigned int BOOL;
#endif

/// defination for FALSE
#define FALSE           0
/// defination for TRUE
#define TRUE            1

#define NULL    0

#define BIT0    (0x0001)
#define BIT1    (0x0002)
#define BIT2    (0x0004)
#define BIT3    (0x0008)
#define BIT4    (0x0010)
#define BIT5    (0x0020)
#define BIT6    (0x0040)
#define BIT7    (0x0080)

//---------------------------------------------------------------------------

#define ENABLE_PRINTF   1

#if( ENABLE_PRINTF )

    #if defined(__aeon__)// AEON_R2
        #if 0
            #define ENABLE_AEON_UART_PRINTF     1
        #elif( ENABLE_MSTAR_WHISKY )
            #define ENABLE_LOADER2_AEON_UART    1
            #define ENABLE_SIMPLE_PRINTF        1
        #endif
    #elif defined(__mips__) // MIPS
      #if ( ENABLE_MSTAR_MILAN || ENABLE_MSTAR_MARLON) // Milan/Marlon use PIU Uart
        #define ENABLE_SIMPLE_PRINTF  1
      #endif
    #else //ARM
        #define ENABLE_SIMPLE_PRINTF  1
    #endif

#endif


#ifndef ENABLE_AEON_UART_PRINTF // Use system uart driver & printf()
    #define ENABLE_AEON_UART_PRINTF    0
#endif

#ifndef ENABLE_LOADER2_AEON_UART // Use ld_uart driver
    #define ENABLE_LOADER2_AEON_UART    0
#endif

#ifndef ENABLE_SIMPLE_PRINTF // Enable uart_printf
    #define ENABLE_SIMPLE_PRINTF  0
#endif

//---------------------------------------------------------------------------

#if 1//(ENABLE_SBOOT_USE_BDMA_TO_CHECK_CRC)
#define USE_BDMA_FOR_CRC_CHECK 1
#endif

#define UBOOT_IDENTIFIER    0x0000B007

//---------------------------------------------------------------------------

#if ENABLE_MSTAR_WHISKY
    #define ENABLE_JOBA_BIN 1
#else
    #define ENABLE_JOBA_BIN 0
#endif

#if(ENABLE_JOBA_BIN)
    #define JOBA_MMAP_CODE_ADDR     0x020000

    //#define JOBA_MMAP_DEBUG_ADDR    0x0F0000
    #define JOBA_PARA_MAGIC_ID      0x985598AA
#endif

//---------------------------------------------------------------------------

#if defined(__mips__)
    #define RIU_MAP 0xBF200000
    #define LOCAL_BDMA_REG_BASE 0x900
#elif defined(__aeon__)
    #define RIU_MAP 0xA0000000
#elif defined(__arm__) || defined(__arm64__)
    #define RIU_MAP 0x1F000000
    #define LOCAL_BDMA_REG_BASE    0x100900
    #define USE_BDMA_MOVE_CODE 1
#else
    #error not define
#endif

#if( defined(__aeon__) || defined(__mips__) || (defined(__arm__) || defined(__arm64__)))
    #define RIU     ((unsigned short volatile *) RIU_MAP)
    #define RIU8    ((unsigned char  volatile *) RIU_MAP)
#endif

//---------------------------------------------------------------------------


//====================================================================================

#if defined(__mips__) || (defined(__arm__) || defined(__arm64__))
    #define __loader_2nd
    #define __loader_2nd_bss
#else
    #define __loader_2nd     __attribute__ ((__section__ (".loader_2nd.text")))
    #define __loader_2nd_bss __attribute__ ((__section__ (".loader_2nd.bss")))
#endif

//====================================================================================

unsigned char __loader_2nd_bss *pDecompressionBuf = NULL;
unsigned char __loader_2nd_bss *pEndofDecompressionBuf = NULL;
unsigned char __loader_2nd_bss *pCurrentBufPtr = NULL;

void MemoryAllocatorInit(unsigned char *ptr, unsigned long nSize);
extern void __BDMA_FlashCopy2MIU0(U32 FlashPA, U32 DRAMPA, U32 Len);
extern void __BDMA_FlashCopy2MIU0_No_Wait(U32 FlashPA, U32 DRAMPA, U32 Len);
//extern void __BDMA_FlashCopy2MIU0_WaitDone(void);
extern BOOL __BDMA_Is_JobDone(void);

extern void mhal_dcache_flush(U32 u32Base, U32 u32Size);
void *zalloc(void *ptr, unsigned long nNum, unsigned long nSize);
void zfree(void *x, void *address, unsigned nbytes);
U8 MsDecompress(U8 *pSrc, U8 *pDst, U32 srclen, U32 dstlen, U32 *pDecompressionLength);
void DoMSDecompression(void);

void __loader_2nd Print_Buffer(U8* pu8Buf, U32 u32Size );
void __loader_2nd uart_printf(const char *pFmt, U32 dwVal);


//--------------------------------------------------------------------------------------------

//================================================================================

/*void init_version(void);

void __loader_2nd init_version(void)
{
    //Init version
    static U8 tmpVar;
    tmpVar = _sbt_version.APP.tag[0];
}
*/

void __loader_2nd MemoryAllocatorInit(unsigned char *ptr, unsigned long nSize)
{
   pDecompressionBuf = ptr;
   pEndofDecompressionBuf = pDecompressionBuf + nSize*sizeof(unsigned char);
   pCurrentBufPtr = pDecompressionBuf;
}

void __loader_2nd *zalloc(void *ptr, unsigned long nNum, unsigned long nSize)
{
// Because C language use call by value
// so if we modify ptr value, it will not afftect the caller value
// Considering decompression performance, we don't do memory pool
// region check here
#if 1
    if (pCurrentBufPtr >= pEndofDecompressionBuf)
    {
        //printf("Memory Allocate Fail\n");
        ptr = NULL;
    }

    nSize = ( nSize*nNum + 3 ) & ~3;

    ptr = (void *)pCurrentBufPtr;
    pCurrentBufPtr += nSize;

#else
    ptr = (void *)pCurrentBufPtr;
    // 4' alignment
    pCurrentBufPtr += ( nSize*nNum + 3 ) & ~3;
#endif

    return ptr;
}

void __loader_2nd zfree(void *x, void *address, unsigned nbytes)
{
    // Considering decompression performance, we don't
    // do memory free operation here
    // This will cause a lot of memory usage, but I think we can
    // afford this, because our target files are not so big
    x = x;
    address = address;
    nbytes = nbytes;
}

/*inline void _HaltCPU(void)
{
    //halt cpu
    __asm__ __volatile__(
        "\tSDBBP");
}*/

U8 __loader_2nd MsDecompress(U8 *pSrc, U8 *pDst, U32 srclen, U32 dstlen, U32 *pDecompressionLength)
{
    z_stream z;
    z.opaque=NULL;
    z.zalloc = (alloc_func)zalloc;
    z.zfree = zfree;
    z.next_in = pSrc;
    z.avail_in = srclen;
    z.next_out = pDst;
    z.avail_out = dstlen;

    if ( inflateInit2(&z, -MAX_WBITS) != Z_OK )
    {
        //printf("Z Error:%s\n", z.msg);
        inflateEnd(&z);
        return 1;
    }

    if ( inflate(&z, Z_FINISH) < Z_OK )
    {
        //printf("Z Error:%s\n", z.msg);
        inflateEnd(&z);
        return 1;
    }

    *pDecompressionLength = z.total_out;

    inflateEnd(&z);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
/* 2009-08-14 : Igor Pavlov : Public domain */

#include "7types.h"
#include "7alloc.h"
#include "lzmadec.h"

__loader_2nd void *SzAlloc(void *p, size_t size);
__loader_2nd void SzFree(void *p, void *address);
U8 __loader_2nd MsDecompress7(U8 *pSrc, U8 *pDst, U32 srclen);

__loader_2nd void *SzAlloc(void *p, size_t size) { p = p; return MyAlloc(size); }
__loader_2nd void SzFree(void *p, void *address) { p = p; MyFree(address); }
__loader_2nd_bss ISzAlloc g_Alloc = { SzAlloc, SzFree };
/////////////////////////////////////////////////////////////////////////////

#if (ENABLE_BOOTING_FROM_EXT_EMMC_WITH_CPU == 0)
U8 __loader_2nd MsDecompress7(U8 *pSrc, U8 *pDst, U32 srclen)
{

    U32 OriginalFileLength = 0;
    int result = SZ_OK;
    U8 i;
    ELzmaStatus estatus;
    SizeT destLen, srcLen;

    #define LZMA_PROPS_SIZE 5
    estatus=LZMA_STATUS_NOT_SPECIFIED;

    /* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
    srclen -= (LZMA_PROPS_SIZE + 8);

    // It seems that in our system, we never have a chance to have a compressed data which needs to use U64 integer to store its file size
    // So we use just U32 here to instead of U64
    /* Read and parse header */
    for (i = 0; i < 8; i++)
        OriginalFileLength += ( (U32)pSrc[LZMA_PROPS_SIZE + i] << (i << 3) );

    destLen = OriginalFileLength;
    srcLen = srclen;

    result = LzmaDecode(pDst, &destLen, pSrc + (LZMA_PROPS_SIZE + 8), &srcLen, pSrc, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &estatus, &g_Alloc);
    switch( estatus )
    {
    case LZMA_STATUS_FINISHED_WITH_MARK:
    case LZMA_STATUS_NOT_FINISHED:
    case LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK:
        break;
    default:
    //LZMA_STATUS_NOT_SPECIFIED,               /* use main error code instead */
    //LZMA_STATUS_NEEDS_MORE_INPUT,            /* you must provide more input bytes */
        //printf("ELzmaStatus = %d\n", estatus);
        //Decompression fail!
        return FALSE;
    }

    if (result != SZ_OK)
    {
    //  SZ_ERROR_DATA - Data error
    //  SZ_ERROR_MEM  - Memory allocation error
    //  SZ_ERROR_UNSUPPORTED - Unsupported properties
    //  SZ_ERROR_INPUT_EOF - It needs more bytes in input buffer (src).
    //#define SZ_ERROR_DATA 1
    //#define SZ_ERROR_MEM 2
    //#define SZ_ERROR_CRC 3
    //#define SZ_ERROR_UNSUPPORTED 4
    //#define SZ_ERROR_PARAM 5
    //#define SZ_ERROR_INPUT_EOF 6
    //#define SZ_ERROR_OUTPUT_EOF 7
    //#define SZ_ERROR_READ 8
    //#define SZ_ERROR_WRITE 9
    //#define SZ_ERROR_PROGRESS 10
    //#define SZ_ERROR_FAIL 11
    //#define SZ_ERROR_THREAD 12
    //#define SZ_ERROR_ARCHIVE 16
    //#define SZ_ERROR_NO_ARCHIVE 17
        //printf("error code:%d\n", result);
        //Decompression fail!
        return FALSE;
    }

    if ( destLen != OriginalFileLength )
    {
        //Decompression fail!
        return FALSE;
    }

    if ( srcLen != srclen )
    {
        //Decompression fail!
        return FALSE;
    }
    //Decompression OK!


    return TRUE;
}
#endif //#if (ENABLE_BOOTING_FROM_EXT_EMMC_WITH_CPU == 0)

#if( defined(__mips__) || defined(__aeon__) || (defined(__arm__) || defined(__arm64__))) // Non-OS - Milan/Marlon

U8 __loader_2nd msReadByte( U32 u32Reg );
U8 __loader_2nd msReadByte( U32 u32Reg )
{
#if( defined(__aeon__) )
    return RIU8[(u32Reg << 1) - (u32Reg & 1)];
#endif

#if( defined(__mips__) )
    return ((unsigned char volatile *)0xBF000000)[(u32Reg << 1) - (u32Reg & 1)];
#endif

#if(defined(__arm__) || defined(__arm64__))
    //return RIU8[(u32Reg << 1) - (u32Reg & 1)];
    return ((unsigned char volatile *)0x1F000000)[(u32Reg << 1) - (u32Reg & 1)];
#endif
}

U16 __loader_2nd msRead2Bytes( U32 u32Reg );
U16 __loader_2nd msRead2Bytes( U32 u32Reg )
{
#if 0
    if( (u32Reg & 0x01) )
    {
        u32Reg <<= 1;
        return RIU8[u32Reg - 1] + (RIU8[u32Reg + 2] << 8);
    }
    else
    {
        return RIU[u32Reg];
    }
#else
    return (U16)msReadByte(u32Reg)|((U16)msReadByte(u32Reg+1)<<8);

#endif
}

void __loader_2nd msWriteByte( U32 u32Reg, U8 u8Value );
void __loader_2nd msWriteByte( U32 u32Reg, U8 u8Value )
{
#if( defined(__aeon__) )
    RIU8[(u32Reg << 1) - (u32Reg & 1)] = u8Value;
#endif

#if( defined(__mips__) )
    ((unsigned char volatile *)0xBF000000)[(u32Reg << 1) - (u32Reg & 1)] = u8Value;
#endif

#if(defined(__arm__) || defined(__arm64__))
    ((unsigned char volatile *)0x1F000000)[(u32Reg << 1) - (u32Reg & 1)] = u8Value;
#endif
}

void __loader_2nd msWrite2Bytes( U32 u32Reg, U16 u16Value );
void __loader_2nd msWrite2Bytes( U32 u32Reg, U16 u16Value )
{
#if 1
    msWriteByte(u32Reg, u16Value&0xFF);
    msWriteByte(u32Reg+1, u16Value>>8);
#else
    if( ((u32Reg) & 0x01) )
    {
        RIU8[((u32Reg) * 2) - 1] = (U8)((u16Value));                                  \
        RIU8[((u32Reg) + 1) * 2] = (U8)((u16Value) >> 8);                             \
    }
    else
    {
        RIU[u32Reg] = u16Value;                                                       \
    }
#endif
}

#endif

//=================================================================================

#if defined(__aeon__)// AEON_R2
    #include "MSDecompress_NonOS_AeonUart.c"
#endif // Aeon

#include "MSDecompress_NonOS_driver.c"

//=================================================================================

#ifdef __AEONR2__
#if 0//(ENABLE_MSTAR_MARIA10 || ENABLE_MSTAR_MACAW12 || ENABLE_MSTAR_EDEN || ENABLE_MSTAR_EULER || ENABLE_MSTAR_MAYA || ENABLE_MSTAR_NASA || ENABLE_MSTAR_WHISKY  || ENABLE_MSTAR_MELODY)
#if(ENABLE_MSTAR_EULER || ENABLE_MSTAR_MAYA)
#if defined(CONFIG_MSTAR_EULER_MMAP_128MB)
#define _DRAM_RESET_VECTOR_BASE_ADDR  0x400000
#else
#define _DRAM_RESET_VECTOR_BASE_ADDR  0x200000
#endif
#else
#define _DRAM_RESET_VECTOR_BASE_ADDR  0x200000
#endif
void __loader_2nd Boot2DRAM(void);
void __loader_2nd Boot2DRAM(void)
{
    U32 u32Addr = _DRAM_RESET_VECTOR_BASE_ADDR;

    //set up reset vector base

    RIU[0x1002B4] = (U16)(u32Addr >> 16);
    RIU[0x002E54] = (U16)(0x829f);  // set wtd_mcu_rst passwd
    RIU[0x002E52] &= (~0x0100); // non-fire wtd_mcu_rst
    //wdt reset to dram: bellow 2 lines must in the same cache line 256bytes
    RIU[0x100280] |= (0x002F); // set sdr_boot=1, release rstz_miu
    RIU[0x002E52] |= (0x0100); // fire wtd_mcu_rst

    //########################################
    //#
    //# Due to the fact that __loader_2nd is a specified section
    //# Have to avoid to use some api functions out of this section
    //#
    //########################################
    //MDrv_Write2Byte(0x1002B4, (U16)(u32Addr >> 16));
    //MDrv_Write2Byte(0x002E54, 0x829f);  // set wtd_mcu_rst passwd
    //MDrv_Write2Byte(0x002E52, MDrv_Read2Byte(0x002E52)&(~BIT8));  // non-fire wtd_mcu_rst
    //wdt reset to dram: bellow 2 lines must in the same cache line 256bytes
    //MDrv_Write2Byte(0x100280, MDrv_Read2Byte(0x100280)|(0x002F));  // set sdr_boot=1, release rstz_miu
    //MDrv_Write2Byte(0x002E52, MDrv_Read2Byte(0x002E52)|(BIT8));  // fire wtd_mcu_rst
}
#endif

void __loader_2nd ReBootAtDRAM(U32 u32MemAddr);
void __loader_2nd ReBootAtDRAM(U32 u32MemAddr)
{
    //U32 u32Addr = _DRAM_RESET_VECTOR_BASE_ADDR;

    //set up reset vector base

    RIU[0x1002B4] = (U16)(u32MemAddr >> 16);
    RIU[0x002E54] = (U16)(0x829f);  // set wtd_mcu_rst passwd
    RIU[0x002E52] &= (~0x0100); // non-fire wtd_mcu_rst
    //wdt reset to dram: bellow 2 lines must in the same cache line 256bytes
    RIU[0x100280] |= (0x002F); // set sdr_boot=1, release rstz_miu
    RIU[0x002E52] |= (0x0100); // fire wtd_mcu_rst

    //########################################
    //#
    //# Due to the fact that __loader_2nd is a specified section
    //# Have to avoid to use some api functions out of this section
    //#
    //########################################
    //MDrv_Write2Byte(0x1002B4, (U16)(u32Addr >> 16));
    //MDrv_Write2Byte(0x002E54, 0x829f);  // set wtd_mcu_rst passwd
    //MDrv_Write2Byte(0x002E52, MDrv_Read2Byte(0x002E52)&(~BIT8));  // non-fire wtd_mcu_rst
    //wdt reset to dram: bellow 2 lines must in the same cache line 256bytes
    //MDrv_Write2Byte(0x100280, MDrv_Read2Byte(0x100280)|(0x002F));  // set sdr_boot=1, release rstz_miu
    //MDrv_Write2Byte(0x002E52, MDrv_Read2Byte(0x002E52)|(BIT8));  // fire wtd_mcu_rst
}

#endif // __AEONR2__

//===============================================================================

#if defined(__mips__) || (defined(__arm__) || defined(__arm64__))

void __loader_2nd uart_putc(char ch);

#if 1//( ENABLE_MSTAR_MILAN || ENABLE_MSTAR_MARLON) // Milan use PIU Uart
#if 1//(PIU_UART_VER == 2)
#define UART_RX        (0 * 2)  // In:  Receive buffer (DLAB=0)
#define UART_TX        (0 * 2)  // Out: Transmit buffer (DLAB=0)
#define UART_DLL       (0 * 2)  // Out: Divisor Latch Low (DLAB=1)
#define UART_DLM       (1 * 2)  // Out: Divisor Latch High (DLAB=1)
#define UART_IER       (1 * 2)  // Out: Interrupt Enable Register
#define UART_IIR       (2 * 2)  // In:  Interrupt ID Register
#define UART_FCR       (2 * 2)  // Out: FIFO Control Register
#define UART_LCR       (3 * 2)  // Out: Line Control Register
#define UART_MCR       (4 * 2)	// Out: Modem Control Register
#define UART_LSR       (5 * 2)  // In:  Line Status Register
#define UART_MSR       (6 * 2)  // In:  Modem Status Register
#define UART_USR       (7 * 2)  // Out: USER Status Register
#endif
#define UART_LSR_THRE               0x20          // Transmit-hold-register empty
void __loader_2nd uart_putc(char ch)
{
    U32 timeout_count = 0;
    U32 u32AddrBase = 0;

    #if (defined(__arm__) || defined(__arm64__))
    u32AddrBase = 0x1f201300;
    #else
    u32AddrBase = 0xbf201300;
    #endif

    //while ( (!( UART_REG8(UART_LSR) & UART_LSR_THRE)) && (timeout_count++ < 2000) );
    //while ( (!( (*(volatile U8*)(0xbf201300+UART_LSR)) & UART_LSR_THRE)) && (timeout_count++ < 2000) );
    while ( (!( (*(volatile U8*)(u32AddrBase+UART_LSR)) & UART_LSR_THRE)) && (timeout_count++ < 2000) );

    //UART_REG8(UART_TX) = *buf++;
    //*(volatile U8*)(0xbf201300+UART_TX) = ch;
    *(volatile U8*)(u32AddrBase+UART_TX) = ch;
}
#else
void __loader_2nd uart_putc(char ch)
{
  #if (defined(__arm__) || defined(__arm64__))
    *(volatile U32*)(0x1f201300) = ch;
  #else
    *(volatile U32*)(0xbf201300) = ch;
  #endif
}
#endif // #if ( ENABLE_MSTAR_MILAN || ENABLE_MSTAR_MARLON)
#endif // defined(__mips__) || defined(__arm__) || defined(__arm64__)


//========================================================

#if ( ENABLE_SIMPLE_PRINTF )

#if defined(__aeon__)
  #if ENABLE_LOADER2_AEON_UART
    #define LOADER2_UART_PUTC   loader2_Aeon_uart_putc
  #else
    #define LOADER2_UART_PUTC   uart_putc
  #endif
#else
    #define LOADER2_UART_PUTC   uart_putc
#endif


void __loader_2nd uart_printDigi(U8 u8Num);
void __loader_2nd uart_printDigi(U8 u8Num)
{
    if( u8Num >= 16 )
        LOADER2_UART_PUTC('?');
    else if( u8Num >= 10 )
        LOADER2_UART_PUTC(u8Num - 10 + 'A');
    else
        LOADER2_UART_PUTC(u8Num + '0');
}

void __loader_2nd uart_printHex(U32 dwVal);
void __loader_2nd uart_printHex(U32 dwVal)
{
    U8 u8Tmp;
    BOOL bIsNotZero = 0;

    u8Tmp = (dwVal>>24)&0xFF;
    if( u8Tmp )
        bIsNotZero = 1;
    if( bIsNotZero )
    {
        uart_printDigi(u8Tmp>>4);
        uart_printDigi(u8Tmp&0xF);
    }

    u8Tmp = (dwVal>>16)&0xFF;
    if( u8Tmp )
        bIsNotZero = 1;
    if( bIsNotZero )
    {
        uart_printDigi(u8Tmp>>4);
        uart_printDigi(u8Tmp&0xF);
    }

    u8Tmp = (dwVal>>8)&0xFF;
    if( u8Tmp )
        bIsNotZero = 1;
    if( bIsNotZero )
    {
        uart_printDigi(u8Tmp>>4);
        uart_printDigi(u8Tmp&0xF);
    }

    u8Tmp = (dwVal)&0xFF;
    if( u8Tmp )
        bIsNotZero = 1;

    if( bIsNotZero )
    {
        uart_printDigi(u8Tmp>>4);
    }
    uart_printDigi(u8Tmp&0xF);
}


#define PRINT_FLAG_HEX  0x01//_BIT0

#ifdef __arm__
    #define PATCH_ARM_DIV   0
#else
    #define PATCH_ARM_DIV   0
#endif

#if( PATCH_ARM_DIV )// Patch Div
U32 ARM_Div( U32 u32Val1, U32 u32Val2 );
U32 ARM_Div( U32 u32Val1, U32 u32Val2 )
{
    U32 u32Result = 0;

    if( u32Val2 == 0x10 )
    {
        u32Result = u32Val1 >> 4;
    }
    else if( u32Val2 == 10 )
    {
        if( u32Val1 == 1000000000 )
            u32Result = 100000000;
        else if( u32Val1 == 1000000000 )
            u32Result = 100000000;
        else if( u32Val1 == 100000000 )
            u32Result = 10000000;
        else if( u32Val1 == 10000000 )
            u32Result = 1000000;
        else if( u32Val1 == 1000000 )
            u32Result = 100000;
        else if( u32Val1 == 100000 )
            u32Result = 10000;
        else if( u32Val1 == 10000 )
            u32Result = 1000;
        else if( u32Val1 == 1000 )
            u32Result = 100;
        else if( u32Val1 == 100 )
            u32Result = 10;
        else if( u32Val1 == 10 )
            u32Result = 1;
        else if( u32Val1 == 1 )
            u32Result = 0;
    }
    else
    {
        while( u32Val1 )
        {
            if( u32Val1 < u32Val2 )
                break;
            else // u32Val1 >= u32Val2
            {
                u32Val1 -= u32Val2;
                u32Result += 1;
            }
        }
    }

    return u32Result;
}
#endif


#define PRINT_STRBUF_MAX    11

#define EN_DEBUG_DW_STR     0
#define DEBUG_DW_STR(x)     //x
void __loader_2nd DWordToStr(U32 dwVal, U8* acStrBuf, U8 ucFlag );
void __loader_2nd DWordToStr(U32 dwVal, U8* acStrBuf, U8 ucFlag )
{
    U8 i;
    char c;
    BOOL bHex = 0;
    U32 dwDivider;
    BOOL bNotZero = 0;
    U8 u8LoopCount = 0;


#if EN_DEBUG_DW_STR
    LOADER2_UART_PUTC('D');
    LOADER2_UART_PUTC('2');
    LOADER2_UART_PUTC('S');
    LOADER2_UART_PUTC(':');
    uart_printHex(dwVal);
    LOADER2_UART_PUTC(',');
    //uart_printHex((U32)acStrBuf);
    //uart_printHex((U32)dwVal/0x10000);
    //LOADER2_UART_PUTC(',');
    //uart_printHex(ucFlag);
    LOADER2_UART_PUTC('\r');
    LOADER2_UART_PUTC('\n');
#endif

    if( dwVal == 0 )
    {
        acStrBuf[0] = '0';
        acStrBuf[1] = 0;
        return;
    }

    if( ucFlag&PRINT_FLAG_HEX ) // 16¶i¨î
        bHex = TRUE;

    i = 0;
    if( bHex )
        dwDivider = 0x10000000;
    else
        dwDivider = 1000000000;

    for( u8LoopCount=0; (u8LoopCount<(PRINT_STRBUF_MAX-1)) &&dwDivider; u8LoopCount += 1)
    {
    #if PATCH_ARM_DIV
        c = ARM_Div(dwVal, dwDivider);
    #else
        c = dwVal/dwDivider;
    #endif

    #if EN_DEBUG_DW_STR
        uart_printHex(dwVal);
        LOADER2_UART_PUTC(',');

        uart_printHex(dwDivider);
        LOADER2_UART_PUTC(',');

        uart_printHex(c);
        LOADER2_UART_PUTC(',');
    #endif

        if( c )
            bNotZero = 1;

        if( bNotZero )
        {
            if( c >= 16)
                acStrBuf[i] = 'X';
            else if( c > 9 )
                acStrBuf[i] = c - 10 + 'A';
            else
                acStrBuf[i] = c + '0';
            ++ i;

        #if EN_DEBUG_DW_STR
            uart_printHex(i);
            LOADER2_UART_PUTC(',');
        #endif
        }

    #if PATCH_ARM_DIV // Patch
        dwVal = dwVal - (dwDivider * c);
    #else
        dwVal = dwVal%dwDivider;
    #endif

    #if EN_DEBUG_DW_STR
        //uart_printHex(dwVal);
        //LOADER2_UART_PUTC(',');
    #endif

        if( bHex )
        {
            dwDivider /= 0x10;
        }
        else
        {
        #if PATCH_ARM_DIV // Patch
            dwDivider = ARM_Div(dwDivider, 10);
        #else
            dwDivider /= 10;
        #endif
        }

    #if EN_DEBUG_DW_STR
        LOADER2_UART_PUTC('\r');
        LOADER2_UART_PUTC('\n');
    #endif
    }

    acStrBuf[i] = 0;

}

void __loader_2nd uart_printU32(U32 dwVal, U8 ucFlag);
void __loader_2nd uart_printU32(U32 dwVal, U8 ucFlag)
{
// !!! If code like this ==> U8 acStrBuf[PRINT_STRBUF_MAX] = {0};
// It will cause compile warning when CPU=R2 !!

    U8 acStrBuf[PRINT_STRBUF_MAX];// = {0};
    U8 i;

#if 0
    LOADER2_UART_PUTC('U');
    LOADER2_UART_PUTC('3');
    LOADER2_UART_PUTC('2');
    LOADER2_UART_PUTC(':');
    uart_printHex(dwVal);
    LOADER2_UART_PUTC(',');
    LOADER2_UART_PUTC('\r');
    LOADER2_UART_PUTC('\n');
#endif

    DWordToStr( dwVal, acStrBuf, ucFlag );
    for( i = 0; acStrBuf[i]!=0; ++ i )
    {
        LOADER2_UART_PUTC(acStrBuf[i]);
    }
}

void __loader_2nd uart_printf(const char *pFmt, U32 dwVal);
void __loader_2nd uart_printf(const char *pFmt, U32 dwVal)
{
    U8 ucBff;

    //if( 0 == g_UART_bEnablePrint )
    //    return;

#if 0//defined(__aeon__)
    LOADER2_UART_PUTC('f');
    uart_printU32((U32)pFmt, PRINT_FLAG_HEX);
    LOADER2_UART_PUTC(' ');
#endif
#if 0
    LOADER2_UART_PUTC('p');
    LOADER2_UART_PUTC('f');
    LOADER2_UART_PUTC(':');
#endif

    while( 1 )
    {
    // decompress is running in dram, but string is not in dram!!
    // String is in flash!
    #if defined(__aeon__)
        ucBff = Ld_MDrv_Flash_ReadU8((U32)pFmt);
        pFmt += 1;
    #else
        ucBff = *(pFmt++);
    #endif

        if( ucBff == 0 )
            break;

        if( ucBff == '%' ) // check special case
        {
        #if defined(__aeon__)
            ucBff = Ld_MDrv_Flash_ReadU8((U32)pFmt);
            pFmt += 1;
        #else
            ucBff = *(pFmt++);
        #endif

            if( ucBff == 0 )
                break;

            switch( ucBff ) // check next character
            {
                case 'x': // hexadecimal number
                case 'X':
                    //LOADER2_UART_PUTC('0');
                    //LOADER2_UART_PUTC('x');
                    uart_printU32(dwVal, PRINT_FLAG_HEX);
                    break;
                case 'd': // decimal number
                case 'i':
                case 'u':
                //case 'U':
                    uart_printU32(dwVal,0);
                    break;

                case 'U':
                    uart_printU32(dwVal,0);
                    //putstr("(0x");
                    LOADER2_UART_PUTC('(');
                    LOADER2_UART_PUTC('0');
                    LOADER2_UART_PUTC('x');
                    uart_printU32(dwVal, PRINT_FLAG_HEX);
                    LOADER2_UART_PUTC(')');
                    break;
            } // switch
        }
        else if( ucBff == '\r' )// general
        {}
        else if( ucBff == '\n' )
        {
            LOADER2_UART_PUTC('\r'); // put a character
            LOADER2_UART_PUTC('\n'); // put a character
        }
        else // general
        {
            LOADER2_UART_PUTC(ucBff); // put a character
        }
    } // while}
}
#else

void __loader_2nd uart_printf(const char *pFmt, U32 dwVal);
void __loader_2nd uart_printf(const char *pFmt, U32 dwVal)
{
    pFmt=pFmt;
    dwVal=dwVal;
}

#endif // #if( ENABLE_SIMPLE_PRINTF )


#if( defined(__aeon__) && ENABLE_AEON_UART_PRINTF )
extern void MDrv_InitConsole( void );
extern void uart_putc(char c);
extern int printf(const char *fmt, ...);
/*
void __loader_2nd Ld_puts(const char *s);
void __loader_2nd Ld_puts(const char *s)
{
    char cTmp;

    for (; (cTmp = *s); s++)
    {
        if( cTmp == '\r' )
        {
        }
        else if( cTmp == '\n' )
        {
            uart_putc('\r');
            uart_putc('\n');
        }
        else
        {
            uart_putc(cTmp);
        }
    }
}*/

    #define Ld_printf(...)    printf(__VA_ARGS__)
    #define Ld_puts(str)      printf(str)

#elif ( ENABLE_SIMPLE_PRINTF )//( ENABLE_LOADER2_AEON_UART )
    #define Ld_printf(...)    uart_printf(__VA_ARGS__)
    #define Ld_puts(str)      uart_printf(str, 0)

#else
    #define Ld_printf(...)
    #define Ld_puts(str)
#endif


void __loader_2nd Print_Buffer(U8* pu8Buf, U32 u32Size )
{
    U32 i;

    // pu8Buf align to 16
    i = (U32)pu8Buf;
    i = (i + 15) & 0xFFFFFFF0;
    pu8Buf = (U8*)i;

    Ld_printf("\n==========  Memory (0x%X)  ==========\n", (U32)pu8Buf );

    for( i = 0; i < u32Size; ++ i )
    {
        if( (i % 16) == 0 )
        {
            Ld_printf("%X", ((U32)pu8Buf)+i);
            Ld_printf("(%X) : ",  i);
        }

        Ld_printf("%X ", pu8Buf[i]);

        if( (i % 16) == 15 )
        {
            Ld_printf("\n", 0);
        }
    }

    Ld_printf("\n--------------------------------------------------\n", 0);

}

//========================================================
typedef struct
{
    U32 u32RomStart;
    U32 u32RomEnd;
    //U32 u32RomSize;

    U32 u32RamStart;
    U32 u32RamEnd;
    //U32 u32RamSize;

    U32 u32CRC;

    U32 u32BootLoaderSize;
    //U32 u32BinaryId;

    U8 u8DebugFlag;

} StuChunkHeaderInfo;

#if defined(__mips__) || (defined(__arm__) || defined(__arm64__))
U32 __loader_2nd Loader_Flash_ConvertToPhyAddr(U32 u32FlashAddr);
U32 __loader_2nd Loader_Flash_ConvertToPhyAddr(U32 u32FlashAddr)
{
    if( (u32FlashAddr >= MIPS_FLASH_IO_BASE_1) && (u32FlashAddr < 0xC0000000) )
    {
        return u32FlashAddr - MIPS_FLASH_IO_BASE_1;
    }

    if( (u32FlashAddr >= MIPS_FLASH_IO_BASE_2) && (u32FlashAddr <= 0xB4FFFFFF) )
    {
        return u32FlashAddr - MIPS_FLASH_IO_BASE_2;
    }

    return u32FlashAddr;
}
#endif

#if defined(__aeon__)||defined(__mips__) || (defined(__arm__) || defined(__arm64__))
void __loader_2nd Loader_Get_ChunkHeader(U32 u32FlashAddr, StuChunkHeaderInfo* pstChunkHeaderInfo);
void __loader_2nd Loader_Get_ChunkHeader(U32 u32FlashAddr, StuChunkHeaderInfo* pstChunkHeaderInfo)
{
    pstChunkHeaderInfo->u32RomStart = Ld_MDrv_Flash_ReadU32(u32FlashAddr + 0 ); //This address should be 4' alignment
    pstChunkHeaderInfo->u32RomEnd = Ld_MDrv_Flash_ReadU32( u32FlashAddr + 12 ); //This address should be 4' alignment
    //pstChunkHeaderInfo->u32RomSize = pstChunkHeaderInfo->u32RomEnd - pstChunkHeaderInfo->u32RomStart + 1;

#if( defined(__mips__) /* || (defined(__arm__) || defined(__arm64__))*/ )
    pstChunkHeaderInfo->u32RomStart = Loader_Flash_ConvertToPhyAddr(pstChunkHeaderInfo->u32RomStart);
    pstChunkHeaderInfo->u32RomEnd = Loader_Flash_ConvertToPhyAddr(pstChunkHeaderInfo->u32RomEnd);
#endif

    pstChunkHeaderInfo->u32RamStart = Ld_MDrv_Flash_ReadU32(u32FlashAddr + 4); //This address should be 4' alignment
    pstChunkHeaderInfo->u32RamEnd = Ld_MDrv_Flash_ReadU32(u32FlashAddr + 8);
    //pstChunkHeaderInfo->u32RamSize = pstChunkHeaderInfo->u32RamEnd - pstChunkHeaderInfo->u32RamStart + 1;

    pstChunkHeaderInfo->u32CRC = Ld_MDrv_Flash_ReadU32(u32FlashAddr + CHUNK_HEADER_CRC32_POS);

    pstChunkHeaderInfo->u32BootLoaderSize = Ld_MDrv_Flash_ReadU32(u32FlashAddr + 0x18);

    //pstChunkHeaderInfo->u32BinaryId = Loader2_ReadFlashU32(u32FlashAddr + 0x1C);

#if defined(__arm__)
    pstChunkHeaderInfo->u8DebugFlag = 0xFF;
#else
    pstChunkHeaderInfo->u8DebugFlag = Ld_MDrv_Flash_ReadU32(u32FlashAddr + 0x7F);
#endif
}
#endif

#if 1//defined(__mips__)
typedef struct
{
    U32 u32MagicId;
    U8 au8CompressMagicNum[3];
    U32 u32CompressedLength;
    U32 u32MagicEndAddr;
    //U32 u32MagicEnd;

} StuApImgInfo;
void __loader_2nd Loader_Get_ApImgInfo(U32 u32ApRomStart, StuApImgInfo* pstApImgInfo);
void __loader_2nd Loader_Get_ApImgInfo(U32 u32ApRomStart, StuApImgInfo* pstApImgInfo)
{
    pstApImgInfo->u32MagicId = Ld_MDrv_Flash_ReadU32(u32ApRomStart + AP_IMG_INFO_MAGIC_ID_ADDR );

    pstApImgInfo->au8CompressMagicNum[0] = Ld_MDrv_Flash_ReadU8(u32ApRomStart + AP_IMG_INFO_COMPRESS_INFO_ADDR + 0 );
    pstApImgInfo->au8CompressMagicNum[1] = Ld_MDrv_Flash_ReadU8(u32ApRomStart + AP_IMG_INFO_COMPRESS_INFO_ADDR + 1 );
    pstApImgInfo->au8CompressMagicNum[2] = Ld_MDrv_Flash_ReadU8(u32ApRomStart + AP_IMG_INFO_COMPRESS_INFO_ADDR + 2 );

    pstApImgInfo->u32CompressedLength = Ld_MDrv_Flash_ReadU32(u32ApRomStart + AP_IMG_INFO_COMPRESS_LEN_ADDR );

    pstApImgInfo->u32MagicEndAddr = Ld_MDrv_Flash_ReadU32(u32ApRomStart + AP_IMG_INFO_MAGIC_ID_2_ADDR );
#if( defined(__mips__) )
    pstApImgInfo->u32MagicEndAddr = Loader_Flash_ConvertToPhyAddr(pstApImgInfo->u32MagicEndAddr);
#endif

}
#endif

#if defined(__mips__) || (defined(__arm__) || defined(__arm64__))
U32 __loader_2nd Loader_MDrv_BDMA_CalCRC(U32 u32Addr, U32 u32Size, BOOL bSrcIsDram);
U32 __loader_2nd Loader_MDrv_BDMA_CalCRC(U32 u32Addr, U32 u32Size, BOOL bSrcIsDram)
{
    U32 u32CRC32_HW = 0x11111111;

#if 0 // Reset BDMA engine
    //MDrv_WriteByte(REG_BDMA_TRIG__STOP, BDMA_CH_STOP); // [4]:Reg_stop
    RIU8[(LOCAL_BDMA_REG_BASE + 0x00)*2] = 0x10;

    //MDrv_PIU_DelayUs(1);
    uart_putc('.');

    // Un-Reset BDMA engine
    //MDrv_WriteByte(REG_BDMA_TRIG__STOP, 0);
    RIU8[(LOCAL_BDMA_REG_BASE + 0x00)*2] = 0x00;
#endif

    // Wait hw not busy
    if( (RIU8[(LOCAL_BDMA_REG_BASE + 0x02)*2] & 0x03) )
    {
        //Ld_printf(" Wait BDMA free\n",0);
        while (RIU8[(LOCAL_BDMA_REG_BASE + 0x02)*2] & 0x03);
    }

    //set source address
    RIU[(LOCAL_BDMA_REG_BASE + 0x08 )] = (U16)(u32Addr);
    RIU[(LOCAL_BDMA_REG_BASE + 0x0A )] = (U16)((u32Addr) >> 16);
    //set destination address
    RIU[(LOCAL_BDMA_REG_BASE + 0x0C )] = 0;
    RIU[(LOCAL_BDMA_REG_BASE + 0x0E )] = 0;
    // set length
    RIU[(LOCAL_BDMA_REG_BASE + 0x10 )] = (U16)(u32Size);
    RIU[(LOCAL_BDMA_REG_BASE + 0x12 )] = (U16)((u32Size) >> 16);
    //polynomial
    RIU[(LOCAL_BDMA_REG_BASE + 0x14 )] = (U16)(0x1DB7);
    RIU[(LOCAL_BDMA_REG_BASE + 0x16 )] = (U16)(0x04C1);
    //seed
    RIU[(LOCAL_BDMA_REG_BASE + 0x18 )] = 0;
    RIU[(LOCAL_BDMA_REG_BASE + 0x1a )] = 0;

    RIU8[(LOCAL_BDMA_REG_BASE + 0x06)*2] = 0x0; //increasing

    //##############################
    // Data Width=4 (16 byte)
    //  Marlon/Macan/M3822
    // Dev:
    //  0:MIU0, 1:MIU1, 3:CRC, 5:Flash
    //##############################
    if( bSrcIsDram )
    {
        RIU[(LOCAL_BDMA_REG_BASE+0x04)] = 0x0340;  //((DstDataWidth|DstDEv)  <<8) |(SrcDataWidth|SrcDEv) ;
    }
    else // Data from flash
    {
        RIU[(LOCAL_BDMA_REG_BASE+0x04)] = 0x0345;  //((DstDEv|DstDataWidth)  <<8) |(SrcDataWidth|SrcDEv) ;
    }

    RIU8[ (LOCAL_BDMA_REG_BASE*2)] = 0x01; // Trigger

    // Wait BDMA done
    if ( (RIU8[(LOCAL_BDMA_REG_BASE+0x02)*2] & 0x08) != 0x08 )
    {
        //Ld_printf(" Wait CRC done\n",0);
        while( 1 )
        {
            if ( (RIU8[(LOCAL_BDMA_REG_BASE+0x02)*2] & 0x08) == 0x08 )
                break;
        }
        //Ld_printf(" CRC done~\n",0);
    }

#if 0 // Wait not busy
    if( (RIU8[(LOCAL_BDMA_REG_BASE + 0x02)*2] & 0x03) )
    {
        //u32CRC32_HW(" Wait BDMA free\n",0);
        while (RIU8[(LOCAL_BDMA_REG_BASE + 0x02)*2] & 0x03);
    }
#endif

    //Get CRC32 result
    u32CRC32_HW = RIU[(LOCAL_BDMA_REG_BASE + 0x18 )] ;      //low word
    u32CRC32_HW |= (RIU[(LOCAL_BDMA_REG_BASE + 0x1a )]  <<  16);        //high word

    //Ld_printf("Loader_MDrv_BDMA_CalCRC() end => %X,", u32CRC32_HW );

    return u32CRC32_HW;
}

#endif

//==============================================================================
#define DEBUG_FLAG_HEADER       0x01
#define DEBUG_FLAG_CRC          0x02
#define DEBUG_FLAG_DECOMP       0x04
#define DEBUG_FLAG_FOD          0x08

static U8 g_DeComp_u8DebugFlag = 0;

//==============================================================================
#if ( (defined(__mips__) || (defined(__arm__) || defined(__arm64__)))  && ENABLE_NON_OS)
#define REG_BDMA_BASE   0x900

#define DEBUG_DECOMP(x)         x

#if 1
#define DEBUG_DECOMP_DECOMP(x)  do{ if(g_DeComp_u8DebugFlag&DEBUG_FLAG_DECOMP) {x;} }while(0)
#define DEBUG_DECOMP_CRC(x)     do{ if(g_DeComp_u8DebugFlag&DEBUG_FLAG_CRC) {x;} }while(0)
#define DEBUG_DECOMP_FOD(x)     do{ if(g_DeComp_u8DebugFlag&DEBUG_FLAG_FOD) {x;} }while(0)
#else
#define DEBUG_DECOMP_DECOMP(x)  x
#define DEBUG_DECOMP_CRC(x)     x
#define DEBUG_DECOMP_FOD(x)     x
#endif

#if defined(__mips__)
//    #define DRAM_MEMORY_BASE        0xA0000000

#elif(defined(__arm__))
    //#define DRAM_MEMORY_BASE        0x20000000
#endif

#if( defined(__arm__) || defined(__mips__) )
U32 __loader_2nd msOS_VaToPa(U32 u32Va);
U32 __loader_2nd msOS_VaToPa(U32 u32Va)
{
    U32 u32Pa = u32Va;

#if( defined(__arm__) )
    u32Pa = u32Va & 0x0FFFFFFF;
#endif

#if( defined(__mips__) )
    u32Pa = u32Va & 0x0FFFFFFF;
#endif

    return u32Pa;
}

U32 __loader_2nd msOS_PaToVa_NonCache(U32 u32Pa);
U32 __loader_2nd msOS_PaToVa_NonCache(U32 u32Pa)
{
    U32 u32Va = u32Pa & 0x0FFFFFFF; // Dram max is 256M

#if( defined(__arm__) )
    u32Va = u32Va | 0x40000000; // Non-Cache
    //u32Va = u32Va | 0x20000000; // Cache
#endif

#if( defined(__mips__) )
    u32Va = u32Va | 0xA0000000;
#endif

    return u32Va;
}
#endif

#include "MSDecompress_NonOS_FOD.c"

#if(defined(__arm__)) && ( (ENABLE_MSTAR_MACAN)||(ENABLE_MSTAR_M3822)||(ENABLE_MSTAR_MAZDA))
    #include "MSDecompress_NonOS_MMU.c"
    #define CHECK_CRC32_ENABLE          0
    #define DEBUG_CRC32_MSG(x)          //x

#elif (defined(__mips__)) && ( (ENABLE_MSTAR_MILAN)||(ENABLE_MSTAR_MARLON))
    #define CHECK_CRC32_ENABLE          0
    #define DEBUG_CRC32_MSG(x)          //x

#endif

#define AP_IN_FLASH_ADDR    0x00020000      //This address should be 4' alignment

void __loader_2nd DoMSDecompression(void)
{
    U8 IsCompressMagicNumber = 0;
    U32 CompressedFileLength = 0;
    U32 OriginalFileLength = 0;
    //U32 nOutputLength=0;

    U32 u32BOOT_LOADER_LENGTH = 0;
    U32 u32ROM_START = 0;//*((U32 *)(AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH + 0));    //This address should be 4' alignment
    U32 u32ROM_END = 0;//*((U32 *)(AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH + 12));    //This address should be 4' alignment
    U32 u32ROM_Size = 0;

    U32 u32CRC32_HW = 0, u32CRC32_SW = 0;
    U32 u32RAM_START, u32RAM_END;
    //BOOL bIsBootToBL = FALSE; // Not AP
    U32 u32Header_BootLoaderSize;
    U32 u32RAM_START_PA=0;

    StuChunkHeaderInfo stChunkHeaderInfo;
    BOOL bLoad2ndAP = FALSE;
    StuApImgInfo stApImgInfo;

#if(ENABLE_FLASH_ON_DRAM)
    BOOL bDoFOD = FALSE;
#endif

#if defined (CONFIG_MSTAR_STR_ENABLE)
    U16 u16STRFlag = 0;
    BOOL bSTREnable = FALSE;

  #if (defined(__arm__))
    #if ((ENABLE_MSTAR_MACAN)||(ENABLE_MSTAR_MAZDA))
    #define REG_STR_FLAG        0x0e70
    u16STRFlag = msRead2Bytes(REG_STR_FLAG) & 0xF000;
    bSTREnable = (u16STRFlag != 0xF000)? TRUE:FALSE;
    #endif
    #if ((ENABLE_MSTAR_M3822))
    #define REG_STR_FLAG        0x1222
    u16STRFlag = msRead2Bytes(REG_STR_FLAG) & 0xF000;
    bSTREnable = (u16STRFlag != 0xF000)? TRUE:FALSE; //STR:0x0000, NonSTR:0xF000
    #endif

    Ld_printf("u16STRFlag :0x%X\n", u16STRFlag);
    Ld_printf("bSTREnable=%d\n", bSTREnable);
    if(bSTREnable) //DC On
    {
      #if CHECK_CRC32_ENABLE//nonOS CRC confirm method in sboot
        #define APP_CRC32_ADR   0xD30000
        #define APP_CRC32_LEN   0x300000
        #define REG_CRC32_DM0  0x003DB8
        #define REG_CRC32_DM1  0x003DBA
        U32 lx_mem_addr = 0, lx_mem_size = 0;
        U32 crc_result_cnt = 0;
        U32 u32CRCValue_BDMA = 0;
        U32 u32CRCValue_LogRiu = 0;

        u32CRCValue_LogRiu = (((U32)msRead2Bytes(REG_CRC32_DM0)) << 16) | ((U32)msRead2Bytes(REG_CRC32_DM1));
        DEBUG_CRC32_MSG(Ld_printf("(((===>)))u32CRCValue_LogRiu :%X\n",u32CRCValue_LogRiu));
        lx_mem_addr      = APP_CRC32_ADR;      //nos beon start address
        lx_mem_size        = APP_CRC32_LEN;      //calc 3m
        crc_result_cnt = 10;
        while(crc_result_cnt)
        {
            u32CRCValue_BDMA = Loader_MDrv_BDMA_CalCRC(lx_mem_addr, lx_mem_size, 1);
            DEBUG_CRC32_MSG(Ld_printf("u32CRCValue_BDMA :%X\n",u32CRCValue_BDMA));
            if((u32CRCValue_LogRiu == u32CRCValue_BDMA))
            {
                DEBUG_CRC32_MSG(Ld_printf("STR skip decompress\n",0));
                return;
            }
            else
            {
                DEBUG_CRC32_MSG(Ld_printf("STR re-compute CRC32\n",0));
                crc_result_cnt--;
            }
        }
        DEBUG_CRC32_MSG(Ld_printf("STR do decompress\n",0));
        msWrite2Bytes(REG_STR_FLAG, 0xFFFF);
      #else
        DEBUG_CRC32_MSG(Ld_printf("STR skip decompress\n",0));
        return;
      #endif
    }
  #elif (defined(__mips__)) && ((ENABLE_MSTAR_MILAN)||(ENABLE_MSTAR_MARLON))
    #define REG_STR_FLAG        0x000e68
    u16STRFlag = msRead2Bytes(REG_STR_FLAG);
    bSTREnable = (u16STRFlag == 0xBABE)? TRUE:FALSE;
    DEBUG_CRC32_MSG(Ld_printf("u16STRFlag :0x%X\n", u16STRFlag));
    DEBUG_CRC32_MSG(Ld_printf("bSTREnable=%d\n", bSTREnable));
    if(bSTREnable) //DC On
    {
      #if CHECK_CRC32_ENABLE//nonOS CRC confirm method in sboot
        U32 lx_mem_addr = 0, lx_mem_size = 0,lx_mem_addr2 = 0, lx_mem_size2 = 0;
        U32 crc_result_1 = 0,crc_result_2 = 0;
        U32 u32CRC_Pointer1 = (0xA2671000+0x400000);      //offset 4m, for store CRC value1
        U32 u32CRC_Pointer2 = (0xA2671000+0x400004);     //offset 4m+0x04, for store CRC value2
        U32 u32CRC_PM_VALUE1 = 0;
        U32 u32CRC_PM_VALUE2 = 0;

        lx_mem_addr        = 0xa00000;      //nos beon start address
        lx_mem_size        = 0x400000;      //calc 4m
        lx_mem_addr2        = 0x2671000;    //nos STR_BAK_BUF start address
        lx_mem_size2        = 0x200000;     //calc 2m
        crc_result_1 = Loader_MDrv_BDMA_CalCRC(lx_mem_addr, lx_mem_size, 1);
        crc_result_2 = Loader_MDrv_BDMA_CalCRC(lx_mem_addr2, lx_mem_size2, 1);
        Loader_MemCpy((U8 *)&u32CRC_PM_VALUE1, (U8 *) u32CRC_Pointer1, 4);
        Loader_MemCpy((U8 *)&u32CRC_PM_VALUE2, (U8 *) u32CRC_Pointer2, 4);
        if((u32CRC_PM_VALUE1 == crc_result_1)&&(u32CRC_PM_VALUE2 == crc_result_2))
        {
            DEBUG_CRC32_MSG(Ld_printf("STR skip decompress\n",0));
            return;
        }
        else
        {
            DEBUG_CRC32_MSG(Ld_printf("STR do decompress\n",0));
            DEBUG_CRC32_MSG(Ld_printf("u32CRC_PM_VALUE1 :%X\n",u32CRC_PM_VALUE1));
            DEBUG_CRC32_MSG(Ld_printf("u32CRC_PM_VALUE2 :%X\n",u32CRC_PM_VALUE2));
            DEBUG_CRC32_MSG(Ld_printf("crc_result_1 :%X\n",crc_result_1));
            DEBUG_CRC32_MSG(Ld_printf("crc_result_2 :%X\n",crc_result_2));
        }
    #else
        DEBUG_CRC32_MSG(Ld_printf("STR skip decompress\n",0));
        return;
    #endif
    }
  #endif
#endif

    Ld_printf("\nDecomp() at %u\r\n", Loader_MDrv_PIU_Get_Time() );


    // Get first ap chunk header info
    DEBUG_DECOMP( Ld_printf("Get 1st chunk header at flash %X\n", AP_IN_FLASH_ADDR););
    Loader_Get_ChunkHeader(AP_IN_FLASH_ADDR, &stChunkHeaderInfo);


    // Get boot loader bin size from flash chunker-header
    //u32Header_BootLoaderSize = *((U32 *)(AP_IN_FLASH_ADDR + 0x18));
    u32Header_BootLoaderSize = stChunkHeaderInfo.u32BootLoaderSize;
    DEBUG_DECOMP( Ld_printf(" u32Header_BootLoaderSize=%X\n", u32Header_BootLoaderSize););

    if( u32Header_BootLoaderSize )
    {
        // Get u32BOOT_LOADER_LENGTH
        u32BOOT_LOADER_LENGTH = u32Header_BootLoaderSize - 0x20000; //minus 64k for sboot and 64k for pm code
    }
    else
    {
        u32BOOT_LOADER_LENGTH = 0;
    }
    DEBUG_DECOMP( Ld_printf(" u32BOOT_LOADER_LENGTH=%X\n", u32BOOT_LOADER_LENGTH););

    // If boot loader exist, get 2nd ap chunk-header
    if( u32BOOT_LOADER_LENGTH )
    {
        bLoad2ndAP = TRUE;
    }

    //u32ROM_START = *((U32 *)(AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH + 0));    //This address should be 4' alignment
    u32ROM_START = stChunkHeaderInfo.u32RomStart;
    //u32ROM_END = *((U32 *)(AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH + 12));    //This address should be 4' alignment
    u32ROM_END = stChunkHeaderInfo.u32RomEnd; //This address should be 4' alignment
    u32ROM_Size = u32ROM_END - u32ROM_START;
    DEBUG_DECOMP( Ld_printf(" u32ROM_START=%X\n", u32ROM_START););
    DEBUG_DECOMP( Ld_printf(" u32ROM_END=%X\n", u32ROM_END););
    //DEBUG_DECOMP( Ld_printf(" u32ROM_Size=%X\n", u32ROM_Size););


    //Compressed Format
    /*if( (u32ROM_START>=AP_IN_FLASH_ADDR)
      && (u32ROM_START<(AP_IN_FLASH_ADDR+0x800000))
      && (u32ROM_END>=AP_IN_FLASH_ADDR)
      && (u32ROM_END<(AP_IN_FLASH_ADDR+0x800000)) ) //8MB flash
      */
    if( bLoad2ndAP )
    {
        DEBUG_DECOMP( Ld_printf("\nCheck MagicId\n", 0); );

        // Get 2nd ap chunk header info
        DEBUG_DECOMP( Ld_printf("Get 2nd chunk header at flash %X\n", AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH););
        Loader_Get_ChunkHeader(AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH, &stChunkHeaderInfo);
        DEBUG_DECOMP( Ld_printf(" DebugFlag=0x%X\n", stChunkHeaderInfo.u8DebugFlag););

        u32ROM_START = stChunkHeaderInfo.u32RomStart;
        u32ROM_END = stChunkHeaderInfo.u32RomEnd;
        u32ROM_Size = u32ROM_END - u32ROM_START;
        u32RAM_START = stChunkHeaderInfo.u32RamStart;
        u32RAM_END = stChunkHeaderInfo.u32RamEnd;
        u32CRC32_SW = stChunkHeaderInfo.u32CRC;
        DEBUG_DECOMP( Ld_printf(" u32ROM_START=0x%X\n", u32ROM_START); );
        DEBUG_DECOMP( Ld_printf(" u32ROM_END=0x%X\n", u32ROM_END); );
        DEBUG_DECOMP( Ld_printf(" u32ROM_Size=%X\n", u32ROM_Size););
        DEBUG_DECOMP( Ld_printf(" u32RAM_START=0x%X\n", u32RAM_START); );
        DEBUG_DECOMP( Ld_printf(" u32RAM_END=0x%X\n", u32RAM_END); );
        //DEBUG_DECOMP_CRC( Ld_printf(" u32CRC32_SW=0x%X\n", u32CRC32_SW); );

        // Main AP
        //u32RAM_START = *((U32 *)(AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH + 4));    //This address should be 4' alignment
        //u32RAM_END = *((U32 *)(AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH + 8));
        //DEBUG_DECOMP(uart_printf("u32RAM_START=%X\n", u32RAM_START););
        //DEBUG_DECOMP(uart_printf("u32RAM_END=%X\n", u32RAM_END););

        // Read imgInfo
        DEBUG_DECOMP( Ld_printf("Read ImgInfo from %X\n", u32ROM_START); );
        Loader_Get_ApImgInfo(u32ROM_START, &stApImgInfo);
        DEBUG_DECOMP( Ld_printf(" u32MagicId=%X\n", stApImgInfo.u32MagicId); );
        DEBUG_DECOMP( Ld_printf(" u32MagicEndAddr=%X\n", stApImgInfo.u32MagicEndAddr); );
        //DEBUG_DECOMP( Ld_printf(" u32MagicEnd=%X\n", stApImgInfo.u32MagicEnd); );


        // Check magic end/start
        {
            U32 u32MAGIC_END_ADDR, u32MAGIC_END = 0, u32MAGIC_START;

            // Check AP ImgInfo-MagicId_1...
        #if 1
            u32MAGIC_START = stApImgInfo.u32MagicId;
        #else
            // Magic start is at  .prog_img first 3 bytes
            u32MAGIC_START = *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS-0x4A)+3));
            u32MAGIC_START <<= 8;
            u32MAGIC_START |= *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS-0x4A)+2));
            u32MAGIC_START <<= 8;
            u32MAGIC_START |= *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS-0x4A)+1));
            u32MAGIC_START <<= 8;
            u32MAGIC_START |= *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS-0x4A)+0));
        #endif
            DEBUG_DECOMP( Ld_printf("u32MAGIC_START=%X\n", u32MAGIC_START););
            if( u32MAGIC_START != 0x55AA5678 )
            {
                Ld_printf("u32MAGIC_START(%X) != 0x55AA5678\n", u32MAGIC_START);
                goto LOAD_LOADER_AP;
            }

            // Check AP ImgInfo-MagicId_2...
            // Get magic end flash addr
            //DEBUG_DECOMP(uart_printf("u32MAGIC_END_ADDR FlashAddr=%X\n", (u32ROM_START+COMPRESSED_LENGTH_POS+4)););

        #if 1
            u32MAGIC_END_ADDR = stApImgInfo.u32MagicEndAddr;
        #else
            u32MAGIC_END_ADDR = *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS+4)+3));
            u32MAGIC_END_ADDR <<= 8;
            u32MAGIC_END_ADDR |= *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS+4)+2));
            u32MAGIC_END_ADDR <<= 8;
            u32MAGIC_END_ADDR |= *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS+4)+1));
            u32MAGIC_END_ADDR <<= 8;
            u32MAGIC_END_ADDR |= *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS+4)+0));
        #endif
            DEBUG_DECOMP(Ld_printf("u32MAGIC_END_ADDR=%X\n", u32MAGIC_END_ADDR););

            // Get MagicEnd
        #if 1
            if( u32MAGIC_END_ADDR < 0x1000000 ) // < 16M
            {
                u32MAGIC_END = Ld_MDrv_Flash_ReadU32(u32MAGIC_END_ADDR );
            }
        #else
            u32MAGIC_END = *((U8 *)(u32MAGIC_END_ADDR+3));
            u32MAGIC_END <<= 8;
            u32MAGIC_END |= *((U8 *)(u32MAGIC_END_ADDR+2));
            u32MAGIC_END <<= 8;
            u32MAGIC_END |= *((U8 *)(u32MAGIC_END_ADDR+1));
            u32MAGIC_END <<= 8;
            u32MAGIC_END |= *((U8 *)(u32MAGIC_END_ADDR+0));
        #endif
            DEBUG_DECOMP(Ld_printf("u32MAGIC_END=%X\n", u32MAGIC_END););
            if(u32MAGIC_END != 0x55AAABCD)
            {
                Ld_printf("u32MAGIC_END(%X) != 0x55AAABCD\n", u32MAGIC_END);
                goto LOAD_LOADER_AP;
            }
        }
    }
    else
    {
        // Load Boot-Loader
LOAD_LOADER_AP:

        Ld_printf("LOAD Frist AP: at %u\n", Loader_MDrv_PIU_Get_Time());

        // Use first AP(No matter what it is)
        Loader_Get_ChunkHeader(AP_IN_FLASH_ADDR, &stChunkHeaderInfo);
        DEBUG_DECOMP( Ld_printf(" DebugFlag=0x%X\n", stChunkHeaderInfo.u8DebugFlag););

        u32ROM_START = stChunkHeaderInfo.u32RomStart;
        u32ROM_END = stChunkHeaderInfo.u32RomEnd;
        u32ROM_Size = u32ROM_END - u32ROM_START;
        u32RAM_START = stChunkHeaderInfo.u32RamStart;
        u32RAM_END = stChunkHeaderInfo.u32RamEnd;
        u32CRC32_SW = stChunkHeaderInfo.u32CRC;

        DEBUG_DECOMP( Ld_printf(" u32ROM_START=0x%X\n", u32ROM_START); );
        DEBUG_DECOMP( Ld_printf(" u32ROM_END=0x%X\n", u32ROM_END); );
        DEBUG_DECOMP( Ld_printf(" u32ROM_Size=%X\n", u32ROM_Size););
        DEBUG_DECOMP( Ld_printf(" u32RAM_START=0x%X\n", u32RAM_START); );
        DEBUG_DECOMP( Ld_printf(" u32RAM_END=0x%X\n", u32RAM_END); );
        //DEBUG_DECOMP_CRC( Ld_printf(" u32CRC32_SW=0x%X\n", u32CRC32_SW); );
    }

    // Setup debug flag
    g_DeComp_u8DebugFlag = stChunkHeaderInfo.u8DebugFlag;



//LOAD_CODE_POINT:
    DEBUG_DECOMP(Ld_printf("LOAD_CODE_POINT: at %u\n", Loader_MDrv_PIU_Get_Time()); );

    //DEBUG_DECOMP( Ld_printf(" u32ROM_START+AP_IMG_INFO_SECTION_OFFSET=0x%X\n", u32ROM_START+AP_IMG_INFO_SECTION_OFFSET); );
    Loader_Get_ApImgInfo(u32ROM_START, &stApImgInfo);
    DEBUG_DECOMP_DECOMP( Ld_printf(" u32MagicId=0x%x\n", stApImgInfo.u32MagicId); );
    DEBUG_DECOMP_DECOMP( Ld_printf(" au8CompressMagicNum[0]=0x%x\n", stApImgInfo.au8CompressMagicNum[0]); );
    DEBUG_DECOMP_DECOMP( Ld_printf(" au8CompressMagicNum[1]=0x%x\n", stApImgInfo.au8CompressMagicNum[1]); );
    DEBUG_DECOMP_DECOMP( Ld_printf(" au8CompressMagicNum[2]=0x%x\n", stApImgInfo.au8CompressMagicNum[2]); );

    // Check if this bin file is compressed format, find compress format magic number
    //if( ( *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS-4)+3)) == 0xDE )
    //  && ( *((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS-4)+2)) == 0xAD ) )
    if( ( stApImgInfo.au8CompressMagicNum[2] == 0xDE )
     && ( stApImgInfo.au8CompressMagicNum[1] == 0xAD )
      )
    {
        U8 u8CompressFormat = stApImgInfo.au8CompressMagicNum[0];//*((U8 *)((u32ROM_START+COMPRESSED_LENGTH_POS-4)+1));

        DEBUG_DECOMP_DECOMP(Ld_printf("u8CompressFormat=%X\n", u8CompressFormat););

        if ( u8CompressFormat == 0xBE )
            IsCompressMagicNumber = 1; // comp
        else if ( u8CompressFormat == 0xEF )
            IsCompressMagicNumber = 2; // comp7
        else
        {
            IsCompressMagicNumber = 0;
            Ld_printf("\nError: u8CompressFormat=0x%x\n", u8CompressFormat);
            while(1){}
        }
    }
    else
    {
        IsCompressMagicNumber = 0;
    }

    DEBUG_DECOMP_DECOMP(Ld_printf("IsCompressMagicNumber=%X\n", IsCompressMagicNumber););

#if(ENABLE_FLASH_ON_DRAM)
    if( (IsCompressMagicNumber != 0) // Ap no comp ==> No need FOD
      &&(u32ROM_Size >= 0x100000) // It's normal AP, not bloader! BLoader no need FOD
      )
    {
        bDoFOD = TRUE;
    }
#endif

    //Compressed Format
    if ( IsCompressMagicNumber )
    {
        // Get compressed file size
        CompressedFileLength = stApImgInfo.u32CompressedLength;
        CompressedFileLength -= 6; /*6: 2 for crc16 and 4 for length*/
        DEBUG_DECOMP_DECOMP(Ld_printf("CompressedFileLength=%U\n", CompressedFileLength););

        // Get Original file size
        if( IsCompressMagicNumber == 1 ) // comp
        {
            DEBUG_DECOMP_DECOMP( Ld_printf(" u32ROM_START+CompressedFileLength=0x%X\n", u32ROM_START+CompressedFileLength); );
            OriginalFileLength = Ld_MDrv_Flash_ReadU32( u32ROM_START+CompressedFileLength);
        }
        else if( IsCompressMagicNumber == 2 )// comp7
        {
            DEBUG_DECOMP_DECOMP( Ld_printf(" u32ROM_START+UNCOMPRESSED_LENGTH+5=0x%X\n", u32ROM_START+UNCOMPRESSED_LENGTH+5); );
            OriginalFileLength = Ld_MDrv_Flash_ReadU32( u32ROM_START+UNCOMPRESSED_LENGTH+5);
        }
        else
        {
            //OriginalFileLength = 0x800000; // 8M
        }

        DEBUG_DECOMP_DECOMP( Ld_printf(" OriginalFileLength=0x%X\n", OriginalFileLength); );

        /////////////////////////////////////////////////////////////////////////////
        // Check AP_C.bin CRC32

        DEBUG_DECOMP( Ld_printf("DECOMPRESS_BUF_PA=0x%X\n", DECOMPRESS_BUF_PA); );
        //DEBUG_DECOMP_DECOMP( Ld_printf(" u32DecompBuf_PA=0x%X\n", u32DecompBuf_PA); );

        Ld_printf("CRC at %u\n", Loader_MDrv_PIU_Get_Time() );

        // Cal CRC on dram
        {
            // Test dram crc
            U32 u32CrcBuf_PA = (DECOMPRESS_BUF_PA - UNCOMPRESSED_LENGTH);

            DEBUG_DECOMP_CRC( Ld_printf(" u32CrcBuf_PA=0x%X\n", u32CrcBuf_PA); );

            Ld_printf("__BDMA_FlashCopy2MIU0(%X,", u32ROM_START );
            Ld_printf(" %X,", u32CrcBuf_PA );
            Ld_printf(" %X)\n", u32ROM_Size );

            DEBUG_DECOMP_CRC( Ld_printf("CP Ap at %u\n", Loader_MDrv_PIU_Get_Time() ); );
            __BDMA_FlashCopy2MIU0((u32ROM_START), u32CrcBuf_PA, u32ROM_Size);
            DEBUG_DECOMP_CRC( Ld_printf("CRC 1 at %u\n", Loader_MDrv_PIU_Get_Time() ); );
            u32CRC32_HW = Loader_MDrv_BDMA_CalCRC( u32CrcBuf_PA, u32ROM_Size, 1);
            DEBUG_DECOMP_CRC( Ld_printf("CRC 2 at %u\n", Loader_MDrv_PIU_Get_Time() ); );
            DEBUG_DECOMP_CRC( Ld_printf("u32CRC32_HW=0x%X\n", u32CRC32_HW); );
            DEBUG_DECOMP_CRC( Ld_printf(" u32CRC32_SW=0x%X\n", u32CRC32_SW); );
        }

        Ld_printf("CRC done at %u\n", Loader_MDrv_PIU_Get_Time() );

        if ( u32CRC32_HW != u32CRC32_SW )
        {
            Ld_printf("u32CRC32_HW(%X) != u32CRC32_SW\n", u32CRC32_HW);
            //AP in flash is bad, load boot loader instead
            goto LOAD_LOADER_AP;
        }

        //DEBUG_DECOMP_CRC(uart_printf("CRC OK\n", 0););

        u32RAM_START_PA = msOS_VaToPa(u32RAM_START);
        Ld_printf("u32RAM_START_PA=0x%X\n", u32RAM_START_PA);

    #if 1   //COPY AP BY BDMA

        //Copy uncompressed part first
        Ld_printf("Cp AP_1 to %X\n", u32RAM_START_PA );

        Ld_printf("__BDMA_FlashCopy2MIU0(%X,", u32ROM_START );
        Ld_printf(" %X,", u32RAM_START_PA );
        Ld_printf(" %X)\n", ((UNCOMPRESSED_LENGTH + 7) & ~7UL) );

        __BDMA_FlashCopy2MIU0((u32ROM_START), (u32RAM_START_PA),((UNCOMPRESSED_LENGTH + 7) & ~7UL));

      #ifdef __arm__
        {
        //################################
        //
        // After Devices(IP) access DRAM, it needs to
        // invalidate cache to refresh DRAM for the case
        // that CPU trys to access DRAM.
        //
        //############################
        extern void v7_dma_inv_range(U32 u32Start,U32 u32End);
        U32 u32Start=0,u32End=0;
        u32Start = DECOMPRESS_BUF_CACHEABLE;
        u32End = u32Start + u32ROM_Size;

        Ld_printf("v7_dma_inv_range(%X,", u32Start );
        Ld_printf("%X)\n", u32End );

        v7_dma_inv_range(u32Start,u32End);
        }
      #endif

    #else  // Copy AP by CPU
        //Copy uncompressed part first
        for(i=0;i<((UNCOMPRESSED_LENGTH + 7) & ~7UL);i+=4)
        {
            *((U32 *)(u32RAM_START+i)) = *((U32 *)((U32)(u32ROM_START+i)));
        }

        //Copy compressed part second
        for(i=0;i<(((u32ROM_END-u32ROM_START-UNCOMPRESSED_LENGTH) + 7) & ~7UL);i+=4)
        {
            *((U32 *)(DECOMPRESS_BUF + i)) = *((U32 *)(u32ROM_START + UNCOMPRESSED_LENGTH + i));
        }
    #endif

    #if(ENABLE_FLASH_ON_DRAM)
        if( bDoFOD)
        {
            Ld_printf("[FOD]\n", 0);
            bDoFOD = FOD_ReadHeader(u32RAM_START);

            if( bDoFOD )
            {
                // Check data1 addr...
                if( g_FOD_Header.u32FlashDataAddr_1 >= 0x01000000 ) // Means AP ROM_End
                {
                    g_FOD_Header.u32FlashDataAddr_1 = u32ROM_END;// - 0xBFC00000;
                    g_FOD_Header.u32FlashDataAddr_1 = (((g_FOD_Header.u32FlashDataAddr_1+7) & ~7UL)&0xFFFFFF);
                    DEBUG_DECOMP_FOD( Ld_printf(" => u32FlashDataAddr_1=%X\n", g_FOD_Header.u32FlashDataAddr_1); );
                }

                // BDMA : flash to dram
                __BDMA_FlashCopy2MIU0_No_Wait(
                    g_FOD_Header.u32FlashDataAddr_1,
                    g_FOD_Header.u32DramBaseAddr+g_FOD_Header.u32FlashDataAddr_1,
                    g_FOD_Header.u32FlashDataSize_1);

                // Add info for AP checking...
                {
                    U32 u32FOD_DramBase_Va = msOS_PaToVa_NonCache(g_FOD_Header.u32DramBaseAddr);

                    DEBUG_DECOMP_FOD( Ld_printf("u32FOD_DramBase_Va:0x%X\n", (U32)u32FOD_DramBase_Va); );

                    *((U32 *)(u32FOD_DramBase_Va)) = (U32)FOD_MAGIC_ID;
                    *((U32 *)(u32FOD_DramBase_Va+4)) = (U32)g_FOD_Header.u32DramBaseAddr + g_FOD_Header.u32FlashDataAddr_1;
                    *((U32 *)(u32FOD_DramBase_Va+8)) = (U32)g_FOD_Header.u32FlashDataSize_1;
                }
            }
        }
    #endif


        // Start to decompress
        if( IsCompressMagicNumber == 1 )
        {
            U32 nOutputLength = 0;
            U32 u32DecompSrcBuf = DECOMPRESS_BUF_CACHEABLE;
            U32 u32DecompDstBuf = ((u32RAM_START + UNCOMPRESSED_LENGTH) & NON_CACHEABLE_TO_CACHEABLE_MASK);
            U32 u32DecompPoolBuf = MEMORY_POOL_CACHEABLE;
            U32 u32RealCompDataLen = CompressedFileLength - UNCOMPRESSED_LENGTH;

            DEBUG_DECOMP_DECOMP( Ld_printf("u32RealCompDataLen=0x%X\n", u32RealCompDataLen ); );
            DEBUG_DECOMP_DECOMP( Ld_printf("u32DecompPoolBuf=0x%X\n", u32DecompPoolBuf ); );

            // Clear pool memory
            //memset( (U8 *)u32DecompPoolBuf, 0, MEMORY_POOL_SIZE);

            // Init decomp pool buffer...
            MemoryAllocatorInit((U8 *)u32DecompPoolBuf, MEMORY_POOL_SIZE);


            //Ld_printf("Decomp at %u\r\n", Loader_MDrv_PIU_Get_Time() );
            Ld_printf("MsDecompress(%X,", u32DecompSrcBuf );
            Ld_printf(" %X,", u32DecompDstBuf );
            //Ld_printf(" %X,", CompressedFileLength );
            Ld_printf(" %X,", u32RealCompDataLen );
            Ld_printf(" %X) ", OriginalFileLength );
            Ld_printf(" at %u\n", Loader_MDrv_PIU_Get_Time() );

            //Print_Buffer((U8 *)u32DecompSrcBuf, 0x100);
            //Print_Buffer((U8 *)u32DecompSrcBuf + CompressedFileLength - 0x3800 - 64, 0x100);

            MsDecompress(
                (U8 *)u32DecompSrcBuf,
                (U8 *)u32DecompDstBuf,
                u32RealCompDataLen,//CompressedFileLength,
                OriginalFileLength,
                &nOutputLength );

            if ( nOutputLength != OriginalFileLength )
            {
                Ld_printf(" => DeComp failed!\n", 0);

                //Decompression fail! Jump to boot loader to recover FW
            #ifdef __arm__
                while(1){}
            #endif

                goto LOAD_LOADER_AP;
            }
            //Print_Buffer((U8 *)u32DecompSrcBuf, 0x100);
            //Print_Buffer((U8 *)0x20b38000, 0x100);

        }
        else    // IsCompressMagicNumber == 2
        {
            U32 u32Decomp7SrcBuf = DECOMPRESS_BUF_CACHEABLE;
            U32 u32Decomp7DstBuf = ((u32RAM_START + UNCOMPRESSED_LENGTH) & NON_CACHEABLE_TO_CACHEABLE_MASK);
            U32 u32Decomp7PoolBuf = MEMORY_POOL_CACHEABLE;

            #ifdef __arm__
             u32Decomp7DstBuf |= 0x20000000UL;
            #endif

            Ld_printf("Decomp7 at %u\r\n", Loader_MDrv_PIU_Get_Time() );

            Ld_printf("u32Decomp7PoolBuf=0x%X\n", u32Decomp7PoolBuf );
            MemoryAllocatorInit((U8 *)u32Decomp7PoolBuf, MEMORY_POOL_SIZE);

            Ld_printf("MsDecompress7(%X,", u32Decomp7SrcBuf );
            Ld_printf(" %X,", u32Decomp7DstBuf );
            Ld_printf(" %X)\n", CompressedFileLength+4-UNCOMPRESSED_LENGTH );

            //Print_Buffer((U8 *)u32Decomp7SrcBuf, 0x100);
            //Print_Buffer((U8 *)0x20b38000, 0x100);
            if( MsDecompress7(
                    (U8 *)u32Decomp7SrcBuf,
                    (U8 *)u32Decomp7DstBuf,
                    CompressedFileLength+4-UNCOMPRESSED_LENGTH ) != TRUE
              )
            {
                Ld_printf(" => DeComp7 failed!\n", 0);

                //Decompression fail! Jump to boot loader to recover FW
            #ifdef __arm__
                while(1){}
            #endif
                goto LOAD_LOADER_AP;
            }
            //Print_Buffer((U8 *)u32Decomp7SrcBuf, 0x100);
            //Print_Buffer((U8 *)0x20b38000, 0x100);

        }

        Ld_printf("Decomp done at %u\r\n", Loader_MDrv_PIU_Get_Time() );

        //while(1){}

    #if (ENABLE_FLASH_ON_DRAM)
        if( bDoFOD )
        {
            // Need check BDMA finish before end
            while(1)
            {
                if( __BDMA_Is_JobDone() )
                    break;

                Ld_printf(".", 0 );
            }
        }
    #endif

    }
    else
    {
        // Uncompressed format
        Ld_printf("AP No Comp at %u\r\n", Loader_MDrv_PIU_Get_Time() );

        // Copy AP to dram
        u32RAM_START_PA = msOS_VaToPa(u32RAM_START);

        __BDMA_FlashCopy2MIU0( u32ROM_START, (u32RAM_START_PA), u32ROM_Size);

        Ld_printf("CRC at %u\r\n", Loader_MDrv_PIU_Get_Time() );
    #if 1 // Cal CRC from Dram
        u32CRC32_HW = Loader_MDrv_BDMA_CalCRC(u32RAM_START_PA, u32ROM_Size, 1);
    #else
        //Calculate CRC32 from flash
        while (RIU8[(LOCAL_BDMA_REG_BASE + 0x02)*2] & 0x01);
        RIU[(LOCAL_BDMA_REG_BASE + 0x08 )] = (U16)((U32)(u32ROM_START&0x3fffff));
        RIU[(LOCAL_BDMA_REG_BASE + 0x0A )] = (U16)(((U32)(u32ROM_START&0x3fffff)) >> 16);
        RIU[(LOCAL_BDMA_REG_BASE + 0x0C )] = 0;
        RIU[(LOCAL_BDMA_REG_BASE + 0x0E )] = 0;
        RIU[(LOCAL_BDMA_REG_BASE + 0x10 )] = (U16)((U32)(u32ROM_END-u32ROM_START)) ;
        RIU[(LOCAL_BDMA_REG_BASE + 0x12 )] = (U16)(((U32)(u32ROM_END-u32ROM_START))>>16);
        //polynomial
        RIU[(LOCAL_BDMA_REG_BASE + 0x14 )] = (U16)(0x1DB7);
        RIU[(LOCAL_BDMA_REG_BASE + 0x16 )] = (U16)(0x04C1);
        //seed
        RIU[(LOCAL_BDMA_REG_BASE + 0x18 )] = 0;
        RIU[(LOCAL_BDMA_REG_BASE + 0x1a )] = 0;

        RIU8[(LOCAL_BDMA_REG_BASE + 0x06)*2] = 0x00;   //increasing
        RIU[(LOCAL_BDMA_REG_BASE+0x04)] = 0x0335;  //((DstDEv|DstDataWidth)  <<8) |(SrcDataWidth|SrcDEv) ;

        RIU8[ (LOCAL_BDMA_REG_BASE*2)] = 0x01 ;
        while( 1 )
        {
            if ( (RIU8[(LOCAL_BDMA_REG_BASE+0x02)*2] & 0x08) == 0x08 )
                break;
        }

        //Get CRC32 result
        u32CRC32_HW = RIU[(LOCAL_BDMA_REG_BASE + 0x18 )] ;      //low word
        u32CRC32_HW |= (RIU[(LOCAL_BDMA_REG_BASE + 0x1a )]  <<  16);        //high word
    #endif

        if ( u32CRC32_HW != u32CRC32_SW )
        {
        #if 1
            Ld_printf("CRC fail!\n", 0);
            goto LOAD_LOADER_AP;
        #else
            //AP in flash is bad, load boot loader instead
            u32ROM_START = *((U32 *)(AP_IN_FLASH_ADDR + 0));    //This address should be 4' alignment
            u32RAM_START = *((U32 *)(AP_IN_FLASH_ADDR + 4));    //This address should be 4' alignment
            u32RAM_END = *((U32 *)(AP_IN_FLASH_ADDR + 8));
            u32ROM_END = *((U32 *)(AP_IN_FLASH_ADDR + 12));    //This address should be 4' alignment

            u32CRC32_SW = *((U8 *)(AP_IN_FLASH_ADDR+CRC32_POS+3));
            u32CRC32_SW <<= 8;
            u32CRC32_SW |= *((U8 *)(AP_IN_FLASH_ADDR+CRC32_POS+2));
            u32CRC32_SW <<= 8;
            u32CRC32_SW |= *((U8 *)(AP_IN_FLASH_ADDR+CRC32_POS+1));
            u32CRC32_SW <<= 8;
            u32CRC32_SW |= *((U8 *)(AP_IN_FLASH_ADDR+CRC32_POS+0));
            goto LOAD_CODE_POINT;
        #endif
        }
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    #if 0
        asm volatile (
        // Load ROM_START, RAM_START, RAM_END, ROM_END from 0xBFC20000
        #ifdef CONFIG_MSTAR_KAISERIN
            "li      $15, 0x94050000;" /*0xBFC20000 + u32BOOT_LOADER_LENGTH*/
        #else
            "li      $15, 0xbfc50000;" /*0xBFC20000 + u32BOOT_LOADER_LENGTH*/
        #endif

            "lw      $8, 0($15);"
            "lw      $9, 4($15);"
            "lw      $10, 8($15);"
            "lw      $11, 12($15);"

        "1:;"
            "ld      $12, 0($8);"      //#ld=lw lw to save time
            "ld      $14, 8($8);"
            "sd      $12, 0($9);"
            "sd      $14, 8($9);"
            "addu    $8, $8, 16;"
            "addu    $9, $9, 16;"
            "bltu    $9, $10, 1b;"
            "nop;"
            :::"$15", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "memory"
        );
    #endif

    }

   // init_version();
}

#else

//================================================================
#if 0
void __loader_2nd Loader_PrintChunkHeader(StuChunkHeaderInfo* pstChunkHeaderInfo);
void __loader_2nd Loader_PrintChunkHeader(StuChunkHeaderInfo* pstChunkHeaderInfo)
{
}
#endif

#if( ENABLE_JOBA_BIN )

#define DEBUG_JOBA(x)   //x

void __loader_2nd Loader_Joba_Init_Aeon(U32 u32MemAddr);
void __loader_2nd Loader_Joba_Init_Aeon(U32 u32MemAddr)
{
    // Pass info to joba...
    // Set magic id
    Ld_Mem_WriteU32(u32MemAddr, JOBA_PARA_MAGIC_ID);

    // Set Aeon dram Base
    Ld_Mem_WriteU32(u32MemAddr+4, u32MemAddr);


    Ld_HAL_COPRO_Enable(u32MemAddr);
}

BOOL __loader_2nd Loader_Joba_Init(U32 u32ApFlashAddr);
BOOL __loader_2nd Loader_Joba_Init(U32 u32ApFlashAddr)
{
    U32 u32JobaPtr;
    U16 u16BinCount;
    U32 u32BinTableAddr = u32ApFlashAddr + AP_BIN_TABLE_START_ADDR;

    DEBUG_JOBA( Ld_printf("Loader_Joba_Init() at %u \n", Loader_MDrv_PIU_Get_Time()); );

    u32JobaPtr = Ld_MDrv_Flash_ReadU32(u32ApFlashAddr+AP_IMG_INFO_JOBA_DATA_PTR_ADDR);
    DEBUG_JOBA( Ld_printf(" u32JobaPtr=0x%X\n", u32JobaPtr); );
    if( u32JobaPtr == 0 )
    {
        Ld_printf("-- No Joba data\n", 0);
        return FALSE;
    }

    // Find joba.bin from bin table...

    // Get bin number
    u16BinCount = Ld_MDrv_Flash_ReadU16_BE(u32BinTableAddr + BIN_TBL_OFFSET_BIN_COUNT);
    DEBUG_JOBA( Ld_printf(" u16BinCount=0x%X\n", u16BinCount); );
    // Check bin count valid?
    if( u16BinCount > 100 )
    {
        return FALSE;
    }

    // Check if joba bin exist
    {
        U8 u8BinIndex;
        U32 u32BinItemAddr;
        U16 u16BinId;
        U32 u32JobaBinFlashAddr;
        U32 u32JobaBinSize;

        for( u8BinIndex = 0 ; u8BinIndex < u16BinCount; u8BinIndex += 1 )
        {
            u32BinItemAddr = u32BinTableAddr + BIN_TBL_OFFSET_BIN_ITEM_START
                             + (u8BinIndex * BIN_TBL_BIN_ITEM_SIZE);

            u16BinId = Ld_MDrv_Flash_ReadU16_BE(u32BinItemAddr);

            DEBUG_JOBA( Ld_printf(" u16BinId=0x%X\n", u16BinId); );

            if( u16BinId == 0x98 ) // Joba.bin id is 0x98
            {
                // Get joba info
                u32JobaBinFlashAddr = Ld_MDrv_Flash_ReadU32_BE(u32BinItemAddr+2);
                u32JobaBinSize = Ld_MDrv_Flash_ReadU32_BE(u32BinItemAddr+6);
                DEBUG_JOBA( Ld_printf(" u32JobaBinFlashAddr=0x%X\n", u32JobaBinFlashAddr); );
                DEBUG_JOBA( Ld_printf(" u32JobaBinSize=0x%X\n", u32JobaBinSize); );

                // BDMA joba to dram (addr=JOBA_MMAP_CODE_ADDR)
                Loader2_MDrv_BDMA_CopyFlash2Dram(u32JobaBinFlashAddr, JOBA_MMAP_CODE_ADDR, u32JobaBinSize, 1);

                Loader_Joba_Init_Aeon(JOBA_MMAP_CODE_ADDR);
                break;
            }
        }
    }

    return TRUE;
}
#endif

//================================================================

#if( ENABLE_MSTAR_WHISKY )
    #define ENABLE_UART_DEBUG   1
#else
    #define ENABLE_UART_DEBUG   0
#endif

#if( ENABLE_UART_DEBUG )

    #define DEBUG_DECOMP(x)         x

    #define DEBUG_DECOMP_CRC(x)     do{ if(g_DeComp_u8DebugFlag&DEBUG_FLAG_CRC)     {x;} }while(0)
    #define DEBUG_DECOMP_DECOMP(x)  do{ if(g_DeComp_u8DebugFlag&DEBUG_FLAG_DECOMP)  {x;} }while(0)
    #define DEBUG_DECOMP_FOD(x)     do{ if(g_DeComp_u8DebugFlag&DEBUG_FLAG_FOD)     {x;} }while(0)

#else
    #define DEBUG_DECOMP(x)
    #define DEBUG_DECOMP_CRC(x)
    #define DEBUG_DECOMP_DECOMP(x)
    #define DEBUG_DECOMP_FOD(x)

#endif

#include "MSDecompress_NonOS_FOD.c"

#define AP_IN_FLASH_ADDR    0x00020000      //This address should be 4' alignment

void __loader_2nd DoMSDecompression(void)
{
    U8 IsCompressMagicNumber = 0;
    U32 CompressedFileLength = 0;
    U32 OriginalFileLength = 0;
    U32 u32OutputLength;
    U32 u32FlashIOAddr;
    U32 u32ROM_START = 0;
    U32 u32ROM_END = 0;
    U32 u32RomSize = 0;
    U32 u32RAM_START = 0, u32RAM_END = 0;
    U32 u32BOOT_LOADER_LENGTH = 0;

    U32 u32CRC32_HW = 0, u32CRC32_SW = 0;
    U32 u32Header_BootLoaderSize;

    StuChunkHeaderInfo stChunkHeaderInfo;
    StuApImgInfo stApImgInfo;
    BOOL bLoad2ndAP = FALSE;

#if( ENABLE_JOBA_BIN )
    BOOL bEnableJoba = TRUE;
#endif

#if(ENABLE_FLASH_ON_DRAM)
    BOOL bDoFOD = FALSE;
#endif


#if( ENABLE_AEON_UART_PRINTF /*|| ENABLE_LOADER2_AEON_UART*/)
    // Copy sboot.bin  to dram for call function
    Loader2_MDrv_BDMA_CopyFlash2Dram(0x0000, 0x0000, 0x10000, 1 );
#endif

#if( ENABLE_AEON_UART_PRINTF )
    MDrv_InitConsole();
#elif( ENABLE_LOADER2_AEON_UART )
    Loader2_MDrv_Uart_Init();
#endif

    DEBUG_DECOMP( Ld_printf("\n==== DoMSDecompression() at %u =====\n", Loader_MDrv_PIU_Get_Time()); );


    u32FlashIOAddr = Loader_MDrv_Flash_Get_IOAddr();
    DEBUG_DECOMP( Ld_printf("u32FlashIOAddr=0x%X\n", u32FlashIOAddr); );
    //DEBUG_DECOMP( Ld_printf("AP_IN_FLASH_ADDR=0x%X\n", AP_IN_FLASH_ADDR); );


    // Get first ap chunk header info
    DEBUG_DECOMP( Ld_printf(" Get 1st chunk header at flash %X\n", AP_IN_FLASH_ADDR););
    Loader_Get_ChunkHeader(AP_IN_FLASH_ADDR, &stChunkHeaderInfo);
    DEBUG_DECOMP( Ld_printf(" DebugFlag=0x%X\n", stChunkHeaderInfo.u8DebugFlag););

    // Get boot loader bin size from first ap chunker-header
    u32Header_BootLoaderSize = stChunkHeaderInfo.u32BootLoaderSize;
    DEBUG_DECOMP( Ld_printf("  u32Header_BootLoaderSize=%X\n", u32Header_BootLoaderSize););

    if( u32Header_BootLoaderSize ) // boot loader is exist
    {
        // Get u32BOOT_LOADER_LENGTH
        u32BOOT_LOADER_LENGTH = u32Header_BootLoaderSize - 0x20000; //minus 64k for sboot and 64k for pm code
    }
    else
    {
        u32BOOT_LOADER_LENGTH = 0;
    }
    DEBUG_DECOMP( Ld_printf("  u32BOOT_LOADER_LENGTH=%X\n", u32BOOT_LOADER_LENGTH););

    // If boot loader exist, get 2nd ap chunk-header
    if( u32BOOT_LOADER_LENGTH )
    {
        bLoad2ndAP = TRUE;
    }


    // Which AP? BootLoader or Main AP?
    //u32BINARY_ID = *((U32 *)(u32FlashIOAddr + CHUNK_HEADER_ADDR + 0x1C));
    //DEBUG_DECOMP( Ld_printf("u32BINARY_ID=0x%X\n", u32BINARY_ID); );


    // Check magic end/start
    /*if( (u32ROM_START >= AP_IN_FLASH_ADDR)
      && (u32ROM_START < (AP_IN_FLASH_ADDR+u32REV_FLASH_SIZE))
      && (u32ROM_END >= AP_IN_FLASH_ADDR)
      && (u32ROM_END < (AP_IN_FLASH_ADDR+u32REV_FLASH_SIZE))) //u32REV_FLASH_SIZE: flash reserved size
      */
    //bLoad2ndAP = TRUE;
    if( bLoad2ndAP )
    {
        // Main AP
        DEBUG_DECOMP( Ld_puts("\n----- Load 2nd AP chunk header: -----\n"); );

        // Get 2nd ap chunk header info
        DEBUG_DECOMP( Ld_printf(" Get 2nd chunk header at flash %X\n", AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH););
        Loader_Get_ChunkHeader(AP_IN_FLASH_ADDR + u32BOOT_LOADER_LENGTH, &stChunkHeaderInfo);
        DEBUG_DECOMP( Ld_printf(" DebugFlag=0x%X\n", stChunkHeaderInfo.u8DebugFlag););

        u32ROM_START = stChunkHeaderInfo.u32RomStart;
        u32ROM_END = stChunkHeaderInfo.u32RomEnd;
        u32RAM_START = stChunkHeaderInfo.u32RamStart;
        u32RAM_END = stChunkHeaderInfo.u32RamEnd;
        DEBUG_DECOMP( Ld_printf("  u32ROM_START=0x%X\n", u32ROM_START); );
        DEBUG_DECOMP( Ld_printf("  u32ROM_END=0x%X\n", u32ROM_END); );
        DEBUG_DECOMP( Ld_printf("  u32RAM_START=0x%X\n", u32RAM_START); );
        DEBUG_DECOMP( Ld_printf("  u32RAM_END=0x%X\n", u32RAM_END); );


        // Read imgInfo
        DEBUG_DECOMP( Ld_printf("Read ImgInfo from %X\n", u32ROM_START); );
        Loader_Get_ApImgInfo(u32ROM_START, &stApImgInfo);
        DEBUG_DECOMP( Ld_printf(" u32MagicId=%X\n", stApImgInfo.u32MagicId); );
        DEBUG_DECOMP( Ld_printf(" u32MagicEndAddr=%X\n", stApImgInfo.u32MagicEndAddr); );
        //DEBUG_DECOMP( Ld_printf(" u32MagicEnd=%X\n", stApImgInfo.u32MagicEnd); );

        //DEBUG_DECOMP( Ld_printf("COMPRESSED_LENGTH_POS=0x%X\n", (U32)COMPRESSED_LENGTH_POS); );

        // Check magic end/start
        {
            U32 u32MAGIC_END_ADDR, u32MAGIC_END, u32MAGIC_START;

            // Check AP magic ID ...
            //DEBUG_DECOMP( Ld_printf("MagicId_1 addr=0x%X\n", u32ROM_START+AP_IMG_INFO_MAGIC_ID_ADDR); );
            //u32MAGIC_START = Ld_MDrv_Flash_ReadU32(u32ROM_START+COMPRESSED_LENGTH_POS-0x4A);
            //u32MAGIC_START = Ld_MDrv_Flash_ReadU32(u32ROM_START+AP_IMG_INFO_MAGIC_ID_ADDR);
            u32MAGIC_START = stApImgInfo.u32MagicId;
            //DEBUG_DECOMP( Ld_printf("u32MAGIC_START=0x%X\n", u32MAGIC_START); );
            if(u32MAGIC_START != 0x55AA5678)
            {
                //DEBUG_DECOMP( Ld_puts("u32MAGIC_START != 0x55AA5678\n"); );
                Ld_puts("u32MAGIC_START != 0x55AA5678\n");
                goto LOAD_LOADER_AP;
            }

            //DEBUG_DECOMP( Ld_printf("u32MAGIC_END_ADDR addr=0x%X\n", u32ROM_START+AP_IMG_INFO_MAGIC_ID_2_ADDR); );
            //u32MAGIC_END_ADDR = Ld_MDrv_Flash_ReadU32(u32ROM_START+AP_IMG_INFO_MAGIC_ID_2_ADDR);
            u32MAGIC_END_ADDR = stApImgInfo.u32MagicEndAddr;
            //DEBUG_DECOMP( Ld_printf("u32MAGIC_END_ADDR=0x%X\n", u32MAGIC_END_ADDR); );

            u32MAGIC_END = Ld_MDrv_Flash_ReadU32(u32MAGIC_END_ADDR);
            DEBUG_DECOMP( Ld_printf("u32MAGIC_END=0x%X\n", u32MAGIC_END); );
            if( u32MAGIC_END != 0x55AAABCD )
            {
                Ld_puts("u32MAGIC_END != 0x55AAABCD\n");
                goto LOAD_LOADER_AP;
            }

        }
    }
    else
    {
        // Loader 1st AP
LOAD_LOADER_AP:
        DEBUG_DECOMP( Ld_puts("\nLOAD_LOADER_AP:\n"); );
        DEBUG_DECOMP( Ld_puts("Load 1st AP\n"); );

        // Use first AP(No matter what it is)
        Loader_Get_ChunkHeader(AP_IN_FLASH_ADDR, &stChunkHeaderInfo);

        u32ROM_START = stChunkHeaderInfo.u32RomStart;
        u32ROM_END = stChunkHeaderInfo.u32RomEnd;
        u32RAM_START = stChunkHeaderInfo.u32RamStart;
        u32RAM_END = stChunkHeaderInfo.u32RamEnd;
        DEBUG_DECOMP( Ld_printf(" u32ROM_START=0x%X\n", u32ROM_START); );
        DEBUG_DECOMP( Ld_printf(" u32ROM_END=0x%X\n", u32ROM_END); );
        DEBUG_DECOMP( Ld_printf(" u32RAM_START=0x%X\n", u32RAM_START); );
        DEBUG_DECOMP( Ld_printf(" u32RAM_END=0x%X\n", u32RAM_END); );
    }

    // Setup debug flag
    g_DeComp_u8DebugFlag = stChunkHeaderInfo.u8DebugFlag;

    u32RomSize = u32ROM_END - u32ROM_START;
    DEBUG_DECOMP( Ld_printf(" u32RomSize=0x%X\n", u32RomSize); );
    u32CRC32_SW = stChunkHeaderInfo.u32CRC;
    DEBUG_DECOMP_CRC( Ld_printf(" u32CRC32_SW=0x%X\n", u32CRC32_SW); );


LOAD_CODE_POINT:
    DEBUG_DECOMP( Ld_printf("\n--- LOAD_CODE_POINT: at %u ---\n", Loader_MDrv_PIU_Get_Time()); );

    Loader_Get_ApImgInfo(u32ROM_START, &stApImgInfo);

    //au8CompressInfo[0] = Ld_MDrv_Flash_ReadU8( u32ROM_START+AP_IMG_INFO_COMPRESS_INFO_ADDR+0 );
    //au8CompressInfo[1] = Ld_MDrv_Flash_ReadU8( u32ROM_START+AP_IMG_INFO_COMPRESS_INFO_ADDR+1 );
    //au8CompressInfo[2] = Ld_MDrv_Flash_ReadU8( u32ROM_START+AP_IMG_INFO_COMPRESS_INFO_ADDR+2 );
    DEBUG_DECOMP_DECOMP( Ld_printf(" u32MagicId=0x%x\n", stApImgInfo.u32MagicId); );
    DEBUG_DECOMP_DECOMP( Ld_printf(" au8CompressInfo[0]=0x%x,", stApImgInfo.au8CompressMagicNum[0]); );
    DEBUG_DECOMP_DECOMP( Ld_printf(" 0x%x,", stApImgInfo.au8CompressMagicNum[1]); );
    DEBUG_DECOMP_DECOMP( Ld_printf(" 0x%x\n", stApImgInfo.au8CompressMagicNum[2]); );

    // Check if this bin file is compressed format, find compress format magic number
    if( (stApImgInfo.au8CompressMagicNum[2] == 0xDE )
     && (stApImgInfo.au8CompressMagicNum[1] == 0xAD )
      )
    {
        U8 u8CompressFormat = stApImgInfo.au8CompressMagicNum[0];
        DEBUG_DECOMP_DECOMP( Ld_printf(" u8CompressFormat=0x%x\n", u8CompressFormat); );

        if ( u8CompressFormat == 0xBE )
        {
            IsCompressMagicNumber = 1; // comp
        }
        else if ( u8CompressFormat == 0xEF )
        {
            IsCompressMagicNumber = 2; // comp7
        }
        else
        {
            IsCompressMagicNumber = 0;
        }
    }
    else
    {
        IsCompressMagicNumber = 0;
    }

    DEBUG_DECOMP_DECOMP( Ld_printf(" IsCompressMagicNumber=%u\n", IsCompressMagicNumber); );

#if(ENABLE_FLASH_ON_DRAM)
    if( (IsCompressMagicNumber != 0) // Ap no comp ==> No need FOD
      &&(u32RomSize >= 0x100000) // It's normal AP, not bloader! BLoader no need FOD
      )
    {
        bDoFOD = TRUE;
    }
#endif


    //Compressed Format
    if ( IsCompressMagicNumber )
    {
        U32 u32DecompDstMemAddr;
        U32 u32DecompDstMemSize;
        U32 u32DecompSrcMemAddr;
        U32 u32DecompSrcMemSize;
        U32 u32DecompTmpMemAddr;
        U32 u32CrcBuf_PA = 0;


        DEBUG_DECOMP( Ld_puts("\n--- AP is compressed format ---\n"); );

        // Get compressed file size
        CompressedFileLength = stApImgInfo.u32CompressedLength;
        //CompressedFileLength = Ld_MDrv_Flash_ReadU32( u32ROM_START+AP_IMG_INFO_COMPRESS_LEN_ADDR );
        CompressedFileLength -= 6;
        DEBUG_DECOMP_DECOMP( Ld_printf("  CompressedFileLength=0x%X\n", CompressedFileLength); );

        // Get Original file size
        if( IsCompressMagicNumber == 1 ) // comp
        {
            DEBUG_DECOMP_DECOMP( Ld_printf(" u32ROM_START+CompressedFileLength=0x%X\n", u32ROM_START+CompressedFileLength); );
            OriginalFileLength = Ld_MDrv_Flash_ReadU32( u32ROM_START+CompressedFileLength);
        }
        else if( IsCompressMagicNumber == 2 )// comp7
        {
            DEBUG_DECOMP_DECOMP( Ld_printf(" u32ROM_START+UNCOMPRESSED_LENGTH+5=0x%X\n", u32ROM_START+UNCOMPRESSED_LENGTH+5); );
            OriginalFileLength = Ld_MDrv_Flash_ReadU32( u32ROM_START+UNCOMPRESSED_LENGTH+5);
        }
        else
        {
            //OriginalFileLength = 0x800000; // 8M
        }
        DEBUG_DECOMP( Ld_printf("  OriginalFileLength=0x%X\n", OriginalFileLength); );

        //-------------------------------------------------------------------------------------
        // Setup memory for decomp ...
        u32DecompDstMemAddr = u32RAM_START + UNCOMPRESSED_LENGTH;
        u32DecompDstMemSize = OriginalFileLength;

    #if defined(__aeon__)
        u32DecompSrcMemAddr = MMAP_DECOMP_SRC_BUF_ADDR;
    #else
        u32DecompSrcMemAddr = u32DecompDstMemAddr + u32DecompDstMemSize;
        u32DecompSrcMemAddr = ALIGNED_VALUE(u32DecompSrcMemAddr, 0x10000); // Size = CompressedFileLength
    #endif

        u32DecompSrcMemSize = ALIGNED_VALUE(CompressedFileLength, 0x10000);

        u32DecompTmpMemAddr = u32DecompSrcMemAddr + u32DecompSrcMemSize;

        u32CrcBuf_PA = u32DecompSrcMemAddr - UNCOMPRESSED_LENGTH;

        DEBUG_DECOMP_DECOMP( Ld_printf(" u32DecompDstMemAddr=0x%X\n", u32DecompDstMemAddr); );
        DEBUG_DECOMP_DECOMP( Ld_printf(" u32DecompDstMemSize=0x%X\n", u32DecompDstMemSize); );
        DEBUG_DECOMP_DECOMP( Ld_printf(" u32DecompSrcMemAddr=0x%X\n", u32DecompSrcMemAddr); );
        DEBUG_DECOMP_DECOMP( Ld_printf(" u32DecompSrcMemSize=0x%X\n", u32DecompSrcMemSize); );
        DEBUG_DECOMP_DECOMP( Ld_printf(" u32DecompTmpMemAddr=0x%X\n", u32DecompTmpMemAddr); );


        // Calculate CRC32 from Dram...

        // Copy AP to dram
        DEBUG_DECOMP_CRC( Ld_printf(" u32CrcBuf_PA=0x%X\n", u32CrcBuf_PA); );
        DEBUG_DECOMP( Ld_printf("CP Ap at %u\n", Loader_MDrv_PIU_Get_Time() ); );
        Loader2_MDrv_BDMA_CopyFlash2Dram(u32ROM_START, u32CrcBuf_PA, u32RomSize, 1 );

        // CRC from dram
        DEBUG_DECOMP( Ld_printf("CRC at %u\n", Loader_MDrv_PIU_Get_Time() ); );
        u32CRC32_HW = Loader2_MDrv_BDMA_CalCRC(u32CrcBuf_PA, u32RomSize, BDMA_CRC_TYPE_DRAM);
        DEBUG_DECOMP( Ld_printf("CRC done at %u\n", Loader_MDrv_PIU_Get_Time() ); );
        DEBUG_DECOMP_CRC( Ld_printf(" u32CRC32_HW=0x%X\n", u32CRC32_HW); );
        //DEBUG_DECOMP_CRC( Ld_printf(" u32CRC32_SW=0x%X\n", u32CRC32_SW); );
        if ( u32CRC32_HW != u32CRC32_SW )
        {
            Ld_printf("\nCRC Error: u32CRC32_HW=0x%X,", u32CRC32_HW);
            Ld_printf(" u32CRC32_SW=0x%X\n", u32CRC32_SW);

            //AP in flash is bad, load boot loader instead
            goto LOAD_LOADER_AP;
        }

    #if 0 //Calculate CRC32 from flash
        DEBUG_DECOMP( Ld_printf(" CRC at %u \n", Loader_MDrv_PIU_Get_Time()); );
        u32CRC32_HW = Loader2_MDrv_BDMA_CalCRC(u32ROM_START, u32ROM_END-u32ROM_START, BDMA_CRC_TYPE_FLASH);
        DEBUG_DECOMP( Ld_printf(" CRC done at %u \n", Loader_MDrv_PIU_Get_Time()); );
        DEBUG_DECOMP_CRC( Ld_printf(" u32CRC32_HW=0x%X\n", u32CRC32_HW); );
        if ( u32CRC32_HW != u32CRC32_SW )
        {
            Ld_printf("CRC error: u32CRC32_HW=0x%X,", u32CRC32_HW);
            Ld_printf(" u32CRC32_SW=0x%X\n", u32CRC32_SW);

            //AP in flash is bad, load boot loader instead
            goto LOAD_LOADER_AP;
        }
    #endif

        DEBUG_DECOMP( Ld_printf(" BDMA AP to dram at %u\n", Loader_MDrv_PIU_Get_Time()); );

        //Copy uncompressed part first
        Ld_printf("Cp AP_1 to %X\n", u32RAM_START );
        Loader2_MDrv_BDMA_CopyFlash2Dram(u32ROM_START, u32RAM_START, UNCOMPRESSED_LENGTH, 1 );

        //Copy compressed part second
    #if 0 // Already copy to dram at checking CRC
        Loader2_MDrv_BDMA_CopyFlash2Dram(u32ROM_START + UNCOMPRESSED_LENGTH, u32DecompSrcMemAddr, u32RAM_END-u32RAM_START-UNCOMPRESSED_LENGTH, 1 );
    #endif

    #if( ENABLE_JOBA_BIN )
        bEnableJoba = Loader_Joba_Init(u32ROM_START);
    #endif

    #if(ENABLE_JOBA_BIN&&ENABLE_FLASH_ON_DRAM)
        if( bEnableJoba )
        {
            bDoFOD = FALSE;
        }
    #endif

    #if(ENABLE_FLASH_ON_DRAM)
        if( bDoFOD )
        {
            Ld_printf("[FOD]\n", 0);
            bDoFOD = FOD_ReadHeader(u32RAM_START);

            if( bDoFOD )
            {
                // Check data1 addr...
                if( g_FOD_Header.u32FlashDataAddr_1 >= 0x01000000 ) // Means AP ROM_End
                {
                    g_FOD_Header.u32FlashDataAddr_1 = u32ROM_END;
                    g_FOD_Header.u32FlashDataAddr_1 = (((g_FOD_Header.u32FlashDataAddr_1+7) & ~7UL)&0xFFFFFF);
                    DEBUG_DECOMP_FOD( Ld_printf(" => u32FlashDataAddr_1=%X\n", g_FOD_Header.u32FlashDataAddr_1); );
                }

                // BDMA : flash to dram
                Loader2_MDrv_BDMA_CopyFlash2Dram(
                    g_FOD_Header.u32FlashDataAddr_1,
                    g_FOD_Header.u32DramBaseAddr+g_FOD_Header.u32FlashDataAddr_1,
                    g_FOD_Header.u32FlashDataSize_1,
                    0);

                // Add info for AP checking...
                *((U32 *)(g_FOD_Header.u32DramBaseAddr)) = (U32)FOD_MAGIC_ID;
                *((U32 *)(g_FOD_Header.u32DramBaseAddr+4)) = (U32)g_FOD_Header.u32DramBaseAddr + g_FOD_Header.u32FlashDataAddr_1;
                *((U32 *)(g_FOD_Header.u32DramBaseAddr+8)) = (U32)g_FOD_Header.u32FlashDataSize_1;

            }
        }
    #endif

        // Start to decompress
        if( IsCompressMagicNumber == 1 )
        {
            DEBUG_DECOMP( Ld_printf(" Start decomp at %u \n", Loader_MDrv_PIU_Get_Time()); );

            //MemoryAllocatorInit((U8 *)MEMORY_POOL_CACHEABLE, MEMORY_POOL_SIZE);
            MemoryAllocatorInit((U8 *)u32DecompTmpMemAddr, MEMORY_POOL_SIZE);

            //MsDecompress((U8 *)DECOMPRESS_BUF_CACHEABLE, (U8 *)((u32RAM_START + UNCOMPRESSED_LENGTH) & NON_CACHEABLE_TO_CACHEABLE_MASK), CompressedFileLength, OriginalFileLength, &u32OutputLength);
            MsDecompress(   (U8 *)u32DecompSrcMemAddr, // src
                            //(U8 *)((u32RAM_START + UNCOMPRESSED_LENGTH) & NON_CACHEABLE_TO_CACHEABLE_MASK), // Dst
                            //(U8 *)((u32DecompDstMemAddr) & NON_CACHEABLE_TO_CACHEABLE_MASK), // Dst
                            (U8 *)u32DecompDstMemAddr, // Dst
                            CompressedFileLength, // srcLen
                            OriginalFileLength, // DstLen
                            &u32OutputLength);

            if ( u32OutputLength != OriginalFileLength )
            {
                //Decompression fail! Jump to boot loader to recover FW
                //Ld_puts("Decomp fail!\n");
                Ld_printf("\n Decomp fail! u32OutputLength=%u\n", u32OutputLength );
                while(1){}
            }
            else
            {
                DEBUG_DECOMP( Ld_printf(" decomp done at %u \n", Loader_MDrv_PIU_Get_Time()); );
            }
        }
        else
        {
            BOOL bDecomp7Result;

            DEBUG_DECOMP( Ld_printf(" Start decomp7 at %u \n", Loader_MDrv_PIU_Get_Time()); );

            //MemoryAllocatorInit((U8 *)MEMORY_POOL_CACHEABLE, MEMORY_POOL_SIZE);
            MemoryAllocatorInit((U8 *)u32DecompTmpMemAddr, MEMORY_POOL_SIZE);

            bDecomp7Result = MsDecompress7(
                //(U8 *)DECOMPRESS_BUF_CACHEABLE, // src
                (U8 *)u32DecompSrcMemAddr, // src
                //(U8 *)((u32RAM_START + UNCOMPRESSED_LENGTH) & NON_CACHEABLE_TO_CACHEABLE_MASK), // dst
                (U8 *)u32DecompDstMemAddr, // dst
                CompressedFileLength+4-UNCOMPRESSED_LENGTH // srcLen
            );

            if ( bDecomp7Result != TRUE )
            {
                //Decompression fail! Jump to boot loader to recover FW
                Ld_puts("Decomp7 fail!\n");
                while(1){}
            }
            else
            { //do d-cache flush
                U32 u32DstAddr, u32Len;

                //u32DstAddr = (U32)((u32RAM_START + UNCOMPRESSED_LENGTH) & NON_CACHEABLE_TO_CACHEABLE_MASK);
                u32DstAddr = u32DecompDstMemAddr;
                u32Len = (U32)(CompressedFileLength+4-UNCOMPRESSED_LENGTH);
                mhal_dcache_flush(u32DstAddr,u32Len);

                DEBUG_DECOMP( Ld_printf(" Decomp7 done at %u \n", Loader_MDrv_PIU_Get_Time()); );
            }
        }

    #if (ENABLE_FLASH_ON_DRAM)
        if( bDoFOD )
        {
            while(1)
            {
                if( Loader2_MDrv_BDMA_Is_JobDone() )
                    break;

                Ld_printf(".", 0 );
            }
        }
    #endif

    }
    else
    {
        DEBUG_DECOMP( Ld_puts("\n----- AP is not compressed format: -----\n"); );

        DEBUG_DECOMP( Ld_printf(" CRC check at %u \n", Loader_MDrv_PIU_Get_Time()); );

        //Calculate CRC32 from flash
        u32CRC32_HW = Loader2_MDrv_BDMA_CalCRC(u32ROM_START, u32ROM_END-u32ROM_START, BDMA_CRC_TYPE_FLASH);
        if ( u32CRC32_HW != u32CRC32_SW )
        {
            //AP in flash is bad, load boot loader instead
            goto LOAD_CODE_POINT;
        }
        DEBUG_DECOMP( Ld_printf(" CRC done at %u \n", Loader_MDrv_PIU_Get_Time()); );

        // Copy ap.bin from flash to dram
        // Uncompressed format
        Loader2_MDrv_BDMA_CopyFlash2Dram(u32ROM_START, u32RAM_START, u32RAM_END - u32RAM_START, 1 );
    }

   // init_version();

    //Ld_puts("Reboot to AP...\n");
    Ld_printf("\n===== Reboot to AP... at %u =====\n", Loader_MDrv_PIU_Get_Time());

#ifdef __AEONR2__

  #if 1//(ENABLE_MSTAR_NASA||ENABLE_MSTAR_EDEN)
    ReBootAtDRAM(u32RAM_START);
  #elif (ENABLE_MSTAR_MARIA10 || ENABLE_MSTAR_MACAW12 || ENABLE_MSTAR_EDEN || ENABLE_MSTAR_EULER || ENABLE_MSTAR_NASA || ENABLE_MSTAR_MAYA  || ENABLE_MSTAR_WHISKY)
    Boot2DRAM();
  #else
    // Move from bootaeonsysinit.c
    // wait 51 polling this register and then reset to new vector table address.
    if(u32RAM_START==0)
    {
        __asm__ __volatile__ (
            "\tl.syncwritebuffer\n"
            "\tl.jr     %0\n"
            "\tl.syncwritebuffer\n"
            : : "r" (0x100));
    }
    else
    {
        RIU[0x1002B4] = u32RAM_START >> 16;
    }
  #endif

    while(1);
#endif

}

#endif
#endif
