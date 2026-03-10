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

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    mdrv_r2.c
/// @brief  Coprocessor related functions
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
/* From linux. */
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>

/* From mstar. */
#include "mdrv_r2.h"
#include "mhal_r2.h"
#include "mdrv_system.h"
#include "mdrv_types.h"

//--------------------------------------------------------------------------------------------------
//  Forward declaration
//--------------------------------------------------------------------------------------------------
#define MDRV_R2_CODE_SZ     (0x100000)
#define MDRV_R2_BUFF_SZ     (128)

#if defined(CONFIG_ARM)
#define MDRV_R2_MAP_PROT    pgprot_noncached(L_PTE_MT_BUFFERABLE | L_PTE_XN)
#elif defined(CONFIG_ARM64)
#define MDRV_R2_MAP_PROT    __pgprot(PROT_NORMAL_NC)
#else
#error "MDRV_R2_MAP_PROT undeclared"
#endif

#if defined(CONFIG_MSTAR_FRC_R2)
#define MDRV_R2_FRC_PART    "frc"
#endif

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef struct _MDRV_R2_INFO
{
    U8                      u8Enable;
    U32                     u32Address;
    U8                      pu8Path[MDRV_R2_BUFF_SZ];
} MDRV_R2_INFO;

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
#if defined(CONFIG_MSTAR_FRC_R2)
static MDRV_R2_INFO         _stFRC_R2 = {0};
#endif

//-------------------------------------------------------------------------------------------------
//  Golbal variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local function
//-------------------------------------------------------------------------------------------------
static unsigned long _MDrv_R2_Ba2Pa(unsigned long ba)
{
    unsigned long pa = 0;

    if ((ba > ARM_MIU0_BUS_BASE) && (ba <= ARM_MIU1_BUS_BASE))
        pa = ba - ARM_MIU0_BUS_BASE + ARM_MIU0_BASE_ADDR;
    else if ((ba > ARM_MIU1_BUS_BASE) && (ba <= ARM_MIU2_BUS_BASE))
        pa = ba - ARM_MIU1_BUS_BASE + ARM_MIU1_BASE_ADDR;
    else
        MHAL_R2_DEBUG("ba=0x%tX, pa=0x%tX.\n", (size_t)ba, (size_t)pa);

    return pa;
}

static int _MDrv_R2_Get_Path(char *part, char *path)
{
    int ret = 0;
    struct file *fp = NULL;
    mm_segment_t fs;
    loff_t pos = 0;
    int blk = 1;
    char buff[MDRV_R2_BUFF_SZ] = {0};
    char key[MDRV_R2_BUFF_SZ] = {0};

    /* Find /dev/block/platform/mstar_mci.0/by-name/? exist. */
    snprintf(buff, MDRV_R2_BUFF_SZ, "/dev/block/platform/mstar_mci.0/by-name/%s", part);
    fp = filp_open(buff, O_RDONLY, 0);
    if (!IS_ERR(fp))
    {
        strncpy(path, buff, MDRV_R2_BUFF_SZ);
        filp_close(fp, NULL);
        return 0;
    }

    /* Store fs. */
    fs = get_fs();
    set_fs(KERNEL_DS);

    /* Find /dev/mmcblk0p#. */
    memset(key, '\0', MDRV_R2_BUFF_SZ);
    snprintf(key, MDRV_R2_BUFF_SZ, "PARTNAME=%s", part);
    do
    {
        memset(buff, '\0', MDRV_R2_BUFF_SZ);
        snprintf(buff, MDRV_R2_BUFF_SZ, "/sys/block/mmcblk0/mmcblk0p%d/uevent", blk);
        fp = filp_open(buff, O_RDONLY, 0);
        if (IS_ERR(fp))
        {
            MHAL_R2_DEBUG("%s R2 path not found.\n", part);
            ret = -ENODATA;
            goto _MDrv_R2_Get_Path_End;
        }
        memset(buff, '\0', MDRV_R2_BUFF_SZ);
        vfs_read(fp, buff, MDRV_R2_BUFF_SZ, &pos);
        filp_close(fp, NULL);

        /* Check found. */
        if (strnstr(buff, key, MDRV_R2_BUFF_SZ))
        {
            memset(buff, '\0', MDRV_R2_BUFF_SZ);
            snprintf(buff, MDRV_R2_BUFF_SZ, "/dev/mmcblk0p%d", blk);
            fp = filp_open(buff, O_RDONLY, 0);
            if (!IS_ERR(fp))
            {
                snprintf(path, MDRV_R2_BUFF_SZ, "/dev/mmcblk0p%d", blk);
                filp_close(fp, NULL);
            }
            else
            {
                snprintf(path, MDRV_R2_BUFF_SZ, "/dev/mmcblk%d", blk);
            }
            break;
        }
        else
        {
            pos = 0;
            blk++;
        }
    } while (1);

_MDrv_R2_Get_Path_End:
    /* Restore fs. */
    set_fs(fs);

    return ret;
}

