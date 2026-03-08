/*
 * Device driver for monitoring ambient light intensity (lux)
 * for device tsl2540.
 *
 * Copyright (c) 2017, ams AG
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>

#include <linux/i2c/ams_tsl2540.h>
#include "ams_i2c.h"
#include "ams_tsl2540_als.h"

extern struct tsl2540_chip *ams_chip;

#ifdef CONFIG_AMS_ADJUST_WITH_BASELINE
#define EEPROM_I2C_ADDRESS       0x50
#define EEPROM_I2C_SND_ADDRESS   0x58
#define EEPROM_I2C_PORT          0    //HWi2C port
#define EEPROM_CAL_DATA_LEN      19   //SKU:XXX:YYY:ZZZ
#define EEPROM_CAL_SKU           0    //color tag = 0
#define EEPROM_MSN_LEN           16
#define EEPROM_PAGE_SIZE         16
#define OPT_STACK_ID             2
extern int MDrv_HW_IIC_WriteBytes(u8 u8Port, u8 u8SlaveIdIIC, u8 u8AddrSizeIIC, u8 *pu8AddrIIC, u32 u32BufSizeIIC, u8 *pu8BufIIC);
extern int MDrv_HW_IIC_ReadBytes(u8 u8Port, u8 u8SlaveIdIIC, u8 u8AddrSizeIIC, u8 *pu8AddrIIC, u32 u32BufSizeIIC, u8 *pu8BufIIC);

/*** FIXME code change to support HVT AMB to keep MM team going to work on DBIQ */
extern char *idme_get_config_name(void);

static int firetv_read_calibration_data(struct device *dev, u8 *buf, int len);
static int firetv_write_calibration_data(struct device *dev, u8 *buf, int len);
static int firetv_read_abm_sn(struct device *dev, u8 *buf, int len);
static int firetv_write_abm_sn(struct device *dev, u8 *buf, int len);
static int eeprom_write_cal(u8 i2cPort, u8 address, u8 addrsize, u8 reg_start, u32 len, u8 *buf);
#endif //for calibratation and EEPROM access

/* TSL2540 Identifiers */
static u8 const tsl2540_ids[] = {
/* ID, AUXID, REV */
	0xE4,	0x00,	0x61,
	0xE4,	0x01,	0x61
};

/* TSL2540 Device Names */
static char const *tsl2540_names[] = {
	"tsl2540",
	"tsl2540"
};

/* in order to use fixed point to calcuate lux, each coefficients
 * is sacled by the factor of 1000000, dgf by 10000
 * the DVT AMB is set to use ATIME=98.56ms
 */
static struct tsl2540_lux_segment als_lux_cofe[][3] = {
   {{1059069, 226239,   69523}, {7284816, -1512431,   10726}, {  168708,   -11358,  226408}},
   {{1921337, -40899,  967741}, { 854231,   -70533, 2077421}, { 1145084,  -162578, 1604706}}
};
/* 
static struct tsl2540_lux_segment als_lux_cofe[][3] = {
   {{1059069, 226239,    69523}, {7284816, -1512431,   10726}, { 168708,   -11358,  226408}},
   {{1782727, -36262,  1034827}, {1524497,  -189168, 1280168}, { 884594,  -119894, 2023467}}
};
*/

/* 99 ATIME=99ms for DVT
static struct tsl2540_lux_segment als_lux_cofe[][3] = {
   {{1059069, 226239,   69523}, {7284816, -1512431,   10726}, { 168708,   -11358,  226408}},
   {{1942088, -39900,  954164}, {1637025,  -203073, 1197418}, {1166928,  -158162, 1540774}}
};
*/

static u32 als_lux_tvis[OPT_STACK_ID]   = {204370000, 32932787}; //scaled by 10000
static u32 als_lux_tir[OPT_STACK_ID]    = {10870000,  1883169};  //scaled by 10000
static u32 als_lux_edge_1[OPT_STACK_ID] = {1262,  642};  //scaled by 1000
static u32 als_lux_edge_2[OPT_STACK_ID] = {3038,  2428}; //scaled by 1000

/*** FIXME code change to support HVT AMB to keep MM team going to work on DBIQ */
static int tsl2540_check_device_id(struct tsl2540_chip *chip)
{
    char config_name[25];
    struct device *dev = &chip->client->dev;

    strncpy (config_name, idme_get_config_name(), 20);

    if ( ( 0 == strcasecmp(config_name, "abc123_hvt")) ||
         ( 0 == strcasecmp(config_name, "abc123eu_ffhvt"))) {
	  dev_info(dev, "%s: HVT device detected %s.\n", __func__, config_name);
	  return 0; //HVT
    }
    else{
	  dev_info(dev, "%s: DVT or up device detected %s.\n", __func__, config_name);
          return 1; //non-HVT device
    }
}

/* Registers to restore */
static u8 const restorable_regs[] = {
	TSL2540_REG_PERS,
	TSL2540_REG_PGCFG0, //what is this, no document
	TSL2540_REG_PGCFG1, //no document
	TSL2540_REG_CFG1,
	TSL2540_REG_CFG2,
	TSL2540_REG_CFG3,
	TSL2540_REG_AILT,
	TSL2540_REG_AILT_HI,
	TSL2540_REG_AIHT,
	TSL2540_REG_AIHT_HI,
	TSL2540_REG_PTIME, //unducumented
	TSL2540_REG_ATIME,
};

#ifdef TSL2540_ENABLE_INTERRUPT
static int tsl2540_irq_handler(struct tsl2540_chip *chip)
{
	u8 status;
	int ret;

	ret = ams_i2c_read(chip->client, TSL2540_REG_STATUS,
			&chip->shadow[TSL2540_REG_STATUS]);
	status = chip->shadow[TSL2540_REG_STATUS];

	if (status == 0){
	    return 0;  /* not our interrupt */
	}
        do {
             /* Clear the interrupts we'll process */
             //ams_i2c_write_direct(chip->client, TSL2540_REG_STATUS, status);

		/*
		 * ALS
		 */
		if (status & TSL2540_ST_ALS_SAT) {
			//chip->in_asat = 1;
			dev_warn(&chip->client->dev,
					"Saturation, ASAT is %d\n", chip->in_asat);
			chip->is_als_valid = 0;
		} else {
			//chip->in_asat = 0;
			chip->is_als_valid = 1;
		}

		if ((status & TSL2540_ST_ALS_IRQ) ||
			(status & TSL2540_ST_ALS_SAT)) {
			tsl2540_read_als(chip);
			tsl2540_report_als(chip);
			//dev_info(&chip->client->dev,
			//	"Lux: %u, ch0: %u, ch1: %u, "
			//	"asat: %u, is_valid: %u\n",
			//	chip->als_inf.lux, chip->als_inf.als_ch0,
			//	chip->als_inf.als_ch1, chip->in_asat,
			//	chip->is_als_valid);
		}

		/*
		 * Calibration
		 */
		if (status & TSL2540_ST_CAL_IRQ) {
			chip->amscalcomplete = true;
			/*
			 * Calibration has completed, no need for more
			 *  calibration interrupts. These events are one-shots.
			 *  next calibration start will re-enable.
			 */
			ams_i2c_modify(chip->client, chip->shadow,
				TSL2540_REG_INTENAB, TSL2540_CIEN, 0);
		}

		ret = ams_i2c_read(chip->client, TSL2540_REG_STATUS,
				&chip->shadow[TSL2540_REG_STATUS]);
		status = chip->shadow[TSL2540_REG_STATUS];
	} while (status != 0);
	return 1;  /* we handled the interrupt */
}

