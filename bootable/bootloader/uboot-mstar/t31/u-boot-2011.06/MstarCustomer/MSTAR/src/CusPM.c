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

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <command.h>
#include <common.h>
#include <ShareType.h>
#include <msAPI_Power.h>
#include <drvWDT.h>
#include <MsSystem.h>
#include <MsEnvironment.h>
#include <CusConfig.h>
#include <MsVfs.h>
#include <MsDebug.h>
#include <drvBDMA.h>
#include <MsApiPM.h>
#include <MsSysUtility.h>
#include <apiCEC.h>
#include <drvPM.h>
#include <MsUboot.h>
#include <MsMmap.h>
#include <MsDrvCache.h>
#include <idme.h>

#if defined (CONFIG_ONESN_ENABLE)
#include <MsOneSN.h>
#endif
#if (ENABLE_MSTAR_NIKON==1) || (ENABLE_MSTAR_MILAN==1) || (ENABLE_MSTAR_MARLON==1)
#ifndef  CUS_IR_HEAD_FILE//Please define it in board file for customization
#include "IR_MSTAR_DTV.h"
#else
#include CUS_IR_HEAD_FILE
#endif
#endif

#if defined(AMZN_FTVE_PM_SIGNING_ENABLED)
#include "amzn_secure_boot.h"
#define PM_START_SIZE 0x10000
#define PM_SIG_START_SIZE 0x10100
#define PM_SIG_LEN (256)
#endif


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Extern Functions
//-------------------------------------------------------------------------------------------------
extern U8 IsPowerButtonPressed(void);
//-------------------------------------------------------------------------------------------------
//  Private Functions
//-------------------------------------------------------------------------------------------------

#if (ENABLE_MSTAR_NIKON==1) || (ENABLE_MSTAR_MILAN==1) || (ENABLE_MSTAR_MARLON==1)
static int SetPm2L1(void);
#else
static int SetPm2Sram(void);
#endif
BOOLEAN get_poweroff_flag(void);
#if (CONFIG_WDT_RESET_BY_ESD)
static void set_poweroff_flag(BOOLEAN bEnable);
#endif
static int If_Boot_To_PM(void);
static BOOLEAN check_pm_standby(void);
static int PM51_PowerDown(void);
static unsigned int get_pm51_program_counter(void);

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

static int PM51_PowerDown(void)
{
    char *s = NULL;
    int iStrCrc = 0;

    UBOOT_TRACE("IN\n");
    if(IsHouseKeepingBootingMode()==TRUE)
    {
        // bring up 51
        if( IsBootingFromMaskRom() == TRUE )
        {
            #if (ENABLE_MSTAR_EDISON==1)
                UBOOT_DEBUG("=== PATCH FOR EDISON GTV ROMBOOT BUT PM IN SPI ===\n");
                msAPI_SetSPIOffsetForMCU();
            #elif (ENABLE_MSTAR_NIKON==1)
                UBOOT_DEBUG("=== NiKon ROMBOOT BUT PM IN L1 ===\n");
                return SetPm2L1();
            #elif (ENABLE_MSTAR_MILAN==1)
                UBOOT_DEBUG("=== Milan ROMBOOT BUT PM IN L1 ===\n");
                return SetPm2L1();
            #elif (ENABLE_MSTAR_MARLON==1)
                UBOOT_DEBUG("=== Marlon ROMBOOT BUT PM IN L1 ===\n");
                return SetPm2L1();
            #else
            UBOOT_DEBUG("== SetPm2Sram ==\n");
            if(SetPm2Sram()!=0)
            {
                UBOOT_ERROR("SetPm2Sram Fail!!\n");
                return -1;
            }
            #endif
        }
        else
        {
            msAPI_SetSPIOffsetForMCU();
        }
    }
    #if (CONFIG_MSTAR_RT_PM_IN_SPI)
    MDrv_PM_STR_CheckFactoryPowerOnModePassword();
    #endif

    if(strcmp(getenv("factory_poweron_mode"), "power_test") == 0)
    {
        s = getenv("str_crc");
        if(NULL != s)
        {
            iStrCrc = (int) simple_strtoul (s, NULL, 10);
            msAPI_Power_SetStrConfig(iStrCrc);
        }
        else
        {
            msAPI_Power_SetStrConfig(1);
        }
    }
    else
    {
        msAPI_Power_SetStrConfig(1);
    }

    //msAPI_Power_SetStrConfig( (strcmp(getenv("str_crc"), "0")==0) ? FALSE : TRUE ) ;
#if defined (CONFIG_ONESN_ENABLE)
    MsDrv_OneSN_LED_RED_ON();
#endif
    #if(CONFIG_VOICE_CM4==1)
        run_command("runcm4",0);
    #endif
    #if (CONFIG_DIGITAL_RT9113B)
        run_command("audio_preinit",0);
    #endif
    msAPI_Power_PowerDown_EXEC();
    UBOOT_TRACE("OK\n");
    return 0;
}

