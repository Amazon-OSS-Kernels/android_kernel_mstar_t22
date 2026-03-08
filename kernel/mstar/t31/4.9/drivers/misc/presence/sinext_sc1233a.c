/*
 * Copyright (c) 2022 Amazon.com, Inc. or its affiliates.  All rights reserved.
 *
 */

#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/log2.h>
#include <linux/sched.h>

#include "mdrv_gpio.h"
#include "mdrv_pm.h"

#include "sinext_sc1233a.h"
#include "sinext_sc1233a_2d_normal_60db_1100us.h"
#include "sinext_sc1233a_motion_wide_40db_1100us.h"
#include "sinext_sc1233a_motion_wide_60db_1100us.h"
#include "sinext_sc1233a_presence_detection.h"


#define ktime_get_boottime_to_ms()	ktime_to_ms(ktime_get_boottime())

#define MAX_U32_CNT		U32_MAX
#define MAX_TIME_LOG_BUF	100
#define SC1233A_SOFT_RESET_RETRY_COUNT	1
#define SC1233A_ERR_MSG_MAX	20
#define SC1233A_LAST_RESULT_NG	0
#define SC1233A_LAST_RESULT_OK	1
#define SC1233A_LAST_I2C_SYSFS_MAX_READ_DATA	(32 * 40) /* 32bit * 40registers */

#define BITMASK_SET(x, mask)	((x) |= (mask))
#define BITMASK_CLEAR(x, mask)	((x) &= (~(mask)))
#define BITMASK_READ(x, mask)	((x) & (mask))

#define PRE_STATUS_MASK		0x0001
#define PRE_DET_CHANGE_MASK	0x0002
#define RATE_CHANGE_MASK	0x0004
#define GEN_ERROR_MASK		0x0010
#define I2C_ERROR_MASK		0x0020
#define OR_LOW_ERROR_MASK	0x0040
#define FIRST_FRAME_LOG_MASK	0x0100
#define PRINT_REQ_MASK		0x0200

#define MAX_FRAME_BUF_IDX	16 //only last 1 frame for phase 1

#define INC_FRAME_BUF_IDX(n)	{n = (++n >= MAX_FRAME_BUF_IDX) ? 0 : n;} \

#define NEXT_FRAME_BUF_IDX(n)	(n+1 >= MAX_FRAME_BUF_IDX) ? 0 : n+1 \


#define OR_WATCH_TIMER_PERIOD  10*INTERVAL_HIGH
#define OR_WATCH_INIT_DELAY    2*INTERVAL_HIGH     //Only used when OR_watch_timer is initialized in sc1233a_attempt_recovery

#define INC_U32_CNT(n)		{n = (++n >= MAX_U32_CNT) ? 1 : n;} \

#define NUMBER_OF_RETRIES	5

#define UDELAY_UNDER_10US(t)		udelay(t)
#define USLEEP_UNDER_20MS(min)		usleep_range(min,min+100)
#define MSLEEP_UNINT(t)			msleep(t)
#define MSLEEP_INTERRUPTIBLE(t)		msleep_interruptible(t)

#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))
#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

#define MAJOR_VERSION	2
#define MINOR_VERSION	0

enum sc1233a_state {
	NO_INIT = 0,
	SHUTDOWN,
	DEEP_SLEEP,
	LIGHT_SLEEP,
	SENSING_CONTINUOUS,
	SENSING_TIMER,
	STATE_MAX
};

static const char * const sc1233a_state_str[] = {
	[NO_INIT]		= "NO_INIT",
	[SHUTDOWN]		= "SHUTDOWN",
	[DEEP_SLEEP]		= "DEEP_SLEEP",
	[LIGHT_SLEEP]		= "LIGHT_SLEEP",
	[SENSING_CONTINUOUS]	= "SENSING_CONTINUOUS",
	[SENSING_TIMER]		= "SENSING_TIMER",
};

enum sc1233a_sensing_mode {
	SENSOR_INIT_MODE = 0,
	SENSOR_OFF_MODE,
	CONTINUOUS_MODE,
	TIMER_MODE,
	TIMER_DIS_LOW_MODE,
	TIMER_DIS_MID_MODE,
	TIMER_DIS_HIGH_MODE,
	OR_PIN_TEST_MODE,
	DETOUT_PIN_TEST_MODE,
	CLI_MODE,
	ALGO_TEST_MODE,
	DETOUT_MODE,
	SENSING_MODE_MAX
};

static const char * const sc1233a_sensing_mode_str[] = {
	[SENSOR_INIT_MODE]	= "SENSOR_INIT_MODE",
	[SENSOR_OFF_MODE]	= "SENSOR_OFF_MODE",
	[CONTINUOUS_MODE]	= "CONTINUOUS_MODE",
	[TIMER_MODE]		= "TIMER_MODE",
	[TIMER_DIS_LOW_MODE]	= "TIMER_DIS_LOW_MODE",
	[TIMER_DIS_MID_MODE]	= "TIMER_DIS_MID_MODE",
	[TIMER_DIS_HIGH_MODE]	= "TIMER_DIS_HIGH_MODE",
	[OR_PIN_TEST_MODE]	= "OR_PIN_TEST_MODE",
	[DETOUT_PIN_TEST_MODE]	= "DETOUT_PIN_TEST_MODE",
	[CLI_MODE]		= "CLI_MODE",
	[ALGO_TEST_MODE]	= "ALGO_TEST_MODE",
	[DETOUT_MODE]		= "DETOUT_MODE"
};

enum sc1233a_hal_distance_set_mode {
	HAL_SENSOR_OFF_MODE = 0,
	HAL_TIMER_DIS_LOW_MODE,
	HAL_TIMER_DIS_MID_MODE,
	HAL_TIMER_DIS_HIGH_MODE
};

enum sc1233a_cli_set_config_mode {
	CLI_DETOUT_MODE			= 1,
	CLI_PRESENCE_TRACKING_MODE	= 2,
	CLI_FFT_MODE			= 3,
	CLI_ARF_BRF_TEST_MODE		= 4,
	CLI_COMPLIANCE_TEST_MODE	= 5,
	CLI_TIMER_MODE_TEST		= 11,
	CLI_CONTINUOUS_MODE_TEST	= 12,
	CLI_CHIP_BOOT_MODE		= 13,
	CLI_ALGO_MODE_TEST		= 14,
	CLI_SET_CONFIG_MODE_MAX
};

//User Settings
#define SC1233A_RADAR_INTER_VAL 	(SC1233A_RADAR_INTER_1FPS)
#define SC1233A_RADAR_INTER_BETA_VAL	(SC1233A_RADAR_INTER_BETA_1FPS)

//Radar config pre-processor directives - keep defined or comment
#define SC1233A_ENABLE_ALL_5_DIST_RD //defined - reads 5 distance registers, commented - reads first 2 values
#define SC1233A_ENABLE_ALL_5_PEAKLVL_RD //defined - reads 5 Peak level registers, commented - reads first value

enum sc1233a_cmd {
	MS_CMD_HARD_RESET		= 0xC7,
	MS_CMD_SOFT_RESET		= 0xAB,
	MS_CMD_MODE_CTRL		= 0x01,
	MS_CMD_DEEP_SLEEP		= 0xB9,
	MS_CMD_WRITE_STATUS_REG		= 0x01,
	MS_CMD_READ_STATUS_REG		= 0x05,
	MS_CMD_WRITE_DATA		= 0x02,
	MS_CMD_READ_DATA		= 0x0B,
	MS_CMD_READ_STATUS_REG2		= 0x0F,
	MS_CMD_EN_TIMER			= 0x11,
	MS_CMD_DIS_TIMER		= 0x10,
	MS_CMD_START_SENSING_BY_TIMER	= 0x13,
	MS_CMD_STOP_SENSING_BY_TIMER	= 0x12,
	MS_CMD_HOLD_DT			= 0x15,
	MS_CMD_HOLD_DT_STOP		= 0xAB,
	MS_CMD_UPDATE_DT_1		= 0xB9,
	MS_CMD_UPDATE_DT_2		= 0x14,
};

enum sc1233a_data_reg_addr {
	MS_REG_ADDR_PEAKLVL_RD_RX1_1 	= 0x00019B,
	MS_REG_ADDR_PEAKLVL_RD_RX1_2 	= 0x00019C,
	MS_REG_ADDR_PEAKLVL_RD_RX1_3 	= 0x00019D,
	MS_REG_ADDR_PEAKLVL_RD_RX1_4 	= 0x00019E,
	MS_REG_ADDR_PEAKLVL_RD_RX1_5 	= 0x00019F,
	MS_REG_ADDR_PEAKLVL_RD_RX2_1 	= 0x0001AD,
	MS_REG_ADDR_PEAKLVL_RD_RX2_2 	= 0x0001AE,
	MS_REG_ADDR_PEAKLVL_RD_RX2_3 	= 0x0001AF,
	MS_REG_ADDR_PEAKLVL_RD_RX2_4 	= 0x0001B0,
	MS_REG_ADDR_PEAKLVL_RD_RX2_5 	= 0x0001B1,
	MS_REG_ADDR_DIST_RD_RX1_12 	= 0x0001BC,
	MS_REG_ADDR_DIST_RD_RX1_34 	= 0x0001BD,
	MS_REG_ADDR_DIST_RD_RX1_5 	= 0x0001BE,
	MS_REG_ADDR_DIST_RD_RX2_12 	= 0x0001BF,
	MS_REG_ADDR_DIST_RD_RX2_34 	= 0x0001C0,
	MS_REG_ADDR_DIST_RD_RX2_5 	= 0x0001C1,
	MS_REG_ADDR_FIFO_RD_MEM		= 0x010000,
};

enum sc1233a_status_reg_bit {
	SC1233A_ST_SEQ_BUSY	= 0,
	SC1233A_ST_SEQ_ERR	= 1,
	SC1233A_ST_CONT_SING	= 2,
	SC1233A_ST_START_STOP	= 3,
	SC1233A_ST_UNDEF_COM	= 4,
	SC1233A_ST_FIFO_OVF	= 5,
	SC1233A_ST_FIFO_UDF	= 6,
	SC1233A_ST_FIFO_OR	= 7,
	SC1233A_ST_MAX		= 8,
	SC1233A_ST2_DETOUT	= 0,
};

enum sc1233a_status_reg_mask {
	SC1233A_ST_SEQ_BUSY_MASK	= (1 << SC1233A_ST_SEQ_BUSY),
	SC1233A_ST_SEQ_ERR_MASK		= (1 << SC1233A_ST_SEQ_ERR),
	SC1233A_ST_CONT_SING_MASK	= (1 << SC1233A_ST_CONT_SING),
	SC1233A_ST_START_STOP_MASK	= (1 << SC1233A_ST_START_STOP),
	SC1233A_ST_UNDEF_COM_MASK	= (1 << SC1233A_ST_UNDEF_COM),
	SC1233A_ST_FIFO_OVF_MASK	= (1 << SC1233A_ST_FIFO_OVF),
	SC1233A_ST_FIFO_UDF_MASK	= (1 << SC1233A_ST_FIFO_UDF),
	SC1233A_ST_FIFO_OR_MASK		= (1 << SC1233A_ST_FIFO_OR),
	SC1233A_ST2_DETOUT_MASK		= (1 << SC1233A_ST2_DETOUT),
};

static const char * const sc1233a_status_reg_str[] = {
	[SC1233A_ST_SEQ_BUSY]	= "SEQ_BUSY",
	[SC1233A_ST_SEQ_ERR]	= "SEQ_ERR",
	[SC1233A_ST_CONT_SING]	= "CONTINUOUS",	/* 1:Continuous sensing mode, 0:Single sensing mode */
	[SC1233A_ST_START_STOP]	= "START",	/* 1:START, 0:STOP */
	[SC1233A_ST_UNDEF_COM]	= "UNDEF_COM",
	[SC1233A_ST_FIFO_OVF]	= "FIFO_OVF",
	[SC1233A_ST_FIFO_UDF]	= "FIFO_UDF",
	[SC1233A_ST_FIFO_OR]	= "FIFO_OR",
	[SC1233A_ST_MAX + SC1233A_ST2_DETOUT]	= "DETOUT"
};

struct sc1233a_last_i2c_sysfs {
	u32 addr;
	u32 data[4];
	char mode_2nd;
	u32 write_bytes;
	u32 block_size;
	u32 total_bytes;
	int ret;
	u32 *p_u32_data;
	u8 *p_u8_data;
};

struct sc1233a_ro_int_time {
	s64 irq_occur;
	s64 wq_start_ms;
	s64 wq_elapse_ms;
	char is_detecded;
};

struct sc1233a_event_cnt {
	u32 ro_int_cnt;
	u32 detout_int_cnt;
	u32 presence_cnt;
	u32 i2c_err_cnt;
	u32 err_cnt;
	s64 max_wq_elapse_ms;
	u32 frame_interval_cnt;
	/* Work queue elapse time histogram in 25ms bins */
	u32 wq_elapse_ms_histogram[7];
	u32 wake_up_cnt;
	u32 recovery_try_cnt;
	u32 recovery_success_cnt;
	int int_time_buf_idx;
	struct sc1233a_ro_int_time ro_time_buf[MAX_TIME_LOG_BUF];
};

struct sc1233a_frame_hist {
	s64 frame_time;
	u16 status;
	u32 u32_dist_rx[3];
	u32 rx1_peak[5];
	u16 interval;
};

/* Must match definition in ../arch file */
struct sc1233a_data {
	struct mutex wq_lock;
	struct mutex sysfs_lock;
	struct i2c_client *client;
	int ce_gpio;
	int nrst_gpio;
	int or_gpio;
	int detout_gpio;
	enum sc1233a_state state;
	enum sc1233a_sensing_mode mode;
	struct work_struct algo_update;
	unsigned int host_timer_interval;
	unsigned int watch_timer_interval;
	bool ro_irq_enabled;
	int frame_hist_idx;
	struct sc1233a_frame_hist frame_hist_buf[MAX_FRAME_BUF_IDX];
#ifdef CONFIG_OF
	struct device_node *of_node;
#endif
};

static struct sc1233a_event_cnt sc1233a_cnt;
static struct sc1233a_data *temp_pdata;
static struct workqueue_struct *sc1233a_wq;

static struct timer_list or_host_timer;
static struct timer_list or_watch_timer;

/*Algorithm specific data structures*/
static struct presence_handle handle_app;
static struct parameter_app app_para;
static struct parameter_radar_dynamic radar_para;

int previous_ro_int_cnt = -1;

static u8 fan_bins[DIS_DIM] = {DIS_DIM};
u8 number_of_fan_bins = 0;
u8 empty_room_detected_with_low_power = 0;

