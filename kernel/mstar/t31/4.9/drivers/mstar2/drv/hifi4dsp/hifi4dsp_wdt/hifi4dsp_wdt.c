/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <hifi4dsp_load/hifi4dsp_load.h>
#include <hifi4dsp_wdt/hifi4dsp_wdt.h>

#ifdef CONFIG_AMAZON_DSP_FRAMEWORK
#include "adf/adf_status.h"
#include "adf/adf_common.h"
#endif

#define DRV_NAME        "mtk-dsp_wdt"

struct mtk_dsp_wdt_dev {
    struct device *dev;
    void __iomem *dsp_wdt_base;;
    unsigned int dsp_wdt_irq_id;
    u32 dsp_wdt_gpio;
    u32 dsp_wdt_inverse;
    u32 dsp_wdt_is_enabled;
};

static struct workqueue_struct *dsp_wdt_queue;
static struct work_struct dsp_wdt_work;
static struct platform_device *gpdev = NULL;

BLOCKING_NOTIFIER_HEAD(wdt_notifier_list);

static void dump_adsp_log_buf(void)
{
    u32 tmp;
#ifndef CONFIG_AMAZON_DSP_FRAMEWORK
    char *log_buf = NULL;
    u32 log_size = 0;
    int ret = 0;
    int i;
    char tmp_buf[512];
    u32 tmp_log_start, tmp_log_size;
#endif

    /* disable watchdog */
    spi_read_register(0x1D010000, &tmp, SPI_SPEED_HIGH);
    tmp &= ~(1 << 0);
    tmp |= (0x22000000);
    spi_write_register(0x1D010000, tmp, SPI_SPEED_HIGH);

    spi_read_register(0x1D010000, &tmp, SPI_SPEED_HIGH);

    pr_notice("%s, watchdog mode:0x%x\n", __func__, tmp);

#ifndef CONFIG_AMAZON_DSP_FRAMEWORK
    ret = hifi4dsp_get_log_buf_size(&log_size);
    if (ret) {
        pr_info("Dump DSP log fail, ret=%d\n", ret);
        goto _end;
    }

    log_buf = kzalloc(log_size, GFP_KERNEL);
    if (!log_buf) {
        ret = -ENOMEM;
        goto _end;
    }

    ret = hifi4dsp_get_log_buf(log_buf, log_size);
    if (ret) {
        pr_info("Read log_buf fail, ret=%d\n", ret);
        goto _end;
    }

    pr_info("log_buf size: %d\n", log_size);

    pr_info("==========================\n");
    pr_info("=== dump dsp log start ===\n");
    pr_info("==========================\n");
    for (i = 0, tmp_log_start = 0; i < log_size; i++) {
        if (log_buf[i] == '\n' || i == log_size - 1) {
            tmp_log_size = i - tmp_log_start + 1;
            strncpy(tmp_buf, log_buf + tmp_log_start, tmp_log_size);
            tmp_buf[tmp_log_size] = '\0';
            pr_info("%s", tmp_buf);
            tmp_log_start = i + 1;
        }

    }
    pr_info("==========================\n");
    pr_info("=== dump dsp log end  ====\n");
    pr_info("==========================\n");
_end:
    kfree(log_buf);
#else
	adfDebug_printLog(ADF_LOG_DUMP_ALL);
#endif
    WARN_ON(1);
}

#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
static int notify_rst_adsp(struct notifier_block *this,
                 unsigned long code, void *unused)
{
    hifi4dsp_rst();

    return NOTIFY_DONE;
}
#endif

static int
dbg_notify_show_adsp_log(struct notifier_block *this,
                 unsigned long code, void *unused)
{
    if (hifi4dsp_run_status())
        dump_adsp_log_buf();
    else
        pr_info("DSP is not ready!\n");

    clr_hifi4dsp_run_status();
    return NOTIFY_DONE;
}

static struct notifier_block dbg_show_log_notifier = {
    .notifier_call  = dbg_notify_show_adsp_log,
    .priority       = 100,
};

#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
static struct notifier_block adsp_rst_notifier = {
    .notifier_call  = notify_rst_adsp,
    .priority       = 99,
};
#endif

static irqreturn_t mtk_dsp_wdt_isr(int irq, void *dev_id)
{
        queue_work(dsp_wdt_queue, &dsp_wdt_work);
	return IRQ_NONE;
}