static irqreturn_t tsl2540_irq(int irq, void *handle)
{
	struct tsl2540_chip *chip = handle;
	struct device *dev = &chip->client->dev;
	int ret;

	if (chip->in_suspend) {
		dev_info(dev, "%s: in suspend\n", __func__);
		chip->irq_pending = 1;
		ret = 0;
		goto bypass;
	}
	ret = tsl2540_irq_handler(chip);

bypass:
	return ret ? IRQ_HANDLED : IRQ_NONE;
}
#endif /* TSL2540_ENABLE_INTERRUPT */

static int tsl2540_flush_regs(struct tsl2540_chip *chip)
{
	int i;
	int rc;
	u8 reg;

	for (i = 0; i < ARRAY_SIZE(restorable_regs); i++) {
		reg = restorable_regs[i];
		rc = ams_i2c_write(chip->client, chip->shadow, reg,
				chip->shadow[reg]);
		if (rc) {
			dev_err(&chip->client->dev, "%s: err on reg 0x%02x\n",
					__func__, reg);
			break;
		}
	}

	return rc;
}

static int tsl2540_update_enable_reg(struct tsl2540_chip *chip)
{
	return ams_i2c_write(chip->client, chip->shadow, TSL2540_REG_ENABLE,
			chip->shadow[TSL2540_REG_ENABLE]);
}

#ifdef TSL2540_ENABLE_INPUT
static int tsl2540_pltf_power_on(struct tsl2540_chip *chip)
{
	int rc = 0;

	if (chip->pdata->platform_power) {
		rc = chip->pdata->platform_power(&chip->client->dev,
			POWER_ON);
		mdelay(10);
	}
	chip->unpowered = rc != 0;
	dev_err(&chip->client->dev, "\n\n%s: unpowered=%d\n", __func__,
			chip->unpowered);
	return rc;
}
#endif

static int tsl2540_pltf_power_off(struct tsl2540_chip *chip)
{
	int rc = 0;

	if (chip->pdata->platform_power) {
		rc = chip->pdata->platform_power(&chip->client->dev,
			POWER_OFF);
		chip->unpowered = rc == 0;
	} else {
		chip->unpowered = false;
	}
	dev_err(&chip->client->dev, "\n\n%s: unpowered=%d\n", __func__,
			chip->unpowered);
	return rc;
}

static void tsl2540_set_defaults(struct tsl2540_chip *chip)
{
	u8 *sh = chip->shadow;
	struct device *dev = &chip->client->dev;

	/* Clear the register shadow area */
	memset(chip->shadow, 0x00, sizeof(chip->shadow));
	/* If there is platform data use it */
	if (chip->pdata) {
		dev_info(dev, "%s: Loading pltform data\n", __func__);
		chip->params.persist = chip->pdata->parameters.persist;
		chip->params.als_gain = chip->pdata->parameters.als_gain;
		chip->params.als_gain_factor = chip->pdata->parameters.als_gain_factor;
		chip->params.als_tvis = chip->pdata->parameters.als_tvis;
		chip->params.als_tir   = chip->pdata->parameters.als_tir;
		chip->params.als_edge1 = chip->pdata->parameters.als_edge1;
		chip->params.als_edge2 = chip->pdata->parameters.als_edge2;
		chip->params.als_time  = chip->pdata->parameters.als_time;
		chip->params.als_deltap = chip->pdata->parameters.als_deltap;
		memcpy(chip->params.lux_segment, chip->pdata->parameters.lux_segment, 3*sizeof(struct tsl2540_lux_segment));
		chip->params.az_iterations = chip->pdata->parameters.az_iterations;
	} else {
		dev_info(dev, "%s: use defaults\n", __func__);
		chip->params.persist = ALS_PERSIST(2);
		chip->params.als_gain = AGAIN_4;
		chip->params.als_gain_factor = 1;
		chip->params.als_time = AW_TIME_MS(200);
		chip->params.als_deltap = 10;
		chip->params.als_tvis  = 3293;
		chip->params.als_tir   = 188;
		chip->params.als_edge1 = 642;
		chip->params.als_edge2 = 2428;
		chip->params.als_time  = 0x22;
		memcpy(chip->params.lux_segment, als_lux_cofe[chip->als_inf.stack_id], 3*sizeof(struct tsl2540_lux_segment));
		chip->params.az_iterations = 64;
	}

	/*FIXME, use fixed gain for HVT and auto gain for others */
	if ( chip->als_inf.stack_id == 0 ) {
	   chip->als_gain_auto = false;
	   dev_info(dev, "%s: use fixed gain for HVT device.\n", __func__);
	}
	else {
	   chip->als_gain_auto = true;
	   dev_info(dev, "%s: use auto gain for DVT and up.\n", __func__);
	}

	/* the max count value is 35839 at ATIME = 98.56ms 
	 * 90% of the max coun value is 32255, the defaul
	 * low threshold is 35839/200 = 179
	 */
	chip->als_inf.saturation = ((chip->params.als_time + 1) * 1024 -1) * 9 / 10; //90% of the max count
	chip->als_inf.full_gain = 16;
	chip->als_inf.low_thrs  = chip->als_inf.saturation / 200;
	chip->als_inf.high_thrs = chip->als_inf.saturation / 4;

	/* Copy the default values into the register shadow area */
	sh[TSL2540_REG_PERS]    = chip->params.persist;
	sh[TSL2540_REG_ATIME]   = chip->params.als_time;
	sh[TSL2540_REG_CFG1]    = chip->params.als_gain;
	sh[TSL2540_REG_AZ_CONFIG] = chip->params.az_iterations;
	//set interrupt clear bit, read to clear
	sh[TSL2540_REG_CFG2]    = 0x4;
	sh[TSL2540_REG_CFG3]    = 0xCC;
	//set low threshold for interupt
	sh[TSL2540_REG_AILT]    = 0x0A;
	sh[TSL2540_REG_AILT_HI] = 0x00;
	//set hi threshold for interupt
	sh[TSL2540_REG_AIHT]    = 0x90;
	sh[TSL2540_REG_AIHT_HI] = 0xE2;
	tsl2540_flush_regs(chip);
}

#ifdef ABI_SET_GET_REGISTERS
/* bitmap of registers that are in use */
static u8 reginuse[MAX_REGS / 8] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 0x00 - 0x3f */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 0x40 - 0x7f */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0x80 - 0xbf */
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	/* 0xc0 - 0xff */
};

