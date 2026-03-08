/*
 *  drivers/ffdspctrl/ffsuecreek.c
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

#include "ffsuecreek.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/kdev_t.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include "mdrv_gpio.h"
#include "mhal_gpio.h"
#include "mhal_gpio_reg.h"
#include "ffdspdev.h"

#if defined(CONFIG_IDME)
extern char *idme_get_config_name(void);
#define DTS_STRING_LENGTH 64
#endif

#define GPIO_IN         0x1
#define GPIO_OUT_LOW    0x2
#define GPIO_OUT_HIGH   0x3

#define PAD_GPIO8_PM    (0x000f<<9) + (0x10<<2)

#if defined(CONFIG_ARM64)
extern ptrdiff_t mstar_pm_base;
#define REG_ADDR(addr)  (*((volatile u16 *)((mstar_pm_base + (addr )))))
#define BASEREG_ADDR(addr)  ((mstar_pm_base + (addr )))
#else
#define REG_RIU_BASE 0xFD000000
#define REG_ADDR(addr)  (*((volatile u16 *)((REG_RIU_BASE + (addr )))))
#define BASEREG_ADDR(addr)  (REG_RIU_BASE + (addr ))
#endif

static bool ffmute_enable;

struct ff_suecreek_dev {
	struct ffmute_dev dev_com;
	u32 ffdsp_power_gpio;
	u32 ffmute_led_gpio;
	u32 ffdsp_reset_gpio;
	u32 ffdsp_bootmode_gpio;
	u32 ffmute_gpio;
	u32 ffmute_gpio_requested;
	int irq;
	bool is_suspend;
};

static struct task_struct *ffmute_poll_task = NULL;

static int ffmute_poll(void* arg)
{
	int state = 0;

	struct ff_suecreek_dev *fdev = (struct ff_suecreek_dev *)arg;

	while (1) {
		if (kthread_should_stop())
			break;

		msleep(100);

		state = !gpio_get_value(fdev->ffmute_gpio);
		if (state == fdev->dev_com.ffmute_state)
			continue;

		pr_info("%s, ffdspctrl ffmute state changed, gpio: %u, state: %d\n",
			__func__, fdev->ffmute_gpio, state);

		fdev->dev_com.ffmute_state = state;
		schedule_work(&fdev->dev_com.work);
	}

	return 0;
}

void gpio_init(struct ff_suecreek_dev *dev)
{
	struct device_node *np;
	u32 prop;
	char project_name[DTS_STRING_LENGTH];
	char property_name[DTS_STRING_LENGTH];

	np = of_find_node_by_name(NULL, "ffdspctrl_gpio");
	if (np == NULL) {
		pr_err("ffdspctrl: ffdspctrl_gpio is not defined in dts\n");
		return;
	}

	snprintf((char *)project_name, DTS_STRING_LENGTH, "%s", idme_get_config_name());
	char *hw_build_id = memchr(project_name, '_', sizeof(project_name));
	if (hw_build_id) {
		/*Remove hw_specific string*/
		*hw_build_id = '\0';
	}

	snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "ffdspctrl-power-gpio_", project_name);
	if (!of_property_read_u32(np, property_name, &prop)) {
		dev->ffdsp_power_gpio = prop;
		pr_info("ffdspctrl: %s is %d \n", property_name, prop);
        } else if (!of_property_read_u32(np, "ffdspctrl-power-gpio", &prop)) {
		dev->ffdsp_power_gpio = prop;
		pr_info("ffdspctrl: ffdsp_power_gpio is %d \n", prop);
	} else {
		pr_err("ffdspctrl ffdsp_power_gpio is not defined \n");
		dev->ffdsp_power_gpio = -1;
	}

	snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "ffmute-led-gpio_", project_name);
	if (!of_property_read_u32(np, property_name, &prop)) {
		dev->ffmute_led_gpio = prop;
		pr_info("ffdspctrl: %s is %d \n", property_name, prop);
        } else if (!of_property_read_u32(np, "ffmute-led-gpio", &prop)) {
		dev->ffmute_led_gpio = prop;
		pr_info("ffdspctrl: ffmute_led_gpio is %d \n", prop);
	} else {
		pr_err("ffdspctrl ffmute_led_gpio is not defined \n");
		dev->ffmute_led_gpio = -1;
	}

	snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "ffmute-gpio_", project_name);
	if (!of_property_read_u32(np, property_name, &prop)) {
		dev->ffmute_gpio = prop;
		pr_info("ffdspctrl: %s is %d \n", property_name, prop);
        } else if (!of_property_read_u32(np, "ffmute-gpio", &prop)) {
		dev->ffmute_gpio = prop;
		pr_info("ffdspctrl: ffmute_gpio is %d \n", prop);
	} else {
		pr_err("ffdspctrl ffmute_gpio is not defined\n");
		dev->ffmute_gpio = -1;
	}

	snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "ffdspctrl-reset-gpio_", project_name);
	if (!of_property_read_u32(np, property_name, &prop)) {
		dev->ffdsp_reset_gpio = prop;
		pr_info("ffdspctrl: %s is %d \n", property_name, prop);
        } else if (!of_property_read_u32(np, "ffdspctrl-reset-gpio", &prop)) {
		dev->ffdsp_reset_gpio = prop;
		pr_info("ffdspctrl: ffdsp_reset_gpio is %d \n", prop);
	} else {
		pr_err("ffdspctrl_reset_gpio is not defined \n");
		dev->ffdsp_reset_gpio = -1;
	}

	snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "ffdspctrl-bootmode-gpio_", project_name);
	if (!of_property_read_u32(np, property_name, &prop)) {
		dev->ffdsp_bootmode_gpio = prop;
		pr_info("ffdspctrl: %s is %d \n", property_name, prop);
        } else if (!of_property_read_u32(np, "ffdspctrl-bootmode-gpio", &prop)) {
		dev->ffdsp_bootmode_gpio = prop;
		pr_info("ffdspctrl: ffdsp_bootmode_gpio is %d \n", prop);
	} else {
		pr_err("ffdspctrl_bootmode_gpio is not defined \n");
		dev->ffdsp_bootmode_gpio = -1;
	}

	MDrv_GPIO_Init();

	if ((dev->ffmute_gpio != -1) || (strstr((char *)project_name, "haileyplus") != NULL)) {
		pr_info("ffdspctrl: ffmute enable\n");
		ffmute_enable = true;
	}
}

