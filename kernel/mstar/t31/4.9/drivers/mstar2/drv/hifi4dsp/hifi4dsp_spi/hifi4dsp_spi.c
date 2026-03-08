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

#define HIFI4DSP_SPI_DRV_NAME	"hifi4dsp-spi"
#define pr_fmt(fmt) HIFI4DSP_SPI_DRV_NAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cache.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/platform_data/spi-mt65xx.h>
#include <hifi4dsp_spi/hifi4dsp_spi.h>

/*
 * SPI command description.
 */
#define CMD_PWOFF            0x02 /* Power Off */
#define CMD_PWON            0x04 /* Power On */
#define CMD_RS                0x06 /* Read Status */
#define CMD_WS                0x08 /* Write Status */
#define CMD_CR                0x0a /* Config Read */
#define CMD_CW                0x0c /* Config Write */
#define CMD_RD                0x81 /* Read Data */
#define CMD_WD                0x0e /* Write Data */
#define CMD_CT                0x10 /* Config Type */
/*
 * SPI slave status register (to master).
 */
#define SLV_ON                BIT(0)
#define SR_CFG_SUCCESS        BIT(1)
#define SR_TXRX_FIFO_RDY    BIT(2)
#define SR_RD_ERR            BIT(3)
#define SR_WR_ERR            BIT(4)
#define SR_RDWR_FINISH        BIT(5)
#define SR_TIMOUT_ERR        BIT(6)
#define SR_CMD_ERR            BIT(7)
#define CONFIG_READY  ((SR_CFG_SUCCESS | SR_TXRX_FIFO_RDY))

/*
 * hardware limit for once transfter.
 */
#define MAX_SPI_XFER_SIZE_ONCE        (64 * 1024 - 1)
#define MAX_SPI_TRY_CNT            (10)

/*
 * default never pass more than 32 bytes
 */
#define MTK_SPI_BUFSIZ    max(32, SMP_CACHE_BYTES)

#define DEFAULT_SPI_MODE_SINGLE_HALF_DUPLEX (3)
#define DEFAULT_SPI_MODE_QUAD    (2)
#define DEFAULT_SPI_MODE_DUAL    (1)
#define DEFAULT_SPI_MODE_SINGLE  (0)
#define SPI_FULL_DUPLEX_TRANSFER (80)
#define SPI_READ             true
#define SPI_WRITE             false
#define SPI_READ_STA_ERR_RET    (1)
#define DSP_SPIS1_CLKSEL_ADDR    (0x1d00e0cc)
#define SPI_FREQ_52M        (52*1000*1000)
#define SPI_FREQ_26M        (26*1000*1000)
#define SPI_FREQ_13M        (13*1000*1000)

int default_spi_mode = DEFAULT_SPI_MODE_SINGLE_HALF_DUPLEX;

/* HIFI4DSP specific SPI data */
struct mtk_hifi4dsp_spi_data {
	int spi_bus_idx;
	int reserved;
	void *spi_bus_data[2];
};

static DEFINE_MUTEX(hifi4dsp_bus_lock);
static struct mtk_hifi4dsp_spi_data hifi4dsp_spi_data;
static int hifi4dsp_spi_init_done;
static int g_config_Mode_init;

static inline void *kvzalloc(size_t size, gfp_t flags) {
	void *ret;

	ret = kzalloc(size, flags | __GFP_NOWARN);
	if (!ret)
		ret = vzalloc(size);
	return ret;
}

static int spi_reverse_to_LSB(u8 *data, int datelen, int len)
{
    int i,cnt;
    //printk("[MSPI] %s::%d len = %d\n",__func__,__LINE__,datelen);

    for (cnt = 0; cnt < datelen;cnt++) {
        //printk("[MSPI] data[0x%x] = 0x%02x\n",cnt, data[cnt]);
        u8 reverse_x =0;
        for (i = 0; i <len; i++){
            reverse_x |= (( data[cnt] >> ( ( len - 1 ) - i ) & 1 ) << i);
        }
        data[cnt] = reverse_x;
        printk("%s ===> data[%d] = 0x%02x\n", __func__,cnt,data[cnt]);
    }

}


