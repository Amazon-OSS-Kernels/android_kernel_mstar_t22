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

#include <exports.h>
#include <config.h>
#include <mmc.h>
#include <CusSystem.h>
#include <uboot_mmap.h>
#include <ShareType.h>
#include <MsVfs.h>
#include <MsUtility.h>
#include <MsApiMiu.h>
#include <MsDebug.h>
#include <MsRawIO.h>
#include <MsEnvironment.h>
#include <MsSysUtility.h>
#include <drvPM.h>
#if defined (CONFIG_SET_4K2K_MODE)
#include <apiPNL.h>
#include <apiGOP.h>
#include <MsBoot.h>
#endif
#if defined (CONFIG_URSA_6M40)
#include <ursa/ursa_6m40.h>
#endif
#if defined (CONFIG_URSA_8)
#include <ursa/ursa_8.h>
#endif
#if (ENABLE_MODULE_BOOT_IR == 1)
#include <MsOS.h>
#endif
#include <CusConfig.h>
#if defined (CONFIG_INX_VB1)
#include <panel/panel_INX_vb1.h>
#elif defined (CONFIG_INX_NOVA_VB1)
#include <panel/panel_INX_NOVA_vb1.h>
#endif

#include <drvMIU.h>
#include <apiPNL.h>
#include <MsMmap.h>
#include <MsSystem.h>
#include <mstarstr.h>
#include <bootlogo/MsPoolDB.h>
#include <MsApiPanel.h>
#include <MsUboot.h>

#if defined(AMZN_FTVE_FRC_SIGNING_ENABLED)
#include "amzn_secure_boot.h"
#define FRC_START_SIZE 0xFFF00
#define FRC_SIG_LEN (256)
#define FRC_SIG_START_SIZE 0x100000
#endif

 /* whenever there is db/config data structure change, increase this by 1 */
#define MBOOT_DB_VERSION_NAME  "MBOOT_DB_VERSION"
#define MBOOT_DB_VERSION_VALUE "1"

#ifdef UFBL_FEATURE_IDME
#include <idme.h>
#else
extern int snprintf(char *str, size_t size, const char *fmt, ...);
#endif

#if CONFIG_RESCUE_ENV
char *CUS_BOOT_RECOVER_ENV_LIST[] =
{
    NULL
};
#endif

#if defined(CONFIG_MTK_WIFI_7668)
int do_MTK_wifi_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
    Wifi_Reset_ON();
    mdelay(2);
    Wifi_Reset_OFF();

#if defined(CONFIG_MULTI_PCB_WIFI_RESET)
    UBOOT_TRACE("Wifi Reset for OLD PCB\n");
    Wifi_Reset_OLD_PCB_ON();
    mdelay(2);
    Wifi_Reset_OLD_PCB_OFF();
#endif

    UBOOT_TRACE("OK\n");
    return 0;
}
#endif

#if defined(CONFIG_ENABLE_MIU_SSC)
int do_MIU_SSC (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
    MDrv_MIU_SetSscValue(E_MIU_0,(MS_U16)MIU_SSC_MODULATION, (MS_U16)MIU_SSC_DEVIATION,(MS_BOOL)MIU0_SSC_ENABLE);
    MDrv_MIU_SetSscValue(E_MIU_1,(MS_U16)MIU_SSC_MODULATION, (MS_U16)MIU_SSC_DEVIATION,(MS_BOOL)MIU0_SSC_ENABLE);
    MDrv_MIU_SetSscValue(E_MIU_2,(MS_U16)MIU_SSC_MODULATION, (MS_U16)MIU_SSC_DEVIATION,(MS_BOOL)MIU0_SSC_ENABLE);
    UBOOT_TRACE("OK\n");
    return 0;
}
#endif


int do_envload (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    uchar bReloadEnv = 0;
    char *envbuffer;
    envbuffer=NULL;

#if defined(CONFIG_ENV_IS_IN_SPIFLASH)
    extern int env_isempty(void);
    if(env_isempty())
    {
        printf("\n <env reload for CRC>\n");
        bReloadEnv = 1;
    }
#endif

#if defined(CONFIG_LOAD_ENV_FROM_SN) || defined(CONFIG_Customer)
    envbuffer = getenv ("mboot_default_env");
    if(envbuffer)
    {
        if(simple_strtoul (envbuffer, NULL, 10))
        {
            printf("\n <env reload for CHECK_IF_MBOOT_DEFAULT_ENV>\n");
            bReloadEnv = 1;
        }
    }

    if (getenv("bootargs") == NULL)
    {
        UBOOT_DEBUG("get env configs from SN\n");
        bReloadEnv = 1;
    }
#if (ENABLE_MODULE_ANDROID_BOOT == 1 )
    else if(getenv("recoverycmd") == NULL)
    {
        UBOOT_DEBUG("get env configs from SN\n");
        /* ACOS_MOD_BEGIN {amazon_common_kernel_signing_scheme} */
        /* amazon kernel signing: recoverycmd not used */
        // bReloadEnv = 1;
        /* ACOS_MOD_END {amazon_common_kernel_signing_scheme} */
    }
#endif
#endif

// check INIT_MBOOT_ENV bit in idme dev_flags
#if defined(UFBL_FEATURE_IDME)
    if (!bReloadEnv) {
        char* dev_flags_str = NULL;
        unsigned long dev_flags = 0;
        char dev_flags_buf[8] = {0};
        if ((dev_flags_str = getenv("dev_flags")) != NULL)
            dev_flags = simple_strtoul(dev_flags_str, NULL, 16);
        if ((dev_flags & DEV_FLAGS_INIT_MBOOT_ENV) == DEV_FLAGS_INIT_MBOOT_ENV) {
            UBOOT_DEBUG("reload env based on idme dev_flags\n");
            bReloadEnv = 1;
            dev_flags &= ~DEV_FLAGS_INIT_MBOOT_ENV; // clear the bit
            snprintf(dev_flags_buf, sizeof(dev_flags_buf), "%x", dev_flags);
            if (idme_update_var_ex("dev_flags", dev_flags_buf, sizeof(dev_flags_buf)))
            {
               printf("error updating dev_flags\n");
            }
        }
    }
#endif

    if (!bReloadEnv)
    {
        envbuffer = getenv (MBOOT_DB_VERSION_NAME);
        if((!envbuffer) || (strcmp(envbuffer, MBOOT_DB_VERSION_VALUE)))
        {
#if defined(CONFIG_AN_FASTBOOT_ENABLE)
            // for fastboot reboot-bootloader command
            char *sFastboot = NULL;
            sFastboot = getenv("reboot-bootloader");
            if (!(sFastboot) || (sFastboot[0] != '1')) {
                printf("\n <env reload for correct db version:%s\n",
                       MBOOT_DB_VERSION_VALUE);
                bReloadEnv = 1;
            }
#else
            printf("\n <env reload for correct db version:%s\n",
                   MBOOT_DB_VERSION_VALUE);
            bReloadEnv = 1;
#endif
        }
    }

    if(bReloadEnv)
    {
        char cmd[128]="\0";
        // load env from /system/etc/set_env
        snprintf(cmd,sizeof(cmd)-1,"loadenv %s %s", SET_ENV_PATITION, SET_ENV_FILE);
        run_command(cmd,0);
    }
#if (ENABLE_MODULE_ANDROID_BOOT == 1 )
    else
    {
        // reload env after reboot in recovery mode.
        extern int get_boot_status_from_mtd0(void);
        get_boot_status_from_mtd0();
    }
#endif

     return 0;
}

