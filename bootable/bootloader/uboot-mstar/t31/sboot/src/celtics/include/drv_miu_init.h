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

#ifndef _DRV_MIU_INIT_H_
#define _DRV_MIU_INIT_H_

#ifndef _BOARD_H_
#include "Board.h"
#endif

#ifndef _C_RIUBASE_H_
#include "c_riubase.h"
#endif

#define MIU_VER                         'M','I','U','_','M','A','D','I','S','O','N',' ','V','e','r',':','1','_','0','_','0'
#define REG_ADDR_BASE                   0x1f000000

#ifndef __ASSEMBLER__

#ifndef __DRV_RIU_H__
#include "drvRIU.h"
#endif

#ifndef _MIU_FPGA_CELTICS_H_
#include "MIU_FPGA_CELTICS.h"
#endif


const MS_REG_INIT MIU_PreInit[] =
{
//  _RV32_2(0x113BEC, 0x8000),  //enable dramobf

#if !defined(CONFIG_MIU0_DRAM_NONE)
    //MIU0 Software reset
    _RV32_2(0x10121e, 0x0c00),  
    _RV32_2(0x10121e, 0x0c00),
    _RV32_2(0x10121e, 0x0c00),  
    _RV32_2(0x10121e, 0x0c01),  
    _RV32_2(0x10121e, 0x0c00),  

    //MIU0 Request mask
    _RV32_2(0x101246, 0xFFFE),  
    _RV32_2(0x101266, 0xFFFF),  
    _RV32_2(0x101286, 0xFFFF),  
    _RV32_2(0x1012A6, 0xFFFF),
    _RV32_2(0x161506, 0xFFFF),
    _RV32_2(0x161526, 0xFFFF),
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    //MIU1 Software reset
    _RV32_2(0x10061e, 0x0000),  
    _RV32_2(0x10061e, 0x0000),
    _RV32_2(0x10061e, 0x0000),
    _RV32_2(0x10061e, 0x0c01),
    _RV32_2(0x10061e, 0x0c00),

    //MIU1 Request mask
    _RV32_2(0x100646, 0xFFFE),  
    _RV32_2(0x100666, 0xFFFF),  
    _RV32_2(0x100686, 0xFFFF),  
    _RV32_2(0x1006A6, 0xFFFF),  
    _RV32_2(0x162206, 0xFFFF),  
    _RV32_2(0x162226, 0xFFFF),
#endif

#if !defined(CONFIG_MIU2_DRAM_NONE)
    //MIU2 Software reset
    _RV32_2(0x16201e, 0x0000),
    _RV32_2(0x16201e, 0x0000),
    _RV32_2(0x16201e, 0x0000),
    _RV32_2(0x16201e, 0x0c01),
    _RV32_2(0x16201e, 0x0c00),

    //MIU2 Request mask
    _RV32_2(0x162046, 0xFFFE),  
    _RV32_2(0x162066, 0xFFFF),
    _RV32_2(0x162086, 0xFFFF),
    _RV32_2(0x1620A6, 0xFFFF),
    _RV32_2(0x162306, 0xFFFF),
    _RV32_2(0x162326, 0xFFFF),
#endif
    _END_OF_TBL32_,
    MIU_VER
};

const MS_REG_INIT MIU_PostInit[] =
{
#if !defined(CONFIG_MIU0_DRAM_NONE)
    _RV32_2(0x1012e0, 0x0000),  //stop bist test

    //open all MIU0 request mask (All IP can access MIU resource)
    _RV32_2(0x101246, 0x0000),  //Mask MIU0_group_0
    _RV32_2(0x101266, 0x0000),  //Mask MIU0_group_1
    _RV32_2(0x101286, 0x0000),  //Mask MIU0_group_2
    _RV32_2(0x1012a6, 0x0000),  //Mask MIU0_group_3
    _RV32_2(0x161506, 0x0000),  //Mask MIU0_group_4
    _RV32_2(0x161526, 0x0000),  //Mask MIU0_group_5    
#endif

#if !defined(CONFIG_MIU1_DRAM_NONE)
    //open all MIU1 request mask (All IP can access MIU resource)
    _RV32_2(0x100646, 0x0000),  //Mask MIU1_group_0
    _RV32_2(0x100666, 0x0000),  //Mask MIU1_group_1
    _RV32_2(0x100686, 0x0000),  //Mask MIU1_group_2
    _RV32_2(0x1006a6, 0x0000),  //Mask MIU1_group_3
    _RV32_2(0x162206, 0x0000),  //Mask MIU1_group_4
    _RV32_2(0x162226, 0x0000),  //Mask MIU1_group_5

    //all IP choose sw select( not IP select )
    _RV32_2(0x1006f0, 0x0000),  
    _RV32_2(0x1006f2, 0x0000),  
    _RV32_2(0x1006f4, 0x0000),  
    _RV32_2(0x1006f6, 0x0000),  
    _RV32_2(0x1006f8, 0x0000),  
    _RV32_2(0x1006fa, 0x0000),  
#endif

#if !defined(CONFIG_MIU2_DRAM_NONE)
    //open all MIU2 request mask (All IP can access MIU resource)
    _RV32_2(0x162046, 0x0000),  //Mask MIU2_group_0
    _RV32_2(0x162066, 0x0000),  //Mask MIU2_group_1
    _RV32_2(0x162086, 0x0000),  //Mask MIU2_group_2
    _RV32_2(0x1620a6, 0x0000),  //Mask MIU2_group_3
    _RV32_2(0x162306, 0x0000),  //Mask MIU2_group_4
    _RV32_2(0x162326, 0x0000),  //Mask MIU2_group_5
#endif

    _RV32_2(0x10121e, 0x0c08),  //SW initial done and turn on arbitor

    _END_OF_TBL32_,
    MIU_VER
};

#endif /* !__ASSEMBLER__ */

#endif /* _DRV_MIU_INIT_H_ */