#if (ENABLE_MSTAR_NIKON==1) || (ENABLE_MSTAR_MILAN==1) || (ENABLE_MSTAR_MARLON==1)
extern BOOLEAN msAPI_KeyPad_Initialize(void);

static int SetPm2L1(void)
{
    MS_PHYADDR Addr=0, Size=0;
    U32 u32AddrVA=0;
    U32 u32AddrNVA=0;
    char PMPath[CMD_BUF]="\0";
    UBOOT_TRACE("IN\n");
    if(MDrv_WDT_IsReset())
    {
        UBOOT_DEBUG("Clear WDT Flag\n");
        MDrv_WDT_ClearRstFlag();
    }
    Write2Byte(0x0e68, 0xBEBA);
#if (CONFIG_KEYPAD)
    msAPI_KeyPad_Initialize();
#endif
    static PM_WakeCfg PmWakeCfg =
        {
            .bPmWakeEnableIR = TRUE,
#if (CONFIG_KEYPAD)             
            .bPmWakeEnableSAR = TRUE,
#else
            .bPmWakeEnableSAR = FALSE,
#endif
            .bPmWakeEnableGPIO0 = FALSE,
            .bPmWakeEnableGPIO1 = FALSE,
            .bPmWakeEnableUART1 = FALSE,
            .bPmWakeEnableSYNC = FALSE,
            .bPmWakeEnableESYNC = FALSE,

            .bPmWakeEnableRTC0 = TRUE,
            .bPmWakeEnableRTC1 = TRUE,
            .bPmWakeEnableDVI0 = FALSE,
            .bPmWakeEnableDVI2 = FALSE,
            .bPmWakeEnableCEC = TRUE,
            .bPmWakeEnableAVLINK = FALSE,

            .u8PmWakeIR =
            {   //IR wake-up key define
                IRKEY_POWER, 0x03, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF
            },

            .u8PmWakeIR2 =
            {   //IR wake-up key define
                IRKEY_POWER, 0x03, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF,
                0xFF, 0xFF, 0xFF, 0xFF
            }
        };
    char* s = getenv(POWER_KEY_NAME);
    if (s)
    {
        PmWakeCfg.u8PmWakeIR[0] = simple_strtoul(s, NULL, 16);
    }

    flush_cache((MS_U32)&PmWakeCfg, sizeof(PM_WakeCfg));

    if(MDrv_PM_Init((PM_WakeCfg*)VA2PA((MS_U32) &PmWakeCfg))==0)
    {
        UBOOT_ERROR("MDrv_PM_Init fail !!!\n");
    }

    if(get_map_addr_and_size_from_env(E_PM51_MEM, NO_DEFAULT_MMAP_VALUE, &Addr, NO_DEFAULT_MMAP_VALUE, &Size)!=0)
    {
	    UBOOT_ERROR("get %s mmap fail\n",E_MMAP_ID_PM51_USAGE_MEM_ADR);
        return -1;
    }

    if(vfs_mount(CONFIG)!= 0)
    {
        UBOOT_ERROR("vfs_mount fail\n");
        return -1;
    }
    snprintf(PMPath,sizeof(PMPath),"%s/PM.bin",CONFIG_PATH);
    u32AddrVA=PA2VA(Addr);
    u32AddrNVA=PA2NVA(Addr);

    if(vfs_read((void*)u32AddrVA,PMPath,0,0x10000) != 0)
    {
        UBOOT_ERROR("vfs_read fail !!! \n");
        return -1;
    }
    udelay(50);


    UBOOT_DEBUG("Load PM To DRAM PA[0x%x]VA[0x%x]NVA[0x%x] \n",Addr,u32AddrVA,u32AddrNVA);

    MsOS_DisableAllInterrupts();
    flush_cache(u32AddrVA,0x10000);
    Chip_Flush_Memory();

    enable_cache(0);

    ric_fill_icache((void *)(u32AddrNVA), 0x8000);
    ric_fill_dcache((void *)(u32AddrNVA+0x8000), 0x8000);
    //enable_cache(1); // Move to PM

    printf("[%s][%s] Start PM at :0x%x \n\n",__FILE__,__FUNCTION__, (u32AddrVA|0x400));
    console_init();
    udelay(50);

    asm volatile (
            "move $9, %[i0]\n\t" \
            "j $9\n\t" \
            "nop\n\t" \
            : \
            : [i0] "r"(u32AddrVA|0x400) \
            : "memory" \
    );
    while(1);

    UBOOT_TRACE("OK\n");
    return 0;
}

