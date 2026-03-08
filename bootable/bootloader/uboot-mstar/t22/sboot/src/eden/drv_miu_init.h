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

#ifndef __DDR2_INIT_H__
#define __DDR2_INIT_H__

#include "c_riubase.h"
#include "drvGlobal.h"

#define    _DRAM_SIZE_NO_LIMIT    0x00
#define    _DRAM_SIZE_2M          0x01
#define    _DRAM_SIZE_4M          0x02
#define    _DRAM_SIZE_8M          0x03
#define    _DRAM_SIZE_16M         0x04
#define    _DRAM_SIZE_32M         0x05
#define    _DRAM_SIZE_64M         0x06
#define    _DRAM_SIZE_128M        0x07
#define    _DRAM_SIZE_256M        0x07

#define    _DRAM_ON_BAORD         _DRAM_SIZE_NO_LIMIT

// A CMD + B DATA
#define MIU0_USE_MIU1_DATA (!CHIP_HAS_DDR_A_DATA && !CHIP_HAS_DDR_B_CMD)

#define DDR2_800MHz     0x01
#define DDR2_1066MHz    0x02
#define DDR2_1150MHz    0x03
#define DDR2_1200MHz    0x04
#define DDR2_1333MHz    0x05

#define DDR2_830MHz //based on 800
//#define DDR2_1140MHz //based on 1066

#if ( (ENABLE_MSTAR_EDEN_BD_MST165A_D01A_S) \
   || (ENABLE_MSTAR_EDEN_BD_MST165A_D01B_S) \
   || (ENABLE_MSTAR_EDEN_BD_MST165B_D01A_S) \
   || (ENABLE_MSTAR_EDEN_BD_MST165C_D01A_S) \
   || (ENABLE_MSTAR_EDEN_BD_MST165C_D01B_S) \
   || (ENABLE_MSTAR_EDEN_BD_MST202A_D01A_S) \
   || (ENABLE_MSTAR_EDEN_BD_MST043B_10ALB_12351) \
   || (ENABLE_MSTAR_EDEN_BD_MST043B_10BLB_12413) \
   || (ENABLE_MSTAR_EDEN_BD_MST042B_10BLU_12423) \
   || (ENABLE_MSTAR_EDEN_BD_MST042B_10BLU_12423_32) \
   || (ENABLE_MSTAR_EDEN_BD_MST044B_10BLA_12423) \
   || (ENABLE_MSTAR_EDEN_BD_MST044B_10BLA_12423_32) \
   || (ENABLE_MSTAR_EDEN_BD_MST165C2_D01B_S) \
   )
    #if ENABLE_MEM800MHz
        #define MIU_SPEED       DDR2_800MHz
    #elif ENABLE_MEM1066MHz
        #define MIU_SPEED       DDR2_1066MHz
    #elif ENABLE_MEM1150MHz
        #define MIU_SPEED       DDR2_1150MHz
    #elif ENABLE_MEM1200MHz
        #define MIU_SPEED       DDR2_1200MHz
    #elif ENABLE_MEM1333MHz
        #define MIU_SPEED       DDR2_1333MHz
    #else
        #error "Please select correct DDR2 Speed"
    #endif

#else
    #error "MIU interface isn't defined"
#endif

#if (MIU_SPEED == DDR2_800MHz)//MCLK 400MHz setting
static const MS_REG_INIT MIU_ATOP_Init[]=
{
    //set test mode of DDRPLL
    _RV32_2( 0x110d24 , 0x0000),
    //Clock setting
    _RV32_2( 0x110d36 , 0x0000),
    _RV32_2( 0x110d34 , 0x0200),
    //Set the clock frequency for dram & PLL reset
    #ifdef DDR2_830MHz//MCLK 415MHz setting
    _RV32_2( 0x110d30 , 0x9f26),
    _RV32_2( 0x110d32 , 0x0042),
    #else//MCLK 400MHz setting
    _RV32_2( 0x110d30 , 0xdcfa),
    _RV32_2( 0x110d32 , 0x0045),
    #endif
    _END_OF_TBL32_
};
#else
static const MS_REG_INIT MIU_ATOP_Init_1066[]=//G_1066
{
    //set test mode of DDRPLL
    _RV32_2( 0x110d24 , 0x0000),
    //Clock setting
    _RV32_2( 0x110d36 , 0x0000),
    _RV32_2( 0x110d34 , 0x0200),
    //Set the clock frequency for dram & PLL reset
    #if defined (DDR2_1140MHz)//MCLK 570MHz setting 
    _RV32_2( 0x110d30 , 0x06dc),
    _RV32_2( 0x110d32 , 0x0031),
    #else//MCLK 533MHz setting
    _RV32_2( 0x110d30 , 0x6e1f),
    _RV32_2( 0x110d32 , 0x0034),
    #endif
    _END_OF_TBL32_
};