int spi_config_MSB(void)
{
    int status= 0, retry = 0;
    u8 read_status;
    struct spi_device *spi = hifi4dsp_spi_data.spi_bus_data[0];
    u32 speed = SPI_SPEED_LOW;
    struct spi_message message;

    if (default_spi_mode == DEFAULT_SPI_MODE_DUAL || default_spi_mode == DEFAULT_SPI_MODE_SINGLE_HALF_DUPLEX) {
        struct spi_transfer x[4];
		u8 tx_cmd_type_dual[] = {CMD_CT, 0x05};
		u8 tx_cmd_type_single[] = {CMD_CT, 0x04};
        u8 tx_cmd_read_sta = CMD_RS;
        u8 rx_cmd_read_sta = 0;
        u8 cmd_config[9] = {CMD_CW, 0x00, 0x20,  0x04,  0x1d,  0x03,  0x00,  0x00,  0x00};
        printk("[MSPI]  ================DUAL mode =============\n [MSPI] Start to config MSB to slave %s :: %d\n",__FUNCTION__,__LINE__);
        u8 tx_cmd_wdata = CMD_WD;
        u8 tx_wdata[4] = {0x0C, 0x00, 0x00, 0x00};

    loop:
        spi_message_init(&message);
        memset(x, 0, sizeof(x));
		spi_reverse_to_LSB(&tx_cmd_type_single, ARRAY_SIZE(tx_cmd_type_single), 8);
		x[0].tx_buf    = tx_cmd_type_single;
		x[0].rx_buf    = NULL;
		x[0].len        = ARRAY_SIZE(tx_cmd_type_single);
		x[0].tx_nbits    = SPI_NBITS_SINGLE;
		x[0].rx_nbits    = SPI_NBITS_SINGLE;
		x[0].speed_hz    = speed;
		x[0].cs_change = 0;
		spi_message_add_tail(&x[0], &message);

        spi_reverse_to_LSB(&cmd_config,ARRAY_SIZE(cmd_config),8);
        x[1].tx_buf    = cmd_config;
        x[1].rx_buf    = NULL;
        x[1].len        = ARRAY_SIZE(cmd_config);
        x[1].tx_nbits    = SPI_NBITS_SINGLE;
        x[1].rx_nbits    = SPI_NBITS_SINGLE;
        x[1].speed_hz    = speed;
        x[1].cs_change = 0;
        spi_message_add_tail(&x[1], &message);

        spi_reverse_to_LSB(&tx_cmd_read_sta,1,8);
        x[2].tx_buf    = &tx_cmd_read_sta;
        x[2].rx_buf    = NULL;
        x[2].len        =  1;
        x[2].tx_nbits    = SPI_NBITS_SINGLE;
        x[2].rx_nbits    = SPI_NBITS_SINGLE;
        x[2].speed_hz    = speed;
        x[2].cs_change = 0;
        spi_message_add_tail(&x[2], &message);

        x[3].tx_buf    = NULL;
        x[3].rx_buf    = &rx_cmd_read_sta;
        x[3].len        =  1;
        x[3].tx_nbits    = SPI_NBITS_SINGLE;
        x[3].rx_nbits    = SPI_NBITS_SINGLE;
        x[3].speed_hz    = speed;
        x[3].cs_change = 0;
        spi_message_add_tail(&x[3], &message);

        status = spi_sync(spi, &message);
        if (status) {
            printk("[MSPI]  status = %d,%s :: %d\n",status,__FUNCTION__,__LINE__);
            goto tail;
        }

        spi_reverse_to_LSB(&rx_cmd_read_sta,1,8);
        read_status = rx_cmd_read_sta;

        if ((read_status & CONFIG_READY) != CONFIG_READY) {
            if (retry++ <= MAX_SPI_TRY_CNT) {
                pr_warn("SPI slave status error: 0x%x, retry conut= %d, line:%d\n",read_status, retry, __LINE__);
                msleep(100);
                goto loop;
            }
            else {
                pr_warn("SPI slave status error: 0x%x, line:%d\n",
                read_status, __LINE__);
                status = 1;
                goto tail;
            }
        }
        else {
            memset(x, 0, sizeof(x));
            tx_cmd_read_sta = CMD_RS;
            rx_cmd_read_sta = 0;
            spi_message_init(&message);
            //Write data
            printk("[MSPI]  Start to write MSB to slave %s :: %d\n",__FUNCTION__,__LINE__);
            spi_reverse_to_LSB(&tx_cmd_wdata,1,8);
            x[3].tx_buf    = &tx_cmd_wdata;
            x[3].rx_buf    = NULL;
            x[3].len        =  1;
            x[3].tx_nbits    = SPI_NBITS_SINGLE;
            x[3].rx_nbits    = SPI_NBITS_SINGLE;
            x[3].speed_hz    = speed;
            x[3].cs_change = 1;
            spi_message_add_tail(&x[3], &message);

			spi_reverse_to_LSB(&tx_wdata, ARRAY_SIZE(tx_wdata), 8);
			x[0].tx_buf = tx_wdata;
			x[0].rx_buf    = NULL;
			x[0].tx_nbits = SPI_NBITS_SINGLE;
			x[0].rx_nbits = SPI_NBITS_SINGLE;
			x[0].len = ARRAY_SIZE(tx_wdata);
			x[0].speed_hz = speed;
			x[0].cs_change = 0;
			spi_message_add_tail(&x[0], &message);

            /*
            * Check SPI-Slave Read Status,
            * SR_RDWR_FINISH = 1 & RD_ERR/WR_ERR = 0 ???
            */

            x[1].tx_buf    =  &tx_cmd_read_sta;
            x[1].rx_buf    = NULL;
            x[1].len        = 1;
            x[1].tx_nbits    = SPI_NBITS_SINGLE;
            x[1].rx_nbits    = SPI_NBITS_SINGLE;
            x[1].speed_hz    = speed;
            x[1].cs_change = 0;
            spi_message_add_tail(&x[1], &message);

            x[2].tx_buf    =  NULL;
            x[2].rx_buf = &rx_cmd_read_sta;
            x[2].len        = 1;
            x[2].tx_nbits    = SPI_NBITS_SINGLE;
            x[2].rx_nbits    = SPI_NBITS_SINGLE;
            x[2].speed_hz    = speed;
            x[2].cs_change = 0;
            spi_message_add_tail(&x[2], &message);
            status = spi_sync(spi, &message);
            if (status) {
                printk("[MSPI]  status = %d,%s :: %d\n",status,__FUNCTION__,__LINE__);
                goto tail;
            }

            printk("[MSPI]  %s :: %d  rx_cmd_read_sta = 0x%2x\n",__FUNCTION__,__LINE__,rx_cmd_read_sta);
            read_status = rx_cmd_read_sta;
            if (((read_status & SR_RDWR_FINISH) != SR_RDWR_FINISH)
                || ((read_status & SR_RD_ERR) == SR_RD_ERR)
                || ((read_status & SR_WR_ERR) == SR_WR_ERR)) {
                pr_warn("SPI slave status error: 0x%x, line:%d\n",read_status, __LINE__);
                status = SPI_READ_STA_ERR_RET;
            }
        }
        tail:
        if (status) {
            pr_err("config  MSB err, line(%d), ret(%d)\n",
                    __LINE__, status);
        }
        else {
             spi->mode &= ~(SPI_LSB_FIRST);
             printk("[MSPI]  spi->mode = %d,%s :: %d\n",spi->mode,__FUNCTION__,__LINE__);
        }
    }
    else {  // single
        struct spi_transfer x[3];
        u8 tx_cmd_type_single[] = {CMD_CT, 0x04};
        u8 tx_cmd_read_sta[] = {CMD_RS, 0x00};
        u8 rx_cmd_read_sta[] = {0, 0};
        u8 cmd_config[9] = {CMD_CW, 0x00, 0x20,  0x04,  0x1d,  0x03,  0x00,  0x00,  0x00};
        u8 tx_cmd_wdata[5] = {CMD_WD, 0x0C, 0x00, 0x00, 0x00};
         printk("[MSPI]  ================SINGLE mode =============\n [MSPI] Start to config MSB to slave %s :: %d\n",__FUNCTION__,__LINE__);

    loop_s:
        spi_message_init(&message);
        memset(x, 0, sizeof(x));
        spi_reverse_to_LSB(&tx_cmd_type_single,ARRAY_SIZE(tx_cmd_type_single),8);
        x[0].tx_buf    = tx_cmd_type_single;
        x[0].len        = ARRAY_SIZE(tx_cmd_type_single);;
        x[0].tx_nbits    = SPI_NBITS_SINGLE;
        x[0].rx_nbits    = SPI_NBITS_SINGLE;
        x[0].speed_hz    = speed;
        spi_message_add_tail(&x[0], &message);
        spi_reverse_to_LSB(&cmd_config,ARRAY_SIZE(cmd_config),8);
        x[1].tx_buf    = cmd_config;
        x[1].len        = ARRAY_SIZE(cmd_config);
        x[1].tx_nbits    = SPI_NBITS_SINGLE;
        x[1].rx_nbits    = SPI_NBITS_SINGLE;
        x[1].speed_hz    = speed;
        spi_message_add_tail(&x[1], &message);
        spi_reverse_to_LSB(&tx_cmd_read_sta,ARRAY_SIZE(tx_cmd_read_sta),8);
        x[2].tx_buf    = tx_cmd_read_sta;
        x[2].rx_buf    = rx_cmd_read_sta;
        x[2].len        =  ARRAY_SIZE(tx_cmd_read_sta);
        x[2].tx_nbits    = SPI_NBITS_SINGLE;
        x[2].rx_nbits    = SPI_NBITS_SINGLE;
        x[2].speed_hz    = speed;
        spi_message_add_tail(&x[2], &message);
        status = spi_sync(spi, &message);
        if (status) {
            printk("[MSPI]  status = %d,%s :: %d\n",status,__FUNCTION__,__LINE__);
            goto tail_s;
        }
        spi_reverse_to_LSB(&rx_cmd_read_sta,ARRAY_SIZE(rx_cmd_read_sta),8);
        read_status = rx_cmd_read_sta[1];
        if ((read_status & CONFIG_READY) != CONFIG_READY) {
            if (retry++ <= MAX_SPI_TRY_CNT) {
                pr_warn("SPI slave status error: 0x%x, retry conut= %d, line:%d\n", read_status, retry, __LINE__);
                msleep(100);
                goto loop_s;
            }
            else {
                pr_warn("SPI slave status error: 0x%x, line:%d\n",
                read_status, __LINE__);
                status = 1;
                goto tail_s;
            }
        }
        else {
            memset(x, 0, sizeof(x));
            spi_message_init(&message);
            //Write data
            printk("[MSPI]  Start to write MSB to slave %s :: %d\n",__FUNCTION__,__LINE__);
            spi_reverse_to_LSB(&tx_cmd_wdata,ARRAY_SIZE(tx_cmd_wdata),8);
            x[0].tx_buf =tx_cmd_wdata;
            x[0].tx_nbits = SPI_NBITS_SINGLE;
            x[0].rx_nbits = SPI_NBITS_SINGLE;
            x[0].len = ARRAY_SIZE(tx_cmd_wdata);;
            x[0].speed_hz = speed;
            spi_message_add_tail(&x[0], &message);
            /*
            * Check SPI-Slave Read Status,
            * SR_RDWR_FINISH = 1 & RD_ERR/WR_ERR = 0 ???
            */
            tx_cmd_read_sta[0] = CMD_RS;
            tx_cmd_read_sta[1] = 0x00;
            rx_cmd_read_sta[0] = 0x00;
            rx_cmd_read_sta[1] = 0x00;
            x[1].tx_buf    =  tx_cmd_read_sta;
            x[1].rx_buf = rx_cmd_read_sta;
            x[1].len        = ARRAY_SIZE(tx_cmd_read_sta);
            x[1].tx_nbits    = SPI_NBITS_SINGLE;
            x[1].rx_nbits    = SPI_NBITS_SINGLE;
            x[1].speed_hz    = speed;
            spi_message_add_tail(&x[1], &message);
            status = spi_sync(spi, &message);
            if (status) {
                printk("[MSPI]  status = %d,%s :: %d\n",status,__FUNCTION__,__LINE__);
                goto tail_s;
            }
            read_status = rx_cmd_read_sta[1];
            if (((read_status & SR_RDWR_FINISH) != SR_RDWR_FINISH)
                || ((read_status & SR_RD_ERR) == SR_RD_ERR)
                || ((read_status & SR_WR_ERR) == SR_WR_ERR)) {
                pr_warn("SPI slave status error: 0x%x, line:%d\n",read_status, __LINE__);
                status = SPI_READ_STA_ERR_RET;
            }
        }
        tail_s:
        if (status) {
            pr_err("config  MSB err, line(%d), ret(%d)\n",
                    __LINE__, status);
        }
            else {
                 spi->mode &= ~(SPI_LSB_FIRST);
                 printk("[MSPI]  spi->mode = %d,%s :: %d\n",spi->mode,__FUNCTION__,__LINE__);
            }
    }

    return status;
}