static ssize_t tsl2540_regs_get(struct tsl2540_chip *chip, char *buf,
		int bufsiz)
{
	u8 regval[16];
	int i, j, cnt;

	/* find first */
	for (i = 0; i < ARRAY_SIZE(reginuse) / sizeof(reginuse[0]); i++) {
		if (reginuse[i] != 0)
			break;
	}

	/* round down to the start of a group of 16 */
	i &= ~1;

	/* set to actual register id */
	i *= 8;

	cnt = 0;
	for (; i < MAX_REGS; i += 16) {
		cnt += snprintf(buf + cnt, bufsiz - cnt, "%02x  ", i);

		ams_i2c_blk_read(chip->client, i, &regval[0], 16);

		for (j = 0; j < 16; j++) {

			if (reginuse[(i >> 3) + (j >> 3)] & (1 << (j & 7))) {
				cnt += snprintf(buf + cnt, bufsiz - cnt,
						" %02x", regval[j]);
			} else {
				cnt += snprintf(buf + cnt, bufsiz - cnt, " --");
			}

			if (j == 7)
				cnt += snprintf(buf + cnt, bufsiz - cnt, "  ");
		}

		cnt += snprintf(buf + cnt, bufsiz - cnt, "\n");
	}

	cnt += snprintf(buf + cnt, bufsiz - cnt, "\n");
	return cnt;

}

void tsl2540_reg_log(struct tsl2540_chip *chip)
{
	char *buf;

	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (buf) {
		tsl2540_regs_get(chip, &buf[0], PAGE_SIZE);
		pr_err("%s", buf);
		kfree(buf);
	} else {
		dev_err(&chip->client->dev, "%s: out of memory!\n", __func__);
	}
}

static ssize_t tsl2540_regs_dump(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	return tsl2540_regs_get(dev_get_drvdata(dev), buf, PAGE_SIZE);
}

static ssize_t tsl2540_reg_set(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	int preg;
	int pval;
	int pmask = -1;
	int numparams;
	int rc;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	numparams = sscanf(buf, "0x%x:0x%x:0x%x", &preg, &pval, &pmask);
	if (numparams == 0) {
		/* try decimal */
		numparams = sscanf(buf, "%d:%d:%d", &preg, &pval, &pmask);
	}

	if ((numparams < 2) || (numparams > 3))
		return -EINVAL;
	if ((numparams >= 1) && ((preg < 0) || ((reginuse[(preg >> 3)] &
			(1 << (preg & 7))) == 0)))
		return -EINVAL;
	if ((numparams >= 2) && (preg < 0 || preg > 0xff))
		return -EINVAL;
	if ((numparams >= 3) && (pmask < 0 || pmask > 0xff))
		return -EINVAL;

	AMS_MUTEX_LOCK(&chip->lock);

	if (pmask == -1) {
		rc = ams_i2c_write(chip->client, chip->shadow, preg, pval);
	} else {
		rc = ams_i2c_modify(chip->client, chip->shadow,
			preg, pmask, pval);
	}

	AMS_MUTEX_UNLOCK(&chip->lock);

	return rc ? rc : size;
}

static DEVICE_ATTR(regs, 0644, tsl2540_regs_dump, tsl2540_reg_set);

static ssize_t tsl2540_irq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        int ret;
	u8 reg_irq, enable;
        struct tsl2540_chip *chip = dev_get_drvdata(dev);

	AMS_MUTEX_LOCK(&chip->lock);
	ret = ams_i2c_read(chip->client, TSL2540_REG_INTENAB, &reg_irq);
	AMS_MUTEX_UNLOCK(&chip->lock);

	if (ret < 0)
	    return -EINVAL;

	if (reg_irq)
           enable = 1;
	else
	   enable = 0;
	return snprintf(buf, PAGE_SIZE, "%d\n", enable);
}

static ssize_t tsl2540_irq_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	int flag, reg_irq, ret;
        struct tsl2540_chip *chip = dev_get_drvdata(dev);

        if (sscanf(buf, "%d", &flag) != 1)
                return -EINVAL;

	if (flag)
	    reg_irq = 0x90; //enable both saturated and normal interrupt
	else
	    reg_irq = 0; //disable

	AMS_MUTEX_LOCK(&chip->lock);
	ret = ams_i2c_write(chip->client, chip->shadow, TSL2540_REG_INTENAB, reg_irq);
	AMS_MUTEX_UNLOCK(&chip->lock);

	if (ret < 0)
	    return -EINVAL;
	else
	    return size;
}

static DEVICE_ATTR(irq, 0644, tsl2540_irq_show, tsl2540_irq_store);

static ssize_t tsl2540_irq_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
        int ret;
	u8 status;
        struct tsl2540_chip *chip = dev_get_drvdata(dev);

	AMS_MUTEX_LOCK(&chip->lock);
	ret = ams_i2c_read(chip->client, TSL2540_REG_STATUS, &status);
	AMS_MUTEX_UNLOCK(&chip->lock);
	if (ret < 0)
	    return -EINVAL;
	else {
	    if (status & 0x90)
	        return snprintf(buf, PAGE_SIZE, "%d\n", 1);
	    else
	        return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

}

static DEVICE_ATTR(irq_status, 0444, tsl2540_irq_status_show, NULL);

#endif /* #ifdef ABI_SET_GET_REGISTERS */
static int tsl2540_get_id(struct tsl2540_chip *chip, u8 *id, u8 *rev, u8 *auxid)
{
	ams_i2c_read(chip->client, TSL2540_REG_AUXID, auxid);
	ams_i2c_read(chip->client, TSL2540_REG_REVID, rev);
	ams_i2c_read(chip->client, TSL2540_REG_ID, id);

	return 0;
}

#ifdef TSL2540_ENABLE_INPUT
static int tsl2540_power_on(struct tsl2540_chip *chip)
{
	int rc;

	rc = tsl2540_pltf_power_on(chip);
	if (rc)
		return rc;
	dev_info(&chip->client->dev, "%s: chip was off, restoring regs\n",
			__func__);
	return tsl2540_flush_regs(chip);
}

static int tsl2540_als_idev_open(struct input_dev *idev)
{
	struct tsl2540_chip *chip = dev_get_drvdata(&idev->dev);
	bool als = chip->p_idev && chip->p_idev->users;
	int rc = 0;

	dev_info(&idev->dev, "%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	if (chip->unpowered) {
		rc = tsl2540_power_on(chip);
		if (rc)
			goto chip_on_err;
	}
	rc = tsl2540_configure_als_mode(chip, 1);
	if (rc && !als)
		tsl2540_pltf_power_off(chip);
chip_on_err:
	AMS_MUTEX_UNLOCK(&chip->lock);
	return 0;
}

static void tsl2540_als_idev_close(struct input_dev *idev)
{
	struct tsl2540_chip *chip = dev_get_drvdata(&idev->dev);

	dev_info(&idev->dev, "%s\n", __func__);

	AMS_MUTEX_LOCK(&chip->lock);
	tsl2540_configure_als_mode(chip, 0);
	if (!chip->p_idev || !chip->p_idev->users)
		tsl2540_pltf_power_off(chip);
	AMS_MUTEX_UNLOCK(&chip->lock);
}
#endif /* TSL2540_ENABLE_INPUT */

