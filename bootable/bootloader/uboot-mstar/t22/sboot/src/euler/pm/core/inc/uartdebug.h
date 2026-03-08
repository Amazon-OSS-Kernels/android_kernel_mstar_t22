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

#ifndef __UART_H
#define __UART_H


#ifdef _UART_C_
#define INTERFACE
#define INITVALUE(v)    = (v)
#else
#define INTERFACE  extern
#define INITVALUE(v)
#endif

////////////////////////////////////////////////////////////////////////////////

#define ENABLE_MSTV_UART_DEBUG          1

#define ENABLE_UART2                    0

////////////////////////////////////////////////////////////////////////////////

typedef enum _UartCommadEnumType
{
    _UART_CMD_CODE_,
    _UART_CMD_INDEX1_,
    _UART_CMD_INDEX2_,
    _UART_CMD_INDEX3_,
    _UART_CMD_INDEX4_,
    _UART_CMD_INDEX5_,
    _UART_CMD_INDEX6_,
    _UART_CMD_INDEX7_,
    _UART_CMD_INDEX8_,
    _UART_CMD_INDEX9_,
    _UART_CMD_INDEX10_,
    _UART_CMD_INDEX11_,
    _UART_CMD_INDEX12_,
    _UART_CMD_INDEX13_,
    _UART_CMD_INDEX14_,
    _UART_CMD_INDEX15_,
    _UART_CMD_INDEX16_,
    _UART_CMD_INDEX17_,
    _UART_CMD_INDEX18_,
    _UART_CMD_INDEX19_,
    _UART_CMD_INDEX20_,

    _UART_CMD_LENGTH_
}UartCommadEnumType;

typedef struct _UartCommadType
{
    U8 Buffer[_UART_CMD_LENGTH_];   // command buffer
                                    // [Length+Command] - [Byte 2] - [Byte 3] - [Byte 4] - [Byte 5]
    U8 Index;                       // buffer index

}UartCommadType;



////////////////////////////////////////////////////////////////////////////////

INTERFACE UartCommadType g_UartCommand; // Uart command struct

INTERFACE bit g_bDebugProgStopFlag;
INTERFACE bit g_bUart0Detected; // decode command flag
INTERFACE bit g_bDisableUartDebug INITVALUE(0);
INTERFACE U16 g_Uart0CheckTick;


////////////////////////////////////////////////////////////////////////////////

void putcharb(MS_U8 byte);
void MDrv_UART_RecvHandler(int c);
void MDrv_UART_ConfigWakeup(void);
BOOL Cus_UART_ExecTestCommand(void);
BOOL Cus_UART_DecodeCommand(void);
void Cus_UART_DecodeExtCommand(void);
void MDrv_UART_DecodeExtCommand(void);
void MDrv_UART_ExecTestCommand(void);
void MDrv_UART_DecodeNormalCommand(void);
void UART_DecodeCommand(void);
U8 msAPI_UART_DecodeCommand(void);

////////////////////////////////////////////////////////////////////////////////

#if ENABLE_UART2
INTERFACE UartCommadType g_Uart2Command;
INTERFACE U16 g_Uart2CheckTick;
INTERFACE bit g_bUart2Detected;
INTERFACE U8  g_ucUart2BufIndex;

void MDrv_UART2_RecvHandler(int c);
#endif // #if ENABLE_UART2


////////////////////////////////////////////////////////////////////////////////

#define _EOS_                   '\0' // end of string

#define uartWrite_MST_Bank_n    0x01
#define uartRead_MST_Bank_n     0x02
#define uartXC_Read_MST_Bank    0x20
#define uartXC_Write_MST_Bank   0x21


#define uartRead_MCU_XDATA      0x04
#define uartWrite_MCU_XDATA     0x05

#define uartRead_Demod          0x06
#define uartWrite_Demod         0x07
#define uartRead_Tuner          0x0A
#define uartWrite_Tuner         0x0B

#define uartTest_Command        0x09