static int spi_config_type_wr(struct spi_device *spi, int type, u32 addr,
                          int len, bool wr, u32 speed)
{
    int status, i, try = 0;
    u8 tx_cmd_type_single[] = {CMD_CT, 0x04};
    u8 tx_cmd_type_dual[]   = {CMD_CT, 0x05};
    u8 tx_cmd_type_quad[]   = {CMD_CT, 0x06};
    u8 cmd_config[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,};
    u8 read_status;
    void *buffer;
    struct spi_message message;

    if (len > SPI_FULL_DUPLEX_TRANSFER) {
        u8 tx_cmd_read_sta = CMD_RS;
        u8 rx_cmd_read_sta = 0;
        struct spi_transfer x[4];
    loop:
        spi_message_init(&message);
        memset(x, 0, sizeof(x));
		if (type == DEFAULT_SPI_MODE_QUAD) {
            buffer = tx_cmd_type_quad;
		} else if (type == DEFAULT_SPI_MODE_DUAL) {
            buffer = tx_cmd_type_dual;
		} else if (type == DEFAULT_SPI_MODE_SINGLE || type == DEFAULT_SPI_MODE_SINGLE_HALF_DUPLEX) {
            buffer = tx_cmd_type_single;
        } else {
            status = -EINVAL;
    		pr_notice("Input wrong type!\n");
            goto tail;
        }

		if (!g_config_Mode_init) {
			x[0].tx_buf    = buffer;
			x[0].rx_buf    = NULL;
			x[0].len        = ARRAY_SIZE(tx_cmd_type_single);
			x[0].tx_nbits    = SPI_NBITS_SINGLE;
			x[0].rx_nbits    = SPI_NBITS_SINGLE;
			x[0].speed_hz    = speed;
			x[0].cs_change = 0;
			spi_message_add_tail(&x[0], &message);
			g_config_Mode_init = 1;
		}

        if (wr)
            cmd_config[0] = CMD_CR;
        else
            cmd_config[0] = CMD_CW;

        for (i = 0; i < 4; i++) {
            cmd_config[1 + i] = (addr & (0xff << (i * 8))) >> (i * 8);
            cmd_config[5 + i] = ((len - 1) & (0xff << (i * 8))) >> (i * 8);
        }
        /*
            {
            printk("[MSPI] cmd = 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x\n",
            cmd_config[0],cmd_config[1],cmd_config[2],cmd_config[3],cmd_config[4],cmd_config[5],
            cmd_config[6],cmd_config[7],cmd_config[8]);
            }
        */

        x[1].tx_buf    = cmd_config;
        x[1].rx_buf    = NULL;
        x[1].len        = ARRAY_SIZE(cmd_config);
        x[1].tx_nbits    = SPI_NBITS_SINGLE;
        x[1].rx_nbits    = SPI_NBITS_SINGLE;
        x[1].speed_hz    = speed;
        x[1].cs_change = 0;
        spi_message_add_tail(&x[1], &message);

        x[2].tx_buf    = &tx_cmd_read_sta;
        x[2].rx_buf    = NULL;
        x[2].len        = 1;
        x[2].tx_nbits    = SPI_NBITS_SINGLE;
        x[2].rx_nbits    = SPI_NBITS_SINGLE;
        x[2].speed_hz    = speed;
        x[2].cs_change = 0;
        spi_message_add_tail(&x[2], &message);

        x[3].tx_buf    = NULL;;
        x[3].rx_buf    = &rx_cmd_read_sta;
        x[3].len        = 1;
        x[3].tx_nbits    = SPI_NBITS_SINGLE;
        x[3].rx_nbits    = SPI_NBITS_SINGLE;
        x[3].speed_hz    = speed;
        x[3].cs_change = 0;
        spi_message_add_tail(&x[3], &message);

        status = spi_sync(spi, &message);
    	if (status)
            goto tail;
        //printk("[MSPI]  %s :: %d  rx_cmd_read_sta[0] = 0x%2x,rx_cmd_read_sta[1] = 0x%2x,\n",__FUNCTION__,__LINE__,rx_cmd_read_sta[0],rx_cmd_read_sta[1]);
        read_status = rx_cmd_read_sta;
        if ((read_status & CONFIG_READY) != CONFIG_READY) {
			printk_ratelimited("SPI slave status error: 0x%x, line:%d\n",
                        read_status, __LINE__);
			printk_ratelimited("[MSPI] cmd = 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x\n",
            cmd_config[0],cmd_config[1],cmd_config[2],cmd_config[3],cmd_config[4],cmd_config[5],
            cmd_config[6],cmd_config[7],cmd_config[8]);

            if (try++ <= MAX_SPI_TRY_CNT)
                goto loop;
        }
    tail:
        if (status) {
			printk_ratelimited("config type & addr & len err, line(%d), type(%d), ret(%d)\n",
                    __LINE__, type, status);
        }
    }
    else {
        u8 tx_cmd_read_sta[2] = {CMD_RS, 0x00};
        u8 rx_cmd_read_sta[2] = {0, 0};
        struct spi_transfer x[3];
    loop_s:
        spi_message_init(&message);
        memset(x, 0, sizeof(x));
        memset(rx_cmd_read_sta, 0, ARRAY_SIZE(rx_cmd_read_sta));

		if (type == DEFAULT_SPI_MODE_QUAD) {
            buffer = tx_cmd_type_quad;
		} else if (type == DEFAULT_SPI_MODE_DUAL) {
            buffer = tx_cmd_type_dual;
		} else if (type == DEFAULT_SPI_MODE_SINGLE || type == DEFAULT_SPI_MODE_SINGLE_HALF_DUPLEX) {
            buffer = tx_cmd_type_single;
        } else {
            status = -EINVAL;
    		pr_notice("Input wrong type!\n");
            goto tail;
        }

		if (!g_config_Mode_init) {
			x[0].tx_buf    = buffer;
			x[0].len        = ARRAY_SIZE(tx_cmd_type_single);
			x[0].tx_nbits    = SPI_NBITS_SINGLE;
			x[0].rx_nbits    = SPI_NBITS_SINGLE;
			x[0].speed_hz    = speed;
			spi_message_add_tail(&x[0], &message);
			g_config_Mode_init = 1;
		}

        if (wr)
            cmd_config[0] = CMD_CR;
        else
            cmd_config[0] = CMD_CW;

        for (i = 0; i < 4; i++) {
            cmd_config[1 + i] = (addr & (0xff << (i * 8))) >> (i * 8);
            cmd_config[5 + i] = ((len - 1) & (0xff << (i * 8))) >> (i * 8);
        }
        /*---------------------
        printk("[MSPI] cmd = 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x , 0x%02x\n",
            cmd_config[0],cmd_config[1],cmd_config[2],cmd_config[3],cmd_config[4],cmd_config[5],
            cmd_config[6],cmd_config[7],cmd_config[8]);
        //---------------------*/

        x[1].tx_buf    = cmd_config;
        x[1].len        = ARRAY_SIZE(cmd_config);
        x[1].tx_nbits    = SPI_NBITS_SINGLE;
        x[1].rx_nbits    = SPI_NBITS_SINGLE;
        x[1].speed_hz    = speed;
        spi_message_add_tail(&x[1], &message);

        x[2].tx_buf    = tx_cmd_read_sta;
        x[2].rx_buf    = rx_cmd_read_sta;
        x[2].len        = ARRAY_SIZE(tx_cmd_read_sta);
        x[2].tx_nbits    = SPI_NBITS_SINGLE;
        x[2].rx_nbits    = SPI_NBITS_SINGLE;
        x[2].speed_hz    = speed;
        spi_message_add_tail(&x[2], &message);

        status = spi_sync(spi, &message);
    	if (status)
            goto tail_s;

		read_status = rx_cmd_read_sta[1];
		if ((read_status & CONFIG_READY) != CONFIG_READY) {
			printk_ratelimited("SPI slave status error: 0x%x, line:%d\n",
					read_status, __LINE__);
			if (try++ <= MAX_SPI_TRY_CNT)
				goto loop_s;
			else
				status = SPI_READ_STA_ERR_RET;
		}
    tail_s:
        if (status) {
			printk_ratelimited("config type & addr & len err, line(%d), type(%d), ret(%d)\n",
                    __LINE__, type, status);
        }
    }
    return status;
}

