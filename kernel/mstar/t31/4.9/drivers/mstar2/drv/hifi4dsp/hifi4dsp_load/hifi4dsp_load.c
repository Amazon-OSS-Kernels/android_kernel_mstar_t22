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
#define HIFI4DSP_LOAD_DRV_NAME "hifi4dsp-load"
#define pr_fmt(fmt) HIFI4DSP_LOAD_DRV_NAME ": " fmt

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/firmware.h>
#include <linux/freezer.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <hifi4dsp_load/hifi4dsp_load.h>
#include <hifi4dsp_wdt/hifi4dsp_wdt.h>
#include "mdrv_mstypes.h"
#include <adsp_ipi.h>

#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
#include "adsp_ipi.h"
#endif

#ifdef CONFIG_AMAZON_DSP_FRAMEWORK
#include "adf/adf_status.h"
#include "adf/adf_common.h"
static int adfDbgReadFunc(uintptr_t dest, int size);
#endif

#define DSP_LOAD_UNIT_TEST	0

/*
 * hifi4dsp registers and bits
 */
#define SCPSYS_REG_BASE            (0x1D00B000)
#define REG_SLEEP_DSP0             (SCPSYS_REG_BASE + 0x004)
#define REG_SLEEP_DSP1             (SCPSYS_REG_BASE + 0x008)
#define REG_SLEEP_PWR_STA          (SCPSYS_REG_BASE + 0x060)
#define REG_SLEEP_PWR_STAS         (SCPSYS_REG_BASE + 0x064)

#define DSP0_REG_BASE              (0x1D062000)
#define DSP1_REG_BASE              (0x1D063000)
    #define OFFSET_DSP0_1          (DSP1_REG_BASE - DSP0_REG_BASE)
#define DSP0_GPR1c                 (DSP0_REG_BASE + 0xA0)
#define DSP0_GPR1d                 (DSP0_REG_BASE + 0xA4)
#define DSP1_GPR1d                 (DSP1_REG_BASE + 0x00a4)
#define DSP1_GPR1f                 (DSP1_REG_BASE + 0xAC)
#define REG_ALT_RESET_VEC(n)       (DSP0_REG_BASE + 0x04 + OFFSET_DSP0_1 * n)
#define REG_P_DEBUG_BUS0(n)        (DSP0_REG_BASE + 0x0C + OFFSET_DSP0_1 * n)
	#define PDEBUG_ENABLE        0
#define REG_SEL_RESET_SW(n)        (DSP0_REG_BASE + 0x24 + OFFSET_DSP0_1 * n)
	/*reset sw*/
	#define BRESET_SW            0
	#define DRESET_SW            1
	#define PRESET_SW            2
	#define RUNSTALL             3
	#define STATVECTOR_SEL       4
	#define AUTO_BOOT_SW_RESET_B 5

#define REG_DSP_CHIP_VER           (0x1D00E404)
#define DSP_ROMCODE_BASE           (0x1E030000)
#define DSP_SRAM_BASE              (0x1FC00000)
#define MTK_DSP_CHIP_HW_VER_1      (0x0)
#define MTK_DSP_CHIP_HW_VER_2      (0x100)
#define DSP_LOG_BUF_SIZE           (0x1000)
#define DSP_LOG_BUF_MAGIC          (0x67676F6C)
#define GPR_LOG_BUF_ADDR           (DSP0_GPR1d)
#define GPR_LOG_BUF_SIZE_ADDR      (DSP0_GPR1c)
#define GPR_DSP1_REBOOT            (DSP1_GPR1d)

#define DSP_LOG_DUMP_PERIOD        (200)

/*
 * hifi4dsp image package and format.
 *
 * |------------|-----------|----------------|----------------|
 *
 * [romcode header]  [preloader.bin]  [DSP_A.bin]  [DSP_B.bin]
 *
 */

struct hifi4dsp_image_info {
	int dual_core;
	u32 boot_addr;
};

struct hifi4dsp_debug_reg {
	char cmd[10];
	u32 addr;
	u32 data;
};

struct hifi4dsp_log_ctrl {
	u32 magic;
	u32 start;
	u32 size;
	u32 offset;
	int full;
};

struct hifi4dsp_callback {
	callback_fn cb;
	void *args;
};

#define HIFI4DSP_IMAGE_NAME	"hifi4dsp_load.bin"
#define MAX_HIFI4DSP_IMAGE_SIZE  (0x200000)

#define XIP_CODE_48K_SIZE  (48 * 1024)
#define HIFI4DSP_BIN_MAGIC ((u64)(0x4D495053444B544D))

struct hifi4dsp_load_dev {
	struct device *dev;
	int boot_done;
	struct hifi4dsp_callback callback;
	struct hifi4dsp_image_info img_info;
	struct dentry *dent;
	struct hifi4dsp_log_ctrl log_ctrl;
	struct hifi4dsp_debug_reg debug_reg_cmd;
	struct notifier_block reboot_notifier_to_dsp;
	u32 hifi4dsp_reset_gpio;
};

static struct hifi4dsp_load_dev *hifi4dsp_load;