#define SC1233A_ERR(x, args...) if (sc1233a_log_level >= SC1233A_LOG_ERR) \
		{pr_err("%s: "x, __func__, ##args);} \

#define SC1233A_INFO(x, args...) if (sc1233a_log_level >= SC1233A_LOG_INFO) \
		{pr_err("%s: "x, __func__, ##args);} \

#define SC1233A_DEBUG(x, args...) if (sc1233a_log_level >= SC1233A_LOG_DEBUG) \
		{pr_err("%s: "x, __func__, ##args);} \

/* local prototype */
static irqreturn_t sc1233a_or_irq(int irq, void *handle);
static void sc1233a_or_irq_handler(struct sc1233a_data *pdata);
static void sc1233a_enable_irq(struct sc1233a_data *pdata, int gpio, bool enable);
static void sc1233a_or_timer_func(unsigned long data);
static void sc1233a_or_timer_init(struct sc1233a_data *pdata, unsigned int interval);
static int sc1233a_attempt_recovery(struct sc1233a_data *pdata);

static void sc1233a_or_check_func(unsigned long data);
static void sc1233a_or_watch_init(struct sc1233a_data *pdata, unsigned int interval);

static int sc1233a_i2c_write_1byte_only(struct i2c_client *client, u8 u8_data)
{
	struct i2c_msg msg[1];
	int ret;

	memset(msg, 0, sizeof(msg));
	msg[0].addr = client->addr;
	msg[0].len = 1;
	msg[0].buf = &u8_data;

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0) {
		INC_U32_CNT(sc1233a_cnt.i2c_err_cnt);
		dev_err(&client->dev, "%s: failed ret=%d,d=0x%X\n", __func__,
			ret, u8_data);
		return ret;
	}

	return ret;
}

static int sc1233a_i2c_write_byte(struct i2c_client *client, u8 u8_reg, u8 u8_data)
{
	struct i2c_msg msg[1];
	int ret;
	u8 u8_buf[2];

	u8_buf[0] = u8_reg;
	u8_buf[1] = u8_data;

	memset(msg, 0, sizeof(msg));
	msg[0].addr = client->addr;
	msg[0].len = 2;
	msg[0].buf = u8_buf;

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0) {
		INC_U32_CNT(sc1233a_cnt.i2c_err_cnt);
		dev_err(&client->dev, "%s: failed ret=%d,a=0x%X,d=0x%X\n", __func__,
			ret, u8_reg, u8_data);
		return ret;
	}

	return ret;
}

static int sc1233a_i2c_read_byte(struct i2c_client *client, u8 u8_reg, u8 *u8_data)
{
	struct i2c_msg msg[2];
	int ret;

	memset(msg, 0, sizeof(msg));
	msg[0].addr = client->addr;
	msg[0].len = 1;
	msg[0].buf = &u8_reg;
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = u8_data;

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret < 0) {
		INC_U32_CNT(sc1233a_cnt.i2c_err_cnt);
		dev_err(&client->dev, "%s: failed ret=%d,a=0x%X\n", __func__,
			ret, u8_reg);
		return ret;
	}

	return ret;
}

static int sc1233a_i2c_write_u8_bulk(struct i2c_client *client, u32 u32_reg, u8 u8_data[], int wbytes)
{
	u8 u8_reg[20];
	struct i2c_msg msg[1];
	int ret;
	int i;

	i = 0;
	u8_reg[i++] = MS_CMD_WRITE_DATA;
	u8_reg[i++] = (u32_reg >> 16) & 0xFF;
	u8_reg[i++] = (u32_reg >>  8) & 0xFF;
	u8_reg[i++] = (u32_reg >>  0) & 0xFF;

	/* TODO: */
	if (wbytes == 2) {
		u8_reg[i++] = u8_data[0];
		u8_reg[i++] = u8_data[1];
	} else if (wbytes == 16) {
		u8_reg[i++] = u8_data[0];
		u8_reg[i++] = u8_data[1];
		u8_reg[i++] = u8_data[2];
		u8_reg[i++] = u8_data[3];
		u8_reg[i++] = u8_data[4];
		u8_reg[i++] = u8_data[5];
		u8_reg[i++] = u8_data[6];
		u8_reg[i++] = u8_data[7];
		u8_reg[i++] = u8_data[8];
		u8_reg[i++] = u8_data[9];
		u8_reg[i++] = u8_data[10];
		u8_reg[i++] = u8_data[11];
		u8_reg[i++] = u8_data[12];
		u8_reg[i++] = u8_data[13];
		u8_reg[i++] = u8_data[14];
		u8_reg[i++] = u8_data[15];
	} else {
		dev_err(&client->dev, "%s: not support %dB\n", __func__, wbytes);
		return -EINVAL;
	}

	memset(msg, 0, sizeof(msg));
	msg[0].addr = client->addr;
	msg[0].len = 4/*addr 4B*/ + wbytes;;
	msg[0].buf = u8_reg;

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0) {
		INC_U32_CNT(sc1233a_cnt.i2c_err_cnt);
		dev_err(&client->dev, "%s: failed ret=%d,a=0x%X,%dbytes\n", __func__,
			ret, u32_reg, wbytes);
		return ret;
	}

	return ret;
}

static int sc1233a_i2c_write_u32_bulk(struct i2c_client *client, u32 u32_reg, u32 u32_data[], int wbytes)
{
	u8 u8_reg[20];
	struct i2c_msg msg[1];
	int ret;
	int i;

	i = 0;
	u8_reg[i++] = MS_CMD_WRITE_DATA;
	u8_reg[i++] = (u32_reg >> 16) & 0xFF;
	u8_reg[i++] = (u32_reg >>  8) & 0xFF;
	u8_reg[i++] = (u32_reg >>  0) & 0xFF;
	u8_reg[i++] = (u32_data[0] >> 24) & 0xFF;
	u8_reg[i++] = (u32_data[0] >> 16) & 0xFF;
	u8_reg[i++] = (u32_data[0] >>  8) & 0xFF;
	u8_reg[i++] = (u32_data[0] >>  0) & 0xFF;

	memset(msg, 0, sizeof(msg));
	msg[0].addr = client->addr;
	//msg[0].len = 4/*addr 4B*/ + wbytes;
	msg[0].len = 4/*addr 4B*/ + 4; //TODO: 4 bytes for now
	msg[0].buf = u8_reg;

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0) {
		INC_U32_CNT(sc1233a_cnt.i2c_err_cnt);
		dev_err(&client->dev, "%s: failed ret=%d,a=0x%X,d=0x%X,%dbytes\n", __func__,
			ret, u32_reg, u32_data[0], wbytes);
		return ret;
	}

	return ret;
}

static int sc1233a_i2c_read_u32_bulk(struct i2c_client *client, u32 u32_reg, u32 *u32_data, int rbytes)
{
	u8 u8_reg[4], *u8_data = (u8 *)u32_data;
	struct i2c_msg msg[2];
	int ret;
	int i;

	/* only read 4 bytes register */
	if (rbytes % 4 != 0) {
		dev_err(&client->dev, "%s: invalid %dbytes for u32\n", __func__, rbytes);
		return -1;
	}

	u8_reg[0] = MS_CMD_READ_DATA;
	u8_reg[1] = (u32_reg >> 16) & 0xFF;
	u8_reg[2] = (u32_reg >>  8) & 0xFF;
	u8_reg[3] = (u32_reg >>  0) & 0xFF;

	memset(msg, 0, sizeof(msg));
	msg[0].addr = client->addr;
	msg[0].len = 4/*addr 4B*/;
	msg[0].buf = u8_reg;
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = rbytes;
	msg[1].buf = u8_data;

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret < 0) {
		INC_U32_CNT(sc1233a_cnt.i2c_err_cnt);
		dev_err(&client->dev, "%s: failed ret=%d,a=0x%X,%dbytes\n", __func__,
			ret, u32_reg, rbytes);
		return ret;
	}

	for (i = 0; i < rbytes/4; i++) {
		u32_data[i] = SWAP_UINT32(u32_data[i]);
	}

	return ret;
}

static int sc1233a_i2c_read_u8_bulk(struct i2c_client *client, u32 u32_reg, u8 u8_data[], int rbytes)
{
	u8 u8_reg[4];
	struct i2c_msg msg[2];
	int ret;

	//rbytes = 4; //TODO: 4 bytes for now

	u8_reg[0] = MS_CMD_READ_DATA;
	u8_reg[1] = (u32_reg >> 16) & 0xFF;
	u8_reg[2] = (u32_reg >>  8) & 0xFF;
	u8_reg[3] = (u32_reg >>  0) & 0xFF;

	memset(msg, 0, sizeof(msg));
	msg[0].addr = client->addr;
	msg[0].len = 4/*addr 4B*/;
	msg[0].buf = u8_reg;
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = rbytes;
	msg[1].buf = u8_data;

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret < 0) {
		INC_U32_CNT(sc1233a_cnt.i2c_err_cnt);
		dev_err(&client->dev, "%s: failed ret=%d,a=0x%X,%dbytes\n", __func__,
			ret, u32_reg, rbytes);
		return ret;
	}

	return ret;
}

#define sc1233a_i2c_write_soft_reset(c)			sc1233a_i2c_write_1byte_only(c, MS_CMD_SOFT_RESET)
#define sc1233a_i2c_write_mode_ctrl(c)			sc1233a_i2c_write_1byte_only(c, MS_CMD_MODE_CTRL)

#define sc1233a_i2c_write_fft_2b(c, addr, data)		sc1233a_i2c_write_u8_bulk(c, addr, data, 2)
#define sc1233a_i2c_write_seq_16b(c, addr, data)	sc1233a_i2c_write_u8_bulk(c, addr, data, 16)
#define sc1233a_i2c_write_u32_reg(c, addr, data)	sc1233a_i2c_write_u32_bulk(c, addr, data, 4)

#define sc1233a_i2c_read_u32_reg(c, addr, data)		sc1233a_i2c_read_u32_bulk(c, addr, data, 4)
#define sc1233a_i2c_read_u32_fifo(c, addr, data)	sc1233a_i2c_read_u32_bulk(c, addr, data, 4)

#define sc1233a_i2c_write_status_reg(c, data)		sc1233a_i2c_write_byte(c, MS_CMD_WRITE_STATUS_REG, data)
#define sc1233a_i2c_read_status_reg(c, data)		sc1233a_i2c_read_byte(c, MS_CMD_READ_STATUS_REG, data)
#define sc1233a_i2c_read_status_reg2(c, data)		sc1233a_i2c_read_byte(c, MS_CMD_READ_STATUS_REG2, data)


static int sc1233a_i2c_modify_u32_reg(struct i2c_client *client, u32 addr, u32 *rdata, u32 wdata, u32 mask)
{
	int ret;

	ret = sc1233a_i2c_read_u32_reg(client, addr, rdata);
	if (ret < 0) {
		SC1233A_ERR("read failed ret=%d,addr=0x%X\n", ret, addr);
		return ret;
	}

	*rdata = (*rdata & (~mask)) | (wdata & mask);
	ret = sc1233a_i2c_write_u32_reg(client, addr, rdata);
	if (ret < 0) {
		SC1233A_ERR("write failed ret=%d,addr=0x%X,data=0x%X\n", ret, addr, *rdata);
		return ret;
	}

	return ret;
}

static int sc1233a_hard_reset(struct i2c_client *client, struct sc1233a_data *pdata)
{
	int ret;

	/* disable or interrupt source */
	sc1233a_enable_irq(pdata, pdata->or_gpio, false);
	del_timer_sync(&or_host_timer);

	//Negate CE Pin and NRST Pin
	MDrv_GPIO_Set_Low(pdata->nrst_gpio);
	UDELAY_UNDER_10US(10); /* Min. 5us */
	MDrv_GPIO_Set_Low(pdata->ce_gpio);
	USLEEP_UNDER_20MS(5000); /* no specific delay in data sheet */
	SC1233A_DEBUG("reset low, ce_out=%d, nrst_out=%d\n",
		MDrv_GPIO_Pad_Read(pdata->ce_gpio), MDrv_GPIO_Pad_Read(pdata->nrst_gpio));

	//Assert CE Pin and NRST Pin
	MDrv_GPIO_Set_High(pdata->ce_gpio);
	USLEEP_UNDER_20MS(1000); /* Min. 100us */
	MDrv_GPIO_Set_High(pdata->nrst_gpio);
	SC1233A_DEBUG("reset high, ce_out=%d, nrst_out=%d\n",
		MDrv_GPIO_Pad_Read(pdata->ce_gpio), MDrv_GPIO_Pad_Read(pdata->nrst_gpio));
	USLEEP_UNDER_20MS(1000); /* Min. 0us */

	// Issue Hard Reset Comment HRST = 0xC7
	ret = sc1233a_i2c_write_1byte_only(client, MS_CMD_HARD_RESET);
	if (ret < 0) {
		goto error;
	}
	USLEEP_UNDER_20MS(10000); /* soft reset Min. 5ms, hard reset may required more */

	pdata->state = DEEP_SLEEP;

	return 0;

error:
	SC1233A_ERR("HRST I2C cmd fail, ret=%d\n", ret);
	return ret;
}

static int sc1233a_soft_reset(struct i2c_client *client, struct sc1233a_data *pdata)
{
	int ret;
	int retry;

	ret = -EIO;
	retry = 0;
	while (retry <= SC1233A_SOFT_RESET_RETRY_COUNT) {
		if ((sc1233a_i2c_write_soft_reset(client) < 0) || (sc1233a_i2c_write_mode_ctrl(client) < 0)) {
			retry++;
			USLEEP_UNDER_20MS(10000);
			continue;
		}
		pdata->state = LIGHT_SLEEP;
		USLEEP_UNDER_20MS(6000); /* Min. 5ms */
		ret = 0;
		break;
	}

	if (ret < 0) {
		SC1233A_ERR("%d retry failed\n", (retry-1));
	}

	return ret;
}

static int sc1233a_start_sensing(struct i2c_client *client, struct sc1233a_data *pdata,
	enum sc1233a_sensing_mode mode, int resume, u16 interval, u8 beta)
{
	u32 addr;
	u32 data, data_read;
	u8  u8_data;
	int ret;
	enum sc1233a_cmd cmd_val;
	sc1233a_cnt.frame_interval_cnt = interval;

	SC1233A_DEBUG("enter, %s\n", sc1233a_sensing_mode_str[mode]);

	if (mode == CONTINUOUS_MODE){
		/* set corresponding interrupt source  */
		sc1233a_enable_irq(pdata, pdata->or_gpio, false);
		sc1233a_or_timer_init(pdata, 1000u);

		//Force initialize data in FIFO to 0
		addr = 0xAA;
		data = 0x00;
		SC1233A_DEBUG("[FIFO set 0]: register write %06x %08x\n", addr, data);
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(1000);

		//Release FIFO
		addr = 0xAA;
		data = 0x01100000;
		SC1233A_DEBUG("[FIFO release]: register write %06x %08x\n", addr, data);
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(1000);

		//Start continuous sensing mode

		u8_data = 0x0C;
		SC1233A_DEBUG("[Start cont Sense]: register write %02x\n", u8_data);
		ret = sc1233a_i2c_write_status_reg(client, u8_data);
		if (ret < 0)
			goto error;

		/* switch to SENSING_CONTINUOUS state */
		pdata->state = SENSING_CONTINUOUS;
	} else if ((mode == TIMER_MODE) || (mode == DETOUT_PIN_TEST_MODE)) {
		/* set corresponding interrupt source  */
		del_timer_sync(&or_host_timer);
		sc1233a_enable_irq(pdata, pdata->or_gpio, true);

		//Force initialize data in FIFO to 0
/*
		addr = 0xAA;
		data = 0x00;
		SC1233A_DEBUG("[FIFO set 0]: register write %06x %08x\n", addr, data);
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(1000);

		//Release FIFO
		addr = 0xAA;
		data = 0x01100000;
		SC1233A_DEBUG("[FIFO release]: register write %06x %08x\n", addr, data);
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(1000);
*/

		/* skip for detout pin test mode otherwise pin always high */
		if (mode != DETOUT_PIN_TEST_MODE) {
			addr = 0xB0;
			if (resume == 0)
				data = 0x9F0;
			else
				data = 0x100;
			ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
			SC1233A_DEBUG("Start sensing 0xB0 values is %d \n", data);
			if (ret < 0)
				goto error;

			data = 0x0;
			ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
			SC1233A_DEBUG("Start sensing 0xB0 values is %d \n", data);
			if (ret < 0)
				goto error;
		}

		//Following steps are for Timer mode operation minus Sensing FIFO reset
		//Write bit[28] of the 0x00009B to 1 to activate internal timer clock
		addr = 0x9B;
		data = (0x001FFC77 | 0x10000000);
		SC1233A_DEBUG("[Act intn Timer]: register write %06x %08x\n", addr, data);
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(5000);

		//Write the sensing interval value
		addr = 0xA1;
		//data = ((SC1233A_RADAR_INTER_VAL*40*64)/40); //0x33443301;
		data = ((interval*40*64)/40);
		SC1233A_DEBUG("[Write Sensing Intv]: register write %06x %08x\n", addr, data);
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(1000);

		//Write the Beta value
		addr = 0x44;
		//Read from the register first to Preserve other fields
		ret = sc1233a_i2c_read_u32_reg(client, addr, &data_read);
		if (ret < 0)
			goto error;
		//data = (data_read & 0xFFFF00FF) | ((SC1233A_RADAR_INTER_BETA_VAL << 8) & (0x0000FF00));
		data = (data_read & 0xFFFF00FF) | ((beta << 8) & (0x0000FF00));

		//Write the new Beta value, other fileds unchanged
		SC1233A_DEBUG("[Beta value]: register write %06x %08x\n", addr, data);
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		if (ret < 0)
			goto error;

		//Issue deep sleep command to reset internal circuit
		cmd_val = MS_CMD_DEEP_SLEEP;
		//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(5000);

		//Issue ENATM command
		cmd_val = MS_CMD_EN_TIMER;
		//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(1000);

		//Issue RUNTM command
		cmd_val = MS_CMD_START_SENSING_BY_TIMER;
		//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(1000);

		/* Might not require this soft reset but just have it here since there in reference code. */
/*
		ret = sc1233a_soft_reset(client, pdata);
		if (ret < 0)
			goto error;

		addr = 0xB0;
		if (resume == 0)
			data = 0x9F0;
		else
			data = 0x100;
		data = 0x100;
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		pr_err("%s: Start sensing 0xB0 values is %d \n", __func__, data);
		if (ret < 0)
			goto error;

		ret = sc1233a_i2c_read_status_reg(pdata->client, &data_byte);
		if (ret < 0)
			goto error;
		pr_err("%s: Status register variable value is %d\n", __func__, data_byte);

		addr = 0x9B;
		data = 0x1FFC77 | 0x1000000;
		ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
		if (ret < 0)
			goto error;

		cmd_val = MS_CMD_DEEP_SLEEP;
		pr_err("%s: [Deep sleep cmd]: register write %02x \n", __func__, cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		cmd_val = MS_CMD_EN_TIMER;
		pr_err("%s: [Deep sleep cmd]: register write %02x \n", __func__, cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		cmd_val = MS_CMD_START_SENSING_BY_TIMER;
		pr_err("%s: [Deep sleep cmd]: register write %02x \n", __func__, cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;
*/

		/* switch to SENSING_TIMER state */
		pdata->state = SENSING_TIMER;
	}

	/* Following steps are for Timer mode operation minus Sensing FIFO reset
	//Write bit[28] of the 0x00009B to 1 to activate internal timer clock
	addr = 0x9B;
	data = (0x001FFC77 | 0x10000000);
	pr_err("%s: [Act intn Timer]: register write %06x %08x\n", __func__, addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Write the sensing interval value
	addr = 0xA1;
	data = (1000/(40*40*64));
	pr_err("%s: [Write Sensing Intv]: register write %06x %08x\n", __func__, addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue deep sleep command to reset internal circuit
	cmd_val = MS_CMD_DEEP_SLEEP;
	pr_err("%s: [Deep sleep cmd]: register write %02x \n", __func__, cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Issue ENATM command
	cmd_val = MS_CMD_EN_TIMER;
	pr_err("%s: [Deep sleep cmd]: register write %02x \n", __func__, cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue RUNTM command
	cmd_val = MS_CMD_START_SENSING_BY_TIMER;
	pr_err("%s: [Deep sleep cmd]: register write %02x \n", __func__, cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);
	//end of Continuous mode */

	MSLEEP_INTERRUPTIBLE(100); //Giving enough delay here so that OR pin can be detected

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	SC1233A_ERR("failed, ret=%d\n", ret);
	return ret;
}

static int sc1233a_start_sensing_update_distance(struct i2c_client *client, struct sc1233a_data *pdata,
	enum sc1233a_sensing_mode mode)
{
	u32 addr;
	u32 data;
	int ret;
	enum sc1233a_cmd cmd_val;
	int i;

	SC1233A_ERR("enter, %s\n", sc1233a_sensing_mode_str[mode]);

	/* set corresponding interrupt source  */
	del_timer_sync(&or_host_timer);
	sc1233a_enable_irq(pdata, pdata->or_gpio, true);

	for (i=0; i<MAX_FRAME_BUF_IDX; i++) {
		pdata->frame_hist_buf[i].status = 0x0;
	}

	addr = 0xB0;
	data = 0x100;
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	SC1233A_DEBUG("Start sensing 0xB0 values is %d \n", data);
	if (ret < 0)
		goto error;

	data = 0x0;
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	SC1233A_DEBUG("Start sensing 0xB0 values is %d \n", data);
	if (ret < 0)
		goto error;