#else
static int SetPm2Sram(void)
{
    MS_PHYADDR Addr=0, Size=0;

    char PMPath[CMD_BUF]="\0";
    UBOOT_TRACE("IN\n");

    if(get_map_addr_and_size_from_env(E_PM51_MEM, NO_DEFAULT_MMAP_VALUE, &Addr, NO_DEFAULT_MMAP_VALUE,&Size)!=0)
    {
	    UBOOT_ERROR("get %s mmap fail\n",E_MMAP_ID_PM51_USAGE_MEM_ADR);
        return -1;
    }

    if(vfs_mount(CONFIG)!= 0)
    {
        UBOOT_ERROR("vfs_mount fail\n");
        return -1;
    }

#if defined(AMZN_FTVE_PM_SIGNING_ENABLED)
        MS_PHYADDR PM_Addr=0;
        PM_Addr = malloc(PM_SIG_START_SIZE * sizeof(unsigned char));
        snprintf(PMPath,sizeof(PMPath),"%s/PM.bin",CONFIG_PATH);
        if(vfs_read((void*)PM_Addr,PMPath,0,PM_SIG_START_SIZE) != 0)
        {
            UBOOT_ERROR("vfs_read fail\n");
        }

        UBOOT_DEBUG("Verify PM\n");
        if (amzn_verify_pm((void*) PM_Addr)) {
            free(PM_Addr);
            while(1) {
                UBOOT_ERROR("Failed to verify PM\n");
            }
        }
        free(PM_Addr);
        UBOOT_DEBUG("Verify PM PASS\n");
#endif

    snprintf(PMPath,sizeof(PMPath),"%s/PM.bin",CONFIG_PATH);
    if(vfs_read((void*)PA2NVA(Addr),PMPath,0,CONFIG_PM_SIZE) != 0)
    {
        UBOOT_ERROR("vfs_read fail\n");
        return -1;
    }

    if(MDrv_BDMA_CopyHnd(Addr, 0x0, CONFIG_PM_SIZE, E_BDMA_SDRAM2SRAM1K_HK51, 0) != TRUE )
    {
        UBOOT_ERROR("MDrv_BDMA_CopyHnd fail\n");
        return -1;
    }
    MDrv_PM_SetSRAMOffsetForMCU();
    UBOOT_TRACE("OK\n");
    return 0;
}
#endif

#if defined(AMZN_FTVE_PM_SIGNING_ENABLED)
int amzn_verify_pm(void *pm_buf)
{
    int ret = 0; // success
    unsigned int pm_size = PM_START_SIZE;

    unsigned int key_len = 0;
    const uint8_t *key = NULL;

#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_ANNA)
    key = amzn_get_sign_key(&key_len);
#else
    key = amzn_get_unlock_key(&key_len);
#endif

    if (!key || !key_len) {
        UBOOT_ERROR("%s: Failed to get key\n", __FUNCTION__);
        ret = 1;
        goto done;
    }
    const unsigned char* sig_buf = (const unsigned char*)pm_buf+pm_size;

    ret = amzn_verify_code_internal((const unsigned char*)pm_buf, pm_size,
        sig_buf, PM_SIG_LEN, key, key_len);
    UBOOT_DEBUG("PM amzn_verify_code_internal ret = %d\n", ret);

done:
    return ret;
}
#endif