#ifdef CONFIG_OF
//read parameters from the device tree
int tsl2540_init_dt(struct tsl2540_i2c_platform_data *pdata)
{
	struct device_node *np = pdata->of_node;
	const char *str;
	u32 val;
//	s32 ival;

	if (!pdata->of_node)
		return 0;

	if (!of_property_read_string(np, "als_name", &str))
		pdata->als_name = str;

	if (!of_property_read_u32(np, "persist", &val))
		pdata->parameters.persist = val;

	if (!of_property_read_u32(np, "als_gain", &val))
		pdata->parameters.als_gain = val;

	if (!of_property_read_u32(np, "als_gain_factor", &val))
		pdata->parameters.als_gain_factor = val;

	if (!of_property_read_u32(np, "als_deltap", &val))
		pdata->parameters.als_deltap = val;

	//if (!of_property_read_u32(np, "als_time", &val))
	//	pdata->parameters.als_time = val;

	pdata->parameters.als_time = 0x22; //overwritten dbt

	//if (!of_property_read_u32(np, "d_factor", &val))
	//	pdata->parameters.d_factor = val;

	//if (!of_property_read_u32(np, "ch0_coef0", &val))
	//	pdata->parameters.lux_segment[0].ch0_coef = val;

	//if (!of_property_read_u32(np, "ch1_coef0", &val))
	//	pdata->parameters.lux_segment[0].ch1_coef = val;

	//if (!of_property_read_u32(np, "ch0_coef1", &val))
	//	pdata->parameters.lux_segment[1].ch0_coef = val;

	//if (!of_property_read_s32(np, "ch1_coef1", &ival))
	//	pdata->parameters.lux_segment[1].ch1_coef = ival;

	//if (!of_property_read_u32(np, "az_iterations", &val))
	pdata->parameters.az_iterations = 32;

	if (!of_property_read_u32(np, "als_can_wake", &val))
		pdata->als_can_wake = (val == 0) ? false : true;

	pdata->boot_on = of_property_read_bool(np, "boot-on");

	return 0;
}
#endif

#ifdef CONFIG_OF
static const struct of_device_id tsl2540_i2c_dt_ids[] = {
		{ .compatible = "amstaos,tsl2540" },
		{}
};
MODULE_DEVICE_TABLE(of, tsl2540_i2c_dt_ids);
#endif

#ifdef CONFIG_AMS_ADJUST_WITH_BASELINE
#define ALS_LUX_400 400
#define ALS_LUX_20  20
/*
 *  Calibration format is
 *    <stack_id>:<ch0_reading_0>,<actual_lux_0>,<ch1_reading_0>:
 *    <ch0_reading_20>,<actual_lux 20>,<ch1_reading_20>:<ch0_reading_400>,
 *    <actual_lux_400>,<ch1_reading_400>
 *    where all values are 16bit integers.
 *    e.g. For calibration at input lux=0 and lux=400 is
 *    0:1,2,1:120,13,23:670,399,450
 */
static void tsl2540_get_calibration(struct tsl2540_chip *chip)
{
	struct tsl2540_i2c_platform_data *pdata = chip->pdata;
	struct device *dev = &chip->client->dev;
	u8 cal_data[EEPROM_CAL_DATA_LEN], stack_id;
	int ch0_raw_lux0, ch1_raw_lux0, lux_0, ch0_raw_lux20, ch1_raw_lux20, lux_20;
	int ch0_raw_lux400, ch1_raw_lux400, lux_400;
	int ret, i;

	/* FIXME change to use config_name rather then stack ID for ABM type*/
        stack_id = tsl2540_check_device_id(chip);
	ret = firetv_read_calibration_data(dev, cal_data, EEPROM_CAL_DATA_LEN);
	if (ret <= 0) {
	    pr_warn("tsl2540: failed to load calibration data from EEPROM! retry...\n");
	    for (i = 0; i < 3; i++ ){
               ret = firetv_read_calibration_data(dev, cal_data, EEPROM_CAL_DATA_LEN);
	       mdelay(50);
	       if (ret > 0)
                   break;
	    }
	}
        if (ret <= 0)
	    goto failed;

	/* if the ABM module has STACK_ID programmed then it should be used to identify the module
	 * this is not the case now so has to use config_name to match the ABM module this will stop
	 * working if DVT ABM module is put on HVT device or vice versa. Ignore the STACK_ID fron
	 * EEPROM.
	 */

	if ( stack_id > 1 ) //only support two groups of coefficents HVT and DVT.
	     goto failed;

	//stack_id = 1; //remove it to add to support HVT
	chip->als_inf.stack_id = stack_id;
	ch0_raw_lux0 = (int) (cal_data[1] << 8 | cal_data[2]);
	lux_0 = (int) (cal_data[3] << 8 | cal_data[4]);
        ch1_raw_lux0 = (int) (cal_data[5] << 8 | cal_data[6]);
	ch0_raw_lux20 = (int) (cal_data[7] << 8 | cal_data[8]);
	lux_20= (int) (cal_data[9] << 8 | cal_data[10]);
	ch1_raw_lux20 = (int) (cal_data[11] << 8 | cal_data[12]);
	ch0_raw_lux400 = (int) (cal_data[13] << 8 | cal_data[14]);
	lux_400 = (int) (cal_data[15] << 8 | cal_data[16]);
	ch1_raw_lux400 = (int) (cal_data[17] << 8 | cal_data[18]);
	if ( ch0_raw_lux400 == 0 || ch1_raw_lux400 == 0 || ch0_raw_lux400 == 0xFFFF || ch1_raw_lux400 == 0xFFFF)
	    goto failed;

	pdata->lux400_ch0 = ch0_raw_lux400;
	pdata->lux400_ch1 = ch1_raw_lux400;
	pdata->lux400_lux = lux_400;
	pdata->lux20_lux  = lux_20;
	pr_info("als cal data: vir_400 = %d ir_400 = %d.\n", ch0_raw_lux400, ch1_raw_lux400);
	return;
failed:
	/* calibration data is not available */
	pdata->lux400_ch0 = 2500;
	pdata->lux400_ch1 = 130;
	pdata->lux400_lux = ALS_LUX_400;
	pdata->lux20_lux  = ALS_LUX_20;
	chip->als_inf.stack_id  = 1; // if the ABM is not calibrated or an invalid stack_id is detected, set it to be DVT ABM
	pr_info("als cal data: failed to read data from epprom, use default values vis_400 = %d, ir_400 =  %d,\n",
                pdata->lux400_ch0, pdata->lux400_ch1);
}


