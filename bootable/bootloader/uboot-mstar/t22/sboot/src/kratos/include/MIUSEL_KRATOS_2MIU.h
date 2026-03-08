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

#ifndef _MIUSEL_KRATOS_2MIU_H_
#define _MIUSEL_KRATOS_2MIU_H_

//----------------------------------------------------------
//priority setting: 0:highest priority, 3:lowest priority
//----------------------------------------------------------
//MIU group priority
//----------------------------------------------------------

#define  GROUP_0                  0
#define  GROUP_1                  1
#define  GROUP_2                  2
#define  GROUP_3                  3

#define  MIU0_1ST_PRIORITY        GROUP_2
#define  MIU0_2ND_PRIORITY        GROUP_0
#define  MIU0_3RD_PRIORITY        GROUP_1
#define  MIU0_4TH_PRIORITY        GROUP_3

#define  MIU1_1ST_PRIORITY        GROUP_1
#define  MIU1_2ND_PRIORITY        GROUP_2
#define  MIU1_3RD_PRIORITY        GROUP_3
#define  MIU1_4TH_PRIORITY        GROUP_0

#define MIU0_GROUP_PRIORITY       ( ( MIU0_4TH_PRIORITY << 6 )| (MIU0_3RD_PRIORITY<<4)| (MIU0_2ND_PRIORITY<<2) | (MIU0_1ST_PRIORITY) )
#define MIU1_GROUP_PRIORITY       ( ( MIU1_4TH_PRIORITY << 6 )| (MIU1_3RD_PRIORITY<<4)| (MIU1_2ND_PRIORITY<<2) | (MIU1_1ST_PRIORITY) )

////////////////////////////////////////////////////////////////////////////////
// MIU assignment   0: MIU0, 1: MIU1  2:IP_
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------
//
//----------------------------------------
//group_0
#define  RESERVED_for_MIU_SELF     0
#define  VIVALDI9_DSP_DECODER      0
#define  VIVALDI9_DSP_SE           0
#define  MIPS_W                    0
#define  MIPS_R                    0
#define  MAU0_W                    0
#define  MAU1_R                    0
#define  VD_MHEG5_DCACHE_RW        1
#define  VD_MHEG5_QDMA_RW          1
#define  GE_RW                     0
#define  HVD_RW                    1
#define  HVD_BBU_RW                1
#define  UHC0_RW                   0
#define  UHC1_RW                   0
#define  PIU_DMA_RW                2
#define  ADCDVIPLL_RW              0    //co_client with MIU

//group_1
#define  VIVALDI9_MAD              0
#define  VIVALDI9_DMA              0
#define  VIVALDI9_AU_DMA           0
#define  MVOP_RW                   1   //mvop_128
#define  GOP_W                     0
#define  GOP0_R                    0
#define  GOP1_R                    0
#define  GOP2_R                    0
#define  GOP3_R                    0
#define  SC_OP_R                   0
#define  SC_IP_MIAN_W              0
#define  SC_IP_MIAN_R              0
#define  VD_COMB_W                 0
#define  VD_COMB_R                 0
#define  VE_W                      0
#define  VE_R                      0

//backward competiable
#define   MIUSEL_FLAG_DNRB_R        SC_IP_MIAN_R
#define   MIUSEL_FLAG_DNRB_RW       (SC_IP_MIAN_R | SC_IP_MIAN_W)


//group_2
#define  OD_W                      0
#define  OD_R                      0
#define  OD_LSB_W                  0
#define  OD_LSB_R                  0
#define  SC_IP_SUB_W               0
#define  SC_IP_SUB_R               0
#define  DISP_IPATH_NR_RW          0
#define  DISP_IPATH_MR_RW          0
#define  DISP_IPATH_DI_W           0
#define  VD_MHEG5_ICACHE           1
#define  TSP_W                     0
#define  TSP_R                     0
#define  VD_TTX_RW                 0
#define  VD_TTXSL_W                0
#define  TSP_ORZ_W                 0
#define  TSP_ORZ_R                 0

