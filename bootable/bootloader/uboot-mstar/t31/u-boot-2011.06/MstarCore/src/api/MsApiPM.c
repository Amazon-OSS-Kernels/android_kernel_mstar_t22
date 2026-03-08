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
#include <MsTypes.h>
#include <msAPI_Power.h>
#include <drvBDMA.h>
#include <drvPM.h>
#include <drvWDT.h>
#include <MsApiMiu.h>
#include <MsDebug.h>
#include <MsMmap.h>
#include <MsSystem.h>
#include <MsSysUtility.h>
#include <MsEnvironment.h>
#include <CusConfig.h>
#include <MsVfs.h>

#if defined(CONFIG_MTK_LOADER)
#include "eeprom_if.h"
#endif

#if defined(AMZN_FTVE_RTPM_SIGNING_ENABLED)
#include "amzn_secure_boot.h"
#define RTPM_SIG_LEN (256)
#define RTPM_SIG_START_SIZE 0x10100
#endif

extern int snprintf(char *str, size_t size, const char *fmt, ...);

#if defined(AMZN_FTVE_RTPM_SIGNING_ENABLED)
int amzn_verify_rtpm(void *rtpm_buf);
void env_bin_verify(unsigned char status, unsigned char bit_op);
#endif

#if (CONFIG_ENABLE_RTPM)
#define RUNTIME_PM_START_SIZE 0x10000
#define u32Dummy 0x0EA2
#if(CONFIG_MSTAR_URSA6_VB1)
#define URSA 0x2
#elif(CONFIG_MSTAR_URSA9_VB1)
#define URSA 0x4
#endif
int do_run_time_pm( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    UBOOT_TRACE("IN");
    MS_PHYADDR Addr=0, Size=0;
    MS_PHYADDR RT_PM_Addr=0;
    char cmd[CMD_BUF] ="\0";
    memset(cmd,0,sizeof(cmd));

#if(CONFIG_ENABLE_URSA_RTPM)
    Write2Byte(u32Dummy, URSA);
#endif
    if(get_map_addr_and_size_from_env(E_RTPM_MEM, NO_DEFAULT_MMAP_VALUE, &Addr, NO_DEFAULT_MMAP_VALUE, &Size) != 0)
    {
        UBOOT_ERROR("get %s mmap fail, try %s\n", E_MMAP_ID_RTPM_MEM_ADR, E_MMAP_ID_PM51_USAGE_MEM_ADR);
        if(get_map_addr_and_size_from_env(E_PM51_MEM, NO_DEFAULT_MMAP_VALUE, &Addr, NO_DEFAULT_MMAP_VALUE, &Size) != 0)
        {
            UBOOT_ERROR("get %s mmap fail\n", E_MMAP_ID_PM51_USAGE_MEM_ADR);
            return -1;
        }
    }

    UBOOT_DEBUG("=================================================\n");
    UBOOT_DEBUG("    Addr=0x%llx\n",(unsigned long long)Addr);
    UBOOT_DEBUG("    RUNTIME_PM_START_SIZE=0x%x\n",RUNTIME_PM_START_SIZE);
    UBOOT_DEBUG("=================================================\n");

#if defined(AMZN_FTVE_RTPM_SIGNING_ENABLED)
        RT_PM_Addr = malloc(RTPM_SIG_START_SIZE * sizeof(unsigned char));
        snprintf(cmd, sizeof(cmd)-1, "mmc read.p 0x%08lX RTPM 0x10100", (unsigned long)RT_PM_Addr);

        if (run_command(cmd, 0) < 0) {
            UBOOT_ERROR("Failed to load RTPM\n");
            free(RT_PM_Addr);
            return -1;
        }

        UBOOT_DEBUG("Verify RTPM\n");
        if (amzn_verify_rtpm((void*) RT_PM_Addr)) {
            free(RT_PM_Addr);
            while(1) {
                UBOOT_ERROR("Failed to verify RTPM\n");
            }
        }
        free(RT_PM_Addr);
        UBOOT_DEBUG("Verify RTPM PASS\n");
#endif

#if defined(CONFIG_SECURITY_BOOT) && (ENABLE_STB_BOOT)
#if (CONFIG_MSTAR_RT_PM_IN_SPI)
    #error Security boot not support RT_PM_IN_SPI
#elif (CONFIG_MSTAR_RT_PM_IN_NAND)
    do_Authenticate_RawData("RTPM",(U32)(PA2NVA(Addr)));
#elif (CONFIG_MSTAR_RT_PM_IN_EMMC)
    #error Security boot support RT_PM_IN_EMMC
#else
    #error
#endif

#else
#if (CONFIG_MSTAR_RT_PM_IN_SPI)
    MDrv_BDMA_CopyHnd(0x20000,Addr,RUNTIME_PM_START_SIZE,E_BDMA_FLASH2SDRAM,0);
#elif (CONFIG_MSTAR_RT_PM_IN_NAND)
    snprintf(cmd, sizeof(cmd)-1, "nand read.e 0x%08lX RTPM 10000", (unsigned long)(PA2NVA(Addr)));
#elif (CONFIG_MSTAR_RT_PM_IN_EMMC)
    snprintf(cmd, sizeof(cmd)-1, "mmc read.p 0x%08lX RTPM 0x10000", (unsigned long)(PA2NVA(Addr)));
#else
    #error
#endif
    if(0 != run_command(cmd, 0))
    {
        UBOOT_ERROR("%s: '%s' fails, at %d\n", __func__, cmd, __LINE__);
        return -1;
    }
#endif

    MsApi_RunTimePmProtect(Addr, (Addr+RUNTIME_PM_START_SIZE));
    MDrv_PM_SetDRAMOffsetForMCU(Addr);                     // Wake up DDR_PM

	UBOOT_TRACE("OK\n");
    return 0;
}
#endif

