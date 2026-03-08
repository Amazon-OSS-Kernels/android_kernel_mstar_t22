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

#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <linux/i2c/ams_tsl2540.h>
#include "ams_i2c.h"

//#define LUX_DBG             1
#define CONFIG_AMZN_AMS_ALS   1

#define GAIN_CFG2_HALF (0x00)
#define GAIN_CFG2      (0x04)
#define GAIN_CFG2_128  (0x14)

static u8 const als_gains[] = {
	1,
	4,
	16,
	64,
};


static u8 const restorable_als_regs[] = {
	TSL2540_REG_ATIME,
	TSL2540_REG_WTIME,
	TSL2540_REG_PERS,
	TSL2540_REG_CFG0,
	TSL2540_REG_CFG1,
	TSL2540_REG_CFG2,
	TSL2540_REG_AZ_CONFIG,
};

static int tsl2540_flush_als_regs(struct tsl2540_chip *chip)
{
	unsigned int i;
	int rc;
	u8 reg;

	for (i = 0; i < ARRAY_SIZE(restorable_als_regs); i++) {
		reg = restorable_als_regs[i];
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

int tsl2540_read_als(struct tsl2540_chip *chip)
{
	int ret;
        //u8 tmp_gain;

        // the VIS and IR readback is alway one integration cycle delay, try to read gain register
	// to see if this will help as told by other project.

	//ams_i2c_read(chip->client, TSL2540_REG_CFG1, &tmp_gain);
	//ams_i2c_read(chip->client, TSL2540_REG_CFG2, &tmp_gain);

	//tmp_gain = tmp_gain;// supress warning

	ret = ams_i2c_blk_read(chip->client, TSL2540_REG_CH0DATA,
			&chip->shadow[TSL2540_REG_CH0DATA], 4 * sizeof(u8));

	if (ret >= 0) {
		chip->als_inf.als_ch0 = le16_to_cpu(
			*((const __le16 *) &chip->shadow[TSL2540_REG_CH0DATA]));
		chip->als_inf.als_ch1 = le16_to_cpu(
			*((const __le16 *) &chip->shadow[TSL2540_REG_CH1DATA]));
		ret = 0;
	}

	return ret;
}

int tsl2540_configure_als_mode(struct tsl2540_chip *chip, u8 state)
{
	struct i2c_client *client = chip->client;
	u8 *sh = chip->shadow;

	 /* Turning on ALS */
	if (state) {
		chip->shadow[TSL2540_REG_ATIME] = chip->params.als_time;

		/* set PERS.apers to 2 consecutive ALS values out of range */
		chip->shadow[TSL2540_REG_PERS] &= (~TSL2540_MASK_APERS);
		chip->shadow[TSL2540_REG_PERS] |= 0x02;

		tsl2540_flush_als_regs(chip);

#ifdef TSL2540_ENABLE_INTERRUPT
		ams_i2c_modify(client, sh, TSL2540_REG_INTENAB,
				TSL2540_AIEN, TSL2540_AIEN);
#endif /* TSL2540_ENABLE_INTERRUPT */
		ams_i2c_modify(client, sh, TSL2540_REG_ENABLE,
				TSL2540_WEN | TSL2540_AEN | TSL2540_PON,
				TSL2540_WEN | TSL2540_AEN | TSL2540_PON);
		chip->als_enabled = true;
	} else {
		/* Disable ALS, Wait and ALS Interrupt */
#ifdef TSL2540_ENABLE_INTERRUPT
		ams_i2c_modify(client, sh, TSL2540_REG_INTENAB,
				TSL2540_AIEN, 0);
#endif /* TSL2540_ENABLE_INTERRUPT */
		ams_i2c_modify(client, sh, TSL2540_REG_ENABLE,
				TSL2540_WEN | TSL2540_AEN, 0);
		chip->als_enabled = false;

		/* If nothing else is enabled set PON = 0; */
		if (!(sh[TSL2540_REG_ENABLE] & TSL2540_EN_ALL))
			ams_i2c_modify(client, sh, TSL2540_REG_ENABLE,
			TSL2540_PON, 0);
	}

	return 0;
}

static int tsl2540_set_als_gain(struct tsl2540_chip *chip, int gain)
{
	int rc, inte_cycle = 0;
	u8 cfg1_reg, cfg2_reg;
	u8 saved_enable;

	switch (gain) {
	case 0: // 0.5
	     cfg1_reg = AGAIN_1;
             cfg2_reg = GAIN_CFG2_HALF;
             chip->als_inf.full_gain = 0;
             break;
	case 1:
	     cfg1_reg = AGAIN_1;
             cfg2_reg = GAIN_CFG2;
             chip->als_inf.full_gain = 1;
             break;
	case 4:
             cfg1_reg = AGAIN_4;
	     cfg2_reg = GAIN_CFG2;
             chip->als_inf.full_gain = 4;
             break;
	case 16:
	     cfg1_reg = AGAIN_16;
	     cfg2_reg = GAIN_CFG2;
             chip->als_inf.full_gain = 16;
	     break;
	case 64:
	     cfg1_reg = AGAIN_64;
	     cfg2_reg =GAIN_CFG2;
             chip->als_inf.full_gain = 64;
 	     break;
	case 128:
	     cfg1_reg = AGAIN_64;
	     cfg2_reg = GAIN_CFG2_128;
             chip->als_inf.full_gain = 128;
	     break;
	default:
		dev_err(&chip->client->dev, "%s: wrong als gain %d\n",
				__func__, gain);
		return -EINVAL;
	}

	/*
	 * Turn off ALS, so that new ALS gain value will take
	 * effect at start of new integration cycle.
	 * New ALS gain value will then be used in next lux calculation.
	 */
	ams_i2c_read(chip->client, TSL2540_REG_ENABLE, &saved_enable);
	ams_i2c_write(chip->client, chip->shadow, TSL2540_REG_ENABLE, 0);

	ams_i2c_modify(chip->client, chip->shadow, TSL2540_REG_CFG1,TSL2540_MASK_AGAIN, cfg1_reg);
	ams_i2c_modify(chip->client, chip->shadow, TSL2540_REG_CFG2, (TSL2540_MASK_AGAINL | TSL2540_MASK_AGAINMAX), cfg2_reg);

	ams_i2c_write(chip->client, chip->shadow, TSL2540_REG_ENABLE, saved_enable);

	chip->params.als_gain = chip->shadow[TSL2540_REG_CFG1];
	ams_i2c_read(chip->client, TSL2540_REG_STATUS, &saved_enable); //clear and discard int
	if ( chip->als_inf.full_gain  == 0 )
            dev_info(&chip->client->dev, "%s: new als gain is 0.5.\n", __func__);
	else
	    dev_info(&chip->client->dev, "%s: new als gain is %d\n", __func__, chip->als_inf.full_gain);

	//need to wait for one integration cycle afater a gain changing
	inte_cycle = chip->shadow[TSL2540_REG_ATIME] + 1;
	inte_cycle *= INTEGRATION_CYCLE;
	inte_cycle += 500; //rounding
	inte_cycle /= 1000; //to ms
        msleep(inte_cycle);

	return rc;
}

static int tsl2540_inc_gain(struct tsl2540_chip *chip)
{
	u8 gain = 0;

	if ( chip->als_inf.full_gain == 128 ){
	    //dev_info(&chip->client->dev,"%s: gain is at maxmail 128.\n", __func__);
	    return 1;
	}
	else if (chip->als_inf.full_gain == 64 ) {
            gain = 128;
	}
	else if (chip->als_inf.full_gain == 16 ) {
            gain = 64;
	}
	else if (chip->als_inf.full_gain == 4 ) {
            gain = 16;
	}
	else if (chip->als_inf.full_gain == 1 ) {
            gain = 4;
	}
	else {
            gain = 1;
	}
	tsl2540_set_als_gain(chip, gain);
        tsl2540_flush_als_regs(chip);
	return 0;
}

static int tsl2540_dec_gain(struct tsl2540_chip *chip)
{
	u8 gain = 0;

	if ( chip->als_inf.full_gain == 0 ){
	    //dev_info(&chip->client->dev,"%s: gain is at minimal 0.5.\n", __func__);
	    return 1;
	}
	else if (chip->als_inf.full_gain == 128 ) {
            gain = 64;
	}
	else if (chip->als_inf.full_gain == 64 ) {
            gain = 16;
	}
	else if (chip->als_inf.full_gain == 16 ) {
            gain = 4;
	}
	else if (chip->als_inf.full_gain == 4 ) {
            gain = 1;
	}
	else {
            gain = 0;
	}

	tsl2540_set_als_gain(chip, gain);
        tsl2540_flush_als_regs(chip);
	return 0;
}

static int tsl2540_gain_remapping(struct tsl2540_chip *chip)
{
   //als gain register value need to be remapped for
   //Lux equation, the gain is also scaled by 10 for fixed point
   //algorithm

   int gain  = 0;
   if (chip == NULL){
       pr_err("can't remap gain, chip is NULL!\n");
       return gain;
   }

   if ( (chip->params.als_gain == AGAIN_1) && (chip->shadow[TSL2540_REG_CFG2] == 0) )
        gain = 5; // 0.5
   else if ( chip->params.als_gain == AGAIN_1 )
        gain = 10;
   else if ( chip->params.als_gain == AGAIN_4 )
        gain = 40;
   else if ( chip->params.als_gain == AGAIN_16 )
        gain = 160;
   else if ( (chip->params.als_gain == AGAIN_64) && (chip->shadow[TSL2540_REG_CFG2] == 0x14) )
        gain = 1400;
   else
        gain = 670;
   return gain;
}

static int tsl2540_max_als_value(struct tsl2540_chip *chip)
{
	int val;

	val = chip->shadow[TSL2540_REG_ATIME];
	val++;
	if (val > 63)
		val = 0xffff;
	else
		val = ((val * 1024) - 1);
	return val;
}

int tsl2540_get_lux(struct tsl2540_chip *chip)
{
	u16 ch0, ch1, lux;
        u32 atime, visfc, irfc, ratio;
	int64_t lux_tmp, coef_visfc, coef_irfc, tmp;
	int  idx = 0, gain;
	u8   satu_flag = 0, int_status = 0;

#ifdef CONFIG_AMS_ADJUST_WITH_BASELINE
	struct tsl2540_i2c_platform_data *pdata = chip->pdata;
#endif

        chip->is_als_valid = 1;
	ams_i2c_read(chip->client, TSL2540_REG_STATUS, &chip->shadow[TSL2540_REG_STATUS]);
	int_status = chip->shadow[TSL2540_REG_STATUS];
	satu_flag = int_status & TSL2540_ST_ALS_SAT;

	if (pdata->lux400_ch0 == 0 || pdata->lux400_ch1 == 0 ) {
	    dev_err_ratelimited(&chip->client->dev,"%s: AMB is not calibrated, can't calculate lux value.\n", __func__);
	    return 0;
	}
	ch0 = (u32)chip->als_inf.als_ch0;
	ch1 = (u32)chip->als_inf.als_ch1;

	/* tvis and tir are multipled by 10000 */
	lux_tmp  = (uint64_t)ch0 * (uint64_t)chip->params.als_tvis / (uint64_t)pdata->lux400_ch0; //calibration data
	visfc    = (u32)lux_tmp;
	lux_tmp  = (uint64_t)ch1 * (uint64_t)chip->params.als_tir / (uint64_t)pdata->lux400_ch1;
	irfc     = (u32)lux_tmp;
#ifdef LUX_DBG
	dev_info(&chip->client->dev,"%s: visfc is: %d, irfc is: %d.\n", __func__, visfc, irfc);
#endif
	if ( visfc > 0 ) {
	    tmp = (int64_t)irfc * 1000 / (int64_t)visfc;
            ratio = (u32)tmp;
            if (ratio < chip->params.als_edge1 )
                idx = 0;
	    else if (ratio < chip->params.als_edge2 )
	        idx = 1;
	    else
	        idx = 2;

	    // this is the final lux eqaution change dated 3/21 for old ABM. No need to scale down the lux value
	    // the reading is accurate based on the simulatoin using HVT data.
	    atime = (chip->params.als_time + 1)*INTEGRATION_CYCLE;
	    gain =  tsl2540_gain_remapping(chip);
            if ( atime > 0 ) {
		coef_visfc = (int64_t)chip->params.lux_segment[idx].ch0_coef * (int64_t)visfc;
	        coef_irfc  = (int64_t)chip->params.lux_segment[idx].ch1_coef * (int64_t)irfc;
		lux_tmp    = (coef_visfc + coef_irfc) / (int64_t)atime;
		lux_tmp    = lux_tmp * (int64_t)chip->params.lux_segment[idx].dgf / (int64_t)gain;
                lux_tmp = (lux_tmp + 5000000000) / 10000000000;
	        lux = (u16)(lux_tmp);
#ifdef LUX_DBG
	        dev_info(&chip->client->dev,"%s: coef_visfc : %lld, coef_irfc: %lld, lux_tmp: %lld, gain: %d, lux: %d.\n", __func__, coef_visfc, coef_irfc, lux_tmp, gain, lux);
#endif
	    }
	    else{
	        dev_err_ratelimited(&chip->client->dev,"%s: ALS atime is zero, use prve value.\n", __func__);
	        lux = chip->als_inf.lux;
	    }
	}
	else
           lux = 0;
#ifdef LUX_DBG
	dev_info(&chip->client->dev,
		"%s: lux:%d [%d, %d, %d, %d] %u [%u %d %u] [%u %u] %u [%u, ((%u*%u)\n",
		__func__, lux,
		ch0, ch1, visfc, irfc,
		chip->params.als_time,
		chip->params.lux_segment[idx].ch0_coef,
		chip->params.lux_segment[idx].ch1_coef,
		chip->params.lux_segment[idx].dgf,
		chip->params.als_tvis,
		chip->params.als_tir,
		chip->shadow[TSL2540_REG_ATIME],
		als_gains[(chip->params.als_gain & TSL2540_MASK_AGAIN)],
		INTEGRATION_CYCLE,
		chip->als_inf.saturation);
#endif /* #ifdef LUX_DBG */
       if (lux < 0) {
	  chip->als_inf.lux = 0;
	  chip->is_als_valid = 0;
	  dev_err_ratelimited(&chip->client->dev, "%s: lux < 0, lux:%d [%d, %d, %d, %d] %u [%u %d %u] [%u %u] %u [%u, ((%u*%u)\n",
		__func__, lux,
		ch0, ch1, visfc, irfc,
		chip->params.als_time,
		chip->params.lux_segment[idx].ch0_coef,
		chip->params.lux_segment[idx].ch1_coef,
		chip->params.lux_segment[idx].dgf,
		chip->params.als_tvis,
		chip->params.als_tir,
		chip->shadow[TSL2540_REG_ATIME],
		als_gains[(chip->params.als_gain & TSL2540_MASK_AGAIN)],
		INTEGRATION_CYCLE,
		chip->als_inf.saturation);
   	  return 1;
        }

	chip->als_inf.lux = (u16)lux;

	/* ..... Not in Autogain ........... */
	if (!chip->als_gain_auto) {
		if ((ch0 <= TSL2540_MIN_ALS_VALUE) ||
		    (ch1 <= TSL2540_MIN_ALS_VALUE)) {
			chip->als_inf.lux = 0;
                        dev_info_ratelimited(&chip->client->dev, "%s: ch0 or ch1 is too lowe, lux value invalid.\n", __func__);
			return 1;
		}
		if ((ch0 >= chip->als_inf.saturation) ||
                    (ch1 >= chip->als_inf.saturation) || 
		    (satu_flag)) {
                        dev_info_ratelimited(&chip->client->dev, "%s: ch0 or ch1 is saturated to the maximal value, or the corner case happend, lux invalid.\n", __func__);
			chip->als_inf.lux = 65535;
			chip->is_als_valid = 0;
			return 1;
		}
		return 0;
	}
	/* ..... In Autogain ..........*/
        /* in case the ASTA bit is set while the raw counts are low and in the range
	 * to increase the gain. This will cause a loop between ASTA is set and gain needs to be
	 * decreased and then increased agian due to the raw count reading is low
	 * the mitigation is to reduce the high or low threshold so that after the gain is lowered by one
	 * step, it will not cause a gain increase. The hight or low threshold will set back after the other
	 * gain increase by non SATA reason. Both methods are tested working but it looks lower the
	 * high threshold is better.
	 */

	/* method one, lower the high threshold */
        if (satu_flag) {
	   if ((ch0 < chip->als_inf.saturation) && (ch1 < chip->als_inf.saturation)){
		if (chip->als_inf.als_ch0 > chip->als_inf.als_ch1)
	            chip->als_inf.high_thrs = 9 * (u32)chip->als_inf.als_ch0 / 40;
		else
	            chip->als_inf.high_thrs = 9 * (u32)chip->als_inf.als_ch1 / 40;
	   }
	   if ( 1 == tsl2540_dec_gain(chip) ){
               chip->als_inf.lux = 65534; //gain is at minimal
               dev_info_ratelimited(&chip->client->dev, "%s: ASTA is still set 0x%02x even the gain is at minimal %d.\n",
				    __func__,  int_status, chip->params.als_gain);
	       return 1;
	   }
           chip->is_als_valid = 0;
	   return 0;
	}
        if ((ch0 < chip->als_inf.low_thrs && (ch1 < chip->als_inf.high_thrs)) ||
	    (ch1 < chip->als_inf.low_thrs && (ch0 < chip->als_inf.high_thrs))) {
	   chip->als_inf.high_thrs = chip->als_inf.saturation / 4;
           if (1 == tsl2540_inc_gain(chip))
	      return 1; //gain is at maximal can't be moved up
	   chip->is_als_valid = 0;
	   return 0;
	}
        if ((ch0 > chip->als_inf.saturation) || (ch1 > chip->als_inf.saturation)) {
	   chip->is_als_valid = 0;
	   chip->als_inf.high_thrs = chip->als_inf.saturation / 4;
	   if ( 1 == tsl2540_dec_gain(chip)){
               chip->als_inf.lux = 65535; //gain is at minimal
	       chip->is_als_valid = 1;
               dev_info_ratelimited(&chip->client->dev, "%s: ch0 or ch1 still saturated %d at lowest gain %d, lux invalid.\n",
				    __func__, chip->als_inf.saturation, chip->params.als_gain);
	       return 1;
	   }
	}

	/* method one, lower the high threshold */
	/***************
	if ( satu_flag ) {
	   if ((ch0 < chip->als_inf.saturation) && (ch1 < chip->als_inf.saturation)){
	       chip->als_inf.low_thrs /= 2;
	   }
	   if ( 1 == tsl2540_dec_gain(chip) ){
               chip->als_inf.lux = 65534; //gain is at minimal
               dev_info_ratelimited(&chip->client->dev, "%s: ASTA is still set 0x%02x even the gain is at minimal %d.\n",
				    __func__,  int_status, chip->params.als_gain);
	       return 1;
	   }
           chip->is_als_valid = 0;
	   return 0;
	}

       	if ((ch0 < chip->als_inf.low_thrs && (ch1 < chip->als_inf.high_thrs)) ||
	    (ch1 < chip->als_inf.low_thrs && (ch0 < chip->als_inf.high_thrs))) {
	   chip->als_inf.high_thrs *= 2;
	   if (chip->als_inf.low_thrs > (tsl2540_max_als_value(chip) / 200))
	       chip->als_inf.low_thrs = tsl2540_max_als_value(chip) / 200;
           if (1 == tsl2540_inc_gain(chip))
	      return 1; //gain is at maximal can't be moved up
	   chip->is_als_valid = 0;
	   return 0;
	}

	if ((ch0 > chip->als_inf.saturation) || (ch1 > chip->als_inf.saturation)) {
	   chip->is_als_valid = 0;
	   chip->als_inf.low_thrs = tsl2540_max_als_value(chip) / 200;
	   if ( 1 == tsl2540_dec_gain(chip)){
               chip->als_inf.lux = 65535; //gain is at minimal
	       chip->is_als_valid = 1;
               dev_info_ratelimited(&chip->client->dev, "%s: ch0 or ch1 still saturated %d at lowest gain %d, lux invalid.\n",
				    __func__, chip->als_inf.saturation, chip->params.als_gain);
	       return 1;
	   }
	}
	********************/
	return 0; //used in intterupt mode to adjust threshold
}

int tsl2540_update_als_thres(struct tsl2540_chip *chip, bool on_enable)
{
	s32 ret;
	u16 deltap = chip->params.als_deltap;
	u16 from, to, cur;
	u16 saturation = chip->als_inf.saturation;

	cur = chip->als_inf.als_ch0;

	if (on_enable) {
		/* move deltap far away from current position to force an irq */
		from = to = cur > (saturation / 2) ? 0 : saturation;
	} else {
		deltap = cur * deltap / 100;
		if (!deltap)
			deltap = 1;

		if (cur > deltap)
			from = cur - deltap;
		else
			from = 0;

		if (cur < (saturation - deltap))
			to = cur + deltap;
		else
			to = saturation;
	}

	*((__le16 *) &chip->shadow[TSL2540_REG_AILT]) = cpu_to_le16(from);
	*((__le16 *) &chip->shadow[TSL2540_REG_AIHT]) = cpu_to_le16(to);

	dev_info(&chip->client->dev,
			"%s: low:0x%x  hi:0x%x, oe:%d cur:%d deltap:%d (%d) sat:%d\n",
			__func__, from, to, on_enable, cur, deltap,
			chip->params.als_deltap, saturation);

	ret = ams_i2c_reg_blk_write(chip->client, TSL2540_REG_AILT,
			&chip->shadow[TSL2540_REG_AILT],
			(TSL2540_REG_AIHT_HI - TSL2540_REG_AILT) + 1);

	return (ret < 0) ? ret : 0;
}

void tsl2540_report_als(struct tsl2540_chip *chip)
{
	int lux;
	int rc;

	if (chip->a_idev) {
		rc = tsl2540_get_lux(chip);
		if (!rc) {
			lux = chip->als_inf.lux;
			input_report_abs(chip->a_idev, ABS_MISC, lux);
			input_sync(chip->a_idev);
			tsl2540_update_als_thres(chip, 0);
		} else {
			tsl2540_update_als_thres(chip, 1);
		}
	}
}

/*
 * ABI Functions
 */

static ssize_t tsl2540_device_als_lux(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = 0;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	AMS_MUTEX_LOCK(&chip->lock);
	//lux value can be invalid and in this case
	//gain need to be adjusted and then recaculate the lux
	//if an valid lux can't be obtained then return
	//65535 or the maximal LUX
	if (!chip->als_gain_auto) {
	    tsl2540_read_als(chip);
	    tsl2540_get_lux(chip);
	}
	else {
	    for (idx = 0; idx < 10; idx++) {
	        tsl2540_read_als(chip);
	        tsl2540_get_lux(chip);
	        if (chip->is_als_valid)
		    break;
	        if (chip->als_inf.lux == 65535)
		   break;
	   }
	}
	AMS_MUTEX_UNLOCK(&chip->lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", chip->als_inf.lux);
}

static DEVICE_ATTR(als_lux, 0444, tsl2540_device_als_lux, NULL);

static ssize_t tsl2540_lux_table_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	int k;

	AMS_MUTEX_LOCK(&chip->lock);

	k = snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d\n",
		chip->params.als_gain,
		chip->params.lux_segment[0].ch0_coef,
		chip->params.lux_segment[0].ch1_coef,
		chip->params.lux_segment[0].dgf);

	AMS_MUTEX_UNLOCK(&chip->lock);

	return k;
}

static ssize_t tsl2540_lux_table_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	u32 d_factor;
        int ch0_coef1, ch1_coef1, dgf;

	if (sscanf(buf, "%10d,%d,%d,%d", &d_factor,
			&ch0_coef1, &ch1_coef1, &dgf) != 4)
		return -EINVAL;

	AMS_MUTEX_LOCK(&chip->lock);

	chip->params.lux_segment[d_factor].ch0_coef = ch0_coef1;
	chip->params.lux_segment[d_factor].ch1_coef = ch1_coef1;
	chip->params.lux_segment[d_factor].dgf = dgf;

	AMS_MUTEX_UNLOCK(&chip->lock);
	return size;
}