//FIXME, this function does not work if reg_start is not zero
static int eeprom_write_cal(u8 i2cPort, u8 address, u8 addrsize, u8 reg_start, u32 len, u8 *buf)
{
    int ret, pages, remd, idx, idy;
    u8  reg_addr[EEPROM_PAGE_SIZE];
    u8 *pBuf = buf;

    pages = len / EEPROM_PAGE_SIZE;
    remd  = len % EEPROM_PAGE_SIZE;

    for (idx = 0; idx < pages; idx++) {
        for (idy = 0; idy < EEPROM_PAGE_SIZE; idy++){
	    reg_addr[idy] = idx * EEPROM_PAGE_SIZE + idy + reg_start;
	}
        ret = MDrv_HW_IIC_WriteBytes(i2cPort, address, 1, reg_addr, EEPROM_PAGE_SIZE, pBuf);
	pBuf += EEPROM_PAGE_SIZE;
        if (ret < 0) {
           return (ret);
        }
    }
    for (idx = 0; idx < remd; idx++){
	reg_addr[idx] = pages * EEPROM_PAGE_SIZE + idx + reg_start;
	printk("tsl2540 write the last few bytes of the cal data\n");
    }
    mdelay(100);
    ret = MDrv_HW_IIC_WriteBytes(i2cPort, address, 1, reg_addr, remd, pBuf);
    return (ret);
}

static int firetv_read_calibration_data(struct device *dev, u8 *buf, int len)
{
	int i, ret = 0;
	u8 reg_list[EEPROM_CAL_DATA_LEN], idx = EEPROM_CAL_SKU;

	if (buf == NULL || len < 1 || len > EEPROM_CAL_DATA_LEN)
	{
	   dev_err(dev, "%s: tsl2540, buffer or data length is zero or out of range!\n", __func__);
           ret = -EINVAL;
	}
	else {
		idx *= EEPROM_CAL_DATA_LEN;
	        for (i=0; i < len; i++)
	            reg_list[i] = idx + i;
                ret = MDrv_HW_IIC_ReadBytes(EEPROM_I2C_PORT, ((EEPROM_I2C_ADDRESS << 1) + 1), 1, reg_list, len, buf);
	        if (ret < 0) {
		    dev_err(dev, "%s: tsl2540, failed to read calibration data from eeprom!\n",
				__func__);
	        }
	}
        return ret;
}

static int firetv_write_calibration_data(struct device *dev, u8 *buf, int len)
{
	int ret = 0;

	if (buf == NULL || len < 1 || len > EEPROM_CAL_DATA_LEN)
	{
	   dev_err(dev, "%s: tsl2540, buffer or data length is zero or out of range!\n", __func__);
           ret = -EINVAL;
	}

        ret =  eeprom_write_cal(EEPROM_I2C_PORT, (EEPROM_I2C_ADDRESS << 1), 1, 0, len, buf);
	if (ret < 0) {
		dev_err(dev, "%s: failed to write calibration data!\n",__func__);
	}
        return ret;
}

static int firetv_read_abm_sn(struct device *dev, u8 *buf, int len)
{
	int i, ret = 0;
	u8 msn[EEPROM_MSN_LEN];

	if (buf == NULL || len < 1 || len > EEPROM_MSN_LEN)
	{
	   dev_err(dev, "%s: tsl2540, buffer or data length is zero or out of range!\n", __func__);
           ret = -EINVAL;
	}
	else {
	        for (i=0; i < len; i++)
	            msn[i] = i;
                ret = MDrv_HW_IIC_ReadBytes(EEPROM_I2C_PORT, ((EEPROM_I2C_SND_ADDRESS << 1) + 1), 1, msn, len, buf);
	        if (ret < 0) {
		    dev_err(dev, "%s: tsl2540, failed to read module SN from eeprom!\n",__func__);
	        }
	}
        return ret;
}
static int firetv_write_abm_sn(struct device *dev, u8 *buf, int len)
{
	int i, ret = 0;
	u8 reg_list[EEPROM_MSN_LEN];

	if (buf == NULL || len < 1 || len > EEPROM_MSN_LEN)
	{
	   dev_err(dev, "%s: tsl2540, buffer or data length is zero or out of range!\n", __func__);
           ret = -EINVAL;
	}

	for (i=0; i < len; i++)
	     reg_list[i] = i;
        ret = MDrv_HW_IIC_WriteBytes(EEPROM_I2C_PORT, (EEPROM_I2C_SND_ADDRESS << 1), 1, reg_list, len, buf);
	if (ret < 0) {
		dev_err(dev, "%s: failed to write ABM SN!\n", __func__);
	}
        return ret;
}

static int firetv_lock_abm_sn(struct device *dev)
{
	int ret = 0;
	u8 reg_list[EEPROM_MSN_LEN], buf[2]={0x2,0x0};

	reg_list[0] = 0x40;

        ret = MDrv_HW_IIC_WriteBytes(EEPROM_I2C_PORT, (EEPROM_I2C_SND_ADDRESS << 1), 1, reg_list, 1, buf);
	if (ret < 0) {
		dev_err(dev, "%s: failed to lock identification!\n",__func__);
	}
        return ret;
}

static ssize_t tsl2540_cal_show(struct device *dev, struct device_attribute *attr, char *buf)
{
       struct tsl2540_chip *chip = dev_get_drvdata(dev);
       int ret, num;
       u8 cal_value[EEPROM_CAL_DATA_LEN],tag;

       u16 ch0_raw_lux0, ch1_raw_lux0, lux0;
       u16 ch0_raw_lux20, ch1_raw_lux20, lux20;
       u16 ch0_raw_lux400, ch1_raw_lux400, lux400;


       AMS_MUTEX_LOCK(&chip->lock);
       ret = firetv_read_calibration_data(dev, cal_value, EEPROM_CAL_DATA_LEN);
       if (ret < 0) {
	   dev_err(dev, "%s: failed to read calibration data!\n", __func__);
           num = snprintf(buf, PAGE_SIZE, "0:0,0,0:0,0,0:0,0,0");
       }
       else {
	  tag = (u8) cal_value[0];
	  ch0_raw_lux0 = (u16) (cal_value[1] << 8 | cal_value[2]);
	  lux0 = (u16)(cal_value[3] << 8 | cal_value[4]);
	  ch1_raw_lux0 = (u16) (cal_value[5] << 8 | cal_value[6]);
	  ch0_raw_lux20 = (u16) (cal_value[7] << 8 | cal_value[8]);
	  lux20  = (u16) (cal_value[9] << 8 | cal_value[10]);
	  ch1_raw_lux20= (u16) (cal_value[11] << 8 | cal_value[12]);
	  ch0_raw_lux400 = (u16) (cal_value[13] << 8 | cal_value[14]);
	  lux400  = (u16) (cal_value[15] << 8 | cal_value[16]);
	  ch1_raw_lux400 = (u16) (cal_value[17] << 8 | cal_value[18]);

	  num = snprintf(buf, PAGE_SIZE, "%hhu:%hu,%hu,%hu:%hu,%hu,%hu:%hu,%hu,%hu\n", tag, ch0_raw_lux0,
		       lux0, ch1_raw_lux0, ch0_raw_lux20, lux20, ch1_raw_lux20, ch0_raw_lux400,
	               lux400, ch1_raw_lux400);
       }
       AMS_MUTEX_UNLOCK(&chip->lock);
       return num;
}

