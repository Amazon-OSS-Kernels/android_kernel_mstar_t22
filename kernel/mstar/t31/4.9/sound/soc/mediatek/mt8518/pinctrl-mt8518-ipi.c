/*
 * mt65xx pinctrl driver based on Allwinner A1X pinctrl driver.
 * Copyright (c) 2014 MediaTek Inc.
 * Author: Hongzhou.Yang <hongzhou.yang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <adsp_ipi.h>
#include "adsp_ipi_platform.h"
#include "pinctrl-mt8518-ipi.h"

static int check_msg_format(const struct gpio_ipi_msg_t *p_ipi_msg,
				unsigned int len)
{
	if (p_ipi_msg->magic != IPI_MSG_GPIO_MAGIC_NUMBER) {
		pr_notice("magic 0x%x error!!", p_ipi_msg->magic);
		return -1;
	}
	if (sizeof(struct gpio_ipi_msg_t) > len) {
		pr_notice("len 0x%x error!!", len);
		return -1;
	}
	return 0;
}
int gpio_send_ipi_msg(uint16_t pin,
			uint16_t set_ipi_type, uint16_t param)
{
	struct gpio_ipi_msg_t *p_ipi_msg;
	uint32_t ipi_msg_len = 0;

	int send_status = -1;

	p_ipi_msg  = kmalloc(sizeof(struct gpio_ipi_msg_t), GFP_KERNEL);
	if (p_ipi_msg == NULL)
		return -1;
	memset(p_ipi_msg, 0, sizeof(struct gpio_ipi_msg_t));

	p_ipi_msg->magic = IPI_MSG_GPIO_MAGIC_NUMBER;
	p_ipi_msg->pin = pin;
	p_ipi_msg->param = param;
	p_ipi_msg->set_ipi_type = set_ipi_type;
	ipi_msg_len = sizeof(struct gpio_ipi_msg_t);

	if (check_msg_format(p_ipi_msg, ipi_msg_len) != 0) {
		pr_info("[%s], drop msg due to ipi fmt err", __func__);
		return -1;
	}
	send_status = adsp_ipi_send(
						ADSP_IPI_GPIO,
						p_ipi_msg,
						ipi_msg_len,
						0, /* default don't wait */
						0);

		if (send_status == 0) {
			pr_info("[%s], set msg ok\n", __func__);
		} else {
			pr_info("[%s], gpio_ipi_send error %d",
					  __func__, send_status);
		}

	return (send_status == 0) ? 0 : -1;
}

