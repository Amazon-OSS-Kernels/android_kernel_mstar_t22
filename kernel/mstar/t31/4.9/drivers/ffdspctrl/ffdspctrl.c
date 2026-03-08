/*
 *  drivers/ffdspctrl/ffdspctrl.c
 *
 *
 * This ffdsp  driver is used to control FarFeild power
 *
 * Portions copyright 2020 - 2021 Amazon Technologies, Inc. All Rights Reserved.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>

#include "ffmt8570.h"
#include "ffsuecreek.h"

#define DEVICE_NAME		"ffdspctrl"

enum {
	DSP_SUECREEK,
	DSP_MT8570,
	DSP_MUX,
};

static struct platform_device ffdspctrl_device = {
	.name	= DEVICE_NAME,
};

static int dsp_type;

int get_dsp_type(void)
{
	if (strstr(saved_command_line, "farfield.dsp.name=suecreek") != NULL) {
		pr_info("ffdspctrl suecreek dsp is supported on this product\n");
		return DSP_SUECREEK;
	} else if (strstr(saved_command_line, "farfield.dsp.name=mt8570") != NULL) {
		pr_info("ffdspctrl mt8570 dsp is supported on this product\n");
		return DSP_MT8570;
	} else {
		pr_err("ffdspctrl unknowed dsp is supported on this product\n");
		return DSP_MUX;
	}

	return DSP_MUX;
}

static int ffdspctrl_probe(struct platform_device *pdev)
{
	int ret = 0;

	dsp_type = get_dsp_type();
	switch(dsp_type) {
	case DSP_SUECREEK:
		ret = ffdsp_suecreek_init(pdev);
		if (ret != 0) {
			pr_err("ffdspctrl ffdsp suecreek init err\n");
			return ret;
		}
		break;
	case DSP_MT8570:
		ret = ffdsp_mt8570_init(pdev);
		if (ret != 0) {
			pr_err("ffdspctrl ffdsp mt8570 init err\n");
			return ret;
		}
		break;
	default:
		break;
	}

	return 0;
}

static int ffdspctrl_remove(struct platform_device *pdev)
{
	switch(dsp_type) {
	case DSP_SUECREEK:
		ffdsp_suecreek_deinit(pdev);
		break;
	case DSP_MT8570:
		ffdsp_mt8570_deinit(pdev);
		break;
	default:
		break;
	}

	return 0;
}

static int ffdspctrl_suspend(struct platform_device *pdev, pm_message_t state)
{
	switch(dsp_type) {
	case DSP_SUECREEK:
		return ffdsp_suecreek_suspend(pdev, state);
	case DSP_MT8570:
		break;
	default:
		break;
	}

	return 0;
}

static int ffdspctrl_resume(struct platform_device *pdev)
{
	switch(dsp_type) {
	case DSP_SUECREEK:
		return ffdsp_suecreek_resume(pdev);
	case DSP_MT8570:
		break;
	default:
		break;
	}

	return 0;
}

static struct platform_driver ffdspctrl_driver = {
	.probe		= ffdspctrl_probe,
	.remove		= ffdspctrl_remove,
	.suspend	= ffdspctrl_suspend,
	.resume		= ffdspctrl_resume,
	.driver		= {
		.name	= DEVICE_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init ffdspctrl_init(void)
{
	int ret;
	ret = platform_device_register(&ffdspctrl_device);
	if (ret < 0) {
		pr_err("failed to register ffdspctrl device\n");
		return ret;
	}
	ret = platform_driver_register(&ffdspctrl_driver);
	if (ret < 0) {
		pr_err("failed to register ffdspctrl driver\n");
		platform_device_unregister(&ffdspctrl_device);
	}
	return ret;
}

static void __exit ffdspctrl_exit(void)
{
	platform_driver_unregister(&ffdspctrl_driver);
	platform_device_unregister(&ffdspctrl_device);
}

module_init(ffdspctrl_init);
module_exit(ffdspctrl_exit);

MODULE_DESCRIPTION("Farfeild DSP control");
MODULE_LICENSE("GPL");

