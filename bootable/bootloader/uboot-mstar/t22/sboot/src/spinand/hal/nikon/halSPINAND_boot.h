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

#ifndef _HAL_SPINAND_BOOT_H_
#define _HAL_SPINAND_BOOT_H_

#include "halSPINAND.h"

#if defined(CONFIG_MCU_ARM)
#define CONFIG_SRAM_SPINAND_OFFSET  0x1FC02800 //512B Reserved for SPINAND Flash Driver
#define CONFIG_SRAM_SPINAND_BUF     (0x1FC02800+0x100)
#define RIU_BASE                    0x1F200000
#elif defined(CONFIG_MCU_MIPS32)
#define CONFIG_SRAM_SPINAND_OFFSET  0xBF403800 //512B Reserved for SPINAND Flash Driver
#define CONFIG_SRAM_SPINAND_BUF     (0xBF403800+0x100)
#define RIU_BASE                    0xBF200000
#else
#error  "Unknown CPU Platform"
#endif

#define REG_OFFSET_SHIFT_BITS       2
#define GET_REG_ADDR(x, y)          ((x)+((y) << REG_OFFSET_SHIFT_BITS))

#define REG_BANK_PORTSTATUS         0x0280U
#define PORTSTATUS_BASE             GET_REG_ADDR(RIU_BASE, REG_BANK_PORTSTATUS)
#define BOOTSTAGE_REG               GET_REG_ADDR(PORTSTATUS_BASE, 0x7)

#define DEBUG_REG                   GET_REG_ADDR(PORTSTATUS_BASE, 0x01)
#define DEBUG_REG_SYS_L1            GET_REG_ADDR(PORTSTATUS_BASE, 0x02)
#define DEBUG_REG_SYS_L2            GET_REG_ADDR(PORTSTATUS_BASE, 0x03)

#define REG_READ_UINT16(reg_addr, val)      val = REG((reg_addr))
#define REG_WRITE_UINT16(reg_addr, val)     REG((reg_addr)) = (val)

#endif

