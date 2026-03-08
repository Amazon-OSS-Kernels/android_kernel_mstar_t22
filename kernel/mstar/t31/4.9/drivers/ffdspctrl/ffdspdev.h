/*
 *  drivers/ffdspctrl/ffdspdev.h
 *
 *
 * This ffdsp  driver is used to control FarFeild device
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

#ifndef __FFDSPDEV_H__
#define __FFDSPDEV_H__

#include <linux/workqueue.h>

typedef void (*dev_switch_work)(struct work_struct *work);

struct ffmute_dev {
	struct device *dev;
	struct work_struct work;
	dev_switch_work work_func;
	int ffmute_state;
};

int ffdsp_device_init(struct ffmute_dev *fdev, bool ffmute);
void ffdsp_device_deinit(struct ffmute_dev *fdev, bool ffmute);

#endif