void hifi4dsp_wdt_handler(void)
{
    blocking_notifier_call_chain(&wdt_notifier_list, 0, NULL);
}

void dsp_wdt_work_handler(struct work_struct *unused)
{
    char data[32], *envp[] = { data, NULL };
    mtk_dsp_wdt_disable();
    pr_notice("[%s] ADSP happens exception!\n", __func__);

    snprintf(data, sizeof(data), "ACTION=DSP_WTD_WHOLE");
    kobject_uevent_env(&gpdev->dev.kobj, KOBJ_CHANGE, envp);
    pr_warning("[%s][DSP_HANG]\n", __func__);

}

int register_adsp_wdt_notifier(struct notifier_block *nb)
{
    return blocking_notifier_chain_register(&wdt_notifier_list, nb);
}

int unregister_adsp_wdt_notifier(struct notifier_block *nb)
{
    return blocking_notifier_chain_unregister(&wdt_notifier_list, nb);
}

static int mtk_dsp_wdt_probe(struct platform_device *pdev)
{

	if (strstr(saved_command_line, "farfield.dsp.name=mt8570") == NULL) {
		pr_info("mt8570 is not supported\n");
		return -EINVAL;
    }

#if defined(CONFIG_IDME)
	char property_name[DTS_STRING_LENGTH];
	char buffer_default[DTS_STRING_LENGTH];
	char *hw_build_id;
#endif
    int err;
	u32 prop;
    struct mtk_dsp_wdt_dev *mtk_dsp_wdt;

    pr_info("%s() enter.\n", __func__);

    mtk_dsp_wdt = devm_kzalloc(&pdev->dev,
                sizeof(*mtk_dsp_wdt), GFP_KERNEL);
    if (!mtk_dsp_wdt)
        return -ENOMEM;

#if defined(CONFIG_IDME)
	memset (property_name, 0, sizeof(property_name));
	memset (buffer_default, 0, sizeof(buffer_default));

	snprintf((char *)property_name, DTS_STRING_LENGTH, "%s", idme_get_config_name());
	hw_build_id = memchr(property_name, '_', sizeof(property_name));
	if (hw_build_id) {
		/*Remove hw_specific string*/
		*hw_build_id = '\0';
	}

	snprintf((char *)buffer_default, DTS_STRING_LENGTH, "%s%s", "dsp-wdt-gpio_", property_name);
	if (!of_property_read_u32(pdev->dev.of_node, buffer_default, &prop)) {
		mtk_dsp_wdt->dsp_wdt_gpio = prop;
		pr_info("%s:  %s is %d \n", __func__, buffer_default, prop);
	} else
#endif
	if (!of_property_read_u32(pdev->dev.of_node, "dsp-wdt-gpio", &prop)) {
		mtk_dsp_wdt->dsp_wdt_gpio = prop;
		pr_info("%s: dsp-wdt-gpio is %d \n", __func__, prop);
	} else {
		pr_err("%s: dsp-wdt-gpio is not defined \n", __func__);
		mtk_dsp_wdt->dsp_wdt_gpio = 7;
	}

#if defined(CONFIG_IDME)
	snprintf((char *)buffer_default, DTS_STRING_LENGTH, "%s%s", "dsp-wdt-inverse_", property_name);
	if (!of_property_read_u32(pdev->dev.of_node, buffer_default, &prop)) {
		mtk_dsp_wdt->dsp_wdt_inverse = prop;
		pr_info("%s: %s is %d \n", __func__, buffer_default, prop);
	} else
#endif
	if (!of_property_read_u32(pdev->dev.of_node, "dsp-wdt-inverse", &prop)) {
		mtk_dsp_wdt->dsp_wdt_inverse = prop;
		pr_info("%s: %s is %d \n", __func__, property_name, prop);
	} else {
		pr_err("%s: dsp-wdt-inverse is not defined \n", __func__);
		mtk_dsp_wdt->dsp_wdt_inverse = 0;
	}

	dsp_wdt_queue = create_singlethread_workqueue("dsp_wdt_kworker");
	INIT_WORK(&dsp_wdt_work, dsp_wdt_work_handler);

	err = request_gpio_irq(mtk_dsp_wdt->dsp_wdt_gpio, mtk_dsp_wdt_isr, (mtk_dsp_wdt->dsp_wdt_inverse ? IRQF_TRIGGER_RISING : IRQF_TRIGGER_FALLING), &pdev->dev);
	if (err != 0) {
		pr_notice("hifi4dsp: %s: failed to request irq %d(err:%d)\n", __func__, mtk_dsp_wdt->dsp_wdt_gpio, err);
		devm_kfree(&pdev->dev, mtk_dsp_wdt);
		return err;
	}
    else {
        mtk_dsp_wdt->dsp_wdt_is_enabled = 1;
    }

    register_adsp_wdt_notifier(&dbg_show_log_notifier);
#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
    register_adsp_wdt_notifier(&adsp_rst_notifier);
#endif

	platform_set_drvdata(pdev, mtk_dsp_wdt);
   gpdev = pdev;

    return 0;
}