static u32 hifi4dsp_get_chip_hw_ver(void)
{
#if 0
	u32 val = 0;

	spi_read_register(REG_DSP_CHIP_VER, &val, SPI_SPEED_LOW);
	return (val & 0xFFFF);
#else
return MTK_DSP_CHIP_HW_VER_2;
#endif
}

static int hifi4dsp_debug_reg_show(struct seq_file *m, void *data)
{
	struct hifi4dsp_debug_reg reg = hifi4dsp_load->debug_reg_cmd;
	int i;
	u32 val = 0;
	u32 len = reg.data;
	u32 *data_buf = NULL;

	if (strncmp(reg.cmd, "read", 4) == 0) {
		if (len % 4) {
			seq_puts(m, "the length is not 4bytes aligned\n");
			return 0;
		}
		data_buf = kzalloc(len, GFP_KERNEL);
		if (data_buf == NULL)
			return -ENOMEM;

		if (dsp_spi_read_ex(reg.addr, data_buf, len, SPI_SPEED_HIGH)) {
			seq_puts(m, "read fail\n");
			kfree(data_buf);
			return -ENODEV;
		}
		for (i = 0; i < len / 4; i++) {
			if (i % 4 == 0)
				seq_printf(m, "0x%08x: %08x",
					reg.addr + i, data_buf[i]);
			else if (i % 4 == 3)
				seq_printf(m, " %08x\n", data_buf[i]);
			else
				seq_printf(m, " %08x", data_buf[i]);
		}
		seq_puts(m, "\n");
		kfree(data_buf);
	} else if (strncmp(reg.cmd, "write", 5) == 0) {
		spi_read_register(reg.addr, &val, SPI_SPEED_HIGH);
		seq_printf(m, "0x%08x: 0x%08x\n", reg.addr, val);
	}

	return 0;
}

static int hifi4dsp_debug_reg_open(struct inode *inode, struct file *file)
{
	return single_open(file, hifi4dsp_debug_reg_show, inode->i_private);
}

static ssize_t hifi4dsp_debug_reg_write(struct file *file,
				const char __user *ubuf,
				size_t len, loff_t *offp)
{
	struct hifi4dsp_debug_reg *reg = &hifi4dsp_load->debug_reg_cmd;
	char buf[30];

	if (len >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (sscanf(buf, "%s 0x%x 0x%x", reg->cmd, &(reg->addr), &(reg->data))
		!= 3)
		return -EINVAL;

	if (strncmp(reg->cmd, "write", 5) == 0) {
		spi_write_register(reg->addr, reg->data, SPI_SPEED_HIGH);
		return len;
	}

	return len;
}

static int hifi4dsp_get_log_buf_ctrl_info(void)
{
    u32 log_buf_start;

    spi_read_register(GPR_LOG_BUF_ADDR, &log_buf_start, SPI_SPEED_LOW);
    dsp_spi_read_ex(log_buf_start, &hifi4dsp_load->log_ctrl,
        sizeof(struct hifi4dsp_log_ctrl), SPI_SPEED_LOW);

    if (hifi4dsp_load->log_ctrl.magic != DSP_LOG_BUF_MAGIC) {
        pr_info("hifi4dsp_load->log_ctrl.magic = 0x%lx , %s::%d\n",hifi4dsp_load->log_ctrl.magic, __func__,__LINE__);
        return -EINVAL;
    }
    return 0;
}

int hifi4dsp_get_log_buf_size(u32 *log_size)
{
	int ret;

	ret = hifi4dsp_get_log_buf_ctrl_info();
	if (ret) {
		*log_size = 0;
		return ret;
	}

	if (hifi4dsp_load->log_ctrl.full)
		*log_size = hifi4dsp_load->log_ctrl.size;
	else
		*log_size = hifi4dsp_load->log_ctrl.offset;

	return 0;
}

int hifi4dsp_get_log_buf(char *log_buf, u32 log_size)
{
	return dsp_spi_read_ex(hifi4dsp_load->log_ctrl.start, log_buf,
		log_size, SPI_SPEED_LOW);
}

static int hifi4dsp_debug_log_show(struct seq_file *m, void *data)
{
	char *log_buf = NULL;
	u32 log_size = 0;
	int ret = 0;

	ret = hifi4dsp_get_log_buf_size(&log_size);
	if (ret) {
		seq_printf(m, "Dump DSP log fail, ret=%d\n", ret);
		goto _end;
	}

	log_buf = kzalloc(log_size, GFP_KERNEL);
	if (!log_buf) {
		ret = -ENOMEM;
		goto _end;
	}

	ret = hifi4dsp_get_log_buf(log_buf, log_size);
	if (ret) {
		seq_printf(m, "Read log_buf fail, ret=%d\n", ret);
		goto _end;
	}

	ret = seq_write(m, log_buf, log_size);
	if (ret) {
		seq_printf(m, "seq_write log_buf fail, ret=%d\n", ret);
		goto _end;
	}

_end:
	kfree(log_buf);
	return ret;
}

static int hifi4dsp_debug_log_open(struct inode *inode, struct file *file)
{
	return single_open(file, hifi4dsp_debug_log_show, inode->i_private);
}

void hifi4dsp_send_WTD_WHOLE(void)
{
	char data[32], *envp[] = { data, NULL };
	snprintf(data, sizeof(data), "ACTION=DSP_WTD_WHOLE");
	kobject_uevent_env(&hifi4dsp_load->dev->kobj, KOBJ_CHANGE, envp);
	return 0;
}
extern void set_DSP_DRV(void);
void set_DSP_DRV(void)
{
	/* Configure DSP Driving */
	/* 0 :3 mA */
	/* 1 :6 mA */
	/* 2 :9 mA */
	/* 3 :12 mA */
	u32 dsp_drv = 1, reg_val=0; // default 6 mA

	spi_read_register(0x1D00DD30, &reg_val, SPI_SPEED_LOW);
	reg_val &= ~(3 << 1);
	char *ptr = strstr(saved_command_line, "DSP_DRV");
	if (ptr != NULL) {
		sscanf(ptr + strlen("DSP_DRV="), "%u", &dsp_drv);
	}

	if (dsp_drv > 3) {
		printk ("DSP_DRV=%d in wrong range, should be (0-3)\n Set to 1 here", dsp_drv);
		dsp_drv = 1;
	}
	printk("DSP_DRV=%d ==> %d mA\n", dsp_drv, (dsp_drv + 1) * 3);

	reg_val |= (dsp_drv << 1);
	printk("reg_val = 0x%lx \n",reg_val);
	if(0 != spi_write_register(0x1D00DD30, reg_val, SPI_SPEED_LOW)) {
		printk("DSP_DRV config failed ret = %d\n");
	}
}

static ssize_t hifi4dsp_debug_cli_write(struct file *file,
				const char __user *ubuf,
				size_t len, loff_t *offp)
{
	char buf[50];
	int ret;

	if (len >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, ubuf, len))
		return -EFAULT;

	buf[len] = '\0';

	if (strncmp(buf, "reload", strlen("reload")) == 0) {
		pr_info("SW triggered WDT. Reloading FW!!");
		mtk_dsp_wdt_disable();
		spi_config_MSB();
		set_DSP_DRV();
		hifi4dsp_rst();
		hifi4dsp_send_WTD_WHOLE();
		return len;
	}
	ret = adsp_ipi_send(ADSP_IPI_CLI, buf, strlen(buf), 0, 0);
	if (ret) {
		pr_info("send cli cmd failed\n");
		return -EINVAL;
	}
	return len;
}