//group_3
#define  MVOP_64_R                 1
#define  M4VE_0_RW                 0
#define  M4VE_ME_R                 0
#define  M4VE_2                    0
#define  MVD_RW                    1
#define  MVD_BBU_RW                1
#define  RVD_RW                    1
#define  RVD_BBU_RW                1
#define  JPD_RW                    0
#define  DSCRMB_RW                 0
#define  STRLD_RW                  0
#define  FUART_RW                  0
#define  EMAC_RW                   0
#define  NAND_RW                   0
#define  MPIF_RW                   2   // MIU assignment   0: MIU0, 1: MIU1  2:IP_ASSIGN
#define  RESERVED_G3D_RW           0

#define  ClientGruop0_in_MIU1     ((( VIVALDI9_DSP_DECODER == 1)? 0x0002 : 0  )| \
                                   (( VIVALDI9_DSP_SE      == 1)? 0x0004 : 0  )| \
                                   (( MIPS_W               == 1)? 0x0008 : 0  )| \
                                   (( MIPS_R               == 1)? 0x0010 : 0  )| \
                                   (( MAU0_W               == 1)? 0x0020 : 0  )| \
                                   (( MAU1_R               == 1)? 0x0040 : 0  )| \
                                   (( VD_MHEG5_DCACHE_RW   == 1)? 0x0080 : 0  )| \
                                   (( VD_MHEG5_QDMA_RW     == 1)? 0x0100 : 0  )| \
                                   (( GE_RW                == 1)? 0x0200 : 0  )| \
                                   (( HVD_RW               == 1)? 0x0400 : 0  )| \
                                   (( HVD_BBU_RW           == 1)? 0x0800 : 0  )| \
                                   (( UHC0_RW              == 1)? 0x1000 : 0  )| \
                                   (( UHC1_RW              == 1)? 0x2000 : 0  )| \
                                   (( PIU_DMA_RW           == 1)? 0x4000 : 0  )| \
                                   (( ADCDVIPLL_RW         == 1)? 0x8000 : 0  )  )

#define  ClientGruop1_in_MIU1     (((VIVALDI9_MAD         == 1)? 0x0001 : 0  )| \
                                   ((VIVALDI9_DMA         == 1)? 0x0002 : 0  )| \
                                   ((VIVALDI9_AU_DMA      == 1)? 0x0004 : 0  )| \
                                   ((MVOP_RW              == 1)? 0x0008 : 0  )| \
                                   ((GOP_W                == 1)? 0x0010 : 0  )| \
                                   ((GOP0_R               == 1)? 0x0020 : 0  )| \
                                   ((GOP1_R               == 1)? 0x0040 : 0  )| \
                                   ((GOP2_R               == 1)? 0x0080 : 0  )| \
                                   ((GOP3_R               == 1)? 0x0100 : 0  )| \
                                   ((SC_OP_R              == 1)? 0x0200 : 0  )| \
                                   ((SC_IP_MIAN_W         == 1)? 0x0400 : 0  )| \
                                   ((SC_IP_MIAN_R         == 1)? 0x0800 : 0  )| \
                                   ((VD_COMB_W            == 1)? 0x1000 : 0  )| \
                                   ((VD_COMB_R            == 1)? 0x2000 : 0  )| \
                                   ((VE_W                 == 1)? 0x4000 : 0  )| \
                                   ((VE_R                 == 1)? 0x8000 : 0  )  )