	//Following steps are for Timer mode operation minus Sensing FIFO reset
	//Write bit[28] of the 0x00009B to 1 to activate internal timer clock
	addr = 0x9B;
	data = (0x001FFC77 | 0x10000000);
	SC1233A_DEBUG("[Act intn Timer]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Update the distance value.
	addr = 0x5F;
	switch (mode)
	{
	case TIMER_DIS_LOW_MODE:
		data = 0xfffc0000;
		break;
	case TIMER_DIS_MID_MODE:
		data = 0xfffff000;
		break;
	case TIMER_DIS_HIGH_MODE:
		data = 0xffffffe0;
		break;
	default:
		ret = -1;
		goto error;
	}

	SC1233A_DEBUG("[Write Distance value]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue deep sleep command to reset internal circuit
	cmd_val = MS_CMD_DEEP_SLEEP;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Issue ENATM command
	cmd_val = MS_CMD_EN_TIMER;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue RUNTM command
	cmd_val = MS_CMD_START_SENSING_BY_TIMER;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	/* switch to SENSING_TIMER state */
	pdata->state = SENSING_TIMER;

	MSLEEP_INTERRUPTIBLE(100); //Giving enough delay here so that OR pin can be detected

	SC1233A_ERR("ok\n");
	return 0;

error:
	SC1233A_ERR("failed, ret=%d\n", ret);
	return ret;
}

static int sc1233a_start_sensing_with_distance_DETOUT(struct i2c_client *client, struct sc1233a_data *pdata,
	enum sc1233a_sensing_mode mode)
{
	u32 addr;
	u32 data, data_read;
	u8  u8_data;
	int ret;
	enum sc1233a_cmd cmd_val;

	SC1233A_DEBUG("enter, mode=%d\n", mode);


	del_timer_sync(&or_host_timer);
	//sc1233a_enable_irq(pdata, pdata->or_gpio, true);

	//Following steps are for Timer mode operation minus Sensing FIFO reset
	//Write bit[28] of the 0x00009B to 1 to activate internal timer clock
	addr = 0x9B;
	data = (0x001FFC77 | 0x10000000);
	SC1233A_DEBUG("[Act intn Timer]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	addr = 0x4F;
	switch (mode)
	{
	case TIMER_DIS_LOW_MODE:
		data = 0xfffc0000;
		break;
	case TIMER_DIS_MID_MODE:
		data = 0xfffff000;
		break;
	case TIMER_DIS_HIGH_MODE:
		data = 0xffffffe0;
		break;
	default:
		ret = -1;
		goto error;
	}

	SC1233A_DEBUG("[Write Distance value]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue deep sleep command to reset internal circuit
	cmd_val = MS_CMD_DEEP_SLEEP;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Issue ENATM command
	cmd_val = MS_CMD_EN_TIMER;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue RUNTM command
	cmd_val = MS_CMD_START_SENSING_BY_TIMER;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	/* switch to SENSING_TIMER state */
	pdata->state = SENSING_TIMER;

	MSLEEP_INTERRUPTIBLE(100); //Giving enough delay here so that OR pin can be detected

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	SC1233A_ERR("failed, ret=%d\n", ret);
	return ret;
}

static int sc1233a_start_sensing_update_distance_DETOUT(struct i2c_client *client, struct sc1233a_data *pdata,
	enum sc1233a_sensing_mode mode)
{
	u32 addr;
	u32 data;
	int ret;
	enum sc1233a_cmd cmd_val;

	/* set corresponding interrupt source  */
	del_timer_sync(&or_host_timer);
	//sc1233a_enable_irq(pdata, pdata->or_gpio, true);

	addr = 0xB0;
	data = 0x100;
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	SC1233A_DEBUG("Start sensing 0xB0 values is %d \n", data);
	if (ret < 0)
		goto error;

	data = 0x0;
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	SC1233A_DEBUG("Start sensing 0xB0 values is %d \n", data);
	if (ret < 0)
		goto error;

	//Following steps are for Timer mode operation minus Sensing FIFO reset
	//Write bit[28] of the 0x00009B to 1 to activate internal timer clock
	addr = 0x9B;
	data = (0x001FFC77 | 0x10000000);
	SC1233A_DEBUG("[Act intn Timer]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Update the distance value.
	addr = 0x4F;
	switch (mode)
	{
	case TIMER_DIS_LOW_MODE:
		data = 0xfffc0000;
		break;
	case TIMER_DIS_MID_MODE:
		data = 0xfffff000;
		break;
	case TIMER_DIS_HIGH_MODE:
		data = 0xffffffe0;
		break;
	default:
		ret = -1;
		goto error;
	}

	SC1233A_DEBUG("[Write Distance value]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue deep sleep command to reset internal circuit
	cmd_val = MS_CMD_DEEP_SLEEP;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Issue ENATM command
	cmd_val = MS_CMD_EN_TIMER;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue RUNTM command
	cmd_val = MS_CMD_START_SENSING_BY_TIMER;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	/* switch to SENSING_TIMER state */
	pdata->state = SENSING_TIMER;

	MSLEEP_INTERRUPTIBLE(100); //Giving enough delay here so that OR pin can be detected

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	SC1233A_ERR("failed, ret=%d\n", ret);
	return ret;
}

static int sc1233a_start_sensing_DETOUT(struct i2c_client *client, struct sc1233a_data *pdata,
	enum sc1233a_sensing_mode mode, int resume, u16 interval, u8 alpha)
{
	u32 addr;
	u32 data, data_read;
	u8  u8_data;
	int ret;
	enum sc1233a_cmd cmd_val;

	SC1233A_DEBUG("enter, mode=%d\n", mode);


	del_timer_sync(&or_host_timer);
	//sc1233a_enable_irq(pdata, pdata->or_gpio, true);

	//Following steps are for Timer mode operation minus Sensing FIFO reset
	//Write bit[28] of the 0x00009B to 1 to activate internal timer clock
	addr = 0x9B;
	data = (0x001FFC77 | 0x10000000);
	SC1233A_DEBUG("[Act intn Timer]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Write the sensing interval value
	addr = 0xA1;
	//data = ((SC1233A_RADAR_INTER_VAL*40*64)/40); //0x33443301;
	data = ((interval*40*64)/40);
	SC1233A_DEBUG("[Write Sensing Intv]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Write Alpha value
	addr = 0x44;
	//Read from the register first to Preserve other fields
	ret = sc1233a_i2c_read_u32_reg(client, addr, &data_read);
	if (ret < 0)
		goto error;
	data = (data_read & 0xFF00FFFF) | ((alpha << 16) & (0x00FF0000));

	//Write the new Alpha value, other fileds unchanged
	SC1233A_DEBUG("[Alpha value]: register write %06x %08x\n", addr, data);
	ret = sc1233a_i2c_write_u32_reg(client, addr, &data);
	if (ret < 0)
		goto error;

	//Issue deep sleep command to reset internal circuit
	cmd_val = MS_CMD_DEEP_SLEEP;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//Issue ENATM command
	cmd_val = MS_CMD_EN_TIMER;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Issue RUNTM command
	cmd_val = MS_CMD_START_SENSING_BY_TIMER;
	//SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
	ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	/* switch to SENSING_TIMER state */
	pdata->state = SENSING_TIMER;

	MSLEEP_INTERRUPTIBLE(100); //Giving enough delay here so that OR pin can be detected

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	SC1233A_ERR("failed, ret=%d\n", ret);
	return ret;
}


static int sc1233a_stop_sensing(struct i2c_client *client, struct sc1233a_data *pdata,
	enum sc1233a_sensing_mode mode, bool from_wq)
{
	u32 addr;
	u32 rdata;
	int ret;
	enum sc1233a_cmd cmd_val;

	SC1233A_DEBUG("enter, %s\n", sc1233a_sensing_mode_str[mode]);

	/* disable or interrupt source */
	sc1233a_enable_irq(pdata, pdata->or_gpio, false);
	del_timer_sync(&or_host_timer);
	del_timer_sync(&or_watch_timer);

	/* if wq run right after stop, i2c err in wq and then recovery is cause of kernel panic */
	if (!from_wq) {
		/* wait pending wq before change settings */
		flush_workqueue(sc1233a_wq);
	}

	if (mode == SENSOR_OFF_MODE){
		/* TODO: motion sensor off from upper layer */
		ret = sc1233a_soft_reset(client, pdata);
		if (ret < 0)
			goto error;

	} else if (mode == CONTINUOUS_MODE) {
		/* Not implemented for now*/

	} else if (mode >= TIMER_MODE && mode <= TIMER_DIS_HIGH_MODE ) {
		cmd_val = MS_CMD_STOP_SENSING_BY_TIMER;
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		cmd_val = MS_CMD_DIS_TIMER;
		SC1233A_DEBUG("[Disable timer cmd]: register write %02x \n", cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		/* sensor enter deep sleep state after MS_CMD_DIS_TIMER */
		pdata->state = DEEP_SLEEP;

	} else if (mode == DETOUT_PIN_TEST_MODE) {
		cmd_val = MS_CMD_STOP_SENSING_BY_TIMER;
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		cmd_val = MS_CMD_HOLD_DT;
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(5000);

		cmd_val = MS_CMD_SOFT_RESET;
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		addr = 0xA1;
		ret = sc1233a_i2c_read_u32_reg(client, addr, &rdata);
		if (ret < 0)
			goto error;
		SC1233A_DEBUG("Wait time value is %d \n", rdata);
		MSLEEP_UNINT(rdata/0x40 + 1); // Let's just wait an additional 1ms to be safe.

		cmd_val = MS_CMD_DEEP_SLEEP;
		SC1233A_DEBUG("[Deep sleep cmd]: register write %02x \n", cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		cmd_val = MS_CMD_UPDATE_DT_2;
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		cmd_val = MS_CMD_DIS_TIMER;
		SC1233A_DEBUG("[Disable timer cmd]: register write %02x \n", cmd_val);
		ret = sc1233a_i2c_write_1byte_only(client, cmd_val);
		if (ret < 0)
			goto error;

		/* sensor enter deep sleep state after MS_CMD_DIS_TIMER */
		pdata->state = DEEP_SLEEP;

		/* switch to LIGHT_SLEEP */
		ret = sc1233a_soft_reset(client, pdata);
		if (ret < 0)
			goto error;

	} else {
		/* defatul stop mode */
		ret = sc1233a_soft_reset(client, pdata);
		if (ret < 0)
			goto error;
	}

	MSLEEP_INTERRUPTIBLE(100); //Giving enough delay here so that OR pin can be detected

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	SC1233A_ERR("failed, ret=%d\n", ret);
	return ret;
}

static u8 log10_dB(u32 number)
{
	u8 value;
	int size = 0;
	int i;

	if ((number < 5) || (number >= 16384))
		return (u8)(3*ilog2(number));

	//size = sizeof(lookup_table)/sizeof(lookup_table[0]);
	value = 7;
	for (i=0; i <LOOKUP_TABLE_SIZE; i++) {
		if (number == lookup_table[i])
			return (u8)(value + i);
		if (number < lookup_table[i])
			break;
	}

	return (u8)(value+i-1);
}

static void sc1233a_last_frames_log_print(struct sc1233a_data *pdata, int number_of_frames)
{
	struct sc1233a_frame_hist *buf = pdata->frame_hist_buf;
	int idx = pdata->frame_hist_idx;
	int n, i;
	bool isFirstFrame = true;

	/* if print 1 frame, print the most recent */
	if (number_of_frames == 1){
	    i = idx;
	}else{
	    i = NEXT_FRAME_BUF_IDX(idx);
	}
	for (n = 0; n < number_of_frames; n++) {
		if (BITMASK_READ(buf[i].status, PRINT_REQ_MASK)) {
			BITMASK_CLEAR(buf[i].status, PRINT_REQ_MASK);
			if (isFirstFrame) {
				BITMASK_SET(buf[i].status, FIRST_FRAME_LOG_MASK);
			}

/* log format
<search keyword>:<status>/<frame timestamp>/<frame interval>/<dist_rx1>/<peak_rx1>/<sensor mode>/<i2c err count>
*/
			printk("sc1233a:0x%X/%lums/%d"
			"/%d,%d,%d,%d,%d"
			"/%d,%d,%d,%d,%d"
			"/%d/%lu\n",
			buf[i].status,
			buf[i].frame_time,
			buf[i].interval,
			((buf[i].u32_dist_rx[0] & 0xFFFF0000)>>16), (buf[i].u32_dist_rx[0] & 0x0000FFFF),
			((buf[i].u32_dist_rx[1] & 0xFFFF0000)>>16), (buf[i].u32_dist_rx[1] & 0x0000FFFF),
			(buf[i].u32_dist_rx[2] & 0x0000FFFF),
			buf[i].rx1_peak[0], buf[i].rx1_peak[1], buf[i].rx1_peak[2], buf[i].rx1_peak[3], buf[i].rx1_peak[4],
			pdata->mode, (long unsigned int)sc1233a_cnt.i2c_err_cnt);

			isFirstFrame = false;
		}
		INC_FRAME_BUF_IDX(i);
	}
	/* Done printing frames. Now, we print fan bins related PCAM logs */
	printk("sc1233a:PCAM:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",
		handle_app.rx1_distance_histogram_10fps_previous[0],
		handle_app.rx1_distance_histogram_10fps_previous[1],
		handle_app.rx1_distance_histogram_10fps_previous[2],
		handle_app.rx1_distance_histogram_10fps_previous[3],
		handle_app.rx1_distance_histogram_10fps_previous[4],
		handle_app.rx1_distance_histogram_10fps_previous[5],
		handle_app.rx1_distance_histogram_10fps_previous[6],
		handle_app.rx1_distance_histogram_10fps_previous[7],
		handle_app.rx1_distance_histogram_10fps_previous[8],
		handle_app.rx1_distance_histogram_10fps_previous[9],
		handle_app.rx1_distance_histogram_10fps_previous[10],
		handle_app.rx1_distance_histogram_10fps_previous[11],
		handle_app.rx1_distance_histogram_10fps_previous[12],
		handle_app.rx1_distance_histogram_10fps_previous[13],
		handle_app.rx1_distance_histogram_10fps_previous[14],
		handle_app.rx1_distance_histogram_10fps_previous[15],
		handle_app.rx1_distance_histogram_10fps_previous[16],
		handle_app.rx1_distance_histogram_10fps_previous[17],
		handle_app.rx1_distance_histogram_10fps_previous[18],
		handle_app.rx1_distance_histogram_10fps_previous[19],
		handle_app.rx1_distance_histogram_10fps_previous[20],
		handle_app.rx1_distance_histogram_10fps_previous[21],
		handle_app.rx1_distance_histogram_10fps_previous[22],
		handle_app.rx1_distance_histogram_10fps_previous[23],
		handle_app.rx1_distance_histogram_10fps_previous[24],
		handle_app.rx1_distance_histogram_10fps_previous[25],
		handle_app.rx1_distance_histogram_10fps_previous[26]);
	printk("sc1233a:LP_PCAM:[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[0],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[1],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[2],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[3],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[4],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[5],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[6],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[7],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[8],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[9],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[10],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[11],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[12],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[13],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[14],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[15],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[16],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[17],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[18],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[19],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[20],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[21],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[22],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[23],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[24],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[25],
		handle_app.rx1_distance_histogram_lowpower_10fps_previous[26]);
}

static int sc1233a_start_fifo_read(struct i2c_client *client, struct sc1233a_data *pdata, enum sc1233a_sensing_mode mode)
{
	u32 data;
	u8 u8_dummy[16];
	u32 rx1_distance1;
	u32 rx1_distances[MAX_NUMBER_OF_TARGETS];
	u8 rx1_qualified_peaks=0;
	u8 threshold;
	int ret;
	int i;
	int gpio_val;
	u8 prev_detection;
	u8 logs_printed = 0;

	u32 u32_dist_rx[3], u32_peak_lv_rx1[MAX_NUMBER_OF_TARGETS];
	u8 rx1_peak[MAX_NUMBER_OF_TARGETS];

	pdata->frame_hist_buf[pdata->frame_hist_idx].status = PRINT_REQ_MASK;
	pdata->frame_hist_buf[pdata->frame_hist_idx].frame_time = ktime_get_boottime_to_ms();

	gpio_val = MDrv_GPIO_Pad_Read(pdata->or_gpio);
	if (gpio_val == 0) {
		SC1233A_ERR("OR pin is low, skip this function\n");
		BITMASK_SET(pdata->frame_hist_buf[pdata->frame_hist_idx].status, OR_LOW_ERROR_MASK);
		INC_FRAME_BUF_IDX(pdata->frame_hist_idx);
		return -400;
	}

	if (mode == CONTINUOUS_MODE)
	{
		//Read_FIFO data over I2C
		//Read 16 bytes from fifo for OR pin to go low
		ret = sc1233a_i2c_read_u8_bulk(client, MS_REG_ADDR_FIFO_RD_MEM, &u8_dummy, sizeof(u8_dummy));
		if (ret < 0)
			goto error;
		SC1233A_INFO("i2c_read_data= %x\n", data);

	} else if (mode == TIMER_MODE) {
		//Issue the command HLDDT (I2C w/ timer) - 0x15 followed by 0xAB
		//Send 0x15 as HLDDT command
		ret = sc1233a_i2c_write_1byte_only((pdata->client), MS_CMD_HOLD_DT);
		if (ret < 0)
			goto error;

		//Write 0xAB opcode to enter light sleep
		ret = sc1233a_i2c_write_soft_reset(pdata->client);
		if (ret < 0)
			goto error;
		USLEEP_UNDER_20MS(1000);

		//Read FIFO data over I2C
		//Read 16 bytes from fifo for OR pin to go low. No need to record the read data
		ret = sc1233a_i2c_read_u8_bulk(client, MS_REG_ADDR_FIFO_RD_MEM, &u8_dummy, sizeof(u8_dummy));
		if (ret < 0)
			goto error;

		/* read MS_REG_ADDR_DIST_RD_RX1_12 to MS_REG_ADDR_PEAKLVL_RD_RX2_5 */
		ret = sc1233a_i2c_read_u32_bulk(pdata->client, MS_REG_ADDR_DIST_RD_RX1_12, u32_dist_rx, 3*4);
		if (ret < 0)
			goto error;

		/* read MS_REG_ADDR_PEAKLVL_RD_RX1_1 to MS_REG_ADDR_PEAKLVL_RD_RX1_5 */
		ret = sc1233a_i2c_read_u32_bulk(pdata->client, MS_REG_ADDR_PEAKLVL_RD_RX1_1, u32_peak_lv_rx1, 5*4);
		if (ret < 0)
			goto error;

		//Command UPDDT (I2C w/timer) - 0xB9 followed by 0x14
		ret = sc1233a_i2c_write_1byte_only((pdata->client), MS_CMD_DEEP_SLEEP);
		if (ret < 0)
			goto error;

		//Write 0x14
		ret = sc1233a_i2c_write_1byte_only((pdata->client), MS_CMD_UPDATE_DT_2);
		if (ret < 0)
			goto error;
		//USLEEP_UNDER_20MS(1000); /* end of function already have enough delay */

		rx1_distance1 = (u32_dist_rx[0] & 0xFFFF0000) >> 16; /* MS_REG_ADDR_DIST_RD_RX1_12 */
		
		rx1_distances[0] = rx1_distance1;
		rx1_distances[1] = (u32_dist_rx[0] & 0x0000FFFF);
		rx1_distances[2] = ((u32_dist_rx[1] & 0xFFFF0000)>>16);
		rx1_distances[3] = (u32_dist_rx[1] & 0x0000FFFF);
		rx1_distances[4] = (u32_dist_rx[2] & 0x0000FFFF);


		rx1_peak[0] = log10_dB(u32_peak_lv_rx1[0]); /* MS_REG_ADDR_PEAKLVL_RD_RX1_1 */
		rx1_peak[1] = log10_dB(u32_peak_lv_rx1[1]);
		rx1_peak[2] = log10_dB(u32_peak_lv_rx1[2]);
		rx1_peak[3] = log10_dB(u32_peak_lv_rx1[3]);
		rx1_peak[4] = log10_dB(u32_peak_lv_rx1[4]); /* MS_REG_ADDR_PEAKLVL_RD_RX1_5 */
		
		for(i=0;i<MAX_NUMBER_OF_TARGETS;i++)
		{
		    if(rx1_peak[i] < app_para.presence_threshold || rx1_peak[i]==INVALID_RX_PEAK)
		        break;
		    rx1_qualified_peaks += 1;
		}

		for (i = 0; i < MAX_NUMBER_OF_TARGETS; i++ ) {
			if (i < 3)
				pdata->frame_hist_buf[pdata->frame_hist_idx].u32_dist_rx[i] = u32_dist_rx[i];
			pdata->frame_hist_buf[pdata->frame_hist_idx].rx1_peak[i] = rx1_peak[i];
		}

		/* Done reading distance and peak registers. Now run algorithm.*/
		if (pdata->mode == ALGO_TEST_MODE ||
			pdata->mode == TIMER_DIS_LOW_MODE ||
			pdata->mode == TIMER_DIS_MID_MODE ||
			pdata->mode == TIMER_DIS_HIGH_MODE) {
			if (rx1_peak[0] != INVALID_RX_PEAK) {
				prev_detection = handle_app.status;
				presence_detection(&handle_app, &app_para, &radar_para, rx1_peak[0], rx1_distance1, rx1_distances, rx1_peak, rx1_qualified_peaks);
				if (handle_app.status) {
					INC_U32_CNT(sc1233a_cnt.presence_cnt);
				}
				SC1233A_DEBUG("Presence Detection Output is: %01d \n", handle_app.status);

				/* status update and print last frames log */
				pdata->frame_hist_buf[pdata->frame_hist_idx].interval = radar_para.interval;
				if (sc1233a_log_level == SC1233A_LOG_ERR)
				{ 				
					if (handle_app.rate_change_flag) {
						BITMASK_SET(pdata->frame_hist_buf[pdata->frame_hist_idx].status, RATE_CHANGE_MASK);
					}
					if (handle_app.status) {
						BITMASK_SET(pdata->frame_hist_buf[pdata->frame_hist_idx].status, PRE_STATUS_MASK);
					}
					if (prev_detection != handle_app.status) {
						BITMASK_SET(pdata->frame_hist_buf[pdata->frame_hist_idx].status, PRE_DET_CHANGE_MASK);
					}
					if (prev_detection != handle_app.status){
					    if (handle_app.status){
					        sc1233a_last_frames_log_print(pdata, 16);
					    }
					    else{
					        sc1233a_last_frames_log_print(pdata, 1);
					    }
					    logs_printed = 1;
					}
					if (handle_app.rate_change_flag && !logs_printed){
					    sc1233a_last_frames_log_print(pdata, 1);
					}
				}
				if (handle_app.rate_change_flag) {
					/*Stop sensing*/
					SC1233A_DEBUG("Started rate change \n");
					ret = sc1233a_stop_sensing(pdata->client, pdata, TIMER_MODE, true);
					if (ret < 0)
						goto error;

					/*Soft reset*/
					ret = sc1233a_soft_reset(pdata->client, pdata);
					if (ret < 0)
						goto error;

					/*Start sensing with new interval, beta*/
					ret = sc1233a_start_sensing(pdata->client, pdata, TIMER_MODE, 1, radar_para.interval, radar_para.beta);
					if (ret < 0)
						goto error;

					/*Clear the flag to change rate*/
					handle_app.rate_change_flag = 0;
					SC1233A_DEBUG("Ended rate change \n");
				}
			}
		}

/* remove per frame log */
//#ifdef DEBUG_LOGS
		/* avoid duplicate log print */
		if (sc1233a_log_level == SC1233A_LOG_INFO) {
			printk("sc1233a:Dist;PkLvl;PrSt;FI;Rate;Th;QP;FC;Score;PowerScore;FanbinCount="
			"[%03d,%03d,%03d,%03d,%03d];"
			"[%02d,%02d,%02d,%02d,%02d];"
			"%01d,%d,%d,%d,%d,%d,%d,%d,%d\n",
			((u32_dist_rx[0] & 0xFFFF0000)>>16), (u32_dist_rx[0] & 0x0000FFFF),
			((u32_dist_rx[1] & 0xFFFF0000)>>16), (u32_dist_rx[1] & 0x0000FFFF),
			(u32_dist_rx[2] & 0x0000FFFF),
			rx1_peak[0], rx1_peak[1], rx1_peak[2], rx1_peak[3], rx1_peak[4],
			handle_app.status,handle_app.frame_index,radar_para.interval,app_para.presence_threshold,rx1_qualified_peaks,handle_app.frame_counter,handle_app.counter_score, handle_app.power_score, handle_app.number_of_fan_bins);
		}
//#endif
	}
	//MSLEEP_INTERRUPTIBLE(10);

	INC_FRAME_BUF_IDX(pdata->frame_hist_idx);
	//SC1233A_DEBUG("ok\n");
	return 0;

error:
	BITMASK_SET(pdata->frame_hist_buf[pdata->frame_hist_idx].status, I2C_ERROR_MASK);
	INC_FRAME_BUF_IDX(pdata->frame_hist_idx);
	SC1233A_ERR("failed, ret=%d\n", ret);
	return ret;
}

static u8 sc1233a_get_presence_status(void)
{
	return handle_app.status;
}

static void sc1233a_or_wq_fn(struct work_struct *work)
{
	struct sc1233a_data *pdata
		= container_of(work, struct sc1233a_data,
			algo_update);
	int ret;
	int int_time_buf_idx = sc1233a_cnt.int_time_buf_idx;
	int histogram_key;

	SC1233A_DEBUG("enter\n");

	mutex_lock(&pdata->wq_lock);

	sc1233a_cnt.ro_time_buf[int_time_buf_idx].wq_start_ms = ktime_get_boottime_to_ms();

	if (pdata->mode == SENSOR_OFF_MODE || pdata->state <= LIGHT_SLEEP) {
		SC1233A_DEBUG("skip wq, invalid sensor state=%d or mode=%d\n", pdata->state, pdata->mode);
		goto end;
	}

	ret = sc1233a_start_fifo_read(pdata->client, pdata, TIMER_MODE); //read fifos
	if (ret == -2 || ret == -4 || ret == -400)
	    ret = sc1233a_attempt_recovery(pdata);
	if (ret < 0) {
		sc1233a_cnt.ro_time_buf[int_time_buf_idx].is_detecded = 'e';
		goto end;
	}

#if 0 //disable for now
		if (1) {
			/* count how many event sent to ambient service */
			sc1233a_cnt.presence_cnt = (++sc1233a_cnt.presence_cnt > 1000) ? 1 : sc1233a_cnt.presence_cnt;
			sc1233a_cnt.ro_time_buf[int_time_buf_idx].is_detecded = 'y';
		}
		else {
			sc1233a_cnt.ro_time_buf[int_time_buf_idx].is_detecded = 'n';
		}
#endif

end:
	sc1233a_cnt.ro_time_buf[int_time_buf_idx].wq_elapse_ms =
		ktime_get_boottime_to_ms() - sc1233a_cnt.ro_time_buf[int_time_buf_idx].wq_start_ms;
	sc1233a_cnt.max_wq_elapse_ms =
		(sc1233a_cnt.ro_time_buf[int_time_buf_idx].wq_elapse_ms > sc1233a_cnt.max_wq_elapse_ms) ?
			sc1233a_cnt.ro_time_buf[int_time_buf_idx].wq_elapse_ms :
			sc1233a_cnt.max_wq_elapse_ms;
	
	histogram_key = (int)(sc1233a_cnt.ro_time_buf[int_time_buf_idx].wq_elapse_ms/25);
	histogram_key = (histogram_key >= 6) ? 6 : histogram_key;
		INC_U32_CNT(sc1233a_cnt.wq_elapse_ms_histogram[histogram_key]);
	
	mutex_unlock(&pdata->wq_lock);
}

static void sc1233a_or_irq_handler(struct sc1233a_data *pdata)
{
	INC_U32_CNT(sc1233a_cnt.ro_int_cnt);
	sc1233a_cnt.int_time_buf_idx = (++sc1233a_cnt.int_time_buf_idx >= MAX_TIME_LOG_BUF) ? 0 : sc1233a_cnt.int_time_buf_idx;
	sc1233a_cnt.ro_time_buf[sc1233a_cnt.int_time_buf_idx].irq_occur = ktime_get_boottime_to_ms();
	sc1233a_cnt.ro_time_buf[sc1233a_cnt.int_time_buf_idx].wq_start_ms = 0;
	sc1233a_cnt.ro_time_buf[sc1233a_cnt.int_time_buf_idx].wq_elapse_ms = 0;

	SC1233A_DEBUG("enter, ro_int_cnt=%d\n", sc1233a_cnt.ro_int_cnt);

	if (!pdata) {
		SC1233A_DEBUG("pdata is NULL\n");
		return;
	}

	if (!sc1233a_wq) {
		SC1233A_DEBUG("%s(), workqueue is NULL\n", __func__ );
		return;
	}
	queue_work(sc1233a_wq, &pdata->algo_update);
	SC1233A_DEBUG("ok\n");
}

static irqreturn_t sc1233a_or_irq(int irq, void *handle)
{
	struct sc1233a_data *pdata = temp_pdata;

	sc1233a_or_irq_handler(pdata);

	return IRQ_HANDLED;
}

static void sc1233a_detout_irq_handler(struct sc1233a_data *pdata)
{
	INC_U32_CNT(sc1233a_cnt.detout_int_cnt);
}

static irqreturn_t sc1233a_detout_irq(int irq, void *handle)
{
	struct sc1233a_data *pdata = handle;

	sc1233a_detout_irq_handler(pdata);

	return IRQ_HANDLED;
}

static void sc1233a_enable_irq(struct sc1233a_data *pdata, int gpio, bool enable)
{
	if (gpio == pdata->or_gpio) {
		if ((pdata->ro_irq_enabled && enable) || (!pdata->ro_irq_enabled && !enable))  {
			SC1233A_DEBUG("skip ro irq already %d %d\n", pdata->ro_irq_enabled, enable);
			return;
		}
		if (enable) {
			request_gpio_irq(pdata->or_gpio, &sc1233a_or_irq, E_GPIO_RISING_EDGE+1, &pdata->client->dev);
		} else {
			//request_gpio_irq(pdata->or_gpio, NULL, E_GPIO_RISING_EDGE+1, &pdata->client->dev);
			free_gpio_irq(pdata->or_gpio, &pdata->client->dev);
		}
		pdata->ro_irq_enabled = enable;
	} else if (gpio == pdata->detout_gpio) {
		if (enable) {
			request_gpio_irq(pdata->detout_gpio, &sc1233a_detout_irq, E_GPIO_RISING_EDGE+1, &pdata->client->dev);
		} else {
			//request_gpio_irq(pdata->detout_gpio, NULL, E_GPIO_RISING_EDGE+1, &pdata->client->dev);
			free_gpio_irq(pdata->detout_gpio, &pdata->client->dev);
		}
	} else {
		SC1233A_ERR("unknown gpio no.(%d)\n", gpio);
	}
}

static void sc1233a_or_timer_func(unsigned long data)
{
	struct sc1233a_data *pdata = (struct sc1233a_data *)data;

	INC_U32_CNT(sc1233a_cnt.ro_int_cnt);
	sc1233a_cnt.int_time_buf_idx = (++sc1233a_cnt.int_time_buf_idx >= MAX_TIME_LOG_BUF) ? 0 : sc1233a_cnt.int_time_buf_idx;
	sc1233a_cnt.ro_time_buf[sc1233a_cnt.int_time_buf_idx].irq_occur = ktime_get_boottime_to_ms();
	sc1233a_cnt.ro_time_buf[sc1233a_cnt.int_time_buf_idx].wq_start_ms = 0;
	sc1233a_cnt.ro_time_buf[sc1233a_cnt.int_time_buf_idx].wq_elapse_ms = 0;

	SC1233A_DEBUG("enter, ro_int_cnt=%d\n", sc1233a_cnt.ro_int_cnt);

	if (!pdata) {
		SC1233A_ERR("pdata is NULL\n");
		return;
	}

	if (!sc1233a_wq) {
		pr_err("%s(), workqueue is NULL\n", __func__ );
		return;
	}
	queue_work(sc1233a_wq, &pdata->algo_update);
	mod_timer(&or_host_timer, jiffies + msecs_to_jiffies(pdata->host_timer_interval));
}

static void sc1233a_or_timer_init(struct sc1233a_data *pdata, unsigned int interval)
{
	pdata->host_timer_interval = interval;

	or_host_timer.expires = jiffies + msecs_to_jiffies(interval);
	or_host_timer.function = sc1233a_or_timer_func;
	or_host_timer.data = (unsigned long)pdata;

	add_timer(&or_host_timer);
}

static void sc1233a_or_check_func(unsigned long data)
{
	struct sc1233a_data *pdata = (struct sc1233a_data *)data;
	if (pdata->mode >= TIMER_DIS_LOW_MODE && pdata->mode <= TIMER_DIS_HIGH_MODE)
	{
		if (sc1233a_cnt.ro_int_cnt == previous_ro_int_cnt)
		{
			// No OR interrupts for the time where the radar is operating actively. Let's try to recover.
			//sc1233a_attempt_recovery(pdata);
			queue_work(sc1233a_wq, &pdata->algo_update);
		}
		else{
			previous_ro_int_cnt = sc1233a_cnt.ro_int_cnt;
		}
	}
	mod_timer(&or_watch_timer, jiffies + msecs_to_jiffies(pdata->watch_timer_interval));
}

static void sc1233a_or_watch_init(struct sc1233a_data *pdata, unsigned int interval)
{
	pdata->watch_timer_interval = interval;

	or_watch_timer.expires = jiffies + msecs_to_jiffies(interval);
	or_watch_timer.function = sc1233a_or_check_func;
	or_watch_timer.data = (unsigned long)pdata;

	add_timer(&or_watch_timer);
}

static int sc1233a_config_radar(struct i2c_client *client, struct sc1233a_data *pdata)
{
	u32 address;
	u32 data;
	u32 crc;
	int ret;
	int i = 0;
	int size = 0;
	char err_msg[SC1233A_ERR_MSG_MAX];
	s64 start_ms, end_ms;

	start_ms = ktime_get_boottime_to_ms();

	//Write the registers _regaddr with _regval
	size = sizeof(mode_2d_normal_60db_1100us_regval) / sizeof(mode_2d_normal_60db_1100us_regval[0]);
	for (i = 0; i < size; i++) {
		address = (u32)mode_2d_normal_60db_1100us_regaddr[i];
		data = (u32)mode_2d_normal_60db_1100us_regval[i];
		ret = sc1233a_i2c_write_u32_reg(client, address, &data);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write reg", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(1000);

	//Disable Sequencer CMD
	address = 0xBF;
	data = 0x0;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Write enable for SEQ SRAM over I2C
	address = 0x9B;
	data = 0x001FFC66; //Bit [1:0]=b10 for write enable on SEQ SRAM in I2C mode
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Write Sequence file, starting at register 0x020000. Each register write is equivalent to 16 bytes
	size = sizeof(mode_2d_normal_60db_1100us_seq) / sizeof(mode_2d_normal_60db_1100us_seq[0]);
	for (i = 0; i < size/16; i ++) {
		address = 0x020000 + i;
		ret = sc1233a_i2c_write_seq_16b(client, address, &mode_2d_normal_60db_1100us_seq[i*16]);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write Seq", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(5000);

	address = 0xCC;
	ret = sc1233a_i2c_read_u32_reg(client, address, &crc);
	if (ret < 0)
		goto error;


	//Write FFT Window values, starting at register 0x030000. Each register write is equivalent to 2 bytes
	size = sizeof(mode_2d_normal_60db_1100us_fftwin) / sizeof(mode_2d_normal_60db_1100us_fftwin[0]);
	for (i = 0; i < size/2; i++) {
		address = 0x030000 + i;
		ret = sc1233a_i2c_write_fft_2b(client, address, &mode_2d_normal_60db_1100us_fftwin[i*2]);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write FFT", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(5000);

	address = 0xCC;
	ret = sc1233a_i2c_read_u32_reg(client, address, &crc);
	if (ret < 0)
		goto error;

	//Write Protected for FFT over I2C by writing value to 0x098
	address = 0x9B;
	data = 0x1FFC77; //Bit [5:4]=b11 for write protect on FFT SRAM
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(5000);

	//soft reset
	ret = sc1233a_soft_reset(client, pdata);
	if (ret < 0)
		goto error;

	//Enable Sequencer CMD
	address = 0xBF;
	data = 0x1;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//soft reset
	ret = sc1233a_soft_reset(client, pdata);
	if (ret < 0)
		goto error;

	end_ms = ktime_get_boottime_to_ms();

	SC1233A_ERR("ok, elapse=%lldms\n", end_ms - start_ms);
	return 0;

error:
	SC1233A_ERR("failed in %s, ret=%d\n", err_msg, ret);
	return ret;
}


static int sc1233a_setup_param_DETOUT_test(struct i2c_client *client, u32 motion_threshold)
{
	/*setup interval*/
	u32 wdata = 1000;
	u32 rdata = 0;
	u32 address = 0xA1;
	u32 mask;
	int ret;
	char err_msg[SC1233A_ERR_MSG_MAX];

	SC1233A_DEBUG("enter\n");

	wdata = wdata * 0x40;
	ret = sc1233a_i2c_write_u32_reg(client, address, &wdata);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "interval update failed");
		goto error;
	}

	/* setup alpha */
	address = 0x44;
	wdata = 153<<16;
	mask = 0xFF0000; // Read the register and update the third byte with new alpha value.

	ret = sc1233a_i2c_modify_u32_reg(client, address, &rdata, wdata, mask);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "setup alpha failed");
		goto error;
	}

	/* setup motion threshold */
	address = 0x45;
	wdata = motion_threshold << 8;
	mask = 0xFFF00;
	ret = sc1233a_i2c_modify_u32_reg(client, address, &rdata, wdata, mask);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "setup threshold failed");
		goto error;
	}

	/* setup start up count */
	address = 0x45;
	wdata = 0 << 0;
	mask = 0xFF;
	ret = sc1233a_i2c_modify_u32_reg(client, address, &rdata, wdata, mask);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "setup startup failed");
		goto error;
	}

	/* setup range - Needs a deep dive. Let's skip for now and see if DETOUT test works without it */

	/* setup HPF parameters - the steps below are applicable only if first order HPF is used */
	/* To do -- replace error messages with more specifics on which register write is failing */
	address = 0xA;
	wdata = 0x33443301;
	ret = sc1233a_i2c_write_u32_reg(client, address, &wdata);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "HPF reg1 failed");
		goto error;
	}

	address = 0xB;
	wdata = 0x31900;
	ret = sc1233a_i2c_write_u32_reg(client, address, &wdata);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "HPF reg2 failed");
		goto error;
	}

	address = 0x11;
	wdata = 0x33443301;
	ret = sc1233a_i2c_write_u32_reg(client, address, &wdata);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "HPF reg3 failed");
		goto error;
	}

	address = 0x12;
	wdata = 0x31900;
	ret = sc1233a_i2c_write_u32_reg(client, address, &wdata);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "HPF reg4 failed");
		goto error;
	}

	address = 0x43;
	wdata = 0xE0380820;
	ret = sc1233a_i2c_write_u32_reg(client, address, &wdata);
	if (ret < 0){
		snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s", "HPF reg5 failed");
		goto error;
	}

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	SC1233A_ERR("failed in %s, ret=%d\n", err_msg, ret);
	return ret;
}

