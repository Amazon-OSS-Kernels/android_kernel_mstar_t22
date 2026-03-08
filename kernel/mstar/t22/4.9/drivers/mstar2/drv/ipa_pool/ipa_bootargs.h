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
/// @file   ipa_bootargs.h
/// @brief  parse ipa pool boot arguments
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _IPA_BOOTARGS_H
#define _IPA_BOOTARGS_H

#include <linux/device.h>

#include "ipa_priv.h"

#define IPA_NAME_LENG  32
#define IPA_CMA_DEFAULT_NUM 16
#define CMA_MEM_ALIGN       (pageblock_nr_pages*PAGE_SIZE)
/* ipa boot args base struct
  *
  * ipa_id:  [heap_type<<6 || heap_id]
  * start: miu offset
  */
#define IPA_BootArgs_Base int miu;  \
    int ipa_id; \
    unsigned long start;    \
    unsigned long size; \
    char name[IPA_NAME_LENG];

/* only for IPA CMA
  * ipa can access this by ipa cma driver
  *
  * now this is designed for vdec xc mfe dip ...
  * driver also use miu kernel protection
  */
struct IPA_CMA_BootArgs_Config {
    IPA_BootArgs_Base
};

/* only for pa address
  * ipa can access this by pa driver
  *
  * now this is designed for memory not in kernel(mmap.h)
  */
struct PA_BootArgs_Config {
    IPA_BootArgs_Base
};

struct IPA_STR_POOL_BootArgs_Config {     
    IPA_BootArgs_Base
};

/* ipa cma array */
extern struct IPA_CMA_BootArgs_Config ipa_cma_config[IPA_CMA_DEFAULT_NUM];
extern struct device ipa_cma_device[IPA_CMA_DEFAULT_NUM];
extern int mstar_driver_boot_ipa_cma_num;

/*pa para array*/
extern struct PA_BootArgs_Config pa_config[MAX_MIU_NUM];

extern struct device pa_device[MAX_MIU_NUM];
extern int mstar_driver_boot_pa_num;

extern struct IPA_STR_POOL_BootArgs_Config ipa_str_pool_config;
extern int mstar_driver_boot_ipa_str_pool_num ;

#endif

