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
#include "drvGlobal.h"
#include "drvAeonUART.h"
#include "risc32_spr.h"
#include "drvPadConf.h"
#include "drvISR.h"
#include "MApp_Wakeup.h"


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
#include "msIIC.h"



//***********************************************************
//              Macro Definitions
//***********************************************************
#define QMEM     ((unsigned  short volatile *) 0xB0000000ul)
#define UART0_BAUDRATE_PM               38400
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
void start(void);


//***********************************************************
//              Local Functions Definition
//***********************************************************
static void MDrv_InitConsole( void )
{
    uart_init(12000000, UART0_BAUDRATE_PM);
    mtspr( SPR_TTMR, 0x40000000 | 12000 );
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
#if((MS_BOARD_TYPE_SEL == BD_MST165A_D01A_S) || (MS_BOARD_TYPE_SEL == BD_MST165A_D01B_S))
    mdrv_gpio_set_low(PAD_GPIO15);
#elif(MS_BOARD_TYPE_SEL == BD_MST165B_D01A_S)
    mdrv_gpio_set_low(PAD_GPIO4);
#elif((MS_BOARD_TYPE_SEL == BD_MST165C_D01A_S)||(MS_BOARD_TYPE_SEL == BD_MST165C_D01B_S))
    mdrv_gpio_set_low(PAD_GPIO10);
#elif(MS_BOARD_TYPE_SEL == BD_MST202A_D01A_S)
    mdrv_gpio_set_low(PAD_TCON0);
#elif(MS_BOARD_TYPE_SEL == BD_MST043B_10ALB_12351)
    mdrv_gpio_set_low(PAD_GPIO10);
#elif(MS_BOARD_TYPE_SEL == BD_MST043B_10BLB_12413)
    mdrv_gpio_set_low(PAD_GPIO10);
#elif((MS_BOARD_TYPE_SEL == BD_MST042B_10BLU_12423)||(MS_BOARD_TYPE_SEL == BD_MST042B_10BLU_12423_32))
    mdrv_gpio_set_low(PAD_GPIO4);
#elif(MS_BOARD_TYPE_SEL == BD_MST044B_10BLA_12423)
    mdrv_gpio_set_low(PAD_GPIO15);
#elif(MS_BOARD_TYPE_SEL == BD_MST044B_10BLA_12423_32)
    mdrv_gpio_set_low(PAD_GPIO15);
#elif(MS_BOARD_TYPE_SEL == BD_MST165C2_D01B_S)
    mdrv_gpio_set_low(PAD_GPIO10);
#else
  #error "Please select correct boards"
#endif
}

void start(void)
{
    MDrv_WDT_Init(E_WDT_DBGLV_NONE);
    MDrv_WDT_Stop(E_WDT_DBGLV_NONE);

    InitialSystemPowerDown();
    MDrv_InitConsole();

    printf("\r\n Hello Eden standby mode \r\n");
    printf("\r\n Console Initial OK (PM) \r\n");

    MDrv_XC_SetIOMapBase_i();
    mdrv_gpio_init();
    //MDrv_IIC_Init();

    //Initialize MsOS, Register Interrupt
    MsOS_CPU_AttachException( E_EXCEPTION_TRAP, ProcessSysTrap, E_EXCEPTION_TRAP );
    MsOS_Init();
    MDrv_ISR_Init();

    //Register & Initialize Timer
    MDrv_Timer_ISR_Register();
    MDrv_Timer_Init();
    MDrv_Interrupt_Init();

    MDrv_Init_Varilabe();

    // Init Wakeup Device
    MDrv_CheckWakeupDeviceFromAP();

    // Turn off standby power
    MDrv_Power_TurnOffStandby();

    // jump to main() should never return
    main();

    while(1);

}

