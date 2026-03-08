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

#ifndef _BOARD_H_
#define _BOARD_H_

#ifndef _ENUM_DEVICE_H_
#include "Enum_Device.h"
#endif

#ifndef __MEMORY_MAP_SEL_H__
#include "mmap/MMap.h"
#endif

//------------------------------------------------------------------------------

#define ENABLE                          1
#define DISABLE                         0
#define OBA2                            0
#define USE_SW_I2C                      1
#define ENABLE_UART1_DEBUG              0

//------------------------------UNUSED MACROS-----------------------------------
#define _FUNC_NOT_USED()                do {} while ( 0 )

//------------------------------MS_BOARD_TYPE_SEL-------------------------------
// Demo board made in Taiwan

#define BD_FPGA_M5621                   0xC000
#define BD_MST285A_D01A_S_M5621         0xC001
#define BD_MST285B_D01A_S_M5621         0xC002
#define BD_MST135B_10AMVL_M5621         0xC003
#define BD_MST136B_10ABQHATL_M5621      0xC004
#define BD_MST151B_10ABQHAT_M5621       0xC005
#define BD_MST135B_10AMVL_M5621_SKODA       0xC006
#define BD_MST135B_10AMVL_M5621_TOPAZ   0xC007
#define BD_MST135B_10AMVL_M5621_CHIEF   0xC008
#define BD_MST135B_10AMVL_M5621_KONKIA       0xC009
#define BD_MST151B_10ABQHAT_M5621_HISENSE 0xC00A
#define BD_MST151B_10ABQHAT_M5621_SW 0xC00B
#define BD_MST135B_10AMVL_M5621_KNIGHT    0xC00C
#define BD_MST151B_10ABQHAT_M5621_HAIER   0xC00D
#define BD_MST151B_10ABQHAT_M5621_MTC     0xC00E
#define BD_MST151B_10ABQHAT_M5621_CHANGHONG 0xC00F
#define BD_MST151B_10ABQHAT_M5621_HKC     0xC010
#define BD_MST151B_10ABQHAT_M5621_SHARP   0xC011
#define BD_UNKNOWN                      0xFFFF

#ifndef MS_BOARD_TYPE_SEL
#if defined (CONFIG_MSTAR_TITANIA_BD_FPGA)
#define MS_BOARD_TYPE_SEL               BD_FPGA_M5621
#include "BD_FPGA_M5621.h"
#elif defined (CONFIG_MSTAR_BD_MST285A_D01A_S_M5621)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL       	BD_MST285A_D01A_S_M5621
    #include "BD_MST285A_D01A_S_M5621.h"

#elif defined (CONFIG_MSTAR_BD_MST285B_D01A_S_M5621)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL       	BD_MST285B_D01A_S_M5621
    #include "BD_MST285B_D01A_S_M5621.h"

#elif defined (CONFIG_MSTAR_BD_MST285C_D01A_S_M5621)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL       	BD_MST285C_D01A_S_M5621
    #include "BD_MST285C_D01A_S_M5621.h"

#elif defined (CONFIG_MSTAR_BD_MST135B_10AMVL_M5621)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL       	BD_MST135B_10AMVL_M5621
    #include "BD_MST135B_10AMVL_M5621.h"

#elif defined (CONFIG_MSTAR_BD_MST136B_10ABQHATL_M5621)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL       	BD_MST136B_10ABQHATL_M5621
    #include "BD_MST136B_10ABQHATL_M5621.h"

#elif defined (CONFIG_MSTAR_BD_MST151B_10ABQHAT_M5621)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST151B_10ABQHAT_M5621
    #include "BD_MST151B_10ABQHAT_M5621.h"
#elif defined (CONFIG_MSTAR_BD_MST151B_10ABQHAT_M5621_HISENSE)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST151B_10ABQHAT_M5621_HISENSE
    #include "BD_MST151B_10ABQHAT_M5621_HISENSE.h"
