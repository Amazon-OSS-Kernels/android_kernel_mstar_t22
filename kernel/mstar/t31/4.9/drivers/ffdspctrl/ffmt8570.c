/*
 *  drivers/ffdspctrl/ffmt8570.c
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

#include "ffmt8570.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>

#include "ffdspdev.h"

struct ff_mt8570_dev {
	struct ffmute_dev dev_com;
};

int ffdsp_mt8570_init(struct platform_device *pdev)
{
	int ret;

	struct ff_mt8570_dev *priv_dev = kzalloc(sizeof(struct ff_mt8570_dev), GFP_KERNEL);
	if (!priv_dev) {
		pr_err("ffdspctrl: ffdspctrl mt8570 allocation fail");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, priv_dev);

	ret = ffdsp_device_init((struct ffmute_dev *)priv_dev, true);
	if (ret != 0) {
		pr_err("ffdspctrl: ffmt8570 device init fail");
		kfree(priv_dev);
		return ret;
	}

	INIT_WORK(&(priv_dev->dev_com.work), priv_dev->dev_com.work_func);

	return 0;
}

void ffdsp_mt8570_deinit(struct platform_device *pdev)
{
	struct ff_mt8570_dev *priv_dev = platform_get_drvdata(pdev);

	cancel_work_sync(&priv_dev->dev_com.work);

	ffdsp_device_deinit((struct ffmute_dev *)priv_dev, true);

	kfree(priv_dev);
}

