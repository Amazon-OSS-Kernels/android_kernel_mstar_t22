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

#ifndef XC_ANALOG_REG_H
#define XC_ANALOG_REG_H


/******************************************************************************/
/*                     Macro                                                  */
/******************************************************************************/

#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif

///////////////////////////////////////////////////////
// Scaler Bank
///////////////////////////////////////////////////////

#define REG_BANK_RESET              0x00
#define REG_BANK_GOPINT             0x00
#define REG_BANK_IP1F2              0x01
#define REG_BANK_IP2F2              0x02
#define REG_BANK_IP1F1              0x03
#define REG_BANK_IP2F1              0x04
#define REG_BANK_OPM                0x05
#define REG_BANK_DNR                0x06
#define REG_BANK_OP1                0x07
#define REG_BANK_OP1PIP             0x08
#define REG_BANK_OP1HVSP            0x09
#define REG_BANK_FILM               0x0A    //T2
#define REG_BANK_ELA                0x0B
#define REG_BANK_ACE                0x0C
#define REG_BANK_HD                 0x0E
#define REG_BANK_S_VOP              0x0F
#define REG_BANK_VOP                0x10
#define REG_BANK_TCON               0x11
#define REG_BANK_SCMI               0x12  //T2
#define REG_BANK_OFFLINE            0x13
#define REG_BANK_OD                 0x16
#define REG_BANK_SRAM               0x17
#define REG_BANK_VIP                0x18
#define REG_BANK_VPS                0x19
#define REG_BANK_DLC                0x1A
#define REG_BANK_OP1PIPEXT          0x1B
#define REG_BANK_SmoothFilter       0x1C
#define REG_BANK_MFC                0x1D
#define REG_BANK_PIP                0x20
#define REG_BANK_EODI               0x21
#define REG_BANK_MADI               0x22
#define REG_BANK_HVSP               0x23
#define REG_BANK_DMS                0x26
#define REG_BANK_PRED               0x28

#define BK_SELECT_00                REG_SCALER_BASE

#define HIPART( u32x )              (((u32x) >> 16) & 0xFFFF)
#define LOPART( u32x )              ((u32x) & 0xFFFF)

#define BK_REG_L( x, y )            ((x) | (((y) << 1)))
#define BK_REG_H( x, y )            (((x) | (((y) << 1))) + 1)

/* Scaler */
#define L_BK_GOPINT(x)      BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_GOPINT(x)      BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_SWRESET(x)     BK_REG_L(REG_SCALER_BASE,x)
#define L_BK_IP1F2(x)       BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_IP1F2(x)       BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_IP2F2(x)       BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_IP2F2(x)       BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_IP1F1(x)       BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_IP1F1(x)       BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_IP2F1(x)       BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_IP2F1(x)       BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_OPM(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_OPM(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_DNR(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_DNR(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_OP1(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_OP1(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_OP1HVSP(x)     BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_OP1HVSP(x)     BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_ELA(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_ELA(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_ACE(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_ACE(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_HD(x)          BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_HD(x)          BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_S_VOP(x)       BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_S_VOP(x)       BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_VOP(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_VOP(x)         BK_REG_H(REG_SCALER_BASE,x)

#define L_BK_OFFLINE(x)     BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_OFFLINE(x)     BK_REG_H(REG_SCALER_BASE,x)

#define L_BK_SCMI(x)        BK_REG_L(REG_SCALER_BASE,x)      //SUPPORT_CHIP==T2
#define H_BK_SCMI(x)        BK_REG_H(REG_SCALER_BASE,x)      //SUPPORT_CHIP==T2
#define L_BK_OD(x)          BK_REG_L(REG_SCALER_BASE,x)      //SUPPORT_CHIP==T2
#define H_BK_OD(x)          BK_REG_H(REG_SCALER_BASE,x)      //SUPPORT_CHIP==T2

#define L_BK_OP1ZZ(x)       BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_OP1ZZ(x)       BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_VIP(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_VIP(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_VPS(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_VPS(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_DLC(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_DLC(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_OP1PIPEXT(x)   BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_OP1PIPEXT(x)   BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_SmoothFilter(x)  BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_SmoothFilter(x)  BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_MFC(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_MFC(x)         BK_REG_H(REG_SCALER_BASE,x)

#define L_BK_EODI(x)        BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_EODI(x)        BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_MADI(x)        BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_MADI(x)        BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_PIP(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_PIP(x)         BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_HVSP(x)        BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_HVSP(x)        BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_PRED(x)        BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_PRED(x)        BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_FILM(x)        BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_FILM(x)        BK_REG_H(REG_SCALER_BASE,x)
#define L_BK_DMS(x)         BK_REG_L(REG_SCALER_BASE,x)
#define H_BK_DMS(x)         BK_REG_H(REG_SCALER_BASE,x)

/* VE */
#define L_BK_VE_ENC(x)      BK_REG_L(REG_BASE_VE_ENCODER, x)
#define H_BK_VE_ENC(x)      BK_REG_H(REG_BASE_VE_ENCODER, x)
#define L_BK_VE_SRC(x)      BK_REG_L(REG_BASE_VE_SOURCE, x)
#define H_BK_VE_SRC(x)      BK_REG_H(REG_BASE_VE_SOURCE, x)


/* LPLL */
#define L_BK_LPLL(x)        BK_REG_L(REG_LPLL_BASE, x)
#define H_BK_LPLL(x)        BK_REG_H(REG_LPLL_BASE, x)

/* IP Mux */
#define L_BK_IPMUX(x)       BK_REG_L(REG_IPMUX_BASE, x)
#define H_BK_IPMUX(x)       BK_REG_H(REG_IPMUX_BASE, x)