static const struct file_operations hifi4dsp_debug_reg_fops = {
	.open = hifi4dsp_debug_reg_open,
	.read = seq_read,
	.write = hifi4dsp_debug_reg_write,
};

static const struct file_operations hifi4dsp_debug_log_fops = {
	.open = hifi4dsp_debug_log_open,
	.read = seq_read,
};

static const struct file_operations hifi4dsp_debug_cli_fops = {
	.write = hifi4dsp_debug_cli_write,
};

static int hifi4dsp_debugfs_init(void)
{
	struct dentry *reg_rw_dent;
	struct dentry *log_buf_dent;
	struct dentry *cli_dent;

	hifi4dsp_load->dent = debugfs_create_dir("hifi4dsp", NULL);
	if (!hifi4dsp_load->dent)
		return -ENOMEM;

	reg_rw_dent = debugfs_create_file("reg", 0644,
			hifi4dsp_load->dent, NULL, &hifi4dsp_debug_reg_fops);
	if (!reg_rw_dent)
		goto out_err;

	cli_dent = debugfs_create_file("cli", 0200,
			hifi4dsp_load->dent, NULL, &hifi4dsp_debug_cli_fops);
	if (!cli_dent)
		goto out_err;

	log_buf_dent = debugfs_create_file("log", 0444,
			hifi4dsp_load->dent, NULL, &hifi4dsp_debug_log_fops);
	if (!log_buf_dent)
		goto out_err;

	return 0;

out_err:
	debugfs_remove_recursive(hifi4dsp_load->dent);
	return -ENOMEM;
}

static int check_image_header_info(u8 *data, int size)
{
	int ret = 0;
	u64 magic;

	/* check miminal header size : 0x800 */
	if (size < 0x800) {
		ret = -1;
		return ret;
	}

	magic = *(u64 *)data;
	if (magic != HIFI4DSP_BIN_MAGIC) {
		ret = -2;
		pr_err("HIFI4DSP_BIN_MAGIC error : 0x%llX\n", magic);
	}

	return ret;
}

static
int load_image_to_dsp(u32 ldr, void *data, u32 len)
{
	int err = 0;
        //printk("before dsp_spi_write_ex   len = %ld\n",len);
	err = dsp_spi_write_ex(ldr, data, (int)len, SPI_LOAD_IMAGE_SPEED);
        //printk("after dsp_spi_write_ex   SPI_LOAD_IMAGE_SPEED  = %d",SPI_LOAD_IMAGE_SPEED);
	/*
	 * Notice:
	 * 1. this load-address is setting to reg DSP1_GPR1f
	 *	for DSP1 using only.
	 * 2. we assume the lastest binary send is HIFI4DSP1.bin
	 */
	spi_write_register(DSP1_GPR1f, ldr, SPI_SPEED_HIGH);

	return err;
}