static int spi_trigger_wr_data(struct spi_device *spi,
            int type, int len, bool wr, void *buf_store, u32 speed)
{
    int status;
    struct spi_message msg;
    void *local_buf = NULL;
    u8 mtk_spi_buffer[MTK_SPI_BUFSIZ];
    u8 read_status;
    u8 tx_cmd_write_sta[2] = {CMD_WS, 0x01};
    u8 rx_cmd_write_sta[2] = {0, 0};

    if (len > SPI_FULL_DUPLEX_TRANSFER) {
        struct spi_transfer x[6];
        u8 tx_cmd_read_sta = CMD_RS;
        u8 rx_cmd_read_sta = 0;
        u8 tx_cmd_rd = CMD_RD;
        u8 tx_cmd_wd = CMD_WD;

        memset(x, 0, sizeof(x));
        if (!buf_store) {
            status = -EINVAL;
            goto tail;
        }

        spi_message_init(&msg);

        if (len > MTK_SPI_BUFSIZ) {
            local_buf = kvzalloc(len, GFP_KERNEL);
            if (!local_buf) {
                pr_notice("tx/rx kvmalloc %zu fail, line:%d\n", len, __LINE__);
                status = -ENOMEM;
                goto tail;
            }
        } else {
            local_buf = mtk_spi_buffer;
            memset(local_buf, 0, MTK_SPI_BUFSIZ);
        }

        if (wr) {
           x[0].tx_buf =&tx_cmd_rd;
           x[0].rx_buf =  NULL;
        } else {;
            x[0].tx_buf =&tx_cmd_wd;
            x[0].rx_buf =  NULL;
        }
        x[0].tx_nbits = SPI_NBITS_SINGLE;
        x[0].rx_nbits = SPI_NBITS_SINGLE;
        x[0].len = 1;
        x[0].speed_hz = speed;
        x[0].cs_change = 1;
        spi_message_add_tail(&x[0], &msg);

        if (wr) {
            x[1].tx_buf = NULL;
            x[1].rx_buf = local_buf;
        } else {
            x[1].tx_buf = buf_store;
            x[1].rx_buf = NULL;
        }
        x[1].tx_nbits = SPI_NBITS_SINGLE;
        x[1].rx_nbits = SPI_NBITS_SINGLE;
        if (type == DEFAULT_SPI_MODE_DUAL) {
            x[1].tx_nbits = SPI_NBITS_DUAL;
            x[1].rx_nbits = SPI_NBITS_DUAL;
        } else if (type == DEFAULT_SPI_MODE_QUAD) {
            x[1].tx_nbits = SPI_NBITS_QUAD;
            x[1].rx_nbits = SPI_NBITS_QUAD;
        }
        x[1].len = len;
        x[1].speed_hz = speed;
        x[1].cs_change = 0;
        spi_message_add_tail(&x[1], &msg);
        /*
         * Check SPI-Slave Read Status,
         * SR_RDWR_FINISH = 1 & RD_ERR/WR_ERR = 0 ???
         */
        tx_cmd_read_sta = CMD_RS;
        rx_cmd_read_sta = 0;
        x[2].tx_buf    = &tx_cmd_read_sta;
        x[2].rx_buf = NULL;
        x[2].len        = 1;
        x[2].tx_nbits    = SPI_NBITS_SINGLE;
        x[2].rx_nbits    = SPI_NBITS_SINGLE;
        x[2].speed_hz    = speed;
        x[2].cs_change = 0;
        spi_message_add_tail(&x[2], &msg);

        x[3].tx_buf    = NULL;
        x[3].rx_buf = &rx_cmd_read_sta;
        x[3].len        = 1;
        x[3].tx_nbits    = SPI_NBITS_SINGLE;
        x[3].rx_nbits    = SPI_NBITS_SINGLE;
        x[3].speed_hz    = speed;
        x[3].cs_change = 0;
        spi_message_add_tail(&x[3], &msg);
        status = spi_sync(spi, &msg);

        if (status) {
			printk_ratelimited("write/read to slave err, line(%d), len(%d), ret(%d)\n",
                    __LINE__, len, status);
            goto tail;
    	    }
        read_status = rx_cmd_read_sta;
        if (((read_status & SR_RDWR_FINISH) != SR_RDWR_FINISH)
            || ((read_status & SR_RD_ERR) == SR_RD_ERR)
            || ((read_status & SR_WR_ERR) == SR_WR_ERR)) {
			printk_ratelimited("SPI slave status error: 0x%x, line:%d\n",
                    read_status, __LINE__);
            x[4].tx_buf    = tx_cmd_write_sta;
            x[4].rx_buf = NULL;
            x[4].len        = ARRAY_SIZE(tx_cmd_write_sta);
            x[4].tx_nbits    = SPI_NBITS_SINGLE;
            x[4].rx_nbits    = SPI_NBITS_SINGLE;
            x[4].speed_hz    = speed;
            x[4].cs_change = 0;
            spi_message_init(&msg);
            spi_message_add_tail(&x[4], &msg);
            x[5].tx_buf    = NULL;
            x[5].rx_buf = rx_cmd_write_sta;
            x[5].len        = ARRAY_SIZE(rx_cmd_write_sta);
            x[5].tx_nbits    = SPI_NBITS_SINGLE;
            x[5].rx_nbits    = SPI_NBITS_SINGLE;
            x[5].speed_hz    = speed;
            x[5].cs_change = 0;
            spi_message_add_tail(&x[5], &msg);
            status = spi_sync(spi, &msg);
    		if (status)
            goto tail;

            status = SPI_READ_STA_ERR_RET;
        }
        tail:
        /* Only for successful read */
        if (wr && !status)
            memcpy(buf_store, ((u8 *)x[1].rx_buf ), len);
    }
   else {
        size_t size;
        u8 tx_cmd_read_sta[2] = {CMD_RS, 0x00};
        u8 rx_cmd_read_sta[2] = {0, 0};
        struct spi_transfer x[3];
        memset(x, 0, sizeof(x));
        if (!buf_store) {
            status = -EINVAL;
            goto tail_s;
        }

        size = len + 1;
        if (size > MTK_SPI_BUFSIZ) {
            local_buf = kvzalloc(size, GFP_KERNEL);
            if (!local_buf) {
                pr_notice("tx/rx kvmalloc %zu fail, line:%d\n", size, __LINE__);
                status = -ENOMEM;
                goto tail_s;
            }
        } else {
            local_buf = mtk_spi_buffer;
            memset(local_buf, 0, MTK_SPI_BUFSIZ);
        }

        x[0].tx_nbits = SPI_NBITS_SINGLE;
        x[0].rx_nbits = SPI_NBITS_SINGLE;
        if (type == 1) {
            x[0].tx_nbits = SPI_NBITS_DUAL;
            x[0].rx_nbits = SPI_NBITS_DUAL;
        } else if (type == 2) {
            x[0].tx_nbits = SPI_NBITS_QUAD;
            x[0].rx_nbits = SPI_NBITS_QUAD;
        }
        x[0].len = size;
        x[0].speed_hz = speed;
        if (wr) {
            *((u8 *)local_buf) = CMD_RD;
            x[0].tx_buf = local_buf;
            x[0].rx_buf = local_buf;
        } else {
            *((u8 *)local_buf) = CMD_WD;
            memcpy((u8 *)local_buf + 1, buf_store, len);
            x[0].tx_buf = local_buf;
        }
        spi_message_init(&msg);
        spi_message_add_tail(&x[0], &msg);

        /*
         * Check SPI-Slave Read Status,
         * SR_RDWR_FINISH = 1 & RD_ERR/WR_ERR = 0 ???
         */
        memset(rx_cmd_read_sta, 0, ARRAY_SIZE(rx_cmd_read_sta));
        x[1].tx_buf    = tx_cmd_read_sta;
        x[1].rx_buf = rx_cmd_read_sta;
        x[1].len        = ARRAY_SIZE(tx_cmd_read_sta);
        x[1].tx_nbits    = SPI_NBITS_SINGLE;
        x[1].rx_nbits    = SPI_NBITS_SINGLE;
        x[1].speed_hz    = speed;
        spi_message_add_tail(&x[1], &msg);
        status = spi_sync(spi, &msg);
    	if (status)
            goto tail_s;

        read_status = rx_cmd_read_sta[1];
        if (((read_status & SR_RDWR_FINISH) != SR_RDWR_FINISH)
            || ((read_status & SR_RD_ERR) == SR_RD_ERR)
            || ((read_status & SR_WR_ERR) == SR_WR_ERR)) {
    		pr_notice("SPI slave status error: 0x%x, line:%d\n",
                    read_status, __LINE__);
            x[2].tx_buf    = tx_cmd_write_sta;
            x[2].rx_buf = rx_cmd_write_sta;
            x[2].len        = ARRAY_SIZE(tx_cmd_write_sta);
            x[2].tx_nbits    = SPI_NBITS_SINGLE;
            x[2].rx_nbits    = SPI_NBITS_SINGLE;
            x[2].speed_hz    = speed;
            spi_message_init(&msg);
            spi_message_add_tail(&x[2], &msg);
            status = spi_sync(spi, &msg);
    		if (status)
            goto tail_s;

            status = SPI_READ_STA_ERR_RET;
        }
    tail_s:
        /* Only for successful read */
        if (wr && !status)
            memcpy(buf_store, ((u8 *)x[0].rx_buf + 1), len);
    }

    if (local_buf != mtk_spi_buffer)
        kvfree(local_buf);

    if (status)
		pr_notice("write/read to slave err, line(%d), len(%d), ret(%d)\n",
                __LINE__, len, status);

    return status;
}

