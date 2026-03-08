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
/*
 * $Id: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/include/platform.h#4 $
 * $Header: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/include/platform.h#4 $
 * $Date: 2015/06/10 $
 * $DateTime: 2015/06/10 16:00:37 $
 * $Change: 1251790 $
 * $File: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/include/platform.h $
 * $Revision: #4 $
 */

#ifndef PLATEFORM_H
#define PLATEFORM_H
//#include "std_c.h"
#include <MsTypes.h>

#define BOOTROM_VERSION_ASIC

/*==========================================================================
    //CA4 Physical Address Mapping
===========================================================================*/

#define IMI_BASE_ADDR           0x00000000
#define RIU_BASE_ADDR           0x1f000000
#ifdef BOOTROM_VERSION_ZEBU
  #define REG_ADDR_DEBUG            MIU0_START_ADDR
#else
  #define REG_ADDR_DEBUG            REG_ADDR_BASE_MAILBOX
#endif
#define FLAG_INIT_UART_BUSY         0x0BF1

/*==========================================================================
    Put the configuration base address of each IP here.
===========================================================================*/
#define BASE_REG_RIU_PA         0x1F000000

/* macro to get at MMIO space when running virtually */
#define IO_ADDRESS(x)           ( (MS_U32)(x) )

/* read register by word */
#define ms_readw(a) (*(volatile unsigned short *)IO_ADDRESS(a))
/* write register by word */
#define ms_writew(v,a) (*(volatile unsigned short *)IO_ADDRESS(a) = (v))
 /* read register by byte */
#define ms_readb(a) (*(volatile unsigned char *)IO_ADDRESS(a))
/* write register by byte */
#define ms_writeb(v,a) (*(volatile unsigned char *)IO_ADDRESS(a) = (v))


#define INREG8(x)           ms_readb(x)
#define OUTREG8(x, y)       ms_writeb((MS_U8)(y), x)
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))

#define INREG16(x)              ms_readw(x)
#define OUTREG16(x, y)          ms_writew((MS_U16)(y), x)
#define SETREG16(x, y)          OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)          OUTREG16(x, INREG16(x)&~(y))
#define INSREG16(addr, mask, val) OUTREG16(addr, ((INREG16(addr)&(~(mask))) | val))

#define GET_REG8_ADDR(x, y)  	      (x+(y)*2-((y)&1))
#define GET_REG16_ADDR(x, y)  	      (x+(y)*4)
#define GET_REG_OFFSET(x, y)  	      ((x)*0x200+(y)*0x4)
#define GET_BASE_ADDR_BY_BANK(x, y)  ((x) + ((y) << 1))


#define WRIU16(x, y)            OUTREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, x), y)
#define IRIU16(x, y, Z)         INSREG16(GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, x), y, Z)
#define PM_MISC_BASE_ADDR       GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, 0x2E00 )

#define REG_ADDR_CM4_MBX        GET_REG8_ADDR( BASE_REG_RIU_PA, 0x103380 )
#define REG_ADDR_CM4_CPUINT     GET_REG8_ADDR( BASE_REG_RIU_PA, 0x100540 )
#define REG_PM_UART_PAD         GET_REG8_ADDR( BASE_REG_RIU_PA, 0xE00EC )


#define REG_ADDR_BASE_CHIPTOP         GET_REG8_ADDR( RIU_BASE_ADDR, 0x1E00 )
#define REG_ADDR_BASE_CLKGEN          GET_REG8_ADDR( RIU_BASE_ADDR, 0x40200 )

#define REG_ADDR_BASE_PM_UART0        GET_REG8_ADDR( RIU_BASE_ADDR, 0xC00 )
#define REG_ADDR_BASE_PM_SLEEP        GET_REG8_ADDR( RIU_BASE_ADDR, 0xE00 )
#define REG_ADDR_BASE_WDT             GET_REG8_ADDR( RIU_BASE_ADDR, 0x3000 )
#define REG_ADDR_BASE_TIMER0          GET_REG8_ADDR( RIU_BASE_ADDR, 0x3020 )
#define REG_ADDR_BASE_TIMER1          GET_REG8_ADDR( RIU_BASE_ADDR, 0x3040 )
#define REG_ADDR_BASE_TIMER2          GET_REG8_ADDR( RIU_BASE_ADDR, 0x3060 )

//#define REG_ADDR_BASE_MIUPLL          GET_REG8_ADDR( RIU_BASE_ADDR, 0x103100 )
#define REG_ADDR_BASE_MAILBOX         GET_REG8_ADDR( RIU_BASE_ADDR, 0x103380 )
#define REG_ADDR_BASE_INTC            GET_REG8_ADDR( RIU_BASE_ADDR, 0x1019C0 )
#define REG_ADDR_BASE_CPUINT          GET_REG8_ADDR( RIU_BASE_ADDR, 0x100540 )
#define REG_ADDR_BASE_VBDMA           GET_REG8_ADDR( RIU_BASE_ADDR, 0x151A00 )

#if defined(BOOTROM_VERSION_FPGA)
    #define UART_BAUDRATE       38400
    #define UART_CLK            12000000
    #define TIMER_OSC           12000000
#elif defined(BOOTROM_VERSION_ASIC)
    #define UART_BAUDRATE       115200
    #define UART_CLK            24000000
    #define TIMER_OSC           12000000
#elif defined(BOOTROM_VERSION_ZEBU)
    #define UART_BAUDRATE       38400
    #define UART_CLK            12000000
    #define TIMER_OSC           12000000
#else
    #error "incorrect version!!"
#endif

#define BOOL MS_BOOL
/*
#ifndef BOOL
#define BOOL MS_BOOL
#endif
#ifndef BYTE
#define BYTE MS_U8
#endif
#ifndef WORD
#define WORD MS_U16
#endif
#ifndef DWORD
#define DWORD MS_U32
#endif
#ifndef U8
#define U8 MS_U8
#endif
#ifndef U16
#define U16 MS_U16
#endif
#ifndef U32
#define U32 MS_U32
#endif
#ifndef VIRT
#define VIRT MS_VIRT
#endif
#ifndef S32
#define S32 MS_S32
#endif
*/
#endif //PLATEFORM_H