#if defined (CONFIG_MBOOT_VERSION)
int do_setmbootver (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]){
    char *s;
    if((s = getenv (USB_MBOOT_VERSION))!= NULL)
    {
        if(strcmp(s,USB_MBOOT_CURRENT_VERSION))
        {
            setenv(USB_MBOOT_VERSION, USB_MBOOT_CURRENT_VERSION);
            saveenv();
        }
    }
    else
    {
        setenv(USB_MBOOT_VERSION, USB_MBOOT_CURRENT_VERSION);
        saveenv();
    }
    return 0;
}
#endif



#define BOOT_IF_ACTION(c, b)                ((c) & (b))

int get_boot_status_from_mtd0(void)
{
    int ret = 0;
    struct bootloader_message *p_msg = NULL;
    char cmd[128];

    p_msg = (struct bootloader_message*)malloc(sizeof(struct bootloader_message));
    if(NULL == p_msg)
    {
        UBOOT_ERROR("malloc bootloader_message buffer fail!\n");
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    memset(p_msg, 0, sizeof(struct bootloader_message));

#if (ENABLE_MODULE_MMC == 1)
    snprintf(cmd, sizeof(cmd)-1, "mmc read.p 0x%08lX misc 40", (unsigned long)p_msg);
#else
    #if !CONFIG_MSTAR_FTL
    snprintf(cmd, sizeof(cmd)-1, "nand read.e 0x%08lX misc 40", (unsigned long)p_msg);
    #else
    snprintf(cmd, sizeof(cmd)-1, "ftl read.p 0x%08lX misc 40", (unsigned long)p_msg);
    #endif
#endif
    if(-1 != run_command(cmd, 0))
    {
        UBOOT_DEBUG("read the misc partion data\n");
        memset(cmd, 0, sizeof(cmd));
        if ((strlen(p_msg->command) == strlen(BOOT_STATUS_CUSTOMER_ACTIONS_STR)) && (0==strncmp(p_msg->command, BOOT_STATUS_CUSTOMER_ACTIONS_STR, strlen(BOOT_STATUS_CUSTOMER_ACTIONS_STR))))
        {
            int reloadEnv   = 0;
            int reloadPanel = 0;
            char actionByte = p_msg->status[0];
            if (BOOT_IF_ACTION(actionByte, BOOT_STATUS_ACTION_RELOADPANEL_BIT))
            {
                reloadPanel = 1;
            }
            if (BOOT_IF_ACTION(actionByte, BOOT_STATUS_ACTION_RELOADENV_BIT))
            {
                reloadEnv = 1;
            }

            memset(p_msg->command, 0, sizeof(p_msg->command));
            memset(p_msg->status, 0, sizeof(p_msg->status));
            memset(cmd, 0, sizeof(cmd));
#if (ENABLE_MODULE_MMC == 1)
            snprintf(cmd, sizeof(cmd)-1, "mmc write.p 0x%08lX misc 40", (unsigned long)p_msg);
#else
            #if !CONFIG_MSTAR_FTL
            snprintf(cmd, sizeof(cmd)-1, "nand write.e 0x%08lX misc 40", (unsigned long)p_msg);
            #else
            snprintf(cmd, sizeof(cmd)-1, "ftl write.p 0x%08lX misc 40", (unsigned long)p_msg);
            #endif
#endif
            run_command(cmd, 0);

            if (reloadPanel)
            {
                setenv("db_table","0");
                saveenv();
            }

            if (reloadEnv)
            {
                // reload env
                char cmd[128]="\0";
                snprintf(cmd,sizeof(cmd)-1,"loadenv %s %s", SET_ENV_PATITION, SET_ENV_FILE);
                run_command(cmd,0);
            }
        }
    }
    else
    {
        UBOOT_ERROR("read misc partition data failed\n");
        ret = -1;
    }

    free(p_msg);
    return ret;
}

#ifdef CONFIG_SHRUNK_USERDATA_FIXUP
void fix_shrunk_userdata()
{
#if defined(UFBL_FEATURE_IDME)
    int counter = 0;
    int ret = 0;
    char sConfigName[BUFFER_SIZE] = "\0";
    char sModelArray[][BUFFER_SIZE] = {
#if defined(CONFIG_MTK_BD_MT168B_10AT_19133_MT5870_M7332_ABC)
        "ABC_pvt",
#endif
#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_JULIANA)
        "juliana_pvt",
#endif
#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_BRANDENBURG)
        "brandenburg_pvt_4",
        "ABC_hvt",
#endif
#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_ANNA)
        "anna_pvt",
        "ABC_pvt",
#endif
    };

    idme_get_var_external("config_name", sConfigName, (BUFFER_SIZE - 1));

    UBOOT_DEBUG("config_name = %s\n", sConfigName);

    // Check each model needs fixup
    for(counter = 0; counter < sizeof(sModelArray)/BUFFER_SIZE ; counter++)
    {
        if(!strncmp(sConfigName, sModelArray[counter], sizeof(sConfigName)))
        {
            ret = 1;
            break;
        }
    }

    if(ret == 0)
        return;

    char bootmode_buf[6] = {0};
    int bootmode = 1;
    if (!idme_get_var_external("bootmode", bootmode_buf, sizeof(bootmode_buf)-1)) {
            bootmode = atoi(bootmode_buf);
            UBOOT_DEBUG("idme bootmode read [%d]\n",bootmode);
    }else {
            UBOOT_ERROR("idme bootmode failed");
            bootmode = 1;
    }

    // Check if it's in FOS mode
    if(bootmode != IDME_BOOTMODE_NORMAL)
        return;

    #ifdef CONFIG_DOUBLE_MBOOT
        #define RESERVE_BLOCK EMMC_PARTITION_START_V3 //reserved block
    #else
        #define RESERVE_BLOCK 4160 //Emmc table:64block + Emmc driver:2MB
    #endif

    block_dev_desc_t *mmc_dev = NULL;
    disk_partition_t dpt;
    struct mmc *mmc = find_mmc_device(0);
    unsigned int u32MmcCapacityBlock =0;
    unsigned int u32RemainingBlock = 0;
    char u8Name[32] = {'\0'};
    unsigned long partnum = 1;
    unsigned long total_partnum = 0;

    mmc_dev = mmc_get_dev(0);

    if(mmc_dev == NULL || mmc == NULL)
        return;

    // Block size 512bytes
    u32MmcCapacityBlock = mmc->capacity / 512;
    u32RemainingBlock = u32MmcCapacityBlock - get_emmc_used_blockcount(mmc_dev);

    UBOOT_DEBUG("u32MmcCapacityBlock %d, u32RemainingBlock %d\n", u32MmcCapacityBlock, u32RemainingBlock);

    // Check if remaining size < 500MB(1024000 blocks)
    if(u32RemainingBlock < (1024000 + RESERVE_BLOCK))
        return;

    for(;;)
    {
        if(get_partition_info_emmc(mmc_dev, partnum, &dpt))
        {
            UBOOT_ERROR("Error >> end searching partition\n");
            return;
        }
        if(!strcmp("userdata", (const char *)dpt.name))
        {
            UBOOT_DEBUG("name=%s\n",(const char *)dpt.name);
            UBOOT_DEBUG("size=0x%x # number of blocks \n",(unsigned int)dpt.size);
            UBOOT_DEBUG("blksz=0x%x\n",(unsigned int)dpt.blksz);
            break;
        }
        partnum++;
    }

    total_partnum = get_emmc_partition_number(mmc_dev);
    UBOOT_DEBUG("total_partnum %d, partnum: %d\n", total_partnum, partnum);

    // Check if userdata is the last partition
    if(total_partnum != partnum)
        return;

    UBOOT_DEBUG("\n[AT][begin erase][%lu]\n", MsSystemGetBootTime());
    run_command("mmc remove userdata", 0);
    run_command("mmc create userdata MAX", 0);
    UBOOT_DEBUG("\n[AT][end erase][%lu]\n", MsSystemGetBootTime());

    if(!get_partition_info_emmc(mmc_dev, partnum, &dpt))
    {
        printf("userdata resized.\n");
        UBOOT_DEBUG("name=%s\n",(const char *)dpt.name);
        UBOOT_DEBUG("size=0x%x # number of blocks \n",(unsigned int)dpt.size);
        UBOOT_DEBUG("blksz=0x%x\n",(unsigned int)dpt.blksz);
    }
#endif
    return;
}
#endif

int do_initenv(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

    UBOOT_TRACE("IN\n");

#ifdef CONFIG_SHRUNK_USERDATA_FIXUP
    fix_shrunk_userdata();
#endif
    run_command("mmc erase.p MPOOL", 0);
    run_command("cleanallenv", 0);
    setenv("bootargs", "console=ttyS0,115200 androidboot.console=ttyS0 init=/init CORE_DUMP_PATH=/data/core_dump.%%p.gz KDebug=1 delaylogo=true security=selinux platform=sn SD_CONFIG=2 tee_mode=optee pm_path=/tvconfig/config/PM.bin loop.max_part=7 vmalloc=500M");
    setenv("bootlogo_gopidx","1");
    setenv("bootlogo_buffer", "E_MMAP_ID_BOOTLOGO_BUFFER");
    //setenv("OSD_BufferAddr", "E_MMAP_ID_JPD_WRITE_ADR");
    setenv("str_crc", "2");
    setenv("db_table", "0");
    setenv("verify", "n");
    setenv("WDT_ENABLE", "1");
    setenv("sync_mmap", "1");
    setenv("CONFIG_PATH", "/vendor/tvconfig/config");
    setenv("mboot_default_env", "0");
    setenv("MAP_TYPE", "MI");
    setenv("MI_MAP_PARTITION", "tvconfig");
    setenv("MI_MAP_PATH", "config/MMAP_MI.h");
    setenv("customer_ini_path", "/tvconfig/config/model/Customer_1.ini");
    setenv("_BootlogoFile", "/tvconfig/bootlogo.jpg");
    setenv("devicestate", "lock");
    setenv("unlock_cmi", "1");
    setenv("kernel_loglevel", "6");
    setenv("bt_usb_port", "2");
    setenv(MBOOT_DB_VERSION_NAME, MBOOT_DB_VERSION_VALUE);
    saveenv();
    run_command("sync_mmap", 0);
    run_command("dbtable_init 1", 0);


    UBOOT_TRACE("OK\n");

    return 0;
}

int do_loadenv(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");

    run_command("initenv", 0);
    run_command("reset",0);

#if 0 // run initenv command instead of loading from emmc
    if(argc < 3)
    {
        cmd_usage(cmdtp);
        return -1;
    }

    if(argv[1] == NULL)
    {
        cmd_usage(cmdtp);
        return -1;
    }

    if(argv[2] == NULL)
    {
        cmd_usage(cmdtp);
        return -1;
    }
    UBOOT_DEBUG("load env from partition -> %s path -> %s\n",argv[1],argv[2]);
#if ENABLE_MODULE_LOAD_ENV_FROM_SN
    vfs_mount(argv[1]);
    runscript_linebyline(argv[2]);
    vfs_umount();
#endif

#endif // run initenv command instead of loading from emmc
    UBOOT_TRACE("OK\n");

    return 0;

}

#if defined (CONFIG_SET_4K2K_MODE)
extern U8 g_logoGopIdx;

MS_BOOL __Sc_is_interlace(void)
{
    return 0;
}

MS_U16 __Sc_get_h_cap_start(void)
{
    return 0x60;
}

void __Sys_PQ_ReduceBW_ForOSD(MS_U8 PqWin, MS_BOOL bOSD_On)
{

}

int do_setFRC(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    GOP_InitInfo pGopInit;

    if(get_poweroff_flag())
    {
        memset((void *)&pGopInit, 0, sizeof(pGopInit));
        MApi_GOP_RegisterXCIsInterlaceCB(__Sc_is_interlace);
        MApi_GOP_RegisterXCGetCapHStartCB(__Sc_get_h_cap_start);
        MApi_GOP_RegisterXCReduceBWForOSDCB(__Sys_PQ_ReduceBW_ForOSD);
        pGopInit.u16PanelWidth = g_IPanel.Width();
        pGopInit.u16PanelHeight = g_IPanel.Height();
        pGopInit.u16PanelHStr = g_IPanel.HStart();
        pGopInit.u32GOPRBAdr = 0x0;
        pGopInit.u32GOPRBLen = 0x0;

        pGopInit.u32GOPRegdmaAdr = 0;
        pGopInit.u32GOPRegdmaLen = 0;
        pGopInit.bEnableVsyncIntFlip = FALSE;

        MApi_GOP_InitByGOP(&pGopInit, g_logoGopIdx);
        MApi_GOP_GWIN_SetGOPDst(g_logoGopIdx, E_GOP_DST_FRC);
    }
    return 0;
}

int Set_4K2K_OP0(void)
{
    GOP_InitInfo pGopInit;

    //close lvds
    MDrv_Ursa_6M40_Set_Lvds_Off();

    //disable osd
    MDrv_Ursa_6M40_Set_Osd_Off();

    //set OP0
    memset((void *)&pGopInit, 0, sizeof(pGopInit));
    MApi_GOP_RegisterXCIsInterlaceCB(__Sc_is_interlace);
    MApi_GOP_RegisterXCGetCapHStartCB(__Sc_get_h_cap_start);
    MApi_GOP_RegisterXCReduceBWForOSDCB(__Sys_PQ_ReduceBW_ForOSD);
    pGopInit.u16PanelWidth = g_IPanel.Width();
    pGopInit.u16PanelHeight = g_IPanel.Height();
    pGopInit.u16PanelHStr = g_IPanel.HStart();
    pGopInit.u32GOPRBAdr = 0x0;
    pGopInit.u32GOPRBLen = 0x0;

    pGopInit.u32GOPRegdmaAdr = 0;
    pGopInit.u32GOPRegdmaLen = 0;
    pGopInit.bEnableVsyncIntFlip = FALSE;

    MApi_GOP_InitByGOP(&pGopInit, g_logoGopIdx);
    MApi_GOP_GWIN_SetGOPDst(g_logoGopIdx, E_GOP_DST_OP0);

    //open lvds
    MDrv_Ursa_6M40_Set_Lvds_On();

    return 0;
}
#endif

#if defined (CONFIG_URSA_6M40)
int do_ursa_lvds_on (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_BOOL bRet = FALSE;
    bRet = MDrv_Ursa_6M40_Set_Lvds_On();
    return 0;
}
int do_ursa_lvds_off(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_BOOL bRet = FALSE;
    bRet = MDrv_Ursa_6M40_Set_Lvds_Off();
    return 0;
}

int do_ursa_osd_unmute (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_BOOL bRet = FALSE;
    bRet = MDrv_Ursa_6M40_Set_Osd_Unmute();
    return 0;
}
int do_ursa_2k_mode_on (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_BOOL bRet = FALSE;
    bRet = MDrv_Ursa_6M40_Set_2K_Mode_On();
    return 0;
}

int do_ursa_set_osd_mode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u16 protect_mode =0;
    int ret =0;

    protect_mode = g_UrsaCMDGenSetting.g_OsdMode.protect_mode;
    if(protect_mode>=0 && protect_mode < 3)
    {

        printf("osd_protect_mode = %d\n",protect_mode);
            switch(protect_mode)
                {
                case URSA_6M40_OSD_PROTECT_OFF:
                    {
                        MDrv_Ursa_6M40_SendCmd(CMD_6M40_OSD_PROTECT_OFF);
                    }
                    break;
                case URSA_6M40_OSD_PROTECT_ON:
                    {
                        MDrv_Ursa_6M40_SendCmd(CMD_6M40_OSD_PROTECT_ON);
                    }
                    break;
                case URSA_6M40_OSD_PROTECT_ON_EMMC:
                    {
                        MDrv_Ursa_6M40_SendCmd(CMD_6M40_OSD_PROTECT_ON_EMMC);
                    }
                    break;
                default:
                         break;
                }
        }
        else
        {
            UBOOT_ERROR("OSD protect Mode setting not correct\n");
            ret = -1;
        }
    return ret;
}