#if defined(AMZN_FTVE_RTPM_SIGNING_ENABLED)
int amzn_verify_rtpm(void *rtpm_buf)
{
    int ret = 0; // success
    unsigned int rtpm_size = RUNTIME_PM_START_SIZE;

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
    const unsigned char* sig_buf = (const unsigned char*)rtpm_buf+rtpm_size;

    ret = amzn_verify_code_internal((const unsigned char*)rtpm_buf, rtpm_size,
        sig_buf, RTPM_SIG_LEN, key, key_len);
    UBOOT_DEBUG("RTPM amzn_verify_code_internal ret = %d\n", ret);

done:
    return ret;
}
#endif

MS_BOOL pm_check_back_ground_active(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    MS_BOOL bActive = FALSE;
    if(E_PM_FAIL  == MDrv_PM_IsActiveStandbyMode(&bActive))
    {
        printf("\x1b[37;41m ===== [%s:%s:%d] MDrv_PM_IsBackGroundModeActive FAIL ===== \x1b[0m\n",__FILE__,__FUNCTION__,__LINE__);
    }
    if(bActive)
    {
        UBOOT_DEBUG(" Active Standby Mode TRUE =====\n");
    }
    else
    {
        UBOOT_DEBUG(" Active Standby Mode False =====\n");
    }

#if defined(CONFIG_MTK_LOADER)
    if(EEPRON_IsQuiteHotBootMode())
    {
        bActive = TRUE;
    }
#endif
    return bActive;
}

MS_BOOL get_poweroff_flag(void)
{
    ///ac power off also use this flag -> dc_poweroff
    BOOLEAN ret = FALSE;
    UBOOT_TRACE("IN\n");

#if (ENABLE_ENV_IN_NAND == 1)
    if(vfs_mount(CUSTOMER)!=-1)
    {
        char PathBuf[64] = "\0";
        snprintf(PathBuf, sizeof(PathBuf), "%s/dc_poweroff", CUSTOMER_PATH);
        if(vfs_getsize(PathBuf) > 0)
        {
            UBOOT_DEBUG("dc_poweroff is Ture\n");;
            ret = TRUE;
        }
        else
        {
            UBOOT_DEBUG("dc_poweroff is False\n");;
            ret = FALSE;
        }
    }
#else
    if(getenv("dc_poweroff") == NULL)
    {
        setenv("dc_poweroff", "0");
        saveenv();
    }

    if(strcmp(getenv("dc_poweroff"), "1") == 0)
    {
        ret = TRUE;
    }
#endif
    UBOOT_TRACE("OK\n");
    return ret;
}
