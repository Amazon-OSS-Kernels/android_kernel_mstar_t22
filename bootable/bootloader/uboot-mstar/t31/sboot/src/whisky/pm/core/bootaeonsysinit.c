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
/*------------------------------------------------------------------------------*/
/*  INCLUDE                                                                     */
/*------------------------------------------------------------------------------*/
#include "datatype.h"
#include "hwreg.h"
#include "Board.h"
#include "drvGlobal.h"
#include "risc32_spr.h"
#include "drvPadConf.h"
#include "drvISR.h"
#include "MApp_Wakeup.h"
#include "uartdebug.h"

//*******************
//
//  utopia include files
//
//*******************
#include "MsCommon.h"
#include "drvMMIO.h"
#include "drvBDMA.h"
#include "drvWDT.h"
#include "drvGPIO.h"
#include "drvUART.h"
#include "drvRTC.h"

//*******************
//
//  other include files
//
//*******************



//***********************************************************
//              Macro Definitions
//***********************************************************
#define QMEM     ((unsigned  short volatile *) 0xB0000000ul)
#define _SPI_BOOTLOADER_RESET_VECTOR    0x00000


//***********************************************************
//              External Veriables
//***********************************************************


//***********************************************************
//              Local Veriables
//***********************************************************


//***********************************************************
//              Global Veriables
//***********************************************************


//***********************************************************
//              External Functions
//***********************************************************
extern MS_BOOL MDrv_XC_SetIOMapBase_i(void);
extern void InitialSystemPowerDown(void);
extern void MDrv_Sync_ConfigWakeup(void);
extern void MDrv_RTC_ConfigWakeup(void);
extern void MDrv_UART_ConfigWakeup(void);
extern int main(void);
#if BLOCK_POWER_FUNC
extern void MDrv_Block_PowerUp(void);
#endif


//***********************************************************
//              Global Functions Declaration
//***********************************************************
void SystemBootFromSPI(U32 u32BootAddr);
void SystemWholeChipReset(void);
void MDrv_Init_Varilabe(void);
void MDrv_Power_TurnOffStandby(void);
void MDrv_Power_SaveCustumization(void);
void start(void);

U32 g_u32UartHandle2, g_u32UartHandle3;
//***********************************************************
//              Local Functions Definition
//***********************************************************
static void MDrv_InitConsole( void )
{
  #if (USE_UART_BAUDRATE==115200)
    //set R2 clock to 24MHz
    MDrv_WriteRegBit(0x100b25, 0, BIT5);
    MDrv_WriteByte(0x100b25, 0x1C);
    MDrv_WriteRegBit(0x100b25, 1, BIT5);

    //set PIU UART0 clock to 24MHz
    #if ((USE_1ST_UART==UART_SRC_PIUUART0)||(USE_2ND_UART==UART_SRC_PIUUART0)||(USE_3RD_UART==UART_SRC_PIUUART0))
    {
        MDrv_WriteByte(0x100b27, 0x1C);
    }
    #endif

    #if ((USE_1ST_UART==UART_SRC_PIUUART1)||(USE_2ND_UART==UART_SRC_PIUUART1)||(USE_3RD_UART==UART_SRC_PIUUART1))
    {
        MDrv_WriteByte(0x100b28, 0x1C);
    }
    #endif
  #else
    //Run Xtal Clock 12MHz
     #if ((USE_1ST_UART==UART_SRC_PIUUART0)||(USE_2ND_UART==UART_SRC_PIUUART0)||(USE_3RD_UART==UART_SRC_PIUUART0))
     {
        MDrv_WriteRegBit(0x110c03, 0, BIT0); //reg_mpll_pd : power-up MPLL
        MDrv_WriteRegBit(0x100b27, 0, BIT0); //clk_uart0 : enable uart0 clock
     }
     #endif
     #if ((USE_1ST_UART==UART_SRC_PIUUART1)||(USE_2ND_UART==UART_SRC_PIUUART1)||(USE_3RD_UART==UART_SRC_PIUUART1))
     {
         MDrv_WriteRegBit(0x110c03, 0, BIT0); //reg_mpll_pd : power-up MPLL
         MDrv_WriteRegBit(0x100b28, 0, BIT0); //clk_uart1 : enable uart0 clock
     }
     #endif
  #endif

    //#####################
    //# (1) The 1st UART
    //#####################
    #if (USE_1ST_UART)
    {
        U8 u81stUartSrc = MDrv_UART_GetUartSrc(0);
        MDrv_UART_Init(u81stUartSrc, USE_UART_BAUDRATE);
        mdrv_uart_connect(USE_1ST_UART_PORT, u81stUartSrc);
    }
    #endif

    //#####################
    //# (2) The 2nd UART
    //#####################
    #if (USE_2ND_UART)
    {
        U8 u82ndUartSrc = MDrv_UART_GetUartSrc(1);
        if(USE_1ST_UART==UART_SRC_NONE)
        {
        MDrv_UART_Init(u82ndUartSrc, USE_UART_BAUDRATE);
        }
        else
        {
            g_u32UartHandle2 = mdrv_uart_open(u82ndUartSrc);
        }
        mdrv_uart_set_baudrate(g_u32UartHandle2, USE_UART_BAUDRATE);
        mdrv_uart_connect(USE_2ND_UART_PORT, u82ndUartSrc);
    }
    #endif

    //#####################
    //# (3) The 3rd UART
    //#####################
    #if (USE_3RD_UART)
    {
        U8 u83rdUartSrc = MDrv_UART_GetUartSrc(2);
        if((USE_1ST_UART==UART_SRC_NONE)&&(USE_2ND_UART==UART_SRC_NONE))
        {
        MDrv_UART_Init(u83rdUartSrc, USE_UART_BAUDRATE);
        }
        else
        {
            g_u32UartHandle3 = mdrv_uart_open(u83rdUartSrc);
        }
        mdrv_uart_set_baudrate(g_u32UartHandle3, USE_UART_BAUDRATE);
        mdrv_uart_connect(USE_3RD_UART_PORT, u83rdUartSrc);
    }
    #endif

    printf("\r\n[PM_WHISKY] Hello\r\n");
    printf("[PM_WHISKY] Console Initial OK\r\n");

}