int ffmute_init(struct ff_suecreek_dev *dev)
{
	int ret;

	if (dev->ffmute_gpio == -1)
		return 0;

	dev->ffmute_gpio_requested = gpio_request(dev->ffmute_gpio, "ffmute");
	if (dev->ffmute_gpio_requested < 0) {
		pr_info("%s, ffdspctrl gpio[%d] maybe has already been requested, ret: %d\n",
			__func__, dev->ffmute_gpio, ret);
	}

	ret = gpio_direction_input(dev->ffmute_gpio);
	if (ret < 0) {
		pr_err("%s: ffdspctrl gpio[%d] direction input err, ret: %d\n", __func__, dev->ffmute_gpio, ret);
		goto err_set_gpio_input;
	}

	if (ffmute_poll_task == NULL) {
		ffmute_poll_task = kthread_create(ffmute_poll, dev, "ffmute poll Task");
		if (IS_ERR(ffmute_poll_task)) {
			pr_err("ffdspctrl create kthread for ffmute GPIO poll Task fail\n");
			ret = PTR_ERR(ffmute_poll_task);
			goto err_set_gpio_input;
		} else {
			wake_up_process(ffmute_poll_task);
		}
	} else {
		pr_err("%s, ffdspctrl ffmute_poll_task is already created\n", __func__);
		ret = -EPERM;
		goto err_set_gpio_input;
	}

	pr_info("%s: ffdspctrl exit\n", __func__);

	return 0;

err_set_gpio_input:
	if (dev->ffmute_gpio_requested == 0)
		gpio_free(dev->ffmute_gpio);

	return ret;
}