/*To do - Clean up config_radar to accept sequencer as a parameter*/
static int sc1233a_config_radar_DETOUT_test(struct i2c_client *client, struct sc1233a_data *pdata)
{
	u32 address;
	u32 data;
	int ret;
	int i = 0;
	int size = 0;
	char err_msg[SC1233A_ERR_MSG_MAX];

	SC1233A_DEBUG("enter\n");

	//Write the registers _regaddr with _regval
	size = sizeof(mode_motion_wide_40db_1100us_regval) / sizeof(mode_motion_wide_40db_1100us_regval[0]);
	for (i = 0; i < size; i++) {
		address = (u32)mode_motion_wide_40db_1100us_regaddr[i];
		data = (u32)mode_motion_wide_40db_1100us_regval[i];
		ret = sc1233a_i2c_write_u32_reg(client, address, &data);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write reg", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(5000);

	//Disable Sequencer CMD
	address = 0xBF;
	data = 0x0;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Write enable for SEQ SRAM over I2C
	address = 0x9B;
	data = 0x001FFC66; //Bit [1:0]=b10 for write enable on SEQ SRAM in I2C mode
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Write Sequence file, starting at register 0x020000. Each register write is equivalent to 16 bytes
	size = sizeof(mode_motion_wide_40db_1100us_seq) / sizeof(mode_motion_wide_40db_1100us_seq[0]);
	for (i = 0; i < size/16; i ++) {
		address = 0x020000 + i;
		ret = sc1233a_i2c_write_seq_16b(client, address, &mode_motion_wide_40db_1100us_seq[i*16]);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write Seq", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(5000);

	//Write FFT Window values, starting at register 0x030000. Each register write is equivalent to 2 bytes
	size = sizeof(mode_motion_wide_40db_1100us_fftwin) / sizeof(mode_motion_wide_40db_1100us_fftwin[0]);
	for (i = 0; i < size/2; i++) {
		address = 0x030000 + i;
		ret = sc1233a_i2c_write_fft_2b(client, address, &mode_motion_wide_40db_1100us_fftwin[i*2]);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write FFT", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(5000);


	//Write Protected for FFT over I2C by writing value to 0x098
	address = 0x9B;
	data = 0x1FFC77; //Bit [5:4]=b11 for write protect on FFT SRAM
	//data = 0x1FFC77 | 0x10000000;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;

	//soft reset
	ret = sc1233a_soft_reset(client, pdata);
	if (ret < 0)
		goto error;

	//Enable Sequencer CMD
	address = 0xBF;
	data = 0x1;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//soft reset
	ret = sc1233a_soft_reset(client, pdata);
	if (ret < 0)
		goto error;

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	SC1233A_ERR("failed in %s, ret=%d\n", err_msg, ret);
	return ret;
}

static int sc1233a_config_radar_DETOUT(struct i2c_client *client, struct sc1233a_data *pdata)
{
	u32 address;
	u32 data;
	int ret;
	int i = 0;
	int size = 0;
	char err_msg[SC1233A_ERR_MSG_MAX];

	SC1233A_DEBUG("enter\n");

	//Write the registers _regaddr with _regval
	size = sizeof(mode_motion_wide_60db_1100us_regval) / sizeof(mode_motion_wide_60db_1100us_regval[0]);
	for (i = 0; i < size; i++) {
		address = (u32)mode_motion_wide_60db_1100us_regaddr[i];
		data = (u32)mode_motion_wide_60db_1100us_regval[i];
		ret = sc1233a_i2c_write_u32_reg(client, address, &data);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write reg", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(5000);

	//Disable Sequencer CMD
	address = 0xBF;
	data = 0x0;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Write enable for SEQ SRAM over I2C
	address = 0x9B;
	data = 0x001FFC66; //Bit [1:0]=b10 for write enable on SEQ SRAM in I2C mode
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//Write Sequence file, starting at register 0x020000. Each register write is equivalent to 16 bytes
	size = sizeof(mode_motion_wide_60db_1100us_seq) / sizeof(mode_motion_wide_60db_1100us_seq[0]);
	for (i = 0; i < size/16; i ++) {
		address = 0x020000 + i;
		ret = sc1233a_i2c_write_seq_16b(client, address, &mode_motion_wide_60db_1100us_seq[i*16]);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write Seq", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(5000);

	//Write FFT Window values, starting at register 0x030000. Each register write is equivalent to 2 bytes
	size = sizeof(mode_motion_wide_60db_1100us_fftwin) / sizeof(mode_motion_wide_60db_1100us_fftwin[0]);
	for (i = 0; i < size/2; i++) {
		address = 0x030000 + i;
		ret = sc1233a_i2c_write_fft_2b(client, address, &mode_motion_wide_60db_1100us_fftwin[i*2]);
		if (ret < 0) {
			snprintf(err_msg, SC1233A_ERR_MSG_MAX, "%s %d/%d", "write FFT", i, size);
			goto error;
		}
	}
	USLEEP_UNDER_20MS(5000);


	//Write Protected for FFT over I2C by writing value to 0x098
	address = 0x9B;
	data = 0x1FFC77; //Bit [5:4]=b11 for write protect on FFT SRAM
	//data = 0x1FFC77 | 0x10000000;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;

	//soft reset
	ret = sc1233a_soft_reset(client, pdata);
	if (ret < 0)
		goto error;

	//Enable Sequencer CMD
	address = 0xBF;
	data = 0x1;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(1000);

	//soft reset
	ret = sc1233a_soft_reset(client, pdata);
	if (ret < 0)
		goto error;

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	SC1233A_ERR("failed in %s, ret=%d\n", err_msg, ret);
	return ret;
}


static int sc1233a_chip_boot(struct i2c_client *client, struct sc1233a_data *pdata)
{
	u32 address;
	u32 data;
	int ret;

	SC1233A_DEBUG("ce_out=%d, nrst_out=%d\n",
		MDrv_GPIO_Pad_Read(pdata->ce_gpio), MDrv_GPIO_Pad_Read(pdata->nrst_gpio));

	ret = sc1233a_hard_reset(client, pdata);
	if (ret < 0)
		goto error;

	MSLEEP_UNINT(100);

	//Issue the command SRST (0xAB) to enter the Light Sleep state
	ret = sc1233a_soft_reset(client, pdata);
	if (ret < 0)
		goto error;

	//start reading eFuse
	address = 0xBE;
	data = 0x1;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;
	USLEEP_UNDER_20MS(100);

	//stop the clock for eFuse readout
	address = 0x9B;
	data = 0x1FFC77; //77 at the end for write protection
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;

	//Read bit [15:8] of register 0x0000F6 to confirm that hardware configuration was correct
	address = 0xF6;
	data = 0x0;
	ret = sc1233a_i2c_read_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;

	//instruct RF circuit to use eFuse values
	address = 0x40;
	data = 0x0;
	ret = sc1233a_i2c_write_u32_reg(client, address, &data);
	if (ret < 0)
		goto error;

	SC1233A_DEBUG("ok\n");
	return 0;

error:
	dev_err(&client->dev, "%s: failed ret=%d,ce=%d,nrst=%d\n", __func__,
		ret, MDrv_GPIO_Pad_Read(pdata->ce_gpio), MDrv_GPIO_Pad_Read(pdata->nrst_gpio));
	return ret;
}

static int sc1233a_attempt_recovery(struct sc1233a_data *pdata)
{
	int ret;
	int i,j;
	u8 resume;
	u8 beta = 0;
	u16 interval = 0;
	char err_msg[SC1233A_ERR_MSG_MAX];

	del_timer_sync(&or_watch_timer);

	SC1233A_ERR("In Attempt Recovery\n");
	INC_U32_CNT(sc1233a_cnt.recovery_try_cnt);

	if ((pdata->mode != SENSOR_OFF_MODE && pdata->mode < TIMER_DIS_LOW_MODE) || pdata->mode > TIMER_DIS_HIGH_MODE)
		return -500;

	for (i=0; i<NUMBER_OF_RETRIES; i++) {
		ret = sc1233a_hard_reset(pdata->client, pdata);
		if (ret < 0) {
			MSLEEP_INTERRUPTIBLE(1000);
		} else {
			break;
		}
	}
	if (ret < 0)
		return ret;

	SC1233A_DEBUG("Successfully reset the radar in attempt recovery\n");
	// Successfully reset the radar.
	if (pdata->mode == SENSOR_OFF_MODE){
		ret = sc1233a_soft_reset(pdata->client, pdata);
		if (ret < 0)
			goto error;

		//Negate CE Pin and NRST Pin
		MDrv_GPIO_Set_Low(pdata->nrst_gpio);
		UDELAY_UNDER_10US(10);
		MDrv_GPIO_Set_Low(pdata->ce_gpio);
		MSLEEP_INTERRUPTIBLE(100);
		SC1233A_DEBUG("enter shutdown, ce_out=%d, nrst_out=%d\n",
			MDrv_GPIO_Pad_Read(pdata->ce_gpio), MDrv_GPIO_Pad_Read(pdata->nrst_gpio));

	}
	else{
		ret = sc1233a_chip_boot(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 2: Configure the radar */
		ret = sc1233a_config_radar(pdata->client, pdata);
		if (ret < 0)
			goto error;

		resume = 0;
		if (handle_app.initialized == 1){
			interval = radar_para.interval;
			beta = radar_para.beta;      // Trade-off in ingress detection latency, if the person leaves while attempting to recover the radar.
			number_of_fan_bins = handle_app.number_of_fan_bins;
		        for(j=0;j<DIS_DIM;j++)
		        {
		            fan_bins[j] = handle_app.fan_bins[j];
		        }
			empty_room_detected_with_low_power = handle_app.empty_room_fan_detected_with_low_power;
		}
		else{
		/* step 3: Initialize Algo parameters*/
			presence_init(&handle_app, fan_bins, number_of_fan_bins,empty_room_detected_with_low_power);
			parameter_app_update(&app_para, 10);
			parameter_radar_dynamic_update(&radar_para, 100);
			interval = 100;
			beta = 205;
		}

		/* step 4: Start Sensing in Timer mode */
		/* TODO: Config the distance range*/
		ret = sc1233a_start_sensing(pdata->client, pdata, TIMER_MODE, resume, interval, beta);
		if (ret < 0)
			goto error;

		ret = sc1233a_stop_sensing(pdata->client, pdata, TIMER_MODE, true);
		if (ret < 0)
			goto error;

		/*Soft reset*/
		ret = sc1233a_soft_reset(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 5: update distance*/
		ret = sc1233a_start_sensing_update_distance(pdata->client, pdata, pdata->mode);
		if (ret < 0)
			goto error;

		SC1233A_DEBUG("Successfully restarted the radar in attempt recovery\n");

		MSLEEP_UNINT(OR_WATCH_INIT_DELAY);

		sc1233a_or_watch_init(pdata, OR_WATCH_TIMER_PERIOD);
	}
	INC_U32_CNT(sc1233a_cnt.recovery_success_cnt);
	return ret;

error:
	SC1233A_ERR("failed in sc1233a_attempt_recovery, ret=%d\n", ret);
	return ret;
}

/* read from DeviceTree */
#ifdef CONFIG_OF
int sc1233a_init_dt(struct sc1233a_data *pdata)
{
	struct device_node *np = pdata->of_node;
	u32 val;

	if (!pdata->of_node) {
		SC1233A_ERR("DeviceTree data required\n");
		return -EINVAL;
	}

	if (!of_property_read_u32(np, "ce-gpio", &val))
		pdata->ce_gpio = val;

	if (!of_property_read_u32(np, "nrst-gpio", &val))
		pdata->nrst_gpio = val;

	if (!of_property_read_u32(np, "or-gpio", &val))
		pdata->or_gpio = val;

	if (!of_property_read_u32(np, "detout-gpio", &val))
		pdata->detout_gpio = val;

	SC1233A_DEBUG("ce-gpio=%d, nrstg-pio=%d, or-gpio=%d, detout-gpio=%d\n",
		pdata->ce_gpio, pdata->nrst_gpio, pdata->or_gpio, pdata->detout_gpio);
	return 0;
}

static const struct of_device_id sc1233a_i2c_dt_ids[] = {
	{ .compatible = "socionext,sc1233a" },
	{}
};
MODULE_DEVICE_TABLE(of, sc1233a_i2c_dt_ids);
#endif /* CONFIG_OF */


/* efues sysfs */
static ssize_t sc1233a_efuse_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 address;
	u32 data;
	int ret;

	struct sc1233a_data *pdata = dev_get_drvdata(dev);

	mutex_lock(&pdata->sysfs_lock);

	/* step 1 */
	address = 0xBE;
	data = 0x1;
	ret = sc1233a_i2c_write_u32_reg(pdata->client, address, &data);
	if (ret < 0)
		goto error;

	/* step 2 */
	address = 0x9B;
	data = 0x1FFC44;
	ret = sc1233a_i2c_write_u32_reg(pdata->client, address, &data);
	if (ret < 0)
		goto error;

	/* step 3 */
	address = 0xF6;
	data = 0;
	ret = sc1233a_i2c_read_u32_reg(pdata->client, address, &data);
	if (ret < 0)
		goto error;

	mutex_unlock(&pdata->sysfs_lock);
	return snprintf(buf, PAGE_SIZE, "0x%lX\n", (long unsigned int)data);

error:
	mutex_unlock(&pdata->sysfs_lock);
	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}
static DEVICE_ATTR(efuse, 0444, sc1233a_efuse_show, NULL);


/* test_i2c_rw sysfs */
static ssize_t sc1233a_test_i2c_rw_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 address;
	u32 data;
	int ret;

	struct sc1233a_data *pdata = dev_get_drvdata(dev);

	mutex_lock(&pdata->sysfs_lock);

	/* step 4 */
	address = 0x67;
	data = 0x12345679;
	ret = sc1233a_i2c_write_u32_reg(pdata->client, address, &data);
	if (ret < 0)
		goto error;

	/* step 5 */
	address = 0x67;
	data = 0;
	ret = sc1233a_i2c_read_u32_reg(pdata->client, address, &data);
	if (ret < 0)
		goto error;

	mutex_unlock(&pdata->sysfs_lock);
	return snprintf(buf, PAGE_SIZE, "0x%lX\n", (long unsigned int)data);

error:
	mutex_unlock(&pdata->sysfs_lock);
	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}
static DEVICE_ATTR(test_i2c_rw, 0444, sc1233a_test_i2c_rw_show, NULL);


/**
 * sysfs: debug_counter
 *
 * @read: show event history
  */
static ssize_t sc1233a_counter_dump_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	u32 debug_mode;
	int i;
	u32 interval;

	if (sscanf(buf, "%X", &debug_mode) < 1) {
		return -ENXIO;
	}

	mutex_lock(&pdata->sysfs_lock);

	if (debug_mode == 0) {
		interval = sc1233a_cnt.frame_interval_cnt;
		memset(&sc1233a_cnt, 0, sizeof(sc1233a_cnt));
		sc1233a_cnt.frame_interval_cnt = interval;
	} else if (debug_mode == 1) {
		for (i = 0; i < MAX_TIME_LOG_BUF; i++) {
			SC1233A_ERR("[%s%2d] ro:%lld(%lld),wq:%lld(%lld),el:%lld,det=%c\n",
				(i == sc1233a_cnt.int_time_buf_idx) ? "*" : "",
				i,
				sc1233a_cnt.ro_time_buf[i].irq_occur,
				(i == 0) ?
					sc1233a_cnt.ro_time_buf[i].irq_occur - sc1233a_cnt.ro_time_buf[MAX_TIME_LOG_BUF - 1].irq_occur :
					sc1233a_cnt.ro_time_buf[i].irq_occur - sc1233a_cnt.ro_time_buf[i - 1].irq_occur,
				sc1233a_cnt.ro_time_buf[i].wq_start_ms,
				sc1233a_cnt.ro_time_buf[i].wq_start_ms - sc1233a_cnt.ro_time_buf[i].irq_occur,
				sc1233a_cnt.ro_time_buf[i].wq_elapse_ms,
				sc1233a_cnt.ro_time_buf[i].is_detecded);
		}
	}

	mutex_unlock(&pdata->sysfs_lock);
	return size;
}

static ssize_t sc1233a_counter_dump_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);

	return snprintf(buf, PAGE_SIZE,
		"PRESEN:%c\n"
		"MODE:%s STATE:%s I2CERR:%lu ERR:%lu REC:%lu/%lu\n"
		"PRESCNT:%lu ROINT:%lu DETINT:%lu WAKEUP:%lu MAXWQ:%lu INTV:%lu\n"
		"<25ms <50ms <75ms <100ms <125ms <150ms >150ms\n"
		"%lu %lu %lu %lu %lu %lu %lu\n",
		sc1233a_get_presence_status() ? 'Y' : 'N',
		sc1233a_sensing_mode_str[pdata->mode],
		sc1233a_state_str[pdata->state],
		(long unsigned int)sc1233a_cnt.i2c_err_cnt,
		(long unsigned int)sc1233a_cnt.err_cnt,
		(long unsigned int)sc1233a_cnt.recovery_success_cnt,
		(long unsigned int)sc1233a_cnt.recovery_try_cnt,
		(long unsigned int)sc1233a_cnt.presence_cnt,
		(long unsigned int)sc1233a_cnt.ro_int_cnt,
		(long unsigned int)sc1233a_cnt.detout_int_cnt,
		(long unsigned int)sc1233a_cnt.wake_up_cnt,
		(long unsigned int)sc1233a_cnt.max_wq_elapse_ms,
		(long unsigned int)sc1233a_cnt.frame_interval_cnt,
		(long unsigned int)sc1233a_cnt.wq_elapse_ms_histogram[0],
		(long unsigned int)sc1233a_cnt.wq_elapse_ms_histogram[1],
		(long unsigned int)sc1233a_cnt.wq_elapse_ms_histogram[2],
		(long unsigned int)sc1233a_cnt.wq_elapse_ms_histogram[3],
		(long unsigned int)sc1233a_cnt.wq_elapse_ms_histogram[4],
		(long unsigned int)sc1233a_cnt.wq_elapse_ms_histogram[5],
		(long unsigned int)sc1233a_cnt.wq_elapse_ms_histogram[6]);
}
static DEVICE_ATTR(debug_counter, 0644, sc1233a_counter_dump_show, sc1233a_counter_dump_store);


/**
 * sysfs: log_level
 *
 * @read: display current log level
 * @write: set log level
 */
static ssize_t sc1233a_log_level_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n"
		"  0: SC1233A_ERR(default)\n"
		"  1: SC1233A_INFO\n"
		"  2: SC1233A_DEBUG\n", sc1233a_log_level);
}

