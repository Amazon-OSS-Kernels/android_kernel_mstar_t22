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

#define DDR2_800MHz     0x00 //mcp
#define DDR2_1066MHz    0x01 //mcp
#define DDR3_1333MHz    0x02 //mcp
#define DDR2_800MHz_e   0x11 //external
#define DDR2_1066MHz_e  0x12 //external
#define DDR3_1300MHz_e  0x13 //external
#define DDR3_1600MHz_e  0x14 //external

//#define DDR2_900MHz
//#define DDR2_960MHz
//#define DDR2_1200MHz
//#define DDR3_1000MHz
//#define DDR3_800MHz

#if ( (ENABLE_MSTAR_EULER_BD_MST165A_D01B_S) \
   || (ENABLE_MSTAR_EULER_BD_MST165B_D01B_S) \
   || (ENABLE_MSTAR_EULER_BD_MST042B_EULER) \
   || (ENABLE_MSTAR_EULER_BD_MST047B_EULER) \
   || (ENABLE_MSTAR_EULER_BD_MST165C_D01B_S) \
   )
    #if ENABLE_MEM800MHz
        #define MIU_SPEED       DDR2_800MHz
    #elif ENABLE_MEM1066MHz
        #define MIU_SPEED       DDR2_1066MHz
    #elif ENABLE_MEM1333MHz
        #define MIU_SPEED       DDR3_1333MHz
    #else
        #error "Please select correct DDR2 Speed"
    #endif

#else
    #error "MIU interface isn't defined"
#endif

static const MS_REG_INIT MIU_ATOP_Init[]=
{
    //set test mode of DDRPLL
    _RV32_2(0x110d24, 0x0000),
    //Clock setting
    _RV32_2(0x110d36, 0x0000),
    _RV32_2(0x110d34, 0x0200),
    //Set the clock frequency for dram & PLL reset
  #if (MIU_SPEED == DDR2_800MHz)//MCLK 400MHz setting
    _RV32_2(0x110d30, 0xdcf9),
    _RV32_2(0x110d32, 0x0045),
    _RV32_2(0x110d24, 0x1000),
    _RV32_2(0x110da4, 0x1000),
    _RV32_2(0x110d24, 0x0000),
    _RV32_2(0x110da4, 0x0000),
  #elif (MIU_SPEED == DDR2_1066MHz)//MCLK 533MHz setting
    #ifdef DDR2_900MHz //MCLK 450MHz setting
    _RV32_2(0x110d30, 0x70a3),
    _RV32_2(0x110d32, 0x003d),
    #elif defined (DDR2_960MHz)//MCLK 480MHz setting
    _RV32_2(0x110d30, 0x9999),
    _RV32_2(0x110d32, 0x0039),
    #elif defined (DDR2_1200MHz)//MCLK 600MHz setting
    _RV32_2(0x110d30, 0x147e),
    _RV32_2(0x110d32, 0x002e),
    #else//MCLK 533MHz setting
    _RV32_2(0x110d30, 0x6e1e),
    _RV32_2(0x110d32, 0x0034),
    _RV32_2(0x110d24, 0x1000),
    _RV32_2(0x110da4, 0x1000),
    _RV32_2(0x110d24, 0x0000),
    _RV32_2(0x110da4, 0x0000),
    #endif
  #elif (MIU_SPEED == DDR3_1333MHz)//MCLK 650MHz setting
    #ifdef DDR3_1000MHz
    _RV32_2(0x110d30, 0x4bcb),
    _RV32_2(0x110d32, 0x0037),
    _RV32_2(0x110d24, 0x1000),
    _RV32_2(0x110da4, 0x1000),
    _RV32_2(0x110d24, 0x0000),
    _RV32_2(0x110da4, 0x0000),
    #elif defined (DDR3_800MHz)//MCLK 480MHz setting
    _RV32_2(0x110d30, 0xdcf9),
    _RV32_2(0x110d32, 0x0045),
    _RV32_2(0x110d24, 0x1000),
    _RV32_2(0x110da4, 0x1000),
    _RV32_2(0x110d24, 0x0000),
    _RV32_2(0x110da4, 0x0000),
    #else
    _RV32_2(0x110d30, 0x7b80),
    _RV32_2(0x110d32, 0x0029),
    _RV32_2(0x110d24, 0x1000),
    _RV32_2(0x110da4, 0x1000),
    _RV32_2(0x110d24, 0x0000),
    _RV32_2(0x110da4, 0x0000),
    #endif
  #else
    #error "MIU interface isn't defined"
  #endif

    _END_OF_TBL32_
};