#if (CONFIG_WDT_RESET_BY_ESD)
static void set_poweroff_flag(BOOLEAN bEnable)///ac power off also use this flag -> dc_poweroff
{
    UBOOT_TRACE("IN\n");

#if (ENABLE_ENV_IN_NAND == 1)
    ////Do nothing because vfs_write is not implement now
#else
    BOOLEAN bOrgValue = FALSE;
    if (strcmp(getenv("dc_poweroff"), "1") == 0)
    {
        bOrgValue = TRUE;
    }

    if (bEnable != bOrgValue)
    {
        if(bEnable)
        {
            setenv("dc_poweroff", "1");
        }
        else
        {
            setenv("dc_poweroff", "0");
        }
        saveenv();
    }
#endif
    UBOOT_TRACE("OK\n");
}
#endif


int isCodBootByPowerLoss(void)
{
	return (Read2Byte(BOOT_REASON_PM_ADDR_OFFSET) == PM_SPARE_COLD_BOOT_POWER_SUPPLY);
}
int isScreenStateOff(void)
{
	return (Read2Byte(BOOT_REASON_PM_ADDR_OFFSET) & PM_SPARE_SCREEN_STATE);
}

static BOOLEAN check_pm_standby(void)
{
    BOOLEAN ret = FALSE;
	unsigned int boot_reason;
    UBOOT_TRACE("IN\n");
	boot_reason = Read2Byte(BOOT_REASON_PM_ADDR_OFFSET);
	printf("check_pm_standby boot_reason = 0x%x \n", boot_reason);
	 if((boot_reason == PM_SPARE_BOOT_LONG_PWR_KEY_PRESS) &&
                (atoi(getenv("bootmode")) == IDME_BOOTMODE_DIAG))
        {
		/* This is for STR power consumption measurement on production line 
		   Tester can long press POWER key to let TV enter bootloader STR mode in Diag
		*/
		return TRUE;;
	}

#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_DUCKIE) || defined(CONFIG_MTK_BD_MT164B_10AT_M7632_HAILEY) || defined(CONFIG_MTK_BD_MT168B_10AT_19133_MT5870_M7332_ABC)
    if(atoi(getenv("bootmode")) == IDME_BOOTMODE_DIAG)
    {
        int result;
        result = run_command("usb start 0", 0);
        if (!result) {
            /* This is for STR power comsumption measurement on SMT production line.
               TV enter bootloader STR in Diag mode if a specific file exists on USB disk.
            */
            result = run_command("fatfilesize usb 0:1 fac_boot_aging_exit.cvt", 0);
            if (!result) {
                run_command("usb stop", 0);
                return TRUE;
            }

            /*
	       factroy requests to have a mode for AC power on when a specific file is detected in USB disk.
            */
	    result = run_command("fatfilesize usb 0:1 fac_boot_smart.cvt", 0);
            if (!result) {
                run_command("usb stop", 0);
                return FALSE;
            }
        }
    }
#endif
    if(boot_reason & PM_SPARE_SPECIAL_MODE_SILENT_OTA ){
         printf("Silent OTA was interrupted, it will resume on secondary boot \n");
         return TRUE;
    }

    // if the device is in store demo mode, bypass standby
    if( (getenv("usr_flags") != NULL) && ((simple_strtoul(getenv("usr_flags"), NULL, 16))&USR_FLAGS_STOREDEMO_MODE) )
         return FALSE;

    if( (getenv("dev_flags") != NULL) && ((simple_strtoul(getenv("dev_flags"), NULL, 16))&DEV_FLAGS_BYPASS_SECONDARY_BOOT) )
         return FALSE;

    if (IsPowerButtonPressed())
         return FALSE;

    if((getenv("force_onetime_standby")!=NULL) && (strcmp(getenv("force_onetime_standby"), "1") == 0))
    {
         setenv("force_onetime_standby",NULL);
         setenv("MstarUpgrade_complete","1");
         saveenv();
         ret = TRUE;
    }
    else if(strcmp(getenv("factory_poweron_mode"), "power_test") == 0)
    {
         ret = TRUE;
    }
    //Secondary mode, AC on will always enter standby mode, except system is runing the SW upgrade.
    else if((strcmp(getenv("factory_poweron_mode"), "secondary") == 0) &&
            (strcmp(getenv("upgrade_mode"), "null")==0) &&
            (strcmp(getenv("burnmode_poweron"), "false")==0))
    {
        MS_CEC_INIT_INFO stMsCECInfo;
        //value assigned
        stMsCECInfo.u32XTAL_CLK_Hz = 12000000UL;
        MApi_CEC_InitChip(&stMsCECInfo);
        MApi_CEC_ConfigWakeUp();
        ret = TRUE;
	if ((atoi(getenv("bootmode")) == IDME_BOOTMODE_NORMAL) &&
		((boot_reason == PM_SPARE_COLD_BOOT_POWER_SUPPLY) ||
		(boot_reason & PM_SPARE_SHUTDOWN_SW))) {
		printf("Enter PM standy mode \n");
		ret = TRUE;
	} else if ((atoi(getenv("bootmode")) == IDME_BOOTMODE_DIAG) &&
		((boot_reason == PM_SPARE_COLD_BOOT_POWER_SUPPLY) ||
		(boot_reason & PM_SPARE_SHUTDOWN_SW) ||
		(boot_reason & PM_SPARE_WARM_BOOT_SW))) {
		printf("Enter PM standy mode from diag mode\n");
		ret = TRUE;
	} else {
		if(isScreenStateOff() && (boot_reason&(PM_SPARE_WARM_BOOT_HW_WDOG|PM_SPARE_WARM_BOOT_KERNEL_PANIC | PM_SPARE_WARM_BOOT_KERNEL_WDOG )))
		{
			printf("Screen is off when watch dog or kernel panic happens, Enter PM standy mode \n");
			return TRUE;
		}

		printf("check_pm_standby: boot_reason = %x\n", boot_reason);
		ret = FALSE;
	}
    }
    else if(strcmp(getenv("factory_poweron_mode"), "memory") == 0) //Memory, DC off -> AC off -> AC on -> Standby
    {
        #if (CONFIG_MSTAR_RT_PM_IN_SPI)
        if(MDrv_PM_STR_CheckFactoryPowerOnMode_Second(1)!= 1)
        {
            return FALSE;
        }
        #endif
        ret = get_poweroff_flag();
    }

    UBOOT_TRACE("OK\n");
    return ret;
}