static ssize_t sc1233a_log_level_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	u32 log_lv;

	sscanf(buf, "%u", &log_lv);

	mutex_lock(&pdata->sysfs_lock);

	if (log_lv < SC1233A_LOG_MAX)
		sc1233a_log_level = log_lv;
	else
		sc1233a_log_level = SC1233A_LOG_MAX - 1;

	mutex_unlock(&pdata->sysfs_lock);
	return size;
}
static DEVICE_ATTR(log_level, 0644, sc1233a_log_level_show, sc1233a_log_level_store);


/**
 * sysfs: status_reg
 *
 * @read: status=0x%X status2=%d, OR=%d, DETOUT=%d
 * @write: one byte write for status register
 */
static ssize_t sc1233a_status_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u8 status_data;
	u8 status2_data;
	u8 or_gpio_in;
	u8 detout_gpio_in;
	int ret;
	struct sc1233a_data *pdata = dev_get_drvdata(dev);

	mutex_lock(&pdata->sysfs_lock);

	or_gpio_in = MDrv_GPIO_Pad_Read(pdata->or_gpio);
	detout_gpio_in = MDrv_GPIO_Pad_Read(pdata->detout_gpio);

	/* only can access status reg in LIGHT_SLEEP state */
	if (pdata->state != LIGHT_SLEEP) {
		mutex_unlock(&pdata->sysfs_lock);
		return snprintf(buf, PAGE_SIZE,
			"status=N/A status2=N/A, OR=%d, DETOUT=%d\n",
			or_gpio_in, detout_gpio_in);
	}

	status_data = 0x0;
	ret = sc1233a_i2c_read_status_reg(pdata->client, &status_data);
	if (ret < 0)
		goto error;

	status2_data = 0x0;
	ret = sc1233a_i2c_read_status_reg2(pdata->client, &status2_data);
	if (ret < 0)
		goto error;

	mutex_unlock(&pdata->sysfs_lock);
	return snprintf(buf, PAGE_SIZE, "status=0x%X status2=%d, OR=%d, DETOUT=%d\n"
		"[7]:%s [6]:%s [5]:%s [4]:%s [3]:%s [2]:%s  [1]:%s  [0]:%s  [R2][0]:%s\n",
		status_data, (status2_data&0x1),
		or_gpio_in, detout_gpio_in,
		(status_data & SC1233A_ST_FIFO_OR_MASK) ? sc1233a_status_reg_str[SC1233A_ST_FIFO_OR] : "",
		(status_data & SC1233A_ST_FIFO_UDF_MASK) ? sc1233a_status_reg_str[SC1233A_ST_FIFO_UDF] : "",
		(status_data & SC1233A_ST_FIFO_OVF_MASK) ? sc1233a_status_reg_str[SC1233A_ST_FIFO_OVF] : "",
		(status_data & SC1233A_ST_UNDEF_COM_MASK) ? sc1233a_status_reg_str[SC1233A_ST_UNDEF_COM] : "",
		(status_data & SC1233A_ST_START_STOP_MASK) ? sc1233a_status_reg_str[SC1233A_ST_START_STOP] : "STOP",
		(status_data & SC1233A_ST_CONT_SING_MASK) ? sc1233a_status_reg_str[SC1233A_ST_CONT_SING] : "SINGLE",
		(status_data & SC1233A_ST_SEQ_ERR_MASK) ? sc1233a_status_reg_str[SC1233A_ST_SEQ_ERR] : "",
		(status_data & SC1233A_ST_SEQ_BUSY_MASK) ? sc1233a_status_reg_str[SC1233A_ST_SEQ_BUSY] : "",
		(status2_data & SC1233A_ST2_DETOUT_MASK) ? sc1233a_status_reg_str[SC1233A_ST_MAX + SC1233A_ST2_DETOUT] : "");