#endif

#if defined (CONFIG_ENABLE_4K2K_PANEL)
int do_inx_panel_set_init (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined (CONFIG_INX_VB1)
    MDrv_Panel_INX_VB1_Set_Pre_Init();
#elif defined(CONFIG_INX_NOVA_VB1)
    MDrv_Panel_INX_NOVA_VB1_Unlock_AHB();
#endif
    return 0;
}

int do_inx_panel_set_fhd (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined (CONFIG_INX_VB1)
    MDrv_Panel_INX_VB1_Set_FHD();
#elif defined (CONFIG_INX_LVDS)
    MDrv_PANEL_INX_LVDS_Set_FHD();
#endif
    return 0;
}

int do_inx_panel_set_4k2k (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined (CONFIG_INX_VB1)
    MDrv_Panel_INX_VB1_Set_4K2K();
#elif defined (CONFIG_INX_LVDS)
    MDrv_PANEL_INX_LVDS_Set_4K2K();
#elif defined (CONFIG_INX_NOVA_VB1)
    MDrv_Panel_INX_NOVA_VB1_Set_UHD_DIVISION(UHD_2_DIVISION);
#endif
    return 0;
}

#if defined (CONFIG_INX_NOVA_VB1)
int do_inx_nova_set_4k2k_2division (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MDrv_Panel_INX_NOVA_VB1_Set_UHD_DIVISION(UHD_2_DIVISION);
    return 0;
}
#endif

