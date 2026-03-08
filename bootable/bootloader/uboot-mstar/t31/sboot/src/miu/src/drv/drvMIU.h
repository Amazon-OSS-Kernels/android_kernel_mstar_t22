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
#ifndef _DRV_MIU_H_
#define _DRV_MIU_H_

#if defined(CONFIG_MSTAR_KRATOS)
  #include "../hal/kratos/halMIU.h"
#elif defined(CONFIG_MSTAR_MASERATI)
  #include "../hal/maserati/halMIU.h"
#elif defined(CONFIG_MSTAR_MAXIM)
  #include "../hal/maxim/halMIU.h"
#elif defined(CONFIG_MSTAR_MOONEY)
  #include "../hal/mooney/halMIU.h"
#elif defined(CONFIG_MSTAR_MAINZ)
  #include "../hal/mainz/halMIU.h"
#elif defined(CONFIG_MSTAR_MUSTANG)
  #include "../hal/mustang/halMIU.h"
#elif defined(CONFIG_MSTAR_M7621)
  #include "../hal/M7621/halMIU.h"
#elif defined(CONFIG_MSTAR_M7821)
  #include "../hal/M7821/halMIU.h"
#elif defined(CONFIG_MSTAR_MACAN)
  #include "../hal/macan/halMIU.h"
#elif defined(CONFIG_MSTAR_M7221)
  #include "../hal/M7221/halMIU.h"
#elif defined(CONFIG_MSTAR_M7322)
  #include "../hal/M7322/halMIU.h"
#elif defined(CONFIG_MSTAR_M7622)
  #include "../hal/M7622/halMIU.h"
#elif defined(CONFIG_MSTAR_M5621)
  #include "../hal/M5621/halMIU.h"
#elif defined(CONFIG_MSTAR_M3822)
  #include "../hal/M3822/halMIU.h"
#elif defined(CONFIG_MSTAR_M7632)
  #include "../hal/M7632/halMIU.h"
#elif defined(CONFIG_MSTAR_M7332)
  #include "../hal/M7332/halMIU.h"
#elif defined(CONFIG_MSTAR_M7642)
  #include "../hal/M7642/halMIU.h"
#elif defined(CONFIG_MSTAR_MAZDA)
  #include "../hal/mazda/halMIU.h"
#else
  #error "Error! no platform selected."
#endif


int MDrv_MIU_RestoreProtect(void);

#endif