error:
	mutex_unlock(&pdata->sysfs_lock);
	return snprintf(buf, PAGE_SIZE, "status register read failed, ret=%d\n", ret);
}

static ssize_t sc1233a_status_reg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 u32_data;
	int ret;
	struct sc1233a_data *pdata = dev_get_drvdata(dev);

	if (pdata->state < LIGHT_SLEEP)
		return -ENXIO;

	if (sscanf(buf, "%X", &u32_data) < 1) {
		SC1233A_ERR("sscanf error\n");
		return -ENXIO;
	}

	mutex_lock(&pdata->sysfs_lock);

	ret = sc1233a_i2c_write_status_reg(pdata->client, (u8)u32_data);
	if (ret < 0)
		goto error;

	mutex_unlock(&pdata->sysfs_lock);
	return size;

error:
	mutex_unlock(&pdata->sysfs_lock);
	return -ENXIO;
}
static DEVICE_ATTR(status_reg, 0644, sc1233a_status_reg_show, sc1233a_status_reg_store);


/**
 * sysfs: i2ctransfer
 *
 * @read: display last i2ctransfer result
 * @write: usage: i2ctransfer w <n> [r/w] <m> 0xXX 0xXX 0xXX...
 */
static struct sc1233a_last_i2c_sysfs last_i2ctransfer;
static ssize_t sc1233a_i2ctransfer_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE,
		"last: ret=%d, w %d %c %d addr:0x%X data:0x%X 0x%X 0x%X 0x%X\n\n"
		"usage: i2ctransfer w <n> [r/w] <m> 0xXX 0xXX 0xXX...\n"
		"  n:write bytes(1,4), m:read(1,4)/write(0,1,2,4,16) bytes, *must use hexdecimal only for i2c data\n"
		"  ex) i2ctransfer w 4 r 4 0x0B 0x00 0x00 0x67\n"
		"  ex) i2ctransfer w 1 w 1 0x01 0xFF\n",
		last_i2ctransfer.ret, last_i2ctransfer.write_bytes,
		last_i2ctransfer.mode_2nd, last_i2ctransfer.total_bytes,
		last_i2ctransfer.addr,
		last_i2ctransfer.data[0], last_i2ctransfer.data[1],
		last_i2ctransfer.data[2], last_i2ctransfer.data[3]);
}

static ssize_t sc1233a_i2ctransfer_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 address = 0;
	u32 data[20];
	char mode_1st, mode_2nd;
	u32 write_bytes, rw_bytes;
	int num;

	int ret;
	struct sc1233a_data *pdata = dev_get_drvdata(dev);

	num = sscanf(buf, "%c %u %c %u %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X",
		&mode_1st, &write_bytes, &mode_2nd, &rw_bytes,
		&data[0], &data[1], &data[2], &data[3],
		&data[4], &data[5], &data[6], &data[7],
		&data[8], &data[9], &data[10], &data[11],
		&data[12], &data[13], &data[14], &data[15],
		&data[16], &data[17], &data[18], &data[19]);

	SC1233A_DEBUG("%c %d %c %d %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X\n",
		mode_1st, write_bytes, mode_2nd, rw_bytes,
		data[0], data[1], data[2], data[3],
		data[4], data[5], data[6], data[7],
		data[8], data[9], data[10], data[11],
		data[12], data[13], data[14], data[15],
		data[16], data[17], data[18], data[19]);

	mutex_lock(&pdata->sysfs_lock);

	if (write_bytes==1 && rw_bytes==0) {
		address = data[0];
		ret = sc1233a_i2c_write_1byte_only(pdata->client, data[0]);
	} else if (write_bytes==1 && rw_bytes==1) {
		address = data[0];
		if (mode_2nd == 'w') {
			data[0] = data[1];
			ret = sc1233a_i2c_write_byte(pdata->client, address, data[0]);
		} else {
			data[0] = 0x0;
			ret = sc1233a_i2c_read_byte(pdata->client, address, (u8 *)&data[0]);
		}
	} else if (mode_2nd == 'w') {
		address =
			data[1] << 16 |
			data[2] <<  8 |
			data[3] <<  0;
		if (rw_bytes == 2) {
			data[0] =
				data[4] <<  8 |
				data[5] <<  0;
		} else /* 4 or 16 */ {
			data[0] =
				data[4] << 24 |
				data[5] << 16 |
				data[6] <<  8 |
				data[7] <<  0;
			data[1] =
				data[8] << 24 |
				data[9] << 16 |
				data[10] <<  8 |
				data[11] <<  0;
			data[2] =
				data[12] << 24 |
				data[13] << 16 |
				data[14] <<  8 |
				data[15] <<  0;
			data[3] =
				data[16] << 24 |
				data[17] << 16 |
				data[18] <<  8 |
				data[19] <<  0;
		}

		ret = sc1233a_i2c_write_u32_bulk(pdata->client, address, &data[0], rw_bytes);
	} else {
		address =
			data[1] << 16 |
			data[2] <<  8 |
			data[3] <<  0;
		ret = sc1233a_i2c_read_u32_bulk(pdata->client, address, &data[0], rw_bytes);
	}

	last_i2ctransfer.addr = address;
	last_i2ctransfer.data[0] = data[0];
	last_i2ctransfer.data[1] = data[1];
	last_i2ctransfer.data[2] = data[2];
	last_i2ctransfer.data[3] = data[3];
	last_i2ctransfer.mode_2nd = mode_2nd;
	last_i2ctransfer.write_bytes = write_bytes;
	last_i2ctransfer.total_bytes = rw_bytes;
	last_i2ctransfer.ret = ret;

	mutex_unlock(&pdata->sysfs_lock);
	return size;
}
static DEVICE_ATTR(i2ctransfer, 0644, sc1233a_i2ctransfer_show, sc1233a_i2ctransfer_store);


/**
 * sysfs: cli_sensor_restart
 *
 * @read: display last restart mode and result
 * @write: 1:hard reset, 2:soft reset, 3:chip boot
 */
static int sc1233a_last_cli_sensor_restart_result;
static ssize_t sc1233a_cli_sensor_restart_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE,
		"last mode=%d, ret=%s\n"
		"  1:hard reset, 2:soft reset, 3:chip boot\n",
		sc1233a_last_cli_sensor_restart_result >> 8,
		(sc1233a_last_cli_sensor_restart_result & 0xFF) ? "OK" : "NG");
}

static ssize_t sc1233a_cli_sensor_restart_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	int ret;
	u32 restart_mode;

	mutex_lock(&pdata->sysfs_lock);

	sscanf(buf, "%u", &restart_mode);
	sc1233a_last_cli_sensor_restart_result = (restart_mode << 8);

	if (restart_mode == 1) {
		ret = sc1233a_hard_reset(pdata->client, pdata);
		SC1233A_DEBUG("Presence sensor mode is %d\n", pdata->mode);
	} else if (restart_mode == 2) {
		ret = sc1233a_soft_reset(pdata->client, pdata);
	} else if (restart_mode == 3) {
		ret = sc1233a_chip_boot(pdata->client, pdata);
	} else {
		ret = -ENXIO;
	}

	if (ret < 0)
		sc1233a_last_cli_sensor_restart_result |= SC1233A_LAST_RESULT_NG;
	else
		sc1233a_last_cli_sensor_restart_result |= SC1233A_LAST_RESULT_OK;

	mutex_unlock(&pdata->sysfs_lock);
	return size;
}
static DEVICE_ATTR(cli_sensor_restart, 0644, sc1233a_cli_sensor_restart_show, sc1233a_cli_sensor_restart_store);


/**
 * sysfs: cli_sensor_set_config
 *
 * @read: display last sensing mode and result
 * @write: sensing mode loading
 */
static int sc1233a_last_cli_sensor_set_config_result;
static ssize_t sc1233a_cli_sensor_set_config_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	/* this description must match with enum sc1233a_cli_set_config_mode */
	return snprintf(buf, PAGE_SIZE, "last mode=%d, ret=%s\n"
		"   1: DETOUT mode\n"
		"   2: Presence Tracking mode\n"
		"   3: FFT Mode\n"
		"   4: ARF/BRF testing mode\n"
		"   5: Compliance testing mode\n"
		"  11: timer mode test\n"
		"  12: continuous mode test\n"
		"  13: chip boot for mode stop \n"
		"  14: algorithm test mode\n",
		sc1233a_last_cli_sensor_set_config_result >> 8,
		(sc1233a_last_cli_sensor_set_config_result & 0xFF) ? "OK" : "NG");
}