static DEVICE_ATTR(als_lux_table, 0644, tsl2540_lux_table_show,
			tsl2540_lux_table_store);

static ssize_t tsl2540_als_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", chip->als_enabled);
}

static ssize_t tsl2540_als_enable_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	bool value;

	if (strtobool(buf, &value))
		return -EINVAL;

	if (value)
		tsl2540_configure_als_mode(chip, 1);
	else
		tsl2540_configure_als_mode(chip, 0);

	return size;
}

static DEVICE_ATTR(als_power_state, 0644, tsl2540_als_enable_show,
			tsl2540_als_enable_store);

static ssize_t tsl2540_auto_gain_enable_show(
		struct device *dev, struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%s\n",
			chip->als_gain_auto ? "auto" : "manual");
}

static ssize_t tsl2540_auto_gain_enable_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	bool value;

	if (strtobool(buf, &value))
		return -EINVAL;

	if (value)
		chip->als_gain_auto = true;
	else
		chip->als_gain_auto = false;

	return size;
}

static DEVICE_ATTR(als_auto_gain, 0644,
		tsl2540_auto_gain_enable_show, tsl2540_auto_gain_enable_store);

static ssize_t tsl2540_als_gain_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	int gain_ex = chip->shadow[TSL2540_REG_CFG2] & (TSL2540_MASK_AGAINL | TSL2540_MASK_AGAINMAX);
	int gain = chip->shadow[TSL2540_REG_CFG1] & TSL2540_MASK_AGAIN;

	if (gain == AGAIN_1 && gain_ex == GAIN_CFG2_HALF){
	    return snprintf(buf, PAGE_SIZE, "%s (%s)\n",
			    "0.5", chip->als_gain_auto ? "auto" : "manual");
	}
	else if (gain == AGAIN_64 && gain_ex == GAIN_CFG2_128){
	    return snprintf(buf, PAGE_SIZE, "%d (%s)\n",
			    128, chip->als_gain_auto ? "auto" : "manual");
	}
        else {
	   return snprintf(buf, PAGE_SIZE, "%d (%s)\n",
			als_gains[(chip->params.als_gain & TSL2540_MASK_AGAIN)],
			chip->als_gain_auto ? "auto" : "manual");
	}
}

