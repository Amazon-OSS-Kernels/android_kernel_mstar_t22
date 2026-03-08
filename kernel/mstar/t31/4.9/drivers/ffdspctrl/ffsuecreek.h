/*
 *  drivers/ffdspctrl/ffsuecreek.h
 *
 *
 * This ffdsp  driver is used to control suecreek dsp FarFeild
 *
 * Portions copyright 2021 Amazon Technologies, Inc. All Rights Reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#ifndef __FFSUECREEK_H__
#define __FFSUECREEK_H__

#include <linux/platform_device.h>

int ffdsp_suecreek_init(struct platform_device *pdev);
void ffdsp_suecreek_deinit(struct platform_device *pdev);
int ffdsp_suecreek_suspend(struct platform_device *pdev, pm_message_t state);
int ffdsp_suecreek_resume(struct platform_device *pdev);

#endif

