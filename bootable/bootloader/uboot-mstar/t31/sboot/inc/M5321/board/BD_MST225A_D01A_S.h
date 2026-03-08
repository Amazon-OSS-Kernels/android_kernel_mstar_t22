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
#ifndef _MSBOARD_H_
#define _MSBOARD_H_

#define BOARD_NAME                  "BD_MST225A_D01A"

#define PIN_SPI_CZ1                     PAD_PM_SPI_CZ
#define PIN_SPI_CZ2                     0
#define PIN_SPI_CZ3                     0
#define PIN_FLASH_WP0               0
#define PIN_FLASH_WP1               0

//------PAD MUX setting-------------------------------------------------------
#define Unknown_pad_mux         0
#define TCON0_MODE(X)           X //1: PAD_TCON0 2: PAD_UART_TX2
#define TCON1_MODE(X)           X //1: PAD_TCON1 2: PAD_UART_RX2
#define TCON2_MODE(X)           X //1: PAD_TCON2 2: PAD_GPIO0
#define TCON3_MODE(X)           X //1: PAD_TCON3 2: PAD_GPIO1
#define TCON4_MODE(X)           X //1: PAD_TCON4 2: PAD_GPIO2
#define TCON5_MODE(X)           X //1: PAD_TCON5 2: PAD_DDCR_DA
#define TCON6_MODE(X)           X //1: PAD_TCON6 2: PAD_DDCR_CK
#define TCON7_MODE(X)           X //1: PAD_TCON7

#define UART2_MODE(X)           X //(TX1,RX1) X=1: PAD_GPIO7/PAD_GPIO8,  X=2: PAD_GPIO19/PAD_GPIO20 X=3: PAD_GPIO3/PAD_GPIO4
#define UART3_MODE(X)           X //(TX2,RX2) X=1: PAD_GPIO9/PAD_GPIO10, X=2: PAD_I2S_IN_WS/PAD_I2S_IN_BCK X=3: PAD_UART_TX2/PAD_UART_RX2
#define UART4_MODE(X)           X //(TX3,RX3) x=1: PAD_GPIO11/PAD_GPIO12,X=2: PAD_GPIO23/PAD_GPIO24 X=3: PAD_EMMC_IO[0]/PAD_EMMC_IO[8]
#define UARTF_MODE(X)           X //(CTS0,RTS0) X=1: PAD_GPIO5/PAD_GPIO6,X=2: PAD_GPIO25/PAD_GPIO26 X=3: PAD_GPIO13/PAD_GPIO14
#define I2SIN_MODE(X)           X //1: PAD_I2S_IN_WS/PAD_I2S_IN_BCK/PAD_I2S_IN_SD
#define I2SIN_MASTER_MODE(X)    X //1: PAD_I2S_IN_WS/PAD_I2S_IN_BCK
#define I2SOUT_MODE(X)          X
    //1: single channel PAD_I2S_OUT_MCK/PAD_I2S_OUT_WS/PAD_I2S_OUT_BCK/PAD_I2S_OUT_SD
    //2: multi  channel PAD_I2S_OUT_MCK/PAD_I2S_OUT_WS/PAD_I2S_OUT_BCK/PAD_I2S_OUT_SD/PAD_I2S_OUT_SD1/PAD_I2S_OUT_SD2/PAD_I2S_OUT_SD3
#define ET_MODE(X)              X //1: PAD_GPIO19/PAD_GPIO22/PAD_GPIO20/PAD_GPIO24/PAD_GPIO27/PAD_GPIO21/PAD_GPIO26/PAD_GPIO23/PAD_GPIO25
#define LED_MODE(X)             X //1: PAD_GPIO19/GPIO20 2: PAD_GPIO26/PAD_GPIO27 3: PAD_TS1_VLD/PAD_EMMC_IO[1]
#define SPDIF_IN_MODE(X)        X //1: PAD_SPDIF_IN
#define SPDIF_OUT_MODE(X)       X //1: PAD_SPDIF_OUT
#define SPDIF2_OUT_MODE(X)      X //1: PAD_I2S_OUT_SD1
#define PWM0_MODE(X)            X //1: PAD_PWM0 2: PAD_PCM_A[6] 3:PAD_TS1_VLD
#define PWM1_MODE(X)            X //1: PAD_PWM1 2: PAD_PCM_A[7] 3:PAD_EMMC_IO[1]
#define PWM2_MODE(X)            X //1: PAD_PWM2
#define PWM3_MODE(X)            X //1: PAD_PWM3
#define PWM4_MODE(X)            X //1: PAD_PWM4
#define PWM5_MODE(X)            X //1: PAD_B_ODD_3 2: PAD_VSYNC_LIKE 3: PAD_TCON_0
#define DDCR_MODE(X)            X //1 and 2: PAD_DDCR_DA/PAD_DDCR_CK 2: PAD_DDCR_DA/PAD_DDCR_CK 3 and 4: PAD_TS1_CLK/PAD_TS1_SYNC
#define MIIC_MODE(X)            X //1: PAD_TCON3/PAD_TCON2, 2: PAD_NAND_AD6/PAD_NAND_AD7, 3: PAD_GPIO1/PAD_GPIO0, 4: PAD_TCON0/PAD_TCON2
#define PCMCTRL_MODE(X)         X //1: PAD_PCM_XXX
#define TS0_MODE(X)             X //1: P_IN, 2: S_IN,
#define TS1_MODE(X)             X //1: P_IN, 2: P_OUT, 3: S_IN, 4: MSPI 5: windermere scan mode
#define TSO_MODE(X)             X //1:
#define NAND_MODE(X)            X //1: PCM_D, 2: PCM_A, 3:NAND_AD 4:NAND_AD_DQS
#define DISEQCOUT_MODE(X)       X //1: PAD_TGPIO1, 2: PAD_GPIO3
#define DISEQCIN_MODE(X)        X //1: PAD_TGPIO0, 2: PAD_GPIO4
#define ARC_MODE(X)             X //1: PAD_ARC

