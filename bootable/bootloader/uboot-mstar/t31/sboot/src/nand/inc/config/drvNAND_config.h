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
#ifndef __UNFD_CONFIG_H__
#define __UNFD_CONFIG_H__

//=====================================================
// select a HW platform:
//   - 1: enable, 0: disable.
//   - only one platform can be 1, others have to be 0.
//=====================================================
#define NAND_DRV_TV_SBOOT           1


//=====================================================
// do NOT edit the following content.
//=====================================================
#if defined(CONFIG_MSTAR_EMERALD) && CONFIG_MSTAR_EMERALD
    #include "drvNAND_emerald_sboot.h"
#elif defined(CONFIG_MSTAR_NUGGET) && CONFIG_MSTAR_NUGGET
    #include "drvNAND_nugget_sboot.h"
#elif defined(CONFIG_MSTAR_NIKON) && CONFIG_MSTAR_NIKON
    #include "drvNAND_nikon_sboot.h"
#elif defined(CONFIG_MSTAR_EAGLE) && CONFIG_MSTAR_EAGLE
	#include "drvNAND_eagle_sboot.h"
#elif defined(CONFIG_MSTAR_EIFFEL) && CONFIG_MSTAR_EIFFEL
	#include "drvNAND_eiffel_sboot.h"
#elif defined(CONFIG_MSTAR_NIKE) && CONFIG_MSTAR_NIKE
	#include "drvNAND_nike_sboot.h"
#elif defined(CONFIG_MSTAR_MADISON) && CONFIG_MSTAR_MADISON
	#include "drvNAND_madison_sboot.h"
#elif defined(CONFIG_MSTAR_CLIPPERS) && CONFIG_MSTAR_CLIPPERS
	#include "drvNAND_clippers_sboot.h"
#elif defined(CONFIG_MSTAR_MIAMI) && CONFIG_MSTAR_MIAMI
	#include "drvNAND_miami_sboot.h"
#elif defined(CONFIG_MSTAR_MONACO) && CONFIG_MSTAR_MONACO
	#include "drvNAND_monaco_sboot.h"
#elif defined(CONFIG_MSTAR_MUJI) && CONFIG_MSTAR_MUJI
	#include "drvNAND_muji_sboot.h"
#elif defined(CONFIG_MSTAR_CELTICS) && CONFIG_MSTAR_CELTICS
	#include "drvNAND_celtics_sboot.h"
#elif defined(CONFIG_MSTAR_KENYA) && CONFIG_MSTAR_KENYA
    #include "drvNAND_kenya_sboot.h"
#elif defined(CONFIG_MSTAR_NADAL) && CONFIG_MSTAR_NADAL
	#include "drvNAND_nadal_sboot.h"
#elif defined(CONFIG_MSTAR_NAPOLI) && CONFIG_MSTAR_NAPOLI
	#include "drvNAND_napoli_sboot.h"
#elif defined(CONFIG_MSTAR_KRITI) && CONFIG_MSTAR_KRITI
    #include "drvNAND_kriti_sboot.h"
#elif defined(CONFIG_MSTAR_KRATOS) && CONFIG_MSTAR_KRATOS
    #include "drvNAND_kratos_sboot.h"
#elif defined(CONFIG_MSTAR_MONET) && CONFIG_MSTAR_MONET
	#include "drvNAND_monet_sboot.h"
#elif defined(CONFIG_MSTAR_MOONEY) && CONFIG_MSTAR_MOONEY
	#include "drvNAND_mooney_sboot.h"
#elif defined(CONFIG_MSTAR_MESSI) && CONFIG_MSTAR_MESSI
	#include "drvNAND_messi_sboot.h"
#elif defined(CONFIG_MSTAR_MILAN) && CONFIG_MSTAR_MILAN
        #include "drvNAND_milan_sboot.h"
#elif defined(CONFIG_MSTAR_MARLON) && CONFIG_MSTAR_MARLON
        #include "drvNAND_marlon_sboot.h"
#elif defined(CONFIG_MSTAR_MUNICH) && CONFIG_MSTAR_MUNICH
        #include "drvNAND_munich_sboot.h"
#elif defined(CONFIG_MSTAR_M5321) && CONFIG_MSTAR_M5321
        #include "drvNAND_m5321_sboot.h"
#elif defined(CONFIG_MSTAR_MANHATTAN) && CONFIG_MSTAR_MANHATTAN
        #include "drvNAND_manhattan_sboot.h"
#elif defined(CONFIG_MSTAR_MASERATI) && CONFIG_MSTAR_MASERATI
        #include "drvNAND_maserati_sboot.h"
#elif defined(CONFIG_MSTAR_MACAN) && CONFIG_MSTAR_MACAN
        #include "drvNAND_macan_sboot.h"