static void MDrv_SysDelayUs(U32 u32InputTime)
{
    U32 u32DelayTime;
    U16 u16TMRStatus;

    u32DelayTime = 12 * u32InputTime;
    MDrv_Write2Byte(0x003024, u32DelayTime & 0x0000FFFF);
    MDrv_Write2Byte(0x003026, u32DelayTime >>16);
    MDrv_Write2Byte(0x003020, (MDrv_Read2Byte(0x003020) | BIT1));

    do{
        u16TMRStatus = MDrv_Read2Byte(0x003022);
        u16TMRStatus &= BIT0;
    }while(u16TMRStatus == 0);
}


//***********************************************************
//              Global Functions Definition
//***********************************************************
//
//  Boot from BootLoader
//  SPI boot from offset u32BootAddr
//
void SystemBootFromSPI(U32 u32BootAddr)
{

    //(1) Disable MIU0 first
    MDrv_Write2Byte(0x101200, MDrv_Read2Byte(0x101200)&(~BIT0));
    //(2) Set up reset vector base
    MDrv_Write2Byte(0x1002B4, (U16)(u32BootAddr >> 16));
    //(3) Set wtd_mcu_rst passwd
    MDrv_Write2Byte(0x002E54, 0x829f);
    //(4) Reset R2
    MDrv_Write2Byte(0x002E52, MDrv_Read2Byte(0x002E52)&(~BIT8));
    //wdt reset to dram: bellow 2 lines must in the same cache line 256bytes
    //(5) Set SPI Boot
    MDrv_Write2Byte(0x100280, 0x001B);  // set spi_boot=1, riu_sw_rstz=1, miu_sw_rstz=0, r2_sw_rstz=1, r2_enable=1
    //(6) Release R2
    MDrv_Write2Byte(0x002E52, MDrv_Read2Byte(0x002E52)|(BIT8));  // fire wtd_mcu_rst
}

//
//  Boot from BootLoader
//  System whole chip reset
//
void SystemWholeChipReset(void)
{
    printf("\r\n ======= Whole Chip Reset ======= \r\n");
    #if 0
    //This GPIO controlling DDR2 1.8volt power must be pulled HIGH before whole chip reset
    //because PAD_SARx (analog pads) cannot be auto-pull high by whole chip reset
    #endif

    #if BLOCK_POWER_FUNC
    MDrv_Block_PowerUp();
    #endif
    // add according to alex.tung
    MDrv_WriteRegBit(0x3DA3,1,BIT7);

    MDrv_Write2Byte(REG_PM_GCR_SWCK, MDrv_Read2Byte(REG_PM_GCR_SWCK) & (~0x0100));
    MDrv_Write4Byte(0x0EA0, 0x51685168);
    MDrv_WriteRegBit(0x2E52, 1, BIT7);
    MDrv_WriteRegBit(0x2E52, 1, BIT6);
    MDrv_WriteByte(0x2E5C,0xFF);
    MDrv_WriteByte(0x2E5C,0x79);

    while(1);
}