#define uartI2C_Device_Write    0x10
#define uartI2C_Device_Read     0x11

#define uartEEPROM_Write        0x12
#define uartEEPROM_Read         0x13


////////////////////////////////////////////////////////////////////////////////


#define UART_CMD_LENGTH         (g_UartCommand.Buffer[_UART_CMD_CODE_] >> 5)
#define UART_CMD                (g_UartCommand.Buffer[_UART_CMD_CODE_] & 0x1F)
#define UART_CMD_MS_REG         (g_UartCommand.Buffer[_UART_CMD_INDEX1_])
#define UART_CMD_MS_DAT         (g_UartCommand.Buffer[_UART_CMD_INDEX2_])

// for EXT Command
#define UART_EXT                (UART_CMD_LENGTH == 7)
#define UART_CMD_EXT_LENGTH     (g_UartCommand.Buffer[_UART_CMD_INDEX1_])
#define UART_EXT_DEV            (g_UartCommand.Buffer[_UART_CMD_INDEX2_])
#define UART_EXT_CMD            (g_UartCommand.Buffer[_UART_CMD_INDEX3_])
#define UART_EXT_CMD_MS_DAT1    (g_UartCommand.Buffer[_UART_CMD_INDEX4_])
#define UART_EXT_CMD_MS_DAT2    (g_UartCommand.Buffer[_UART_CMD_INDEX5_])
#define UART_EXT_CMD_MS_DAT3    (g_UartCommand.Buffer[_UART_CMD_INDEX6_])
#define UART_EXT_CMD_MS_DAT4    (g_UartCommand.Buffer[_UART_CMD_INDEX7_])
#define UART_EXT_CMD_MS_DAT5    (g_UartCommand.Buffer[_UART_CMD_INDEX8_])
#define UART_EXT_CMD_MS_DAT6    (g_UartCommand.Buffer[_UART_CMD_INDEX9_])
#define UART_EXT_CMD_MS_DAT7    (g_UartCommand.Buffer[_UART_CMD_INDEX10_])
#define UART_EXT_CMD_MS_DAT8    (g_UartCommand.Buffer[_UART_CMD_INDEX11_])
#define UART_EXT_CMD_MS_DAT9    (g_UartCommand.Buffer[_UART_CMD_INDEX12_])
#define UART_EXT_CMD_MS_DAT10   (g_UartCommand.Buffer[_UART_CMD_INDEX13_])
#define UART_EXT_CMD_MS_DAT11   (g_UartCommand.Buffer[_UART_CMD_INDEX14_])
#define UART_EXT_CMD_MS_DAT12   (g_UartCommand.Buffer[_UART_CMD_INDEX15_])
#define UART_EXT_CMD_MS_DAT13   (g_UartCommand.Buffer[_UART_CMD_INDEX16_])

#define UART_CMD_FLASH_LENGTH    (g_UartCommand.Buffer[_UART_CMD_CODE_] >> 3)
#define UART_CMD_FLASH_ADDR0    (g_UartCommand.Buffer[_UART_CMD_INDEX1_])
#define UART_CMD_FLASH_ADDR1    (g_UartCommand.Buffer[_UART_CMD_INDEX2_])
#define UART_CMD_FLASH_ADDR2    (g_UartCommand.Buffer[_UART_CMD_INDEX3_])
#define UART_CMD_FLASH_DATA     (g_UartCommand.Buffer[_UART_CMD_INDEX1_])
#define UART_FLASH_CMD          (g_UartCommand.Buffer[_UART_CMD_CODE_] & 0x07)