static ssize_t tsl2540_cal_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
       struct tsl2540_chip *chip = dev_get_drvdata(dev);
       struct tsl2540_i2c_platform_data *pdata = chip->pdata;

       int ret, num;
       u8 cal_value[EEPROM_CAL_DATA_LEN], tag;

       u16 ch0_raw_lux0, ch1_raw_lux0, lux0;
       u16 ch0_raw_lux20, ch1_raw_lux20, lux20;
       u16 ch0_raw_lux400, ch1_raw_lux400, lux400;


       AMS_MUTEX_LOCK(&chip->lock);
       num = sscanf(buf, "%hhu:%hu,%hu,%hu:%hu,%hu,%hu:%hu,%hu,%hu", &tag, &ch0_raw_lux0,
                          &lux0, &ch1_raw_lux0, &ch0_raw_lux20, &lux20, &ch1_raw_lux20,
			  &ch0_raw_lux400, &lux400, &ch1_raw_lux400 );
       if (num != 10)
	   ret = -EINVAL;
       else {
	     ret = size;
	     cal_value[0] = (u8)tag;
	     cal_value[1] = (u8)((ch0_raw_lux0 >> 8) & 0xFF);
	     cal_value[2] = (u8)(ch0_raw_lux0 & 0xFF);
	     cal_value[3] = (u8)((lux0 >> 8) & 0xFF);
	     cal_value[4] = (u8)(lux0 & 0xFF);
	     cal_value[5] = (u8)((ch1_raw_lux0 >> 8) & 0xFF);
	     cal_value[6] = (u8)(ch1_raw_lux0 & 0xFF);
	     cal_value[7] = (u8)((ch0_raw_lux20 >> 8) & 0xFF);
	     cal_value[8] = (u8)(ch0_raw_lux20 & 0xFF);
	     cal_value[9] = (u8)((lux20 >> 8)  & 0xFF);
	     cal_value[10] = (u8)(lux20 & 0xFF);
	     cal_value[11] = (u8)((ch1_raw_lux20 >> 8) & 0xFF);
	     cal_value[12] = (u8)(ch1_raw_lux20  & 0xFF);
	     cal_value[13] = (u8)((ch0_raw_lux400 >> 8) & 0xFF);
	     cal_value[14] = (u8)(ch0_raw_lux400 & 0xFF);
	     cal_value[15] = (u8)((lux400 >> 8) & 0xFF);
	     cal_value[16] = (u8)(lux400 & 0xFF);
	     cal_value[17] = (u8)((ch1_raw_lux400 >> 8) & 0xFF);
	     cal_value[18] = (u8)(ch1_raw_lux400 & 0xFF);

             ret = firetv_write_calibration_data(dev, cal_value, EEPROM_CAL_DATA_LEN);
       }
       chip->als_inf.stack_id = tag;
       pdata->lux400_ch0 = ch0_raw_lux400;
       pdata->lux400_ch1 = ch1_raw_lux400;
       pdata->lux400_lux = lux400;
       pdata->lux20_lux  = lux20;
       AMS_MUTEX_UNLOCK(&chip->lock);
       if (ret < 0)
	   ret = -EINVAL;
       else
	   ret = size;
       return (ret);
}

static DEVICE_ATTR(cal, 0644, tsl2540_cal_show, tsl2540_cal_store);

static ssize_t tsl2540_abmsn_show(struct device *dev, struct device_attribute *attr, char *buf)
{
       struct tsl2540_chip *chip = dev_get_drvdata(dev);
       int ret, num;
       u8 abm_sn[EEPROM_MSN_LEN + 1];

       AMS_MUTEX_LOCK(&chip->lock);
       ret = firetv_read_abm_sn(dev, abm_sn, EEPROM_MSN_LEN);
       abm_sn[EEPROM_MSN_LEN] = '\0';
       if (ret >= 0)
	   num = snprintf(buf, PAGE_SIZE, "%s\n", abm_sn);
       else
	   num = snprintf(buf, PAGE_SIZE, "%s\n", "failed to read ABM SN");
       AMS_MUTEX_UNLOCK(&chip->lock);
       return (num);
}

static ssize_t tsl2540_abmsn_lock_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
       struct tsl2540_chip *chip = dev_get_drvdata(dev);

       AMS_MUTEX_LOCK(&chip->lock);
       firetv_lock_abm_sn(dev);
       AMS_MUTEX_UNLOCK(&chip->lock);

       return (size);
}

static ssize_t tsl2540_abmsn_lock_show(struct device *dev, struct device_attribute *attr, char *buf)
{
       struct tsl2540_chip *chip = dev_get_drvdata(dev);
       int enable, num;
       u8 abm_sn[EEPROM_MSN_LEN + 1];

       AMS_MUTEX_LOCK(&chip->lock);
       firetv_read_abm_sn(dev, abm_sn, EEPROM_MSN_LEN);
       num = firetv_write_abm_sn(dev, abm_sn, 1);
       AMS_MUTEX_UNLOCK(&chip->lock);
       if (num < 0)
	   enable = 1; //locked
       else
	   enable = 0; //unlocked
       return snprintf(buf, PAGE_SIZE, "%d\n", enable);
}

static ssize_t tsl2540_abmsn_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
       struct tsl2540_chip *chip = dev_get_drvdata(dev);
       int ret, num;
       u8 abm_sn[EEPROM_MSN_LEN];

       AMS_MUTEX_LOCK(&chip->lock);
       ret = sscanf(buf, "%s\n", abm_sn);
       if (ret == 1)
           num = firetv_write_abm_sn(dev, abm_sn, EEPROM_MSN_LEN);
       else
	   num = -1;
       AMS_MUTEX_UNLOCK(&chip->lock);
       if (num < 0)
	   ret = -EINVAL;
       else
	   ret = size;
       return (ret);
}
static DEVICE_ATTR(abm_sn_lock, 0644, tsl2540_abmsn_lock_show, tsl2540_abmsn_lock_store);
static DEVICE_ATTR(abm_sn, 0644, tsl2540_abmsn_show, tsl2540_abmsn_store);
#endif //end of CONFIG_AMS_ADJUST_WITH_BASELINE

static struct attribute *tsl2540_attributes[] = {
	&dev_attr_regs.attr,
#ifdef CONFIG_AMS_ADJUST_WITH_BASELINE
	&dev_attr_irq.attr,
	&dev_attr_irq_status.attr,
	&dev_attr_cal.attr,
	&dev_attr_abm_sn.attr,
	&dev_attr_abm_sn_lock.attr,
#endif
	NULL
};

static const struct attribute_group tsl2540_attr_group = {
	.attrs = tsl2540_attributes,
};

static void tsl2540_setup(struct tsl2540_chip *chip)
{
	struct tsl2540_i2c_platform_data *pdata = chip->pdata;

	if (pdata->boot_on)
		tsl2540_configure_als_mode(chip, 1);
}