static ssize_t tsl2540_als_gain_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	unsigned long gain;
	int rc;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	rc = kstrtoul(buf, 10, &gain);

	if (rc)
		return -EINVAL;
	if (gain != 0 && gain != 1 && gain != 4 && gain != 16 && gain != 64 && gain != 128)
		return -EINVAL;


	AMS_MUTEX_LOCK(&chip->lock);

	rc = tsl2540_set_als_gain(chip, gain);

	AMS_MUTEX_UNLOCK(&chip->lock);

	return rc ? -EIO : size;
}

static DEVICE_ATTR(als_gain, 0644, tsl2540_als_gain_show,
			tsl2540_als_gain_store);

static ssize_t tsl2540_als_persist_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			(((chip->shadow[TSL2540_REG_PERS]) &
					TSL2540_MASK_APERS)));
}

static ssize_t tsl2540_als_persist_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	long persist;
	int rc;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	rc = kstrtoul(buf, 10, &persist);
	if (rc)
		return -EINVAL;

	AMS_MUTEX_LOCK(&chip->lock);
	chip->shadow[TSL2540_REG_PERS] &= ~TSL2540_MASK_APERS;
	chip->shadow[TSL2540_REG_PERS] |= ((u8)persist & TSL2540_MASK_APERS);

	tsl2540_flush_als_regs(chip);

	AMS_MUTEX_UNLOCK(&chip->lock);
	return size;
}

