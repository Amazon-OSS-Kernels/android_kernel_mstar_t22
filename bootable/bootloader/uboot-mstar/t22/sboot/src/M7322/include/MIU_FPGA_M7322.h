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

#ifndef _MIU_FPGA_M7322_H_
#define _MIU_FPGA_M7322_H_

#if (ENABLE_MSTAR_TITANIA_BD_FPGA)

const MS_REG_INIT MIU0_DDR_Init[] =
{
    _RV32_2(0x002bc6 ,0x3c11),
    _RV32_2(0x002bca ,0x403c),
    _RV32_2(0x002bc4 ,0x0004),
    _RV32_2(0x101202 ,0x0000),
    _RV32_2(0x101202 ,0x0818),
    _RV32_2(0x101204 ,0x0185),
    _RV32_2(0x101206 ,0x0120),
    _RV32_2(0x101208 ,0x0833),
    _RV32_2(0x10120a ,0x0c33),
    _RV32_2(0x10120c ,0x7111),
    _RV32_2(0x10120e ,0x100e),
    _RV32_2(0x101210 ,0x0031),
    _RV32_2(0x101228 ,0x4000),
    _RV32_2(0x10121e ,0x0C01),
    _RV32_2(0x10121e ,0x0C00),
    _RV32_2(0x101200 ,0x0008),
    _RV32_2(0x101200 ,0x000c),
    _RV32_2(0x101200 ,0x000e),
    _RV32_2(0x101200 ,0x001f),
    _RV32_2(0x10121e ,0x0C08),
    _RV32_2(0x1012f8 ,0x0000),
    _RV32_2(0x1012fa ,0x03FF),

    _END_OF_TBL32_,
    MIU_VER
};

#ifndef CONFIG_MIU1_DRAM_NONE
const MS_REG_INIT MIU1_DDR_Init[] =
{
    _END_OF_TBL32_,
};
#endif  //CONFIG_MIU1_DRAM_NONE

#endif  //ENABLE_MSTAR_TITANIA_BD_FPGA

#endif  //_MIU_FPGA_M7322_H_