#if defined (CONFIG_INX_VB1)
int do_panel_inx_vb1_init (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#if defined(CONFIG_ENABLE_4K2K_PANEL)
    MDrv_Panel_INX_VB1_RX_INIT();
#endif
    return 0;
}
#endif
#endif

#if defined (CONFIG_URSA_8)
int do_ursa8_lvds_on (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_BOOL bRet = FALSE;
    bRet = MDrv_Ursa_8_Set_Lvds_On();
    return 0;
}


int do_ursa8_set_osd_mode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u16 protect_mode =0;
    int ret =0;

    protect_mode = g_UrsaCMDGenSetting.g_OsdMode.protect_mode;
    if(protect_mode>=0 && protect_mode < 3)
    {

        printf("osd_protect_mode = %d\n",protect_mode);
            switch(protect_mode)
                {
                case URSA_8_OSD_PROTECT_OFF:
                    {
                        MDrv_Ursa_8_SendCmd(URSA_8_CMD_OSD_PROTECT_OFF);
                    }
                    break;
                case URSA_8_OSD_PROTECT_ON:
                    {
                        MDrv_Ursa_8_SendCmd(URSA_8_CMD_OSD_PROTECT_ON);
                    }
                    break;
                case URSA_8_OSD_PROTECT_ON_EMMC:
                    {
                        MDrv_Ursa_8_SendCmd(URSA_8_CMD_OSD_PROTECT_ON_EMMC);
                    }
                    break;
                default:
                         break;
                }
        }
        else{
            UBOOT_ERROR("OSD protect Mode setting not correct\n");
            ret = -1;
            }


    return ret;

}

