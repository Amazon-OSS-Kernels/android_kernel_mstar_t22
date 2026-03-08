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

#include <stdio.h>

#include "datatype.h"
#include "drvRIU.h"
#include "Board.h"
#include "c_riubase.h"
#include "hwreg_M7632.h"
#include "chip/bond.h"

#if (ENABLE_ONEBIN_ENABLE == 1)
#define PREBUFFERSIZE                  512      //array size
#define BUFFERSIZE                     2048      //array size
#endif

#define GPIO_NONE                   0       // Not GPIO pin (default)
#define GPIO_IN                     1       // GPI
#define GPIO_OUT_LOW                2       // GPO output low
#define GPIO_OUT_HIGH               3       // GPO output high

#if defined(ARM_CHAKRA) || defined(MIPS_CHAKRA) || defined(MSOS_TYPE_LINUX)
#define _MapBase_nonPM_M7632       (RIU_MAP + 0x00200000)
#define _MapBase_PM_M7632          RIU_MAP
#else
#define _MapBase_nonPM_M7632        0xa0200000
#define _MapBase_PM_M7632           0xa0000000
#endif


#define _MEMMAP_REGBANK_00_         _RVM1(0x0000, 0x00, 0xFF)
#define _MEMMAP_REGBANK_10_         _RVM1(0x0000, 0x10, 0xFF)
#define _MEMMAP_REGBANK_11_         _RVM1(0x0000, 0x11, 0xFF)
#define _MEMMAP_REGBANK_32_         _RVM1(0x0000, 0x32, 0xFF)

#if (ENABLE_ONEBIN_ENABLE == 1)
//U8 PRERomBuffer[PREBUFFERSIZE] ={0};
U8 MIUBuffer[BUFFERSIZE] ={0};
U8 MIUSTRBuffer[BUFFERSIZE] ={0};
U8 RamBuffer[BUFFERSIZE] ={0};
#endif