static int _MDrv_R2_Copy_Bin(unsigned long dst_addr, char *src_path)
{
    int ret = 0;
    struct file *fp = NULL;
    mm_segment_t fs;
    loff_t pos = 0;
    phys_addr_t *va = 0;

    /* Check data init. */
    if ((strlen(src_path) == 0) || (dst_addr == 0))
    {
        MHAL_R2_DEBUG("Invalid argument: src=%s, dst=0x%lX.\n", src_path, dst_addr);
        return -EINVAL;
    }

    /*
     * Prepare flow:
     * 1. open fd.
     * 2. mapping address.
     * 3. store fs.
     */
    fp = filp_open(src_path, O_RDONLY, 0);
    if (IS_ERR(fp))
    {
        MHAL_R2_DEBUG("filp_open() fail.\n");
        return -EBADF;
    }
    if (pfn_valid(__phys_to_pfn(dst_addr)))
    {
        va = dma_common_contiguous_remap(pfn_to_page(__phys_to_pfn(dst_addr)),
                                        MDRV_R2_CODE_SZ,
                                        VM_USERMAP,
                                        MDRV_R2_MAP_PROT,
                                        __func__);
    }
    else
    {
        va = ioremap_nocache(dst_addr, MDRV_R2_CODE_SZ);
    }
    if (va == NULL)
    {
        MHAL_R2_DEBUG("Mapping fail: ba=0x%tX sz=0x%tX.\n",
                    (size_t)dst_addr, (size_t)MDRV_R2_CODE_SZ);
        ret = -EIO;
        goto _MDrv_R2_Copy_Bin_End;
    }
    fs = get_fs();
    set_fs(KERNEL_DS);

    /* Copy form destination partition to DRAM address. */
    vfs_read(fp, (char *)va, MDRV_R2_CODE_SZ, &pos);

    /*
     * Complete to do:
     * 1. restore fs.
     * 2. un-mapping address.
     * 3. close fd.
     */
    set_fs(fs);
    if (pfn_valid(__phys_to_pfn(dst_addr)))
    {
        dma_common_free_remap(va, MDRV_R2_CODE_SZ, VM_USERMAP);
    }
    else
    {
        iounmap(va);
    }
_MDrv_R2_Copy_Bin_End:
    filp_close(fp, NULL);

    return ret;
}

//-------------------------------------------------------------------------------------------------
//  Golbal function
//-------------------------------------------------------------------------------------------------
#if defined(CONFIG_MSTAR_FRC_R2)
S32 MDrv_FRC_R2_Suspend(MDRV_R2_STATE eState)
{
    S32 s32Ret = 0;

    switch (eState)
    {
        case E_R2_STATE_STORE_INFO:
            if (_stFRC_R2.u8Enable == 1)
            {
                /* Check path exist. */
                if (strlen(_stFRC_R2.pu8Path) == 0)
                {
                    /* Get FRC-R2 bin path. */
                    s32Ret = _MDrv_R2_Get_Path(MDRV_R2_FRC_PART, _stFRC_R2.pu8Path);
                }
            }
            break;

        default:
            MHAL_R2_DEBUG("Invalid argument.\n");
            s32Ret = -EINVAL;
            break;
    }

    return s32Ret;
}

S32 MDrv_FRC_R2_Resume(MDRV_R2_STATE eState)
{
    S32 s32Ret = 0;

    switch (eState)
    {
        case E_R2_STATE_POWER_ON_DC:
            if (_stFRC_R2.u8Enable == 1)
            {
                /* Check data. */
                if ((strlen(_stFRC_R2.pu8Path) == 0) || (_stFRC_R2.u32Address == 0))
                {
                    MHAL_R2_DEBUG("Data error: path=%s addr=0x%X.\n",
                                _stFRC_R2.pu8Path, _stFRC_R2.u32Address);
                    s32Ret = -EFAULT;
                    break;
                }
                /*
                 * 1. Disable FRC-R2.
                 * 2. Copy FRC-R2 bin.
                 * 3. Enable FRC-R2.
                 */
                MHal_FRC_R2_Disable();
                s32Ret = _MDrv_R2_Copy_Bin(_stFRC_R2.u32Address, _stFRC_R2.pu8Path);
                MHal_FRC_R2_Enable(_MDrv_R2_Ba2Pa(_stFRC_R2.u32Address));
            }
            break;

        default:
            MHAL_R2_DEBUG("Invalid argument.\n");
            s32Ret = -EINVAL;
            break;
    }

    return s32Ret;
}
#endif

//-------------------------------------------------------------------------------------------------
//  Early
//-------------------------------------------------------------------------------------------------
#if defined(CONFIG_MSTAR_FRC_R2)
static int __init _MDrv_R2_Set_FRC_Info(char *str)
{
    if (str != NULL)
    {
        sscanf(str, "%hhu, %x", &(_stFRC_R2.u8Enable), &(_stFRC_R2.u32Address));
    }
    return 0;
}
early_param("FRC_R2_INFO", _MDrv_R2_Set_FRC_Info);
#endif
