/*
 *  drivers/ffdspctrl/ffdspdev.c
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

#include "ffdspdev.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kdev_t.h>

#define FFMUTE_NAME		"ffmute"
#define DEVICE_NAME		"ffdspctrl"

struct class *ffdspctrl_class;

static ssize_t state_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	int state = 0;
	struct ffmute_dev *fdev = (struct ffmute_dev *)dev_get_drvdata(dev);

	if (kstrtouint(buf, 10, &state))
		return -EINVAL;

	pr_info("%s, ffdspctrl ffmute store state changed, state: %d\n", __func__, state);

	if (fdev->ffmute_state != state) {
		fdev->ffmute_state = state;
		schedule_work(&fdev->work);
	}

	return size;
}

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct ffmute_dev *fdev = (struct ffmute_dev *)dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", fdev->ffmute_state);
}

static ssize_t name_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%s\n", FFMUTE_NAME);
}

static DEVICE_ATTR(state, S_IRUGO|S_IWUSR, state_show, state_store);
static DEVICE_ATTR(name, S_IRUGO, name_show, NULL);

static void ffmute_switch_work(struct work_struct *work)
{
	char name_buf[120];
	char state_buf[120];
	char *prop_buf;
	char *envp[3];
	int env_offset = 0;
	int length;

	struct ffmute_dev *fdev = container_of(work, struct ffmute_dev, work);

	pr_info("%s, ffdspctrl ffmute state: %d\n", __func__, fdev->ffmute_state);

	prop_buf = (char *)get_zeroed_page(GFP_KERNEL);
	if (prop_buf) {
		length = name_show(fdev->dev, NULL, prop_buf);
		if (length > 0) {
			if (prop_buf[length - 1] == '\n')
				prop_buf[length - 1] = 0;
			snprintf(name_buf, sizeof(name_buf),
				"FFMUTE_NAME=%s", prop_buf);
			envp[env_offset++] = name_buf;
		}
		length = state_show(fdev->dev, NULL, prop_buf);
		if (length > 0) {
			if (prop_buf[length - 1] == '\n')
				prop_buf[length - 1] = 0;
			snprintf(state_buf, sizeof(state_buf),
				"FFMUTE_STATE=%s", prop_buf);
			envp[env_offset++] = state_buf;
		}
		envp[env_offset] = NULL;
		kobject_uevent_env(&fdev->dev->kobj, KOBJ_CHANGE, envp);
		free_page((unsigned long)prop_buf);
	} else {
		pr_err(KERN_ERR "ffdspctrl out of memory in switch_set_state\n");
		kobject_uevent(&fdev->dev->kobj, KOBJ_CHANGE);
	}
}

int ffdsp_device_init(struct ffmute_dev *fdev, bool ffmute_enable)
{
	int ret;
	static struct device *ffmute_dev;

	ffdspctrl_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(ffdspctrl_class)) {
		pr_err("ffdspctrl: create class fail\n");
		return PTR_ERR(ffdspctrl_class);
	}
	ffmute_dev = device_create(ffdspctrl_class, NULL, MKDEV(0, 0), fdev, "ffdscptcrl%d", 1);
	fdev->dev = ffmute_dev;

	if (!ffmute_enable) {
		pr_info("ffdspctrl: no need to creat device file\n");
		return 0;
	}

	ret = device_create_file(ffmute_dev, &dev_attr_state);
	if (ret < 0) {
		pr_err("%s: ffdspctrl device_create_state err, ret: %d\n", __func__, ret);
		goto err_dev_attr_state;
	}

	ret = device_create_file(ffmute_dev, &dev_attr_name);
	if (ret < 0) {
		pr_err("%s: ffdspctrl device_create_name err, ret: %d\n", __func__, ret);
		goto err_dev_attr_name;
	}

	fdev->work_func = ffmute_switch_work;

	return 0;

err_dev_attr_name:
	device_remove_file(fdev->dev, &dev_attr_state);
err_dev_attr_state:
	device_destroy(ffdspctrl_class, MKDEV(0, 0));
	class_destroy(ffdspctrl_class);

	return ret;
}

void ffdsp_device_deinit(struct ffmute_dev *fdev, bool ffmute)
{
	if (ffmute) {
		device_remove_file(fdev->dev, &dev_attr_name);
		device_remove_file(fdev->dev, &dev_attr_state);
	}

	device_destroy(ffdspctrl_class, MKDEV(0, 0));
	class_destroy(ffdspctrl_class);
}