static int tsl2540_probe(struct i2c_client *client,
	const struct i2c_device_id *idp)
{
	int i, ret;
	u8 id = 0, rev = 0, auxid = 0;
	struct device *dev = &client->dev;
	static struct tsl2540_chip *chip;
	struct tsl2540_i2c_platform_data *pdata = dev->platform_data;
	bool powered = 0;

	pr_info("\nTSL2540: probe()\n");

#ifdef CONFIG_OF
	if (!pdata) {
		pdata = devm_kzalloc(&client->dev,
				sizeof(struct tsl2540_i2c_platform_data),
				GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;

		if (of_match_device(tsl2540_i2c_dt_ids, &client->dev)) {
			pdata->of_node = client->dev.of_node;
			ret = tsl2540_init_dt(pdata);
			if (ret)
				return ret;
		}
	}
#endif

	/*
	 * Validate bus and device registration
	 */

	dev_info(dev, "%s: client->irq = %d\n", __func__, client->irq);
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(dev, "%s: i2c smbus byte data unsupported\n", __func__);
		ret = -EOPNOTSUPP;
		goto init_failed;
	}
	if (!pdata) {
		dev_err(dev, "%s: platform data required\n", __func__);
		ret = -EINVAL;
		goto init_failed;
	}

	if (!(pdata->als_name) || client->irq < 0) {
		dev_err(dev, "%s: no reason to run.\n", __func__);
		ret = -EINVAL;
		goto init_failed;
	}

	if (pdata->platform_init) {
		ret = pdata->platform_init();
		if (ret)
			goto init_failed;
	}
	if (pdata->platform_power) {
		ret = pdata->platform_power(dev, POWER_ON);
		if (ret) {
			dev_err(dev, "%s: pltf power on failed\n", __func__);
			goto pon_failed;
		}
		powered = true;
		mdelay(10);
	}

	chip = kzalloc(sizeof(struct tsl2540_chip), GFP_KERNEL);
	if (!chip) {
		ret = -ENOMEM;
		goto malloc_failed;
	}

	mutex_init(&chip->lock);
	chip->client = client;
	chip->pdata = pdata;
	i2c_set_clientdata(client, chip);

	//init setup for lux calcualtion
#ifdef CONFIG_AMS_ADJUST_WITH_BASELINE
	tsl2540_get_calibration(chip);
#endif
        pdata->parameters.als_tvis  = als_lux_tvis[chip->als_inf.stack_id];
	pdata->parameters.als_tir   = als_lux_tir[chip->als_inf.stack_id];
	pdata->parameters.als_edge1 = als_lux_edge_1[chip->als_inf.stack_id];
	pdata->parameters.als_edge2 = als_lux_edge_2[chip->als_inf.stack_id];
	memcpy(pdata->parameters.lux_segment, als_lux_cofe[chip->als_inf.stack_id], 3*sizeof(struct tsl2540_lux_segment));
	dev_info(dev, "%s: device setup, stack_id: %d, tvis: %u, tir: %u, edge_1: %u, edge_2: %u\n",
			__func__, chip->als_inf.stack_id, pdata->parameters.als_tvis, pdata->parameters.als_tir, pdata->parameters.als_edge1, pdata->parameters.als_edge2);
	dev_info(dev, "%s: lux_seg[0].ch0_coef: %u, lux_seg[0].ch1_coef: %d, lux_seg[0].dgf: %u\n",
			__func__, pdata->parameters.lux_segment[0].ch0_coef, pdata->parameters.lux_segment[0].ch1_coef, pdata->parameters.lux_segment[0].dgf);
	dev_info(dev, "%s: lux_seg[1].ch0_coef: %u, lux_seg[1].ch1_coef: %d, lux_seg[1].dgf: %u\n",
			__func__, pdata->parameters.lux_segment[1].ch0_coef, pdata->parameters.lux_segment[1].ch1_coef, pdata->parameters.lux_segment[1].dgf);
	dev_info(dev, "%s: lux_seg[2].ch0_coef: %u, lux_seg[2].ch1_coef: %d, lux_seg[2].dgf: %u\n",
			__func__, pdata->parameters.lux_segment[2].ch0_coef, pdata->parameters.lux_segment[2].ch1_coef, pdata->parameters.lux_segment[2].dgf);

	/*
	 * Validate the appropriate ams device is available for this driver
	 */

	tsl2540_get_id(chip, &id, &rev, &auxid);

	dev_info(dev, "%s: device id:%02x device aux id:%02x device rev:%02x\n",
			__func__, id, auxid, rev);

	id &= 0xfc; /* clear the 2 LSbits, they indicate the bus voltage */
	rev &= 0xe7; /* clear all but fuse bits */
	for (i = 0; i < ARRAY_SIZE(tsl2540_ids)/3; i++) {
		if (id == (tsl2540_ids[i*3+0]))
			if (auxid == (tsl2540_ids[i*3+1]))
				if (rev == (tsl2540_ids[i*3+2]))
					break;
	}
	if (i < ARRAY_SIZE(tsl2540_names)) {
		dev_info(dev, "%s: '%s rev. 0x%x' detected\n", __func__,
			tsl2540_names[i], rev);
		chip->device_index = i;
	} else {
		dev_err(dev, "%s: not supported chip id\n", __func__);
		ret = -EOPNOTSUPP;
		goto id_failed;
	}

	/*
	 * Set chip defaults
	 */
	ams_chip = chip; //for other driver to use

	tsl2540_set_defaults(chip);
	ret = tsl2540_flush_regs(chip);
	if (ret)
		goto flush_regs_failed;
	if (pdata->platform_power) {
		pdata->platform_power(dev, POWER_OFF);
		powered = false;
		chip->unpowered = true;
	}

	/*
	 * Initialize ALS
	 */

	if (!pdata->als_name)
		goto bypass_als_idev;

#ifdef TSL2540_ENABLE_INPUT
	chip->a_idev = input_allocate_device();
	if (!chip->a_idev) {
		dev_err(dev, "%s: no memory for input_dev '%s'\n",
				__func__, pdata->als_name);
		ret = -ENODEV;
		goto input_a_alloc_failed;
	}
	chip->a_idev->name = pdata->als_name;
	chip->a_idev->id.bustype = BUS_I2C;
	set_bit(EV_ABS, chip->a_idev->evbit);
	set_bit(ABS_MISC, chip->a_idev->absbit);
	input_set_abs_params(chip->a_idev, ABS_MISC, 0, 65535, 0, 0);
	chip->a_idev->open = tsl2540_als_idev_open;
	chip->a_idev->close = tsl2540_als_idev_close;
	dev_set_drvdata(&chip->a_idev->dev, chip);
	ret = input_register_device(chip->a_idev);
	if (ret) {
		input_free_device(chip->a_idev);
		dev_err(dev, "%s: cant register input '%s'\n",
				__func__, pdata->als_name);
		goto input_a_alloc_failed;
	}
#endif /* TSL2540_ENABLE_INPUT */
bypass_als_idev:

#ifdef ABI_SET_GET_REGISTERS
#ifdef TSL2540_ENABLE_INPUT
	/*
	 * Allocate device
	 */
	chip->d_idev = input_allocate_device();
	if (!chip->d_idev) {
		dev_err(dev, "%s: no memory for input_dev '%s'\n",
				__func__, tsl2540_names[chip->device_index]);
		ret = -ENODEV;
		goto input_d_alloc_failed;
	}

	chip->d_idev->name = tsl2540_names[chip->device_index];
	chip->d_idev->id.bustype = BUS_I2C;
	set_bit(EV_ABS, chip->d_idev->evbit);
	set_bit(ABS_DISTANCE, chip->d_idev->absbit);
	input_set_abs_params(chip->d_idev, ABS_DISTANCE, 0, 1, 0, 0);
	dev_set_drvdata(&chip->d_idev->dev, chip);
	ret = input_register_device(chip->d_idev);
	if (ret) {
		input_free_device(chip->d_idev);
		dev_err(dev, "%s: cant register input '%s'\n",
				__func__, tsl2540_names[chip->device_index]);
		goto input_d_alloc_failed;
	}
#endif /* TSL2540_ENABLE_INPUT */
	ret = sysfs_create_group(&chip->client->dev.kobj, &tsl2540_attr_group);
	if (ret)
		goto input_d_alloc_failed;
#endif /* #ifdef ABI_SET_GET_REGISTERS */

	ret = sysfs_create_group(&chip->client->dev.kobj,
			&tsl2540_als_attr_group);
	if (ret)
		goto base_sysfs_failed;

#ifdef TSL2540_ENABLE_INTERRUPT
	/*
	 * Initialize IRQ & Handler
	 */

	ret = request_threaded_irq(client->irq, NULL, &tsl2540_irq,
		      IRQF_TRIGGER_FALLING|IRQF_SHARED|IRQF_ONESHOT,
		      dev_name(dev), chip);
	if (ret) {
		dev_info(dev, "Failed to request irq %d\n", client->irq);
		goto irq_register_fail;
	}
#endif /* TSL2540_ENABLE_INTERRUPT */

	/* Power up device */
	ams_i2c_write(chip->client, chip->shadow, TSL2540_REG_ENABLE, 0x01);

	tsl2540_update_enable_reg(chip);

	tsl2540_setup(chip);

	dev_info(dev, "Probe ok.\n");
	return 0;

	/*
	 * Exit points for device functional failures (ALS)
	 * This must be unwound in the correct order, reverse
	 * from initialization above
	 */
#ifdef ABI_SET_GET_REGISTERS
base_sysfs_failed:
	sysfs_remove_group(&chip->client->dev.kobj, &tsl2540_attr_group);
#endif /* ABI_SET_GET_REGISTERS */

#ifdef TSL2540_ENABLE_INTERRUPT
irq_register_fail:
	sysfs_remove_group(&chip->client->dev.kobj, &tsl2540_als_attr_group);
#endif /* TSL2540_ENABLE_INTERRUPT */

#ifdef ABI_SET_GET_REGISTERS
#ifdef TSL2540_ENABLE_INPUT
	if (chip->d_idev)
		input_unregister_device(chip->d_idev);
#endif /* TSL2540_ENABLE_INPUT */

input_d_alloc_failed:
#endif /* ABI_SET_GET_REGISTERS */

#ifdef TSL2540_ENABLE_INPUT
	if (chip->a_idev)
		input_unregister_device(chip->a_idev);
input_a_alloc_failed:
	if (chip->p_idev)
		input_unregister_device(chip->p_idev);
#endif /* TSL2540_ENABLE_INPUT */

	/*
	 * Exit points for general device initialization failures
	 */

flush_regs_failed:
id_failed:
	i2c_set_clientdata(client, NULL);
	kfree(chip);
malloc_failed:
	if (powered && pdata->platform_power)
		pdata->platform_power(dev, POWER_OFF);
pon_failed:
	if (pdata->platform_teardown)
		pdata->platform_teardown(dev);
init_failed:
	dev_err(dev, "Probe failed.\n");
	return ret;
}