static int parse_and_load_image(const struct firmware *fw)
{
	int err;
	int loop;
	u8 *fw_data;
	u8 *data;
	int total_image_len;
	int signature;
	u64 img_bin_tb_inf;
	int boot_adr_num;
	u32 dsp_boot_adr;
	int img_bin_inf_num;
	u32 section_off;
	u32 section_len;
	u32 section_ldr;
	u32 fix_offset;
	const char fwname[] = HIFI4DSP_IMAGE_NAME;
	struct hifi4dsp_image_info *info = &hifi4dsp_load->img_info;

	/*
	 * step1: check image header and magic.
	 */
	if (fw->size > MAX_HIFI4DSP_IMAGE_SIZE) {
		err = -EFBIG;
		pr_notice("firmware %s size too large!\n", fwname);
		goto tail;
	}
#if 0
	if (fw->size <= XIP_CODE_48K_SIZE) {
		err = -1;
		pr_notice("firmware %s size too small!\n", fwname);
		goto tail;
	}
#endif
	pr_info("firmware %s load success, size = %d\n",
		fwname, (int)fw->size);
#if 0
	fw_data = (u8 *)fw->data + XIP_CODE_48K_SIZE;
	total_image_len = (int)fw->size - XIP_CODE_48K_SIZE;
#else
	fw_data = (u8 *)fw->data;
	total_image_len = (int)fw->size;
#endif

	err = check_image_header_info(fw_data, total_image_len);
	if (err) {
		pr_notice("firmware %s may be corrupted!\n", fwname);
		goto tail;
	}

	/*
	 * step2: Parse Image Header
	 * 1), check image signature or not,
	 * 2), bypass encrypted image with spi-write() if signatured;
	 * 3), check single core or dual core if no signature;
	 */
	img_bin_tb_inf = *(u64 *)(fw_data + 16);
	/*
	 * sizeof (TB_INF) = 8bytes.
	 * TB_INF[7]: authentication type for IMG_BIN_INF_TB
	 * TB_INF[6]: authentication type for IMG_BIN
	 * TB_INF[5]: encryption type for IMG_BIN
	 * TB_INF[4:2]: reserved
	 * TB_INF[1]:
	 * bit0: indicate if enabling authentication (1) or not (0)
	 *	for IMG_BIN_INF_TB
	 * TB_INF[0]: version of IMG_BIN_INF_TB
	 */
	signature = (img_bin_tb_inf) ? 1 : 0;
	if (signature || hifi4dsp_get_chip_hw_ver() == MTK_DSP_CHIP_HW_VER_2) {
		/*
		 * Send all fw_data.bin to sram for DSP-romcode parsing
		 * if authentication enabled
		 */
		err = load_image_to_dsp(DSP_SRAM_BASE, fw_data,
					total_image_len);
		if (err) {
			pr_notice("%s write all %s (%d bytes) fail!\n",
				__func__, fwname, total_image_len);
		}
		info->boot_addr = DSP_ROMCODE_BASE;
		goto tail;
	}

	/* 28bytes =
	 * |BIN_MAGIC|BIN_TOTAL_SZ|IMG_BIN_INF_TB_SZ|TB_INF|TB_LD_ADR|
	 */
	/* BOOT_ADR_NO(M) */
	boot_adr_num = *(u32 *)(fw_data + 28);
	/* DSP_1_ADR for DSP0 bootup entry, other *_ADR ignored */
	dsp_boot_adr = *(u32 *)(fw_data + 28 + 4);
	/* IMG_BIN_INF_NO(N) */
	img_bin_inf_num = *(u32 *)(fw_data + 32 + 4 * boot_adr_num);

	info->boot_addr = dsp_boot_adr;
	info->dual_core = (boot_adr_num == 1) ? 0 : 1;

	/*
	 * skip preloader.bin first.
	 * MG_BIN_INF_X (20bytes, loop read info)
	 */
	for (loop = 1; loop < img_bin_inf_num; loop++) {
		fix_offset = 32 + (4 * boot_adr_num) + 8 + (20 * loop);
		/* IMG_BIN_OFST */
		section_off = *(u32 *)(fw_data + fix_offset + 4);
		/* IMG_SZ */
		section_len = *(u32 *)(fw_data + fix_offset + 12);
		/* LD_ADR */
		section_ldr = *(u32 *)(fw_data + fix_offset + 16);

		pr_info(
			"section%d: load_addr = 0x%08X, offset = 0x%08X, len = %u\n",
			loop, section_ldr, section_off, section_len);

		/*
		 * IMG_BIN_OFST: start from beginning of IMG_BINS.
		 * #0x00000814 = Total_HEAD_len
		 *	= 8(BIN_MAGIC) + 4(BIN_TOTAL_SZ) + 4(IMG_BIN_INF_TB_SZ)
		 *	+ 0x800(IMG_BIN_INF_TB) + 4(IMG_BINS_SZ)
		 */
		data = fw_data + 0x00000814 + section_off;
		err = load_image_to_dsp(section_ldr, data, section_len);
		if (err) {
			pr_notice("%s write section%d bin (%d bytes) fail!\n",
				__func__, loop, section_len);
			goto tail;
		}
	}
tail:
	return err;
}