//define the following values from 1
#define PADS_TCON0_MODE       TCON0_MODE(1)
#define PADS_TCON1_MODE       TCON1_MODE(1)
#define PADS_TCON2_MODE       TCON2_MODE(1)
#define PADS_TCON3_MODE       TCON3_MODE(1)
#define PADS_TCON4_MODE       TCON4_MODE(1)
#define PADS_TCON5_MODE       TCON5_MODE(1)
#define PADS_TCON6_MODE       TCON6_MODE(1)
#define PADS_TCON7_MODE       TCON7_MODE(1)
#define PADS_UART2_MODE      Unknown_pad_mux
#define PADS_UART3_MODE      Unknown_pad_mux
#define PADS_UART4_MODE      Unknown_pad_mux
#define PADS_UARTF_MODE      Unknown_pad_mux
#define PADS_3DLR_MODE       Unknown_pad_mux
#define PADS_TTL_MODE        Unknown_pad_mux
#define PADS_MHLCBUS_MODE    MHLCBUS_MODE(1)
#define PADS_MHLVBUS_MODE    MHLVBUS_MODE(1)
#define PADS_MHLDBG_MODE     Unknown_pad_mux
#define PADS_MHLDET_MODE     MHLDET_MODE(1)
#define PADS_DDCDA_MODE      DDCDA_MODE(1)
#define PADS_DDCDB_MODE      DDCDB_MODE(1)
#define PADS_DDCDC_MODE      DDCDC_MODE(1)
#define PADS_I2SIN_MODE      I2SIN_MODE(1)
#define PADS_I2SIN_MASTER_MODE Unknown_pad_mux
#define PADS_I2SOUT_MODE     I2SOUT_MODE(1)
#define PADS_ET_MODE         Unknown_pad_mux
#define PADS_LED_MODE        LED_MODE(1)
#define PADS_CEC_MODE        CEC_MODE(1)
#define PADS_SPDIFIN_MODE    SPDIF_IN_MODE(1)
#define PADS_SPDIFOUT_MODE   SPDIF_OUT_MODE(1)
#define PADS_SPDIFOUT2_MODE  Unknown_pad_mux
#define PADS_INTGPIO_MODE    Unknown_pad_mux
#define PADS_PWM0_MODE       PWM0_MODE(1)
#define PADS_PWM1_MODE       Unknown_pad_mux
#define PADS_PWM2_MODE       Unknown_pad_mux
#define PADS_PWM3_MODE       Unknown_pad_mux
#define PADS_PWM4_MODE       Unknown_pad_mux
#define PADS_PWM5_MODE       Unknown_pad_mux
#define PADS_DDCR_MODE       DDCR_MODE(1)
#define PADS_TS0_MODE        Unknown_pad_mux
#define PADS_TS1_MODE        Unknown_pad_mux
#define PADS_TSO_MODE        Unknown_pad_mux
#define PADS_TS2_MODE        Unknown_pad_mux
#define PADS_DISEQCIN_MODE   Unknown_pad_mux
#define PADS_DISEQCOUT_MODE  DISEQCOUT_MODE(1)
#define PADS_ARC_MODE        ARC_MODE(1)
#define PADS_NAND_MODE       NAND_MODE(2)
#define PADS_PCM1_CTRL       PCMCTRL_MODE(1)
//------GPIO setting(default GPIO pin level)------------------------------------
#define PAD_GPIO_PM0_IS_GPIO        GPIO_OUT_LOW     //LCD backlight control (high : off)
#define PAD_GPIO_PM4_IS_GPIO        GPIO_OUT_HIGH     //system power (low : off)
#define PAD_GPIO_PM7_IS_GPIO        GPIO_OUT_LOW     //I2S DAC mute (high : mute)
#define PAD_GPIO_PM8_IS_GPIO        GPIO_OUT_HIGH       //SPI_FLASH1 write protect(low:protect)
#define PAD_GPIO_PM10_IS_GPIO       GPIO_OUT_HIGH       // EarPhone mute (high : mute)
#define PAD_SAR2_IS_GPIO            GPIO_OUT_HIGH       // USB power (low : off)
#define PAD_SAR4_IS_GPIO            GPIO_OUT_LOW        //Panel power (low : on)
#define PAD_PWM1_IS_GPIO            GPIO_OUT_LOW        //CI power (high : power enable)
#define PAD_PWM4_IS_GPIO            GPIO_OUT_HIGH        // Class D Amp enable (high : enable)
#define PAD_GPIO5_IS_GPIO           GPIO_OUT_LOW        // PreAMP_EN (high : enable)
//------Chip Type---------------------------------------------------------------
#include "chip/MSD93B0TM3.h"

#define  SHARE_GND               	ENABLE

//------I2C devices-------------------------------------------------------------
#if USE_SW_I2C
#undef USE_SW_I2C
#define USE_SW_I2C                  1
#define USE_SW_I2C_HIGHSPEED        0
#endif

#define I2C_DEV_DATABASE            ((E_I2C_BUS_SYS << 8) | 0xA4)
#define I2C_DEV_HDCPKEY             ((E_I2C_BUS_SYS << 8) | 0xA8)
#define I2C_DEV_EDID_A0             ((E_I2C_BUS_DDCA0 << 8) | 0xA0)
#define I2C_DEV_EDID_D0             ((E_I2C_BUS_DDCD0 << 8) | 0xA0)
#define I2C_DEV_EDID_D1             ((E_I2C_BUS_DDCD1 << 8) | 0xA0)
#define I2C_DEV_EDID_D2             ((E_I2C_BUS_DDCD2 << 8) | 0xA0)

//----------------------------------------------------
#define TUNER_IIC_BUS               E_I2C_BUS_DDCD0
#define DEMOD_IIC_BUS               E_I2C_BUS_DDCD0
#define MHL_IIC_BUS                 E_I2C_BUS_DDCD0