#ifdef CONFIG_MTK_HIFI4DSP_CHECK_DSP_DVFS
static int dsp_check_spis1_clk(u32 *value, u32 speed)
{
    int ret, try = 0, len = 4;
    int type = default_spi_mode;
    struct spi_device *spi = hifi4dsp_data.spi_bus_data[0];
    u32 addr = DSP_SPIS1_CLKSEL_ADDR;

    pr_debug("%s addr = 0x%08x, len = %d\n", __func__, addr, len);

    mutex_lock(&hifi4dsp_bus_lock);

spis_check_config_read:
    ret = spi_config_type_wr(spi, type, addr, len, SPI_READ, speed);
    if (ret < 0) {
        pr_debug("SPI config read fail!, line:%d\n", __LINE__);
        goto tail;
    }
    ret = spi_trigger_wr_data(spi, type, len, SPI_READ, (u8 *)value, speed);
    if (ret < 0) {
        pr_debug("SPI read data error!, line:%d\n", __LINE__);
        goto tail;
    }
    if (ret > 0) {
        if (try++ < MAX_SPI_TRY_CNT)
            goto spis_check_config_read;
        else
            pr_debug("SPI read failed, retry count > %d, line:%d\n",
                MAX_SPI_TRY_CNT, __LINE__);
    }

tail:
    mutex_unlock(&hifi4dsp_bus_lock);
    return ret;
}