/*
 * 1. Release DSP0 core.
 * 2. Maybe power-on DSP1 if need (not must).
 */
static void hifi4dsp_poweron(void)
{
	u32 val = 0;
	u32 boot_addr = hifi4dsp_load->img_info.boot_addr;
	int dual_core = hifi4dsp_load->img_info.dual_core;

	/* bootup from ROMCODE or SRAM base */
	spi_write_register(REG_ALT_RESET_VEC(0), boot_addr, SPI_SPEED_HIGH);
	spi_read_register(REG_ALT_RESET_VEC(0), &val, SPI_SPEED_HIGH);
	pr_info("DSP0 boot from base addr : 0x%08X\n", val);

	/*
	 * 1.STATVECTOR_SEL pull high to
	 * select external reset vector : altReserVec
	 * 2. RunStall pull high
	 */
	spi_set_register32(REG_SEL_RESET_SW(0),
			(0x1 << STATVECTOR_SEL) | (0x1 << RUNSTALL),
			SPI_SPEED_HIGH);

	/* DReset & BReset pull high */
	spi_set_register32(REG_SEL_RESET_SW(0),
			(0x1 << BRESET_SW) | (0x1 << DRESET_SW),
			SPI_SPEED_HIGH);
	/* DReset & BReset pull low */
	spi_clr_register32(REG_SEL_RESET_SW(0),
			(0x1 << BRESET_SW) | (0x1 << DRESET_SW),
			SPI_SPEED_HIGH);

	/* Enable PDebug */
	spi_set_register32(REG_P_DEBUG_BUS0(0),
			(0x1 << PDEBUG_ENABLE), SPI_SPEED_HIGH);

	if (!dual_core)
		goto tail;

	/* hifi4_dsp1_power_on() by DPS0 */
	pr_notice("dual-core power-on\n");

tail:
	/*
	 * Hifi4_ReleaseDSP, only for DSP0.
	 */
	/* DSP RESET B as high to release DSP */
	spi_set_register32(REG_SEL_RESET_SW(0),
			(0x1 << AUTO_BOOT_SW_RESET_B), SPI_SPEED_HIGH);

	/* RUN_STALL pull down */
	spi_clr_register32(REG_SEL_RESET_SW(0),
			(0x1 << RUNSTALL), SPI_SPEED_HIGH);
}

static void fixup_hifi4dsp_early_setting(void)
{
	u32 old, new;
	u32 spis_mclk, reg_val=0;
	u32 spis1_clk;

	pr_info("%s enter\n", __func__);
	/*
	 * Must first spi-write for DSP SPI-SMT setting.
	 * Disable DSP watchdog subsequently.
	 */
	spi_write_register(0x1D00DA04, 0x1800, SPI_SPEED_LOW);
	spi_write_register(0x1D010000, 0x22000200, SPI_SPEED_LOW);

	/* 400M XTAL */
	spi_read_register(0x1d00c000, &reg_val, SPI_SPEED_LOW);
	reg_val |= (1 << 0);
	spi_write_register(0x1d00c000, reg_val, SPI_SPEED_LOW);
	spi_read_register(0x1d00c000, &reg_val, SPI_SPEED_LOW);
	reg_val &= ~(1 << 1);
	spi_write_register(0x1d00c000, reg_val, SPI_SPEED_LOW);

	/* 400M PLL */
	spi_read_register(0x1d00c170, &reg_val, SPI_SPEED_LOW);
	reg_val |= (1 << 0);
	spi_write_register(0x1d00c170, reg_val, SPI_SPEED_LOW);
	spi_read_register(0x1d00c170, &reg_val, SPI_SPEED_LOW);
	reg_val &= 0xfffffffd;
	spi_write_register(0x1d00c170, reg_val, SPI_SPEED_LOW);
	spis1_clk = 0x820f6276;
	spi_write_register(0x1d00c164, spis1_clk, SPI_SPEED_LOW);
	spi_read_register(0x1d00c160, &spis1_clk, SPI_SPEED_LOW);
	spis1_clk |= (1 << 0);
	spi_write_register(0x1d00c160, spis1_clk, SPI_SPEED_LOW);

	/* SPIS1 module clk to 400MHz */
	spi_read_register(0x1D00E0CC, &spis_mclk, SPI_SPEED_LOW);
	reg_val = spis_mclk & (~(0xff << 16));
	reg_val |= (0x2 << 16);
	spi_write_register(0x1D00E0CC, reg_val, SPI_SPEED_LOW);

	/* DSP & Bus module clk to 400MHz */
	spi_read_register(0x1D00E0D4, &reg_val, SPI_SPEED_LOW);
	reg_val &= ~(0xf << 0);
	reg_val |= (0x2 << 0);
	spi_write_register(0x1D00E0D4, reg_val, SPI_SPEED_LOW);

	set_DSP_DRV();

	/*
	 * Work for DSP 32K I/D cache random issue,
	 * Must setting before DSP core running.
	 */
#define FIXED_ADDRESS (u32)(0x1D060024)
	spi_read_register(FIXED_ADDRESS, &old, SPI_SPEED_HIGH);
	spi_write_register(FIXED_ADDRESS, 0xAAAAAAAA, SPI_SPEED_HIGH);
	spi_read_register(FIXED_ADDRESS, &new, SPI_SPEED_HIGH);
	pr_err("Readback, address: 0x%X: 0x%X ===>>> 0x%X\n",
			FIXED_ADDRESS, old, new);
	/*
	 * Poweron DSP_SRAM1/2/3/4/5/6/7,
	 * total 4M SRAM to run dual-core.
	 */
	mtcmos_init();

    /* Configure DSP SPI_MODE */
    /* DSP_SPI_MODE=0 :falling triggered, rising sampled (as default in DSP) */
    /* DSP_SPI_MODE=1 :rising triggered, falling sampled */
    spi_read_register(0x1d042000, &reg_val, SPI_SPEED_HIGH);
    reg_val &= ~(1 << 0);
    if (strstr(saved_command_line, "DSP_SPIMODE=1")) {
        printk ("DSP_SPIMODE=1 \n");
        reg_val |= (1 << 0);
    } else {
        printk ("DSP_SPIMODE=0 \n");
    }
    printk ("reg_val = 0x%lx \n",reg_val);
    if(0 != spi_write_register(0x1d042000, reg_val, SPI_SPEED_HIGH)) {
        printk ("DSP_SPIMODE config failed ret = %d\n");
    }


    /* Configure DSP EARLY MISO funtion */
    /* EARLY_MISO=0 :normal case (as default in DSP) */
    /* EARLY_MISO=1 :half a SCLK earlier */
    spi_read_register(0x1d042000, &reg_val, SPI_SPEED_HIGH);
    reg_val &= ~(1 << 16);
    if (strstr(saved_command_line, "EARLY_MISO=1")) {
        printk ("EARLY_MISO=1 \n");
        reg_val |= (1 << 16);
    } else {
        printk ("EARLY_MISO=0 \n");
    }
    printk ("reg_val = 0x%lx \n",reg_val);
    if(0 != spi_write_register(0x1d042000, reg_val, SPI_SPEED_HIGH)) {
        printk ("EARLY_MISO config failed ret = %d\n");
    }

	pr_info("%s exit\n", __func__);
}