void ffmute_deinit(struct ff_suecreek_dev *dev)
{
	if (dev->ffmute_gpio != -1) {
		kthread_stop(ffmute_poll_task);
		ffmute_poll_task = NULL;

		if (dev->ffmute_gpio_requested == 0)
			gpio_free(dev->ffmute_gpio);
	}
}

int ffdsp_suecreek_init(struct platform_device *pdev)
{
	int ret;

	struct ff_suecreek_dev *priv_dev = kzalloc(sizeof(struct ff_suecreek_dev), GFP_KERNEL);
	if (!priv_dev) {
		pr_err("ffdspctrl: ffdspctrl suecreek allocation fail");
		return -ENOMEM;
	}

	gpio_init(priv_dev);

	platform_set_drvdata(pdev, priv_dev);

	ret = ffdsp_device_init((struct ffmute_dev *)priv_dev, ffmute_enable);
	if (ret != 0) {
		pr_err("ffdspctrl: ffsuecreek device init fail");
		kfree(priv_dev);
		return ret;
	}

	if (!ffmute_enable)
		return 0;

	INIT_WORK(&(priv_dev->dev_com.work), priv_dev->dev_com.work_func);

	ret = ffmute_init(priv_dev);
	if (ret != 0) {
		pr_err("ffdspctrl: ffmute init fail");
		ffdsp_device_deinit((struct ffmute_dev *)priv_dev, ffmute_enable);
		return ret;
	}

	return 0;
}

void ffdsp_suecreek_deinit(struct platform_device *pdev)
{
	struct ff_suecreek_dev *priv_dev = platform_get_drvdata(pdev);

	ffmute_deinit(priv_dev);

	cancel_work_sync(&priv_dev->dev_com.work);

	ffdsp_device_deinit((struct ffmute_dev *)priv_dev, ffmute_enable);

	kfree(priv_dev);
}

int ffdsp_suecreek_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct ff_suecreek_dev *dev = platform_get_drvdata(pdev);
	if (dev) {
		if (dev->ffdsp_power_gpio != -1) {
			MDrv_GPIO_Set_High(dev->ffdsp_power_gpio);
		} else if (dev->ffdsp_reset_gpio != -1) {
			MDrv_GPIO_Set_Low(dev->ffdsp_reset_gpio);
		}

		if (dev->ffmute_led_gpio != -1) {
			MDrv_GPIO_Set_High(dev->ffmute_led_gpio);
		}

		dev->is_suspend = true;
	}
	return 0;
}

int ffdsp_suecreek_resume(struct platform_device *pdev)
{
	struct ff_suecreek_dev *dev = platform_get_drvdata(pdev);

    if (strstr(idme_get_config_name(), "brandenburg") != NULL && \
            strstr(idme_get_config_name(), "abc123") == NULL) {
        /* GPIO8_PM would conflict between abc123, reconfig to output mode and lo for BBURG */
        pr_info("[%s] Re-config GPIO8_PM to output mode and lo for BBURG\n", __func__);
        REG_ADDR(PAD_GPIO8_PM) = (REG_ADDR(PAD_GPIO8_PM) & ~(BIT(1)|BIT(0))) | GPIO_OUT_LOW;
    }

	if (dev) {
		if (dev->ffdsp_power_gpio != -1 ) {
			MDrv_GPIO_Set_Low(dev->ffdsp_power_gpio);
		}

		if (dev->ffdsp_reset_gpio != -1 ) {
			MDrv_GPIO_Set_Low(dev->ffdsp_reset_gpio);
		}

		mdelay(10);
		if (dev->ffdsp_bootmode_gpio != -1) {
			MDrv_GPIO_Set_Low(dev->ffdsp_bootmode_gpio);
		}
		mdelay(5);
		if (dev->ffdsp_reset_gpio != -1 ) {
			MDrv_GPIO_Set_High(dev->ffdsp_reset_gpio);
		}
		if (dev->ffmute_led_gpio != -1 ) {
			MDrv_GPIO_Set_Low(dev->ffmute_led_gpio);
		}

		dev->is_suspend = false;
	}
	return 0;
}