static int spi_select_speed(u32 spis_clk_sel, int target_speed,
                int *selected_speed) {
    if (spis_clk_sel & (0x7 << 17)) {
        if (target_speed > SPI_FREQ_52M) {
            pr_debug("Availabel max spi speed is 52MHz!\n");
            *selected_speed = SPI_FREQ_52M;
        } else
            *selected_speed = target_speed;
        return 0;
    } else if (spis_clk_sel & (0x1 << 20)) {
        if (target_speed > SPI_FREQ_26M) {
            pr_debug("Availabel max spi speed is 26MHz!\n");
            *selected_speed = SPI_FREQ_26M;
        } else
            *selected_speed = target_speed;
        return 0;
    } else if (spis_clk_sel & (0xe1 << 16)) {
        if (target_speed > SPI_FREQ_13M) {
            pr_debug("Availabel max spi speed is 13MHz!\n");
            *selected_speed = SPI_FREQ_13M;
        } else
            *selected_speed = target_speed;
        return 0;
    }

    return -1;
}
#endif

int dsp_spi_write(u32 addr, void *value, int len, u32 speed)
{
    int ret, try = 0, xfer_speed;
    int type = default_spi_mode;
    struct spi_device *spi = hifi4dsp_spi_data.spi_bus_data[0];
    void *tx_store;

#ifdef CONFIG_MTK_HIFI4DSP_CHECK_DSP_DVFS
    u32 dsp_spis1_clksel_reg;
#endif

	pr_debug("%s addr = 0x%08x, len = %d\n", __func__, addr, len);
	xfer_speed = speed;

#ifdef CONFIG_MTK_HIFI4DSP_CHECK_DSP_DVFS
    if (speed > SPI_FREQ_13M) {
        ret = dsp_check_spis1_clk(&dsp_spis1_clksel_reg, SPI_SPEED_LOW);
        if (ret < 0) {
            pr_debug("SPI check dsp spis1 failed! line:%d\n",
                 __LINE__);
            return ret;
        }

        ret = spi_select_speed(dsp_spis1_clksel_reg, speed,
                       &xfer_speed);
        if (ret < 0) {
            pr_debug("DSP SPIs clk err! line:%d\n", __LINE__);
            return ret;
        }
    }
#endif
    mutex_lock(&hifi4dsp_bus_lock);

spi_config_write:
    ret = spi_config_type_wr(spi, type, addr, len, SPI_WRITE, xfer_speed);
    if (ret != 0) {
        pr_info("SPI config write fail! line:%d\n", __LINE__);
        goto tail;
    }
    tx_store = value;
    ret = spi_trigger_wr_data(spi, type, len, SPI_WRITE, tx_store,
                  xfer_speed);
    if (ret != 0) {
        pr_info("SPI write data error! line:%d\n", __LINE__);
        goto tail;
    }
    if (ret > 0) {
		if (try++ < MAX_SPI_TRY_CNT)
			goto spi_config_write;
		else
            pr_info("SPI write fail, retry count > %d, line:%d\n",
                 MAX_SPI_TRY_CNT, __LINE__);
    }

tail:
	mutex_unlock(&hifi4dsp_bus_lock);
	return ret;
}

