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
/// file    mdrv_pwm.c
/// @brief  TEMP Driver Interface for Export
/// @author MStar Semiconductor Inc.
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>

//drver header files
#include "mst_devid.h"
//#include "mdrv_mstypes.h"
#include "MsTypes.h"
#include "mdrv_pwm.h"
#include "mhal_pwm.h"
#include "mreg_pwm.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int                         s32Major;
    int                         s32Minor;
    struct cdev                 stCDev;
    struct file_operations      fops;
} PWM_DEV;


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Forward declaration
//--------------------------------------------------------------------------------------------------
static int                  _mod_pwm_open (struct inode *inode, struct file *filp);
static int                  _mod_pwm_release(struct inode *inode, struct file *filp);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long                 _mod_pwm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#else
static int                  _mod_pwm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#endif

PWM_DEV devPWM =
{
    .s32Major   =           MDRV_MAJOR_PWM,
    .s32Minor   =           MDRV_MINOR_PWM,
    .stCDev =
    {
        .kobj   =           {.name = MDRV_NAME_PWM, },
        .owner  =           THIS_MODULE,
    },
    .fops =
    {
        .open   =           _mod_pwm_open,
        .release=           _mod_pwm_release,
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,20)
        .unlocked_ioctl  =  _mod_pwm_ioctl,
        #else
        .ioctl  =           _mod_pwm_ioctl,
        #endif
    },
};


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
static int _mod_pwm_open(struct inode *inode, struct file *filp)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);
    filp->private_data = (void*)(iminor(inode) - devPWM.s32Minor);

    return FALSE;
}

static int _mod_pwm_release(struct inode *inode, struct file *filp)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
static long _mod_pwm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int _mod_pwm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    int err= 0;

    PWM_PRINT("%s is invoked\n", __FUNCTION__);
    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((PWM_IOC_MAGIC != _IOC_TYPE(cmd)) || (_IOC_NR(cmd) > PWM_IOC_MAXNR))
    {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (err)
    {
        return -EFAULT;
    }

    #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
    switch(cmd)
    {
        default:
            PWM_PRINT("ioctl: unknown command\n");
            return -ENOTTY;
    }
    #else
    switch(cmd)
    {
        default:
            PWM_PRINT("ioctl: unknown command\n");
            return -ENOTTY;
    }
    #endif

    return 0;
}


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
MSYSTEM_STATIC int __init mod_pwm_init(void)
{
    int         s32Ret;
    dev_t       dev;


    PWM_PRINT("%s is invoked\n", __FUNCTION__);

    if (devPWM.s32Major)
    {
        dev = MKDEV(devPWM.s32Major, devPWM.s32Minor);
        s32Ret = register_chrdev_region(dev, PWM_Num, MDRV_NAME_PWM);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, devPWM.s32Minor, PWM_Num, MDRV_NAME_PWM);
        devPWM.s32Major = MAJOR(dev);
    }

    if (0 > s32Ret)
    {
        PWM_PRINT("Unable to get major %d\n", devPWM.s32Major);
        return s32Ret;
    }

    cdev_init(&devPWM.stCDev, &devPWM.fops);
    if (0 != (s32Ret = cdev_add(&devPWM.stCDev, dev, PWM_Num)))
    {
        PWM_PRINT("Unable add a character device\n");
        unregister_chrdev_region(dev, PWM_Num);
        return s32Ret;
    }
    MDrv_PWM_Init(E_PWM_DBGLV_ERR_ONLY);

    return 0;
}

MSYSTEM_STATIC void __exit mod_pwm_exit(void)
{
    PWM_PRINT("%s is invoked\n", __FUNCTION__);

    cdev_del(&devPWM.stCDev);
    unregister_chrdev_region(MKDEV(devPWM.s32Major, devPWM.s32Minor), PWM_Num);
}

module_init(mod_pwm_init);
module_exit(mod_pwm_exit);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("pwm driver");
MODULE_LICENSE("GPL");

