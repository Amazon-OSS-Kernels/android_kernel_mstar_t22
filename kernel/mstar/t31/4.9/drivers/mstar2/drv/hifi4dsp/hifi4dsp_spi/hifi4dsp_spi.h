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

#ifndef _H_HIFI4DSP_SPI_H_
#define _H_HIFI4DSP_SPI_H_

#include <linux/interrupt.h>

/* HIFI4DSP SPI xfer speed */
#define SPI_LOAD_IMAGE_SPEED		(18*1000*1000)
//#define SPI_SPEED_LOW				(12*1000*1000)
#define SPI_SPEED_HIGH				(18*1000*1000)
#define SPI_SPEED_LOW 1 //read speed form bootarg, ref:spi-mstar.c
#if defined(CONFIG_ARM64)
extern ptrdiff_t mstar_pm_base;
#define REG_ADDR(addr)  (*((volatile u16 *)((mstar_pm_base + (addr )))))
#define BASEREG_ADDR(addr)  ((mstar_pm_base + (addr )))
#else
#define REG_RIU_BASE 0xFD000000
#define REG_ADDR(addr)  (*((volatile u16 *)((REG_RIU_BASE + (addr )))))
#define BASEREG_ADDR(addr)  (REG_RIU_BASE + (addr ))
#endif

extern int request_gpio_irq(int gpio_num, irq_handler_t handler, unsigned long irqflags, void *dev_id);
extern int free_gpio_irq(int gpio_num, void *dev_id);
#if defined(CONFIG_IDME)
extern char *idme_get_config_name(void);
#define DTS_STRING_LENGTH (64)
#endif

/*
 * Public function API for audio system
 */
extern int dsp_spi_write(u32 addr, void *value, int len, u32 speed);
extern int dsp_spi_write_ex(u32 addr, void *value, int len, u32 speed);
extern int dsp_spi_read(u32 addr, void *value, int len, u32 speed);
extern int dsp_spi_read_ex(u32 addr, void *value, int len, u32 speed);

extern int spi_read_register(u32 addr, u32 *val, u32 speed);
extern int spi_write_register(u32 addr, u32 val, u32 speed);
extern int spi_set_register32(u32 addr, u32 val, u32 speed);
extern int spi_clr_register32(u32 addr, u32 val, u32 speed);
extern int spi_write_register_mask(u32 addr, u32 val, u32 msk, u32 speed);

extern int hifi4dsp_spi_get_status(void);
extern void hifi4dsp_spi_set_config_mode_status(int status);
extern int spi_config_MSB(void);

#endif /*_H_HIFI4DSP_SPI_H_*/