//for menu mode used.
#define  UART_MENU_CMD0        (g_UartCommand.Buffer[_UART_CMD_CODE_])
#define  UART_MENU_CMD1        (g_UartCommand.Buffer[_UART_CMD_INDEX1_])
#define  UART_MENU_CMD2        (g_UartCommand.Buffer[_UART_CMD_INDEX2_])
#define  UART_MENU_CMD3        (g_UartCommand.Buffer[_UART_CMD_INDEX3_])
#define  UART_MENU_CMD4        (g_UartCommand.Buffer[_UART_CMD_INDEX4_])
#define  UART_MENU_CMD5        (g_UartCommand.Buffer[_UART_CMD_INDEX5_])
#define  UART_MENU_CMD6        (g_UartCommand.Buffer[_UART_CMD_INDEX6_])
#define  UART_MENU_CMD7        (g_UartCommand.Buffer[_UART_CMD_INDEX7_])
#define  UART_MENU_CMD8        (g_UartCommand.Buffer[_UART_CMD_INDEX8_])
#define  UART_MENU_CMD9        (g_UartCommand.Buffer[_UART_CMD_INDEX9_])
#define  UART_MENU_CMD10        (g_UartCommand.Buffer[_UART_CMD_INDEX10_])
#define  UART_MENU_CMD11        (g_UartCommand.Buffer[_UART_CMD_INDEX11_])
#define  UART_MENU_CMD12        (g_UartCommand.Buffer[_UART_CMD_INDEX12_])
#define  UART_MENU_CMD13        (g_UartCommand.Buffer[_UART_CMD_INDEX13_])
#define  UART_MENU_CMD14        (g_UartCommand.Buffer[_UART_CMD_INDEX14_])
#define  UART_MENU_CMD15        (g_UartCommand.Buffer[_UART_CMD_INDEX15_])
#define  UART_MENU_CMD16        (g_UartCommand.Buffer[_UART_CMD_INDEX16_])

#define _EOS_ '\0' // end of string

#define uartWrite_MST_Bank_n    0x01
#define uartRead_MST_Bank_n     0x02
#define uartXC_Read_MST_Bank    0x20
#define uartXC_Write_MST_Bank   0x21


#define uartRead_MCU_XDATA      0x04
#define uartWrite_MCU_XDATA     0x05

#define uartRead_Demod          0x06
#define uartWrite_Demod         0x07
#define uartRead_Tuner          0x0A
#define uartWrite_Tuner         0x0B
#if 0//(FRONTEND_TUNER_TYPE == MSTAR_MSR1200_TUNER)
#define uartRead_PI2C           0x0C
#define uartWrite_PI2C          0x0D
#endif

#define uartTest_Command        0x09

#define uartI2C_Device_Write    0x10
#define uartI2C_Device_Read     0x11

#define uartEEPROM_Write        0x12
#define uartEEPROM_Read         0x13

//---------------------------------
#define uartExtDev_I2C_DEVICE               0x00
#define uartExtDev_Other_Tuner_Debug        0x02
#define uartExtDev_EEPROM_HDCP              0x03
#define uartExtDev_MstDlcTest               0x05
#define uartExtDev_ACE_REPORT               0x07
#define DEV_MST_DBC_TEST                    0x0A
#define uartExtDev_XC_rw                    0x0B

#define uartExtDev_SAMSUNG_S5H1409_DEMODE   0x10
#define uartExtDev_MstAutoBench 0x15

#define uartExtDev_Autotest_Key             0x36 //'6'
#define uartExtDev_Autotest_Sp              0x37 //'7'
#define uartExtDev_USB_Download             0x38 //'8'
#define uartExtDev_Power_Test               0x39 //'9'

#define URCMD_EEPROM_HDCP_GET_ADDR          0
#define URCMD_EEPROM_HDCP_WRITE             1
#define URCMD_EEPROM_HDCP_WRITE_END         2
#define URCMD_EEPROM_HDCP_GET_CHECKSUM      3

//---------------------------------

// kWrite_MST_Bank_n, kRead_MST_Bank_n
#define UART_CMD_MS_BANK_H     (g_UartCommand.Buffer[_UART_CMD_INDEX1_])
#define UART_CMD_MS_BANK_L     (g_UartCommand.Buffer[_UART_CMD_INDEX2_])
#define UART_CMD_MS_REGINDEX   (g_UartCommand.Buffer[_UART_CMD_INDEX3_])
#define UART_CMD_MS_REGDATA    (g_UartCommand.Buffer[_UART_CMD_INDEX4_])