static const MS_REG_INIT MIU_ATOP_Init_1333[]=//G_1333
{
    //set test mode of DDRPLL
    _RV32_2( 0x110d24 , 0x0000),
    //Clock setting
    _RV32_2( 0x110d36 , 0x0000),
    _RV32_2( 0x110d34 , 0x0200),
    //Set the clock frequency for dram & PLL reset
    #if (MIU_SPEED == DDR2_1150MHz)//MCLK 575MHz setting
    _RV32_2( 0x110d30, 0x06dc ),
    _RV32_2( 0x110d32, 0x0031 ),
    #elif (MIU_SPEED == DDR2_1200MHz)//MCLK 600MHz setting
    _RV32_2( 0x110d30, 0x8000 ),
    _RV32_2( 0x110d32, 0x002e ),
    #elif (MIU_SPEED == DDR2_1333MHz)//MCLK 666MHz setting
    _RV32_2( 0x110d30, 0xedab ),
    _RV32_2( 0x110d32, 0x0029 ),
    #endif
    _END_OF_TBL32_
};
#endif


#if (MIU_SPEED == DDR2_800MHz)//MCLK 400MHz setting
static const MS_REG_INIT MIU_DDR_Init[] =
{
    #if (ENABLE_MSTAR_EDEN_MMAP_32MB)
    _RV32_2( 0x101202, 0x0252 ),        
    #else
    _RV32_2( 0x101202, 0x0292 ),
    #endif    
    _RV32_2( 0x101204, 0x008b ), //rd_timing
    #ifdef DDR2_830MHz//MCLK 415MHz setting
    _RV32_2( 0x101206, 0x0332 ),
    #else//MCLK 400MHz setting
    _RV32_2( 0x101206, 0x0330 ),
    #endif
    _RV32_2( 0x101208, 0x1666 ),
    _RV32_2( 0x10120a, 0x1c56 ),
    _RV32_2( 0x10120c, 0x7485 ),
    _RV32_2( 0x10120e, 0x202a ),
    _RV32_2( 0x101210, 0x0a62 ),
    _RV32_2( 0x101212, 0x4006 ),
    _RV32_2( 0x101214, 0x8000 ),
    _RV32_2( 0x101216, 0xc000 ),
    #ifdef DDR2_830MHz//MCLK 415MHz setting
    _RV32_2( 0x110d38, 0x0077 ),//reg_clkph_dq0             5       : [3:0]
                                //reg_clkph_dq1         5       : [7:4]
                                //reg_clkph_dqs0        0       : [11:8]
                                //reg_clkph_dqs1        0       : [15:12]
    _RV32_2( 0x110d3a, 0x0552 ),//reg_clkph_cmd             0       : [3:0]
                                //reg_clkph_dqsm0        4       : [7:4]
                                //reg_clkph_clk_a          4       : [11:8]
                                //reg_clkph_dqsm1          0       : [15:12]
    #else//MCLK 400MHz setting
    _RV32_2( 0x110d36, 0x0002 ),//reg_clkph_cko
    _RV32_2( 0x110d38, 0x2288 ),//reg_clkph_dq0             5       : [3:0]
                                //reg_clkph_dq1         5       : [7:4]
                                //reg_clkph_dqs0        0       : [11:8]
                                //reg_clkph_dqs1        0       : [15:12]
    _RV32_2( 0x110d3a, 0x0110 ),//reg_clkph_cmd             0       : [3:0]
                                //reg_clkph_dqsm0        4       : [7:4]
                                //reg_clkph_clk_a          4       : [11:8]
                                //reg_clkph_dqsm1          0       : [15:12]
    #endif
    _RV32_2( 0x110d02, 0x0000 ),
    _RV32_2( 0x10122c, 0x9300 ),
    _RV32_2( 0x110d04, 0x0000 ),//reg_dqs_pre_state : [7:0]
    _RV32_2( 0x110d3c, 0x0200 ),//reg_dqsm0_skew 0 [2:0]
                                //reg_dqsm1_skew 0 [6:4]
                                //reg_dqs_skew   2 [10:8]
                                //reg_cmd1_skew  0 [14:12]
    _RV32_2( 0x110d3e, 0x0022 ),//[2:0]   reg_oen_skew
                                //[6:4]   reg_dq_skew
                                //[10:8]  reg_cmd_skew
                                //[14:12] reg_cko_skew

    _RV32_2( 0x110d0e, 0x00e9 ),//[3:1] reg_dqsm_dly
    _RV32_2( 0x110d0e, 0x00a9 ),
    #ifdef DDR2_830MHz//MCLK 415MHz setting
    _RV32_2( 0x110d6e, 0x0065 ),//reg_dll0_ph : [3:0]
                                //reg_dll1_ph : [7:4]
    #else//MCLK 400MHz setting
    _RV32_2( 0x110d6e, 0x0076 ),//reg_dll0_ph : [3:0]
                                //reg_dll1_ph : [7:4]
    #endif
    _RV32_2( 0x110d6a, 0x004f ),
    _RV32_2( 0x110d68, 0x004f ),

    _END_OF_TBL32_,
};
#else
static const MS_REG_INIT MIU_DDR_Init_1066[]=//G_1066
{
    _RV32_2( 0x101202, 0x0292 ),
    _RV32_2( 0x101204, 0x008c ), //rd_timing
    #if defined DDR2_1140MHz
    _RV32_2( 0x101206, 0x0430 ),
    _RV32_2( 0x101208, 0x1a98 ),
    _RV32_2( 0x10120a, 0x2555 ),
    _RV32_2( 0x10120c, 0x55a6 ),
    _RV32_2( 0x10120e, 0x203c ),
    _RV32_2( 0x101210, 0x0e72 ),
    #else
    _RV32_2( 0x101206, 0x0330 ),
    _RV32_2( 0x101208, 0x1888 ),
    _RV32_2( 0x10120a, 0x2246 ),
    _RV32_2( 0x10120c, 0x54a6 ),
    _RV32_2( 0x10120e, 0x2038 ),
    _RV32_2( 0x101210, 0x0e72 ),
    #endif
    _RV32_2( 0x101212, 0x4006 ), //0x4042//150 ohm for DRAM ODT ON (0x101200[4] = 1)
    _RV32_2( 0x101214, 0x8000 ),
    _RV32_2( 0x101216, 0xc000 ),

    _RV32_2( 0x110d38, 0x0055 ),//reg_clkph_dq0             5       : [3:0]
                                //reg_clkph_dq1         5       : [7:4]
                                //reg_clkph_dqs0        0       : [11:8]
                                //reg_clkph_dqs1        0       : [15:12]
    _RV32_2( 0x110d3a, 0x0000 ),//reg_clkph_cmd             0       : [3:0]
                                //reg_clkph_dqsm0        0       : [7:4]
                                //reg_clkph_clk_a          0       : [11:8]
                                //reg_clkph_dqsm1          0       : [15:12]
    _RV32_2( 0x110d02, 0x0000 ),
    _RV32_2( 0x10122c, 0x9300 ),
    _RV32_2( 0x110d04, 0x0000 ),//reg_dqs_pre_state : [7:0]
    _RV32_2( 0x110d3c, 0x0022 ),//reg_dqsm0_skew 0 [2:0]
                                //reg_dqsm1_skew 0 [6:4]
                                //reg_dqs_skew   2 [10:8]
                                //reg_cmd1_skew  0 [14:12]
    _RV32_2( 0x110d3e, 0x0000 ),//[2:0]   reg_oen_skew
                                //[6:4]   reg_dq_skew
                                //[10:8]  reg_cmd_skew
                                //[14:12] reg_cko_skew

    //##################==
    _RV32_2( 0x110d0e, 0x00e7 ),//[3:1] reg_dqsm_dly
    _RV32_2( 0x110d0e, 0x00a7 ),
    //##################==
    _RV32_2( 0x110d6e, 0x0065 ),//reg_dll0_ph : [3:0]
                                //reg_dll1_ph : [7:4]
    _RV32_2( 0x110d6a, 0x004f ),
    _RV32_2( 0x110d68, 0x004f ),
    _END_OF_TBL32_,
};

