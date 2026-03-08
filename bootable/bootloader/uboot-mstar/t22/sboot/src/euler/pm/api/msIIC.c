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
#define _IIC_C_

#include "Board.h"
#include "hwreg.h"
#include "datatype.h"
#include "GPIO_macro.h"
#include "drvIIC.h"
#include "MsIRQ.h"
#include "MsOS.h"

//                      SCL             SDA             SPEED
#if((MS_BOARD_TYPE_SEL == BD_MST149A_D01A_S)||(MS_BOARD_TYPE_SEL ==BD_MST031B_10AL0_11523))
    #define MIIC_BUS_SYS    PAD_GPIO0,  PAD_GPIO1,  SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCA0  PAD_GPIO0,  PAD_GPIO1,  SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCD0  PAD_GPIO0,  PAD_GPIO1,  SWI2C_SPEED_MAPPING_100K
#elif ((MS_BOARD_TYPE_SEL == BD_MST149B_D01A_S) || (MS_BOARD_TYPE_SEL == BD_MST149B_C01A))
    #define MIIC_BUS_SYS    PAD_GPIO0,  PAD_GPIO1,  SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCA0  PAD_GPIO0,  PAD_GPIO1,  SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCD0  PAD_GPIO0,  PAD_GPIO1,  SWI2C_SPEED_MAPPING_100K
#elif ((MS_BOARD_TYPE_SEL == BD_MST149C_D01A_S))
    #define MIIC_BUS_SYS    PAD_GPIO10, PAD_GPIO11, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCA0  PAD_GPIO10, PAD_GPIO11, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCD0  PAD_GPIO10, PAD_GPIO11, SWI2C_SPEED_MAPPING_100K
#elif ((MS_BOARD_TYPE_SEL == BD_MST149D_D01A_S))
    #define MIIC_BUS_SYS    PAD_GPIO10, PAD_GPIO11, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCA0  PAD_GPIO10, PAD_GPIO11, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCD0  PAD_GPIO10, PAD_GPIO11, SWI2C_SPEED_MAPPING_100K
#elif ((MS_BOARD_TYPE_SEL == BD_MST149E_D01A_S))
    #define MIIC_BUS_SYS    PAD_GPIO11, PAD_GPIO10, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCA0  PAD_GPIO11, PAD_GPIO10, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCD0  PAD_GPIO11, PAD_GPIO10, SWI2C_SPEED_MAPPING_100K
#elif ((MS_BOARD_TYPE_SEL == BD_MST149G_D01A_S))
    #define MIIC_BUS_SYS    PAD_GPIO11, PAD_GPIO10, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCA0  PAD_GPIO11, PAD_GPIO10, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCD0  PAD_GPIO11, PAD_GPIO10, SWI2C_SPEED_MAPPING_100K
#elif ((MS_BOARD_TYPE_SEL == BD_MST030B_10AL8_12052)||(MS_BOARD_TYPE_SEL == BD_MST030B_20AL8_12052))
    #define MIIC_BUS_SYS    PAD_TCON0,  PAD_TCON1,  SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCA0  PAD_TCON0,  PAD_TCON1,  SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCD0  PAD_TCON0,  PAD_TCON1,  SWI2C_SPEED_MAPPING_100K	
#elif((MS_BOARD_TYPE_SEL == BD_MST034B_10AL6_12071)|| (MS_BOARD_TYPE_SEL == BD_MST034B_20AL6_12071))
    #define MIIC_BUS_SYS    PAD_GPIO1, PAD_GPIO2, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCA0  PAD_GPIO1, PAD_GPIO2, SWI2C_SPEED_MAPPING_100K
    #define MIIC_BUS_DDCD0  PAD_GPIO1, PAD_GPIO2, SWI2C_SPEED_MAPPING_100K	
  #else
    #error "board type incorrect\n"
  #endif

SWI2C_BusCfg g_I2CBus[] =
{
    {MIIC_BUS_SYS},
    {MIIC_BUS_DDCA0},
    {MIIC_BUS_DDCD0},
};

/******************************************************************************/
///I2C Initialize: set I2C Clock and enable I2C
/******************************************************************************/
void MDrv_IIC_Init()
{
    MApi_SWI2C_Init(g_I2CBus, sizeof(g_I2CBus)/sizeof(SWI2C_BusCfg)); // FIXME: 5 should be counted automatically
    return;
}

#undef _IIC_C_

