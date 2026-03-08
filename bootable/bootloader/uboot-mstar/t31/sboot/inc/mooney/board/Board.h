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

//------------------------------UNUSED MACROS-----------------------------------
#define _FUNC_NOT_USED()                do {} while ( 0 )

//------------------------------MS_BOARD_TYPE_SEL-------------------------------
// Demo board made in Taiwan

#define BD_FPGA_MOONEY                  0xC000
#define BD_MST272A_D01A_S_MOONEY        0xC001
#define BD_MST272A_D02A_S_MOONEY        0xC002
#define BD_MST272B_D01A_S_MOONEY        0xC003
#define BD_MST272C_D01A_S_MOONEY        0xC004

#define BD_MST116B_10A_MOONEY           0xD000
#define BD_MST117B_10A_MOONEY           0xD001
#define BD_MST123B_10A_MOONEY           0xD002
#define BD_MST129D_10A_MOONEY           0xD003
#define BD_MST128B_10A_MOONEY           0xD004

#define BD_UNKNOWN                      0xFFFF

#ifndef MS_BOARD_TYPE_SEL
#if defined (CONFIG_MSTAR_TITANIA_BD_FPGA)
#define MS_BOARD_TYPE_SEL               BD_FPGA_MOONEY
#include "BD_FPGA_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST272A_D01A_S_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST272A_D01A_S_MOONEY
#include "BD_MST272A_D01A_S_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST272A_D02A_S_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST272A_D02A_S_MOONEY
#include "BD_MST272A_D02A_S_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST272B_D01A_S_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST272B_D01A_S_MOONEY
#include "BD_MST272B_D01A_S_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST272H_D01A_S_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST272H_D01A_S_MOONEY
#include "BD_MST272H_D01A_S_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST272C_D01A_S_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST272C_D01A_S_MOONEY
#include "BD_MST272C_D01A_S_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST272F_D01A_S_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST272F_D01A_S_MOONEY
#include "BD_MST272F_D01A_S_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST116B_10A_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST116B_10A_MOONEY
#include "BD_MST116B_10A_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST117B_10A_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST117B_10A_MOONEY
#include "BD_MST117B_10A_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST123B_10A_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST123B_10A_MOONEY
#include "BD_MST123B_10A_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST161D_10A_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST161D_10A_MOONEY
#include "BD_MST161D_10A_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST162B_10A_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST162B_10A_MOONEY
#include "BD_MST162B_10A_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST129D_10A_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST129D_10A_MOONEY
#include "BD_MST129D_10A_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST128B_10A_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST128B_10A_MOONEY
#include "BD_MST128B_10A_MOONEY.h"

#elif defined(CONFIG_MSTAR_BD_MST142B_10A_MOONEY)
#define MS_BOARD_TYPE_SEL               BD_MST142B_10A_MOONEY
#include "BD_MST142B_10A_MOONEY.h"

#else
#error "Unknown Board Selection"

#endif
#endif

#endif // _BOARD_H_