static const MS_REG_INIT MIU_DDR_Init[] =
{
  #if (MIU_SPEED == DDR2_800MHz)
    _RV32_2( 0x101202, 0x0292 ),
    _RV32_2( 0x101204, 0x008b ), //rd_timing
    _RV32_2( 0x101206, 0x0330 ),
    _RV32_2( 0x101208, 0x1466 ),
    _RV32_2( 0x10120a, 0x1644 ),
    _RV32_2( 0x10120c, 0x7465 ),
    _RV32_2( 0x10120e, 0x204f ),
    _RV32_2( 0x101210, 0x0a62 ),
    _RV32_2( 0x101212, 0x4040 ), //{[6], [2]} = 10
    _RV32_2( 0x101214, 0x8000 ),
    _RV32_2( 0x101216, 0xc000 ),
  #elif (MIU_SPEED == DDR2_1066MHz)
    _RV32_2( 0x101202, 0x0292 ),
    _RV32_2( 0x101204, 0x008c ), //rd_timing
    _RV32_2( 0x101206, 0x0434 ),
    _RV32_2( 0x101208, 0x1688 ),
    _RV32_2( 0x10120a, 0x2246 ),
    _RV32_2( 0x10120c, 0x7496 ),
    _RV32_2( 0x10120e, 0x206a ),
    _RV32_2( 0x101210, 0x0e72 ),
    _RV32_2( 0x101212, 0x4040 ), //{[6], [2]} = 10 //0x4042//150 ohm for DRAM ODT ON (0x101200[4] = 1)
    _RV32_2( 0x101214, 0x8000 ),
    _RV32_2( 0x101216, 0xc000 ),
  #elif (MIU_SPEED == DDR3_1333MHz)
    #ifdef DDR3_1000MHz
    _RV32_2( 0x101202, 0x03a3 ),
    _RV32_2( 0x101204, 0x000a ), //rd_timing
    _RV32_2( 0x101206, 0x1420 ),
    _RV32_2( 0x101208, 0x1899 ),
    _RV32_2( 0x10120a, 0x2155 ),
    _RV32_2( 0x10120c, 0xa5a7 ),
    _RV32_2( 0x10120e, 0x406b ),
    _RV32_2( 0x101210, 0x1b50 ),
    _RV32_2( 0x101212, 0x4004 ),
    _RV32_2( 0x101214, 0x8010 ),
    _RV32_2( 0x101216, 0xc000 ),
    #else
    _RV32_2( 0x101202, 0x03a3 ),
    _RV32_2( 0x101204, 0x004b ), //rd_timing
    _RV32_2( 0x101206, 0x1434 ),
    _RV32_2( 0x101208, 0x1899 ),
    _RV32_2( 0x10120a, 0x2155 ),
    _RV32_2( 0x10120c, 0xa5a7 ),
    _RV32_2( 0x10120e, 0x406b ),
    _RV32_2( 0x101210, 0x1b50 ),
    _RV32_2( 0x101212, 0x4040 ),
    _RV32_2( 0x101214, 0x8010 ),
    _RV32_2( 0x101216, 0xc000 ),
    #endif
  #else
    #error "MIU interface isn't defined"
  #endif

  #if (MIU_SPEED == DDR2_800MHz)
    _RV32_2( 0x110d54, 0x0000 ),
    _RV32_2( 0x110d00, 0x0019 ),
    _RV32_2( 0x110d38, 0x0065 ),//reg_clkph_dq0             5       : [3:0]
                                //reg_clkph_dq1         5       : [7:4]
                                //reg_clkph_dqs0        0       : [11:8]
                                //reg_clkph_dqs1        0       : [15:12]
    _RV32_2( 0x110d3a, 0x0000 ),//reg_clkph_cmd             0       : [3:0]
                                //reg_clkph_dqsm0        4       : [7:4]
                                //reg_clkph_clk_a          4       : [11:8]
                                //reg_clkph_dqsm1          0       : [15:12]
    _RV32_2( 0x110d02, 0x0000 ),
    _RV32_2( 0x110d04, 0x0000 ),//reg_dqs_pre_state : [7:0]
    _RV32_2( 0x110d3c, 0x0200 ),//reg_dqsm0_skew 0 [2:0]
                                //reg_dqsm1_skew 0 [6:4]
                                //reg_dqs_skew   2 [10:8]
                                //reg_cmd1_skew  0 [14:12]
    _RV32_2( 0x110d3e, 0x0022 ),//[2:0]   reg_oen_skew
                                //[6:4]   reg_dq_skew
                                //[10:8]  reg_cmd_skew
                                //[14:12] reg_cko_skew

    _RV32_2( 0x110d0e, 0x00e7 ),//[3:1] reg_dqsm_dly
    _RV32_2( 0x110d0e, 0x00a7 ),
    _RV32_2( 0x110d6e, 0x0066 ),//reg_dll0_ph : [3:0]
                                //reg_dll1_ph : [7:4]
    _RV32_2( 0x110d6a, 0x00c2 ),
    _RV32_2( 0x110d68, 0x00c2 ),
    _RV32_2( 0x110d54, 0x0000 ),
  #elif (MIU_SPEED == DDR2_1066MHz)
    _RV32_2( 0x110d54, 0x0000 ),
    _RV32_2( 0x110d00, 0x0019 ),
    _RV32_2( 0x110d38, 0x0055 ),//reg_clkph_dq0             3       : [3:0]
                                //reg_clkph_dq1         3       : [7:4]
                                //reg_clkph_dqs0        0       : [11:8]
                                //reg_clkph_dqs1        0       : [15:12]
    _RV32_2( 0x110d3a, 0x0000 ),//reg_clkph_cmd             0       : [3:0]
                                //reg_clkph_dqsm0        0       : [7:4]
                                //reg_clkph_clk_a          0       : [11:8]
                                //reg_clkph_dqsm1          0       : [15:12]
    _RV32_2( 0x110d02, 0x0000 ),
    _RV32_2( 0x110d04, 0x0000 ),//reg_dqs_pre_state : [7:0]
    _RV32_2( 0x110d3c, 0x0022 ),//reg_dqsm0_skew 0 [2:0]
                                //reg_dqsm1_skew 0 [6:4]
                                //reg_dqs_skew   2 [10:8]
                                //reg_cmd1_skew  0 [14:12]
    _RV32_2( 0x110d3e, 0x0000 ),//[2:0]   reg_oen_skew
                                //[6:4]   reg_dq_skew
                                //[10:8]  reg_cmd_skew
                                //[14:12] reg_cko_skew

    _RV32_2( 0x110d0e, 0x00e7 ),//[3:1] reg_dqsm_dly
    _RV32_2( 0x110d0e, 0x00a7 ),
    _RV32_2( 0x110d6e, 0x0056 ),//reg_dll0_ph : [3:0]
                                //reg_dll1_ph : [7:4]
    _RV32_2( 0x110d6a, 0x00c2 ),
    _RV32_2( 0x110d68, 0x00c2 ),
    _RV32_2( 0x110d54, 0x0000 ),
  #elif (MIU_SPEED == DDR3_1333MHz)
    #ifdef DDR3_1000MHz
    _RV32_2( 0x110d54, 0x0000 ),
    _RV32_2( 0x110d7e, 0x00f8 ),
    _RV32_2( 0x110d00, 0x0018 ),
    _RV32_2( 0x110d04, 0x0080 ),//reg_dqs_pre_state : [7:0]
    _RV32_2( 0x110d38, 0x0055 ),//reg_clkph_dq0             4       : [3:0]
                                //reg_clkph_dq1         4       : [7:4]
                                //reg_clkph_dqs0        0       : [11:8]
                                //reg_clkph_dqs1        0       : [15:12]
    _RV32_2( 0x110d3a, 0x0001 ),//reg_clkph_cmd             2       : [3:0]
                                //reg_clkph_dqsm        0       : [7:4]
                                //reg_clkph_sp0          0       : [11:8]
                                //reg_clkph_sp1          0       : [15:12]
    _RV32_2( 0x110d4a, 0x0002 ),
    _RV32_2( 0x110d02, 0x0000 ),
    _RV32_2( 0x110d3c, 0x0622 ),//reg_dqsm0_skew 0 [2:0]
                                //[10:8]  reg_dqsm_skew
                                //[14:12] reg_dqs_skew
    _RV32_2( 0x110d3e, 0x0066 ),//[2:0]   reg_oen_skew
                                //[6:4]   reg_dq_skew
                                //[10:8]  reg_cmd_skew
                                //[14:12] reg_cko_skew
    _RV32_2( 0x110d50, 0x0000 ),//[3:1] reg_dqsm_dly

    _RV32_2( 0x110d0e, 0x00e5 ),//[3:1] reg_dqsm_dly
    _RV32_2( 0x110d0e, 0x00a5 ),
    _RV32_2( 0x110d6e, 0x0044 ),//reg_dll0_ph : [3:0]
                                //reg_dll1_ph : [7:4]
    _RV32_2( 0x110d6a, 0x0124 ),
    _RV32_2( 0x110d68, 0x0124 ),
    _RV32_2( 0x110d54, 0x0000 ),
    #else
    _RV32_2( 0x110d54, 0x0000 ),
    _RV32_2( 0x110d7e, 0x00f8 ),
    _RV32_2( 0x110d00, 0x0018 ),
    _RV32_2( 0x110d04, 0x0080 ),//reg_dqs_pre_state : [7:0]
    _RV32_2( 0x110d38, 0x0066 ),//reg_clkph_dq0             4       : [3:0]
                                //reg_clkph_dq1         4       : [7:4]
                                //reg_clkph_dqs0        0       : [11:8]
                                //reg_clkph_dqs1        0       : [15:12]
    _RV32_2( 0x110d3a, 0x0222 ),//reg_clkph_cmd             2       : [3:0]
                                //reg_clkph_dqsm0        0       : [7:4]
                                //reg_clkph_dqsm1          0       : [11:8]
                                //reg_clkph_clk_a          0       : [15:12]
    _RV32_2( 0x110d4a, 0x0002 ),
    _RV32_2( 0x110d02, 0x0000 ),
    _RV32_2( 0x110d3c, 0x1733 ),//reg_dqsm0_skew 0 [2:0]
                                //[10:8]  reg_dqsm_skew
                                //[14:12] reg_dqs_skew
    _RV32_2( 0x110d3e, 0x1077 ),//[2:0]   reg_oen_skew
                                //[6:4]   reg_dq_skew
                                //[10:8]  reg_cmd_skew
                                //[14:12] reg_cko_skew
    _RV32_2( 0x110d50, 0x0001 ),//[2:0]      reg_cs0_skew
                                //[6:4]      reg_cs1_skew
                                //[10:8]     reg_cs2_skew
                                //[14:12]    reg_cs3_skew
    _RV32_2( 0x110d0e, 0x00e5 ),//[3:1] reg_dqsm_dly
    _RV32_2( 0x110d0e, 0x00a5 ),
    _RV32_2( 0x110d6e, 0x0033 ),//reg_dll0_ph : [3:0]
                                //reg_dll1_ph : [7:4]
    _RV32_2( 0x110d6a, 0x0124 ),
    _RV32_2( 0x110d68, 0x0124 ),
    _RV32_2( 0x110d54, 0xc000 ), //reg_cmd_mode_sel    [15:14]
    #endif
  #else
    #error "MIU interface isn't defined"
  #endif

    _END_OF_TBL32_,
};

#endif /* __DDR2_INIT_H__ */