static ssize_t sc1233a_cli_sensor_set_config_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	int ret,j;
	enum sc1233a_cli_set_config_mode cli_mode;
	u8 resume;
	u8 beta = 0;
	u16 interval = 0;

	mutex_lock(&pdata->sysfs_lock);

	sscanf(buf, "%u", &cli_mode);
	sc1233a_last_cli_sensor_set_config_result = (cli_mode << 8);

	SC1233A_DEBUG("enter cli_mode=%d\n", cli_mode);

	pdata->mode = CLI_MODE;
	if (cli_mode == CLI_TIMER_MODE_TEST) {
		if (sc1233a_log_level < SC1233A_LOG_INFO)
			sc1233a_log_level = SC1233A_LOG_INFO;

		/* step 1: Chip boot */
		ret = sc1233a_chip_boot(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 2: Configure the radar */
		ret = sc1233a_config_radar(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 3: Start Sensing in Timer mode */
		resume = 0;
		interval = 100;
		beta = 205;
		ret = sc1233a_start_sensing(pdata->client, pdata, TIMER_MODE, resume, interval, beta);

		if (ret < 0)
			goto error;

	} else if (cli_mode == CLI_CONTINUOUS_MODE_TEST) {
		/* step 1: Chip boot */
		ret = sc1233a_chip_boot(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 2: Configure the radar */
		ret = sc1233a_config_radar(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 4: Start Sensing in Continuous mode */
		resume = 0;
		ret = sc1233a_start_sensing(pdata->client, pdata, CONTINUOUS_MODE, resume, interval, beta);
		if (ret < 0)
			goto error;

	} else if (cli_mode == CLI_ALGO_MODE_TEST) {
		pdata->mode = TIMER_DIS_HIGH_MODE; // This by default just starts the radar in HIGH distance mode.

		/* step 1: Chip boot */
		ret = sc1233a_chip_boot(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 2: Configure the radar */
		ret = sc1233a_config_radar(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 3: Initialize Algo parameters*/
		if (handle_app.initialized == 1)
		{
		    number_of_fan_bins = handle_app.number_of_fan_bins;
		    for(j=0;j<DIS_DIM;j++)
		    {
		        fan_bins[j] = handle_app.fan_bins[j];
		    }
		    empty_room_detected_with_low_power = handle_app.empty_room_fan_detected_with_low_power;
		}
		presence_init(&handle_app, fan_bins, number_of_fan_bins, empty_room_detected_with_low_power);
		parameter_app_update(&app_para, 10);
		parameter_radar_dynamic_update(&radar_para, 100);

		/* step 4: Start Sensing in Timer mode */
		resume = 0;
		interval = 100;
		beta = 205;
		/* TODO: Config the distance range*/
		ret = sc1233a_start_sensing(pdata->client, pdata, TIMER_MODE, resume, interval, beta);

		if (ret < 0)
			goto error;
	} else if (cli_mode == CLI_DETOUT_MODE) {
		pdata->mode = TIMER_DIS_HIGH_MODE;

		del_timer_sync(&or_watch_timer); // When testing this mode, OR interrupts will not increase, so let us not attempt to reset the radar.
		flush_workqueue(sc1233a_wq);

		// Let's first reset the radar before booting and reconfiguring the radar.
		ret = sc1233a_soft_reset(pdata->client, pdata);
		if (ret < 0)
			goto error;

		MSLEEP_INTERRUPTIBLE(1000);

		/* step 1: Chip boot */
		ret = sc1233a_chip_boot(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 2: Configure the radar for DETOUT test*/
		ret = sc1233a_config_radar_DETOUT(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 3: Start Sensing in timer mode */
		ret = sc1233a_start_sensing_with_distance_DETOUT(pdata->client, pdata, pdata->mode);
		if (ret < 0)
			goto error;
	} else if (cli_mode == CLI_CHIP_BOOT_MODE) {
		ret = sc1233a_chip_boot(pdata->client, pdata);
		if (ret < 0)
			goto error;
		del_timer_sync(&or_host_timer);

	} else {
		ret = -ENXIO;
	}

	if (ret < 0)
		sc1233a_last_cli_sensor_set_config_result |= SC1233A_LAST_RESULT_NG;
	else
		sc1233a_last_cli_sensor_set_config_result |= SC1233A_LAST_RESULT_OK;

	mutex_unlock(&pdata->sysfs_lock);
	SC1233A_DEBUG("ok\n");
	return size;

error:
	mutex_unlock(&pdata->sysfs_lock);
	SC1233A_ERR("mode %d failed, ret=%d\n", cli_mode, ret);
	return size;
}
static DEVICE_ATTR(cli_sensor_set_config, 0644, sc1233a_cli_sensor_set_config_show, sc1233a_cli_sensor_set_config_store);


/**
 * sysfs: cli_sensor_get_config
 *
 * @read: TBD, this sysfs same as status_reg sysfs for now
 * @write: 
 */
static DEVICE_ATTR(cli_sensor_get_config, 0644, sc1233a_status_reg_show, sc1233a_status_reg_store);


/**
 * sysfs: cli_sensor_read_write_test
 *
 * @read: display last cli_sensor_read_write_test result
 * @write: usage: cli_sensor_read_write_test <hex:address> <hex:data>
 */
static struct sc1233a_last_i2c_sysfs cli_sensor_read_write_test_result;
static ssize_t sc1233a_cli_sensor_read_write_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE,
		"%s\naddr:0x%lX, data:0x%lX\n"
		"  usage: usage: cli_sensor_read_write_test <hex:address> <hex:data>\n",
		(cli_sensor_read_write_test_result.ret) ? "OK" : "NG",
		(long unsigned int)cli_sensor_read_write_test_result.addr,
		(long unsigned int)cli_sensor_read_write_test_result.data[0]);
}

static ssize_t sc1233a_cli_sensor_read_write_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	int ret;
	u32 addr = 0;
	u32 u32_data;

	if (sscanf(buf, "%X %X", &addr, &u32_data) < 2) {
		cli_sensor_read_write_test_result.ret = SC1233A_LAST_RESULT_NG;
		return -ENXIO;
	}

	mutex_lock(&pdata->sysfs_lock);

	ret = sc1233a_i2c_write_u32_reg(pdata->client, addr, &u32_data);
	/* read when no error in write */
	if (ret >= 0) {
		ret = sc1233a_i2c_read_u32_reg(pdata->client, addr, &u32_data);
	}

	cli_sensor_read_write_test_result.addr = addr;
	cli_sensor_read_write_test_result.data[0] = u32_data;
	cli_sensor_read_write_test_result.ret = (ret < 0) ? SC1233A_LAST_RESULT_NG : SC1233A_LAST_RESULT_OK;

	mutex_unlock(&pdata->sysfs_lock);
	return (ret < 0) ? -ENXIO : size;
}
static DEVICE_ATTR(cli_sensor_read_write_test, 0644, sc1233a_cli_sensor_read_write_test_show, sc1233a_cli_sensor_read_write_test_store);


/**
 * sysfs: cli_sensor_i2c_read_block
 *
 * @read: display last i2c_read_block result
 * @write: usage: cli_sensor_i2c_read_block <hex:address> <decimal:no. of registers>
 */
static struct sc1233a_last_i2c_sysfs last_cli_sensor_i2c_read_block_result;
static ssize_t sc1233a_cli_sensor_i2c_read_block_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int buf_write;
	int i;

	if (last_cli_sensor_i2c_read_block_result.p_u32_data == NULL)
		last_cli_sensor_i2c_read_block_result.ret = SC1233A_LAST_RESULT_NG;

	buf_write = snprintf(buf, PAGE_SIZE,
		"last: ret=%s, addr:0x%lX, no_of_reg:%d\n"
		"  usage: cli_sensor_i2c_read_block <hex:address> <decimal:no. of registers>\n",
		(last_cli_sensor_i2c_read_block_result.ret) ? "OK" : "NG",
		(long unsigned int)last_cli_sensor_i2c_read_block_result.addr,
		last_cli_sensor_i2c_read_block_result.total_bytes);

	//SC1233A_DEBUG("num=%d, PAGE_SIZE=%d, no_of_reg=%d\n", num, PAGE_SIZE,
	//	last_cli_sensor_i2c_read_block_result.total_bytes);

	if (last_cli_sensor_i2c_read_block_result.ret != SC1233A_LAST_RESULT_OK)
		return buf_write;

	for (i = 0; i < last_cli_sensor_i2c_read_block_result.total_bytes; i++) {
		buf_write += snprintf(buf + buf_write, PAGE_SIZE - buf_write,
			"0x%06lX: %04lX %04lX\n",
			(long unsigned int)last_cli_sensor_i2c_read_block_result.addr + i,
			(long unsigned int)last_cli_sensor_i2c_read_block_result.p_u32_data[i] >> 16,
			(long unsigned int)last_cli_sensor_i2c_read_block_result.p_u32_data[i] & 0xFFFF);
		/* not enough output buffer */
		if (buf_write >= PAGE_SIZE)
			break;
	}

	return buf_write;
}

static ssize_t sc1233a_cli_sensor_i2c_read_block_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	int ret;
	int i;
	u32 *p_u32_data = last_cli_sensor_i2c_read_block_result.p_u32_data;
	u32 addr = 0;
	u32 no_of_reg;

	if (sscanf(buf, "%X %u", &addr, &no_of_reg) < 2) {
		last_cli_sensor_i2c_read_block_result.ret = SC1233A_LAST_RESULT_NG;
		return -ENXIO;
	}

	/* each register have 32bit data */
	if (no_of_reg > (SC1233A_LAST_I2C_SYSFS_MAX_READ_DATA / 32) )
		no_of_reg = (SC1233A_LAST_I2C_SYSFS_MAX_READ_DATA / 32);

	mutex_lock(&pdata->sysfs_lock);

	/* allocate max size memory for first time only */
	if (p_u32_data == NULL) {
		p_u32_data = devm_kzalloc(&pdata->client->dev,
			SC1233A_LAST_I2C_SYSFS_MAX_READ_DATA,
			GFP_KERNEL);
		if (!p_u32_data) {
			SC1233A_ERR("p_u32_data is NULL\n");
			last_cli_sensor_i2c_read_block_result.ret = SC1233A_LAST_RESULT_NG;
			mutex_unlock(&pdata->sysfs_lock);
			return -ENXIO;
		}
		last_cli_sensor_i2c_read_block_result.p_u32_data = p_u32_data;
	}

	memset(p_u32_data, 0, SC1233A_LAST_I2C_SYSFS_MAX_READ_DATA);
	for (i = 0; i < no_of_reg; i++) {
		ret = sc1233a_i2c_read_u32_reg(pdata->client, addr + i, &p_u32_data[i]);
		if (ret < 0) {
			SC1233A_ERR("failed at addr=0x%X, start addr:0x%X, ret=%d\n",
				addr + i, addr, ret);
			break;
		}
		//SC1233A_DEBUG("addr=0x%X, data[%d]=0x%X\n", addr+i, i, p_u32_data[i]);
	}
	last_cli_sensor_i2c_read_block_result.addr = addr;
	last_cli_sensor_i2c_read_block_result.total_bytes = no_of_reg;
	last_cli_sensor_i2c_read_block_result.ret = (ret < 0) ? SC1233A_LAST_RESULT_NG : SC1233A_LAST_RESULT_OK;

	mutex_unlock(&pdata->sysfs_lock);
	return (ret < 0) ? -ENXIO : size;
}
static DEVICE_ATTR(cli_sensor_i2c_read_block, 0644, sc1233a_cli_sensor_i2c_read_block_show, sc1233a_cli_sensor_i2c_read_block_store);


/**
 * sysfs: cli_sensor_i2c_write_block
 *
 * @read: display last i2c_write_block result
 * @write: usage: cli_sensor_i2c_write_block <hex:address> <hex:data1> ... <hex:data16>
 */
static struct sc1233a_last_i2c_sysfs last_cli_sensor_i2c_write_block_result;
static ssize_t sc1233a_cli_sensor_i2c_write_block_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE,
		"last: ret=%s, addr:0x%lX, no_of_data:%d\n"
		"  usage: cli_sensor_i2c_write_block <hex:address> <hex:data1> ... <hex:data16>\n",
		(last_cli_sensor_i2c_write_block_result.ret) ? "OK" : "NG",
		(long unsigned int)last_cli_sensor_i2c_write_block_result.addr,
		last_cli_sensor_i2c_write_block_result.total_bytes);
}

static ssize_t sc1233a_cli_sensor_i2c_write_block_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 no_of_data;
	u32 addr = 0;
	u32 u32_data[16];
	int i;
	int ret;
	struct sc1233a_data *pdata = dev_get_drvdata(dev);

	mutex_lock(&pdata->sysfs_lock);

	/* 1 addr, max. 16 data */
	no_of_data = sscanf(buf, "%X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X",
		&addr,
		&u32_data[0], &u32_data[1], &u32_data[2], &u32_data[3],
		&u32_data[4], &u32_data[5], &u32_data[6], &u32_data[7],
		&u32_data[8], &u32_data[9], &u32_data[10], &u32_data[11],
		&u32_data[12], &u32_data[13], &u32_data[14], &u32_data[15]);

	no_of_data--; 
	for (i = 0; i < no_of_data; i++) {
		ret = sc1233a_i2c_write_u32_reg(pdata->client, addr + i, &u32_data[i]);
		if (ret < 0) {
			SC1233A_ERR("failed at addr=0x%X, data=%d, ret=%d\n",
				addr+i, u32_data[i], ret);
			break;
		}
		//SC1233A_DEBUG("addr=0x%X, data[%d]=0x%X\n", addr+i, i, u32_data[i]);
	}
	last_cli_sensor_i2c_write_block_result.addr = addr;
	last_cli_sensor_i2c_write_block_result.total_bytes = no_of_data;
	last_cli_sensor_i2c_write_block_result.ret = (ret < 0) ? SC1233A_LAST_RESULT_NG : SC1233A_LAST_RESULT_OK;

	mutex_unlock(&pdata->sysfs_lock);
	return (ret < 0) ? -ENXIO : size;
}
static DEVICE_ATTR(cli_sensor_i2c_write_block, 0644, sc1233a_cli_sensor_i2c_write_block_show, sc1233a_cli_sensor_i2c_write_block_store);


/**
 * sysfs: cli_sensor_radar_Tx_start
 *
 * @write: 
 */
static ssize_t sc1233a_cli_sensor_radar_Tx_start_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 freq_val;
	int num;
	int ret;

	num = sscanf(buf, "%u", &freq_val);
	ret = 0;

	return size;
}
static DEVICE_ATTR(cli_sensor_radar_Tx_start, 0200, NULL, sc1233a_cli_sensor_radar_Tx_start_store);


/**
 * sysfs: cli_sensor_radar_Tx_stop
 *
 * @write: 
 */
static ssize_t sc1233a_cli_sensor_radar_Tx_stop_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;

	ret = 0;

	return size;
}
static DEVICE_ATTR(cli_sensor_radar_Tx_stop, 0200, NULL, sc1233a_cli_sensor_radar_Tx_stop_store);


/**
 * sysfs: cli_sensor_radar_Rx_start
 *
 * @write: 
 */
static ssize_t sc1233a_cli_sensor_radar_Rx_start_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 freq_val;
	int num;
	int ret;

	num = sscanf(buf, "%u", &freq_val);
	ret = 0;

	return size;
}
static DEVICE_ATTR(cli_sensor_radar_Rx_start, 0200, NULL, sc1233a_cli_sensor_radar_Rx_start_store);


/**
 * sysfs: cli_sensor_radar_Rx_stop
 *
 * @write: 
 */
static ssize_t sc1233a_cli_sensor_radar_Rx_stop_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int ret;

	ret = 0;

	return size;
}
static DEVICE_ATTR(cli_sensor_radar_Rx_stop, 0200, NULL, sc1233a_cli_sensor_radar_Rx_stop_store);


/**
 * sysfs: cli_sensor_run_compl_Tx
 *
 * @write: 
 */
static ssize_t sc1233a_cli_sensor_run_compl_Tx_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 freq_val;
	int num;
	int ret;

	num = sscanf(buf, "%u", &freq_val);
	ret = 0;

	return size;
}
static DEVICE_ATTR(cli_sensor_run_compl_Tx, 0200, NULL, sc1233a_cli_sensor_run_compl_Tx_store);


/**
 * sysfs: cli_sensor_run_compl_Rx
 *
 * @write: 
 */
static ssize_t sc1233a_cli_sensor_run_compl_Rx_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 freq_val;
	int num;
	int ret;

	num = sscanf(buf, "%u", &freq_val);
	ret = 0;

	return size;
}
static DEVICE_ATTR(cli_sensor_run_compl_Rx, 0200, NULL, sc1233a_cli_sensor_run_compl_Rx_store);


/**
 * sysfs: cli_sensor_radar_config_store
 *
 * @write: 
 */
static ssize_t sc1233a_cli_sensor_radar_config_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	u32 beta, frame_rate, chirp_per_frame, chirp_dur;
	int num;
	int ret;

	num = sscanf(buf, "%u %u %u %u", &beta, &frame_rate, &chirp_per_frame, &chirp_dur);
	ret = 0;

	return size;
}
static DEVICE_ATTR(cli_sensor_radar_config, 0200, NULL, sc1233a_cli_sensor_radar_config_store);


/**
 * sysfs: cli_sensor_presence_distance
 *
 * @read: 
 */
static ssize_t sc1233a_cli_sensor_presence_distance_read(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int data = 0;

	return snprintf(buf, PAGE_SIZE, "0x%X\n", data);
}
static DEVICE_ATTR(cli_sensor_presence_distance_read, 0444, sc1233a_cli_sensor_presence_distance_read, NULL);


/**
 * sysfs: cli_sensor_noise_read
 *
 * @read: 
 */
static ssize_t sc1233a_cli_sensor_noise_read(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int data = 0;

	return snprintf(buf, PAGE_SIZE, "0x%X\n", data);
}
static DEVICE_ATTR(cli_sensor_noise_read, 0444, sc1233a_cli_sensor_noise_read, NULL);


/**
 * sysfs: cli_sensor_read_or_pin
 *
 * @read: OR(Output Ready) gpio input pin value
 */
static ssize_t sc1233a_cli_sensor_read_or_pin_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	u8 or_pin_val;

	mutex_lock(&pdata->sysfs_lock);

	or_pin_val = MDrv_GPIO_Pad_Read(pdata->or_gpio);

	mutex_unlock(&pdata->sysfs_lock);
	return snprintf(buf, PAGE_SIZE, "%d\n", or_pin_val);
}
static DEVICE_ATTR(cli_sensor_read_or_pin, 0444, sc1233a_cli_sensor_read_or_pin_show, NULL);


/**
 * sysfs: cli_sensor_test_or_pin
 *
 * @write: enforce OR(Output Ready) gpio input pin value
 */
static ssize_t sc1233a_cli_sensor_test_or_pin_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	int ret;
	u32 addr;
	u32 data;
	u32 or_pin_val;

	sscanf(buf, "%u", &or_pin_val);

	mutex_lock(&pdata->sysfs_lock);

	pdata->mode = OR_PIN_TEST_MODE;

	/* do not use chip_boot() here because OR pin keep high */
	ret = sc1233a_hard_reset(pdata->client, pdata);
	if (ret < 0)
		goto error;

	ret = sc1233a_soft_reset(pdata->client, pdata);
	if (ret < 0)
		goto error;

	// write 0x000002 to register 0x00af
	addr = 0xAF;
	data = 0x2;
	ret = sc1233a_i2c_write_u32_reg(pdata->client, addr, &data);
	if (ret < 0)
		goto error;

	// write 0x80000002 to register 0x00b5
	addr = 0xB5;
	data = 0x80000002;
	ret = sc1233a_i2c_write_u32_reg(pdata->client, addr, &data);
	if (ret < 0)
		goto error;

	// set watermark to 0(OR pin high) or 1 (OR pin low)
	addr = 0xA4;
	data = (or_pin_val == 0) ? 0x1 : 0x0;
	ret = sc1233a_i2c_write_u32_reg(pdata->client, addr, &data);
	if (ret < 0)
		goto error;

	mutex_unlock(&pdata->sysfs_lock);
	return size;

error:
	mutex_unlock(&pdata->sysfs_lock);
	SC1233A_ERR("failed, ret=%d\n", ret);
	return size;
}
static DEVICE_ATTR(cli_sensor_test_or_pin, 0200, NULL, sc1233a_cli_sensor_test_or_pin_store);


/**
 * sysfs: cli_sensor_read_detout_pin
 *
 * @read: DETOUT gpio input pin value
 */
static ssize_t sc1233a_cli_sensor_read_detout_pin_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	u8 detout_pin_val;

	mutex_lock(&pdata->sysfs_lock);

	detout_pin_val = MDrv_GPIO_Pad_Read(pdata->detout_gpio);

	mutex_unlock(&pdata->sysfs_lock);
	return snprintf(buf, PAGE_SIZE, "%d\n", detout_pin_val);
}
static DEVICE_ATTR(cli_sensor_read_detout_pin, 0444, sc1233a_cli_sensor_read_detout_pin_show, NULL);


/**
 * sysfs: cli_sensor_test_detout_pin
 *
 * @write: enforce DETOUT gpio input pin value
 */
