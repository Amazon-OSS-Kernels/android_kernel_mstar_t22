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

#ifndef __HAL_SWI2C_H__
#define __HAL_SWI2C_H__

// this ENABLE_HAL_SWI2C_BUS_CFG needs to be set to 1 to enable HAL i2c setting
#define ENABLE_HAL_SWI2C_BUS_CFG 1

// Bus-0
#define PAD_SCL_0 PAD_DDCR_CK
#define PAD_SDA_0 PAD_DDCR_DA
#define DELAY_VAL_0 100

// Bus-1
#define PAD_SCL_1 PAD_TGPIO0
#define PAD_SDA_1 PAD_TGPIO1
#define DELAY_VAL_1 100

// Bus-2
#define PAD_SCL_2 PAD_TGPIO2
#define PAD_SDA_2 PAD_TGPIO3
#define DELAY_VAL_2 100

// Bus-3
#define PAD_SCL_3 PAD_TGPIO2
#define PAD_SDA_3 PAD_TGPIO3
#define DELAY_VAL_3 100

// Bus-4
#define PAD_SCL_4 PAD_TGPIO2
#define PAD_SDA_4 PAD_TGPIO3
#define DELAY_VAL_4 100

// Bus-5
#define PAD_SCL_5 PAD_TGPIO2
#define PAD_SDA_5 PAD_TGPIO3
#define DELAY_VAL_5 100

// Bus-6
#define PAD_SCL_6 PAD_TGPIO2
#define PAD_SDA_6 PAD_TGPIO3
#define DELAY_VAL_6 100

// Bus-7
#define PAD_SCL_7 PAD_TGPIO2
#define PAD_SDA_7 PAD_TGPIO3
#define DELAY_VAL_7 100

#endif //__HAL_SWI2C_H__