const U8 padInitTbl_PreInit[] =
{
    0xFF, 0xFF, 0xFF, 0xFF,         // magic code for ISP_Tool

    // programable device number
    // spi flash count
    0,
    0x00,                           // nor
    0x00,                           // nand
    0x00,                           // reserved
    0x00,                           // reserved
    0x00,                           // reserved

//---------------------------------------------------------------------
// GPIO Configuartion
//---------------------------------------------------------------------
    _MEMMAP_REGBANK_00_,

//---------------------------------------------------------------------
// Pad Configuartion
//---------------------------------------------------------------------

    _MEMMAP_REGBANK_10_,

// SDR/DDR  1.8V/3.3V, DQS at PAD_EMMC_IO8
#ifdef PADS_NAND_MODE
#if (PADS_NAND_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_NAND_MODE_MODE1 ((PADS_NAND_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_NAND_MODE == CONFIG_PADMUX_MODE2) ? (0x01 << 1) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2964, CONFIG_NAND_MODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// NAND CS1
#ifdef PADS_NAND_CS1_EN
#if (PADS_NAND_CS1_EN != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_NAND_CS1_EN_MODE1 ((PADS_NAND_CS1_EN == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2964, CONFIG_NAND_CS1_EN_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EMMC RST
#ifdef PADS_EMMC_RSTN_EN_PM
#if (PADS_EMMC_RSTN_EN_PM != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EMMC_RSTN_EN_PM_MODE1 ((PADS_EMMC_RSTN_EN_PM == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_00_,
    _RVM1(0x0e62, CONFIG_EMMC_RSTN_EN_PM_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EMMC RST
#ifdef PADS_EMMC_RSTN_EN
#if (PADS_EMMC_RSTN_EN != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EMMC_RSTN_EN_MODE1 ((PADS_EMMC_RSTN_EN == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2960, CONFIG_EMMC_RSTN_EN_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EMMC MODE
#ifdef PADS_EMMC_CONFIG
#if (PADS_EMMC_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EMMC_CONFIG_MODE1 ((PADS_EMMC_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2960, CONFIG_EMMC_CONFIG_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

//---------------------------------------------------------------------
//Please copy related configuration of storage devices to here


//=============================================================================
    //reg_allpad_in
    _RVM1(0x1ea1, 0, BIT7),     //reg[101ea1]#7 = 0b
    _END_OF_TBL_,
};

const U8 padInitTbl[] =
{
    0x39, 0xB6, 0x5B, 0x53,     // magic code for ISP_Tool

    // programable device number
    // spi flash count
    1 + (PIN_SPI_CZ1 != 0) + (PIN_SPI_CZ2 != 0) + (PIN_SPI_CZ3 != 0),
    0x00,                       // nor
    0x00,                       // nand
    0x00,                       // reserved
    0x00,                       // reserved
    0x00,                       // reserved

//---------------------------------------------------------------------
// GPIO Configuartion
//---------------------------------------------------------------------

_MEMMAP_REGBANK_00_,
    #if(PAD_DDCA_CK_IS_GPIO != GPIO_NONE)
        #define PAD_DDCA_CK_OEN (PAD_DDCA_CK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DDCA_CK_OUT (PAD_DDCA_CK_IS_GPIO == GPIO_OUT_HIGH ? BIT2: 0)
        _RVM1(0x0494, PAD_DDCA_CK_OUT, BIT2),
        _RVM1(0x0494, PAD_DDCA_CK_OEN, BIT1),
        //reg_gpio2a0_en
        _RVM1(0x0494, BIT7, BIT7),   //reg[0494]#7 = 1b
    #endif

    #if(PAD_DDCA_DA_IS_GPIO != GPIO_NONE)
        #define PAD_DDCA_DA_OEN (PAD_DDCA_DA_IS_GPIO == GPIO_IN ? BIT5: 0)
        #define PAD_DDCA_DA_OUT (PAD_DDCA_DA_IS_GPIO == GPIO_OUT_HIGH ? BIT6: 0)
        _RVM1(0x0494, PAD_DDCA_DA_OUT, BIT6),
        _RVM1(0x0494, PAD_DDCA_DA_OEN, BIT5),
        //reg_gpio2a0_en
        _RVM1(0x0494, BIT7, BIT7),   //reg[0494]#7 = 1b
    #endif

    #if(PAD_IRIN_IS_GPIO != GPIO_NONE)
        #define PAD_IRIN_OEN (PAD_IRIN_IS_GPIO == GPIO_IN ? BIT1'b1: 0)
        #define PAD_IRIN_OUT (PAD_IRIN_IS_GPIO == GPIO_OUT_HIGH ? BITNA: 0)
        _RVM1(0x, PAD_IRIN_OUT, BITNA),
        _RVM1(0xb, PAD_IRIN_OEN, BIT1'b1),
        //reg_ir_is_gpio
        _RVM1(0x0e38, BIT4, BIT4),   //reg[0e38]#4 = 1b
    #endif

    #if(PAD_PWM_PM_IS_GPIO != GPIO_NONE)
        #define PAD_PWM_PM_OEN (PAD_PWM_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_PWM_PM_OUT (PAD_PWM_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f28, PAD_PWM_PM_OUT, BIT1),
        _RVM1(0x0f28, PAD_PWM_PM_OEN, BIT0),
        //reg_pwm_pm_is_gpio
        _RVM1(0x0e38, BIT5, BIT5),   //reg[0e38]#5 = 1b
    #endif

    #if(PAD_CEC0_IS_GPIO != GPIO_NONE)
        #define PAD_CEC0_OEN (PAD_CEC0_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_CEC0_OUT (PAD_CEC0_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f2a, PAD_CEC0_OUT, BIT1),
        _RVM1(0x0f2a, PAD_CEC0_OEN, BIT0),
        //reg_cec_is_gpio
        _RVM1(0x0e38, BIT6, BIT6),   //reg[0e38]#6 = 1b
    #endif

    #if(PAD_GPIO0_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO0_PM_OEN (PAD_GPIO0_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO0_PM_OUT (PAD_GPIO0_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f00, PAD_GPIO0_PM_OUT, BIT1),
        _RVM1(0x0f00, PAD_GPIO0_PM_OEN, BIT0),
        //reg_sd_cdz_mode
        _RVM1(0x0e4f, 0, BIT6),   //reg[0e4f]#6 = 0b
        //reg_spi_pad_sel
        _RVM1(0x0eed, 0, BIT2),   //reg[0eed]#2 = 0b
        //reg_ld_spi1_config
        _RVM1(0x0ee5, 0, BIT5),   //reg[0ee5]#5 = 0b
        //reg_ld_spi3_config
        _RVM1(0x0ee5, 0, BIT7),   //reg[0ee5]#7 = 0b
        //reg_gpio_is_pwm1
        _RVM1(0x0eed, 0, BIT4 | BIT3),   //reg[0eed]#4~#3 = 00b
    #endif

    #if(PAD_GPIO1_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO1_PM_OEN (PAD_GPIO1_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO1_PM_OUT (PAD_GPIO1_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f02, PAD_GPIO1_PM_OUT, BIT1),
        _RVM1(0x0f02, PAD_GPIO1_PM_OEN, BIT0),
        //reg_spi_pad_sel
        _RVM1(0x0eed, 0, BIT2),   //reg[0eed]#2 = 0b
        //reg_ld_spi2_config
        _RVM1(0x0ee5, 0, BIT6),   //reg[0ee5]#6 = 0b
        //reg_ld_spi3_config
        _RVM1(0x0ee5, 0, BIT7),   //reg[0ee5]#7 = 0b
        //reg_uart_is_gpio_4[3:0]
        _RVM1(0x0eec, 0, BIT7 | BIT6),   //reg[0eec]#7~#6 = 00b
        _RVM1(0x0eed, 0, BIT1 | BIT0),   //reg[0eed]#1~#0 = 00b
        //reg_uart_is_gpio_3[3:0]
        _RVM1(0x0eec, 0, 0x3C),   //reg[0eec]#5~#2 = 0000b
        //reg_uart_is_gpio[3:0]
        _RVM1(0x0e6b, 0, 0x0F),   //reg[0e6b]#3~#0 = 0000b
    #endif

    #if(PAD_GPIO2_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO2_PM_OEN (PAD_GPIO2_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO2_PM_OUT (PAD_GPIO2_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f04, PAD_GPIO2_PM_OUT, BIT1),
        _RVM1(0x0f04, PAD_GPIO2_PM_OEN, BIT0),
        //reg_gpio_is_clk_rtc
        _RVM1(0x0ef4, 0, BIT1 | BIT0),   //reg[0ef4]#1~#0 = 00b
        //reg_gpio_is_clk_xtal
        _RVM1(0x0ef4, 0, BIT5 | BIT4),   //reg[0ef4]#5~#4 = 00b
    #endif

    #if(PAD_USB_CTRL_IS_GPIO != GPIO_NONE)
        #define PAD_USB_CTRL_OEN (PAD_USB_CTRL_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_USB_CTRL_OUT (PAD_USB_CTRL_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f06, PAD_USB_CTRL_OUT, BIT1),
        _RVM1(0x0f06, PAD_USB_CTRL_OEN, BIT0),
        //reg_uart_is_gpio_4[3:0]
        _RVM1(0x0eec, 0, BIT7 | BIT6),   //reg[0eec]#7~#6 = 00b
        _RVM1(0x0eed, 0, BIT1 | BIT0),   //reg[0eed]#1~#0 = 00b
        //reg_uart_is_gpio_3[3:0]
        _RVM1(0x0eec, 0, 0x3C),   //reg[0eec]#5~#2 = 0000b
        //reg_uart_is_gpio_2[1:0]
        _RVM1(0x0eec, 0, BIT1 | BIT0),   //reg[0eec]#1~#0 = 00b
    #endif

    #if(PAD_GPIO5_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO5_PM_OEN (PAD_GPIO5_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO5_PM_OUT (PAD_GPIO5_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f0a, PAD_GPIO5_PM_OUT, BIT1),
        _RVM1(0x0f0a, PAD_GPIO5_PM_OEN, BIT0),
        //reg_spi_pad_sel
        _RVM1(0x0eed, 0, BIT2),   //reg[0eed]#2 = 0b
        //reg_ld_spi1_config
        _RVM1(0x0ee5, 0, BIT5),   //reg[0ee5]#5 = 0b
        //reg_ld_spi3_config
        _RVM1(0x0ee5, 0, BIT7),   //reg[0ee5]#7 = 0b
        //reg_uart_is_gpio_4[3:0]
        _RVM1(0x0eec, 0, BIT7 | BIT6),   //reg[0eec]#7~#6 = 00b
        _RVM1(0x0eed, 0, BIT1 | BIT0),   //reg[0eed]#1~#0 = 00b
        //reg_uart_is_gpio_3[3:0]
        _RVM1(0x0eec, 0, 0x3C),   //reg[0eec]#5~#2 = 0000b
        //reg_uart_is_gpio[3:0]
        _RVM1(0x0e6b, 0, 0x0F),   //reg[0e6b]#3~#0 = 0000b
    #endif

    #if(PAD_GPIO6_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO6_PM_OEN (PAD_GPIO6_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO6_PM_OUT (PAD_GPIO6_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f0c, PAD_GPIO6_PM_OUT, BIT1),
        _RVM1(0x0f0c, PAD_GPIO6_PM_OEN, BIT0),
        //reg_spicsz1_gpio
        _RVM1(0x0e6a, BIT2, BIT2),   //reg[0e6a]#2 = 1b
        //reg_ld_spi2_config
        _RVM1(0x0ee5, 0, BIT6),   //reg[0ee5]#6 = 0b
        //reg_ld_spi3_config
        _RVM1(0x0ee5, 0, BIT7),   //reg[0ee5]#7 = 0b
        //reg_gpio_is_pwm1
        _RVM1(0x0eed, 0, BIT4 | BIT3),   //reg[0eed]#4~#3 = 00b
    #endif

    #if(PAD_GPIO7_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO7_PM_OEN (PAD_GPIO7_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO7_PM_OUT (PAD_GPIO7_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f0e, PAD_GPIO7_PM_OUT, BIT1),
        _RVM1(0x0f0e, PAD_GPIO7_PM_OEN, BIT0),
        //reg_miic_mode
        _RVM1(0x0ec9, 0, BIT7 | BIT6),   //reg[0ec9]#7~#6 = 00b
    #endif

    #if(PAD_GPIO8_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO8_PM_OEN (PAD_GPIO8_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO8_PM_OUT (PAD_GPIO8_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f10, PAD_GPIO8_PM_OUT, BIT1),
        _RVM1(0x0f10, PAD_GPIO8_PM_OEN, BIT0),
        //reg_uart_is_gpio_4[3:0]
        _RVM1(0x0eec, 0, BIT7 | BIT6),   //reg[0eec]#7~#6 = 00b
        _RVM1(0x0eed, 0, BIT1 | BIT0),   //reg[0eed]#1~#0 = 00b
        //reg_miic_mode
        _RVM1(0x0ec9, 0, BIT7 | BIT6),   //reg[0ec9]#7~#6 = 00b
        //reg_uart_is_gpio_3[3:0]
        _RVM1(0x0eec, 0, 0x3C),   //reg[0eec]#5~#2 = 0000b
        //reg_uart_is_gpio_2[1:0]
        _RVM1(0x0eec, 0, BIT1 | BIT0),   //reg[0eec]#1~#0 = 00b
        //reg_uart_is_gpio[3:0]
        _RVM1(0x0e6b, 0, 0x0F),   //reg[0e6b]#3~#0 = 0000b
    #endif

    #if(PAD_GPIO9_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO9_PM_OEN (PAD_GPIO9_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO9_PM_OUT (PAD_GPIO9_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f12, PAD_GPIO9_PM_OUT, BIT1),
        _RVM1(0x0f12, PAD_GPIO9_PM_OEN, BIT0),
        //reg_gpio_is_clk_rtc
        _RVM1(0x0ef4, 0, BIT1 | BIT0),   //reg[0ef4]#1~#0 = 00b
        //reg_gpio_is_clk_xtal
        _RVM1(0x0ef4, 0, BIT5 | BIT4),   //reg[0ef4]#5~#4 = 00b
    #endif

    #if(PAD_GPIO10_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO10_PM_OEN (PAD_GPIO10_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO10_PM_OUT (PAD_GPIO10_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f14, PAD_GPIO10_PM_OUT, BIT1),
        _RVM1(0x0f14, PAD_GPIO10_PM_OEN, BIT0),
        //reg_gpio_is_clk_rtc
        _RVM1(0x0ef4, 0, BIT1 | BIT0),   //reg[0ef4]#1~#0 = 00b
        //reg_gpio_is_clk_xtal
        _RVM1(0x0ef4, 0, BIT5 | BIT4),   //reg[0ef4]#5~#4 = 00b
    #endif

    #if(PAD_GPIO11_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO11_PM_OEN (PAD_GPIO11_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO11_PM_OUT (PAD_GPIO11_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f16, PAD_GPIO11_PM_OUT, BIT1),
        _RVM1(0x0f16, PAD_GPIO11_PM_OEN, BIT0),
        //reg_uart_is_gpio_4[3:0]
        _RVM1(0x0eec, 0, BIT7 | BIT6),   //reg[0eec]#7~#6 = 00b
        _RVM1(0x0eed, 0, BIT1 | BIT0),   //reg[0eed]#1~#0 = 00b
        //reg_uart_is_gpio_3[3:0]
        _RVM1(0x0eec, 0, 0x3C),   //reg[0eec]#5~#2 = 0000b
        //reg_uart_is_gpio_1[1:0]
        _RVM1(0x0e6b, 0, BIT7 | BIT6),   //reg[0e6b]#7~#6 = 00b
    #endif

    #if(PAD_GPIO12_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO12_PM_OEN (PAD_GPIO12_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO12_PM_OUT (PAD_GPIO12_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f18, PAD_GPIO12_PM_OUT, BIT1),
        _RVM1(0x0f18, PAD_GPIO12_PM_OEN, BIT0),
        //reg_uart_is_gpio_4[3:0]
        _RVM1(0x0eec, 0, BIT7 | BIT6),   //reg[0eec]#7~#6 = 00b
        _RVM1(0x0eed, 0, BIT1 | BIT0),   //reg[0eed]#1~#0 = 00b
        //reg_uart_is_gpio_3[3:0]
        _RVM1(0x0eec, 0, 0x3C),   //reg[0eec]#5~#2 = 0000b
        //reg_uart_is_gpio_1[1:0]
        _RVM1(0x0e6b, 0, BIT7 | BIT6),   //reg[0e6b]#7~#6 = 00b
    #endif

    #if(PAD_GPIO13_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO13_PM_OEN (PAD_GPIO13_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO13_PM_OUT (PAD_GPIO13_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f1a, PAD_GPIO13_PM_OUT, BIT1),
        _RVM1(0x0f1a, PAD_GPIO13_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO14_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO14_PM_OEN (PAD_GPIO14_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO14_PM_OUT (PAD_GPIO14_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f1c, PAD_GPIO14_PM_OUT, BIT1),
        _RVM1(0x0f1c, PAD_GPIO14_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO15_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO15_PM_OEN (PAD_GPIO15_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO15_PM_OUT (PAD_GPIO15_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f1e, PAD_GPIO15_PM_OUT, BIT1),
        _RVM1(0x0f1e, PAD_GPIO15_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO16_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO16_PM_OEN (PAD_GPIO16_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO16_PM_OUT (PAD_GPIO16_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f20, PAD_GPIO16_PM_OUT, BIT1),
        _RVM1(0x0f20, PAD_GPIO16_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO17_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO17_PM_OEN (PAD_GPIO17_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO17_PM_OUT (PAD_GPIO17_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f48, PAD_GPIO17_PM_OUT, BIT1),
        _RVM1(0x0f48, PAD_GPIO17_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO18_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO18_PM_OEN (PAD_GPIO18_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO18_PM_OUT (PAD_GPIO18_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f4a, PAD_GPIO18_PM_OUT, BIT1),
        _RVM1(0x0f4a, PAD_GPIO18_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO19_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO19_PM_OEN (PAD_GPIO19_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO19_PM_OUT (PAD_GPIO19_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f4c, PAD_GPIO19_PM_OUT, BIT1),
        _RVM1(0x0f4c, PAD_GPIO19_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO20_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO20_PM_OEN (PAD_GPIO20_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO20_PM_OUT (PAD_GPIO20_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f4e, PAD_GPIO20_PM_OUT, BIT1),
        _RVM1(0x0f4e, PAD_GPIO20_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO21_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO21_PM_OEN (PAD_GPIO21_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO21_PM_OUT (PAD_GPIO21_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f50, PAD_GPIO21_PM_OUT, BIT1),
        _RVM1(0x0f50, PAD_GPIO21_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO22_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO22_PM_OEN (PAD_GPIO22_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO22_PM_OUT (PAD_GPIO22_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f52, PAD_GPIO22_PM_OUT, BIT1),
        _RVM1(0x0f52, PAD_GPIO22_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO23_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO23_PM_OEN (PAD_GPIO23_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO23_PM_OUT (PAD_GPIO23_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f54, PAD_GPIO23_PM_OUT, BIT1),
        _RVM1(0x0f54, PAD_GPIO23_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO24_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO24_PM_OEN (PAD_GPIO24_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO24_PM_OUT (PAD_GPIO24_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f56, PAD_GPIO24_PM_OUT, BIT1),
        _RVM1(0x0f56, PAD_GPIO24_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO25_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO25_PM_OEN (PAD_GPIO25_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO25_PM_OUT (PAD_GPIO25_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f58, PAD_GPIO25_PM_OUT, BIT1),
        _RVM1(0x0f58, PAD_GPIO25_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO26_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO26_PM_OEN (PAD_GPIO26_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO26_PM_OUT (PAD_GPIO26_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f5a, PAD_GPIO26_PM_OUT, BIT1),
        _RVM1(0x0f5a, PAD_GPIO26_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO27_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO27_PM_OEN (PAD_GPIO27_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO27_PM_OUT (PAD_GPIO27_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f5c, PAD_GPIO27_PM_OUT, BIT1),
        _RVM1(0x0f5c, PAD_GPIO27_PM_OEN, BIT0),
    #endif

    #if(PAD_GPIO28_PM_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO28_PM_OEN (PAD_GPIO28_PM_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_GPIO28_PM_OUT (PAD_GPIO28_PM_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f5e, PAD_GPIO28_PM_OUT, BIT1),
        _RVM1(0x0f5e, PAD_GPIO28_PM_OEN, BIT0),
    #endif

    #if(PAD_DDCDA_CK_IS_GPIO != GPIO_NONE)
        #define PAD_DDCDA_CK_OEN (PAD_DDCDA_CK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DDCDA_CK_OUT (PAD_DDCDA_CK_IS_GPIO == GPIO_OUT_HIGH ? BIT2: 0)
        _RVM1(0x0496, PAD_DDCDA_CK_OUT, BIT2),
        _RVM1(0x0496, PAD_DDCDA_CK_OEN, BIT1),
        //reg_ej_mode[2:0]
        _RVM1(0x2ec4, 0, 0x07),   //reg[2ec4]#2~#0 = 000b
        //reg_gpio2do_en
        _RVM1(0x0496, BIT7, BIT7),   //reg[0496]#7 = 1b
    #endif

    #if(PAD_DDCDA_DA_IS_GPIO != GPIO_NONE)
        #define PAD_DDCDA_DA_OEN (PAD_DDCDA_DA_IS_GPIO == GPIO_IN ? BIT5: 0)
        #define PAD_DDCDA_DA_OUT (PAD_DDCDA_DA_IS_GPIO == GPIO_OUT_HIGH ? BIT6: 0)
        _RVM1(0x0496, PAD_DDCDA_DA_OUT, BIT6),
        _RVM1(0x0496, PAD_DDCDA_DA_OEN, BIT5),
        //reg_ej_mode[2:0]
        _RVM1(0x2ec4, 0, 0x07),   //reg[2ec4]#2~#0 = 000b
        //reg_gpio2do_en
        _RVM1(0x0496, BIT7, BIT7),   //reg[0496]#7 = 1b
    #endif

    #if(PAD_DDCDB_CK_IS_GPIO != GPIO_NONE)
        #define PAD_DDCDB_CK_OEN (PAD_DDCDB_CK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DDCDB_CK_OUT (PAD_DDCDB_CK_IS_GPIO == GPIO_OUT_HIGH ? BIT2: 0)
        _RVM1(0x0497, PAD_DDCDB_CK_OUT, BIT2),
        _RVM1(0x0497, PAD_DDCDB_CK_OEN, BIT1),
        //reg_ej_mode[2:0]
        _RVM1(0x2ec4, 0, 0x07),   //reg[2ec4]#2~#0 = 000b
        //reg_gpio2d1_en
        _RVM1(0x0497, BIT7, BIT7),   //reg[0497]#7 = 1b
    #endif

    #if(PAD_DDCDB_DA_IS_GPIO != GPIO_NONE)
        #define PAD_DDCDB_DA_OEN (PAD_DDCDB_DA_IS_GPIO == GPIO_IN ? BIT5: 0)
        #define PAD_DDCDB_DA_OUT (PAD_DDCDB_DA_IS_GPIO == GPIO_OUT_HIGH ? BIT6: 0)
        _RVM1(0x0497, PAD_DDCDB_DA_OUT, BIT6),
        _RVM1(0x0497, PAD_DDCDB_DA_OEN, BIT5),
        //reg_ej_mode[2:0]
        _RVM1(0x2ec4, 0, 0x07),   //reg[2ec4]#2~#0 = 000b
        //reg_gpio2d1_en
        _RVM1(0x0497, BIT7, BIT7),   //reg[0497]#7 = 1b
    #endif

    #if(PAD_DDCDC_CK_IS_GPIO != GPIO_NONE)
        #define PAD_DDCDC_CK_OEN (PAD_DDCDC_CK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DDCDC_CK_OUT (PAD_DDCDC_CK_IS_GPIO == GPIO_OUT_HIGH ? BIT2: 0)
        _RVM1(0x0498, PAD_DDCDC_CK_OUT, BIT2),
        _RVM1(0x0498, PAD_DDCDC_CK_OEN, BIT1),
        //reg_ej_mode[2:0]
        _RVM1(0x2ec4, 0, 0x07),   //reg[2ec4]#2~#0 = 000b
        //reg_gpio2d2_en
        _RVM1(0x0498, BIT7, BIT7),   //reg[0498]#7 = 1b
    #endif

    #if(PAD_DDCDC_DA_IS_GPIO != GPIO_NONE)
        #define PAD_DDCDC_DA_OEN (PAD_DDCDC_DA_IS_GPIO == GPIO_IN ? BIT5: 0)
        #define PAD_DDCDC_DA_OUT (PAD_DDCDC_DA_IS_GPIO == GPIO_OUT_HIGH ? BIT6: 0)
        _RVM1(0x0498, PAD_DDCDC_DA_OUT, BIT6),
        _RVM1(0x0498, PAD_DDCDC_DA_OEN, BIT5),
        //reg_ej_mode[2:0]
        _RVM1(0x2ec4, 0, 0x07),   //reg[2ec4]#2~#0 = 000b
        //reg_gpio2d2_en
        _RVM1(0x0498, BIT7, BIT7),   //reg[0498]#7 = 1b
    #endif

    #if(PAD_DDCDD_CK_IS_GPIO != GPIO_NONE)
        #define PAD_DDCDD_CK_OEN (PAD_DDCDD_CK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DDCDD_CK_OUT (PAD_DDCDD_CK_IS_GPIO == GPIO_OUT_HIGH ? BIT2: 0)
        _RVM1(0x0499, PAD_DDCDD_CK_OUT, BIT2),
        _RVM1(0x0499, PAD_DDCDD_CK_OEN, BIT1),
        //reg_ej_mode[2:0]
        _RVM1(0x2ec4, 0, 0x07),   //reg[2ec4]#2~#0 = 000b
        //reg_gpio2d3_en
        _RVM1(0x0499, BIT7, BIT7),   //Reg[0499]#7 = 1b
    #endif

    #if(PAD_DDCDD_DA_IS_GPIO != GPIO_NONE)
        #define PAD_DDCDD_DA_OEN (PAD_DDCDD_DA_IS_GPIO == GPIO_IN ? BIT5: 0)
        #define PAD_DDCDD_DA_OUT (PAD_DDCDD_DA_IS_GPIO == GPIO_OUT_HIGH ? BIT6: 0)
        _RVM1(0x0499, PAD_DDCDD_DA_OUT, BIT6),
        _RVM1(0x0499, PAD_DDCDD_DA_OEN, BIT5),
        //reg_ej_mode[2:0]
        _RVM1(0x2ec4, 0, 0x07),   //reg[2ec4]#2~#0 = 000b
        //reg_gpio2d3_en
        _RVM1(0x0499, BIT7, BIT7),   //Reg[0499]#7 = 1b
    #endif

    #if(PAD_SAR0_IS_GPIO != GPIO_NONE)
        #define PAD_SAR0_OEN (PAD_SAR0_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_SAR0_OUT (PAD_SAR0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
        _RVM1(0x1424, PAD_SAR0_OUT, BIT0),
        _RVM1(0x1423, PAD_SAR0_OEN, BIT0),
        //reg_sar_aisel[0]
        _RVM1(0x1422, 0, BIT0),   //reg[1422]#0 = 0b
    #endif

    #if(PAD_SAR1_IS_GPIO != GPIO_NONE)
        #define PAD_SAR1_OEN (PAD_SAR1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_SAR1_OUT (PAD_SAR1_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x1424, PAD_SAR1_OUT, BIT1),
        _RVM1(0x1423, PAD_SAR1_OEN, BIT1),
        //reg_sar_aisel[1]
        _RVM1(0x1422, 0, BIT1),   //reg[1422]#1 = 0b
    #endif

    #if(PAD_SAR2_IS_GPIO != GPIO_NONE)
        #define PAD_SAR2_OEN (PAD_SAR2_IS_GPIO == GPIO_IN ? BIT2: 0)
        #define PAD_SAR2_OUT (PAD_SAR2_IS_GPIO == GPIO_OUT_HIGH ? BIT2: 0)
        _RVM1(0x1424, PAD_SAR2_OUT, BIT2),
        _RVM1(0x1423, PAD_SAR2_OEN, BIT2),
        //reg_sar_aisel[2]
        _RVM1(0x1422, 0, BIT2),   //reg[1422]#2 = 0b
    #endif

    #if(PAD_SAR3_IS_GPIO != GPIO_NONE)
        #define PAD_SAR3_OEN (PAD_SAR3_IS_GPIO == GPIO_IN ? BIT3: 0)
        #define PAD_SAR3_OUT (PAD_SAR3_IS_GPIO == GPIO_OUT_HIGH ? BIT3: 0)
        _RVM1(0x1424, PAD_SAR3_OUT, BIT3),
        _RVM1(0x1423, PAD_SAR3_OEN, BIT3),
        //reg_sar_aisel[3]
        _RVM1(0x1422, 0, BIT3),   //reg[1422]#3 = 0b
    #endif

    #if(PAD_SAR4_IS_GPIO != GPIO_NONE)
        #define PAD_SAR4_OEN (PAD_SAR4_IS_GPIO == GPIO_IN ? BIT4: 0)
        #define PAD_SAR4_OUT (PAD_SAR4_IS_GPIO == GPIO_OUT_HIGH ? BIT4: 0)
        _RVM1(0x1424, PAD_SAR4_OUT, BIT4),
        _RVM1(0x1423, PAD_SAR4_OEN, BIT4),
        //reg_sar_aisel[4]
        _RVM1(0x1422, 0, BIT4),   //reg[1422]#4 = 0b
    #endif

    #if(PAD_VPLUGIN_IS_GPIO != GPIO_NONE)
        #define PAD_VPLUGIN_OEN (PAD_VPLUGIN_IS_GPIO == GPIO_IN ? BIT5: 0)
        #define PAD_VPLUGIN_OUT (PAD_VPLUGIN_IS_GPIO == GPIO_OUT_HIGH ? BIT5: 0)
        _RVM1(0x1424, PAD_VPLUGIN_OUT, BIT5),
        _RVM1(0x1423, PAD_VPLUGIN_OEN, BIT5),
        //reg_sar_aisel[5]
        _RVM1(0x1422, 0, BIT5),   //reg[1422]#5 = 0b
    #endif

    #if(PAD_SAR6_IS_GPIO != GPIO_NONE)
        #define PAD_SAR6_OEN (PAD_SAR6_IS_GPIO == GPIO_IN ? BIT6: 0)
        #define PAD_SAR6_OUT (PAD_SAR6_IS_GPIO == GPIO_OUT_HIGH ? BIT6: 0)
        _RVM1(0x1424, PAD_SAR6_OUT, BIT6),
        _RVM1(0x1423, PAD_SAR6_OEN, BIT6),
        //reg_sar_aisel[6]
        _RVM1(0x1422, 0, BIT6),   //reg[1422]#6 = 0b
    #endif

    #if(PAD_SAR7_IS_GPIO != GPIO_NONE)
        #define PAD_SAR7_OEN (PAD_SAR7_IS_GPIO == GPIO_IN ? BIT7: 0)
        #define PAD_SAR7_OUT (PAD_SAR7_IS_GPIO == GPIO_OUT_HIGH ? BIT7: 0)
        _RVM1(0x1424, PAD_SAR7_OUT, BIT7),
        _RVM1(0x1423, PAD_SAR7_OEN, BIT7),
        //reg_sar_aisel[7]
        _RVM1(0x1422, 0, BIT7),   //reg[1422]#7 = 0b
    #endif

    #if(PAD_VID0_IS_GPIO != GPIO_NONE)
        #define PAD_VID0_OEN (PAD_VID0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_VID0_OUT (PAD_VID0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
        _RVM1(0x2e84, PAD_VID0_OUT, BIT0),
        _RVM1(0x2e84, PAD_VID0_OEN, BIT1),
    #endif

    #if(PAD_VID1_IS_GPIO != GPIO_NONE)
        #define PAD_VID1_OEN (PAD_VID1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_VID1_OUT (PAD_VID1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
        _RVM1(0x2e85, PAD_VID1_OUT, BIT0),
        _RVM1(0x2e85, PAD_VID1_OEN, BIT1),
    #endif

    #if(PAD_VID2_IS_GPIO != GPIO_NONE)
        #define PAD_VID2_OEN (PAD_VID2_IS_GPIO == GPIO_IN ? BIT0: 0)
        #define PAD_VID2_OUT (PAD_VID2_IS_GPIO == GPIO_OUT_HIGH ? BIT1: 0)
        _RVM1(0x0f22, PAD_VID2_OUT, BIT1),
        _RVM1(0x0f22, PAD_VID2_OEN, BIT0),
    #endif

    #if(PAD_WOL_INT_OUT_IS_GPIO != GPIO_NONE)
        #define PAD_WOL_INT_OUT_OEN (PAD_WOL_INT_OUT_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_WOL_INT_OUT_OUT (PAD_WOL_INT_OUT_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
        _RVM1(0x2e82, PAD_WOL_INT_OUT_OUT, BIT0),
        _RVM1(0x2e82, PAD_WOL_INT_OUT_OEN, BIT1),
        //reg_wol_is_gpio
        _RVM1(0x0e39, BIT1, BIT1),   //reg[0e39]#1 = 1b
    #endif

    #if(PAD_DDCR_CK_IS_GPIO != GPIO_NONE)
        #define PAD_DDCR_CK_OEN (PAD_DDCR_CK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DDCR_CK_OUT (PAD_DDCR_CK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2ee2, PAD_DDCR_CK_OUT, BIT0),
        _RVM1(0x2ee2, PAD_DDCR_CK_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pwm_dac0_mode
        _RVM1(0x2992, 0, BIT0),   //reg[322992]#0 = 0b
        //reg_ddcrmode
        _RVM1(0x2934, 0, BIT1 | BIT0),   //reg[322934]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_00_,
    #endif

_MEMMAP_REGBANK_10_,
    #if(PAD_DDCR_DA_IS_GPIO != GPIO_NONE)
        #define PAD_DDCR_DA_OEN (PAD_DDCR_DA_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DDCR_DA_OUT (PAD_DDCR_DA_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2ee0, PAD_DDCR_DA_OUT, BIT0),
        _RVM1(0x2ee0, PAD_DDCR_DA_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pwm_dac1_mode
        _RVM1(0x2992, 0, BIT4),   //reg[322992]#4 = 0b
        //reg_ddcrmode
        _RVM1(0x2934, 0, BIT1 | BIT0),   //reg[322934]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_DIM0_IS_GPIO != GPIO_NONE)
        #define PAD_DIM0_OEN (PAD_DIM0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DIM0_OUT (PAD_DIM0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f40, PAD_DIM0_OUT, BIT0),
        _RVM1(0x2f40, PAD_DIM0_OEN, BIT1),
        //reg_gpio_dim_pe_00
        _RVM1(0x2f41, BIT0, BIT0),   //reg[322f41]#0 = 1b
        //reg_ld_qspi_config
        _RVM1(0x295e, 0, BIT1 | BIT0),   //reg[32295e]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_DIM1_IS_GPIO != GPIO_NONE)
        #define PAD_DIM1_OEN (PAD_DIM1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DIM1_OUT (PAD_DIM1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f42, PAD_DIM1_OUT, BIT0),
        _RVM1(0x2f42, PAD_DIM1_OEN, BIT1),
        //reg_gpio_dim_pe_01
        _RVM1(0x2f43, BIT0, BIT0),   //reg[322f43]#0 = 1b
        //reg_ld_qspi_config
        _RVM1(0x295e, 0, BIT1 | BIT0),   //reg[32295e]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_DIM2_IS_GPIO != GPIO_NONE)
        #define PAD_DIM2_OEN (PAD_DIM2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DIM2_OUT (PAD_DIM2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f44, PAD_DIM2_OUT, BIT0),
        _RVM1(0x2f44, PAD_DIM2_OEN, BIT1),
        //reg_gpio_dim_pe_02
        _RVM1(0x2f45, BIT0, BIT0),   //reg[322f45]#0 = 1b
        //reg_ld_qspi_config
        _RVM1(0x295e, 0, BIT1 | BIT0),   //reg[32295e]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_DIM3_IS_GPIO != GPIO_NONE)
        #define PAD_DIM3_OEN (PAD_DIM3_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_DIM3_OUT (PAD_DIM3_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f46, PAD_DIM3_OUT, BIT0),
        _RVM1(0x2f46, PAD_DIM3_OEN, BIT1),
        //reg_gpio_dim_pe_03
        _RVM1(0x2f47, BIT0, BIT0),   //reg[322f47]#0 = 1b
        //reg_ld_qspi_config
        _RVM1(0x295e, 0, BIT1 | BIT0),   //reg[32295e]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO0_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO0_OEN (PAD_GPIO0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO0_OUT (PAD_GPIO0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b00, PAD_GPIO0_OUT, BIT0),
        _RVM1(0x2b00, PAD_GPIO0_OEN, BIT1),
        //reg_vsync_like_config
        _RVM1(0x29a4, 0, 0x07),   //reg[3229a4]#2 ~ #0 = 000b
        //reg_ld_spi3_config
        _RVM1(0x29a1, 0, BIT1 | BIT0),   //reg[3229a1]#1 ~ #0 = 00b
        //reg_pwm3_mode
        _RVM1(0x2991, 0, BIT5 | BIT4),   //reg[322991]#5 ~ #4 = 00b
        //reg_p1_enable_b0
        _RVM1(0x29c8, 0, BIT0),   //reg[3229c8]#0 = 0b
        //reg_lg_earc_mode
        _RVM1(0x297c, 0, BIT4),   //reg[32297c]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO1_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO1_OEN (PAD_GPIO1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO1_OUT (PAD_GPIO1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b02, PAD_GPIO1_OUT, BIT0),
        _RVM1(0x2b02, PAD_GPIO1_OEN, BIT1),
        //reg_vsync_like_config
        _RVM1(0x29a4, 0, 0x07),   //reg[3229a4]#2 ~ #0 = 000b
        //reg_ld_spi3_config
        _RVM1(0x29a1, 0, BIT1 | BIT0),   //reg[3229a1]#1 ~ #0 = 00b
        //reg_pwm3_mode
        _RVM1(0x2991, 0, BIT5 | BIT4),   //reg[322991]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO8_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO8_OEN (PAD_GPIO8_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO8_OUT (PAD_GPIO8_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b10, PAD_GPIO8_OUT, BIT0),
        _RVM1(0x2b10, PAD_GPIO8_OEN, BIT1),
        //reg_p1_enable_b7
        _RVM1(0x29cb, 0, BIT4),   //reg[3229cb]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO9_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO9_OEN (PAD_GPIO9_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO9_OUT (PAD_GPIO9_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b12, PAD_GPIO9_OUT, BIT0),
        _RVM1(0x2b12, PAD_GPIO9_OEN, BIT1),
        //reg_miic_mode2
        _RVM1(0x2951, 0, BIT1 | BIT0),   //reg[322951]#1 ~ #0 = 00b
        //reg_extint4
        _RVM1(0x29c2, 0, BIT0),   //reg[3229c2]#0 = 0b
        //reg_p1_enable_b6
        _RVM1(0x29cb, 0, BIT0),   //reg[3229cb]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO10_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO10_OEN (PAD_GPIO10_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO10_OUT (PAD_GPIO10_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b14, PAD_GPIO10_OUT, BIT0),
        _RVM1(0x2b14, PAD_GPIO10_OEN, BIT1),
        //reg_miic_mode2
        _RVM1(0x2951, 0, BIT1 | BIT0),   //reg[322951]#1 ~ #0 = 00b
        //reg_extint5
        _RVM1(0x29c2, 0, BIT4),   //reg[3229c2]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO11_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO11_OEN (PAD_GPIO11_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO11_OUT (PAD_GPIO11_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b16, PAD_GPIO11_OUT, BIT0),
        _RVM1(0x2b16, PAD_GPIO11_OEN, BIT1),
        //reg_fifthuartmode
        _RVM1(0x2945, 0, BIT0),   //reg[322945]#0 = 0b
        //reg_od5thuart
        _RVM1(0x2945, 0, BIT4),   //reg[322945]#4 = 0b
        //reg_tconconfig6
        _RVM1(0x2983, 0, BIT0),   //reg[322983]#0 = 0b
        //reg_extint6
        _RVM1(0x29c3, 0, BIT0),   //reg[3229c3]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO12_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO12_OEN (PAD_GPIO12_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO12_OUT (PAD_GPIO12_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b18, PAD_GPIO12_OUT, BIT0),
        _RVM1(0x2b18, PAD_GPIO12_OEN, BIT1),
        //reg_fifthuartmode
        _RVM1(0x2945, 0, BIT0),   //reg[322945]#0 = 0b
        //reg_od5thuart
        _RVM1(0x2945, 0, BIT4),   //reg[322945]#4 = 0b
        //reg_tconconfig7
        _RVM1(0x2983, 0, BIT4),   //reg[322983]#4 = 0b
        //reg_extint7
        _RVM1(0x29c3, 0, BIT4),   //reg[3229c3]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO13_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO13_OEN (PAD_GPIO13_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO13_OUT (PAD_GPIO13_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b1a, PAD_GPIO13_OUT, BIT0),
        _RVM1(0x2b1a, PAD_GPIO13_OEN, BIT1),
        //reg_p1_enable_b1
        _RVM1(0x29c8, 0, BIT4),   //reg[3229c8]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO14_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO14_OEN (PAD_GPIO14_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO14_OUT (PAD_GPIO14_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b1c, PAD_GPIO14_OUT, BIT0),
        _RVM1(0x2b1c, PAD_GPIO14_OEN, BIT1),
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO15_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO15_OEN (PAD_GPIO15_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO15_OUT (PAD_GPIO15_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b1e, PAD_GPIO15_OUT, BIT0),
        _RVM1(0x2b1e, PAD_GPIO15_OEN, BIT1),
        //reg_i2smutemode
        _RVM1(0x292b, 0, BIT1 | BIT0),   //reg[32292b]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO16_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO16_OEN (PAD_GPIO16_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO16_OUT (PAD_GPIO16_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b20, PAD_GPIO16_OUT, BIT0),
        _RVM1(0x2b20, PAD_GPIO16_OEN, BIT1),
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO17_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO17_OEN (PAD_GPIO17_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO17_OUT (PAD_GPIO17_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b22, PAD_GPIO17_OUT, BIT0),
        _RVM1(0x2b22, PAD_GPIO17_OEN, BIT1),
        //reg_miic_mode2
        _RVM1(0x2951, 0, BIT1 | BIT0),   //reg[322951]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_GPIO18_IS_GPIO != GPIO_NONE)
        #define PAD_GPIO18_OEN (PAD_GPIO18_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_GPIO18_OUT (PAD_GPIO18_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b24, PAD_GPIO18_OUT, BIT0),
        _RVM1(0x2b24, PAD_GPIO18_OEN, BIT1),
        //reg_freeze_tuner
        _RVM1(0x2979, 0, BIT1 | BIT0),   //reg[322979]#1 ~ #0 = 00b
        //reg_miic_mode2
        _RVM1(0x2951, 0, BIT1 | BIT0),   //reg[322951]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_IN_BCK_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_IN_BCK_OEN (PAD_I2S_IN_BCK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_IN_BCK_OUT (PAD_I2S_IN_BCK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e00, PAD_I2S_IN_BCK_OUT, BIT0),
        _RVM1(0x2e00, PAD_I2S_IN_BCK_OEN, BIT1),
        //reg_gpio_i2s_in_pe_00
        _RVM1(0x2e01, BIT0, BIT0),   //reg[322e01]#0 = 1b
        //reg_i2s_in_md
        _RVM1(0x2924, 0, 0x07),   //reg[322924]#2 ~ #0 = 000b
        //reg_thirduartmode
        _RVM1(0x2943, 0, BIT1 | BIT0),   //reg[322943]#1 ~ #0 = 00b
        //reg_od3rduart
        _RVM1(0x2943, 0, BIT5 | BIT4),   //reg[322943]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_IN_DIN0_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_IN_DIN0_OEN (PAD_I2S_IN_DIN0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_IN_DIN0_OUT (PAD_I2S_IN_DIN0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e04, PAD_I2S_IN_DIN0_OUT, BIT0),
        _RVM1(0x2e04, PAD_I2S_IN_DIN0_OEN, BIT1),
        //reg_gpio_i2s_in_pe_02
        _RVM1(0x2e05, BIT0, BIT0),   //reg[322e05]#0 = 1b
        //reg_i2s_in_md
        _RVM1(0x2924, 0, 0x07),   //reg[322924]#2 ~ #0 = 000b
        //reg_miic_mode2
        _RVM1(0x2951, 0, BIT1 | BIT0),   //reg[322951]#1 ~ #0 = 00b
        //reg_3dflagconfig
        _RVM1(0x2970, 0, BIT1 | BIT0),   //reg[322970]#1 ~ #0 = 00b
        //reg_osd3dflag_config
        _RVM1(0x2970, 0, BIT5 | BIT4),   //reg[322970]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_IN_DIN1_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_IN_DIN1_OEN (PAD_I2S_IN_DIN1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_IN_DIN1_OUT (PAD_I2S_IN_DIN1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e06, PAD_I2S_IN_DIN1_OUT, BIT0),
        _RVM1(0x2e06, PAD_I2S_IN_DIN1_OEN, BIT1),
        //reg_i2s_in_md
        _RVM1(0x2924, 0, 0x07),   //reg[322924]#2 ~ #0 = 000b
        //reg_i2s_in_sd2_md
        _RVM1(0x2924, 0, BIT5 | BIT4),   //reg[322924]#5 ~ #4 = 00b
        //reg_miic_mode2
        _RVM1(0x2951, 0, BIT1 | BIT0),   //reg[322951]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_IN_DIN2_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_IN_DIN2_OEN (PAD_I2S_IN_DIN2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_IN_DIN2_OUT (PAD_I2S_IN_DIN2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e08, PAD_I2S_IN_DIN2_OUT, BIT0),
        _RVM1(0x2e08, PAD_I2S_IN_DIN2_OEN, BIT1),
        //reg_gpio_i2s_in_pe_04
        _RVM1(0x2e09, BIT0, BIT0),   //reg[322e09]#0 = 1b
        //reg_mic_md
        _RVM1(0x2920, 0, 0x0F),   //reg[322920]#3 ~ #0 = 0000b
        //reg_i2s_in_md
        _RVM1(0x2924, 0, 0x07),   //reg[322924]#2 ~ #0 = 000b
        //reg_i2s_in_sd2_md
        _RVM1(0x2924, 0, BIT5 | BIT4),   //reg[322924]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_IN_WCK_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_IN_WCK_OEN (PAD_I2S_IN_WCK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_IN_WCK_OUT (PAD_I2S_IN_WCK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e02, PAD_I2S_IN_WCK_OUT, BIT0),
        _RVM1(0x2e02, PAD_I2S_IN_WCK_OEN, BIT1),
        //reg_i2s_in_md
        _RVM1(0x2924, 0, 0x07),   //reg[322924]#2 ~ #0 = 000b
        //reg_thirduartmode
        _RVM1(0x2943, 0, BIT1 | BIT0),   //reg[322943]#1 ~ #0 = 00b
        //reg_od3rduart
        _RVM1(0x2943, 0, BIT5 | BIT4),   //reg[322943]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_OUT_BCK_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_OUT_BCK_OEN (PAD_I2S_OUT_BCK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_OUT_BCK_OUT (PAD_I2S_OUT_BCK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e20, PAD_I2S_OUT_BCK_OUT, BIT0),
        _RVM1(0x2e20, PAD_I2S_OUT_BCK_OEN, BIT1),
        //reg_ej_config
_MEMMAP_REGBANK_10_,
        _RVM1(0x1e27, 0, BIT1 | BIT0),   //reg[101e27]#1 ~ #0 = 00b
        //reg_ej_diagnosis
_MEMMAP_REGBANK_32_,
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_agc_dbg
        _RVM1(0x29e8, 0, BIT0),   //reg[3229e8]#0 = 0b
        //reg_tserrout
        _RVM1(0x29e8, 0, BIT5 | BIT4),   //reg[3229e8]#5 ~ #4 = 00b
        //reg_i2s_out_md
        _RVM1(0x2928, 0, BIT1 | BIT0),   //reg[322928]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_OUT_MCK_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_OUT_MCK_OEN (PAD_I2S_OUT_MCK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_OUT_MCK_OUT (PAD_I2S_OUT_MCK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e22, PAD_I2S_OUT_MCK_OUT, BIT0),
        _RVM1(0x2e22, PAD_I2S_OUT_MCK_OEN, BIT1),
        //reg_fram_delay_flag
        _RVM1(0x2916, 0, BIT0),   //reg[322916]#0 = 0b
        //reg_ej_config
_MEMMAP_REGBANK_10_,
        _RVM1(0x1e27, 0, BIT1 | BIT0),   //reg[101e27]#1 ~ #0 = 00b
        //reg_ej_diagnosis
_MEMMAP_REGBANK_32_,
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_agc_dbg
        _RVM1(0x29e8, 0, BIT0),   //reg[3229e8]#0 = 0b
        //reg_tserrout
        _RVM1(0x29e8, 0, BIT5 | BIT4),   //reg[3229e8]#5 ~ #4 = 00b
        //reg_i2s_out_mck_md
        _RVM1(0x292c, 0, BIT4),   //reg[32292c]#4 = 0b
        //reg_extint2
        _RVM1(0x29c1, 0, BIT0),   //reg[3229c1]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_OUT_SD0_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_OUT_SD0_OEN (PAD_I2S_OUT_SD0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_OUT_SD0_OUT (PAD_I2S_OUT_SD0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e26, PAD_I2S_OUT_SD0_OUT, BIT0),
        _RVM1(0x2e26, PAD_I2S_OUT_SD0_OEN, BIT1),
        //reg_ej_config
_MEMMAP_REGBANK_10_,
        _RVM1(0x1e27, 0, BIT1 | BIT0),   //reg[101e27]#1 ~ #0 = 00b
        //reg_ej_diagnosis
_MEMMAP_REGBANK_32_,
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_i2s_out_md
        _RVM1(0x2928, 0, BIT1 | BIT0),   //reg[322928]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_OUT_SD1_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_OUT_SD1_OEN (PAD_I2S_OUT_SD1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_OUT_SD1_OUT (PAD_I2S_OUT_SD1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e28, PAD_I2S_OUT_SD1_OUT, BIT0),
        _RVM1(0x2e28, PAD_I2S_OUT_SD1_OEN, BIT1),
        //reg_ej_diagnosis
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_spdifoutconfig2
        _RVM1(0x2931, 0, BIT0),   //reg[322931]#0 = 0b
        //reg_i2s_out_md
        _RVM1(0x2928, 0, BIT1 | BIT0),   //reg[322928]#1 ~ #0 = 00b
        //reg_fourthuartmode
        _RVM1(0x2944, 0, BIT1 | BIT0),   //reg[322944]#1 ~ #0 = 00b
        //reg_od4thuart
        _RVM1(0x2944, 0, BIT5 | BIT4),   //reg[322944]#5 ~ #4 = 00b
        //reg_fastuartmode
        _RVM1(0x294e, 0, BIT1 | BIT0),   //reg[32294e]#1 ~ #0 = 00b
        //reg_odfastuart
        _RVM1(0x294e, 0, BIT5 | BIT4),   //reg[32294e]#5 ~ #4 = 00b
        //reg_miic_mode0
        _RVM1(0x2950, 0, BIT1 | BIT0),   //reg[322950]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_OUT_SD2_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_OUT_SD2_OEN (PAD_I2S_OUT_SD2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_OUT_SD2_OUT (PAD_I2S_OUT_SD2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e2a, PAD_I2S_OUT_SD2_OUT, BIT0),
        _RVM1(0x2e2a, PAD_I2S_OUT_SD2_OEN, BIT1),
        //reg_mic_md
        _RVM1(0x2920, 0, 0x0F),   //reg[322920]#3 ~ #0 = 0000b
        //reg_i2s_out_md
        _RVM1(0x2928, 0, BIT1 | BIT0),   //reg[322928]#1 ~ #0 = 00b
        //reg_i2s_out_sd2_md
        _RVM1(0x2928, 0, BIT4),   //reg[322928]#4 = 0b
        //reg_fourthuartmode
        _RVM1(0x2944, 0, BIT1 | BIT0),   //reg[322944]#1 ~ #0 = 00b
        //reg_od4thuart
        _RVM1(0x2944, 0, BIT5 | BIT4),   //reg[322944]#5 ~ #4 = 00b
        //reg_fastuartmode
        _RVM1(0x294e, 0, BIT1 | BIT0),   //reg[32294e]#1 ~ #0 = 00b
        //reg_odfastuart
        _RVM1(0x294e, 0, BIT5 | BIT4),   //reg[32294e]#5 ~ #4 = 00b
        //reg_miic_mode0
        _RVM1(0x2950, 0, BIT1 | BIT0),   //reg[322950]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_I2S_OUT_WCK_IS_GPIO != GPIO_NONE)
        #define PAD_I2S_OUT_WCK_OEN (PAD_I2S_OUT_WCK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_I2S_OUT_WCK_OUT (PAD_I2S_OUT_WCK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e24, PAD_I2S_OUT_WCK_OUT, BIT0),
        _RVM1(0x2e24, PAD_I2S_OUT_WCK_OEN, BIT1),
        //reg_ej_config
_MEMMAP_REGBANK_10_,
        _RVM1(0x1e27, 0, BIT1 | BIT0),   //reg[101e27]#1 ~ #0 = 00b
        //reg_ej_diagnosis
_MEMMAP_REGBANK_32_,
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_i2s_out_md
        _RVM1(0x2928, 0, BIT1 | BIT0),   //reg[322928]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_LD_SPI_CK_IS_GPIO != GPIO_NONE)
        #define PAD_LD_SPI_CK_OEN (PAD_LD_SPI_CK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_LD_SPI_CK_OUT (PAD_LD_SPI_CK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f26, PAD_LD_SPI_CK_OUT, BIT0),
        _RVM1(0x2f26, PAD_LD_SPI_CK_OEN, BIT1),
        //reg_gpio_ld_pe_03
        _RVM1(0x2f27, BIT0, BIT0),   //reg[322f27]#0 = 1b
        //reg_ej_config
_MEMMAP_REGBANK_10_,
        _RVM1(0x1e27, 0, BIT1 | BIT0),   //reg[101e27]#1 ~ #0 = 00b
        //reg_ld_spi1_config
_MEMMAP_REGBANK_32_,
        _RVM1(0x29a0, 0, BIT1 | BIT0),   //reg[3229a0]#1 ~ #0 = 00b
        //reg_ld_spi3_config
        _RVM1(0x29a1, 0, BIT1 | BIT0),   //reg[3229a1]#1 ~ #0 = 00b
        //reg_ld_qspi_config
        _RVM1(0x295e, 0, BIT1 | BIT0),   //reg[32295e]#1 ~ #0 = 00b
        //reg_p1_enable_b5
        _RVM1(0x29ca, 0, BIT4),   //reg[3229ca]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_LD_SPI_CS_IS_GPIO != GPIO_NONE)
        #define PAD_LD_SPI_CS_OEN (PAD_LD_SPI_CS_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_LD_SPI_CS_OUT (PAD_LD_SPI_CS_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f22, PAD_LD_SPI_CS_OUT, BIT0),
        _RVM1(0x2f22, PAD_LD_SPI_CS_OEN, BIT1),
        //reg_gpio_ld_pe_01
        _RVM1(0x2f23, BIT0, BIT0),   //reg[322f23]#0 = 1b
        //reg_ej_config
_MEMMAP_REGBANK_10_,
        _RVM1(0x1e27, 0, BIT1 | BIT0),   //reg[101e27]#1 ~ #0 = 00b
        //reg_ld_spi2_config
_MEMMAP_REGBANK_32_,
        _RVM1(0x29a0, 0, BIT5 | BIT4),   //reg[3229a0]#5 ~ #4 = 00b
        //reg_ld_spi3_config
        _RVM1(0x29a1, 0, BIT1 | BIT0),   //reg[3229a1]#1 ~ #0 = 00b
        //reg_ld_qspi_config
        _RVM1(0x295e, 0, BIT1 | BIT0),   //reg[32295e]#1 ~ #0 = 00b
        //reg_seconduartmode
        _RVM1(0x2942, 0, BIT1 | BIT0),   //reg[322942]#1 ~ #0 = 00b
        //reg_od2nduart
        _RVM1(0x2942, 0, BIT5 | BIT4),   //reg[322942]#5 ~ #4 = 00b
        //reg_fourthuartmode
        _RVM1(0x2944, 0, BIT1 | BIT0),   //reg[322944]#1 ~ #0 = 00b
        //reg_od4thuart
        _RVM1(0x2944, 0, BIT5 | BIT4),   //reg[322944]#5 ~ #4 = 00b
        //reg_fastuartmode
        _RVM1(0x294e, 0, BIT1 | BIT0),   //reg[32294e]#1 ~ #0 = 00b
        //reg_odfastuart
        _RVM1(0x294e, 0, BIT5 | BIT4),   //reg[32294e]#5 ~ #4 = 00b
        //reg_p1_enable_b3
        _RVM1(0x29c9, 0, BIT4),   //reg[3229c9]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_LD_SPI_MISO_IS_GPIO != GPIO_NONE)
        #define PAD_LD_SPI_MISO_OEN (PAD_LD_SPI_MISO_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_LD_SPI_MISO_OUT (PAD_LD_SPI_MISO_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f20, PAD_LD_SPI_MISO_OUT, BIT0),
        _RVM1(0x2f20, PAD_LD_SPI_MISO_OEN, BIT1),
        //reg_gpio_ld_pe_00
        _RVM1(0x2f21, BIT0, BIT0),   //reg[322f21]#0 = 1b
        //reg_ej_config
_MEMMAP_REGBANK_10_,
        _RVM1(0x1e27, 0, BIT1 | BIT0),   //reg[101e27]#1 ~ #0 = 00b
        //reg_ld_spi2_config
_MEMMAP_REGBANK_32_,
        _RVM1(0x29a0, 0, BIT5 | BIT4),   //reg[3229a0]#5 ~ #4 = 00b
        //reg_ld_spi3_config
        _RVM1(0x29a1, 0, BIT1 | BIT0),   //reg[3229a1]#1 ~ #0 = 00b
        //reg_ld_qspi_config
        _RVM1(0x295e, 0, BIT1 | BIT0),   //reg[32295e]#1 ~ #0 = 00b
        //reg_fourthuartmode
        _RVM1(0x2944, 0, BIT1 | BIT0),   //reg[322944]#1 ~ #0 = 00b
        //reg_od4thuart
        _RVM1(0x2944, 0, BIT5 | BIT4),   //reg[322944]#5 ~ #4 = 00b
        //reg_fastuartmode
        _RVM1(0x294e, 0, BIT1 | BIT0),   //reg[32294e]#1 ~ #0 = 00b
        //reg_odfastuart
        _RVM1(0x294e, 0, BIT5 | BIT4),   //reg[32294e]#5 ~ #4 = 00b
        //reg_p1_enable_b2
        _RVM1(0x29c9, 0, BIT0),   //reg[3229c9]#0 = 0b
        //reg_3dflagconfig
        _RVM1(0x2970, 0, BIT1 | BIT0),   //reg[322970]#1 ~ #0 = 00b
        //reg_osd3dflag_config
        _RVM1(0x2970, 0, BIT5 | BIT4),   //reg[322970]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_LD_SPI_MOSI_IS_GPIO != GPIO_NONE)
        #define PAD_LD_SPI_MOSI_OEN (PAD_LD_SPI_MOSI_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_LD_SPI_MOSI_OUT (PAD_LD_SPI_MOSI_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f24, PAD_LD_SPI_MOSI_OUT, BIT0),
        _RVM1(0x2f24, PAD_LD_SPI_MOSI_OEN, BIT1),
        //reg_gpio_ld_pe_02
        _RVM1(0x2f25, BIT0, BIT0),   //reg[322f25]#0 = 1b
        //reg_ej_config
_MEMMAP_REGBANK_10_,
        _RVM1(0x1e27, 0, BIT1 | BIT0),   //reg[101e27]#1 ~ #0 = 00b
        //reg_ld_spi1_config
_MEMMAP_REGBANK_32_,
        _RVM1(0x29a0, 0, BIT1 | BIT0),   //reg[3229a0]#1 ~ #0 = 00b
        //reg_ld_spi3_config
        _RVM1(0x29a1, 0, BIT1 | BIT0),   //reg[3229a1]#1 ~ #0 = 00b
        //reg_ld_qspi_config
        _RVM1(0x295e, 0, BIT1 | BIT0),   //reg[32295e]#1 ~ #0 = 00b
        //reg_seconduartmode
        _RVM1(0x2942, 0, BIT1 | BIT0),   //reg[322942]#1 ~ #0 = 00b
        //reg_od2nduart
        _RVM1(0x2942, 0, BIT5 | BIT4),   //reg[322942]#5 ~ #4 = 00b
        //reg_p1_enable_b4
        _RVM1(0x29ca, 0, BIT0),   //reg[3229ca]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_MIC_BCK_IS_GPIO != GPIO_NONE)
        #define PAD_MIC_BCK_OEN (PAD_MIC_BCK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_MIC_BCK_OUT (PAD_MIC_BCK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e40, PAD_MIC_BCK_OUT, BIT0),
        _RVM1(0x2e40, PAD_MIC_BCK_OEN, BIT1),
        //reg_gpio_dmic_pe_00
        _RVM1(0x2e41, BIT0, BIT0),   //reg[322e41]#0 = 1b
        //reg_mic_md
        _RVM1(0x2920, 0, 0x0F),   //reg[322920]#3 ~ #0 = 0000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_MIC_SD0_IS_GPIO != GPIO_NONE)
        #define PAD_MIC_SD0_OEN (PAD_MIC_SD0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_MIC_SD0_OUT (PAD_MIC_SD0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e42, PAD_MIC_SD0_OUT, BIT0),
        _RVM1(0x2e42, PAD_MIC_SD0_OEN, BIT1),
        //reg_gpio_dmic_pe_01
        _RVM1(0x2e43, BIT0, BIT0),   //reg[322e43]#0 = 1b
        //reg_mic_md
        _RVM1(0x2920, 0, 0x0F),   //reg[322920]#3 ~ #0 = 0000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_MIC_SD1_IS_GPIO != GPIO_NONE)
        #define PAD_MIC_SD1_OEN (PAD_MIC_SD1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_MIC_SD1_OUT (PAD_MIC_SD1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e44, PAD_MIC_SD1_OUT, BIT0),
        _RVM1(0x2e44, PAD_MIC_SD1_OEN, BIT1),
        //reg_gpio_dmic_pe_02
        _RVM1(0x2e45, BIT0, BIT0),   //reg[322e45]#0 = 1b
        //reg_mic_md
        _RVM1(0x2920, 0, 0x0F),   //reg[322920]#3 ~ #0 = 0000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_MIC_SD2_IS_GPIO != GPIO_NONE)
        #define PAD_MIC_SD2_OEN (PAD_MIC_SD2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_MIC_SD2_OUT (PAD_MIC_SD2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e46, PAD_MIC_SD2_OUT, BIT0),
        _RVM1(0x2e46, PAD_MIC_SD2_OEN, BIT1),
        //reg_mic_md
        _RVM1(0x2920, 0, 0x0F),   //reg[322920]#3 ~ #0 = 0000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM2_CD_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM2_CD_N_OEN (PAD_PCM2_CD_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM2_CD_N_OUT (PAD_PCM2_CD_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d66, PAD_PCM2_CD_N_OUT, BIT0),
        _RVM1(0x2d66, PAD_PCM2_CD_N_OEN, BIT1),
        //reg_pcm2ctrlconfig
        _RVM1(0x2911, 0, BIT0),   //reg[322911]#0 = 0b
        //reg_pcm2_cdn_config
        _RVM1(0x2911, 0, BIT4),   //reg[322911]#4 = 0b
        //reg_mspi3_config
        _RVM1(0x2958, 0, 0x07),   //reg[322958]#2 ~ #0 = 000b
        //reg_sdio_config
        _RVM1(0x2968, 0, BIT0),   //reg[322968]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM2_CE_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM2_CE_N_OEN (PAD_PCM2_CE_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM2_CE_N_OUT (PAD_PCM2_CE_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d68, PAD_PCM2_CE_N_OUT, BIT0),
        _RVM1(0x2d68, PAD_PCM2_CE_N_OEN, BIT1),
        //reg_pcm2ctrlconfig
        _RVM1(0x2911, 0, BIT0),   //reg[322911]#0 = 0b
        //reg_mspi3_config
        _RVM1(0x2958, 0, 0x07),   //reg[322958]#2 ~ #0 = 000b
        //reg_sdio_config
        _RVM1(0x2968, 0, BIT0),   //reg[322968]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM2_IRQA_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM2_IRQA_N_OEN (PAD_PCM2_IRQA_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM2_IRQA_N_OUT (PAD_PCM2_IRQA_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d62, PAD_PCM2_IRQA_N_OUT, BIT0),
        _RVM1(0x2d62, PAD_PCM2_IRQA_N_OEN, BIT1),
        //reg_diseqc_out_config
        _RVM1(0x2978, 0, BIT4),   //reg[322978]#4 = 0b
        //reg_pcm2ctrlconfig
        _RVM1(0x2911, 0, BIT0),   //reg[322911]#0 = 0b
        //reg_mspi3_config
        _RVM1(0x2958, 0, 0x07),   //reg[322958]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM2_RESET_IS_GPIO != GPIO_NONE)
        #define PAD_PCM2_RESET_OEN (PAD_PCM2_RESET_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM2_RESET_OUT (PAD_PCM2_RESET_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d60, PAD_PCM2_RESET_OUT, BIT0),
        _RVM1(0x2d60, PAD_PCM2_RESET_OEN, BIT1),
        //reg_diseqc_in_config
        _RVM1(0x2978, 0, BIT0),   //reg[322978]#0 = 0b
        //reg_pcm2ctrlconfig
        _RVM1(0x2911, 0, BIT0),   //reg[322911]#0 = 0b
        //reg_extint3
        _RVM1(0x29c1, 0, BIT4),   //reg[3229c1]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM2_WAIT_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM2_WAIT_N_OEN (PAD_PCM2_WAIT_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM2_WAIT_N_OUT (PAD_PCM2_WAIT_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d64, PAD_PCM2_WAIT_N_OUT, BIT0),
        _RVM1(0x2d64, PAD_PCM2_WAIT_N_OEN, BIT1),
        //reg_pcm2ctrlconfig
        _RVM1(0x2911, 0, BIT0),   //reg[322911]#0 = 0b
        //reg_mspi3_config
        _RVM1(0x2958, 0, 0x07),   //reg[322958]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A0_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A0_OEN (PAD_PCM_A0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A0_OUT (PAD_PCM_A0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d00, PAD_PCM_A0_OUT, BIT0),
        _RVM1(0x2d00, PAD_PCM_A0_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_nand_mode
        _RVM1(0x2964, 0, BIT1 | BIT0),   //reg[322964]#1 ~ #0 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A1_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A1_OEN (PAD_PCM_A1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A1_OUT (PAD_PCM_A1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d02, PAD_PCM_A1_OUT, BIT0),
        _RVM1(0x2d02, PAD_PCM_A1_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_nand_mode
        _RVM1(0x2964, 0, BIT1 | BIT0),   //reg[322964]#1 ~ #0 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A2_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A2_OEN (PAD_PCM_A2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A2_OUT (PAD_PCM_A2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d04, PAD_PCM_A2_OUT, BIT0),
        _RVM1(0x2d04, PAD_PCM_A2_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_nand_mode
        _RVM1(0x2964, 0, BIT1 | BIT0),   //reg[322964]#1 ~ #0 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A3_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A3_OEN (PAD_PCM_A3_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A3_OUT (PAD_PCM_A3_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d06, PAD_PCM_A3_OUT, BIT0),
        _RVM1(0x2d06, PAD_PCM_A3_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_nand_mode
        _RVM1(0x2964, 0, BIT1 | BIT0),   //reg[322964]#1 ~ #0 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A4_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A4_OEN (PAD_PCM_A4_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A4_OUT (PAD_PCM_A4_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d08, PAD_PCM_A4_OUT, BIT0),
        _RVM1(0x2d08, PAD_PCM_A4_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_nand_mode
        _RVM1(0x2964, 0, BIT1 | BIT0),   //reg[322964]#1 ~ #0 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A5_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A5_OEN (PAD_PCM_A5_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A5_OUT (PAD_PCM_A5_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d0a, PAD_PCM_A5_OUT, BIT0),
        _RVM1(0x2d0a, PAD_PCM_A5_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_nand_mode
        _RVM1(0x2964, 0, BIT1 | BIT0),   //reg[322964]#1 ~ #0 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A6_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A6_OEN (PAD_PCM_A6_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A6_OUT (PAD_PCM_A6_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d0c, PAD_PCM_A6_OUT, BIT0),
        _RVM1(0x2d0c, PAD_PCM_A6_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_nand_mode
        _RVM1(0x2964, 0, BIT1 | BIT0),   //reg[322964]#1 ~ #0 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A7_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A7_OEN (PAD_PCM_A7_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A7_OUT (PAD_PCM_A7_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d0e, PAD_PCM_A7_OUT, BIT0),
        _RVM1(0x2d0e, PAD_PCM_A7_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_nand_mode
        _RVM1(0x2964, 0, BIT1 | BIT0),   //reg[322964]#1 ~ #0 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A8_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A8_OEN (PAD_PCM_A8_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A8_OUT (PAD_PCM_A8_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d10, PAD_PCM_A8_OUT, BIT0),
        _RVM1(0x2d10, PAD_PCM_A8_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A9_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A9_OEN (PAD_PCM_A9_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A9_OUT (PAD_PCM_A9_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d12, PAD_PCM_A9_OUT, BIT0),
        _RVM1(0x2d12, PAD_PCM_A9_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A10_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A10_OEN (PAD_PCM_A10_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A10_OUT (PAD_PCM_A10_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d14, PAD_PCM_A10_OUT, BIT0),
        _RVM1(0x2d14, PAD_PCM_A10_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A11_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A11_OEN (PAD_PCM_A11_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A11_OUT (PAD_PCM_A11_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d16, PAD_PCM_A11_OUT, BIT0),
        _RVM1(0x2d16, PAD_PCM_A11_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A12_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A12_OEN (PAD_PCM_A12_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A12_OUT (PAD_PCM_A12_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d18, PAD_PCM_A12_OUT, BIT0),
        _RVM1(0x2d18, PAD_PCM_A12_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A13_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A13_OEN (PAD_PCM_A13_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A13_OUT (PAD_PCM_A13_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d1a, PAD_PCM_A13_OUT, BIT0),
        _RVM1(0x2d1a, PAD_PCM_A13_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_A14_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_A14_OEN (PAD_PCM_A14_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_A14_OUT (PAD_PCM_A14_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d1c, PAD_PCM_A14_OUT, BIT0),
        _RVM1(0x2d1c, PAD_PCM_A14_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_CD_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_CD_N_OEN (PAD_PCM_CD_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_CD_N_OUT (PAD_PCM_CD_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d50, PAD_PCM_CD_N_OUT, BIT0),
        _RVM1(0x2d50, PAD_PCM_CD_N_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_CE_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_CE_N_OEN (PAD_PCM_CE_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_CE_N_OUT (PAD_PCM_CE_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d52, PAD_PCM_CE_N_OUT, BIT0),
        _RVM1(0x2d52, PAD_PCM_CE_N_OEN, BIT1),
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_D0_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_D0_OEN (PAD_PCM_D0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_D0_OUT (PAD_PCM_D0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d20, PAD_PCM_D0_OUT, BIT0),
        _RVM1(0x2d20, PAD_PCM_D0_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_D1_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_D1_OEN (PAD_PCM_D1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_D1_OUT (PAD_PCM_D1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d22, PAD_PCM_D1_OUT, BIT0),
        _RVM1(0x2d22, PAD_PCM_D1_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_D2_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_D2_OEN (PAD_PCM_D2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_D2_OUT (PAD_PCM_D2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d24, PAD_PCM_D2_OUT, BIT0),
        _RVM1(0x2d24, PAD_PCM_D2_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_D3_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_D3_OEN (PAD_PCM_D3_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_D3_OUT (PAD_PCM_D3_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d26, PAD_PCM_D3_OUT, BIT0),
        _RVM1(0x2d26, PAD_PCM_D3_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_D4_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_D4_OEN (PAD_PCM_D4_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_D4_OUT (PAD_PCM_D4_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d28, PAD_PCM_D4_OUT, BIT0),
        _RVM1(0x2d28, PAD_PCM_D4_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_D5_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_D5_OEN (PAD_PCM_D5_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_D5_OUT (PAD_PCM_D5_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d2a, PAD_PCM_D5_OUT, BIT0),
        _RVM1(0x2d2a, PAD_PCM_D5_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_D6_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_D6_OEN (PAD_PCM_D6_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_D6_OUT (PAD_PCM_D6_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d2c, PAD_PCM_D6_OUT, BIT0),
        _RVM1(0x2d2c, PAD_PCM_D6_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_D7_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_D7_OEN (PAD_PCM_D7_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_D7_OUT (PAD_PCM_D7_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d2e, PAD_PCM_D7_OUT, BIT0),
        _RVM1(0x2d2e, PAD_PCM_D7_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_pcmadconfig
        _RVM1(0x2910, 0, BIT0),   //reg[322910]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_IORD_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_IORD_N_OEN (PAD_PCM_IORD_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_IORD_N_OUT (PAD_PCM_IORD_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d44, PAD_PCM_IORD_N_OUT, BIT0),
        _RVM1(0x2d44, PAD_PCM_IORD_N_OEN, BIT1),
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_IOWR_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_IOWR_N_OEN (PAD_PCM_IOWR_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_IOWR_N_OUT (PAD_PCM_IOWR_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d46, PAD_PCM_IOWR_N_OUT, BIT0),
        _RVM1(0x2d46, PAD_PCM_IOWR_N_OEN, BIT1),
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_IRQA_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_IRQA_N_OEN (PAD_PCM_IRQA_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_IRQA_N_OUT (PAD_PCM_IRQA_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d4a, PAD_PCM_IRQA_N_OUT, BIT0),
        _RVM1(0x2d4a, PAD_PCM_IRQA_N_OEN, BIT1),
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_OE_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_OE_N_OEN (PAD_PCM_OE_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_OE_N_OUT (PAD_PCM_OE_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d42, PAD_PCM_OE_N_OUT, BIT0),
        _RVM1(0x2d42, PAD_PCM_OE_N_OEN, BIT1),
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_REG_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_REG_N_OEN (PAD_PCM_REG_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_REG_N_OUT (PAD_PCM_REG_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d4e, PAD_PCM_REG_N_OUT, BIT0),
        _RVM1(0x2d4e, PAD_PCM_REG_N_OEN, BIT1),
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_RESET_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_RESET_OEN (PAD_PCM_RESET_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_RESET_OUT (PAD_PCM_RESET_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d40, PAD_PCM_RESET_OUT, BIT0),
        _RVM1(0x2d40, PAD_PCM_RESET_OEN, BIT1),
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_WAIT_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_WAIT_N_OEN (PAD_PCM_WAIT_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_WAIT_N_OUT (PAD_PCM_WAIT_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d4c, PAD_PCM_WAIT_N_OUT, BIT0),
        _RVM1(0x2d4c, PAD_PCM_WAIT_N_OEN, BIT1),
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PCM_WE_N_IS_GPIO != GPIO_NONE)
        #define PAD_PCM_WE_N_OEN (PAD_PCM_WE_N_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PCM_WE_N_OUT (PAD_PCM_WE_N_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2d48, PAD_PCM_WE_N_OUT, BIT0),
        _RVM1(0x2d48, PAD_PCM_WE_N_OEN, BIT1),
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_pcmctrlconfig
        _RVM1(0x2910, 0, BIT4),   //reg[322910]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PWM0_IS_GPIO != GPIO_NONE)
        #define PAD_PWM0_OEN (PAD_PWM0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PWM0_OUT (PAD_PWM0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f60, PAD_PWM0_OUT, BIT0),
        _RVM1(0x2f60, PAD_PWM0_OEN, BIT1),
        //reg_vsense_en
        _RVM1(0x29f8, 0, BIT0),   //reg[3229f8]#0 = 0b
        //reg_pwm0_mode
        _RVM1(0x2990, 0, BIT0),   //reg[322990]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PWM1_IS_GPIO != GPIO_NONE)
        #define PAD_PWM1_OEN (PAD_PWM1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PWM1_OUT (PAD_PWM1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f62, PAD_PWM1_OUT, BIT0),
        _RVM1(0x2f62, PAD_PWM1_OEN, BIT1),
        //reg_vsync_like_config
        _RVM1(0x29a4, 0, 0x07),   //reg[3229a4]#2 ~ #0 = 000b
        //reg_pwm1_mode
        _RVM1(0x2990, 0, BIT4),   //reg[322990]#4 = 0b
        //reg_allpad_in
        _RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_PWM2_IS_GPIO != GPIO_NONE)
        #define PAD_PWM2_OEN (PAD_PWM2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_PWM2_OUT (PAD_PWM2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f64, PAD_PWM2_OUT, BIT0),
        _RVM1(0x2f64, PAD_PWM2_OEN, BIT1),
        //reg_i2smutemode
        _RVM1(0x292b, 0, BIT1 | BIT0),   //reg[32292b]#1 ~ #0 = 00b
        //reg_pwm2_mode
        _RVM1(0x2991, 0, BIT1 | BIT0),   //reg[322991]#1 ~ #0 = 00b
        //reg_extint0
        _RVM1(0x29c0, 0, BIT0),   //reg[3229c0]#0 = 0b
        //reg_lg_earc_mode
        _RVM1(0x297c, 0, BIT4),   //reg[32297c]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_SPDIF_IN_IS_GPIO != GPIO_NONE)
        #define PAD_SPDIF_IN_OEN (PAD_SPDIF_IN_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_SPDIF_IN_OUT (PAD_SPDIF_IN_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e62, PAD_SPDIF_IN_OUT, BIT0),
        _RVM1(0x2e62, PAD_SPDIF_IN_OEN, BIT1),
        //reg_spdifinconfig
        _RVM1(0x2930, 0, BIT0),   //reg[322930]#0 = 0b
        //reg_tconconfig5
        _RVM1(0x2982, 0, BIT5 | BIT4),   //reg[322982]#5 ~ #4 = 00b
        //reg_extint1
        _RVM1(0x29c0, 0, BIT4),   //reg[3229c0]#4 = 0b
        //reg_3dflagconfig
        _RVM1(0x2970, 0, BIT1 | BIT0),   //reg[322970]#1 ~ #0 = 00b
        //reg_osd3dflag_config
        _RVM1(0x2970, 0, BIT5 | BIT4),   //reg[322970]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_SPDIF_OUT_IS_GPIO != GPIO_NONE)
        #define PAD_SPDIF_OUT_OEN (PAD_SPDIF_OUT_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_SPDIF_OUT_OUT (PAD_SPDIF_OUT_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2e60, PAD_SPDIF_OUT_OUT, BIT0),
        _RVM1(0x2e60, PAD_SPDIF_OUT_OEN, BIT1),
        //reg_spdifoutconfig
        _RVM1(0x2930, 0, BIT4),   //reg[322930]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TCON0_IS_GPIO != GPIO_NONE)
        #define PAD_TCON0_OEN (PAD_TCON0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TCON0_OUT (PAD_TCON0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f00, PAD_TCON0_OUT, BIT0),
        _RVM1(0x2f00, PAD_TCON0_OEN, BIT1),
        //reg_gpio_tcon_pe_00
        _RVM1(0x2f01, BIT0, BIT0),   //reg[322f01]#0 = 1b
        //reg_seconduartmode
        _RVM1(0x2942, 0, BIT1 | BIT0),   //reg[322942]#1 ~ #0 = 00b
        //reg_od2nduart
        _RVM1(0x2942, 0, BIT5 | BIT4),   //reg[322942]#5 ~ #4 = 00b
        //reg_miic_mode0
        _RVM1(0x2950, 0, BIT1 | BIT0),   //reg[322950]#1 ~ #0 = 00b
        //reg_tconconfig0
        _RVM1(0x2980, 0, BIT0),   //reg[322980]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TCON1_IS_GPIO != GPIO_NONE)
        #define PAD_TCON1_OEN (PAD_TCON1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TCON1_OUT (PAD_TCON1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f02, PAD_TCON1_OUT, BIT0),
        _RVM1(0x2f02, PAD_TCON1_OEN, BIT1),
        //reg_gpio_tcon_pe_01
        _RVM1(0x2f03, BIT0, BIT0),   //reg[322f03]#0 = 1b
        //reg_seconduartmode
        _RVM1(0x2942, 0, BIT1 | BIT0),   //reg[322942]#1 ~ #0 = 00b
        //reg_od2nduart
        _RVM1(0x2942, 0, BIT5 | BIT4),   //reg[322942]#5 ~ #4 = 00b
        //reg_miic_mode0
        _RVM1(0x2950, 0, BIT1 | BIT0),   //reg[322950]#1 ~ #0 = 00b
        //reg_tconconfig1
        _RVM1(0x2980, 0, BIT4),   //reg[322980]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TCON2_IS_GPIO != GPIO_NONE)
        #define PAD_TCON2_OEN (PAD_TCON2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TCON2_OUT (PAD_TCON2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f04, PAD_TCON2_OUT, BIT0),
        _RVM1(0x2f04, PAD_TCON2_OEN, BIT1),
        //reg_gpio_tcon_pe_02
        _RVM1(0x2f05, BIT0, BIT0),   //reg[322f05]#0 = 1b
        //reg_thirduartmode
        _RVM1(0x2943, 0, BIT1 | BIT0),   //reg[322943]#1 ~ #0 = 00b
        //reg_od3rduart
        _RVM1(0x2943, 0, BIT5 | BIT4),   //reg[322943]#5 ~ #4 = 00b
        //reg_tconconfig2
        _RVM1(0x2981, 0, BIT0),   //reg[322981]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TCON3_IS_GPIO != GPIO_NONE)
        #define PAD_TCON3_OEN (PAD_TCON3_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TCON3_OUT (PAD_TCON3_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f06, PAD_TCON3_OUT, BIT0),
        _RVM1(0x2f06, PAD_TCON3_OEN, BIT1),
        //reg_thirduartmode
        _RVM1(0x2943, 0, BIT1 | BIT0),   //reg[322943]#1 ~ #0 = 00b
        //reg_od3rduart
        _RVM1(0x2943, 0, BIT5 | BIT4),   //reg[322943]#5 ~ #4 = 00b
        //reg_tconconfig3
        _RVM1(0x2981, 0, BIT4),   //reg[322981]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TCON4_IS_GPIO != GPIO_NONE)
        #define PAD_TCON4_OEN (PAD_TCON4_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TCON4_OUT (PAD_TCON4_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2f08, PAD_TCON4_OUT, BIT0),
        _RVM1(0x2f08, PAD_TCON4_OEN, BIT1),
        //reg_pwm2_mode
        _RVM1(0x2991, 0, BIT1 | BIT0),   //reg[322991]#1 ~ #0 = 00b
        //reg_tconconfig4
        _RVM1(0x2982, 0, BIT0),   //reg[322982]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TGPIO0_IS_GPIO != GPIO_NONE)
        #define PAD_TGPIO0_OEN (PAD_TGPIO0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TGPIO0_OUT (PAD_TGPIO0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b80, PAD_TGPIO0_OUT, BIT0),
        _RVM1(0x2b80, PAD_TGPIO0_OEN, BIT1),
        //reg_vsync_vif_out_en
        _RVM1(0x2979, 0, BIT4),   //reg[322979]#4 = 0b
        //reg_freeze_tuner
        _RVM1(0x2979, 0, BIT1 | BIT0),   //reg[322979]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TGPIO1_IS_GPIO != GPIO_NONE)
        #define PAD_TGPIO1_OEN (PAD_TGPIO1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TGPIO1_OUT (PAD_TGPIO1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b82, PAD_TGPIO1_OUT, BIT0),
        _RVM1(0x2b82, PAD_TGPIO1_OEN, BIT1),
        //reg_freeze_tuner
        _RVM1(0x2979, 0, BIT1 | BIT0),   //reg[322979]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TGPIO2_IS_GPIO != GPIO_NONE)
        #define PAD_TGPIO2_OEN (PAD_TGPIO2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TGPIO2_OUT (PAD_TGPIO2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b84, PAD_TGPIO2_OUT, BIT0),
        _RVM1(0x2b84, PAD_TGPIO2_OEN, BIT1),
        //reg_miic_mode1
        _RVM1(0x2950, 0, BIT4),   //reg[322950]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TGPIO3_IS_GPIO != GPIO_NONE)
        #define PAD_TGPIO3_OEN (PAD_TGPIO3_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TGPIO3_OUT (PAD_TGPIO3_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2b86, PAD_TGPIO3_OUT, BIT0),
        _RVM1(0x2b86, PAD_TGPIO3_OEN, BIT1),
        //reg_miic_mode1
        _RVM1(0x2950, 0, BIT4),   //reg[322950]#4 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_CLK_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_CLK_OEN (PAD_TS0_CLK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_CLK_OUT (PAD_TS0_CLK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c10, PAD_TS0_CLK_OUT, BIT0),
        _RVM1(0x2c10, PAD_TS0_CLK_OEN, BIT1),
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_D0_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_D0_OEN (PAD_TS0_D0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_D0_OUT (PAD_TS0_D0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c00, PAD_TS0_D0_OUT, BIT0),
        _RVM1(0x2c00, PAD_TS0_D0_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_D1_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_D1_OEN (PAD_TS0_D1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_D1_OUT (PAD_TS0_D1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c02, PAD_TS0_D1_OUT, BIT0),
        _RVM1(0x2c02, PAD_TS0_D1_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_D2_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_D2_OEN (PAD_TS0_D2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_D2_OUT (PAD_TS0_D2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c04, PAD_TS0_D2_OUT, BIT0),
        _RVM1(0x2c04, PAD_TS0_D2_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_D3_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_D3_OEN (PAD_TS0_D3_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_D3_OUT (PAD_TS0_D3_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c06, PAD_TS0_D3_OUT, BIT0),
        _RVM1(0x2c06, PAD_TS0_D3_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_D4_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_D4_OEN (PAD_TS0_D4_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_D4_OUT (PAD_TS0_D4_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c08, PAD_TS0_D4_OUT, BIT0),
        _RVM1(0x2c08, PAD_TS0_D4_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_D5_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_D5_OEN (PAD_TS0_D5_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_D5_OUT (PAD_TS0_D5_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c0a, PAD_TS0_D5_OUT, BIT0),
        _RVM1(0x2c0a, PAD_TS0_D5_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_D6_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_D6_OEN (PAD_TS0_D6_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_D6_OUT (PAD_TS0_D6_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c0c, PAD_TS0_D6_OUT, BIT0),
        _RVM1(0x2c0c, PAD_TS0_D6_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_D7_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_D7_OEN (PAD_TS0_D7_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_D7_OUT (PAD_TS0_D7_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c0e, PAD_TS0_D7_OUT, BIT0),
        _RVM1(0x2c0e, PAD_TS0_D7_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_SYNC_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_SYNC_OEN (PAD_TS0_SYNC_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_SYNC_OUT (PAD_TS0_SYNC_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c12, PAD_TS0_SYNC_OUT, BIT0),
        _RVM1(0x2c12, PAD_TS0_SYNC_OEN, BIT1),
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS0_VLD_IS_GPIO != GPIO_NONE)
        #define PAD_TS0_VLD_OEN (PAD_TS0_VLD_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS0_VLD_OUT (PAD_TS0_VLD_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c14, PAD_TS0_VLD_OUT, BIT0),
        _RVM1(0x2c14, PAD_TS0_VLD_OEN, BIT1),
        //reg_ts0config
        _RVM1(0x2900, 0, 0x07),   //reg[322900]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_CLK_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_CLK_OEN (PAD_TS1_CLK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_CLK_OUT (PAD_TS1_CLK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c30, PAD_TS1_CLK_OUT, BIT0),
        _RVM1(0x2c30, PAD_TS1_CLK_OEN, BIT1),
        //reg_ej_diagnosis
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_D0_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_D0_OEN (PAD_TS1_D0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_D0_OUT (PAD_TS1_D0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c20, PAD_TS1_D0_OUT, BIT0),
        _RVM1(0x2c20, PAD_TS1_D0_OEN, BIT1),
        //reg_ej_diagnosis
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_D1_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_D1_OEN (PAD_TS1_D1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_D1_OUT (PAD_TS1_D1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c22, PAD_TS1_D1_OUT, BIT0),
        _RVM1(0x2c22, PAD_TS1_D1_OEN, BIT1),
        //reg_ej_diagnosis
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_D2_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_D2_OEN (PAD_TS1_D2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_D2_OUT (PAD_TS1_D2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c24, PAD_TS1_D2_OUT, BIT0),
        _RVM1(0x2c24, PAD_TS1_D2_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_D3_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_D3_OEN (PAD_TS1_D3_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_D3_OUT (PAD_TS1_D3_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c26, PAD_TS1_D3_OUT, BIT0),
        _RVM1(0x2c26, PAD_TS1_D3_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_D4_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_D4_OEN (PAD_TS1_D4_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_D4_OUT (PAD_TS1_D4_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c28, PAD_TS1_D4_OUT, BIT0),
        _RVM1(0x2c28, PAD_TS1_D4_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_D5_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_D5_OEN (PAD_TS1_D5_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_D5_OUT (PAD_TS1_D5_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c2a, PAD_TS1_D5_OUT, BIT0),
        _RVM1(0x2c2a, PAD_TS1_D5_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_D6_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_D6_OEN (PAD_TS1_D6_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_D6_OUT (PAD_TS1_D6_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c2c, PAD_TS1_D6_OUT, BIT0),
        _RVM1(0x2c2c, PAD_TS1_D6_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_sm_config
        _RVM1(0x2914, 0, BIT1 | BIT0),   //reg[322914]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_D7_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_D7_OEN (PAD_TS1_D7_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_D7_OUT (PAD_TS1_D7_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c2e, PAD_TS1_D7_OUT, BIT0),
        _RVM1(0x2c2e, PAD_TS1_D7_OEN, BIT1),
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_SYNC_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_SYNC_OEN (PAD_TS1_SYNC_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_SYNC_OUT (PAD_TS1_SYNC_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c32, PAD_TS1_SYNC_OUT, BIT0),
        _RVM1(0x2c32, PAD_TS1_SYNC_OEN, BIT1),
        //reg_ej_diagnosis
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS1_VLD_IS_GPIO != GPIO_NONE)
        #define PAD_TS1_VLD_OEN (PAD_TS1_VLD_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS1_VLD_OUT (PAD_TS1_VLD_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c34, PAD_TS1_VLD_OUT, BIT0),
        _RVM1(0x2c34, PAD_TS1_VLD_OEN, BIT1),
        //reg_ej_diagnosis
        _RVM1(0x29e4, 0, BIT5 | BIT4),   //reg[3229e4]#5 ~ #4 = 00b
        //reg_test_in_mode
        //_RVM1(0x29f0, 0, BIT1 | BIT0),   //reg[3229f0]#1 ~ #0 = 00b
        //reg_test_out_mode
        //_RVM1(0x29f0, 0, BIT5 | BIT4),   //reg[3229f0]#5 ~ #4 = 00b
        //reg_ts1config
        _RVM1(0x2900, 0, 0x70),   //reg[322900]#6 ~ #4 = 000b
        //reg_ts_out_mode
        _RVM1(0x2906, 0, 0x07),   //reg[322906]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_CLK_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_CLK_OEN (PAD_TS2_CLK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_CLK_OUT (PAD_TS2_CLK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c50, PAD_TS2_CLK_OUT, BIT0),
        _RVM1(0x2c50, PAD_TS2_CLK_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_i2s_bt_md
        _RVM1(0x292c, 0, BIT1 | BIT0),   //reg[32292c]#1 ~ #0 = 00b
        //reg_mspi3_config
        _RVM1(0x2958, 0, 0x07),   //reg[322958]#2 ~ #0 = 000b
        //reg_seconduartmode
        _RVM1(0x2942, 0, BIT1 | BIT0),   //reg[322942]#1 ~ #0 = 00b
        //reg_od2nduart
        _RVM1(0x2942, 0, BIT5 | BIT4),   //reg[322942]#5 ~ #4 = 00b
        //reg_miic_mode0
        _RVM1(0x2950, 0, BIT1 | BIT0),   //reg[322950]#1 ~ #0 = 00b
        //reg_tconconfig10
        _RVM1(0x2985, 0, BIT0),   //reg[322985]#0 = 0b
        //reg_sdio_config
        _RVM1(0x2968, 0, BIT0),   //reg[322968]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_D0_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_D0_OEN (PAD_TS2_D0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_D0_OUT (PAD_TS2_D0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c40, PAD_TS2_D0_OUT, BIT0),
        _RVM1(0x2c40, PAD_TS2_D0_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_i2s_bt_md
        _RVM1(0x292c, 0, BIT1 | BIT0),   //reg[32292c]#1 ~ #0 = 00b
        //reg_mspi3_config
        _RVM1(0x2958, 0, 0x07),   //reg[322958]#2 ~ #0 = 000b
        //reg_thirduartmode
        _RVM1(0x2943, 0, BIT1 | BIT0),   //reg[322943]#1 ~ #0 = 00b
        //reg_od3rduart
        _RVM1(0x2943, 0, BIT5 | BIT4),   //reg[322943]#5 ~ #4 = 00b
        //reg_tconconfig5
        _RVM1(0x2982, 0, BIT5 | BIT4),   //reg[322982]#5 ~ #4 = 00b
        //reg_sdio_config
        _RVM1(0x2968, 0, BIT0),   //reg[322968]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_D1_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_D1_OEN (PAD_TS2_D1_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_D1_OUT (PAD_TS2_D1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c42, PAD_TS2_D1_OUT, BIT0),
        _RVM1(0x2c42, PAD_TS2_D1_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_D2_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_D2_OEN (PAD_TS2_D2_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_D2_OUT (PAD_TS2_D2_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c44, PAD_TS2_D2_OUT, BIT0),
        _RVM1(0x2c44, PAD_TS2_D2_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_D3_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_D3_OEN (PAD_TS2_D3_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_D3_OUT (PAD_TS2_D3_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c46, PAD_TS2_D3_OUT, BIT0),
        _RVM1(0x2c46, PAD_TS2_D3_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_D4_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_D4_OEN (PAD_TS2_D4_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_D4_OUT (PAD_TS2_D4_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c48, PAD_TS2_D4_OUT, BIT0),
        _RVM1(0x2c48, PAD_TS2_D4_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_ts4config
        _RVM1(0x2902, 0, BIT1 | BIT0),   //reg[322902]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_D5_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_D5_OEN (PAD_TS2_D5_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_D5_OUT (PAD_TS2_D5_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c4a, PAD_TS2_D5_OUT, BIT0),
        _RVM1(0x2c4a, PAD_TS2_D5_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_ts4config
        _RVM1(0x2902, 0, BIT1 | BIT0),   //reg[322902]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_D6_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_D6_OEN (PAD_TS2_D6_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_D6_OUT (PAD_TS2_D6_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c4c, PAD_TS2_D6_OUT, BIT0),
        _RVM1(0x2c4c, PAD_TS2_D6_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_ts4config
        _RVM1(0x2902, 0, BIT1 | BIT0),   //reg[322902]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_D7_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_D7_OEN (PAD_TS2_D7_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_D7_OUT (PAD_TS2_D7_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c4e, PAD_TS2_D7_OUT, BIT0),
        _RVM1(0x2c4e, PAD_TS2_D7_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_ts4config
        _RVM1(0x2902, 0, BIT1 | BIT0),   //reg[322902]#1 ~ #0 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_SYNC_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_SYNC_OEN (PAD_TS2_SYNC_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_SYNC_OUT (PAD_TS2_SYNC_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c52, PAD_TS2_SYNC_OUT, BIT0),
        _RVM1(0x2c52, PAD_TS2_SYNC_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_i2s_bt_md
        _RVM1(0x292c, 0, BIT1 | BIT0),   //reg[32292c]#1 ~ #0 = 00b
        //reg_mspi3_config
        _RVM1(0x2958, 0, 0x07),   //reg[322958]#2 ~ #0 = 000b
        //reg_seconduartmode
        _RVM1(0x2942, 0, BIT1 | BIT0),   //reg[322942]#1 ~ #0 = 00b
        //reg_od2nduart
        _RVM1(0x2942, 0, BIT5 | BIT4),   //reg[322942]#5 ~ #4 = 00b
        //reg_miic_mode0
        _RVM1(0x2950, 0, BIT1 | BIT0),   //reg[322950]#1 ~ #0 = 00b
        //reg_tconconfig9
        _RVM1(0x2984, 0, BIT4),   //reg[322984]#4 = 0b
        //reg_sdio_config
        _RVM1(0x2968, 0, BIT0),   //reg[322968]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS2_VLD_IS_GPIO != GPIO_NONE)
        #define PAD_TS2_VLD_OEN (PAD_TS2_VLD_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS2_VLD_OUT (PAD_TS2_VLD_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c54, PAD_TS2_VLD_OUT, BIT0),
        _RVM1(0x2c54, PAD_TS2_VLD_OEN, BIT1),
        //reg_ts2config
        _RVM1(0x2901, 0, 0x07),   //reg[322901]#2 ~ #0 = 000b
        //reg_i2s_bt_md
        _RVM1(0x292c, 0, BIT1 | BIT0),   //reg[32292c]#1 ~ #0 = 00b
        //reg_mspi3_config
        _RVM1(0x2958, 0, 0x07),   //reg[322958]#2 ~ #0 = 000b
        //reg_thirduartmode
        _RVM1(0x2943, 0, BIT1 | BIT0),   //reg[322943]#1 ~ #0 = 00b
        //reg_od3rduart
        _RVM1(0x2943, 0, BIT5 | BIT4),   //reg[322943]#5 ~ #4 = 00b
        //reg_tconconfig8
        _RVM1(0x2984, 0, BIT0),   //reg[322984]#0 = 0b
        //reg_sdio_config
        _RVM1(0x2968, 0, BIT0),   //reg[322968]#0 = 0b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS3_CLK_IS_GPIO != GPIO_NONE)
        #define PAD_TS3_CLK_OEN (PAD_TS3_CLK_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS3_CLK_OUT (PAD_TS3_CLK_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c70, PAD_TS3_CLK_OUT, BIT0),
        _RVM1(0x2c70, PAD_TS3_CLK_OEN, BIT1),
        //reg_ts3config
        _RVM1(0x2901, 0, BIT5 | BIT4),   //reg[322901]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS3_D0_IS_GPIO != GPIO_NONE)
        #define PAD_TS3_D0_OEN (PAD_TS3_D0_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS3_D0_OUT (PAD_TS3_D0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c60, PAD_TS3_D0_OUT, BIT0),
        _RVM1(0x2c60, PAD_TS3_D0_OEN, BIT1),
        //reg_ts3config
        _RVM1(0x2901, 0, BIT5 | BIT4),   //reg[322901]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS3_SYNC_IS_GPIO != GPIO_NONE)
        #define PAD_TS3_SYNC_OEN (PAD_TS3_SYNC_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS3_SYNC_OUT (PAD_TS3_SYNC_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c72, PAD_TS3_SYNC_OUT, BIT0),
        _RVM1(0x2c72, PAD_TS3_SYNC_OEN, BIT1),
        //reg_ts3config
        _RVM1(0x2901, 0, BIT5 | BIT4),   //reg[322901]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_TS3_VLD_IS_GPIO != GPIO_NONE)
        #define PAD_TS3_VLD_OEN (PAD_TS3_VLD_IS_GPIO == GPIO_IN ? BIT1: 0)
        #define PAD_TS3_VLD_OUT (PAD_TS3_VLD_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_32_,
        _RVM1(0x2c74, PAD_TS3_VLD_OUT, BIT0),
        _RVM1(0x2c74, PAD_TS3_VLD_OEN, BIT1),
        //reg_ts3config
        _RVM1(0x2901, 0, BIT5 | BIT4),   //reg[322901]#5 ~ #4 = 00b
        //reg_allpad_in
        //_RVM1(0x29e0, 0, BIT0),   //reg[3229e0]#0 = 0b
_MEMMAP_REGBANK_10_,
    #endif


//---------------------------------------------------------------------
// Pad Configuartion
//---------------------------------------------------------------------

    _MEMMAP_REGBANK_10_,

//---------------------------------------------------------------------
// Frame delay
#ifdef PADS_FRAM_DELAY_FLAG
#if (PADS_FRAM_DELAY_FLAG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_FRAM_DELAY_FLAG_MODE1 ((PADS_FRAM_DELAY_FLAG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2916, CONFIG_FRAM_DELAY_FLAG_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// JTAG
#ifdef PADS_EJ_CONFIG
#if (PADS_EJ_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EJ_CONFIG_MODE1 ((PADS_EJ_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_EJ_CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                (PADS_EJ_CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
    _RVM1(0x1e27, CONFIG_EJ_CONFIG_MODE1, BITMASK(1:0)),
#endif
#endif

// JTAG_DIAGNOSOS
#ifdef PADS_EJ_DIAGNOSIS
#if (PADS_EJ_DIAGNOSIS != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EJ_DIAGNOSIS_MODE1 ((PADS_EJ_DIAGNOSIS == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                   (PADS_EJ_DIAGNOSIS == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29e4, CONFIG_EJ_DIAGNOSIS_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TEST_IN
#ifdef PADS_TEST_IN_MODE
#if (PADS_TEST_IN_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TEST_IN_MODE_MODE1 ((PADS_TEST_IN_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                   (PADS_TEST_IN_MODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                   (PADS_TEST_IN_MODE == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29f0, CONFIG_TEST_IN_MODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TEST_OUT
#ifdef PADS_TEST_OUT_MODE
#if (PADS_TEST_OUT_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TEST_OUT_MODE_MODE1 ((PADS_TEST_OUT_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                    (PADS_TEST_OUT_MODE == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : \
                                    (PADS_TEST_OUT_MODE == CONFIG_PADMUX_MODE3) ? (0x03 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29f0, CONFIG_TEST_OUT_MODE_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// AGCDebug
#ifdef PADS_AGC_DBG
#if (PADS_AGC_DBG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_AGC_DBG_MODE1 ((PADS_AGC_DBG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29e8, CONFIG_AGC_DBG_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TserrOut
#ifdef PADS_TSERROUT
#if (PADS_TSERROUT != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TSERROUT_MODE1 ((PADS_TSERROUT == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                               (PADS_TSERROUT == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29e8, CONFIG_TSERROUT_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// VIFVSYNC
#ifdef PADS_VSYNC_VIF_OUT_EN
#if (PADS_VSYNC_VIF_OUT_EN != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_VSYNC_VIF_OUT_EN_MODE1 ((PADS_VSYNC_VIF_OUT_EN == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2979, CONFIG_VSYNC_VIF_OUT_EN_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// DISEQCOUT
#ifdef PADS_DISEQC_OUT_CONFIG
#if (PADS_DISEQC_OUT_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_DISEQC_OUT_CONFIG_MODE1 ((PADS_DISEQC_OUT_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2978, CONFIG_DISEQC_OUT_CONFIG_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// DISEQCIN
#ifdef PADS_DISEQC_IN_CONFIG
#if (PADS_DISEQC_IN_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_DISEQC_IN_CONFIG_MODE1 ((PADS_DISEQC_IN_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2978, CONFIG_DISEQC_IN_CONFIG_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// FreezeTuner
#ifdef PADS_FREEZE_TUNER
#if (PADS_FREEZE_TUNER != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_FREEZE_TUNER_MODE1 ((PADS_FREEZE_TUNER == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                   (PADS_FREEZE_TUNER == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                   (PADS_FREEZE_TUNER == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2979, CONFIG_FREEZE_TUNER_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ts0=1pin
// ts0=1sin
// ts0=mspi(mspi0)
// ts0=1sin(3wire)
#ifdef PADS_TS0CONFIG
#if (PADS_TS0CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TS0CONFIG_MODE1 ((PADS_TS0CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_TS0CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                (PADS_TS0CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : \
                                (PADS_TS0CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : \
                                (PADS_TS0CONFIG == CONFIG_PADMUX_MODE4) ? (0x04 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2900, CONFIG_TS0CONFIG_MODE1, BITMASK(2:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ts1=1pin
// ts1=1pout
// ts1=1sin
// ts1=1sin(3wire)
// ts1=mspi(mspi0)
#ifdef PADS_TS1CONFIG
#if (PADS_TS1CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TS1CONFIG_MODE1 ((PADS_TS1CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                (PADS_TS1CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : \
                                (PADS_TS1CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 4) : \
                                (PADS_TS1CONFIG == CONFIG_PADMUX_MODE4) ? (0x04 << 4) : \
                                (PADS_TS1CONFIG == CONFIG_PADMUX_MODE5) ? (0x05 << 4) : \
                                (PADS_TS1CONFIG == CONFIG_PADMUX_MODE6) ? (0x06 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2900, CONFIG_TS1CONFIG_MODE1, BITMASK(6:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ts1=1pout
// ts1=tso_1pout
// ts1=tso_1pout(s2p)
// ts1=tso_1pout(s2p1)
#ifdef PADS_TS_OUT_MODE
#if (PADS_TS_OUT_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TS_OUT_MODE_MODE1 ((PADS_TS_OUT_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                  (PADS_TS_OUT_MODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                  (PADS_TS_OUT_MODE == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : \
                                  (PADS_TS_OUT_MODE == CONFIG_PADMUX_MODE4) ? (0x04 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2906, CONFIG_TS_OUT_MODE_MODE1, BITMASK(2:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ts2=1pin
// ts2=1sin
// ts2=1sin(3wire)
// ts2=mspi(mspi0)
#ifdef PADS_TS2CONFIG
#if (PADS_TS2CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TS2CONFIG_MODE1 ((PADS_TS2CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_TS2CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                (PADS_TS2CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : \
                                (PADS_TS2CONFIG == CONFIG_PADMUX_MODE4) ? (0x04 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2901, CONFIG_TS2CONFIG_MODE1, BITMASK(2:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ts3=1sin
// ts3=mspi(mspi0)
#ifdef PADS_TS3CONFIG
#if (PADS_TS3CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TS3CONFIG_MODE1 ((PADS_TS3CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                (PADS_TS3CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2901, CONFIG_TS3CONFIG_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ts2=2sin
// ts2=2mspi(mspi0)
#ifdef PADS_TS4CONFIG
#if (PADS_TS4CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TS4CONFIG_MODE1 ((PADS_TS4CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_TS4CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2902, CONFIG_TS4CONFIG_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// SDR/DDR1.8V/3.3V, DQS at PAD_EMMC_IO8
// SDR3.3V
#ifdef PADS_NAND_MODE
#if (PADS_NAND_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_NAND_MODE_MODE1 ((PADS_NAND_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_NAND_MODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2964, CONFIG_NAND_MODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// SMARTCard
#ifdef PADS_SM_CONFIG
#if (PADS_SM_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_SM_CONFIG_MODE1 ((PADS_SM_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_SM_CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2914, CONFIG_SM_CONFIG_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PCM1CTRL
#ifdef PADS_PCMCTRLCONFIG
#if (PADS_PCMCTRLCONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PCMCTRLCONFIG_MODE1 ((PADS_PCMCTRLCONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2910, CONFIG_PCMCTRLCONFIG_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PCM1AD
#ifdef PADS_PCMADCONFIG
#if (PADS_PCMADCONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PCMADCONFIG_MODE1 ((PADS_PCMADCONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2910, CONFIG_PCMADCONFIG_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PCM2CTRL
#ifdef PADS_PCM2CTRLCONFIG
#if (PADS_PCM2CTRLCONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PCM2CTRLCONFIG_MODE1 ((PADS_PCM2CTRLCONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2911, CONFIG_PCM2CTRLCONFIG_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PCM2CTRL
#ifdef PADS_PCM2_CDN_CONFIG
#if (PADS_PCM2_CDN_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PCM2_CDN_CONFIG_MODE1 ((PADS_PCM2_CDN_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2911, CONFIG_PCM2_CDN_CONFIG_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// SPDIF_OUT
#ifdef PADS_SPDIFOUTCONFIG
#if (PADS_SPDIFOUTCONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_SPDIFOUTCONFIG_MODE1 ((PADS_SPDIFOUTCONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2930, CONFIG_SPDIFOUTCONFIG_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// SPDIF_IN
#ifdef PADS_SPDIFINCONFIG
#if (PADS_SPDIFINCONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_SPDIFINCONFIG_MODE1 ((PADS_SPDIFINCONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2930, CONFIG_SPDIFINCONFIG_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// SPDIF_OUT2
#ifdef PADS_SPDIFOUTCONFIG2
#if (PADS_SPDIFOUTCONFIG2 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_SPDIFOUTCONFIG2_MODE1 ((PADS_SPDIFOUTCONFIG2 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2931, CONFIG_SPDIFOUTCONFIG2_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// MIC_PDM
// MIC_I2S
// BT
// SPDIF_OUT
// SPDIF_OUT2
// MSPI (MCP C4x)
#ifdef PADS_MIC_MD
#if (PADS_MIC_MD != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_MIC_MD_MODE1 ((PADS_MIC_MD == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE5) ? (0x05 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE6) ? (0x06 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE7) ? (0x07 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE8) ? (0x08 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE9) ? (0x09 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE10) ? (0x010 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE11) ? (0x011 << 0) : \
                             (PADS_MIC_MD == CONFIG_PADMUX_MODE12) ? (0x012 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2920, CONFIG_MIC_MD_MODE1, BITMASK(3:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// I2S_IN
// MSPI (MCP C4x)
#ifdef PADS_I2S_IN_MD
#if (PADS_I2S_IN_MD != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_I2S_IN_MD_MODE1 ((PADS_I2S_IN_MD == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_I2S_IN_MD == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                (PADS_I2S_IN_MD == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : \
                                (PADS_I2S_IN_MD == CONFIG_PADMUX_MODE5) ? (0x05 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2924, CONFIG_I2S_IN_MD_MODE1, BITMASK(2:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// I2S_OUT_SD3
#ifdef PADS_I2S_IN_SD2_MD
#if (PADS_I2S_IN_SD2_MD != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_I2S_IN_SD2_MD_MODE1 ((PADS_I2S_IN_SD2_MD == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                    (PADS_I2S_IN_SD2_MD == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2924, CONFIG_I2S_IN_SD2_MD_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// I2S_OUT
#ifdef PADS_I2S_OUT_MD
#if (PADS_I2S_OUT_MD != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_I2S_OUT_MD_MODE1 ((PADS_I2S_OUT_MD == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                 (PADS_I2S_OUT_MD == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                 (PADS_I2S_OUT_MD == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2928, CONFIG_I2S_OUT_MD_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// I2S_IN_SD3
#ifdef PADS_I2S_OUT_SD2_MD
#if (PADS_I2S_OUT_SD2_MD != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_I2S_OUT_SD2_MD_MODE1 ((PADS_I2S_OUT_SD2_MD == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2928, CONFIG_I2S_OUT_SD2_MD_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// I2S_OUT_MCK
#ifdef PADS_I2S_OUT_MCK_MD
#if (PADS_I2S_OUT_MCK_MD != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_I2S_OUT_MCK_MD_MODE1 ((PADS_I2S_OUT_MCK_MD == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x292c, CONFIG_I2S_OUT_MCK_MD_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// BT
// I2S_OUT_SD3 & I2S_IN_SD3
#ifdef PADS_I2S_BT_MD
#if (PADS_I2S_BT_MD != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_I2S_BT_MD_MODE1 ((PADS_I2S_BT_MD == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_I2S_BT_MD == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x292c, CONFIG_I2S_BT_MD_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// I2S_OUT_MUTE
#ifdef PADS_I2SMUTEMODE
#if (PADS_I2SMUTEMODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_I2SMUTEMODE_MODE1 ((PADS_I2SMUTEMODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                  (PADS_I2SMUTEMODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x292b, CONFIG_I2SMUTEMODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// I2S_OUT_BCK
#ifdef PADS_I2S_OUT_BCK
#if (PADS_I2S_OUT_BCK != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_I2S_OUT_BCK_MODE1 ((PADS_I2S_OUT_BCK == CONFIG_PADMUX_MODE1) ? (0x00 << 4) : \
                                  (PADS_I2S_OUT_BCK == CONFIG_PADMUX_MODE2) ? (0x01 << 4) : \
                                  (PADS_I2S_OUT_BCK == CONFIG_PADMUX_MODE3) ? (0x02 << 4) : \
                                  (PADS_I2S_OUT_BCK == CONFIG_PADMUX_MODE4) ? (0x03 << 4) : (0x02 << 4))

_MEMMAP_REGBANK_32_,
    _RVM1(0x2e20, CONFIG_I2S_OUT_BCK_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// LDM_SPI0
// VSYNC_LIKE_FROM_LD
// LDM_SPI0
#ifdef PADS_VSYNC_LIKE_CONFIG
#if (PADS_VSYNC_LIKE_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_VSYNC_LIKE_CONFIG_MODE1 ((PADS_VSYNC_LIKE_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                        (PADS_VSYNC_LIKE_CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                        (PADS_VSYNC_LIKE_CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : \
                                        (PADS_VSYNC_LIKE_CONFIG == CONFIG_PADMUX_MODE4) ? (0x04 << 0) : \
                                        (PADS_VSYNC_LIKE_CONFIG == CONFIG_PADMUX_MODE5) ? (0x05 << 0) : \
                                        (PADS_VSYNC_LIKE_CONFIG == CONFIG_PADMUX_MODE6) ? (0x06 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29a4, CONFIG_VSYNC_LIKE_CONFIG_MODE1, BITMASK(2:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// 2-wiredMSP0
// 2-wiredMSPI-0atPM
#ifdef PADS_LD_SPI1_CONFIG
#if (PADS_LD_SPI1_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_LD_SPI1_CONFIG_MODE1 ((PADS_LD_SPI1_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                     (PADS_LD_SPI1_CONFIG == CONFIG_PADMUX_MODE2) ? (0x01 << 1) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29a0, CONFIG_LD_SPI1_CONFIG_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// 2-wiredMSPI-1
// 2-wiredMSPI-1atPM
#ifdef PADS_LD_SPI2_CONFIG
#if (PADS_LD_SPI2_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_LD_SPI2_CONFIG_MODE1 ((PADS_LD_SPI2_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                     (PADS_LD_SPI2_CONFIG == CONFIG_PADMUX_MODE2) ? (0x01 << 5) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29a0, CONFIG_LD_SPI2_CONFIG_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// 4wiredMSPIforLXXFCIC/DEMURAorCHNLocalDimming
// 4wiremspiforCHNDemura
// 4-wiredMSPIatPM
#ifdef PADS_LD_SPI3_CONFIG
#if (PADS_LD_SPI3_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_LD_SPI3_CONFIG_MODE1 ((PADS_LD_SPI3_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                     (PADS_LD_SPI3_CONFIG == CONFIG_PADMUX_MODE2) ? (0x01 << 1) : \
                                     (PADS_LD_SPI3_CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29a1, CONFIG_LD_SPI3_CONFIG_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// MSPI2, AMBIL
// MSPI2, AMBIL-2wire
#ifdef PADS_MSPI3_CONFIG
#if (PADS_MSPI3_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_MSPI3_CONFIG_MODE1 ((PADS_MSPI3_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                   (PADS_MSPI3_CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                   (PADS_MSPI3_CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : \
                                   (PADS_MSPI3_CONFIG == CONFIG_PADMUX_MODE4) ? (0x04 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2958, CONFIG_MSPI3_CONFIG_MODE1, BITMASK(2:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// 4-wired QSPI
// 4-wired QSPI , (demura)
#ifdef PADS_LD_QSPI_CONFIG
#if (PADS_LD_QSPI_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_LD_QSPI_CONFIG_MODE1 ((PADS_LD_QSPI_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                     (PADS_LD_QSPI_CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x295e, CONFIG_LD_QSPI_CONFIG_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// UART2
#ifdef PADS_SECONDUARTMODE
#if (PADS_SECONDUARTMODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_SECONDUARTMODE_MODE1 ((PADS_SECONDUARTMODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                     (PADS_SECONDUARTMODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                     (PADS_SECONDUARTMODE == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2942, CONFIG_SECONDUARTMODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ODUART2
#ifdef PADS_OD2NDUART
#if (PADS_OD2NDUART != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_OD2NDUART_MODE1 ((PADS_OD2NDUART == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                (PADS_OD2NDUART == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : \
                                (PADS_OD2NDUART == CONFIG_PADMUX_MODE3) ? (0x03 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2942, CONFIG_OD2NDUART_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// UART3
#ifdef PADS_THIRDUARTMODE
#if (PADS_THIRDUARTMODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_THIRDUARTMODE_MODE1 ((PADS_THIRDUARTMODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                    (PADS_THIRDUARTMODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                    (PADS_THIRDUARTMODE == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2943, CONFIG_THIRDUARTMODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ODUART3
#ifdef PADS_OD3RDUART
#if (PADS_OD3RDUART != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_OD3RDUART_MODE1 ((PADS_OD3RDUART == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                (PADS_OD3RDUART == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : \
                                (PADS_OD3RDUART == CONFIG_PADMUX_MODE3) ? (0x03 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2943, CONFIG_OD3RDUART_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// UART4
#ifdef PADS_FOURTHUARTMODE
#if (PADS_FOURTHUARTMODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_FOURTHUARTMODE_MODE1 ((PADS_FOURTHUARTMODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                     (PADS_FOURTHUARTMODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2944, CONFIG_FOURTHUARTMODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ODUART4
#ifdef PADS_OD4THUART
#if (PADS_OD4THUART != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_OD4THUART_MODE1 ((PADS_OD4THUART == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                (PADS_OD4THUART == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2944, CONFIG_OD4THUART_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// UART5
#ifdef PADS_FIFTHUARTMODE
#if (PADS_FIFTHUARTMODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_FIFTHUARTMODE_MODE1 ((PADS_FIFTHUARTMODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2945, CONFIG_FIFTHUARTMODE_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// ODUART5
#ifdef PADS_OD5THUART
#if (PADS_OD5THUART != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_OD5THUART_MODE1 ((PADS_OD5THUART == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2945, CONFIG_OD5THUART_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// FastUART
#ifdef PADS_FASTUARTMODE
#if (PADS_FASTUARTMODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_FASTUARTMODE_MODE1 ((PADS_FASTUARTMODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                   (PADS_FASTUARTMODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x294e, CONFIG_FASTUARTMODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// FastUART
#ifdef PADS_ODFASTUART
#if (PADS_ODFASTUART != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_ODFASTUART_MODE1 ((PADS_ODFASTUART == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                 (PADS_ODFASTUART == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x294e, CONFIG_ODFASTUART_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PWM0
#ifdef PADS_PWM0_MODE
#if (PADS_PWM0_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PWM0_MODE_MODE1 ((PADS_PWM0_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2990, CONFIG_PWM0_MODE_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PWM1
#ifdef PADS_PWM1_MODE
#if (PADS_PWM1_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PWM1_MODE_MODE1 ((PADS_PWM1_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2990, CONFIG_PWM1_MODE_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PWM2
// 2ndPWM2
#ifdef PADS_PWM2_MODE
#if (PADS_PWM2_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PWM2_MODE_MODE1 ((PADS_PWM2_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                (PADS_PWM2_MODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2991, CONFIG_PWM2_MODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PWM3
#ifdef PADS_PWM3_MODE
#if (PADS_PWM3_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PWM3_MODE_MODE1 ((PADS_PWM3_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                (PADS_PWM3_MODE == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2991, CONFIG_PWM3_MODE_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PWM_DAC0
#ifdef PADS_PWM_DAC0_MODE
#if (PADS_PWM_DAC0_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PWM_DAC0_MODE_MODE1 ((PADS_PWM_DAC0_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2992, CONFIG_PWM_DAC0_MODE_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PWM_DAC1
#ifdef PADS_PWM_DAC1_MODE
#if (PADS_PWM_DAC1_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_PWM_DAC1_MODE_MODE1 ((PADS_PWM_DAC1_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2992, CONFIG_PWM_DAC1_MODE_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// DDCR
// MIIC3
#ifdef PADS_DDCRMODE
#if (PADS_DDCRMODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_DDCRMODE_MODE1 ((PADS_DDCRMODE == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                               (PADS_DDCRMODE == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2934, CONFIG_DDCRMODE_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// MIIC0
#ifdef PADS_MIIC_MODE0
#if (PADS_MIIC_MODE0 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_MIIC_MODE0_MODE1 ((PADS_MIIC_MODE0 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                 (PADS_MIIC_MODE0 == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                 (PADS_MIIC_MODE0 == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2950, CONFIG_MIIC_MODE0_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// MIIC1
#ifdef PADS_MIIC_MODE1
#if (PADS_MIIC_MODE1 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_MIIC_MODE1_MODE1 ((PADS_MIIC_MODE1 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2950, CONFIG_MIIC_MODE1_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// MIIC2
#ifdef PADS_MIIC_MODE2
#if (PADS_MIIC_MODE2 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_MIIC_MODE2_MODE1 ((PADS_MIIC_MODE2 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                 (PADS_MIIC_MODE2 == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                 (PADS_MIIC_MODE2 == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2951, CONFIG_MIIC_MODE2_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON0
#ifdef PADS_TCONCONFIG0
#if (PADS_TCONCONFIG0 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG0_MODE1 ((PADS_TCONCONFIG0 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2980, CONFIG_TCONCONFIG0_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON1
#ifdef PADS_TCONCONFIG1
#if (PADS_TCONCONFIG1 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG1_MODE1 ((PADS_TCONCONFIG1 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2980, CONFIG_TCONCONFIG1_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON2
#ifdef PADS_TCONCONFIG2
#if (PADS_TCONCONFIG2 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG2_MODE1 ((PADS_TCONCONFIG2 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2981, CONFIG_TCONCONFIG2_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON3
#ifdef PADS_TCONCONFIG3
#if (PADS_TCONCONFIG3 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG3_MODE1 ((PADS_TCONCONFIG3 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2981, CONFIG_TCONCONFIG3_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON4
#ifdef PADS_TCONCONFIG4
#if (PADS_TCONCONFIG4 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG4_MODE1 ((PADS_TCONCONFIG4 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2982, CONFIG_TCONCONFIG4_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON5
#ifdef PADS_TCONCONFIG5
#if (PADS_TCONCONFIG5 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG5_MODE1 ((PADS_TCONCONFIG5 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                  (PADS_TCONCONFIG5 == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2982, CONFIG_TCONCONFIG5_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON6
#ifdef PADS_TCONCONFIG6
#if (PADS_TCONCONFIG6 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG6_MODE1 ((PADS_TCONCONFIG6 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2983, CONFIG_TCONCONFIG6_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON7
#ifdef PADS_TCONCONFIG7
#if (PADS_TCONCONFIG7 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG7_MODE1 ((PADS_TCONCONFIG7 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2983, CONFIG_TCONCONFIG7_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON8
#ifdef PADS_TCONCONFIG8
#if (PADS_TCONCONFIG8 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG8_MODE1 ((PADS_TCONCONFIG8 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2984, CONFIG_TCONCONFIG8_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON9
#ifdef PADS_TCONCONFIG9
#if (PADS_TCONCONFIG9 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG9_MODE1 ((PADS_TCONCONFIG9 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2984, CONFIG_TCONCONFIG9_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// TCON10
#ifdef PADS_TCONCONFIG10
#if (PADS_TCONCONFIG10 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_TCONCONFIG10_MODE1 ((PADS_TCONCONFIG10 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2985, CONFIG_TCONCONFIG10_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EXTINT0
#ifdef PADS_EXTINT0
#if (PADS_EXTINT0 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EXTINT0_MODE1 ((PADS_EXTINT0 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c0, CONFIG_EXTINT0_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EXTINT1
#ifdef PADS_EXTINT1
#if (PADS_EXTINT1 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EXTINT1_MODE1 ((PADS_EXTINT1 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c0, CONFIG_EXTINT1_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EXTINT2
#ifdef PADS_EXTINT2
#if (PADS_EXTINT2 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EXTINT2_MODE1 ((PADS_EXTINT2 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c1, CONFIG_EXTINT2_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EXTINT3
#ifdef PADS_EXTINT3
#if (PADS_EXTINT3 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EXTINT3_MODE1 ((PADS_EXTINT3 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c1, CONFIG_EXTINT3_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EXTINT4
#ifdef PADS_EXTINT4
#if (PADS_EXTINT4 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EXTINT4_MODE1 ((PADS_EXTINT4 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c2, CONFIG_EXTINT4_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EXTINT5
#ifdef PADS_EXTINT5
#if (PADS_EXTINT5 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EXTINT5_MODE1 ((PADS_EXTINT5 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c2, CONFIG_EXTINT5_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EXTINT6
#ifdef PADS_EXTINT6
#if (PADS_EXTINT6 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EXTINT6_MODE1 ((PADS_EXTINT6 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c3, CONFIG_EXTINT6_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// EXTINT7
#ifdef PADS_EXTINT7
#if (PADS_EXTINT7 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_EXTINT7_MODE1 ((PADS_EXTINT7 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c3, CONFIG_EXTINT7_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PM51GPIO0
#ifdef PADS_P1_ENABLE_B0
#if (PADS_P1_ENABLE_B0 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_P1_ENABLE_B0_MODE1 ((PADS_P1_ENABLE_B0 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c8, CONFIG_P1_ENABLE_B0_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PM51GPIO1
#ifdef PADS_P1_ENABLE_B1
#if (PADS_P1_ENABLE_B1 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_P1_ENABLE_B1_MODE1 ((PADS_P1_ENABLE_B1 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c8, CONFIG_P1_ENABLE_B1_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PM51GPIO2
#ifdef PADS_P1_ENABLE_B2
#if (PADS_P1_ENABLE_B2 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_P1_ENABLE_B2_MODE1 ((PADS_P1_ENABLE_B2 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c9, CONFIG_P1_ENABLE_B2_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PM51GPIO3
#ifdef PADS_P1_ENABLE_B3
#if (PADS_P1_ENABLE_B3 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_P1_ENABLE_B3_MODE1 ((PADS_P1_ENABLE_B3 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29c9, CONFIG_P1_ENABLE_B3_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PM51GPIO4
#ifdef PADS_P1_ENABLE_B4
#if (PADS_P1_ENABLE_B4 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_P1_ENABLE_B4_MODE1 ((PADS_P1_ENABLE_B4 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29ca, CONFIG_P1_ENABLE_B4_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PM51GPIO5
#ifdef PADS_P1_ENABLE_B5
#if (PADS_P1_ENABLE_B5 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_P1_ENABLE_B5_MODE1 ((PADS_P1_ENABLE_B5 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29ca, CONFIG_P1_ENABLE_B5_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PM51GPIO6
#ifdef PADS_P1_ENABLE_B6
#if (PADS_P1_ENABLE_B6 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_P1_ENABLE_B6_MODE1 ((PADS_P1_ENABLE_B6 == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29cb, CONFIG_P1_ENABLE_B6_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// PM51GPIO7
#ifdef PADS_P1_ENABLE_B7
#if (PADS_P1_ENABLE_B7 != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_P1_ENABLE_B7_MODE1 ((PADS_P1_ENABLE_B7 == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x29cb, CONFIG_P1_ENABLE_B7_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif

// SDIO
#ifdef PADS_SDIO_CONFIG
#if (PADS_SDIO_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_SDIO_CONFIG_MODE1 ((PADS_SDIO_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2968, CONFIG_SDIO_CONFIG_MODE1, BIT0),
_MEMMAP_REGBANK_10_,
#endif
#endif

// 3DFLAGmode
#ifdef PADS_3DFLAGCONFIG
#if (PADS_3DFLAGCONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_3DFLAGCONFIG_MODE1 ((PADS_3DFLAGCONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 0) : \
                                   (PADS_3DFLAGCONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 0) : \
                                   (PADS_3DFLAGCONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 0) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2970, CONFIG_3DFLAGCONFIG_MODE1, BITMASK(1:0)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// 3DFLAGmode
#ifdef PADS_OSD3DFLAG_CONFIG
#if (PADS_OSD3DFLAG_CONFIG != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_OSD3DFLAG_CONFIG_MODE1 ((PADS_OSD3DFLAG_CONFIG == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : \
                                       (PADS_OSD3DFLAG_CONFIG == CONFIG_PADMUX_MODE2) ? (0x02 << 4) : \
                                       (PADS_OSD3DFLAG_CONFIG == CONFIG_PADMUX_MODE3) ? (0x03 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x2970, CONFIG_OSD3DFLAG_CONFIG_MODE1, BITMASK(5:4)),
_MEMMAP_REGBANK_10_,
#endif
#endif

// External EARC mode
#ifdef PADS_LG_EARC_MODE
#if (PADS_LG_EARC_MODE != CONFIG_PADMUX_UNKNOWN)
#define CONFIG_LG_EARC_MODE_MODE1 ((PADS_LG_EARC_MODE == CONFIG_PADMUX_MODE1) ? (0x01 << 4) : 0)
_MEMMAP_REGBANK_32_,
    _RVM1(0x297c, CONFIG_LG_EARC_MODE_MODE1, BIT4),
_MEMMAP_REGBANK_10_,
#endif
#endif


    // Clear all pad in
    _RVM1(0x1EA1, 0, BIT7),

    _END_OF_TBL_,

//---------------------------------------------------------------------
// ISP_TOOL Write Protect
//---------------------------------------------------------------------

};

const U8 analogPadInitTbl[] =
{
    0x39, 0xB6, 0x5B, 0x53,     // magic code for ISP_Tool

    // programable device number
    // spi flash count
    1 + (PIN_SPI_CZ1 != 0) + (PIN_SPI_CZ2 != 0) + (PIN_SPI_CZ3 != 0),
    0x00,                       // nor
    0x00,                       // nand
    0x00,                       // reserved
    0x00,                       // reserved
    0x00,                       // reserved

//---------------------------------------------------------------------
// GPIO Configuartion
//---------------------------------------------------------------------

    #if(PAD_MICIN0_IS_GPIO != GPIO_NONE)
        #define PAD_MICIN0_OEN (PAD_MICIN0_IS_GPIO == GPIO_IN ? BIT6: 0)
        #define PAD_MICIN0_OUT (PAD_MICIN0_IS_GPIO == GPIO_OUT_HIGH ? BIT4: 0)
_MEMMAP_REGBANK_11_,
        _RVM1(0x2edc, PAD_MICIN0_OUT, BIT4),
        _RVM1(0x2edc, PAD_MICIN0_OEN, BIT6),
        _RVM1(0x2edc, BIT5, BIT5),   //reg[112EDC]#5=1b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_MICCM0_IS_GPIO != GPIO_NONE)
        #define PAD_MICCM0_OEN (PAD_MICCM0_IS_GPIO == GPIO_IN ? BIT2: 0)
        #define PAD_MICCM0_OUT (PAD_MICCM0_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_11_,
        _RVM1(0x2edc, PAD_MICCM0_OUT, BIT0),
        _RVM1(0x2edc, PAD_MICCM0_OEN, BIT2),
        _RVM1(0x2edc, BIT1, BIT1),   //reg[112EDC]#1=1b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_MICIN1_IS_GPIO != GPIO_NONE)
        #define PAD_MICIN1_OEN (PAD_MICIN1_IS_GPIO == GPIO_IN ? BIT6: 0)
        #define PAD_MICIN1_OUT (PAD_MICIN1_IS_GPIO == GPIO_OUT_HIGH ? BIT4: 0)
_MEMMAP_REGBANK_11_,
        _RVM1(0x2edd, PAD_MICIN1_OUT, BIT4),
        _RVM1(0x2edd, PAD_MICIN1_OEN, BIT6),
        _RVM1(0x2edd, BIT5, BIT5),   //reg[112EDD]#5=1b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PAD_MICCM1_IS_GPIO != GPIO_NONE)
        #define PAD_MICCM1_OEN (PAD_MICCM1_IS_GPIO == GPIO_IN ? BIT2: 0)
        #define PAD_MICCM1_OUT (PAD_MICCM1_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
_MEMMAP_REGBANK_11_,
        _RVM1(0x2edd, PAD_MICCM1_OUT, BIT0),
        _RVM1(0x2edd, PAD_MICCM1_OEN, BIT2),
        _RVM1(0x2edd, BIT1, BIT1),   //reg[112EDD]#1=1b
_MEMMAP_REGBANK_10_,
    #endif

    #if(PADA_RIN0P_IS_GPIO != GPIO_NONE)
        #define PADA_RIN0P_OEN (PADA_RIN0P_IS_GPIO == GPIO_IN ? 0:BIT0)
        #define PADA_RIN0P_OUT (PADA_RIN0P_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
        _RVM1(0x2537, PADA_RIN0P_OUT, BIT0),
        _RVM1(0x2536, PADA_RIN0P_OEN, BIT0),
        _RVM1(0x2534, BIT0, BIT0),   //reg[102534]#0=1b
        _RVM1(0x2503, 0x0, BIT6),    //reg[102503]#6=0b
    #endif

    #if(PADA_GIN0P_IS_GPIO != GPIO_NONE)
        #define PADA_GIN0P_OEN (PADA_GIN0P_IS_GPIO == GPIO_IN ? 0:BIT0)
        #define PADA_GIN0P_OUT (PADA_GIN0P_IS_GPIO == GPIO_OUT_HIGH ? BIT0: 0)
        _RVM1(0x2539, PADA_GIN0P_OUT, BIT0),
        _RVM1(0x2538, PADA_GIN0P_OEN, BIT0),
        _RVM1(0x2534, BIT0, BIT0),   //reg[102534]#0=1b
        _RVM1(0x2503, 0x0, BIT6),    //reg[102503]#6=0b
    #endif

//=============================================================================
    //reg_allpad_in
    _RVM1(0x1ea1, 0, BIT7),             //reg[101ea1]#7 = 0b
    _END_OF_TBL_,
};