#ifdef CONFIG_AMAZON_DSP_FRAMEWORK
static int adfDbgReadFunc(uintptr_t dest, int size)
{
	u32 log_buf_start;
	int ret;

	ret = spi_read_register(GPR_LOG_BUF_ADDR, &log_buf_start, SPI_SPEED_LOW);
	if (ret != 0) {
		pr_err("%s failed to read over SPI\n",__func__);
		return -1;
	}

	ret = dsp_spi_read_ex(log_buf_start, (void*)dest, size, SPI_SPEED_LOW);
	if (ret != 0) {
		pr_err("%s failed to read DSP log over SPI\n",__func__);
		return -1;
	}

	return size;
}

static int adfDbgCheckRunFunc(void)
{
	return hifi4dsp_load->boot_done;
}
#endif

static void set_hifi4dsp_run_status(void)
{
#ifdef CONFIG_AMAZON_DSP_FRAMEWORK
    u32 log_buf_size = 0;
#endif

	hifi4dsp_load->boot_done = 1;

#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
	adsp_ipi_set_wdt_status();
#endif

	pr_notice("load bin done and start to run now.\n");

#ifdef CONFIG_AMAZON_DSP_FRAMEWORK
	/* wait for DSP initialiation */
	msleep(200);
	spi_read_register(GPR_LOG_BUF_SIZE_ADDR, &log_buf_size, SPI_SPEED_LOW);

	/* init the log dumping thread */
	adfDebug_init((void *)adfDbgCheckRunFunc, (void *)adfDbgReadFunc, DSP_LOG_DUMP_PERIOD,
				  log_buf_size);
#endif
    void mtk_dsp_wdt_enable(void);
    mtk_dsp_wdt_enable();
}

#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
void clr_hifi4dsp_run_status(void)
{
	hifi4dsp_load->boot_done = 0;
}
EXPORT_SYMBOL(clr_hifi4dsp_run_status);
#endif

/*
 * HIFI4DSP has boot done or not.
 */
int hifi4dsp_run_status(void)
{
	return hifi4dsp_load->boot_done;
}
EXPORT_SYMBOL(hifi4dsp_run_status);

static
void load_image_to_dsp_and_run(
			const struct firmware *fw,
			void *context)
{
	int ret;

	if (!fw) {
		pr_notice("error: fw == NULL!\n");
		pr_notice("request_firmware_nowait (%s) not available.\n",
				HIFI4DSP_IMAGE_NAME);
		return;
	}

	if (!hifi4dsp_spi_get_status()) {
		pr_notice("error: hifi4dsp spi driver is not ready!\n");
		return;
	}

	/*
	 * Step1:
	 * Fixup setting before load-bin
	 */
	fixup_hifi4dsp_early_setting();

 	/*
	 * Step2:
	 * Load binary to DSP SRAM bank0/1
	 */
	ret = parse_and_load_image(fw);
	release_firmware(fw);
	if (ret) {
		pr_notice("firmware_async_load Error!\n");
		return;
	}

	/*
	 * Step3:
	 * Power-on DSP boot sequence
	 */
	msleep(50);
	hifi4dsp_poweron();

	set_hifi4dsp_run_status();

	/* callback function for user */
	if (hifi4dsp_load->callback.cb)
		hifi4dsp_load->callback.cb(hifi4dsp_load->callback.args);
}

