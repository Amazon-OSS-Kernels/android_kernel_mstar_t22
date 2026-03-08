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
#include "cache.h"

#define Hit_Invalidate_D    0x11
#define Hit_Writeback_Inv_D 0x15
#define CONFIG_SYS_CACHELINE_SIZE   16
#define Hit_Invalidate_I 0x10
#define _BIT(x)                      (1<<(x))
#define cache_op(op,addr)                       \
	__asm__ __volatile__(                       \
	"   .set    push                    \n" \
	"   .set    noreorder               \n" \
	"   .set    mips3\n\t               \n" \
	"   cache   %0, %1                  \n" \
	"   .set    pop                 \n" \
	:                               \
	: "i" (op), "R" (*(unsigned char *)(addr)))

void flush_cache(unsigned long start_addr, unsigned long size)
{
#if 1
	unsigned long lsize = 16;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (start_addr + size - 1) & ~(lsize - 1);
	unsigned int    dwReadData = 0;	
	while (1) {		
		cache_op(Hit_Writeback_Inv_D, addr);
		cache_op(Hit_Invalidate_I, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}        
	//Flush L2 All and Invalid
	dwReadData = *(volatile unsigned int *)(0xbf203104);
	dwReadData &= ~(0x1F);
	dwReadData |= 0x04;
	//Flush All and Invalid
	*(volatile unsigned int *)(0xbf203104) = dwReadData;
	dwReadData = *(volatile unsigned int *)(0xbf203104);
	dwReadData |= _BIT(4);
	*(volatile unsigned int *)(0xbf203104) = dwReadData;
	do
	{
		dwReadData = *(volatile unsigned int *)(0xbf203104);
		dwReadData &= _BIT(10);
		//Check Status of Flush All Finish
	} while(dwReadData == 0);
	//Flush MIU pipe
	dwReadData = *(volatile unsigned int *)(0xbf203104);
	dwReadData &= ~(0x1F);
	dwReadData |= 0x02;
	//Flush MIU Pipe
	*(volatile unsigned int *)(0xbf203104) = dwReadData;
	dwReadData = *(volatile unsigned int *)(0xbf203104);
	dwReadData |= _BIT(4);
	*(volatile unsigned int *)(0xbf203104) = dwReadData;
	do
	{
		dwReadData = *(volatile unsigned int *)(0xbf203104);
		dwReadData &= _BIT(9);
		//Check Status of Flush MIU Pipe Finish    
	} while(dwReadData == 0);
#endif    
}