#define RM_DEVICE_ADR               I2C_DEV_DATABASE
#define RM_HDCP_ADR                 I2C_DEV_HDCPKEY

//------Peripheral Device Setting-----------------------------------------------
#define PANEL_TYPE_SEL            	g_PNL_TypeSel//PNL_AU37_T370HW01_HD //PNL_AU20_T200XW02_WXGA//PNL_LG32_WXGA //PNL_AU19PW01_WXGA//PNL_AU20_T200XW02_WXGA //PNL_LG19_SXGA  //PNL_CMO22_WSXGA  //PNL_AU20_T200XW02_WXGA  // PNL_CMO22_WSXGA  // PNL_AU20_T200XW02_WXGA // PNL_AU17_EN05_SXGA
#define SATURN_FLASH_TYPE        	FLASH_TYPE_SERIAL
#define SATURN_FLASH_IC          	FLASH_IC_MX25L6445E
#define FLASH_SIZE                  FLASH_SIZE_4MB

#define ENABLE_DDC_RAM            	ENABLE
#define RM_EEPROM_TYPE          	RM_TYPE_24C64//RM_TYPE_24C512
#define DIGITAL_I2S_SELECT        	AUDIO_I2S_NONE

#define INPUT_AV_VIDEO_COUNT      	1
#define INPUT_SV_VIDEO_COUNT       	0
#define INPUT_YPBPR_VIDEO_COUNT    	1
#ifdef ATSC_SYSTEM
#define INPUT_SCART_VIDEO_COUNT   	0
#else
#define INPUT_SCART_VIDEO_COUNT    	1
#endif
#define INPUT_HDMI_VIDEO_COUNT     	3
#ifdef ATSC_SYSTEM
#define ENABLE_SCART_VIDEO         	0
#else
#define ENABLE_SCART_VIDEO        	1
#endif

//------Input Source Mux--------------------------------------------------------
#define INPUT_VGA_MUX               INPUT_PORT_ANALOG0
#define INPUT_VGA_SYNC_MUX          INPUT_PORT_ANALOG0_SYNC //SYNC port of VGA. There is a case which data and sync use different port.
#define INPUT_YPBPR_MUX             INPUT_PORT_ANALOG2
#define INPUT_YPBPR2_MUX            INPUT_PORT_NONE_PORT
#define INPUT_TV_YMUX               INPUT_PORT_YMUX_CVBS0
#define INPUT_AV_YMUX               INPUT_PORT_YMUX_CVBS0
#define INPUT_AV2_YMUX              INPUT_PORT_NONE_PORT
#define INPUT_AV3_YMUX              INPUT_PORT_NONE_PORT
#define INPUT_SV_YMUX               INPUT_PORT_NONE_PORT
#define INPUT_SV_CMUX               INPUT_PORT_NONE_PORT
#define INPUT_SV2_YMUX              INPUT_PORT_NONE_PORT
#define INPUT_SV2_CMUX              INPUT_PORT_NONE_PORT
#define INPUT_SCART_YMUX            INPUT_PORT_YMUX_CVBS1
#define INPUT_SCART_RGBMUX          INPUT_PORT_ANALOG1
#define INPUT_SCART_FB_MUX          SCART_FB1
#define INPUT_SCART2_YMUX           INPUT_PORT_NONE_PORT
#define INPUT_SCART2_RGBMUX         INPUT_PORT_NONE_PORT
#define INPUT_SCART2_FB_MUX         INPUT_PORT_NONE_PORT

#define INPUT_HDMI1_MUX             INPUT_PORT_DVI0
#define INPUT_HDMI2_MUX             INPUT_PORT_DVI1
#define INPUT_HDMI3_MUX             INPUT_PORT_DVI3

/// for PWS path mapping
#define PWS_INPUT_HDMI1_PATH        _HDMI1_
#define PWS_INPUT_HDMI2_PATH        _HDMI3_
#define PWS_INPUT_HDMI3_PATH        _HDMI4_

#define SCART_ID_SEL                0x02   // HSYNC1
#define SCART2_ID_SEL               0x04   // HSYNC2
//============================================
// Use ATSC code base audio path setting
//============================================
#define AUDIO_SOURCE_DTV            AUDIO_DSP1_DVB_INPUT
#define AUDIO_SOURCE_DTV2           AUDIO_DSP2_DVB_INPUT
#define AUDIO_SOURCE_ATV            AUDIO_DSP3_SIF_INPUT
#define AUDIO_SOURCE_PC             AUDIO_AUIN4_INPUT       // no PC line_in, so connect to AV input
#define AUDIO_SOURCE_YPBPR          AUDIO_AUIN2_INPUT
#define AUDIO_SOURCE_YPBPR2         AUDIO_NULL_INPUT
#define AUDIO_SOURCE_AV             AUDIO_AUIN4_INPUT
#define AUDIO_SOURCE_AV2            AUDIO_NULL_INPUT
#define AUDIO_SOURCE_AV3            AUDIO_NULL_INPUT
#define AUDIO_SOURCE_SV             AUDIO_NULL_INPUT
#define AUDIO_SOURCE_SV2            AUDIO_NULL_INPUT
#define AUDIO_SOURCE_SCART          AUDIO_AUIN3_INPUT
#define AUDIO_SOURCE_SCART2         AUDIO_NULL_INPUT
#define AUDIO_SOURCE_HDMI           AUDIO_HDMI_INPUT
#define AUDIO_SOURCE_HDMI2          AUDIO_HDMI_INPUT
#define AUDIO_SOURCE_HDMI3          AUDIO_HDMI_INPUT
#define AUDIO_SOURCE_DVI            AUDIO_SOURCE_PC         // default: AUDIO_SOURCE_PC
#define AUDIO_SOURCE_DVI2           AUDIO_SOURCE_PC
#define AUDIO_SOURCE_DVI3           AUDIO_SOURCE_PC
#define AUDIO_SOURCE_KTV            AUDIO_NULL_INPUT