#endif
#if (ENABLE_MODULE_BOOT_IR == 1)
int do_ir_delay(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char *pEnv = getenv("ir_delay");
    int delay_time = 0;

    if (pEnv != NULL)
    {
        delay_time = (int)simple_strtol(pEnv, NULL, 10);
        if (delay_time <= 0)
        {
            printf("ir_delay = %d, Skip ir delay, for performance\n", delay_time);
        }
        else
        {
            mdelay(delay_time); //sleep
        }
    }

    return 0;
}
#endif

#if(CONFIG_ENABLE_V_I_PWM == 1)
#define BUFFER_SIZE 128
int do_check_safemode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   U8 eep_safemode = 0xff;
   MS_BOOL change_flag = FALSE;
    char str[BUFFER_SIZE];
   char * env_safemode = getenv("safemodeset");
   printf("---wya--- do_check_safemode\n");

   memset(str, 0 , BUFFER_SIZE);
   snprintf(str, BUFFER_SIZE, "eeprom read 0x%x 0x001E 1", (MS_PHYADDR)&eep_safemode);
   run_command(str, 0);

   printf("---wya--- env_safemode =%s\n",env_safemode);
   printf("---wya--- eep_safemode =%d\n",eep_safemode);

   if(env_safemode==NULL && eep_safemode==0xff)
   {
       printf("---wya--- smt progress!! set safemode to off \n");
       setenv("safemodeset","off");
       eep_safemode = 0;
       memset(str, 0 , BUFFER_SIZE);
       snprintf(str, BUFFER_SIZE, "eeprom write 0x%x 0x001E 1", (MS_PHYADDR)&eep_safemode);
       run_command(str, 0);
       return 1;
   }
   if(strcmp(env_safemode, "on")!= 0 && strcmp(env_safemode, "off")!= 0)
   {
       printf("---wya--- other case,we need to set safemode on \n");
       setenv("safemodeset","on");
       eep_safemode = 1;
       memset(str, 0 , BUFFER_SIZE);
       snprintf(str, BUFFER_SIZE, "eeprom write 0x%x 0x001E 1", (MS_PHYADDR)&eep_safemode);
       run_command(str, 0);
       return 1;
   }


   printf("---wya--- before change eep_safemode =%d , env_safemode=%s\n",eep_safemode,env_safemode);
   if(strcmp(env_safemode, "on")== 0 && eep_safemode!= 1)
   {
       eep_safemode = 1;
       memset(str, 0 , BUFFER_SIZE);
       snprintf(str, BUFFER_SIZE, "eeprom write 0x%x 0x001E 1", (MS_PHYADDR)&eep_safemode);
       run_command(str, 0);
       //printf("---wya--- run eeprom wb 0x001E 1 \n");
       //run_command("eeprom wb 0x001E 1", 0);
       change_flag = TRUE;
   }
   if(strcmp(env_safemode, "off")== 0 && eep_safemode!= 0)
   {
       eep_safemode = 0;

       memset(str, 0 , BUFFER_SIZE);
       snprintf(str, BUFFER_SIZE, "eeprom write 0x%x 0x001E 1", (MS_PHYADDR)&eep_safemode);
       run_command(str, 0);
       //printf("---wya--- run eeprom wb 0x001E 0 \n");
       //run_command("eeprom wb 0x001E 0", 0);
       change_flag = TRUE;
   }
   if(change_flag == TRUE)
   {
       memset(str, 0 , BUFFER_SIZE);
       snprintf(str, BUFFER_SIZE, "eeprom read 0x%x 0x001E 1", (MS_PHYADDR)&eep_safemode);
       run_command(str, 0);
       printf("---wya--- after eep_safemode =%d , env_safemode=%s\n",eep_safemode,env_safemode);
       return 1;
   }
   return 0;
}

int do_safemode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   int ret = 0;
    char str[BUFFER_SIZE];
   int eep_safemode = -1;
   if (argc < 2)
       return cmd_usage(cmdtp);
   if (strcmp(argv[1], "get") == 0) {
       char * val = getenv("safemodeset");
       printf("---wya--- safemode get =%s\n",val);
   }else if(strcmp(argv[1], "set") == 0) {
       printf("---wya--- safemode get arg[2]=%s\n",argv[2]);
       if(strcmp(argv[2], "off") == 0)
       {
           eep_safemode = 0;
           setenv("safemodeset","off");
           saveenv();
       }
       else
       {
           eep_safemode = 1;
           setenv("safemodeset","on");
           saveenv();
       }

       memset(str, 0 , BUFFER_SIZE);
       snprintf(str, BUFFER_SIZE, "eeprom write 0x%x 0x001E 1", (MS_PHYADDR)&eep_safemode);
       run_command(str, 0);
       //if(eep_safemode==1)
       //  run_command("eeprom wb 0x001E 1", 0);
       //else
       //  run_command("eeprom wb 0x001E 0", 0);

       memset(str, 0 , BUFFER_SIZE);
       snprintf(str, BUFFER_SIZE, "eeprom read 0x%x 0x001E 1", (MS_PHYADDR)&eep_safemode);
       run_command(str, 0);
       printf("---wya--- eeprom read index=%d\n",eep_safemode);
       run_command("reset",0);
   }
    return ret;
}
#endif

#if (CONFIG_URSA12_VB1_FRC_BRINGUP)//alex_tung###

//################################
#include <MsSystem.h>
#include <MsSysUtility.h>
#include "drvCPU.h"
#include "drvMBX.h"
//################################
#define FRCR2_MBX_QUEUESIZE     8
#define FRCR2_MBX_TIMEOUT       5000
//################################
static MS_BOOL FRCR2_MBX_Init(void)
{
    MBX_CPU_ID eHKCPU;
    MS_U32 u32TimeoutMillSecs = 1500;

    MDrv_MBX_SetDbgLevel(MBX_DBG_LEVEL_ALL);

    //Initialize Mailbox
    eHKCPU = E_MBX_CPU_MIPS;
    if( E_MBX_SUCCESS != MDrv_MBX_Init(eHKCPU, E_MBX_ROLE_HK, u32TimeoutMillSecs))
    {
        printf("[HKCPU] FRCR2_MBX_Init - Fail !!!!\n");
        while(1);
    }
    else
    {
        MDrv_MBX_Enable(TRUE);
        //MDrv_MBX_SetCallDrvFlag(TRUE);
    }

    //Register class for R2
    if( E_MBX_SUCCESS != MDrv_MBX_RegisterMSG(E_MBX_CLASS_FRC, FRCR2_MBX_QUEUESIZE))
    {
        printf("[HKCPU] FRCR2_MBX_Init - register msg error !!!\n");
        return FALSE;
    }

    return TRUE;
}