static ssize_t sc1233a_cli_sensor_test_detout_pin_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	int ret;
	u32 detout_pin_val;
	u32 motion_threshold = 0;
	u16 interval;
	u8 beta;

	sscanf(buf, "%u", &detout_pin_val);

	mutex_lock(&pdata->sysfs_lock);

	pdata->mode = DETOUT_PIN_TEST_MODE;
	motion_threshold = (detout_pin_val == 0) ? 4095 : 0;

	/* step 1: Chip boot */
	ret = sc1233a_chip_boot(pdata->client, pdata);
	if (ret < 0)
		goto error;

	/* step 2: Configure the radar for DETOUT test*/
	ret = sc1233a_config_radar_DETOUT_test(pdata->client, pdata);
	if (ret < 0)
		goto error;

	/* step 3: Setup parameters for DETOUT test*/
	ret = sc1233a_setup_param_DETOUT_test(pdata->client, motion_threshold);
	if (ret < 0)
		goto error;

	/* step 4: Start Sensing in timer mode */
	interval = 1000;
	beta = 52;
	ret = sc1233a_start_sensing(pdata->client, pdata, DETOUT_PIN_TEST_MODE, 0, interval, beta);
	if (ret < 0)
		goto error;

	/* step 5 -- Let's try to read DETOUT */
	ret = sc1233a_soft_reset(pdata->client, pdata);
	if (ret < 0)
		goto error;

	/* step 6: Stop sensing in timer mode */
	ret = sc1233a_stop_sensing(pdata->client, pdata, DETOUT_PIN_TEST_MODE, false);
	if (ret < 0)
		goto error;

	mutex_unlock(&pdata->sysfs_lock);
	return size;

error:
	mutex_unlock(&pdata->sysfs_lock);
	SC1233A_ERR("failed, ret=%d\n", ret);
	return size;
}
static DEVICE_ATTR(cli_sensor_test_detout_pin, 0200, NULL, sc1233a_cli_sensor_test_detout_pin_store);


/**
 * sysfs: hal_presence_status
 *
 * @read: display current presence status
 */
static ssize_t sc1233a_hal_presence_status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", sc1233a_get_presence_status());
}
static DEVICE_ATTR(hal_presence_status, 0444, sc1233a_hal_presence_status_show, NULL);


/**
 * sysfs: hal_distance_set_mode
 *
 * @write: sensing mode set from upper layer
 *           0: sensor off
 *           1: distance low
 *           2: distance mid
 *           3: distance high
 */
static ssize_t sc1233a_hal_distance_set_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	int ret,j;
	u32 distance_mode_val;
	u8 resume;
	u8 beta = 0;
	u16 interval = 0;

	sscanf(buf, "%u", &distance_mode_val);

	SC1233A_ERR("enter mode=%d; sc1233a version is:%d.%d\n", distance_mode_val, MAJOR_VERSION, MINOR_VERSION);

	mutex_lock(&pdata->sysfs_lock);

	if (!(distance_mode_val == HAL_SENSOR_OFF_MODE ||
		distance_mode_val == HAL_TIMER_DIS_LOW_MODE ||
		distance_mode_val == HAL_TIMER_DIS_MID_MODE ||
		distance_mode_val == HAL_TIMER_DIS_HIGH_MODE)) {

		SC1233A_ERR("unknown distance set mode %d\n", distance_mode_val);
		ret = -EINVAL;
		goto error;
	}

	/* disable or interrupt source */
	sc1233a_enable_irq(pdata, pdata->or_gpio, false);
	del_timer_sync(&or_host_timer);
	del_timer_sync(&or_watch_timer);
	/* wait pending wq before change settings */
	flush_workqueue(sc1233a_wq);

	/* i2c not working in shutdown mode */
	if (pdata->mode != SENSOR_OFF_MODE) {
		ret = sc1233a_stop_sensing(pdata->client, pdata, TIMER_MODE, false);
		if (ret < 0)
			goto error;
	}

	if (distance_mode_val == HAL_SENSOR_OFF_MODE) {
		if (pdata->mode == SENSOR_OFF_MODE) {
			SC1233A_ERR("already in sensor off mode, mode=%d, state=%d\n", pdata->mode, pdata->state);
			goto error;
		}

		//Issue the command SRST (0xAB) to enter the Light Sleep state
		ret = sc1233a_soft_reset(pdata->client, pdata);
		if (ret < 0)
			goto error;

		//Negate CE Pin and NRST Pin
		MDrv_GPIO_Set_Low(pdata->nrst_gpio);
		UDELAY_UNDER_10US(10);
		MDrv_GPIO_Set_Low(pdata->ce_gpio);
		MSLEEP_INTERRUPTIBLE(100);
		SC1233A_DEBUG("enter shutdown, ce_out=%d, nrst_out=%d\n",
			MDrv_GPIO_Pad_Read(pdata->ce_gpio), MDrv_GPIO_Pad_Read(pdata->nrst_gpio));

		pdata->mode = SENSOR_OFF_MODE;
		pdata->state = SHUTDOWN;
		handle_app.status = 0; // Clear presence status when turning off the radar.
	} else {
		/* step 1: Chip boot */
		ret = sc1233a_chip_boot(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 2: Configure the radar */
		ret = sc1233a_config_radar(pdata->client, pdata);
		if (ret < 0)
			goto error;

		/* step 3: Initialize Algo parameters*/
		if (handle_app.initialized == 1)
		{
		    number_of_fan_bins = handle_app.number_of_fan_bins;
		    for(j=0;j<DIS_DIM;j++)
		    {
		        fan_bins[j] = handle_app.fan_bins[j];
		    }
		    empty_room_detected_with_low_power = handle_app.empty_room_fan_detected_with_low_power;
		    
		}
		presence_init(&handle_app, fan_bins, number_of_fan_bins, empty_room_detected_with_low_power);
		parameter_app_update(&app_para, 10);
		parameter_radar_dynamic_update(&radar_para, 100);

		/* step 4: Start Sensing in Timer mode */
		resume = 0;
		interval = 100;
		beta = 205;
		/* TODO: Config the distance range*/
		ret = sc1233a_start_sensing(pdata->client, pdata, TIMER_MODE, resume, interval, beta);
		if (ret < 0)
			goto error;

		ret = sc1233a_stop_sensing(pdata->client, pdata, TIMER_MODE, false);
		if (ret < 0)
			goto error;

		/*Soft reset*/
		ret = sc1233a_soft_reset(pdata->client, pdata);
		if (ret < 0)
			goto error;

		if (distance_mode_val == HAL_TIMER_DIS_LOW_MODE)
			pdata->mode = TIMER_DIS_LOW_MODE;
		else if (distance_mode_val == HAL_TIMER_DIS_MID_MODE)
			pdata->mode = TIMER_DIS_MID_MODE;
		else if (distance_mode_val == HAL_TIMER_DIS_HIGH_MODE)
			pdata->mode = TIMER_DIS_HIGH_MODE;

		/* step 5: update distance, beta*/
		ret = sc1233a_start_sensing_update_distance(pdata->client, pdata, pdata->mode);
		if (ret < 0)
			goto error;

		sc1233a_or_watch_init(pdata, OR_WATCH_TIMER_PERIOD);
	}

	mutex_unlock(&pdata->sysfs_lock);
	return size;

error:
	if (ret == -2 || ret == -4)
		ret = sc1233a_attempt_recovery(pdata);
	mutex_unlock(&pdata->sysfs_lock);
	if (ret < 0)
		SC1233A_ERR("failed, ret=%d\n", ret);
	return size;
}
static DEVICE_ATTR(hal_distance_set_mode, 0200, NULL, sc1233a_hal_distance_set_mode_store);

/**
 * sysfs: cli_toggle_presence
 *
 * @read: returns 1 if presence toggled successfully. 0 otherwise. This feature will work only if the radar is not in regular operational modes.
 */
static ssize_t sc1233a_cli_toggle_presence(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	mutex_lock(&pdata->sysfs_lock);
	if ((pdata->mode < TIMER_DIS_LOW_MODE || pdata->mode > TIMER_DIS_HIGH_MODE) && (handle_app.initialized == 1)){
		handle_app.status = (handle_app.status + 1)%2;
		mutex_unlock(&pdata->sysfs_lock);
		return snprintf(buf, PAGE_SIZE, "1\n");
	}
	mutex_unlock(&pdata->sysfs_lock);
	SC1233A_ERR("Toggle only when radar is off.\n");
	return snprintf(buf, PAGE_SIZE, "-1\n");;
}
static DEVICE_ATTR(cli_toggle_presence, 0444, sc1233a_cli_toggle_presence, NULL);

/**
 * register sysfs attributes
 */
static struct attribute *sc1233a_attributes[] = {
	&dev_attr_efuse.attr,
	&dev_attr_test_i2c_rw.attr,
	&dev_attr_debug_counter.attr,
	&dev_attr_log_level.attr,
	&dev_attr_status_reg.attr,
	&dev_attr_i2ctransfer.attr,
	&dev_attr_cli_sensor_restart.attr,
	&dev_attr_cli_sensor_set_config.attr,
	&dev_attr_cli_sensor_get_config.attr,
	//&dev_attr_cli_sensor_scan_i2c_addr.attr,
	&dev_attr_cli_sensor_read_write_test.attr,
	&dev_attr_cli_sensor_i2c_read_block.attr,
	&dev_attr_cli_sensor_i2c_write_block.attr,
	&dev_attr_cli_sensor_radar_Tx_start.attr,
	&dev_attr_cli_sensor_radar_Tx_stop.attr,
	&dev_attr_cli_sensor_radar_Rx_start.attr,
	&dev_attr_cli_sensor_radar_Rx_stop.attr,
	&dev_attr_cli_sensor_run_compl_Tx.attr,
	&dev_attr_cli_sensor_run_compl_Rx.attr,
	&dev_attr_cli_sensor_read_or_pin.attr,
	&dev_attr_cli_sensor_test_or_pin.attr,
	&dev_attr_cli_sensor_read_detout_pin.attr,
	&dev_attr_cli_sensor_test_detout_pin.attr,
	//&dev_attr_cli_sensor_amb_user_status.attr,
	&dev_attr_cli_sensor_radar_config.attr,
	&dev_attr_cli_sensor_presence_distance_read.attr,
	&dev_attr_cli_sensor_noise_read.attr,
	&dev_attr_hal_presence_status.attr,
	&dev_attr_hal_distance_set_mode.attr,
	&dev_attr_cli_toggle_presence.attr,
	NULL
};

static const struct attribute_group sc1233a_attr_group = {
	.attrs = sc1233a_attributes,
};


static int sc1233a_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct device *dev = &client->dev;
	struct sc1233a_data *pdata = dev->platform_data;
	//struct sched_param param = { .sched_priority = 50 };

	SC1233A_ERR("enter\n");
	
	// Log the version of code when module loaded.
	printk("sc1233a version is: "
		"%d.%d \n",
		MAJOR_VERSION,MINOR_VERSION);

#ifdef CONFIG_OF
	if (!pdata) {
		pdata = devm_kzalloc(&client->dev,
			sizeof(struct sc1233a_data),
			GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;
		if (of_match_device(sc1233a_i2c_dt_ids, &client->dev)) {
			pdata->of_node = client->dev.of_node;
			ret = sc1233a_init_dt(pdata);
			if (ret < 0)
				return ret;
		} else {
			SC1233A_ERR("no matched dt ids\n");
			return -EINVAL;
		}
	}
#endif

	if (!pdata) {
		SC1233A_ERR("platform data required\n");
		ret = -EINVAL;
		goto init_failed;
	}

	temp_pdata = pdata;
	mutex_init(&pdata->sysfs_lock);
	mutex_init(&pdata->wq_lock);
	init_timer(&or_host_timer);
	pdata->client = client;
	i2c_set_clientdata(client, pdata);
	pdata->state = NO_INIT;
	pdata->mode = SENSOR_INIT_MODE;
	pdata->ro_irq_enabled = false;
	pdata->frame_hist_idx = 0;
	memset(&sc1233a_cnt, 0, sizeof(sc1233a_cnt));

	ret = sysfs_create_group(&client->dev.kobj,
		&sc1233a_attr_group);
	if (ret) {
		SC1233A_ERR("sysfs create fail\n");
		goto base_sysfs_failed;
	}

	/* step 1: Chip boot */
	ret = sc1233a_chip_boot(client, pdata);
	if (ret == -2 || ret == -4)
	    ret = sc1233a_attempt_recovery(pdata);
	if (ret < 0)
		goto base_sysfs_failed;
	/* step 2: Configure the radar */
	// skip seq. download temporary, test by sysfs
	//ret = sc1233a_config_radar(client, pdata);
	//if (ret < 0)
		//goto error;

	/* current MTK kernel support only one callback for all PM gpios */
	/* can't pass pdata pointer to irq handler due to MTK share pm_irq_handler for all irq, TBD */
	//request_gpio_irq(pdata->or_gpio, &sc1233a_or_irq, E_GPIO_RISING_EDGE+1, NULL);
	sc1233a_enable_irq(pdata, pdata->detout_gpio, true);

	sc1233a_wq = create_singlethread_workqueue("sc1233a_wq");
	if (!sc1233a_wq) {
		sc1233a_cnt.err_cnt = (++sc1233a_cnt.err_cnt >= MAX_U32_CNT) ? 1 : sc1233a_cnt.err_cnt;
		SC1233A_ERR("create_singlethread_workqueue error\n");
	}
	INIT_WORK(&pdata->algo_update, sc1233a_or_wq_fn);

	SC1233A_ERR("ok\n");
	return 0;

/* critical error before create sysfs */
init_failed:
	dev_err(&client->dev, "%s: init failed ret=%d\n", __func__, ret);
	if (pdata)
		devm_kfree(&client->dev, pdata);
	return ret;

/* delete sysfs if hit this error */
base_sysfs_failed:
	dev_err(&client->dev, "%s: base sysfs failed ret=%d\n", __func__, ret);
	sysfs_remove_group(&client->dev.kobj, &sc1233a_attr_group);
	return ret;

/* critical error */
irq_register_fail:
	dev_err(&client->dev, "%s: irq register failed ret=%d\n", __func__, ret);
	return ret;

/* recoverable error */
error:
	SC1233A_ERR("failed with recoverable error\n");
	return 0;
};

static int sc1233a_remove(struct i2c_client *client)
{
	struct sc1233a_data *pdata = i2c_get_clientdata(client);

	if (pdata) {
		sc1233a_enable_irq(pdata, pdata->or_gpio, false);
		sc1233a_enable_irq(pdata, pdata->detout_gpio, false);
	}
	del_timer_sync(&or_host_timer);
	del_timer_sync(&or_watch_timer);

	if (sc1233a_wq) {
		flush_workqueue(sc1233a_wq);
		destroy_workqueue(sc1233a_wq);
	}

	if (last_cli_sensor_i2c_read_block_result.p_u32_data) {
		devm_kfree(&client->dev, last_cli_sensor_i2c_read_block_result.p_u32_data);
		last_cli_sensor_i2c_read_block_result.p_u32_data = NULL;
	}

	//i2c_set_clientdata(client, NULL);
	sysfs_remove_group(&client->dev.kobj, &sc1233a_attr_group);

#ifdef CONFIG_OF
	if (pdata)
		devm_kfree(&client->dev, pdata);
#endif

	return 0;
}


#ifdef CONFIG_PM_SLEEP
static int sc1233a_motion_sensor_suspend(struct device *dev)
{
	struct sc1233a_data *pdata = dev_get_drvdata(dev);
	int ret=0;
	ktime_t uptime;
	s64 uptime_ms;

	mutex_lock(&pdata->sysfs_lock);

	uptime = ktime_get_boottime();
	uptime_ms = ktime_to_ms(uptime);
	SC1233A_DEBUG("enter, uptime=%lld\n", uptime_ms / 1000);

	/* Turn off the radar if it is running */
	if (pdata->mode >= TIMER_DIS_LOW_MODE && pdata->mode <= TIMER_DIS_HIGH_MODE) {

		del_timer_sync(&or_watch_timer);
		flush_workqueue(sc1233a_wq);

		ret = sc1233a_soft_reset(pdata->client, pdata);
		if (ret < 0)
			goto error;

		//Negate CE Pin and NRST Pin
		MDrv_GPIO_Set_Low(pdata->nrst_gpio);
		UDELAY_UNDER_10US(10);
		MDrv_GPIO_Set_Low(pdata->ce_gpio);
		MSLEEP_INTERRUPTIBLE(100);
		SC1233A_DEBUG("enter shutdown, ce_out=%d, nrst_out=%d\n",
			MDrv_GPIO_Pad_Read(pdata->ce_gpio), MDrv_GPIO_Pad_Read(pdata->nrst_gpio));

		pdata->mode = SENSOR_OFF_MODE;
		pdata->state = SHUTDOWN;
	}
	mutex_unlock(&pdata->sysfs_lock);
	return ret;

error:
	// Do we want to attempt recovery when going to STR? Most likely will fail anyway.
	if (ret == -2 || ret == -4)
		ret = sc1233a_attempt_recovery(pdata);
	mutex_unlock(&pdata->sysfs_lock);
	if (ret < 0)
		SC1233A_ERR("failed, ret=%d\n", ret);
	return ret;
}

static int sc1233a_motion_sensor_resume(struct device *dev)
{
	ktime_t uptime;
	s64 uptime_ms;
	
	
	uptime = ktime_get_boottime();
	uptime_ms = ktime_to_ms(uptime);
	SC1233A_DEBUG("enter, uptime=%lld\n", uptime_ms / 1000);
	
	
	if (MDrv_PM_GetWakeupSource() == PM_WAKEUPSRC_GPIO_WOPS) {
		SC1233A_DEBUG("## wake up by PM_WAKEUPSRC_GPIO_WOPS ##\n");
		INC_U32_CNT(sc1233a_cnt.wake_up_cnt);
	}
	return 0;
}
#endif /* CONFIG_PM_SLEEP */
static SIMPLE_DEV_PM_OPS(sc1233a_pm_ops,
	sc1233a_motion_sensor_suspend, sc1233a_motion_sensor_resume);


static const struct i2c_device_id i2c_sc1233a_id[] = {
	{"sc1233a", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, i2c_sc1233a_id);

static struct i2c_driver i2c_sc1233a_driver = {
	.driver = {
		.name = "sc1233a",
		.owner = THIS_MODULE,
		.pm = &sc1233a_pm_ops,
	},
	.id_table = i2c_sc1233a_id,
	.probe = sc1233a_probe,
	.remove = sc1233a_remove,
};

static int __init sc1233a_init(void)
{
	int ret;

	SC1233A_DEBUG("enter\n");
	ret = i2c_add_driver(&i2c_sc1233a_driver);
	if (ret < 0) {
		SC1233A_ERR("failed ret=%d\n", ret);
		return ret;
	}

	return ret;
}

static void __exit sc1233a_exit(void)
{
	i2c_del_driver(&i2c_sc1233a_driver);
}

module_init(sc1233a_init);
module_exit(sc1233a_exit);

MODULE_DESCRIPTION("socionext sc1233a motion sensor driver");
MODULE_LICENSE("GPL");