int dsp_spi_write_ex(u32 addr, void *value, int len, u32 speed)
{
    int ret = 0;
    int res_len;
    int once_len;
    int loop;
    int cycle;
    u32 new_addr;
    u8 *new_buf;

    once_len = MAX_SPI_XFER_SIZE_ONCE;
    cycle = len / once_len;
    res_len = len % once_len;

    for (loop = 0; loop < cycle; loop++) {
        new_addr = addr + once_len * loop;
        new_buf = (u8 *)value + once_len * loop;
        ret = dsp_spi_write(new_addr, new_buf, once_len, speed);
        if (ret) {
            printk("dsp_spi_write() fail! line:%d\n", __LINE__);
            return ret;
            }
    }

    if (res_len) {
        new_addr = addr + once_len * loop;
        new_buf = (u8 *)value + once_len * loop;
        ret = dsp_spi_write(new_addr, new_buf, res_len, speed);
        if (ret) {
            printk("dsp_spi_write() fail! line:%d\n", __LINE__);
            return ret;
            }
    }

    return ret;
}

int dsp_spi_read(u32 addr, void *value, int len, u32 speed)
{
    int ret, try = 0, xfer_speed;
    int type = default_spi_mode;
    struct spi_device *spi = hifi4dsp_spi_data.spi_bus_data[0];

#ifdef CONFIG_MTK_HIFI4DSP_CHECK_DSP_DVFS
    u32 dsp_spis1_clksel_reg;
#endif

	pr_debug("%s addr = 0x%08x, len = %d\n", __func__, addr, len);
	xfer_speed = speed;

#ifdef CONFIG_MTK_HIFI4DSP_CHECK_DSP_DVFS
    if (speed > SPI_FREQ_13M) {
        ret = dsp_check_spis1_clk(&dsp_spis1_clksel_reg, SPI_SPEED_LOW);
        if (ret < 0) {
            pr_debug("SPI check dsp spis1 failed! line:%d\n",
                 __LINE__);
            return ret;
        }

        ret = spi_select_speed(dsp_spis1_clksel_reg, speed,
                       &xfer_speed);
        if (ret < 0) {
            pr_debug("DSP SPIs clk err! line:%d\n", __LINE__);
            return ret;
        }
    }
#endif
    mutex_lock(&hifi4dsp_bus_lock);

spi_config_read:
    ret = spi_config_type_wr(spi, type, addr, len, SPI_READ, xfer_speed);
    if (ret != 0) {
        pr_debug("SPI config write fail! line:%d\n", __LINE__);
        goto tail;
    }

    ret = spi_trigger_wr_data(spi, type, len, SPI_READ, value, xfer_speed);
    if (ret != 0) {
        pr_debug("SPI read data error! line:%d\n", __LINE__);
        goto tail;
    }
    if (ret > 0) {
		if (try++ < MAX_SPI_TRY_CNT)
			goto spi_config_read;
		else
            pr_debug("SPI read fail, retry count > %d, line:%d\n",
                 MAX_SPI_TRY_CNT, __LINE__);
    }

tail:
    mutex_unlock(&hifi4dsp_bus_lock);
    //printk("[MSPI] %s addr = 0x%08x, len = %d  done\n", __func__, addr, len);
    return ret;
}

int dsp_spi_read_ex(u32 addr, void *value, int len, u32 speed)
{
    int ret = 0;
    int res_len;
    int once_len;
    int loop;
    int cycle;
    u32 new_addr;
    u8 *new_buf;

    once_len = MAX_SPI_XFER_SIZE_ONCE;
    cycle = len / once_len;
    res_len = len % once_len;

    for (loop = 0; loop < cycle; loop++) {
        new_addr = addr + once_len * loop;
        new_buf = (u8 *)value + once_len * loop;
        ret = dsp_spi_read(new_addr, new_buf, once_len, speed);
        if (ret)
            pr_debug("dsp_spi_read() fail! line:%d\n", __LINE__);
    }

    if (res_len) {
        new_addr = addr + once_len * loop;
        new_buf = (u8 *)value + once_len * loop;
        ret = dsp_spi_read(new_addr, new_buf, res_len, speed);
        if (ret)
            pr_debug("dsp_spi_read() fail! line:%d\n", __LINE__);
    }

    return ret;
}

int spi_read_register(u32 addr, u32 *val, u32 speed)
{
    return dsp_spi_read(addr, (u8 *)val, 4, speed);
}

int spi_write_register(u32 addr, u32 val, u32 speed)
{
    return dsp_spi_write(addr, (u8 *)&val, 4, speed);
}

int spi_set_register32(u32 addr, u32 val, u32 speed)
{
    u32 read_val;

    spi_read_register(addr, &read_val, speed);
    spi_write_register(addr, read_val | val, speed);
    return 0;
}

int spi_clr_register32(u32 addr, u32 val, u32 speed)
{
    u32 read_val;

    spi_read_register(addr, &read_val, speed);
    spi_write_register(addr, read_val & (~val), speed);
    return 0;
}

int spi_write_register_mask(u32 addr, u32 val, u32 msk, u32 speed)
{
	u32 read_val;

	spi_read_register(addr, &read_val, speed);
	spi_write_register(addr, ((read_val & (~(msk))) | ((val) & (msk))),
						speed);
	return 0;
}