#define AUDIO_PATH_MAIN_SPEAKER     AUDIO_T3_PATH_AUOUT3
#define AUDIO_PATH_HP               AUDIO_T3_PATH_AUOUT1
#define AUDIO_PATH_MONITOROUT       AUDIO_PATH_NULL
#define AUDIO_PATH_SCART            AUDIO_PATH_NULL   // always output ATV/DTV sound
#define AUDIO_PATH_SPDIF            AUDIO_PATH_NULL
#define AUDIO_PATH_SPEAKER          AUDIO_PATH_NULL
#define AUDIO_PATH_LINEOUT          AUDIO_PATH_NULL
#define AUDIO_PATH_SIFOUT           AUDIO_PATH_NULL
#define AUDIO_PATH_SCART1           AUDIO_PATH_NULL

#define AUDIO_OUTPUT_MAIN_SPEAKER   AUDIO_AUOUT3_OUTPUT
#define AUDIO_OUTPUT_HP             AUDIO_HP_OUTPUT         // default: AUDIO_HP_OUTPUT, if no headphone output, define NULL
#define AUDIO_OUTPUT_MONITOROUT     AUDIO_NULL_OUTPUT
#define AUDIO_OUTPUT_SCART          AUDIO_NULL_OUTPUT  // define NULL when no used
#define AUDIO_OUTPUT_LINEOUT        AUDIO_NULL_OUTPUT
#define AUDIO_OUTPUT_SIFOUT         AUDIO_NULL_OUTPUT

//-----------------------Add GPIO switch setting -------------------------
#define Switch_PC()                 _FUNC_NOT_USED()
#define Switch_YPBPR()              _FUNC_NOT_USED()
#define Switch_YPBPR2()             _FUNC_NOT_USED()
#define Switch_AV()                 _FUNC_NOT_USED()
#define Switch_AV2()                _FUNC_NOT_USED()
#define Switch_AV3()                _FUNC_NOT_USED()
#define Switch_SV()                 _FUNC_NOT_USED()
#define Switch_SV2()                _FUNC_NOT_USED()
#define Switch_SCART()              _FUNC_NOT_USED()
#define Switch_SCART2()             _FUNC_NOT_USED()
#define Switch_DVI()                MApi_XC_DVI_SwitchSrc(INPUT_PORT_DVI0)
#define Switch_DVI2()               MApi_XC_DVI_SwitchSrc(INPUT_PORT_DVI1)
#define Switch_DVI3()               _FUNC_NOT_USED()
#define Switch_DVI4()               _FUNC_NOT_USED()
#define Switch_DEFAULT()            _FUNC_NOT_USED()
#define Demod_Reset_On()            _FUNC_NOT_USED() //   mdrv_gpio_set_low( PIN_156 )
#define Demod_Reset_Off()           _FUNC_NOT_USED() //   mdrv_gpio_set_high( PIN_156 )
#define USBPowerOn()                _FUNC_NOT_USED()
#define USBPowerOff()               _FUNC_NOT_USED()

// HDMI switch Setting
#define HDMI_SWITCH_SELECT  HDMI_SWITCH_NONE
//===============================================================
//------MHL Setting-----------------------------------------------------------
#define ENABLE_MHL                              ENABLE
#define MHL_TYPE                                MHL_TYPE_INTERNAL
#define HDMI_PORT_FOR_MHL                       UI_INPUT_SOURCE_HDMI

//For Support MHL chip
#if (ENABLE_MHL == ENABLE)
#define INPUT_SUPPORT_MHL_PATH                  E_INPUT_SUPPORT_MHL_PORT_DVI0
#else
#define INPUT_SUPPORT_MHL_PATH                  E_INPUT_NOT_SUPPORT_MHL
#endif

//------Tuner Setting-----------------------------------------------------------
#ifdef ATSC_SYSTEM
#define TS_CLK_INV                      0
#define TS_PARALLEL_OUTPUT              1
#define FRONTEND_DEMOD_TYPE             EMBEDDED_ATSC_DEMOD
#define FRONTEND_TUNER_TYPE             SILAB_2158_TUNER_A20 //SILAB_2158_TUNER
#define FRONTEND_IF_MIXER_TYPE          XUGUANG_T126CWADC //PHILIPS_FQD1216_TUNER
#define FRONTEND_IF_DEMODE_TYPE         MSTAR_INTERN_VIF  //PHILIPS_TDA9886 //MSTAR_VIF_MSB1210   //MSTAR_VIF //PHILIPS_TDA9886
#else
#define TS_CLK_INV                      0
#define TS_PARALLEL_OUTPUT              1
#define TS_SERIAL_OUTPUT_IF_CI_REMOVED  1
#define FRONTEND_DEMOD_TYPE             EMBEDDED_DVBT_DEMOD
#define FRONTEND_TUNER_TYPE             MxL_601SI_TUNER	//NXP_FH2608_TUNER

#define FRONTEND_IF_MIXER_TYPE          XUGUANG_T126CWADC //PHILIPS_FQD1216_TUNER
#define FRONTEND_IF_DEMODE_TYPE         MSTAR_INTERN_VIF  //PHILIPS_TDA9886 //MSTAR_VIF_MSB1210   //MSTAR_VIF //PHILIPS_TDA9886
#endif
#define FRONTEND_SECOND_DEMOD_TYPE      MSTAR_NONE_DEMOD

//#define BD_MST072A_D01A_WITH_ONBOARD_TUNER_ATV

#define VIF_TUNER_TYPE                  1                   // 0: RF Tuner; 1: Silicon Tuner
#define VIF_SAW_ARCH                    2                   // 0: Dual SAW; 1: external Single SAW; 2:silicon tuner; 3: no saw(VIF); 4: internal single SAW; 5: no SAW(DIF)