static MS_BOOL FRCR2_MBX_SendMsg(U8 u8CmdIdx,U32 u32Data)
{
    MBX_Msg stMB_Command;
    MBX_Result enMbxResult = 0;

    //Write2Byte(0x101EA6,0xFFF2);//alex_tung###

    memset((void*)&stMB_Command, 0x00, sizeof(MBX_Msg));
    //(1) send to CB
    stMB_Command.eRoleID = E_MBX_ROLE_FRC;
    stMB_Command.eMsgType = E_MBX_MSG_TYPE_INSTANT;
    stMB_Command.u8Ctrl = 0;
    stMB_Command.u8MsgClass = E_MBX_CLASS_FRC;
    stMB_Command.u8Index = u8CmdIdx;
    stMB_Command.u8ParameterCount = 4;
    stMB_Command.u8Parameters[0] =  (U8)((u32Data>>24)&0xFF);
    stMB_Command.u8Parameters[1] =  (U8)((u32Data>>16)&0xFF);
    stMB_Command.u8Parameters[2] =  (U8)((u32Data>>8)&0xFF);
    stMB_Command.u8Parameters[3] =  (U8)((u32Data>>0)&0xFF);
    enMbxResult = MDrv_MBX_SendMsg(&stMB_Command);

    return (enMbxResult==E_MBX_SUCCESS)? TRUE : FALSE;
}