/*
 * Assume called by audio system only.
 */
int async_load_hifi4dsp_bin_and_run(
			callback_fn callback, void *param)
{
	int ret = 0;

	if (!hifi4dsp_load)
		return -EPROBE_DEFER;

	if (hifi4dsp_run_status()) {
		pr_notice("error: bootup two times!\n");
		return ret;
	}

	hifi4dsp_load->callback.cb = callback;
	hifi4dsp_load->callback.args = param;

	/* Async load firmware and run hifi4dsp */
	ret = request_firmware_nowait(THIS_MODULE,
			true, HIFI4DSP_IMAGE_NAME, hifi4dsp_load->dev,
			GFP_KERNEL, NULL, load_image_to_dsp_and_run);

	return ret;
}
EXPORT_SYMBOL(async_load_hifi4dsp_bin_and_run);

#if DSP_LOAD_UNIT_TEST
int breed_hifi4dsp(void *data)
{
	int ret = 0;

	/* sleep() or wait_mutex() */
	while (!hifi4dsp_spi_get_status())
		msleep(500);

	ret = async_load_hifi4dsp_bin_and_run(NULL, NULL);

	/*
	 * bootup mission is now accomplished,
	 * the kthead will end self-life.
	 */
	return ret;
}
#endif

static int notifier_of_dsp(struct notifier_block *this,
				unsigned long mode,
				void *cmd);

static int hifi4dsp_load_probe(struct platform_device *pdev)
{
	int ret = 0;
	u32 prop;
	struct device_node *np;
#if defined(CONFIG_IDME)
	char property_name[DTS_STRING_LENGTH];
#endif
#if DSP_LOAD_UNIT_TEST
	static struct task_struct *dsp_task;
#endif

	if (strstr(saved_command_line, "farfield.dsp.name=mt8570") == NULL) {
		pr_info("mt8570 is not supported\n");
		return -EINVAL;
	}

	pr_info("%s() enter.\n", __func__);

	hifi4dsp_load = devm_kzalloc(&pdev->dev,
			sizeof(struct hifi4dsp_load_dev), GFP_KERNEL);
	if (!hifi4dsp_load)
		return -ENOMEM;

	np = of_find_node_by_name(NULL, "hifi4dsp-load");
	if (np == NULL) {
		pr_err("hifi4dsp-load is not defined in dts\n");
		return -EINVAL;
	}

#if defined(CONFIG_IDME)
	snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "hifi4dsp-reset-gpio_", idme_get_config_name());

	if (!of_property_read_u32(np, property_name, &prop)) {
		hifi4dsp_load->hifi4dsp_reset_gpio = prop;
		pr_info("%s:  %s is %d \n", __func__, property_name, prop);
	} else
#endif
	if (!of_property_read_u32(np, "hifi4dsp-reset-gpio", &prop)) {
		hifi4dsp_load->hifi4dsp_reset_gpio = prop;
		pr_info("%s: hifi4dsp_reset_gpio is %d \n", __func__, prop);
	} else {
		pr_err("%s: hifi4dsp_reset_gpio is not defined \n", __func__);
		hifi4dsp_load->hifi4dsp_reset_gpio = 42;
	}

	// Due to SW reboot will be set all of GPIOs into input mode
	// caused aud_afe couldn't reset or restart thus used HW reset to recovery aud_afe.
	hifi4dsp_hw_rst();

	hifi4dsp_load->dev = &pdev->dev;
	hifi4dsp_debugfs_init();

	hifi4dsp_load->reboot_notifier_to_dsp.notifier_call = notifier_of_dsp;
	pr_info("register reboot notifier for dsp in case of kernel panic\n");
	ret = register_reboot_notifier(&hifi4dsp_load->reboot_notifier_to_dsp);
	if (ret != 0)
		pr_notice("cannot register reboot notifier (ret=%d)\n", ret);

#if DSP_LOAD_UNIT_TEST
	dsp_task = kthread_run(breed_hifi4dsp, hifi4dsp_load->dev,
			"breed_hifi4dsp");
	if (IS_ERR(dsp_task)) {
		pr_notice("Couldn't create kthread for breed_hifi4dsp\n");
		ret = PTR_ERR(dsp_task);
		return ret;
	}
	pr_info("Start to run kthread [breed_hifi4dsp]\n");
#endif

	/* config MSB mode before load code */
	spi_config_MSB();
	pr_info("%s() done.\n", __func__);

	return ret;
}