#define MSB1210_TS_SERIAL_INVERSION                     0
#define MSB1210_TS_PARALLEL_INVERSION                   1
#define MSB1210_DTV_DRIVING_LEVEL                       1 //0 or 1
#define MSB1210_WEAK_SIGNAL_PICTURE_FREEZE_ENABLE       1

#define INTERN_DVBT_TS_SERIAL_INVERSION                 0
#define INTERN_DVBT_TS_PARALLEL_INVERSION               1
#define INTERN_DVBT_DTV_DRIVING_LEVEL                   1
#define INTERN_DVBT_WEAK_SIGNAL_PICTURE_FREEZE_ENABLE   1

#define SECAM_L_PRIME_ON()              _FUNC_NOT_USED()
#define SECAM_L_PRIME_OFF()             _FUNC_NOT_USED()
#define EXT_RF_AGC_ON()                 _FUNC_NOT_USED() // ATV mode: external RF AGC
#define EXT_RF_AGC_OFF()                _FUNC_NOT_USED() // DTV mode: internal RF AGC

//------IR & Key Setting--------------------------------------------------------
#define IR_TYPE_SEL                     IR_TYPE_MSTAR_DTV   // IR_TYPE_MSTAR_DTV // IR_TYPE_CUS03_DTV // IR_TYPE_NEW
#define KEYPAD_TYPE_SEL                 KEYPAD_TYPE_ORIG    // KEYPAD_TYPE_DEMO
#define POWER_KEY_SEL                   POWER_KEY_PAD_INT

//------Power Setting-----------------------------------------------------------
#define ENABLE_POWER_SAVING             0
#define POWER_DOWN_SEQ                  1
#define POWER_SAVING_T                  0
#define SCREENSAVER_ENABLE              1
#define NO_SIGNAL_AUTO_SHUTDOWN         1
#define STANDBY_MODE                    POWERMODE_S3
#define POWERUP_MODE                    PUMODE_WORK
#define ENABLE_POWER_GOOD_DETECT        1
#define ENABLE_POWER_SAVING_SIF         1
#define ENABLE_POWER_SAVING_VDMVD       0
#define ENABLE_POWER_SAVING_DPMS        1
#define ENABLE_POWER_SAVING_DPMS_DVI    0

#define ENABLE_PWS                      1   // Analog IP PWS
#define ENABLE_DIP_PWS                  0   //Digital IP PWS, for U3 only now
#define ENABLE_DIP_MONITOR              0   //For U3 oly now


//------Memory Setting----------------------------------------------------------
#define BOOTUP_MIU_BIST                 1
#ifndef MEMORY_MAP
#define MEMORY_MAP                      MMAP_128MB
#endif

#if (ENABLE_MSTAR_TITANIA_MMAP_64MB)
#define  MIU_INTERFACE                  DDR2_INTERFACE_BGA
#elif (ENABLE_MSTAR_TITANIA_MMAP_128MB) || (ENABLE_MSTAR_TITANIA_MMAP_256MB)\
    || (ENABLE_MSTAR_TITANIA_MMAP_128MB_128MB) || (ENABLE_MSTAR_TITANIA_MMAP_256MB_256MB)
#define  MIU_INTERFACE                  DDR3_INTERFACE_BGA
#else
#error "Please select correct memory map for DDR2(64M) and DDR3(128M)"
#endif

//------Analog Function Setting-------------------------------------------------
#define MOD_LVDS_GPIO                   0x820

#define LVDS_PN_SWAP_L                  0xFF
#define LVDS_PN_SWAP_H                  0x3F

#define ENABLE_SSC                      DISABLE
#define ENABLE_LVDSTORGB_CONVERTER      DISABLE
#if ENABLE_SSC
#define MIU_SSC_SPAN_DEFAULT            25
#define MIU_SSC_STEP_DEFAULT            10
#define MIU_SSC_SPAN_MAX                40
#define MIU_SSC_STEP_MAX                20
#define LVDS_SSC_SPAN_DEFAULT           350
#define LVDS_SSC_STEP_DEFAULT           200
#define LVDS_SSC_SPAN_MAX               500
#define LVDS_SSC_STEP_MAX               300
#endif

//------ETHNET PHY_TYPE---------------------------------------------------------
#define ETHNET_PHY_LAN8700              0x0f
#define ETHNET_PHY_IP101ALF             0x01
#define ETHNET_PHY_TYPE                 ETHNET_PHY_IP101ALF

//------DRAM Config---------------------------------------------------------------
#define DRAM_TYPE                       DDR_II
#define DRAM_BUS                        DRAM_BUS_16
#define DDRPLL_FREQ                     DDRLLL_FREQ_400
#define DDRII_ODT

#define MIU_0_02                        0x0C45
#define MIU_0_1A                        0x5151
#define MIU_0_36                        0x0244
#define MIU_0_38                        0x0070

//------MCU use Scaler internal MPLL clock-------------------
#define MCU_CLOCK_SEL                   MCUCLK_144MHZ

#define MST_XTAL_CLOCK_HZ               FREQ_12MHZ
#define MST_XTAL_CLOCK_KHZ              (MST_XTAL_CLOCK_HZ/1000UL)
#define MST_XTAL_CLOCK_MHZ              (MST_XTAL_CLOCK_KHZ/1000UL)

//------MCU Code----------------------------------------------------------------
#define ENABLE_HKMCU_ICACHE_BYPASS      0
#define ENABLE_HKMCU_CODE_ECC           0

//------Extra-------------------------------------------------------------------
#define POWER_DOWN_INFORM_EXTERNALMCU   0

#if POWER_DOWN_INFORM_EXTERNALMCU
#define EXMCU_SLAVE_ADDR                0xA8
#define EXMCU_SUBADDRESS                0x04
#define EXMCU_SLEEP_MODE                0x00
#endif