static DEVICE_ATTR(als_persist, 0644, tsl2540_als_persist_show,
			tsl2540_als_persist_store);

static ssize_t tsl2540_als_itime_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	int t;

	t = chip->shadow[TSL2540_REG_ATIME];
	t++;
	t *= INTEGRATION_CYCLE;
	return snprintf(buf, PAGE_SIZE, "%dms (%dus)\n", t / 1000, t);
}

static ssize_t tsl2540_als_itime_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	long itime;
	int rc;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	rc = kstrtoul(buf, 10, &itime);
	if (rc)
		return -EINVAL;
	itime *= 1000;
	itime /= INTEGRATION_CYCLE;

	AMS_MUTEX_LOCK(&chip->lock);

	itime--;
	chip->shadow[TSL2540_REG_ATIME] = (u8) itime;
	chip->params.als_time = chip->shadow[TSL2540_REG_ATIME];
	//adjust the stautation value
	chip->als_inf.saturation = ((chip->params.als_time + 1) * 1024 - 1) * 9 /10;
	tsl2540_flush_als_regs(chip);

	AMS_MUTEX_UNLOCK(&chip->lock);

	return size;
}

static DEVICE_ATTR(als_itime, 0644, tsl2540_als_itime_show,
			tsl2540_als_itime_store);