static MS_BOOL FRCR2_MBX_RecvMsg(void)
{
    MBX_Msg stMB_Command;
    MBX_Result enMbxResult = 0;

    Write2Byte(0x101EA6,0xFFF4);//alex_tung###
    do
    {
        memset((void*)&stMB_Command, 0x00, sizeof(MBX_Msg));
        enMbxResult = MDrv_MBX_RecvMsg(E_MBX_CLASS_FRC, &stMB_Command, FRCR2_MBX_TIMEOUT, MBX_CHECK_INSTANT_MSG);
    }while((enMbxResult  !=  E_MBX_SUCCESS) && (enMbxResult  !=  E_MBX_ERR_TIME_OUT));


    //(3) check result
    if(enMbxResult == E_MBX_ERR_TIME_OUT)
    {
        printf("[HK] do_frc_send : FAIL to receive message from FRCR2\n");
    }
    else
    {
        printf("[HK] do_frc_send : PASS to receive from FRCR2\n");
        printf("\n[%s] : %s =========> (%d): stMB_Command.eRoleID   =%d\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.eRoleID);//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.eMsgType  =%d\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.eMsgType);//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.u8Ctrl    =0x%02X\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.u8Ctrl);//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.u8MsgClass=%d\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.u8MsgClass);//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.u8Index   =%d\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.u8Index);//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.u8ParameterCount=%d\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.u8ParameterCount );//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.u8Parameters[0]=0x%02X\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.u8Parameters[0] );//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.u8Parameters[1]=0x%02X\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.u8Parameters[1] );//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.u8Parameters[2]=0x%02X\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.u8Parameters[2] );//alex_tung###
        printf("\n[%s] : %s =========> (%d): stMB_Command.u8Parameters[3]=0x%02X\n",__FILE__,__FUNCTION__,__LINE__,stMB_Command.u8Parameters[3] );//alex_tung###
    }
    return (enMbxResult==E_MBX_SUCCESS)? TRUE : FALSE;

}

#if defined(AMZN_FTVE_FRC_SIGNING_ENABLED)
int amzn_verify_frc(void *frc_buf)
{
    int ret = 0; // success
    unsigned int frc_size = FRC_START_SIZE;

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
    const unsigned char* sig_buf = (const unsigned char*)frc_buf+frc_size;

    ret = amzn_verify_code_internal((const unsigned char*)frc_buf, frc_size,
        sig_buf, FRC_SIG_LEN, key, key_len);

    UBOOT_DEBUG("FRC amzn_verify_code_internal ret = %d\n", ret);

done:
    return ret;
}
#endif

//#################################################################
int do_frc_bringup(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    char cmd[128];
    MS_U8 u8MIUProtectkernel_ID[16] ={0};
    char  filePath[256];
    MS_U32 u32fileSize;
    U32 u32AddrVA=0;
    MS_BOOL loadFRC = false;

    MS_PHYADDR FRCR2BootAddr;
    MS_PHYADDR FRC_Addr=0;
    U32 u32FRCR2BootLen;

    if (get_map_addr_and_size_from_env(E_FRC_R2, NO_DEFAULT_MMAP_VALUE, &FRCR2BootAddr, NO_DEFAULT_MMAP_VALUE, &u32FRCR2BootLen) != 0)
    {
        UBOOT_ERROR("get E_MMAP_ID_FRC_R2 mmap fail\n");
        return -1;
    }
    UBOOT_DEBUG("E_MMAP_ID_FRC_R2 = 0x%lx\n",(unsigned long) FRCR2BootAddr);
    UBOOT_DEBUG("(U32)(PA2NVA(u32PanelConfigsAddr)) = 0x%x\n", (U32)(PA2NVA(FRCR2BootAddr)));

    u32AddrVA=(U32)(PA2NVA(FRCR2BootAddr));
    if(vfs_mount(CONFIG)!= 0)
    {
        UBOOT_ERROR("FRC:vfs_mount fail\n");
    }
    else
    {
        char  *FrcBinPath = NULL;
        if (((FrcBinPath = getenv("FrcBinPath")) != NULL) && (strcmp(FrcBinPath,"false") != 0))
        {
            snprintf(filePath, sizeof(filePath),"%s/%s", CONFIG_PATH, FrcBinPath);
            u32fileSize = vfs_getsize(filePath);

            if(vfs_read((void*)u32AddrVA,filePath,0,u32fileSize) != 0)
            {
                UBOOT_ERROR("vfs_read FRC Bin fail ...>>>\n");
            }
            else
            {
                UBOOT_DEBUG("load FRC bin: %s, fileSize = 0x%x\n", filePath, u32fileSize);
                loadFRC = true;
                udelay(50);
            }
        }
    }

    if(loadFRC == false)
    {
#if defined(AMZN_FTVE_FRC_SIGNING_ENABLED)
        if (is_lockdown()) {
            memset(cmd, 0, sizeof(cmd));
            FRC_Addr = malloc(FRC_SIG_START_SIZE * sizeof(unsigned char));
            snprintf(cmd, sizeof(cmd)-1, "mmc read.p 0x%08lX  frc 0x100000", (unsigned long)(FRC_Addr));

            if (run_command(cmd, 0) < 0) {
                UBOOT_ERROR("Failed to load FRC\n");
                free(FRC_Addr);
            }

            UBOOT_DEBUG("Verify FRC\n");
            if (amzn_verify_frc((void*) FRC_Addr)) {
                free(FRC_Addr);
                while(1) {
                    UBOOT_ERROR("Failed to verify FRC\n");
                }
            }
            free(FRC_Addr);
            UBOOT_DEBUG("Verify FRC PASS\n");
        }
#endif

        memset(cmd, 0, sizeof(cmd));
#if (CONFIG_MBOOT_IN_NAND_FLASH)
    snprintf(cmd, sizeof(cmd)-1, "nand read.e 0x%08X frc 0x100000", (unsigned int)(PA2NVA(FRCR2BootAddr)));
#elif (CONFIG_MBOOT_IN_MMC_FLASH)
#if defined(AMZN_FTVE_FRC_SIGNING_ENABLED)
    snprintf(cmd, sizeof(cmd)-1, "mmc read.p 0x%08X frc 0xFFF00", (unsigned int)(PA2NVA(FRCR2BootAddr)));
#else
    snprintf(cmd, sizeof(cmd)-1, "mmc read.p 0x%08X frc 0x100000", (unsigned int)(PA2NVA(FRCR2BootAddr)));
#endif
#elif (CONFIG_MBOOT_IN_UFS_FLASH)
    snprintf(cmd, sizeof(cmd)-1, "ufs read.p 0x%08X frc 0x100000", (unsigned int)(PA2NVA(u32FRCR2BootAddr)));
#endif

        if (-1 != run_command(cmd, 0))
        {
            UBOOT_DEBUG("\n[%s: %d] read the frc partion data@%x:\n",__func__,__LINE__,(unsigned long)FRCR2BootAddr);
        }
        else
        {
            UBOOT_ERROR("%s: '%s' fails, at %d\n", __func__, cmd, __LINE__);
            return -1;
        }
    }

    FRCR2_MBX_Init();
    MDrv_FRCR2_Init_Front();
    MDrv_FRCR2_Init_End(FRCR2BootAddr);

    UBOOT_DEBUG("E_MMAP_ID_FRC_R2 = 0x%lx 0x%x\n", (unsigned long)FRCR2BootAddr, (unsigned int)u32FRCR2BootLen);
    u8MIUProtectkernel_ID[0] = MIU_CLIENT_FRC_R2; //CPU. Ref: drvMIU.h

    return 0;
}

//#################################################################
int do_frc_send(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_U8 u8CmdIdx;
    MS_U32 u32Data;
    MS_BOOL bResult;
    MS_U8 k=0;

    u8CmdIdx=(U8)atoi(argv[1]);
    u32Data = (U32)atoi(argv[2]);

    printf("\n[HK] do_frc_send : u8CmdIdx=%d, u32Data=0x%X\n",u8CmdIdx,(unsigned int)u32Data);
    #if 0
    while(k--)
    {
        bResult = FRCR2_MBX_SendMsg(u8CmdIdx++,u32Data+k);
        mdelay(1000);
    }
    #else
    k=100;
    while(k--)
    {
        mdelay(100);
        bResult = FRCR2_MBX_SendMsg(u8CmdIdx++,u32Data+k);
        if(bResult)
        {
            mdelay(100);
            FRCR2_MBX_RecvMsg();

        }
        mdelay(1000);
    }
    #endif
    Write2Byte(0x101EA6,0xFFF4);//alex_tung###

    return 0;
}

int do_frc_recv(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_U8 u8CmdIdx;
    MS_U32 u32Data;
    MS_BOOL bResult;
    MS_U8 k=5;

    u8CmdIdx=(U8)atoi(argv[1]);
    u32Data = (U32)atoi(argv[2]);

    while(k)
    {
        bResult = FRCR2_MBX_RecvMsg();
        if(bResult)
        {
            k--;
        }
        mdelay(100);
    }
    return 0;

}

#endif

void str_rep_end (char*source, char *find,  char *rep)
{
    if (strstr(source, find))
    {
        int length = strlen(source) - strlen(strstr(source, find));
        if (length > 0 && (length + strlen(rep)) < BUFFER_SIZE)
        {
            char tmp[length+1];
            snprintf(tmp, length, "%s", source);
            tmp[length]='\0';
            memset(source, 0 , BUFFER_SIZE);
            snprintf(source, BUFFER_SIZE-1, "%s%s", tmp, rep);
            source[strlen(source)] = '\0';
        }
    }
}

int do_pnl_switch_check (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");

#if defined(CONFIG_ENABLE_SWITCH_PNL)
#if defined(UFBL_FEATURE_IDME)
    if(atoi(getenv("bootmode")) == IDME_BOOTMODE_DIAG)
    {

#define KEYWORD_INI ".ini"
#define KEYWORD_VBYONE "_vb1"
#define KEYWORD_VBYONEINI KEYWORD_VBYONE KEYWORD_INI
#define USB_CHECK_FILE_HIS "VbyOne.his"
#define USB_CHECK_FILE_CVTE "FAC_BOOT_CVTE_LVDS.cvt"
#define FILENAME_IGNORE "Customer_CVTE.ini"
#define ABC_CONFIGNAME "haileyplus_m"

        int check_result = 0;
        unsigned int boot_reason;
        char cmd[BUFFER_SIZE] = "\0";

        char s_ConfigName[BUFFER_SIZE] = "\0";
        char s_ModelName[BUFFER_SIZE] = "\0";

        idme_get_var_external("model_name", s_ModelName, (BUFFER_SIZE - 1));
        idme_get_var_external("config_name", s_ConfigName, (BUFFER_SIZE - 1));
        boot_reason = Read2Byte(BOOT_REASON_PM_ADDR_OFFSET);

        UBOOT_DEBUG("model_name = %s\n", s_ModelName);
        UBOOT_DEBUG("config_name = %s\n", s_ConfigName);
        UBOOT_DEBUG("boot_reason = %d\n", boot_reason);

        if (strstr(s_ConfigName, ABC_CONFIGNAME) != NULL && boot_reason == PM_SPARE_COLD_BOOT_POWER_SUPPLY)
        {
            UBOOT_DEBUG("ABC diag mode and cool boot, try usb upgrade\n");
            run_command("custar",0);//usb upgrade
        }

        int result = run_command("usb start 0", 0);

        memset(cmd, 0 , BUFFER_SIZE);
        snprintf(cmd, sizeof(cmd)-1, "fatfilesize usb 0:1 %s", USB_CHECK_FILE_HIS);
        int res_his = run_command(cmd, 0);
        memset(cmd, 0 , BUFFER_SIZE);
        snprintf(cmd, sizeof(cmd)-1, "fatfilesize usb 0:1 %s", USB_CHECK_FILE_CVTE);
        int res_cvte = run_command(cmd, 0);
        run_command("usb stop", 0);
        check_result = res_his & res_cvte;

        MS_BOOL bNeedChangeModelName = FALSE;
        MS_BOOL bNeedChangeConfigName = FALSE;

        if (!strstr(s_ModelName, FILENAME_IGNORE))
        {
            if (!check_result)
            {
                if (!strstr(s_ModelName, KEYWORD_VBYONE))
                {
                    bNeedChangeModelName = TRUE;
                    str_rep_end(s_ModelName, KEYWORD_INI, KEYWORD_VBYONEINI);
                }

                if (!strstr(s_ConfigName, KEYWORD_VBYONE))
                {
                    bNeedChangeConfigName = TRUE;
                    sprintf(s_ConfigName, "%s%s", s_ConfigName, KEYWORD_VBYONE);
                }
            }
            else
            {
                if (strstr(s_ModelName, KEYWORD_VBYONE))
                {
                    bNeedChangeModelName = TRUE;
                    str_rep_end(s_ModelName, KEYWORD_VBYONEINI, KEYWORD_INI);
                }

                if (strstr(s_ConfigName, KEYWORD_VBYONE))
                {
                    bNeedChangeConfigName = TRUE;
                    str_rep_end(s_ConfigName, KEYWORD_VBYONE, "");
                }
            }

            UBOOT_DEBUG("bNeedChangeModelName=%d, bNeedChangeConfigName=%d\n", bNeedChangeModelName, bNeedChangeConfigName);

            if (bNeedChangeModelName)
            {
                UBOOT_DEBUG("model_name change to %s\n", s_ModelName);

                memset(cmd, 0 , BUFFER_SIZE);
                snprintf(cmd, sizeof(cmd)-1, "idme model_name %s", s_ModelName);
                run_command(cmd, 0);
            }

            if (bNeedChangeConfigName)
            {
                UBOOT_DEBUG("config_name change to %s\n", s_ConfigName);

                memset(cmd, 0 , BUFFER_SIZE);
                snprintf(cmd, sizeof(cmd)-1, "idme config_name %s", s_ConfigName);
                run_command(cmd, 0);
            }

            if (bNeedChangeModelName || bNeedChangeConfigName)
            {
                run_command("initenv", 0);
                run_command("reset",0);
            }
        }
        else
        {
            UBOOT_DEBUG("Do not switch panel\n");
        }
    }
#endif
#endif
    UBOOT_TRACE("OK\n");
    return 0;
}

/* do_config_gpio is used to re-config GPIO at run-time for co-image case */
int do_config_gpio (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN\n");
#define DSP_MT8570          "mt8570"
#define DSP_SUECREEK        "suecreek"
#define PADS_MIC_MD         0x322920
#define PADS_I2S_IN_MD      0x322924
#define PADS_MIIC_MODE2     0x322951
#define PAD_GPIO1_PM_OEN    0x000F02
#define PAD_GPIO8_PM_OEN    0x000F10
#define GPIO_IN             0x1
#define GPIO_OUT_LOW        0x2
#define GPIO_OUT_HIGH       0x3
#define BITMASK3_0 (BIT3|BIT2|BIT1|BIT0)
#define BITMASK2_0 (BIT2|BIT1|BIT0)
#define BITMASK1_0 (BIT1|BIT0)

    char s_DspName[BUFFER_SIZE] = "\0";
    char s_ConfigName[BUFFER_SIZE] = "\0";
    idme_get_oem_data_field("dsp=", s_DspName, (BUFFER_SIZE - 1));
    idme_get_var_external("config_name", s_ConfigName, (BUFFER_SIZE - 1));
    UBOOT_DEBUG("dsp name = %s, ConfigName = %s\n", s_DspName, s_ConfigName);

    /*
       GPIO8_PM DEFAULT : INPUT mode
       ABC(brandenburg) + suecreek : GPIO OUTPUT LOW
       ABC(brandenburg) + mt8570 : N/A
       ABC + suecreek : N/A
       ABC + mt8570 : GPIO INPUT
       */

    /* config name of ABC is brandenburg_ABC_xvt */
    if (strstr(s_ConfigName, "brandenburg") && !strstr(s_ConfigName, "ABC"))
    {
        UBOOT_DEBUG("re-config GPIO8_PM to output mode and lo for BBURG\n");
        MDrv_WriteByte(PAD_GPIO8_PM_OEN, (MDrv_ReadByte(PAD_GPIO8_PM_OEN) & ~BITMASK1_0) | GPIO_OUT_LOW);
    }

    if (strstr(s_DspName, DSP_MT8570))
    {
        UBOOT_DEBUG("re-config PADS_MIIC_MODE2/PADS_I2S_IN_MD/PADS_MIC_MD for mt8570\n");
        /* PADS_MIC_MD and PADS_I2S_IN_MD would conflict between hailey+ and hailey, Reconfig to CONFIG_PADMUX_MODE5*/
        MDrv_WriteByte(PADS_MIC_MD, MDrv_ReadByte(PADS_MIC_MD) & ~BITMASK3_0 | 0x05); //0x3229_20[3:0] = 0x5
        MDrv_WriteByte(PADS_I2S_IN_MD, MDrv_ReadByte(PADS_I2S_IN_MD) & ~BITMASK2_0 | 0x05); //0x3229_24[2:0] = 0x5
        MDrv_WriteByte(PADS_MIIC_MODE2, MDrv_ReadByte(PADS_MIIC_MODE2) & ~BITMASK1_0); //0x3229_24[2:0] = 0x5

        UBOOT_DEBUG("re-config GPIO1_PM to input mode for mt8570\n");
        MDrv_WriteByte(PAD_GPIO1_PM_OEN, (MDrv_ReadByte(PAD_GPIO1_PM_OEN) & ~BITMASK1_0) | GPIO_IN);
    }

    UBOOT_TRACE("OK\n");
    return 0;
}