#define IIC_BY_HW                       0 //
#define IIC_BY_SW                       1 //
#define _EEPROM_ACCESS                  IIC_BY_SW//IIC_BY_HW
#define EEPROM_CLK_SEL                  EEPROM_CLK_100KHZ

//------MST I/O control definition----------------------------------------------
#define ENABLE_DPWM_FUNCTION            0

//-------------------------------------------------
#define SCART_OUT_ON()                  _FUNC_NOT_USED()
#define SCART_OUT_OFF()                 _FUNC_NOT_USED()

// Video switch Setting
#define Switch_YPbPr1()                 _FUNC_NOT_USED()
#define Switch_YPbPr2()                 _FUNC_NOT_USED()

#define SwitchRGBToSCART()              _FUNC_NOT_USED()
#define SwitchRGBToDSUB()               _FUNC_NOT_USED()

#define MDrv_Sys_GetUsbOcdN()           _FUNC_NOT_USED()
#define MDrv_Sys_GetRgbSw()             _FUNC_NOT_USED()

//MHL
#define MHL_Reset_High()               _FUNC_NOT_USED()
#define MHL_Reset_Low()                _FUNC_NOT_USED()

// Audio Amplifier
#define Audio_Amplifier_ON()            mdrv_gpio_set_low( PAD_PWM_PM )// mdrv_gpio_set_high( PIN_72 )
#define Audio_Amplifier_OFF()           mdrv_gpio_set_high (PAD_PWM_PM ) // mdrv_gpio_set_low( PIN_72 )

#define Adj_Volume_Off()                mdrv_gpio_set_low( PAD_GPIO_PM7 )
#define Adj_Volume_On()                 mdrv_gpio_set_high (PAD_GPIO_PM7 )

#define Panel_VCC_ON()                  mdrv_gpio_set_low (PAD_SAR4 ) // mdrv_gpio_set_low( PIN_165 )
#define Panel_VCC_OFF()                 mdrv_gpio_set_high( PAD_SAR4 ) // mdrv_gpio_set_high( PIN_165 )

#define Panel_Backlight_VCC_ON()        mdrv_gpio_set_low( PAD_GPIO_PM0 ) // mdrv_gpio_set_low( PIN_165 )
#define Panel_Backlight_VCC_OFF()       mdrv_gpio_set_high( PAD_GPIO_PM0 ) // mdrv_gpio_set_high( PIN_165 )

#define Panel_Backlight_PWM_ADJ(x)      MDrv_PWM_DutyCycle(E_PWM_CH2, x)
#define Panel_Backlight_Max_Current(x)  MDrv_PWM_DutyCycle(E_PWM_CH2, x)

#define Panel_VG_HL_CTL_ON()            _FUNC_NOT_USED()
#define Panel_VG_HL_CTL_OFF()           _FUNC_NOT_USED()

// PCMCIA power control
#define PCMCIA_VCC_ON()                 _FUNC_NOT_USED()
#define PCMCIA_VCC_OFF()                _FUNC_NOT_USED()

// Power Saving
#define Power_On()                      _FUNC_NOT_USED()
#define Power_Off()                     _FUNC_NOT_USED()
#define MDrv_Sys_GetSvideoSw()          _FUNC_NOT_USED()

#define Peripheral_Device_Reset_ON()    _FUNC_NOT_USED()
#define Peripheral_Device_Reset_OFF()   _FUNC_NOT_USED()
#define Tuner_ON()                      _FUNC_NOT_USED()
#define Tuner_OFF()                     _FUNC_NOT_USED()
#define Demodulator_ON()                _FUNC_NOT_USED()
#define Demodulator_OFF()               _FUNC_NOT_USED()
#define LAN_ON()                        _FUNC_NOT_USED()
#define LAN_OFF()                       _FUNC_NOT_USED()

#define TunerOffPCMCIA()                _FUNC_NOT_USED()
#define TunerOnPCMCIA()                 _FUNC_NOT_USED()

// LED Control
#define LED_RED_ON()                    _FUNC_NOT_USED()
#define LED_RED_OFF()                   _FUNC_NOT_USED()
#define LED_GREEN_ON()                  _FUNC_NOT_USED()
#define LED_GREEN_OFF()                 _FUNC_NOT_USED()

#define ST_DET_Read()                   0
#define ANT_5V_MNT_Read()               0
#define TU_ERROR_N_Read()               0
#define HDMI_5V_Read()                  0
#define COMP_SW_Read()                  1
#define PANEL_CTL_Off()                 Panel_VCC_OFF()
#define PANEL_CTL_On()                  Panel_VCC_ON()
#define INV_CTL_Off()                   Panel_Backlight_VCC_OFF()
#define INV_CTL_On()                    Panel_Backlight_VCC_ON()
#define POWER_ON_OFF1_On()              Power_On()
#define POWER_ON_OFF1_Off()             Power_Off()
#define MUTE_On()                       Adj_Volume_Off()
#define MUTE_Off()                      Adj_Volume_On()
#define EEPROM_WP_On()                  _FUNC_NOT_USED()
#define EEPROM_WP_Off()                 _FUNC_NOT_USED()
#define LED_GRN_Off()                   LED_GREEN_OFF()
#define LED_GRN_On()                    LED_GREEN_ON()
#define LED_RED_Off()                   LED_GRN_On()
#define LED_RED_On()                    LED_GRN_Off()
#define ANT_5V_CTL_Off()                _FUNC_NOT_USED()
#define ANT_5V_CTL_On()                 _FUNC_NOT_USED()
#define BOOSTER_Off()                   _FUNC_NOT_USED()
#define BOOSTER_On()                    _FUNC_NOT_USED()
#define RGB_SW_On()                     _FUNC_NOT_USED()
#define RGB_SW_Off()                    _FUNC_NOT_USED()
#define SC_RE1_On()                     _FUNC_NOT_USED()
#define SC_RE1_Off()                    _FUNC_NOT_USED()
#define SC_RE2_On()                     _FUNC_NOT_USED()
#define SC_RE2_Off()                    _FUNC_NOT_USED()
#define TU_RESET_N_On()                 _FUNC_NOT_USED()
#define TU_RESET_N_Off()                _FUNC_NOT_USED()
#define DeactivateScartRecord1()        _FUNC_NOT_USED()
#define ActivateScartRecord1()          _FUNC_NOT_USED()
#define DeactivateScartRecord2()        _FUNC_NOT_USED()
#define ActivateScartRecord2()          _FUNC_NOT_USED()
#define USBPowerOn()                    _FUNC_NOT_USED()
#define USBPowerOff()                   _FUNC_NOT_USED()