static ssize_t tsl2540_als_wtime_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int t;
	u8 wlongcurr;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	AMS_MUTEX_LOCK(&chip->lock);

	t = chip->shadow[TSL2540_REG_WTIME];

	wlongcurr = chip->shadow[TSL2540_REG_CFG0] & TSL2540_MASK_WLONG;
	t++;
	if (wlongcurr)
		t *= 12;

	t *= INTEGRATION_CYCLE;
	t /= 1000;

	AMS_MUTEX_UNLOCK(&chip->lock);

	return snprintf(buf, PAGE_SIZE, "%d (in ms)\n", t);
}

static ssize_t tsl2540_als_wtime_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	unsigned long wtime;
	int wlong;
	int rc;

	rc = kstrtoul(buf, 10, &wtime);
	if (rc)
		return -EINVAL;

	wtime *= 1000;
	if (wtime > (256 * INTEGRATION_CYCLE)) {
		wlong = 1;
		wtime /= 12;
	} else {
		wlong = 0;
	}
	wtime /= INTEGRATION_CYCLE;

	AMS_MUTEX_LOCK(&chip->lock);

	chip->shadow[TSL2540_REG_WTIME] = (u8) wtime;
	if (wlong)
		chip->shadow[TSL2540_REG_CFG0] |= TSL2540_MASK_WLONG;
	else
		chip->shadow[TSL2540_REG_CFG0] &= ~TSL2540_MASK_WLONG;

	tsl2540_flush_als_regs(chip);

	AMS_MUTEX_UNLOCK(&chip->lock);
	return size;
}