// kRead_MCU_XDATA, kWrite_MCU_XDATA
#define UART_CMD_MCU_IDX_H      (g_UartCommand.Buffer[_UART_CMD_INDEX1_])
#define UART_CMD_MCU_IDX_H1     (g_UartCommand.Buffer[_UART_CMD_INDEX1_])
#define UART_CMD_MCU_IDX_H2     (g_UartCommand.Buffer[_UART_CMD_INDEX2_])
#define UART_CMD_MCU_IDX_L      (g_UartCommand.Buffer[_UART_CMD_INDEX3_])
#define UART_CMD_MCU_DATA       (g_UartCommand.Buffer[_UART_CMD_INDEX4_])

#define URCMD_MST_DLC_TEST_FORCE_PAUSE      0
#define URCMD_MST_DLC_TEST_FORCE_RUN        1
#define URCMD_MST_DLC_TEST_READ             2
#define URCMD_MST_DLC_TEST_WRITE_1          3
#define URCMD_MST_DLC_TEST_WRITE_2          4
#define URCMD_MST_DLC_TEST_REINIT           5
#define URCMD_MST_DLC_TEST_BOUNDARY_READ    6
#define URCMD_MST_DLC_TEST_BOUNDARY_WRITE   7
#define URCMD_MST_DLC_TEST_READ_CV1         8
#define URCMD_MST_DLC_TEST_READ_CV2A        9
#define URCMD_MST_DLC_TEST_READ_CV2B        10
#define URCMD_MST_DLC_TEST_WRITE_CV1        11
#define URCMD_MST_DLC_TEST_WRITE_CV2A       12
#define URCMD_MST_DLC_TEST_WRITE_CV2B       13
#define URCMD_MST_DLC_TEST_READ_INFO        14
#define URCMD_MST_DLC_TEST_WRITE_INFO       15
#define URCMD_MST_DLC_TEST_CGC_REINIT       16

//ENABLE_DBC
#define URCMD_MST_DBC_TEST_READ_INFO        1
#define URCMD_MST_DBC_TEST_WRITE_INFO       2
#define URCMD_MST_DBC_TEST_REINIT           3

//ENABLE_ACE_REPORT
#define URCMD_MST_ACE_REPORT_CONTRAST   0
#define URCMD_MST_ACE_REPORT_SATURATION 1
#define URCMD_MST_ACE_REPORT_HUE        2
#define URCMD_MST_ACE_REPORT_R_GAIN     3
#define URCMD_MST_ACE_REPORT_G_GAIN     4
#define URCMD_MST_ACE_REPORT_B_GAIN     5
#define URCMD_MST_ACE_REPORT_R_BRI      6
#define URCMD_MST_ACE_REPORT_G_BRI      7
#define URCMD_MST_ACE_REPORT_B_BRI      8
#define URCMD_MST_ACE_REPORT_TYPE       9
#define URCMD_MST_ACE_REPORT_COLOR_CORRECTION_XY_R      10
#define URCMD_MST_ACE_REPORT_COLOR_CORRECTION_XY_G      11
#define URCMD_MST_ACE_REPORT_COLOR_CORRECTION_XY_B      12
#define URCMD_MST_ACE_REPORT_COLOR_CORRECTION_OFFSET_R  13
#define URCMD_MST_ACE_REPORT_COLOR_CORRECTION_OFFSET_G  14
#define URCMD_MST_ACE_REPORT_COLOR_CORRECTION_OFFSET_B  15
#define URCMD_MST_ACE_REPORT_IF_SUPPORT_RGB_GAIN 18

#define URCMD_MST_MV_PW_TEST   0






////////////////////////////////////////////////////////////////////////////////
#undef INTERFACE
#endif /* __UART_H  */