static unsigned int get_pm51_program_counter(void)
{
    unsigned int u32ProgramCounter = 0;
    UBOOT_TRACE("IN\n");
    u32ProgramCounter = *(volatile U32*)(MS_RIU_MAP+(0x10fe<<1));
    u32ProgramCounter = (u32ProgramCounter&0xff)<<16;
    u32ProgramCounter |= *(volatile U32*)(MS_RIU_MAP+(0x10fc<<1));
    UBOOT_TRACE("OK\n");
    return u32ProgramCounter;
}

static int If_Boot_To_PM(void)
{
    char ledCmd[CMD_BUF] = "\0";
    EN_POWER_ON_MODE ePowerMode=EN_POWER_DC_BOOT;
    UBOOT_TRACE("IN\n");

#if (WDT_STANDBY_MODE==1)
    MDrv_WDT_Init(E_WDT_DBGLV_ALL);

    if(MDrv_WDT_IsReset() && get_poweroff_flag())
    {

        MDrv_WDT_ClearRstFlag();
        return 0;
    }
#endif
    ePowerMode=msAPI_Power_QueryPowerOnMode();

    if ( EN_POWER_AC_BOOT == ePowerMode)
    {

        if( (getenv("factory_poweron_mode") == NULL) ||
            (strcmp(getenv("factory_poweron_mode"), "direct") == 0))  //change mode to secondary mode
        {
			printf("Set device to seconardy boot mode \n");
            setenv("factory_poweron_mode", "secondary");
        }
		if(getenv("burnmode_poweron") == NULL)
        	{
            		setenv("burnmode_poweron", "false");
        	}
        	if(getenv("dc_poweroff") == NULL)
        	{
            		setenv("dc_poweroff", "0");
        	}
		saveenv();
#if (CONFIG_WDT_RESET_BY_ESD)
        MS_BOOL bWDTResetToPowerOff = FALSE;
    #if (WDT_STANDBY_MODE == 0)
        MDrv_WDT_Init(E_WDT_DBGLV_ALL);
    #endif
        if(MDrv_WDT_IsReset())      // when we running ESD test, system will reset by wdt
        {
            UBOOT_DEBUG("System is reset by watchdog !!!\n");
            if(!get_poweroff_flag())
            {
                UBOOT_DEBUG("The last status is power on, system will not enter in standby mode\n");
                return 0;
            }
            else
            {
                UBOOT_DEBUG("The last status is power down, system will enter in standby mode\n");
                bWDTResetToPowerOff = TRUE;
            }
        }
#endif

#if (CONFIG_WDT_RESET_BY_ESD)
        if(bWDTResetToPowerOff || check_pm_standby())
#else
        if(check_pm_standby())
#endif
        {
            #if (CONFIG_WDT_RESET_BY_ESD)
            if (!bWDTResetToPowerOff)
            {
                set_poweroff_flag(TRUE);
            }
            #endif

            // load mtk bt patch , and send woble cmd
            #if (CONFIG_MTK_BT_USB)
            run_command("setMtkBT", 0);
            #endif
            UBOOT_INFO("Led: AC ON boot to standby case\n");

            unsigned int percertage_x100 = 1500;
            char *led_standby_setting = getenv("led_standby_setting");

            if(led_standby_setting && (strcmp(getenv("led_standby_setting"), "0") == 0)) {
                percertage_x100 = 0;
            } else {
                char *standby_led = getenv("standby_led");

                if (standby_led) {
                    sscanf(standby_led, "%u", &percertage_x100);
                } else {
                    UBOOT_INFO("standby_led not set, use default setting\n");
                }
            }
// JULIANA series uses GPIO instead of PWM
#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_JULIANA)
            {
                #define PROD_VAR_SIZE 32
                char led_config[PROD_VAR_SIZE+1] = {0,};
                idme_get_oem_data_field("led_config=", led_config, PROD_VAR_SIZE);

                UBOOT_INFO("standby_led %u.%u%%\n", (percertage_x100 / 100), (percertage_x100 % 100));
                if (percertage_x100==0)
                {
                    // for 2-color LED case, "led_config=1" in oem_data field
                    if (strncmp(led_config, "1", 1) == 0)
                    {
                        UBOOT_INFO("Pull up GPIO1_PM/GPIO5_PM to turn off LED (red/green)\n");
                        mdrv_gpio_set_high(PAD_GPIO1_PM);
                        mdrv_gpio_set_high(PAD_GPIO5_PM);
                    }
                    else
                    {
                        UBOOT_INFO("Pull down GPIO1_PM to turn off LED (red)\n");
                        mdrv_gpio_set_low(PAD_GPIO1_PM);
                    }
                }
            }
#else
            snprintf(ledCmd, (sizeof(ledCmd) - 1),"led_pwm %u %u",
                (percertage_x100 / 100),
                (percertage_x100 % 100));

            UBOOT_INFO("standby_led %u.%u%%\n", (percertage_x100 / 100), (percertage_x100 % 100));
            run_command(ledCmd, 0);
#endif
#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_BRANDENBURG) || defined(CONFIG_MTK_BD_MT164B_10AT_M7632_ANNA)
            UBOOT_INFO("Pull down GPIO9_PM to shutdown PSU\n");
            mdrv_gpio_set_low(PAD_GPIO9_PM);
#endif
            //PM51_PowerDown();
            run_command("pm51 standby",0);
        }
        else
        { //to do: patch need to remove
        }
    }

    else
    {//to do: patch need to remove
    }
    UBOOT_TRACE("OK\n");
    return 0;
}

int do_if_boot_to_pm( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    If_Boot_To_PM();
    return 0;
}

int do_pm51( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int ret =0;
    char * u8Command = NULL;
    UBOOT_TRACE("IN\n");
    if (argc < 2 || argv[1]==NULL)
    {
        cmd_usage(cmdtp);
        return 0;
    }
    u8Command = argv[1];

    if(strncmp(u8Command, "stop", 4) == 0)
    {
        UBOOT_INFO("Stop PM51!\n");
        *(volatile U32*)(MS_RIU_MAP+(0x1018<<1))=0x0000;
    }
    else if(strncmp(u8Command, "start", 6) == 0)
    {
        UBOOT_INFO("Start PM51!\n");
        *(volatile U32*)(MS_RIU_MAP+(0x1018<<1))=0x000e;
    }
    else if(strncmp(u8Command, "readpc", 6) == 0)
    {
        UBOOT_INFO("PM51[PC]=0x%x\n",get_pm51_program_counter());
    }
    else if(strncmp(u8Command, "standby", 7) == 0)
    {
         ret = PM51_PowerDown();
    }
    UBOOT_TRACE("OK\n");
    return ret;
}