static const MS_REG_INIT MIU_DDR_Init_1333[]=//G_1066
{
    _RV32_2( 0x101202, 0x0292 ),
    _RV32_2( 0x101204, 0x008d ), //rd_timing
    #if (MIU_SPEED == DDR2_1150MHz)//MCLK 575MHz setting
    _RV32_2( 0x101206, 0x0330 ),
    _RV32_2( 0x101208, 0x1b99 ),
    _RV32_2( 0x10120a, 0x2356 ),
    _RV32_2( 0x10120c, 0x55a8 ),
    _RV32_2( 0x10120e, 0x203f ),
    _RV32_2( 0x101210, 0x0002 ),
    #elif (MIU_SPEED == DDR2_1200MHz)//MCLK 600MHz setting
    _RV32_2( 0x101206, 0x0330 ),
    _RV32_2( 0x101208, 0x1b99 ),
    _RV32_2( 0x10120a, 0x2356 ),
    _RV32_2( 0x10120c, 0x55a8 ),
    _RV32_2( 0x10120e, 0x203f ),
    _RV32_2( 0x101210, 0x0002 ),
    #elif (MIU_SPEED == DDR2_1333MHz)//MCLK 666MHz setting
    _RV32_2( 0x101206, 0x0330 ),
    _RV32_2( 0x101208, 0x1e99 ),
    _RV32_2( 0x10120a, 0x2757 ),
    _RV32_2( 0x10120c, 0x55a8 ),
    _RV32_2( 0x10120e, 0x2046 ),
    _RV32_2( 0x101210, 0x0002 ),
    #endif
    _RV32_2( 0x101212, 0x4006 ), //0x4042//150 ohm for DRAM ODT ON (0x101200[4] = 1)
    _RV32_2( 0x101214, 0x8000 ),
    _RV32_2( 0x101216, 0xc000 ),

    _RV32_2( 0x110d38, 0x0055 ),//reg_clkph_dq0             5       : [3:0]
                                //reg_clkph_dq1         5       : [7:4]
                                //reg_clkph_dqs0        0       : [11:8]
                                //reg_clkph_dqs1        0       : [15:12]
    _RV32_2( 0x110d3a, 0x0000 ),//reg_clkph_cmd             0       : [3:0]
                                //reg_clkph_dqsm0        0       : [7:4]
                                //reg_clkph_clk_a          0       : [11:8]
                                //reg_clkph_dqsm1          0       : [15:12]
    _RV32_2( 0x110d02, 0x0000 ),
    _RV32_2( 0x10122c, 0x9300 ),
    _RV32_2( 0x110d04, 0x0000 ),//reg_dqs_pre_state : [7:0]
    _RV32_2( 0x110d3c, 0x0022 ),//reg_dqsm0_skew 0 [2:0]
                                //reg_dqsm1_skew 0 [6:4]
                                //reg_dqs_skew   2 [10:8]
                                //reg_cmd1_skew  0 [14:12]
    _RV32_2( 0x110d3e, 0x0000 ),//[2:0]   reg_oen_skew
                                //[6:4]   reg_dq_skew
                                //[10:8]  reg_cmd_skew
                                //[14:12] reg_cko_skew

    //##################==
    _RV32_2( 0x110d0e, 0x00eb ),//[3:1] reg_dqsm_dly
    _RV32_2( 0x110d0e, 0x00ab ),
    //##################==
    _RV32_2( 0x110d6e, 0x0065 ),//reg_dll0_ph : [3:0]
                                //reg_dll1_ph : [7:4]
    _RV32_2( 0x110d6a, 0x004f ),
    _RV32_2( 0x110d68, 0x004f ),
    _END_OF_TBL32_,
};
#endif

#endif /* __DDR2_INIT_H__ */

