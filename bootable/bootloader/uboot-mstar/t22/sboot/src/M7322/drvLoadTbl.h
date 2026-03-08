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
//Below area is auto-updated by gen_Tbl.sh, try not to modify it!
#define OUTERSIZE                           0x24000

#define PAD_PRE_SECT_SIZE           0x400
#define PAD_SECT_SIZE                   0x1000
#define MIU0_PRE_SECT_SIZE         0x1000
#define MIU0_SECT_SIZE                 0x1000
#define MIU1_PRE_SECT_SIZE         0x1000
#define MIU1_SECT_SIZE                 0x1000
#define MIU0_STR_PRE_SECT_SIZE 0x1000
#define MIU1_STR_PRE_SECT_SIZE 0x1000
#define MIU0_STR_SECT_SIZE         0x1000
#define MIU1_STR_SECT_SIZE         0x1000
// End of auto-update section

#define PADPRE_TBL_SIZE         0x200
#define PAD_TBL_SIZE            0x800
#define MIU_PRE_TBL_SIZE        0x800
#define MIU_TBL_SIZE            0x800
#define MIU_STR_TBL_SIZE        0x800
#define MIU_PRE_STR_TBL_SIZE    0x800

#define ONEBIN_TBL_SIZE_512   0x200
#define ONEBIN_TBL_SIZE_1024  0x400
#define ONEBIN_TBL_SIZE_2048  0x800
#define ONEBIN_SIG_OFF_512    ONEBIN_TBL_SIZE_512 - 0x100
#define ONEBIN_SIG_OFF_1024    ONEBIN_TBL_SIZE_2048 - 0x100
#define ONEBIN_SIG_OFF_2048    ONEBIN_TBL_SIZE_2048 - 0x100

//TBD move to drvLoadTbl.S
#define TYPE_OFF r10
#define TBL_OFF  r11
#define TBL_SIZE r9

#define PADPRE_SEC_OFF 0x0
#define PAD_SECT_OFF   PAD_PRE_SECT_SIZE
#define MIU0_PRE_SECT_OFF  PAD_SECT_OFF+PAD_SECT_SIZE
#define MIU0_SECT_OFF  MIU0_PRE_SECT_OFF+MIU0_PRE_SECT_SIZE
#define MIU1_PRE_SECT_OFF MIU0_SECT_OFF+MIU0_SECT_SIZE
#define MIU1_SECT_OFF MIU1_PRE_SECT_OFF+MIU1_PRE_SECT_SIZE
#define MIU0_STR_PRE_SECT_OFF MIU1_SECT_OFF+MIU1_SECT_SIZE
#define MIU0_STR_SECT_OFF MIU0_STR_PRE_SECT_OFF+MIU0_STR_PRE_SECT_SIZE
#define MIU1_STR_PRE_SECT_OFF MIU0_STR_SECT_OFF+MIU0_STR_SECT_SIZE
#define MIU1_STR_SECT_OFF MIU1_STR_PRE_SECT_OFF+MIU1_STR_PRE_SECT_SIZE
//TBD move to drvLoadTbl.S

#define PADPRE       0x1
#define PAD          0x2
#define MIU0_Pre     0x3
#define MIU0         0x4
#define MIU1_Pre     0x5
#define MIU1         0x6
#define MIU0_STR_Pre 0x7
#define MIU0_STR     0x8
#define MIU1_STR_Pre 0x9
#define MIU1_STR     0xa