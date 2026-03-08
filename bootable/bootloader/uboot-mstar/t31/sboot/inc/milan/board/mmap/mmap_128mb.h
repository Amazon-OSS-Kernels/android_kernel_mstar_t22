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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mmap_128mb.h
/// @brief  Memory mapping for 128MB RAM
///////////////////////////////////////////////////////////////////////////////////////////////////
//#include <linux/undefconf.h>
//#include "../../../../../include/linux/undefconf.h"
#ifndef _MS_MMAP_128MB_H_
#define _MS_MMAP_128MB_H_

// Memory alignment
#define MemAlignUnit                64UL
#define MemAlign(n, unit)           ((((n)+(unit)-1)/(unit))*(unit))

#define MIU_DRAM_LEN				(0x8000000)

#define MIU0_LEN					(0x8000000)
#define MIU1_LEN					0

//----------------------------------------------------------------------
// MIU 0
//----------------------------------------------------------------------

#define COP_CODE_START				(0x00000000)
#define COP_CODE_LEN				(0x400000)   //for EBASE_OFFSET

// Linux kernel space
#define LINUX_MEM_AVAILABLE			(COP_CODE_START+COP_CODE_LEN)
#define LINUX_MEM_BASE_ADR 			(LINUX_MEM_AVAILABLE)
#define LINUX_MEM_GAP_CHK  			(LINUX_MEM_BASE_ADR-LINUX_MEM_AVAILABLE)
#define LINUX_MEM_LEN				(0x8C0000) //9MB-256K

//dhjung LGE
#if 1
// for MVD and TSP binary dhjung LGE
#define BIN_MEM_AVAILABLE      		(LINUX_MEM_BASE_ADR + LINUX_MEM_LEN)
#define BIN_MEM_ADR                 MemAlign(BIN_MEM_AVAILABLE, 4096)
#define BIN_MEM_GAP_CHK        		(BIN_MEM_ADR-BIN_MEM_AVAILABLE)
#define BIN_MEM_LEN    				0x40000

// must start at nnn0000 - 512k alignment   --[Dean] be careful allocate this big alignment area
#define MAD_BASE_AVAILABLE         	(BIN_MEM_ADR+BIN_MEM_LEN)
#define MAD_BASE_BUFFER_ADR        	MemAlign(MAD_BASE_AVAILABLE, 0x80000)
#define MAD_BASE_BUFFER_ADR_GAP_CHK	(MAD_BASE_BUFFER_ADR-MAD_BASE_AVAILABLE)
#define MAD_BASE_BUFFER_LEN        	0x300000	//3M

#else

#define LINUX_MEM_LEN				(0xC00000)	// 12MB

// must start at nnn0000 - 512k alignment   --[Dean] be careful allocate this big alignment area
#define MAD_BASE_AVAILABLE         	(LINUX_MEM_BASE_ADR+LINUX_MEM_LEN)
#define MAD_BASE_BUFFER_ADR        	MemAlign(MAD_BASE_AVAILABLE, 0x80000)
#define MAD_BASE_BUFFER_ADR_GAP_CHK	(MAD_BASE_BUFFER_ADR-MAD_BASE_AVAILABLE)
#define MAD_BASE_BUFFER_LEN        	0x300000	//3M

#endif

//======================================================================
// Can not add any buffer here (in between VE_FRAMEBUFFER_ADR and SCALER_DNR_BUF_ADR)
// Because USB/OAD download use the buffer from VE_FRAMEBUFFER_ADR to the end of SCALER_DNR_BUF_ADR
//======================================================================

// For Maximum is 1920x1088x3x2 about 12MB in HDMI and YPbPr
// Let it overwrite EVENTDB and MHEG5 buffer(These buffers is DTV only)
#define SCALER_DNR_AVAILABLE  		(MAD_BASE_AVAILABLE + MAD_BASE_BUFFER_LEN)
#define SCALER_DNR_BUF_ADR	  		MemAlign(SCALER_DNR_AVAILABLE, 8)
#define SCALER_DNR_GAP_CHK	  		(SCALER_DNR_BUF_ADR-SCALER_DNR_AVAILABLE)
#define SCALER_DNR_BUF_LEN	  		0x1000000-80 //16MB-80byte  //0xC00000  //12582912	// 12MB

#define SCALER_DNR_W_AVAILABLE		(SCALER_DNR_BUF_ADR + SCALER_DNR_BUF_LEN)
#define SCALER_DNR_W_BARRIER_ADR	(SCALER_DNR_W_AVAILABLE)
#define SCALER_DNR_W_GAP_CHK		(SCALER_DNR_W_BARRIER_ADR-SCALER_DNR_W_AVAILABLE)
#define SCALER_DNR_W_BARRIER_LEN	80//16		// DNR submit 2 64-bit data before compare limit
#define SCALER_DNR_BUF_LEN_EXT		((((896UL-736UL)*3+0x0F) & ~0x0F) * 581UL *2) // the output size of VD will be 848 * 581

