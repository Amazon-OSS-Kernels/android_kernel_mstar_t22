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

 #if (ENABLE_MSTAR_MARLON || ENABLE_MSTAR_WHISKY || ENABLE_MSTAR_M3822)
    #define ENABLE_FLASH_ON_DRAM    1

 #else
    #define ENABLE_FLASH_ON_DRAM    0

 #endif


#if( ENABLE_FLASH_ON_DRAM)

#define FOD_MAGIC_ID        0x12344321
#define FOD_MAGIC_ID_2      0x43211234

#if( defined(__aeon__) )
    #define PROG_IMG_INFO_OFFSET        (0x1100)

#elif( defined(__arm__) )
    #define PROG_IMG_INFO_OFFSET        (0x1100)

#elif( defined(__mips__) )
    #define PROG_IMG_INFO_OFFSET        (0xF80)

#endif

#define PROG_IMG_INFO_FOD_POINTER_OFFSET (PROG_IMG_INFO_OFFSET+0x58)

typedef struct
{
    U32 u32FODMagicId_1;

    U32 u32DramBaseAddr;

    U32 u32FlashDataAddr_1;
    U32 u32FlashDataSize_1;

    U32 u32FlashDataAddr_2;
    U32 u32FlashDataSize_2;

    U32 u32FODMagicId_2;
} __attribute__ ((packed)) StuFODDataForImgInfo;

static StuFODDataForImgInfo g_FOD_Header;

#if( defined(__aeon__) )
    #define FOD_printf  Ld_printf

#elif( defined(__mips__) )
    #define FOD_printf  Ld_printf

#elif( defined(__arm__) )
    #define FOD_printf  Ld_printf

#endif

BOOL __loader_2nd FOD_ReadHeader(U32 u32ApRamStartAddr);
BOOL __loader_2nd FOD_ReadHeader(U32 u32ApRamStartAddr)
{
    U32 u32FOD_Pointer = *(U32*)(u32ApRamStartAddr+PROG_IMG_INFO_FOD_POINTER_OFFSET);

    DEBUG_DECOMP_FOD( FOD_printf("u32ApRamStartAddr=%X\n", u32ApRamStartAddr); );
    //DEBUG_DECOMP_FOD( FOD_printf("pointer addr=%X\n", u32ApRamStartAddr+PROG_IMG_INFO_FOD_POINTER_OFFSET); );
    DEBUG_DECOMP_FOD( FOD_printf("u32FOD_Pointer=%X\n", u32FOD_Pointer); );

    if( u32FOD_Pointer == 0 )
    {
        FOD_printf("No FOD data\n", 0);
        return FALSE;
    }

    // Copy data to g_FOD_Header
    Loader_MemCpy( (U8*)&g_FOD_Header, (U8*)u32FOD_Pointer, sizeof(StuFODDataForImgInfo) );

    //DEBUG_DECOMP_FOD( uart_printf("id1=%X\n", g_FOD_Header.u32FODMagicId_1); );
    //DEBUG_DECOMP_FOD( uart_printf("id2=%X\n", g_FOD_Header.u32FODMagicId_2); );

    if( g_FOD_Header.u32FODMagicId_1 != FOD_MAGIC_ID
      ||g_FOD_Header.u32FODMagicId_2 != FOD_MAGIC_ID_2
      )
    {
        FOD_printf("FOD id error=%X,", g_FOD_Header.u32FODMagicId_1);
        FOD_printf("%X\n", g_FOD_Header.u32FODMagicId_2);
        return FALSE;
    }

    DEBUG_DECOMP_FOD( FOD_printf("u32DramBaseAddr=%X\n", g_FOD_Header.u32DramBaseAddr); );

    DEBUG_DECOMP_FOD( FOD_printf("u32FlashDataAddr_1=%X\n", g_FOD_Header.u32FlashDataAddr_1); );
    DEBUG_DECOMP_FOD( FOD_printf("u32FlashDataSize_1=%X\n", g_FOD_Header.u32FlashDataSize_1); );

    if( g_FOD_Header.u32FlashDataSize_2 )
    {
        DEBUG_DECOMP_FOD( FOD_printf("u32FlashDataAddr_2=%X\n", g_FOD_Header.u32FlashDataAddr_2); );
        DEBUG_DECOMP_FOD( FOD_printf("u32FlashDataSize_2=%X\n", g_FOD_Header.u32FlashDataSize_2); );
    }

    return TRUE;
}

#endif // ENABLE_FLASH_ON_DRAM

#endif // ENABLE_NON_OS