/* Chip Top */
#define L_BK_CHIPTOP(x)     BK_REG_L(REG_CHIPTOP_BASE, x)
#define H_BK_CHIPTOP(x)     BK_REG_H(REG_CHIPTOP_BASE, x)

/* ADC */
#define L_BK_ADC_ATOP(x)    BK_REG_L(REG_ADC_ATOP_BASE, x)
#define H_BK_ADC_ATOP(x)    BK_REG_H(REG_ADC_ATOP_BASE, x)
#define L_BK_ADC_DTOP(x)    BK_REG_L(REG_ADC_DTOP_BASE, x)
#define H_BK_ADC_DTOP(x)    BK_REG_H(REG_ADC_DTOP_BASE, x)

/* MOD */
#define L_BK_MOD(x)         BK_REG_L(REG_MOD_BASE, x)
#define H_BK_MOD(x)         BK_REG_H(REG_MOD_BASE, x)

/* VD:AFEC */
#define L_BK_AFEC(x)        BK_REG_L(REG_AFEC_BASE, x)
#define H_BK_AFEC(x)        BK_REG_H(REG_AFEC_BASE, x)

//------------------------------------------------------------------------------
// Input source select

// BK_IP1F2_02 [2:0]
#define IP_INSSEL_ANALOG1   0x0000
#define IP_INSSEL_ANALOG2   0x0001
#define IP_INSSEL_ANALOG3   0x0002
#define IP_INSSEL_DVI       0x0003
#define IP_INSSEL_VIDEO     0x0004
#define IP_INSSEL_HDTV      0x0005
#define IP_INSSEL_HDMI      0x0007

#define IP_INSSEL_MASK      BITMASK(2:0)
#define IP_VDOSEL_MASK      BITMASK(1:0)

typedef enum
{
    IP_CCIR656_A,
    IP_MST_VD_A,
    IP_CCIR601,
    IP_MST_VD_B, // Don't use; RD's suggestion.
    IP_CCIR656_B=0x20
} VDOSEL;

typedef enum
{
    VE_IPMUX_ADC_A      = 0,            ///< ADC A
    VE_IPMUX_HDMI_DVI   = 1,            ///< DVI
    VE_IPMUX_VD         = 2,            ///< VD
    VE_IPMUX_MVOP       = 3,            ///< MPEG/DC0
    VE_IPMUX_SC_IP1     = 4,            ///< Scaler IP1 output
    VE_IPMUX_EXT_VD     = 5,            ///< External VD
    VE_IPMUX_ADC_B      = 6,            ///< ADC B
} VE_IPMUX_TYPE;

typedef enum
{
    AUTO_DETECT     =0x00,
    HV_SEPARATED    =0x01,
    COMPOSITE_SYNC  =0x02,
    SYNC_ON_GREEN   =0x03
}STYPE;

typedef enum
{
    CSYNC   = 0,
    SOG     = 1
}COMP;

//------------------------------------------------------------------------------
// ADC

#define ADC_AMUXA_MASK      BITMASK(1:0)
#define ADC_YMUX_MASK       BITMASK(3:0)
#define ADC_CMUX_MASK       BITMASK(7:4)


//------------------------------------------------------------------------------
// MUX

typedef enum
{
    ADC_RGB1,
    ADC_RGB2,
    ADC_RGB3,
}AMUX_SEL;

typedef enum // For PC/YPbPr input mux
{
    ANALOG_RGB0 = ADC_RGB1,
    ANALOG_RGB1 = ADC_RGB2,
    ANALOG_RGB2 = ADC_RGB3,
    ANALOG_RGB_DUMMY,
}ANALOG_RGB;

typedef enum
{
    MSVD_YMUX_CVBS0,
    MSVD_YMUX_CVBS1,
    MSVD_YMUX_CVBS2,
    MSVD_YMUX_CVBS3,

    MSVD_YMUX_Y0,
    MSVD_YMUX_Y1,
    MSVD_YMUX_C0,
    MSVD_YMUX_C1,

    MSVD_YMUX_SOG0 = 8,
    MSVD_YMUX_SOG1,
    MSVD_YMUX_SOG2,

    MSVD_YMUX_G0 = 11,
    MSVD_YMUX_G1 = 12,
    MSVD_YMUX_G2 = 13,
    MSVD_YMUX_CVBS4 = MSVD_YMUX_Y0,
    MSVD_YMUX_CVBS5 = MSVD_YMUX_Y1,
    MSVD_YMUX_CVBS6 = MSVD_YMUX_C0,
    MSVD_YMUX_CVBS7 = MSVD_YMUX_C1,
    MSVD_YMUX_NONE = 0xF,

    MSVD_YMUX_DUMMY,
}MS_VD_YMUX;

typedef enum
{
    MSVD_CMUX_CVBS0,
    MSVD_CMUX_CVBS1,
    MSVD_CMUX_CVBS2,
    MSVD_CMUX_CVBS3,

    MSVD_CMUX_Y0 = 4,
    MSVD_CMUX_Y1,
    MSVD_CMUX_C0,
    MSVD_CMUX_C1,

    MSVD_CMUX_SOG0 = 8,
    MSVD_CMUX_SOG1,
    MSVD_CMUX_SOG2,

    MSVD_CMUX_R0 = 11,
    MSVD_CMUX_R1 = 12,
    MSVD_CMUX_R2 = 13,
    MSVD_CMUX_CVBS4 = MSVD_CMUX_Y0,
    MSVD_CMUX_CVBS5 = MSVD_CMUX_C0,
    MSVD_CMUX_CVBS6 = MSVD_CMUX_Y1,
    MSVD_CMUX_CVBS7 = MSVD_CMUX_C1,
    MSVD_CMUX_NONE = 0xF,

    MSVD_CMUX_DUMMY,
}MS_VD_CMUX;

#endif // ANALOG_REG_H