void mtk_dsp_wdt_disable(void)
{
    int ret;
    pr_info("Disable DSP WDT interruption \n");
    struct mtk_dsp_wdt_dev *dev = dev_get_drvdata(&gpdev->dev);
    if (dev->dsp_wdt_is_enabled) {
        ret = free_gpio_irq(dev->dsp_wdt_gpio, &gpdev->dev);
        if (ret != 0) {
            pr_err(" %s: failed to free irq %d(err:%d)\n", __func__, dev->dsp_wdt_gpio, ret);
        }
        else {
            dev->dsp_wdt_is_enabled = 0;
        }
    }
    else
        pr_info("Already DSP WDT is disabled.\n");
}
void mtk_dsp_wdt_enable(void)
{
    int ret;
    pr_info("Enable DSP WDT interruption \n");
    struct mtk_dsp_wdt_dev *dev = dev_get_drvdata(&gpdev->dev);
    if (!dev->dsp_wdt_is_enabled) {
        ret = request_gpio_irq(dev->dsp_wdt_gpio, mtk_dsp_wdt_isr, (dev->dsp_wdt_inverse ? IRQF_TRIGGER_RISING : IRQF_TRIGGER_FALLING), &gpdev->dev);
        if (ret != 0) {
            pr_err(" %s: failed to request irq %d(err:%d)\n", __func__, dev->dsp_wdt_gpio, ret);
        }
        else {
            dev->dsp_wdt_is_enabled = 1;
        }
    }
    else
        pr_info("Already DSP WDT is enabled.\n");
}

static int mtk_dsp_wdt_pm_suspend(struct device *device)
{
    int ret;
	struct mtk_dsp_wdt_dev *dev = dev_get_drvdata(device);
	pr_info("%s is suspend, disabled irq\n", __func__);
	ret = free_gpio_irq(dev->dsp_wdt_gpio, device);
    if (ret != 0) {
        pr_err(" %s: failed to free irq %d(err:%d)\n", __func__, dev->dsp_wdt_gpio, ret);
    }
    else {
        dev->dsp_wdt_is_enabled = 0;
    }

	return 0;
}

static int mtk_dsp_wdt_pm_resume(struct device *device)
{
	struct mtk_dsp_wdt_dev *dev = dev_get_drvdata(device);
	int ret;
	pr_info("%s is resume, enabled irq\n", __func__);
	ret = request_gpio_irq(dev->dsp_wdt_gpio, mtk_dsp_wdt_isr, (dev->dsp_wdt_inverse ? IRQF_TRIGGER_RISING : IRQF_TRIGGER_FALLING), device);
	if (ret != 0) {
		pr_err(" %s: failed to request irq %d(err:%d)\n", __func__, dev->dsp_wdt_gpio, ret);
	}
    else {
        dev->dsp_wdt_is_enabled = 1;
    }
	return 0;
}

struct dev_pm_ops const mtk_dsp_wdt_pm_ops = {
	.suspend = mtk_dsp_wdt_pm_suspend,
	.resume = mtk_dsp_wdt_pm_resume,
};

static const struct of_device_id mtk_dsp_wdt_dt_ids[] = {
    { .compatible = "mediatek,hifi4dsp-wdt" },
    { /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, mtk_dsp_wdt_dt_ids);

static struct platform_driver mtk_dsp_wdt_driver = {
    .probe        = mtk_dsp_wdt_probe,
    .driver        = {
        .name        = DRV_NAME,
#ifdef CONFIG_OF
        .of_match_table    = mtk_dsp_wdt_dt_ids,
#endif
		.pm = &mtk_dsp_wdt_pm_ops,
    },
};

module_platform_driver(mtk_dsp_wdt_driver);

