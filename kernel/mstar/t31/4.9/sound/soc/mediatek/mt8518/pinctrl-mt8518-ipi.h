/*
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

#ifndef __PINCTRL_MT8518_IPI_H
#define __PINCTRL_MT8518_IPI_H

#define MAX_GPIO_IPI_MSG_BUF_SIZE     (272) /* SHARE_BUF_SIZE - 16 */
#define IPI_MSG_GPIO_MAGIC_NUMBER	0x8321

enum set_ipi_type {
	SET_MODE_IPI = 0,
	SET_DIR_IPI,
	SET_DOUT_IPI,
	SET_PULLEN_IPI,
	SET_PULLSEL_IPI,
	SET_IES_IPI,
	SET_SMT_IPI,
};

enum gpio_mode_type {
	GPIO_MODE_00    = 0,
	GPIO_MODE_01    = 1,
	GPIO_MODE_02    = 2,
	GPIO_MODE_03    = 3,
	GPIO_MODE_04    = 4,
	GPIO_MODE_05    = 5,
	GPIO_MODE_06    = 6,
	GPIO_MODE_07    = 7,
};
/* GPIO DIRECTION */
enum gpio_dir_type {
	GPIO_DIR_IN     = 0,
	GPIO_DIR_OUT    = 1,
};
/* GPIO PULL ENABLE*/
enum gpio_pullen_type {
	PULL_DISABLE = 0,
	PULL_ENABLE  = 1,
};

/* GPIO PULL-UP/PULL-DOWN*/
enum gpio_pull_type {
	PULL_DOWN  = 0,
	PULL_UP    = 1,
};
/* GPIO OUTPUT */
enum gpio_dout_type {
	GPIO_OUT_ZERO = 0,
	GPIO_OUT_ONE  = 1,
};
/* GPIO IES ENABLE*/
enum gpio_ies_type {
	IES_DISABLE = 0,
	IES_ENABLE  = 1,
};
/* GPIO SMT ENABLE*/
enum gpio_smt_type {
	SMT_DISABLE = 0,
	SMT_ENABLE  = 1,
};


struct gpio_ipi_msg_t {
	uint16_t magic;
	uint16_t pin;
	uint16_t set_ipi_type;
	uint16_t param;
};

int gpio_send_ipi_msg(uint16_t pin,
		uint16_t set_ipi_type, uint16_t param);

#endif /* __PINCTRL_MT8518_IPI_H */
