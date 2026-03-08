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
//=============================================================================
// Include Files
//=============================================================================
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/pfn.h>
#include <linux/delay.h>
#include <linux/compat.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include "mdrv_graphic_virtualfb.h"
#include "mdrv_graphic_adapter.h"

#define LOG_TAG "VirtualFB " 

int virtualfb_set_par(struct fb_info *pinfo)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,pinfo->node);
    return 0;
}


int virtualfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,info->node);
    return 0;
}

int virtualfb_setcmap(struct fb_cmap * cmap,struct fb_info * info)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,info->node);
    return 0;
}

int virtualfb_blank(int blank, struct fb_info *info)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,info->node);
    return 0;
}

int virtualfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *pinfo)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,pinfo->node);
    return 0;
}

//It's not necessary to support pseudo_palette
int virtualfb_setcolreg(unsigned regno, unsigned red, unsigned green,
                              unsigned blue, unsigned transp, struct fb_info *info)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,info->node);
    return 0;
}

void virtualfb_fillrect(struct fb_info *p, const struct fb_fillrect *rect)
{
#ifdef CONFIG_FB_VIRTUAL
#endif
}

void virtualfb_copyarea(struct fb_info *p, const struct fb_copyarea *area)
{
#ifdef CONFIG_FB_VIRTUAL
#endif
}

void virtualfb_imageblit(struct fb_info *p, const struct fb_image *image)
{
#ifdef CONFIG_FB_VIRTUAL
#endif
}

void virtualfb_destroy(struct fb_info *info)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,info->node);
}

int virtualfb_open(struct fb_info *info, int user)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,info->node);
    return 0;
}

int virtualfb_release(struct fb_info *info, int user)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,info->node);
    return 0;
}

int virtualfb_mmap(struct fb_info *pinfo, struct vm_area_struct *vma)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,pinfo->node);
    return 0;
}

int virtualfb_ioctl(struct fb_info *pinfo, unsigned int u32Cmd, unsigned long u32Arg)
{
    pr_info(LOG_TAG "call %s fbdev node %d \n",__FUNCTION__,pinfo->node);
    return 0;
}
