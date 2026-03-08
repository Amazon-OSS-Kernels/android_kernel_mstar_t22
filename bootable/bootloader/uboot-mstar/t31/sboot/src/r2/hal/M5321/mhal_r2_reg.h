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
#ifndef _REG_R2_H_
#define _REG_R2_H_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Base
//-------------------------------------------------------------------------------------------------
#define BASE_PM                     (0xBF000000)
#define BASE_NON_PM                 (0xBF200000)

#define BASE_PM_OFFSET              (0x000000UL)
#define BASE_NON_PM_OFFSET          (0x001000UL)

//-------------------------------------------------------------------------------------------------
//  Bank
//-------------------------------------------------------------------------------------------------
#define BANK_PM_MISC                (0x002EUL - BASE_PM_OFFSET)

#define BANK_CLKGEN0                (0x100BUL - BASE_NON_PM_OFFSET)
#define BANK_SEC_R2                 (0x122AUL - BASE_NON_PM_OFFSET)
#define BANK_MAU_LV1                (0x122BUL - BASE_NON_PM_OFFSET)
#define BANK_MAU_LV2_0              (0x1639UL - BASE_NON_PM_OFFSET)
#define BANK_MAU_LV2_1              (0x163AUL - BASE_NON_PM_OFFSET)

//-------------------------------------------------------------------------------------------------
#define REG_PM_CPUX_SW_RSTZ         ((BANK_PM_MISC      << 8) + (0x0029UL << 1) + 0)

#define REG_NPM_CLKGEN0_SEC_R2      ((BANK_CLKGEN0      << 8) + (0x004AUL << 1) + 0)

#define REG_SEC_R2_STOP             ((BANK_SEC_R2       << 8) + (0x0040UL << 1) + 0)
#define REG_SEC_R2_RIU_BASE         ((BANK_SEC_R2       << 8) + (0x0045UL << 1) + 0)
#define REG_SEC_R2_SPI_BASE         ((BANK_SEC_R2       << 8) + (0x0048UL << 1) + 0)
#define REG_SEC_R2_DQMEM_BASE       ((BANK_SEC_R2       << 8) + (0x004EUL << 1) + 0)
#define REG_SEC_R2_QMEM_MASK_HIGH   ((BANK_SEC_R2       << 8) + (0x0050UL << 1) + 0)
#define REG_SEC_R2_IO1_BASE         ((BANK_SEC_R2       << 8) + (0x0055UL << 1) + 0)
#define REG_SEC_R2_SPI_BASE1        ((BANK_SEC_R2       << 8) + (0x0056UL << 1) + 0)
#define REG_SEC_R2_SPACE_EN         ((BANK_SEC_R2       << 8) + (0x0058UL << 1) + 0)
#define REG_SEC_R2_RST_BASE         ((BANK_SEC_R2       << 8) + (0x005AUL << 1) + 0)

#define REG_MAU_LV1                 ((BANK_MAU_LV1      << 8) + (0x0001UL << 1) + 0)
#define REG_MAU_LV2_0               ((BANK_MAU_LV2_0    << 8) + (0x0001UL << 1) + 0)
#endif
