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

#ifndef __MSDECOMPRESS_NONOS_PRIV_H__
#define __MSDECOMPRESS_NONOS_PRIV_H__


//===================================================
#define FLASH_SIZE_MAX  0x1000000 // 16M

//===================================================

#ifdef __mips__
    #define MIPS_FLASH_IO_BASE_1    0xBFC00000
    #define MIPS_FLASH_IO_BASE_2    0xB4000000
#elif (defined(__arm__) || defined(__arm64__))
    #define MIPS_FLASH_IO_BASE_1    0x14000000
    #define MIPS_FLASH_IO_BASE_2    0x14000000
#endif

//===================================================
// Chunk header info:
#define CHUNK_HEADER_ADDR       0x20000 // sboot+pm
#define CHUNK_HEADER_LEN        0x80

#define CHUNK_HEADER_CRC32_POS  (0x14)
#define CRC32_POS	(0x14)

//========================================================================
// __section__ (".prog_img_info")
#ifdef __mips__
    #define AP_IMG_INFO_SECTION_OFFSET      0xF80
#elif (defined(__arm__) || defined(__arm64__))
    #define AP_IMG_INFO_SECTION_OFFSET      0x1100
#else // Aeon
    #define AP_IMG_INFO_SECTION_OFFSET      0x1100
#endif

    #define AP_IMG_INFO_MAGIC_ID_ADDR       (AP_IMG_INFO_SECTION_OFFSET+0)
    #define AP_IMG_INFO_COMPRESS_INFO_ADDR  (AP_IMG_INFO_SECTION_OFFSET+0x47)
    #define AP_IMG_INFO_COMPRESS_LEN_ADDR   (AP_IMG_INFO_SECTION_OFFSET+0x4A)

    #define AP_IMG_INFO_MAGIC_ID_2_ADDR     (AP_IMG_INFO_SECTION_OFFSET+0x4E)
    //#define AP_IMG_INFO_MAGIC_ID_2_ADDR     (AP_IMG_INFO_SECTION_OFFSET+0x50)

    #define AP_IMG_INFO_JOBA_DATA_PTR_ADDR  (AP_IMG_INFO_SECTION_OFFSET+0x52)

//---------------------------------------------------------------------------
// __section__ (".img_info")  at 0x2000
#define AP_BIN_TABLE_START_ADDR         (0x2000)

#define BIN_PACKAGE_HEADER_ID0          0x54454C09
#define BIN_PACKAGE_HEADER_ID1          0x58336900

#define BIN_TBL_OFFSET_ID_0             0
#define BIN_TBL_OFFSET_ID_1             4
#define BIN_TBL_OFFSET_BIN_COUNT        12
#define BIN_TBL_OFFSET_BIN_ITEM_START   14

#define BIN_TBL_BIN_ITEM_SIZE           11

//========================================================================

#define UNCOMPRESSED_LENGTH     (14336UL)


#if defined(__mips__)

    #define BOOT_LOADER_LENGTH      (0x30000) // Need refine...

    #define COMPRESSED_LENGTH_POS   (0xFCAUL)

    #if( ENABLE_MSTAR_MILAN    \
       ||ENABLE_MSTAR_MARLON     \
       ||ENABLE_MSTAR_M5321     \
       )
        #define DECOMPRESS_BUF_PA           (0x02000000UL) // 32M
        #define DECOMPRESS_BUF_CACHE        (0xA2000000UL) // 32M
        #define DECOMPRESS_BUF_CACHEABLE    (0x82000000UL) // 32M
        #define MEMORY_POOL_CACHEABLE       (0x83000000UL) // 40M
    #else
        #define DECOMPRESS_BUF_PA           (0x01100000UL) // 17M
        #define DECOMPRESS_BUF_CACHE        (0xA1100000UL) // 17M
        #define DECOMPRESS_BUF_CACHEABLE    (0x81100000UL) // 17M
        #define MEMORY_POOL_CACHEABLE       (0x82100000UL) // 31M
    #endif

    #define MEMORY_POOL_SIZE            (0x0A00000UL)

#elif (defined(__arm__) || defined(__arm64__))
    //#define BOOT_LOADER_LENGTH    (0x30000)
    #define COMPRESSED_LENGTH_POS    (0xFCAUL)

    //jc_db: need to fix cache problem
    // settings for MMU turned on
    #define DECOMPRESS_BUF_PA           (0x02000000UL) // 32M (Physical Address)
    #define DECOMPRESS_BUF_CACHEABLE    (0x22000000UL) // 32M (MIU0: Cached)
    #define DECOMPRESS_BUF_NONCACHE     (0x42000000UL) //32M (MIU0: Non-cached)

    //#define MEMORY_POOL_PA              (0x03000000UL) // 48M (Physical Address)
    #define MEMORY_POOL_CACHEABLE       (0x23000000UL) // 48M (MIU0: Cached)
    //#define MEMORY_POOL_NONCACHE        (0x43000000UL) // 48M (MIU0: Non-cached)

    #define MEMORY_POOL_SIZE            (0x00A00000UL)

#else // Aeon
    // Dram size : 32M ~ 128M

    //#define COMPRESSED_LENGTH_POS   (0x114AUL)

  #if ENABLE_MSTAR_WHISKY
    #define MMAP_LD2_MEM_ADDR           (0x00010100) // 64K
    #define MMAP_DECOMP_SRC_BUF_ADDR    (0x01800000) // 24M
  #else
    #define MMAP_LD2_MEM_ADDR           (0x01000100) // 16M
    #define MMAP_DECOMP_SRC_BUF_ADDR    (0x01100000) // 17M
  #endif

    #define MEMORY_POOL_SIZE            (0x0A00000UL)

#endif


//#define MEMORY_POOL_SIZE        (0x0A00000UL)

//===================================================
#ifdef __mips__

    #define NON_CACHEABLE_TO_CACHEABLE_MASK (0xDFFFFFFF)

#elif (defined(__arm__) || defined(__arm64__))

    #define NON_CACHEABLE_TO_CACHEABLE_MASK (0xFFFFFFFF)

#else

    #define NON_CACHEABLE_TO_CACHEABLE_MASK (0xFFFFFFFF)

#endif

//===============================================================

#define ALIGNED_VALUE(value, align)  ( ((value) + ((align)-1)) & ~((align)-1) )

//===============================================================

#endif /* __MSDECOMPRESS_NONOS_PRIV_H__ */