#ifdef CONFIG_PM_SLEEP
static int tsl2540_suspend(struct device *dev)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	pr_info("\nTSL2540: suspend()\n");
	dev_info(dev, "%s\n", __func__);
	AMS_MUTEX_LOCK(&chip->lock);
	chip->in_suspend = 1;

	if (chip->wake_irq) {
		irq_set_irq_wake(chip->client->irq, 1);
	} else if (!chip->unpowered) {
		dev_info(dev, "powering off\n");
		tsl2540_pltf_power_off(chip);
	}
	AMS_MUTEX_UNLOCK(&chip->lock);

	return 0;
}

static int tsl2540_resume(struct device *dev)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	bool als_on;

	return 0;
	pr_info("\nTSL2540: resume()\n");
	AMS_MUTEX_LOCK(&chip->lock);
	chip->in_suspend = 0;

	dev_info(dev, "%s: powerd %d, als: needed %d  enabled %d",
			__func__, !chip->unpowered, als_on,
			chip->als_enabled);

	if (chip->wake_irq) {
		irq_set_irq_wake(chip->client->irq, 0);
		chip->wake_irq = 0;
	}

/* err_power: */
	AMS_MUTEX_UNLOCK(&chip->lock);

	return 0;
}

static SIMPLE_DEV_PM_OPS(tsl2540_pm_ops, tsl2540_suspend, tsl2540_resume);
#define TSL2540_PM_OPS (&tsl2540_pm_ops)

#else

#define TSL2540_PM_OPS NULL

#endif /* CONFIG_PM_SLEEP */


static int tsl2540_remove(struct i2c_client *client)
{
	struct tsl2540_chip *chip = i2c_get_clientdata(client);

	pr_info("\nTSL2540: REMOVE()\n");
#ifdef TSL2540_ENABLE_INTERRUPT
	free_irq(client->irq, chip);
#endif /* TSL2540_ENABLE_INTERRUPT */
	if (chip->p_idev)
		input_unregister_device(chip->p_idev);
	if (chip->pdata->platform_teardown)
		chip->pdata->platform_teardown(&client->dev);
	i2c_set_clientdata(client, NULL);
#ifdef CONFIG_OF
	kfree(chip->pdata);
#endif
	kfree(chip);
	return 0;
}

static struct i2c_device_id tsl2540_idtable[] = {
	{ "tsl2540", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, tsl2540_idtable);

static struct i2c_driver tsl2540_driver = {
	.driver = {
		.name = "tsl2540",
		.pm = TSL2540_PM_OPS,
	},
	.id_table = tsl2540_idtable,
	.probe = tsl2540_probe,
	.remove = tsl2540_remove,
};

static int __init tsl2540_init(void)
{
	int rc;
	pr_info("\nTSL2540: INIT()\n");

	rc = i2c_add_driver(&tsl2540_driver);
	return rc;
}

static void __exit tsl2540_exit(void)
{
	pr_info("\nTSL2540: exit()\n");
	i2c_del_driver(&tsl2540_driver);
}

module_init(tsl2540_init);
module_exit(tsl2540_exit);

MODULE_DESCRIPTION("AMS-TAOS tsl2540 ALS sensor driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("2.4");