static DEVICE_ATTR(als_wtime, 0644, tsl2540_als_wtime_show,
			tsl2540_als_wtime_store);

static ssize_t tsl2540_als_deltap_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE,
			"%d (in %%)\n", chip->params.als_deltap);
}

static ssize_t tsl2540_als_deltap_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	unsigned long deltap;
	int rc;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	rc = kstrtoul(buf, 10, &deltap);
	if (rc || deltap > 100)
		return -EINVAL;

	AMS_MUTEX_LOCK(&chip->lock);
	chip->params.als_deltap = deltap;
        tsl2540_update_als_thres(chip, 0);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return size;
}

static DEVICE_ATTR(als_thresh_deltap, 0644,
			tsl2540_als_deltap_show, tsl2540_als_deltap_store);

static ssize_t tsl2540_als_ch0_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	tsl2540_read_als(chip);

	return snprintf(buf, PAGE_SIZE, "%d\n", chip->als_inf.als_ch0);
}

static DEVICE_ATTR(als_ch0, 0444, tsl2540_als_ch0_show, NULL);

static ssize_t tsl2540_als_ch1_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	tsl2540_read_als(chip);

	return snprintf(buf, PAGE_SIZE, "%d\n", chip->als_inf.als_ch1);
}

static DEVICE_ATTR(als_ch1, 0444, tsl2540_als_ch1_show, NULL);