#define  ClientGruop2_in_MIU1     (((OD_W                 == 1)? 0x0001 : 0 )| \
                                   ((OD_R                 == 1)? 0x0002 : 0 )| \
                                   ((OD_LSB_W             == 1)? 0x0004 : 0 )| \
                                   ((OD_LSB_R             == 1)? 0x0008 : 0 )| \
                                   ((SC_IP_SUB_W          == 1)? 0x0010 : 0 )| \
                                   ((SC_IP_SUB_R          == 1)? 0x0020 : 0 )| \
                                   ((DISP_IPATH_NR_RW     == 1)? 0x0040 : 0 )| \
                                   ((DISP_IPATH_MR_RW     == 1)? 0x0080 : 0 )| \
                                   ((DISP_IPATH_DI_W      == 1)? 0x0100 : 0 )| \
                                   ((VD_MHEG5_ICACHE      == 1)? 0x0200 : 0 )| \
                                   ((TSP_W                == 1)? 0x0400 : 0 )| \
                                   ((TSP_R                == 1)? 0x0800 : 0 )| \
                                   ((VD_TTX_RW            == 1)? 0x1000 : 0 )| \
                                   ((VD_TTXSL_W           == 1)? 0x2000 : 0 )| \
                                   ((TSP_ORZ_W            == 1)? 0x4000 : 0 )| \
                                   ((TSP_ORZ_R            == 1)? 0x8000 : 0 ))

#define  ClientGruop3_in_MIU1     (((MVOP_64_R            == 1)? 0x0001 : 0  )| \
                                   ((M4VE_0_RW            == 1)? 0x0002 : 0  )| \
                                   ((M4VE_ME_R            == 1)? 0x0004 : 0  )| \
                                   ((M4VE_2               == 1)? 0x0008 : 0  )| \
                                   ((MVD_RW               == 1)? 0x0010 : 0  )| \
                                   ((MVD_BBU_RW           == 1)? 0x0020 : 0  )| \
                                   ((RVD_RW               == 1)? 0x0040 : 0  )| \
                                   ((RVD_BBU_RW           == 1)? 0x0080 : 0  )| \
                                   ((JPD_RW               == 1)? 0x0100 : 0  )| \
                                   ((DSCRMB_RW            == 1)? 0x0200 : 0  )| \
                                   ((STRLD_RW             == 1)? 0x0400 : 0  )| \
                                   ((FUART_RW             == 1)? 0x0800 : 0  )| \
                                   ((EMAC_RW              == 1)? 0x1000 : 0  )| \
                                   ((NAND_RW              == 1)? 0x2000 : 0  )| \
                                   ((MPIF_RW              == 1)? 0x4000 : 0  )| \
                                   ((RESERVED_G3D_RW      == 1)? 0x8000 : 0  ))
//-----------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define  ClientGruop0_in_IP      (((VIVALDI9_DSP_DECODER == 2)? 0x0002:0 )|\
                                  ((VIVALDI9_DSP_SE      == 2)? 0x0004:0 )| \
                                  ((GE_RW                == 2)? 0x0200:0 )| \
                                  ((UHC0_RW              == 2)? 0x1000:0 )| \
                                  ((UHC1_RW              == 2)? 0x2000:0 )| \
                                  ((PIU_DMA_RW           == 2)? 0x4000:0 ))


#define  ClientGruop1_in_IP     (((VIVALDI9_MAD         == 2)? 0x0001 : 0 )| \
                                 ((VIVALDI9_DMA         == 2)? 0x0002 : 0 )| \
                                 ((VIVALDI9_AU_DMA      == 2)? 0x0004 : 0 )| \
                                 ((SC_OP_R              == 2)? 0x0100 : 0 )| \
                                 ((SC_IP_MIAN_W         == 2)? 0x0200 : 0 )| \
                                 ((SC_IP_MIAN_R         == 2)? 0x0800 : 0 )  )


#define  ClientGruop2_in_IP      ((  SC_IP_SUB_W         == 2)? 0x0010 : 0) | \
                                  (  SC_IP_SUB_R         == 2)? 0x0020 : 0)

#define  ClientGruop3_in_IP      ((  DSCRMB_RW           == 2)? 0x0100 : 0 | \
                                  (  STRLD_RW            == 2)? 0x0200 : 0 | \
                                  (  EMAC_RW             == 2)? 0x1000 : 0 | \
                                  (  NAND_RW             == 2)? 0x2000 : 0 | \
                                  (  MPIF_RW             == 2)? 0x4000 : 0 )
//-----------------------------------------------------------------------------

#endif  //_MIUSEL_KRATOS_2MIU_H_