#elif defined(CONFIG_MSTAR_MAZDA) && CONFIG_MSTAR_MAZDA
        #include "drvNAND_mazda_sboot.h"
#elif defined(CONFIG_MSTAR_MUSTANG) && CONFIG_MSTAR_MUSTANG
        #include "drvNAND_mustang_sboot.h"
#elif defined(CONFIG_MSTAR_MAXIM) && CONFIG_MSTAR_MAXIM
        #include "drvNAND_maxim_sboot.h"
#elif defined(CONFIG_MSTAR_M7621) && CONFIG_MSTAR_M7621
        #include "drvNAND_m7621_sboot.h"
#elif defined(CONFIG_MSTAR_M5621) && CONFIG_MSTAR_M5621
        #include "drvNAND_m5621_sboot.h"
#elif defined(CONFIG_MSTAR_MATISSE) && CONFIG_MSTAR_MATISSE
    #include "drvNAND_matisse_sboot.h"
#elif defined(CONFIG_MSTAR_MAINZ) && CONFIG_MSTAR_MAINZ
    #include "drvNAND_mainz_sboot.h"
#elif defined(CONFIG_MSTAR_M7221) && CONFIG_MSTAR_M7221
        #include "drvNAND_m7221_sboot.h"
#elif defined(CONFIG_MSTAR_M7622) && CONFIG_MSTAR_M7622
        #include "drvNAND_m7622_sboot.h"
#elif defined(CONFIG_MSTAR_M3822) && CONFIG_MSTAR_M3822
        #include "drvNAND_m3822_sboot.h"
#elif defined(CONFIG_MSTAR_M7322) && CONFIG_MSTAR_M7322
        #include "drvNAND_m7322_sboot.h"
#elif defined(CONFIG_MSTAR_M7632) && CONFIG_MSTAR_M7632
        #include "drvNAND_m7632_sboot.h"
#elif defined(CONFIG_MSTAR_M7332) && CONFIG_MSTAR_M7332
        #include "drvNAND_m7332_sboot.h"
#elif defined(CONFIG_MSTAR_M7642) && CONFIG_MSTAR_M7642
        #include "drvNAND_m7642_sboot.h"
#else
    #error "Error! no platform selected."
#endif


//do NOT edit the following content, for HAL use.
//=====================================================
// misc. do NOT edit the following content.
//=====================================================
#define NAND_DMA_RACING_PATCH       1
#define NAND_DMA_PATCH_WAIT_TIME    10000 // us -> 10ms
#define NAND_DMA_RACING_PATTERN0    (((U32)'M'<<24)|((U32)0<<16)|((U32)'S'<<8)|(U32)1)
#define NAND_DMA_RACING_PATTERN1    (((U32)'T'<<24)|((U32)6<<16)|((U32)'A'<<8)|(U32)8)


//===========================================================
// Time Dalay, do NOT edit the following content, for NC_WaitComplete use.
//===========================================================
#define DELAY_100us_in_us           HW_TIMER_DELAY_100us
#define DELAY_300us_in_us           HW_TIMER_DELAY_100us * 3
#define DELAY_500us_in_us           HW_TIMER_DELAY_100us * 5
#define DELAY_1ms_in_us             HW_TIMER_DELAY_1ms
#define DELAY_10ms_in_us            HW_TIMER_DELAY_10ms
#define DELAY_100ms_in_us           HW_TIMER_DELAY_100ms
#define DELAY_500ms_in_us           HW_TIMER_DELAY_500ms
#define DELAY_1s_in_us              HW_TIMER_DELAY_1s

#define WAIT_ERASE_TIME             (DELAY_1s_in_us)
#define WAIT_WRITE_TIME             (DELAY_1s_in_us)
#define WAIT_READ_TIME              (DELAY_1ms_in_us)
#define WAIT_PROBE_TIME             (DELAY_300us_in_us)
#define WAIT_RESET_TIME             (DELAY_10ms_in_us)

extern void nand_reset_WatchDog(void);
extern U32 nand_clock_setting(U32 u32ClkParam);
extern U32 nand_config_clock(U16 u16_SeqAccessTime);
extern U32 nand_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt);
extern U32 nand_flush_cache(U32 u32_DMAAddr, U32 u32_ByteCnt);
extern void nand_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size);
extern U32 nand_WaitCompleteIntr(U16 u16_WaitEvent, U32 u32_MicroSec);
extern void nand_enable_intr_mode(void);
extern void nand_flush_miu_pipe(void);

extern void drvNAND_CHECK_FLASH_TYPE(void);

#define NAND_PARTITAION_BYTE_CNT    0x200

#endif /* __UNFD_CONFIG_H__ */