static ssize_t tsl2540_als_az_iterations_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	int iterations;

	iterations = chip->shadow[TSL2540_REG_AZ_CONFIG];
	return snprintf(buf, PAGE_SIZE, "%d\n", iterations);
}

static ssize_t tsl2540_als_az_iterations_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	long iterations;
	int rc;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);

	rc = kstrtoul(buf, 10, &iterations);
	if (rc)
		return -EINVAL;

	AMS_MUTEX_LOCK(&chip->lock);

	chip->shadow[TSL2540_REG_AZ_CONFIG] = (u8) iterations;
	chip->params.az_iterations = chip->shadow[TSL2540_REG_AZ_CONFIG];
	tsl2540_flush_als_regs(chip);

	AMS_MUTEX_UNLOCK(&chip->lock);

	return size;
}

static DEVICE_ATTR(als_az_iterations, 0644,
		tsl2540_als_az_iterations_show,
		tsl2540_als_az_iterations_store);

static ssize_t tsl2540_als_adc_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	int t;
	u8 tmp;
        t = chip->shadow[TSL2540_REG_ATIME];
	t++;
	t *= INTEGRATION_CYCLE;

        tmp = chip->als_inf.full_gain; //current gain, gain may be changed after calling get_lux.

	tsl2540_read_als(chip);
	tsl2540_get_lux(chip);

	if ( tmp == 0)
	   return snprintf(buf, PAGE_SIZE,
			"CH0: %d, CH1: %d, LUX: %d, VALID: %d, ASTA: 0x%02x, GAIN: 0.5, ATIME: %d\n", chip->als_inf.als_ch0,
			chip->als_inf.als_ch1, chip->als_inf.lux, chip->is_als_valid, chip->shadow[TSL2540_REG_STATUS],t);
        else
	   return snprintf(buf, PAGE_SIZE,
			"CH0: %d, CH1: %d, LUX: %d, VAILD: %d, ASTA: 0x%02x, GAIN: %d, ATIME: %d(us)\n", chip->als_inf.als_ch0,
			chip->als_inf.als_ch1, chip->als_inf.lux, chip->is_als_valid, chip->shadow[TSL2540_REG_STATUS], tmp, t);
}