#define DSP_ADDR 0x1fc00000
int spi_multipin_loopback_transfer(int len, int xfer_speed)
{
    int ret = 0;
    void *tx_buf;
    void *rx_buf;
    int i, err = 0;

    tx_buf = kzalloc(len, GFP_KERNEL);
    rx_buf = kzalloc(len, GFP_KERNEL);

    for (i = 0; i < len; i++)
        *((char *)tx_buf + i) = i%255;
    memset(rx_buf, 0, len);

    if (xfer_speed == 13) {
        ret = dsp_spi_write_ex(DSP_ADDR, tx_buf, len, SPI_SPEED_LOW);
        if (ret < 0) {
            pr_err("Write transfer err,line(%d):%d\n", __LINE__,
                 ret);
            goto tail;
        }
        ret = dsp_spi_read_ex(DSP_ADDR, rx_buf, len, SPI_SPEED_LOW);
        if (ret < 0) {
            pr_err("Read transfer err,line(%d):%d\n", __LINE__,
                 ret);
            goto tail;
        }
    } else if (xfer_speed == 52) {
        dsp_spi_write_ex(DSP_ADDR, tx_buf, len, SPI_SPEED_HIGH);
        if (ret < 0) {
            pr_err("Write transfer err,line(%d):%d\n", __LINE__,
                 ret);
            goto tail;
        }
        dsp_spi_read_ex(DSP_ADDR, rx_buf, len, SPI_SPEED_HIGH);
        if (ret < 0) {
            pr_err("Read transfer err,line(%d):%d\n", __LINE__,
                 ret);
            goto tail;
        }
    } else {
        pr_err("Unavailabel speed!\n");
        goto tail;
    }

    for (i = 0; i < len; i++) {
        if (*((char *)tx_buf+i) != *((char *)rx_buf+i)) {
            pr_err("tx[%d]:0x%x, rx[%d]:0x%x\r\n",
                i, *((char *)tx_buf+i), i,
                *((char *)rx_buf + i));
            err++;
        }
    }

    pr_err("total length %d bytes, err %d bytes.\n", len, err);

tail:
    kfree(tx_buf);
    kfree(rx_buf);

    if (ret < 0)
        return ret;

    return err;
}

static ssize_t hifi4dsp_spi_store(struct device *dev,
             struct device_attribute *attr,
             const char *buf, size_t count)
{
    int len, xfer_speed, ret;

	default_spi_mode = DEFAULT_SPI_MODE_SINGLE_HALF_DUPLEX;

    if (!strncmp(buf, "xfer", 4)) {
        buf += 5;
        if (!strncmp(buf, "speed=", 6) &&
            (sscanf(buf + 6, "%d", &xfer_speed) == 1)) {
            buf += 9;
            if (!strncmp(buf, "len=", 4) &&
                (sscanf(buf + 4, "%d", &len) == 1))
                ret = spi_multipin_loopback_transfer(len,
                                xfer_speed);
        }
    }

    return count;
}

static DEVICE_ATTR(hifi4dsp_spi, 0200, NULL, hifi4dsp_spi_store);

static struct device_attribute *spi_attribute[] = {
    &dev_attr_hifi4dsp_spi,
};

static void spi_create_attribute(struct device *dev)
{
    int size, idx;

    size = ARRAY_SIZE(spi_attribute);
    for (idx = 0; idx < size; idx++)
        device_create_file(dev, spi_attribute[idx]);
}

int hifi4dsp_spi_get_status(void)
{
	int ret = 1;

	if (!hifi4dsp_spi_init_done)
		goto tail;

	ret = spi_write_register(0x1D00DA04, 0x1800, SPI_SPEED_LOW);

tail:
	return !ret;
}

void hifi4dsp_spi_set_config_mode_status(int status)
{
	g_config_Mode_init = status;
}

static int hifi4dsp_spi_probe(struct spi_device *spi)
{
    int err = 0;
    static struct task_struct *dsp_task;
    struct mtk_chip_config *data;
    struct mtk_hifi4dsp_spi_data *pri_data = &hifi4dsp_spi_data;

    if (strstr(saved_command_line, "farfield.dsp.name=mt8570") == NULL) {
        pr_info("mt8570 is not supported\n");
        return -EINVAL;
    }

    u32 spi_cfg = 0;

	if (strstr(saved_command_line, "SPI_MODE=2")) {
		default_spi_mode = DEFAULT_SPI_MODE_DUAL;
		printk ("default_spi_mode =dual mode\n");
	} else if (strstr(saved_command_line, "SPI_MODE=1")) {
		default_spi_mode = DEFAULT_SPI_MODE_SINGLE;
		printk ("default_spi_mode =single mode & full duplex\n");
	} else {
		default_spi_mode = DEFAULT_SPI_MODE_SINGLE_HALF_DUPLEX;
		printk ("default_spi_mode =single mode & half duplex\n");
	}

    pr_info("%s() enter.\n", __func__);

    data = kzalloc(sizeof(struct mtk_chip_config), GFP_KERNEL);
    if (!data) {
        err = -ENOMEM;
        goto tail;
    }

    /*
     * Structure filled with mtk-spi crtical values.
     */
    spi->bits_per_word = 8;
    data->rx_mlsb = 0;
    data->tx_mlsb = 0;
    data->command_cnt = 1;
    data->dummy_cnt = 0;
    spi->controller_data = (void *)data;
    spi->mode=SPI_MODE_0|SPI_LSB_FIRST|SPI_RX_DUAL|SPI_TX_DUAL;

	/* Fill  structure mtk_hifi4dsp_spi_data */
    pri_data->spi_bus_data[pri_data->spi_bus_idx++] = spi;
    dsp_task = NULL;

#if 0
    /*
     * Always start the kthread because there may be much seconds
     * for loading HIFI4DSP binary
     * and booting up to FreeRTOS kernel shell.
     */
    dsp_task = kthread_run(breed_hifi4dsp, &(spi->dev), "breed_hifi4dsp");
    if (IS_ERR(dsp_task)) {
        pr_info("Couldn't create kthread for breed_hifi4dsp.\n");
        err = PTR_ERR(dsp_task);
        goto tail;
    } else
        printk("Start to run kthread [breed_hifi4dsp].\n");
#endif
    spi_create_attribute(&spi->dev);
	g_config_Mode_init = 0;

    hifi4dsp_spi_init_done = 1;
tail:
	return err;
}

static int hifi4dsp_spi_remove(struct spi_device *spi)
{
    pr_info("%s().\n", __func__);

    if (spi && spi->controller_data)
        kfree(spi->controller_data);

    return 0;
}

static const struct spi_device_id hifi4dsp_spi_ids[] = {
	{ "mt8570" },
	{}
};
MODULE_DEVICE_TABLE(spi, hifi4dsp_spi_ids);

static const struct of_device_id hifi4dsp_spi_of_ids[] = {
	{ .compatible = "mediatek,hifi4dsp-spi" },
	{}
};
MODULE_DEVICE_TABLE(of, hifi4dsp_spi_of_ids);

static struct spi_driver hifi4dsp_spi_drv = {
	.driver = {
		.name	= HIFI4DSP_SPI_DRV_NAME,
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
		.of_match_table = hifi4dsp_spi_of_ids,
	},
	.id_table	= hifi4dsp_spi_ids,
	.probe	= hifi4dsp_spi_probe,
	.remove	= hifi4dsp_spi_remove,
};

module_spi_driver(hifi4dsp_spi_drv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dehui Sun <dehui.sun@mediatek.com>");
MODULE_DESCRIPTION("SPI driver for hifi4dsp chip");

