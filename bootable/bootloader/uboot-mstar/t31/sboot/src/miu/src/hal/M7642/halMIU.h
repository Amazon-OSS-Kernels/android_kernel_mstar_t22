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

#ifndef _HAL_MIU_H_
#define _HAL_MIU_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#ifndef U64
#define U64  unsigned long long
#endif
#ifndef U32
#define U32  unsigned long
#endif
#ifndef U16
#define U16  unsigned short
#endif
#ifndef U8
#define U8   unsigned char
#endif
#ifndef S32
#define S32  signed long
#endif
#ifndef S16
#define S16  signed short
#endif
#ifndef S8
#define S8   signed char
#endif

typedef U8   BOOLEAN;

#ifndef BOOL
#define BOOL BOOLEAN
#endif
#ifndef DISABLE
#define DISABLE     (0)
#endif
#ifndef ENABLE
#define ENABLE      (1)
#endif

#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000

// Array Number
#define MIU_PROTECT_ENABLE_NUMBER       (2)
#define MIU_PROTECT_ID_GP_Select_NUMBER (1)
#define MIU_PROTECT_ADDR_NUMBER         (4 * 8)
#define MIU_PROTECT_ID_ENABLE_NUMBER    (8)
#define MIU_PROTECT_ID_NUMBER           (16 / 2)
#define MIU_PROTECT_ID_GP1_NUMBER       (16 / 2)
#define MIU_PROTECT_DRAM_SIZE_NUMBER    (4)
#define MIU_Total_Array_NUMBER          (MIU_PROTECT_ENABLE_NUMBER + \
                                         MIU_PROTECT_ID_GP_Select_NUMBER + \
                                         MIU_PROTECT_ADDR_NUMBER + \
                                         MIU_PROTECT_ID_ENABLE_NUMBER + \
                                         MIU_PROTECT_ID_NUMBER + \
                                         MIU_PROTECT_ID_GP1_NUMBER + \
                                         MIU_PROTECT_DRAM_SIZE_NUMBER)

#define CONFIG_RIU_BASE_ADDRESS     (0x1F000000)
#define CONFIG_XIU_BASE_ADDRESS     (0x1F600000)

#define MIU_PM_PROTECT_BASE         (0x000E00UL)
#define MIU_PROTECT_BASE            (0x301A00UL)
#define MIU1_PROTECT_BASE           (0x301B00UL)

#define RIU     ((unsigned short volatile *) CONFIG_RIU_BASE_ADDRESS)
#define RIU8    ((unsigned char  volatile *) CONFIG_RIU_BASE_ADDRESS)
#define XIU     ((unsigned short volatile *) CONFIG_XIU_BASE_ADDRESS)
#define XIU8    ((unsigned char  volatile *) CONFIG_XIU_BASE_ADDRESS)

#define REG_MIU_PM_PROTECT_ARRAY_ADDR   (MIU_PM_PROTECT_BASE + 0x9C)

#define REG_MIU_PROTECT_EN              (MIU_PROTECT_BASE)
#define REG_MIU_PROTECT_DDR_BOUND       (MIU_PROTECT_BASE+0xC2)
#define REG_MIU_PROTECT0_ID0            (MIU_PROTECT_BASE+0xA0)
#define REG_MIU_PROTECT0_GROUP1_ID0     (MIU_PROTECT_BASE+0xB0)
#define REG_MIU_PROTECT_GROUP_SEL       (MIU_PROTECT_BASE+0x0C)
#define REG_MIU_PROTECT0_START          (MIU_PROTECT_BASE+0x20)
#define REG_MIU_PROTECT0_ID_ENABLE      (MIU_PROTECT_BASE+0x80)
#define REG_MIU_PROTECT_HIE_LOG         (MIU_PROTECT_BASE+0xFE)

#define REG_MIU1_PROTECT_EN             (MIU1_PROTECT_BASE)
#define REG_MIU1_PROTECT_DDR_BOUND      (MIU1_PROTECT_BASE+0xC2)
#define REG_MIU1_PROTECT0_ID0           (MIU1_PROTECT_BASE+0xA0)
#define REG_MIU1_PROTECT0_GROUP1_ID0    (MIU1_PROTECT_BASE+0xB0)
#define REG_MIU1_PROTECT_GROUP_SEL      (MIU1_PROTECT_BASE+0x0C)
#define REG_MIU1_PROTECT0_START         (MIU1_PROTECT_BASE+0x20)
#define REG_MIU1_PROTECT0_ID_ENABLE     (MIU1_PROTECT_BASE+0x80)
#define REG_MIU1_PROTECT_HIE_LOG        (MIU1_PROTECT_BASE+0xFE)

#define REG_MIU_PROTECT_LOG_CLR         (BIT0)
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void HAL_MIU_RestoreProtect(void);
#endif