void hifi4dsp_stop_dsp(void)
{
	/* RUN_STALL pull high */
	spi_set_register32(REG_SEL_RESET_SW(0),
			(0x1 << RUNSTALL), SPI_SPEED_LOW);

	/* DReset & BReset pull high */
	spi_set_register32(REG_SEL_RESET_SW(0),
			(0x1 << BRESET_SW) | (0x1 << DRESET_SW),
			SPI_SPEED_LOW);

	/* RUN_STALL pull high */
	spi_set_register32(REG_SEL_RESET_SW(1),
			(0x1 << RUNSTALL), SPI_SPEED_LOW);

	/* DReset & BReset pull high */
	spi_set_register32(REG_SEL_RESET_SW(1),
			(0x1 << BRESET_SW) | (0x1 << DRESET_SW),
			SPI_SPEED_LOW);

	/* clear the DSP1 reboot flag */
	spi_write_register(GPR_DSP1_REBOOT, 0x0, SPI_SPEED_LOW);
}

void hifi4dsp_hw_rst(void)
{
	gpio_direction_output(hifi4dsp_load->hifi4dsp_reset_gpio, 0);
	msleep(10);

	gpio_direction_output(hifi4dsp_load->hifi4dsp_reset_gpio, 1);
	msleep(10);

}
void hifidsp_hw_pull_low(void)
{
    gpio_direction_output(hifi4dsp_load->hifi4dsp_reset_gpio, 0);
}

int hifi4dsp_rst(void)
{
#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
	clr_hifi4dsp_run_status();

	hifi4dsp_stop_dsp();

	mtcmos_deinit();
#endif

	return 0;
}


static int hifi4dsp_load_pm_resume(struct device *device)
{
	printk("%s is resume!\n", __func__);

	hifi4dsp_hw_rst();

    /* PADS_I2S_IN_MD would conflict between hailey+ and hailey, Reconfig to CONFIG_PADMUX_MODE5 for TSLINK SPI */
    pr_info("[%s] Re-config PADS_I2S_IN_MD to TSLINK_SPI\n", __func__);
    REG_ADDR((0x3229<<9) + (0x12<<2)) &= ~(BIT(2)|BIT(1)|BIT(0));
    REG_ADDR((0x3229<<9) + (0x12<<2)) |= 0x05;
    udelay(1000);

    pr_info("[%s] Re-config GPIO1_PM to input mode for mt8570\n", __func__);
    REG_ADDR((0x000F<<9) + (0x01<<2)) |= BIT(0);

	spi_config_MSB();
	hifi4dsp_spi_set_config_mode_status(0);
	set_DSP_DRV();
	//[FIXEDME] need to confirm if we need call this function here
	hifi4dsp_rst();

	pr_info("[%s][resume] SW triggered WDT. Reloading FW!!", __func__);
	hifi4dsp_send_WTD_WHOLE();

	return 0;
}

static int hifi4dsp_load_pm_suspend(struct device *device)
{
	printk("%s is suspend!\n", __func__);
	hifi4dsp_rst();
	gpio_direction_output(hifi4dsp_load->hifi4dsp_reset_gpio, 0);
	msleep(10);
	return 0;
}

static int hifi4dsp_load_remove(struct platform_device *pdev)
{
	hifi4dsp_stop_dsp();
	debugfs_remove_recursive(hifi4dsp_load->dent);
	devm_kfree(&pdev->dev, hifi4dsp_load);
	return 0;
}

static void hifi4dsp_load_shutdown(struct platform_device *pdev)
{
	hifi4dsp_stop_dsp();
	debugfs_remove_recursive(hifi4dsp_load->dent);
	devm_kfree(&pdev->dev, hifi4dsp_load);
}

static int notifier_of_dsp(struct notifier_block *this,
				unsigned long mode,
				void *cmd)
{
	hifi4dsp_stop_dsp();

	/*
	 * Power off DSP1, AFE(skip INFRA: due to SPIS1 uses infra power)
	 * DSP_SRAM 0~7
	 */
	mtcmos_deinit();

	return NOTIFY_DONE;
}

#ifdef CONFIG_OF
static const struct of_device_id hifi4dsp_load_of_ids[] = {
	{ .compatible = "mediatek,hifi4dsp-load" },
	{ /* sentinel */ }
};
#endif

struct dev_pm_ops const hifi4dsp_load_pm_ops = {
	.suspend = hifi4dsp_load_pm_suspend,
	.resume = hifi4dsp_load_pm_resume,
};

static struct platform_driver hifi4dsp_load_drv = {
	.probe		= hifi4dsp_load_probe,
	.remove		= hifi4dsp_load_remove,
	.shutdown	= hifi4dsp_load_shutdown,
	.driver = {
		.name		= HIFI4DSP_LOAD_DRV_NAME,
		.owner		= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table	= hifi4dsp_load_of_ids,
#endif
		.pm = &hifi4dsp_load_pm_ops,
	},

};

static int __init hifi4dsp_load_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&hifi4dsp_load_drv);
	if (ret) {
		pr_notice("register hifi4dsp_load_drv fail, ret %d\n", ret);
		return ret;
	}

	return 0;
}

static void __exit hifi4dsp_load_exit(void)
{
	platform_driver_unregister(&hifi4dsp_load_drv);
}

module_init(hifi4dsp_load_init);
module_exit(hifi4dsp_load_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Joe Yang <joe.yang@mediatek.com>");
MODULE_DESCRIPTION("load hifi4dsp firmware");