//------MST Keypad definition---------------------------------------------------
#define KEYPAD_CHANNEL_SUPPORT          4 //Maximun supported keypad channels
#define ADC_KEY_CHANNEL_NUM             1 //Real supported keypad channels
#define ADC_KEY_LAST_CHANNEL            ADC_KEY_CHANNEL_NUM - 1
//config which keypad channel enabled
#define ENABLE_KPDCHAN_1                ENABLE
#define ENABLE_KPDCHAN_2                DISABLE
#define ENABLE_KPDCHAN_3                DISABLE
#define ENABLE_KPDCHAN_4                DISABLE

#define KEYPAD_KEY_VALIDATION           3
#define KEYPAD_REPEAT_KEY_CHECK         KEYPAD_KEY_VALIDATION + 2
#define KEYPAD_REPEAT_KEY_CHECK_1       KEYPAD_KEY_VALIDATION + 3
#define KEYPAD_STABLE_NUM               10
#define KEYPAD_STABLE_NUM_MIN           9
#define KEYPAD_REPEAT_PERIOD            2 // 6
#define KEYPAD_REPEAT_PERIOD_1          KEYPAD_REPEAT_PERIOD/2

//------------------------------------------------------------------------------
// SAR boundary define
//------------------------------------------------------------------------------
#define KEYPAD_CH1_UB                   0xFF
#define KEYPAD_CH1_LB                   0x70//0x80
#define KEYPAD_CH2_UB                   0xFF
#define KEYPAD_CH2_LB                   0x70//0x80
#define KEYPAD_CH3_UB                   0xFF
#define KEYPAD_CH3_LB                   0x70//0x80
#define KEYPAD_CH4_UB                   0xFF
#define KEYPAD_CH4_LB                   0x70//0x80

//### MAX support 8 level for each channel
#define ADC_KEY_LEVEL                   8
#define ADC_KEY_L0                      0x12
#define ADC_KEY_L1                      0x36
#define ADC_KEY_L2                      0x56
#define ADC_KEY_L3                      0x7B
#define ADC_KEY_LEVEL_MAX               ADC_KEY_L3

//### Currently Only Support 2 keypad Channels
//### Support un-balanced levels for each channel by spec. requirement
#define ADC_CH1_LEVELS                  4 //### must be <= ADC_KEY_LEVEL
#define ADC_CH2_LEVELS                  4 //### must be <= ADC_KEY_LEVEL
#define ADC_CH3_LEVELS                  4 //### must be <= ADC_KEY_LEVEL
#define ADC_CH4_LEVELS                  4 //### must be <= ADC_KEY_LEVEL

#define ADC_KEY_1_L0                    0x10//0x27
#define ADC_KEY_1_L1                    0x30//0x47
#define ADC_KEY_1_L2                    0x50//0x63
#define ADC_KEY_1_L3                    0x6A//0x7B
#define ADC_KEY_1_L4                    0x00//RFU
#define ADC_KEY_1_L5                    0x00//RFU
#define ADC_KEY_1_L6                    0x00//RFU
#define ADC_KEY_1_L7                    0x00//RFU

#define ADC_KEY_2_L0                    0x10//0x27
#define ADC_KEY_2_L1                    0x30//0x47
#define ADC_KEY_2_L2                    0x50//0x63
#define ADC_KEY_2_L3                    0x6A//0x7B
#define ADC_KEY_2_L4                    0x00//RFU
#define ADC_KEY_2_L5                    0x00//RFU
#define ADC_KEY_2_L6                    0x00//RFU
#define ADC_KEY_2_L7                    0x00//RFU

#if (KEYPAD_TYPE_SEL == KEYPAD_TYPE_CUSTMOER)   // CUSTMOER keypad
#define ADC_KEY_1_L0_FLAG               IRKEY_UP
#define ADC_KEY_1_L1_FLAG               IRKEY_MENU
#define ADC_KEY_1_L2_FLAG               IRKEY_LEFT
#define ADC_KEY_1_L3_FLAG               IRKEY_MUTE
#define ADC_KEY_1_L4_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L5_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L6_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L7_FLAG               IRKEY_DUMY//RFU

#define ADC_KEY_2_L0_FLAG               IRKEY_POWER
#define ADC_KEY_2_L1_FLAG               IRKEY_INPUT_SOURCE
#define ADC_KEY_2_L2_FLAG               IRKEY_RIGHT
#define ADC_KEY_2_L3_FLAG               IRKEY_DOWN
#define ADC_KEY_2_L4_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L5_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L6_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L7_FLAG               IRKEY_DUMY//RFU
#elif (KEYPAD_TYPE_SEL == KEYPAD_TYPE_ORIG)   // MStar normal keypad
#define ADC_KEY_1_L0_FLAG               IRKEY_UP
#define ADC_KEY_1_L1_FLAG               IRKEY_MENU
#define ADC_KEY_1_L2_FLAG               IRKEY_LEFT
#define ADC_KEY_1_L3_FLAG               IRKEY_MUTE
#define ADC_KEY_1_L4_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L5_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L6_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L7_FLAG               IRKEY_DUMY//RFU