#elif defined (CONFIG_MSTAR_BD_MST151B_10ABQHAT_M5621_SW)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST151B_10ABQHAT_M5621_SW
    #include "BD_MST151B_10ABQHAT_M5621_SW.h"
#elif defined (CONFIG_MSTAR_BD_MST151B_10ABQHAT_M5621_CHANGHONG)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST151B_10ABQHAT_M5621_CHANGHONG
    #include "BD_MST151B_10ABQHAT_M5621_CHANGHONG.h"
#elif defined (CONFIG_MSTAR_BD_MST135B_10AMVL_M5621_SKODA)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST135B_10AMVL_M5621_SKODA
    #include "BD_MST135B_10AMVL_M5621_SKODA.h"
#elif defined (CONFIG_MSTAR_BD_MST135B_10AMVL_M5621_TOPAZ)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST135B_10AMVL_M5621_TOPAZ
    #include "BD_MST135B_10AMVL_M5621_TOPAZ.h"
#elif defined (CONFIG_MSTAR_BD_MST135B_10AMVL_M5621_CHIEF)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST135B_10AMVL_M5621_CHIEF
    #include "BD_MST135B_10AMVL_M5621_CHIEF.h"
#elif defined (CONFIG_MSTAR_BD_MST135B_10AMVL_M5621_KONKIA)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST135B_10AMVL_M5621_KONKIA
    #include "BD_MST135B_10AMVL_M5621_KONKIA.h"
#elif defined (CONFIG_MSTAR_BD_MST135B_10AMVL_M5621_KNIGHT)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST135B_10AMVL_M5621_KNIGHT
    #include "BD_MST135B_10AMVL_M5621_KNIGHT.h"
#elif defined (CONFIG_MSTAR_BD_MST151B_10ABQHAT_M5621_HAIER)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST151B_10ABQHAT_M5621_HAIER
    #include "BD_MST151B_10ABQHAT_M5621_HAIER.h"
#elif defined (CONFIG_MSTAR_BD_MST151B_10ABQHAT_M5621_MTC)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST151B_10ABQHAT_M5621_MTC
    #include "BD_MST151B_10ABQHAT_M5621_MTC.h"
#elif defined (CONFIG_MSTAR_BD_MST151B_10ABQHAT_M5621_HKC)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST151B_10ABQHAT_M5621_HKC
    #include "BD_MST151B_10ABQHAT_M5621_HKC.h"
#elif defined (CONFIG_MSTAR_BD_MST151B_10ABQHAT_M5621_SHARP)
    #define MIU_STR_PATCH
    #define MS_BOARD_TYPE_SEL           BD_MST151B_10ABQHAT_M5621_SHARP
    #include "BD_MST151B_10ABQHAT_M5621_SHARP.h"

	
#else
#error "Unknown Board Selection"

#endif
#endif

#if defined(CONFIG_MIU0_DDR3_1866)
    #define CONFIG_MIU0_DDR3
#elif defined(CONFIG_MIU0_DDR3_2133)
    #define CONFIG_MIU0_DDR3
#elif defined(CONFIG_MIU0_DDR4_2400)
    #define CONFIG_MIU0_DDR4
#elif defined(CONFIG_MIU0_DDR4_2666)
    #define CONFIG_MIU0_DDR4
#else
#error "Unknown Board Selection"
#endif

#if defined(CONFIG_MIU1_DDR3_1866)
    #define CONFIG_MIU1_DDR3
#elif defined(CONFIG_MIU1_DDR3_2133)
    #define CONFIG_MIU1_DDR3
#elif defined(CONFIG_MIU1_DDR4_2400)
    #define CONFIG_MIU1_DDR4
#elif defined(CONFIG_MIU1_DDR4_2666)
    #define CONFIG_MIU1_DDR4
#else
#error "Unknown Board Selection"
#endif

#endif // _BOARD_H_