static ssize_t tsl2540_als_adc_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	u32 ch0, ch1;

	if (sscanf(buf, "%10d,%10d", &ch0, &ch1) != 2)
		return -EINVAL;

	AMS_MUTEX_LOCK(&chip->lock);

	chip->als_inf.als_ch0 = ch0;
	chip->als_inf.als_ch1 = ch1;

	AMS_MUTEX_UNLOCK(&chip->lock);
	return size;
}

static DEVICE_ATTR(als_adc, 0644, tsl2540_als_adc_show,
			tsl2540_als_adc_store);

#ifdef CONFIG_AMZN_AMS_ALS
static ssize_t als_vis_400_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	struct tsl2540_i2c_platform_data *pdata = chip->pdata;
	int vis_400 = pdata->lux400_ch0;

	return snprintf(buf, PAGE_SIZE, "%d\n", vis_400);
}

static ssize_t als_vis_400_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t size)
{
	long vis_400;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	struct tsl2540_i2c_platform_data *pdata = chip->pdata;

	if (kstrtol(buf, 10, &vis_400))
		return -EINVAL;

	pdata->lux400_ch0 = vis_400;

	return size;
}

static DEVICE_ATTR(als_vis_400, 0644, als_vis_400_show,
		   als_vis_400_store);

static ssize_t als_ir_400_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	struct tsl2540_i2c_platform_data *pdata = chip->pdata;
	int ir_400 = pdata->lux400_ch1;

	return snprintf(buf, PAGE_SIZE, "%d\n", ir_400);
}

static ssize_t als_ir_400_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t size)
{
	long ir_400;
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	struct tsl2540_i2c_platform_data *pdata = chip->pdata;

	if (kstrtol(buf, 10, &ir_400))
		return -EINVAL;

	pdata->lux400_ch1 = ir_400;

	return size;
}

static DEVICE_ATTR(als_ir_400, 0644, als_ir_400_show,
		   als_ir_400_store);

struct tsl2540_chip *ams_chip = NULL; //to be set when driver opens
/****
 *
 * int als_get_lux_value(int sensor, int mode, int channel)
 * Paramters:
 *          sensor, should be set to 0. Only front sensor is supported now.
 *          mode,   1 for read raw channel data, or 0 to read calibrated lux value
 *          channel, if mode is 1 then this one can be 0 for VIS channel or 1 for IR channel
 *          returns VIR or IR channel counts (if mode = 0) or lux value
 *
 */

int als_get_lux_value(int sensor, int mode, int channel){
    int lux = 0;

    if (ams_chip == NULL) {
        pr_err("als driver is not initialized. Exit!\n");
        return -1;
    }
    if (sensor != 0){
	dev_err(&ams_chip->client->dev, "%s: only front sensor is supported.\n", __func__);
	return -1;
    }

    if (mode == 1) {
	AMS_MUTEX_LOCK(&ams_chip->lock);
	tsl2540_read_als(ams_chip);
	tsl2540_get_lux(ams_chip);
	AMS_MUTEX_UNLOCK(&ams_chip->lock);
	lux = ams_chip->als_inf.lux;
	return lux;
    }
    else {
       //raw data
       AMS_MUTEX_LOCK(&ams_chip->lock);
       tsl2540_read_als(ams_chip);
       AMS_MUTEX_UNLOCK(&ams_chip->lock);
       if ( channel == 0)
	    return ams_chip->als_inf.als_ch0;
       else
	    return ams_chip->als_inf.als_ch1;
    }
    dev_err(&ams_chip->client->dev, "%s: unsupported parameters.\n", __func__);
    return -1;
}

static ssize_t tsl2540_als_valid_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tsl2540_chip *chip = dev_get_drvdata(dev);
	int count;

	AMS_MUTEX_LOCK(&chip->lock);
	count =  snprintf(buf, PAGE_SIZE, "%d\n", chip->is_als_valid);
	AMS_MUTEX_UNLOCK(&chip->lock);
	return count;
}

static DEVICE_ATTR(als_valid, 0440, tsl2540_als_valid_show, NULL);

EXPORT_SYMBOL(ams_chip);
EXPORT_SYMBOL(als_get_lux_value);
#endif //CONFIG_AMZN_AMS_ALS

static struct attribute *tsl2540_als_attributes[] = {
	&dev_attr_als_itime.attr,
	&dev_attr_als_wtime.attr,
	&dev_attr_als_lux.attr,
	&dev_attr_als_gain.attr,
	&dev_attr_als_az_iterations.attr,
	&dev_attr_als_thresh_deltap.attr,
	&dev_attr_als_auto_gain.attr,
	&dev_attr_als_lux_table.attr,
	&dev_attr_als_power_state.attr,
	&dev_attr_als_persist.attr,
	&dev_attr_als_ch0.attr,
	&dev_attr_als_ch1.attr,
	&dev_attr_als_valid.attr,
#ifdef CONFIG_AMZN_AMS_ALS
	&dev_attr_als_vis_400.attr,
	&dev_attr_als_ir_400.attr,
#endif
	&dev_attr_als_adc.attr,
	NULL
};

const struct attribute_group tsl2540_als_attr_group = {
	.attrs = tsl2540_als_attributes,
};
