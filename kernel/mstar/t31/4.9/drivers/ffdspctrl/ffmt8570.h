/*
 *  drivers/ffdspctrl/ffmt8570.h
 *
 *
 * This ffdsp  driver is used to control mt8570 dsp FarFeild
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

#ifndef __FFMT8570_H__
#define __FFMT8570_H__

#include <linux/platform_device.h>

int ffdsp_mt8570_init(struct platform_device *pdev);
void ffdsp_mt8570_deinit(struct platform_device *pdev);

#endif