void MDrv_Init_Varilabe(void)
{
    gSyncDetect = FALSE;
    u8HDMIWakeUpDelayCount = 25;
}

void MDrv_Power_TurnOffStandby(void)
{
#if((MS_BOARD_TYPE_SEL == BD_MST233A_D01A_S)||(MS_BOARD_TYPE_SEL == BD_MST090B_10ANU_15213))
        mdrv_gpio_set_low(PAD_TCON1);
#elif((MS_BOARD_TYPE_SEL == BD_MST233B_D01A_S)||(MS_BOARD_TYPE_SEL == BD_MST089B_10ANB_15213))
        mdrv_gpio_set_low(PAD_GPIO32);
#elif((MS_BOARD_TYPE_SEL == BD_MST223B_D01A_S))
        mdrv_gpio_set_low(PAD_GPIO10);
#elif((MS_BOARD_TYPE_SEL == BD_MST269G_D01A_S)|| (MS_BOARD_TYPE_SEL == BD_MST158B_10ALWAT_18354))
        mdrv_gpio_set_low(PAD_GPIO7);//PIN_57 
#else
  #error "Please select correct boards"
#endif

}

void MDrv_Power_SaveCustumization(void)
{
    MDrv_SysDelayUs(1000000); //avoid backlight flash

    ////////////////////////////////////////////
    // Depend on schematics to save power
    ////////////////////////////////////////////
#if((MS_BOARD_TYPE_SEL == BD_MST233A_D01A_S)||(MS_BOARD_TYPE_SEL == BD_MST090B_10ANU_15213))
    mdrv_gpio_set_low(PAD_GPIO2); //AUDIO_EN
    mdrv_gpio_set_low(PAD_TCON2); //PreAMP_EN
    mdrv_gpio_set_low(PAD_TCON3); //PANEL_ON_OFF
#elif((MS_BOARD_TYPE_SEL == BD_MST233B_D01A_S)||(MS_BOARD_TYPE_SEL == BD_MST089B_10ANB_15213))
    mdrv_gpio_set_low(PAD_GPIO3); //AUDIO_EN
    mdrv_gpio_set_low(PAD_GPIO4); //PreAMP_EN
    mdrv_gpio_set_low(PAD_GPIO33); //PANEL_ON_OFF
    mdrv_gpio_set_low(PAD_GPIO34); //HP_DET
#elif((MS_BOARD_TYPE_SEL == BD_MST223B_D01A_S))
    mdrv_gpio_set_low(PAD_GPIO24); //AUDIO_EN
    mdrv_gpio_set_low(PAD_GPIO21); //PreAMP_EN
    mdrv_gpio_set_low(PAD_GPIO17); //PANEL_ON_OFF
#elif((MS_BOARD_TYPE_SEL == BD_MST269G_D01A_S))
    mdrv_gpio_set_high(PAD_GPIO6); //AUDIO_EN
    mdrv_gpio_set_high(PAD_TCON2); //PreAMP_EN
    mdrv_gpio_set_high(PAD_TCON1); //PANEL_ON_OFF
#elif((MS_BOARD_TYPE_SEL == BD_MST158B_10ALWAT_18354))
    mdrv_gpio_set_low(PAD_GPIO6); //AUDIO_EN
    mdrv_gpio_set_high(PAD_TCON2); //PreAMP_EN
    mdrv_gpio_set_low(PAD_TCON1); //PANEL_ON_OFF
#else
    #error "Please select correct boards"
#endif

}

void start(void)
{
    MDrv_WDT_Init(E_WDT_DBGLV_NONE);
    MDrv_WDT_Stop(E_WDT_DBGLV_NONE);

    //(1) power down initialization
    InitialSystemPowerDown();
    //(2)Initialize MsOS, Register Interrupt
    MsOS_Init();
    MDrv_ISR_Init();
    MDrv_Interrupt_Init();
    MDrv_XC_SetIOMapBase_i();
    mdrv_gpio_init();
    //(3)Register & Initialize Timer
    MDrv_Timer_Init();
    MDrv_Timer_ISR_Register();
    MDrv_Init_Varilabe();
    //(4)setup uart
    MDrv_InitConsole();

    //Init Wakeup Device
    MDrv_CheckWakeupDeviceFromAP();
    //Turn off standby power
    MDrv_Power_TurnOffStandby();
    //Save standby power by customization
    MDrv_Power_SaveCustumization();

    //jump to main() should never return
    main();

    while(1);

}