#define ADC_KEY_2_L0_FLAG               IRKEY_POWER
#define ADC_KEY_2_L1_FLAG               IRKEY_INPUT_SOURCE
#define ADC_KEY_2_L2_FLAG               IRKEY_RIGHT
#define ADC_KEY_2_L3_FLAG               IRKEY_DOWN
#define ADC_KEY_2_L4_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L5_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L6_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L7_FLAG               IRKEY_DUMY//RFU
#elif (KEYPAD_TYPE_SEL == KEYPAD_TYPE_DEMO) // MStar demo set keypad
#define ADC_KEY_1_L0_FLAG               IRKEY_MUTE
#define ADC_KEY_1_L1_FLAG               IRKEY_VOLUME_MINUS
#define ADC_KEY_1_L2_FLAG               IRKEY_VOLUME_PLUS
#define ADC_KEY_1_L3_FLAG               IRKEY_DOWN
#define ADC_KEY_1_L4_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L5_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L6_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_1_L7_FLAG               IRKEY_DUMY//RFU

#define ADC_KEY_2_L0_FLAG               IRKEY_POWER
#define ADC_KEY_2_L1_FLAG               IRKEY_UP
#define ADC_KEY_2_L2_FLAG               IRKEY_MENU
#define ADC_KEY_2_L3_FLAG               IRKEY_INPUT_SOURCE
#define ADC_KEY_2_L4_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L5_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L6_FLAG               IRKEY_DUMY//RFU
#define ADC_KEY_2_L7_FLAG               IRKEY_DUMY//RFU
#endif

//-----PIN_OUT_SELECT------------------------------------------------------------------------
//PWMX Period=( PWMX_PERIOD+1 ) *( 1/ Xtal) //X=0~5
//PWMX_duty= (Init_PwmX_DUTY +1 ) * (1/XTAL) //X=0~5
#if (PADS_PWM0_MODE!=Unknown_pad_mux)
#define PWM0_PERIOD                     0xff
#define INIT_PWM0_DUTY                  0x7e
#define PWM0_DIV                        0x00
#endif
#if (PADS_PWM1_MODE!=Unknown_pad_mux)
#define PWM1_PERIOD                     0xff
#define INIT_PWM1_DUTY                  0x7e
#endif
#if (PADS_PWM2_MODE!=Unknown_pad_mux)
#define PWM2_PERIOD                     0xff
#define INIT_PWM2_DUTY                  0x7e
#endif
#if (PADS_PWM3_MODE!=Unknown_pad_mux) //Especially for Tuner+30V Power
#define PWM3_PERIOD                     0x23
#define INIT_PWM3_DUTY                  0x0A
#endif
#if (PADS_PWM4_MODE!=Unknown_pad_mux)
#define PWM4_PERIOD                     0xff
#define INIT_PWM4_DUTY                  0x7e
#endif
#if (PADS_PWM5_MODE!=Unknown_pad_mux)
#define PWM5_PERIOD                     0xff
#define INIT_PWM5_DUTY                  0x7e
#endif

#define BACKLITE_INIT_SETTING           ENABLE

#define PWM0_MUX_SEL                    0x00
//#define PWM2_MUX_SEL                    0x00

//------8051 Serial Port Setting------------------------------------------------
#if OBA2
//----------------------------------------------------------------------------
// UART_SRC_SEL
//----------------------------------------------------------------------------
#define UART0_SRC_SEL                   (UART_SEL_PIU_UART0)
#define UART1_SRC_SEL                   (UART_SEL_MHEG5)
#endif

#if(ENABLE_UART1_DEBUG)
#define ENABLE_UART0                    DISABLE
#define ENABLE_UART0_INTERRUPT          DISABLE
#define ENABLE_UART1                    ENABLE
#define ENABLE_UART1_INTERRUPT          ENABLE
//------STDIO device setting----------------------------------------------------
#define STDIN_DEVICE                    IO_DEV_UART1
#define STDOUT_DEVICE                   IO_DEV_UART1
#else
#define ENABLE_UART0                    ENABLE
#define ENABLE_UART0_INTERRUPT          ENABLE
#define ENABLE_UART1                    DISABLE
#define ENABLE_UART1_INTERRUPT          DISABLE
//------STDIO device setting----------------------------------------------------
#define STDIN_DEVICE                    IO_DEV_UART0
#define STDOUT_DEVICE                   IO_DEV_UART0
#endif

#define ENABLE_PIU_UART0                DISABLE
#define ENABLE_PIU_UART0_INTERRUPT      DISABLE

#define ENABLE_PIU_UART1                DISABLE
#define ENABLE_PIU_UART1_INTERRUPT      DISABLE

//------ PANEL RELATED ---------------------------------------------------------
#define PANEL_PDP_10BIT                 1
#define PANEL_SWAP_PORT                 0
#define PANEL_CONNECTOR_SWAP_LVDS_CH    0
#define PANEL_CONNECTOR_SWAP_LVDS_POL   0
#define PANEL_CONNECTOR_SWAP_PORT       1

#define PANEL_SWAP_LVDS_POL             1
#define PANEL_SWAP_LVDS_CH              0
#define BD_LVDS_CONNECT_TYPE            1

//------ HDMI RELATED ---------------------------------------------------------
#define HDCP_HPD_INVERSE                ENABLE

#define ENABLE_FACTORY_POWER_ON_MODE    ENABLE

//DDR Related
#define MIU0_SIZE_128M  0x8000000UL  //refine later
#define MIU0_BANK       0x8
#define S7J_MIU_UNIT    0x8

#define  MEMCLK_1066MHZ      1
#define  MEMCLK_1300MHZ      2
#define  MEMCLK_800MHZ       3
#define  MEMCLK_667MHZ       4

#define  MEMPLL_SEL         MEMCLK_800MHZ

#endif // _MSBOARD_H_