// no use in ATSC
#define RLD_BUF_AVAILABLE			(SCALER_DNR_W_BARRIER_ADR+SCALER_DNR_W_BARRIER_LEN)
#define RLD_BUF_ADR					MemAlign(RLD_BUF_AVAILABLE, 8)
#define RLD_BUF_GAP_CHK				(RLD_BUF_ADR-RLD_BUF_AVAILABLE)
#define RLD_BUF_LEN					0xDD000//905216

// VD_3DCOMB
#if 	(defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1)&&CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	(defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)&&CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
// RLD is only used in EMP Movie, no need in ATSC
#define VD_3DCOMB_AVAILABLE			(SCALER_DNR_W_BARRIER_ADR+SCALER_DNR_W_BARRIER_LEN)
#else
#define VD_3DCOMB_AVAILABLE			(RLD_BUF_ADR + RLD_BUF_LEN)
#endif
#define VD_3DCOMB_BASE_ADR 			MemAlign(VD_3DCOMB_AVAILABLE, 8)
#define VD_3DCOMB_GAP_CHK  			(VD_3DCOMB_BASE_ADR-VD_3DCOMB_AVAILABLE)
#define VD_3DCOMB_LEN      			(0x400000)	//4MB

//#define VE_FRAMEBUFFER_AVAILABLE	(MVD_BITSTREAM_ADR + MVD_BITSTREAM_LEN)
#define VE_FRAMEBUFFER_AVAILABLE  	(VD_3DCOMB_BASE_ADR+VD_3DCOMB_LEN)
#define VE_FRAMEBUFFER_ADR		  	MemAlign(VE_FRAMEBUFFER_AVAILABLE, 8)
#define VE_FRAMEBUFFER_ADR_GAP_CHK	(VE_FRAMEBUFFER_ADR-VE_FRAMEBUFFER_AVAILABLE)
#define VE_FRAMEBUFFER_LEN			0x195000UL	// 720*576*2*2

#define TTX_BUF_AVAILABLE			(VE_FRAMEBUFFER_ADR + VE_FRAMEBUFFER_LEN)
#define TTX_BUF_ADR					MemAlign(TTX_BUF_AVAILABLE, 4096)
#define TTX_BUF_GAP_CHK				(TTX_BUF_ADR-TTX_BUF_AVAILABLE)
#define TTX_BUF_LEN					0x100000

// no use in ATSC
#define MVD_SW_AVAILABLE     		(RLD_BUF_ADR + RLD_BUF_LEN)
#define MVD_SW_ADR           		MemAlign(MVD_SW_AVAILABLE, 8)
#define MVD_SW_ADR_GAP_CHK   		(MVD_SW_ADR-MVD_SW_AVAILABLE)
#define MVD_SW_LEN           		0xB00000//0x900000//0xB00000	//11MB

#if 	(defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1)&&CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	(defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)&&CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
// in ATSC, just skip MVD_SW (no need to play movie)
#define MPOOL_AVAILABLE				(TTX_BUF_ADR + TTX_BUF_LEN)
#else
#define MPOOL_AVAILABLE				(MVD_SW_ADR + MVD_SW_LEN)
#endif
#define MPOOL_ADR					MemAlign(MPOOL_AVAILABLE, 4096)
#define MPOOL_GAP_CHK				(MPOOL_ADR-MPOOL_AVAILABLE)
#if 	(defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1)&&CONFIG_MSTAR_TITANIA_BD_T2_LG_MECURY_BOARD_ATSC_1) 	\
	|| 	(defined (CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)&&CONFIG_MSTAR_TITANIA_BD_T2_LG_MINERAVA_BOARD_ATSC_1)
#define MPOOL_LEN					(0x4000000 - MPOOL_ADR - 0xB00000-0x400000 /*released from POSD0*/)
#else
#define MPOOL_LEN					(0x1800000UL)//(0x8000000 - MPOOL_ADR - 0x3A00000-0x400000 /*released from POSD0*/)
#endif

#define EMAC_MEM_LEN               	0x100000
#define EMAC_MEM_ADR                (MPOOL_ADR+MPOOL_LEN)//0x00200000UL

#define LINUX_MEM2_AVAILABLE		(EMAC_MEM_ADR+EMAC_MEM_LEN)
#define LINUX_MEM2_BASE_ADR			MemAlign(LINUX_MEM2_AVAILABLE,8) //aligned to 8 byte may not be necessary
#define LINUX_MEM2_GAP_CHK			(LINUX_MEM2_BASE_ADR-LINUX_MEM2_AVAILABLE)
#define LINUX_MEM2_LEN			    (MIU0_LEN-LINUX_MEM2_BASE_ADR)

#define MIU0_END_ADR                (LINUX_MEM2_BASE_ADR+LINUX_MEM2_LEN)//(MPOOL_ADR+MPOOL_LEN)

//#define MIU0_END_ADR                (MPOOL_ADR+MPOOL_LEN)

#endif
