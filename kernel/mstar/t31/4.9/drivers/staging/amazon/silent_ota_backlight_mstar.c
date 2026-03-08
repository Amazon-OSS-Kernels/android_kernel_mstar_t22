/*
 * Copyright (c) 2017 Amazon, Inc.
 * This program is free software. You can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program;
 */

#include <linux/types.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/silent_ota_backlight.h>
#include <linux/sign_of_life.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/input-event-codes.h>

#include "mdrv_gpio.h"
#include "mhal_gpio.h"
#if defined(CONFIG_MSTAR_PM)
#include "mdrv_pm.h"
#endif
#ifdef CONFIG_MSTAR_PWM
#include "mdrv_pwm.h"
#endif

#include "mdrv_mspi.h"

static DEFINE_MUTEX(backlight_lock);

#define REG_ADDR(addr)                           (*((volatile unsigned short int*)(mstar_pm_base + (addr << 1))))
#define PM_ADDR_OFFSET				 0x060a

/* read 2 byte */
#define REG_RR(_reg_)                            ({REG_ADDR(_reg_); })

/* write 2 byte */
#define REG_W2B(_reg_, _val_)    \
		do { REG_ADDR(_reg_) = (_val_); } while (0)

#define DTS_BACKLIGHT_GPIO_DEFAULT   "bl_ctrl"
#define DTS_BACKLIGHT_GPIO_PREFIX    "bl_ctrl_"
#define DTS_BACKLIGHT_INVERT_DEFAULT "bl_ctrl_inverse"
#define DTS_BACKLIGHT_INVERT_PREFIX  "bl_ctrl_inverse_"
#define DTS_STRING_LENGTH 64

/* Global Variables */
static unsigned int backlight_status; /* 1: on, 0: off */
static unsigned int boot_complete;
static unsigned int backlight_inverse;
static unsigned int backlight_gpio;

static unsigned int led_device_backlight_status; /* 1: on, 0: off */

char *idme_get_config_name(void);
char *idme_get_product_name(void);

//Supported LDM Led device type
typedef enum {
    LDM_LED_DEVICE_TYPE_NONE,
    LDM_LED_DEVICE_TYPE_APE5030,
    LDM_LED_DEVICE_TYPE_MAX,
}SupportedLedDeviceType_e;

/* APE5030 Related.. */
#define APE5030_CH_MAP_ARG "ape5030_ch_map="
#define APE5030_DEV_NUM_ARG "ape5030_dev_num="

// Registers
#define LDM_APE5030_SINGLE_BYTE      0x1
#define LDM_APE5030_SINGLE_DEV       0x0
#define LDM_APE5030_MULTI_DEV        0x1
#define LDM_APE5030_WRITE_IDX        0x0
#define LDM_APE5030_READ_IDX         0x1

//MSPI setting
#define LOCAL_DIMMING_MSPI_DC_TRSTART (0x00)
#define LOCAL_DIMMING_MSPI_DC_TREND (0x00)
#define LOCAL_DIMMING_MSPI_DC_TB (0x03)//0x0A
#define LOCAL_DIMMING_MSPI_DC_TRW (0x00)

static MSPI_CH eChannel = 0;
static int device_num = 0;
static unsigned int MspiClk = 0;
static int isLdmLedDeviceInitialized = 0;
static unsigned char u8TailZero[26] = {0};

//check whether LDM support or not from boot env.
static inline int _isSupportedLDM(void)
{
    return (strstr(saved_command_line, "support_ldm=1") != NULL);
}

//get LDM led device name from boot env.
static inline int _getSupportedLdmLedDeivceType(void)
{
    if(strstr(saved_command_line, "ldm_led_device_name=ape5030") != NULL)
        return LDM_LED_DEVICE_TYPE_APE5030;
    else
        return LDM_LED_DEVICE_TYPE_NONE;
}

static void _init_ldm_led_device(void)
{
    MSPI_config stDrvLdMspiInfo = {};
    int i = 0;
    unsigned char ape5030_dev_num = 0;
    char *ape5030_dev_num_arg = strstr(saved_command_line, APE5030_DEV_NUM_ARG);

    if(!isLdmLedDeviceInitialized)
    {
        if(_getSupportedLdmLedDeivceType() == LDM_LED_DEVICE_TYPE_APE5030)
        {
            stDrvLdMspiInfo.tMSPI_DCConfig.u8TrStart = LOCAL_DIMMING_MSPI_DC_TRSTART;
            stDrvLdMspiInfo.tMSPI_DCConfig.u8TrEnd = LOCAL_DIMMING_MSPI_DC_TREND;
            stDrvLdMspiInfo.tMSPI_DCConfig.u8TB = LOCAL_DIMMING_MSPI_DC_TB;
            stDrvLdMspiInfo.tMSPI_DCConfig.u8TRW = LOCAL_DIMMING_MSPI_DC_TRW;
            stDrvLdMspiInfo.eMSPIMode = 0;

            for(i = 0 ; i < 8 ; i++)
            {
                stDrvLdMspiInfo.tMSPI_FrameConfig.u8WBitConfig[i]= 0x07;
                stDrvLdMspiInfo.tMSPI_FrameConfig.u8RBitConfig[i]= 0x07;
            }

            /* get ape5030 device number from boot args. */
            if(ape5030_dev_num_arg)
            {
                sscanf(ape5030_dev_num_arg + strlen(APE5030_DEV_NUM_ARG), "%d", &ape5030_dev_num);
                device_num = ape5030_dev_num;
            }
            else
            {
                device_num = 6; //default.
            }

            MspiClk = 6000000;
            eChannel = E_MSPI1;

            MDrv_MSPI_Init(eChannel);
            MDrv_MSPI_DCConfig(eChannel ,&stDrvLdMspiInfo.tMSPI_DCConfig);
            MDrv_MSPI_SetMode(eChannel , stDrvLdMspiInfo.eMSPIMode);
            MDrv_MSPI_SetCLKByINI(eChannel, MspiClk);
            MDrv_MSPI_FRAMEConfig(eChannel, &stDrvLdMspiInfo.tMSPI_FrameConfig);
        }

        isLdmLedDeviceInitialized = 1;
    }
}

unsigned char MSPI_Read_APE5030_SingleData(MSPI_CH eChannel, unsigned char u8deviceidx, unsigned char u8RegisterAddr, unsigned char DEVICE_NUM)
{
    unsigned char buffer[3] = {0};
    unsigned char pu8InputdData = 0;
    buffer[0] = (unsigned char)(u8deviceidx | (LDM_APE5030_SINGLE_DEV<<7) | (LDM_APE5030_SINGLE_BYTE<<6));
    buffer[1] = (unsigned char)(u8RegisterAddr | (LDM_APE5030_READ_IDX << 7));
    buffer[2] = 0;

    MDrv_MSPI_SlaveEnable_Channel(eChannel,TRUE);
    MDrv_MSPI_Write(eChannel,buffer,2);
    MDrv_MSPI_Write(eChannel,u8TailZero,DEVICE_NUM);
    MDrv_MSPI_Read(eChannel,&pu8InputdData,1);
    MDrv_MSPI_SlaveEnable_Channel(eChannel,false);
    return pu8InputdData;
}

void MSPI_Write_SingleAPE5030_SingleData(MSPI_CH eChannel, unsigned char u8DeiveID, unsigned char u8RegisterAddr, unsigned char u8InputdData, unsigned char DEVICE_NUM)
{
    unsigned char buffer[3] = {0};
    // 1. Set device ID
    // Write same value to same addr for all device
    buffer[0] = (unsigned char)(u8DeiveID | (LDM_APE5030_SINGLE_DEV<<7) | (LDM_APE5030_SINGLE_BYTE<<6));

    // 2. Set addr and data
    buffer[1] = (unsigned char)(u8RegisterAddr | (LDM_APE5030_WRITE_IDX << 7));
    buffer[2] = u8InputdData;

    // 3. Send SPI command
    MDrv_MSPI_SlaveEnable_Channel(eChannel,TRUE);
    MDrv_MSPI_Write(eChannel,buffer,3);
    if((DEVICE_NUM-1) > 0)
    {
        MDrv_MSPI_Write(eChannel,u8TailZero,DEVICE_NUM-1);
    }

    MDrv_MSPI_SlaveEnable_Channel(eChannel,false);
}

static u8 get_ldm_led_device_backlight(void)
{
    int i = 0;
    unsigned char isOn = 0;
    static unsigned char isDoneForFirstRead = false;

    if(!isLdmLedDeviceInitialized)
        _init_ldm_led_device();

    if(_getSupportedLdmLedDeivceType() == LDM_LED_DEVICE_TYPE_APE5030)
    {
        if(isDoneForFirstRead == false) // only first time needs to get the actual status from led device.
        {
            /* read all Led channels from APE5030.  */
            for(i = 0 ; i < device_num ; i++)
            {
                /* read reg CUR_ON_1 and CUR_ON_2 from APE5030, if found any LED channel turning on, will report backlight as 1. */
                if(MSPI_Read_APE5030_SingleData(eChannel, i, 0x01, device_num) || (MSPI_Read_APE5030_SingleData(eChannel, i, 0x02, device_num)))
                {
                    isOn = 1;
                    break;
                }
            }
            led_device_backlight_status = isOn;
            isDoneForFirstRead = true;
        }
        else
        {
            isOn = led_device_backlight_status;
        }
    }

    return isOn;
}

static void set_ldm_led_device_backlight(unsigned char on)
{
    int i = 0;

    if(!isLdmLedDeviceInitialized)
        _init_ldm_led_device();

    if(on)
    {
        if(_getSupportedLdmLedDeivceType() == LDM_LED_DEVICE_TYPE_APE5030)
        {
            unsigned char ape5030_ch_map[12] = {0};
            char *ape5030_ch_map_arg = strstr(saved_command_line, APE5030_CH_MAP_ARG);

            /* get all channels of APE5030 from boot args. */
            if(ape5030_ch_map_arg)
            {
                sscanf(ape5030_ch_map_arg + strlen(APE5030_CH_MAP_ARG), "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x", &ape5030_ch_map[0], &ape5030_ch_map[1], &ape5030_ch_map[2], \
                                                                                                                  &ape5030_ch_map[3], &ape5030_ch_map[4], &ape5030_ch_map[5], \
                                                                                                                  &ape5030_ch_map[6], &ape5030_ch_map[7], &ape5030_ch_map[8], \
                                                                                                                  &ape5030_ch_map[9], &ape5030_ch_map[10], &ape5030_ch_map[11]);
            }

            //CUR_ON_1
            for(i = 0 ; i < device_num ; i++)
            {
                MSPI_Write_SingleAPE5030_SingleData(eChannel, (i+1), 0x01, ape5030_ch_map[i], device_num);
            }

            //CUR_ON_2
            for(i = 0 ; i < device_num ; i++)
            {
                MSPI_Write_SingleAPE5030_SingleData(eChannel, (i+1), 0x02, ape5030_ch_map[i + 6], device_num);
            }

            led_device_backlight_status = true;
        }
    }
    else
    {
        if(_getSupportedLdmLedDeivceType() == LDM_LED_DEVICE_TYPE_APE5030)
        {
            /* disable all channels of APE5030.  */

            //CUR_ON_1
            for(i = 0 ; i < device_num ; i++)
            {
                MSPI_Write_SingleAPE5030_SingleData(eChannel, (i+1), 0x01, 0x00, device_num);
            }

            //CUR_ON_2
            for(i = 0 ; i < device_num ; i++)
            {
                MSPI_Write_SingleAPE5030_SingleData(eChannel, (i+1), 0x02, 0x00, device_num);
            }

            led_device_backlight_status = false;
        }
    }
}

static void set_backlight(unsigned char on)
{
    if (on)
    {
        /* For Local dimming without MCU case, need to init LED driver IC by SOC. */
        if(_isSupportedLDM() && _getSupportedLdmLedDeivceType() == LDM_LED_DEVICE_TYPE_APE5030)
        {
            pr_info("Turn on backlight of Ldm Led device APE5030 from kernel \n");
            set_ldm_led_device_backlight(1);
        }
        else
        {
            pr_info("Turn on backlight from kernel \n");

            if (backlight_inverse) {
                MDrv_GPIO_Set_Low(backlight_gpio);
            } else {
                if (strstr(idme_get_config_name(), "ABC") != NULL) {
#ifdef CONFIG_MSTAR_PWM
                    pr_info("PWM Duty status = %d\n", MDrv_PWM_Shift(E_PWM_CH0, 0xc0000));
#endif
                }
                MDrv_GPIO_Set_High(backlight_gpio);
            }
        }

        /* Record the backlight status in dummy register, which may be used in bootloader */
        REG_W2B(PM_ADDR_OFFSET, REG_RR(PM_ADDR_OFFSET) | PM_SPARE_SCREEN_STATE);

        backlight_status = 1;
    }
    else
    {
        /* For Local dimming without MCU case, need to init LED driver IC by SOC. */
        if(_isSupportedLDM() && _getSupportedLdmLedDeivceType() == LDM_LED_DEVICE_TYPE_APE5030)
        {
            pr_info("Turn off backlight of Ldm Led device APE5030 from kernel \n");
            set_ldm_led_device_backlight(0);
        }
        else
        {
            pr_info("Turn off backlight from kernel \n");

            if (backlight_inverse) {
                MDrv_GPIO_Set_High(backlight_gpio);
            } else {
                if (strstr(idme_get_config_name(), "ABC") != NULL) {
#ifdef CONFIG_MSTAR_PWM
                    pr_info("PWM Duty status = %d\n", MDrv_PWM_Shift(E_PWM_CH0, 0x80000));
#endif
                }
                MDrv_GPIO_Set_Low(backlight_gpio);
            }
        }

        /* Record the backlight status in dummy register */
        REG_W2B(PM_ADDR_OFFSET, REG_RR(PM_ADDR_OFFSET) & ~PM_SPARE_SCREEN_STATE);
        backlight_status = 0;
    }
}

static u32 get_backlight(void)
{
    if(_isSupportedLDM() && _getSupportedLdmLedDeivceType() == LDM_LED_DEVICE_TYPE_APE5030)
    {
        if (backlight_inverse) {
            pr_info("inverse backlight \n");
            return (!MDrv_GPIO_Pad_Read(backlight_gpio) && get_ldm_led_device_backlight());
        } else {
            pr_info("Do not inverse backlight \n");
            return (MDrv_GPIO_Pad_Read(backlight_gpio) && get_ldm_led_device_backlight());
        }
    }
    else
    {
        if (backlight_inverse) {
            pr_info("inverse backlight \n");
            return !MDrv_GPIO_Pad_Read(backlight_gpio);
        } else {
            pr_info("Do not inverse backlight \n");
            return MDrv_GPIO_Pad_Read(backlight_gpio);
        }
    }
}

/*
 * This fucntion will toggle the backlight when user pressing IR power key before boot completed,
 * and it will turn on the backlight when user press "Netflix", "Amazon Video" and "Amazon Music"
 * keys.
 */
unsigned int toggle_backlight(unsigned int keycode)
{
	if (unlikely(!boot_complete)) {
		pr_info("toggle_backlight keycode 0x%x\n", keycode);
		mutex_lock(&backlight_lock);
		if (keycode == KEY_POWER) {
			set_backlight(!backlight_status);
		} else if ((keycode == KEY_HOME) ||
				(keycode == KEY_APP1) ||
				(keycode == KEY_APP2) ||
				(keycode == KEY_APP3) ||
				(keycode == KEY_APP4) ||
				(keycode == KEY_BUTTON1) ||
				(keycode == KEY_BUTTON2) ||
				(keycode == KEY_WAKEUP)) {
			if (!backlight_status)
				set_backlight(1);
		} else if (keycode == KEY_SLEEP) {
			if (backlight_status)
				set_backlight(0);
		} else {
			pr_info("toggle_backlight ignore\n");
		}
		mutex_unlock(&backlight_lock);
	}
	return 0;
}

static int backlight_status_proc_show(struct seq_file *m, void *v)
{
	backlight_status = get_backlight();
	seq_printf(m, "%d\n", backlight_status);
	return 0;
}

/* This fucntion will be called when user pressing the power key on TV keypad */
ssize_t backlight_status_proc_write(struct file *file, const char __user *userbuf, size_t count, loff_t *ppos)
{
	char buf[64];
	unsigned int set;
	count = min_t(size_t, count, (sizeof(buf)-1));
	if (copy_from_user(buf, userbuf, count))
		return -EFAULT;

	buf[count] = '\0';

	if (strict_strtol(buf, 0, &set) != 0) {
		pr_err("value is not correct for backlight_status_proc_write ,buf %s\n", buf);
		return -EINVAL;
	}

	pr_info("backlight_status_proc_write set %d \n", set);
	set_backlight(set);
	return -1;
}

static int backlight_status_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, backlight_status_proc_show, NULL);
}

static const struct file_operations backlight_status_proc_fops = {
	.open		= backlight_status_proc_open,
	.read		= seq_read,
	.write		= backlight_status_proc_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/* This fucntion will be called when checking if system is in silent OTA mode */
static int silent_ota_status_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", mstar_get_silent_ota_flag());
	return 0;
}

static int silent_ota_status_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, silent_ota_status_proc_show, NULL);
}

static const struct file_operations silent_ota_status_proc_fops = {
	.open           = silent_ota_status_proc_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int boot_complete_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", boot_complete);
	return 0;
}

static int boot_complete_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, boot_complete_proc_show, NULL);
}

/* This functioni will be called init.maxim.rc when boot complete */
ssize_t boot_complete_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char buffer[2];

	if (count != 1)
		return count;

	if (copy_from_user(buffer, buf, count))
		return -EFAULT;

	buffer[1] = '\0';
	pr_info("write boot_completed  %s %d \n", buffer, buffer[0]);
	sscanf(buffer, "%d", &boot_complete);
	if (boot_complete == 1)
		mstar_clear_silent_ota_flag();

	return count;
}

static const struct file_operations boot_complete_fops = {
	.open = boot_complete_proc_open,
	.read = seq_read,
	.write = boot_complete_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
	.owner = THIS_MODULE,
};

static int __init ota_backlight_init(void)
{
	int ret = 0;
	struct device_node *np;
	u32 prop;
	char * config_name;
	char buffer[DTS_STRING_LENGTH];

#ifdef CONFIG_MSTAR_PWM
	pr_info("PWM init status = %d\n", MDrv_PWM_Init(E_PWM_DBGLV_NONE));
#endif

	/* Get backlight configuration from device tree */
	np = of_find_node_by_name(NULL, "backlight");
	if (np == NULL) {
		pr_err("silentota: backlight is not defined in dts \n");
		goto err;
	}

	/* get backlight gpio */
	/* try using bl_ctrl_$(full_config_name) */
	snprintf((char *)buffer, DTS_STRING_LENGTH, "%s%s", DTS_BACKLIGHT_GPIO_PREFIX, idme_get_config_name());
	ret = of_property_read_u32(np, buffer, &prop);
	if (ret) {
		/* try using bl_ctrl_$(prefix_of_config_name) */
		snprintf((char *)buffer, DTS_STRING_LENGTH, "%s%s", DTS_BACKLIGHT_GPIO_PREFIX, idme_get_config_name());
		char *hw_build_id = memchr((buffer + sizeof(DTS_BACKLIGHT_GPIO_PREFIX)), '_', (sizeof(buffer) - sizeof(DTS_BACKLIGHT_GPIO_PREFIX)));
		if (hw_build_id) {
			/* Remove hw_specific string */
			*hw_build_id = '\0';
		}
		ret = of_property_read_u32(np, buffer, &prop);
	}

	if (ret) {
		/* try using bl_ctrl_$(product_name) */
		snprintf((char *)buffer, DTS_STRING_LENGTH, "%s%s", DTS_BACKLIGHT_GPIO_PREFIX, idme_get_product_name());
		ret = of_property_read_u32(np, buffer, &prop);
	}

	if (ret) {
		/* try using default bl_ctrl */
		snprintf((char *)buffer, DTS_STRING_LENGTH, "%s", DTS_BACKLIGHT_GPIO_DEFAULT);
		ret = of_property_read_u32(np, buffer, &prop);
	}

	if (!ret) {
		backlight_gpio = prop;
		pr_info("silentota: backlight-gpio is %d (from dts:%s)\n", backlight_gpio, buffer);
	} else {
		pr_err("silentota: backlight-gpio is not defined \n");
		goto err;
	}

	/* get backlight invert */
	/* try using bl_ctrl_inverse_$(full_config_name) */
	snprintf((char *)buffer, DTS_STRING_LENGTH, "%s%s", DTS_BACKLIGHT_INVERT_PREFIX, idme_get_config_name());
	ret = of_property_read_u32(np, buffer, &prop);

	if (ret) {
		/* try using bl_ctrl_inverse_$(prefix_of_config_name) */
		snprintf((char *)buffer, DTS_STRING_LENGTH, "%s%s", DTS_BACKLIGHT_INVERT_PREFIX, idme_get_config_name());
		char *hw_build_id = memchr((buffer + sizeof(DTS_BACKLIGHT_INVERT_PREFIX)), '_', (sizeof(buffer) - sizeof(DTS_BACKLIGHT_INVERT_PREFIX)));
		if (hw_build_id) {
			/* Remove hw_specific string */
			*hw_build_id = '\0';
		}
		ret = of_property_read_u32(np, buffer, &prop);
	}

	if (ret) {
		/* try using bl_ctrl_inverse_$(product_name) */
		snprintf((char *)buffer, DTS_STRING_LENGTH, "%s%s", DTS_BACKLIGHT_INVERT_PREFIX, idme_get_product_name());
		ret = of_property_read_u32(np, buffer, &prop);
	}

	if (ret) {
		/* try using default bl_ctrl_inverse */
		snprintf((char *)buffer, DTS_STRING_LENGTH, "%s", DTS_BACKLIGHT_INVERT_DEFAULT);
		ret = of_property_read_u32(np, buffer, &prop);
	}

	if (!ret) {
		backlight_inverse = prop;
		pr_info("silentota: backlight-inverse is %d (from dts:%s)\n", backlight_inverse, buffer);
	} else {
		pr_err("silentota: backlight-inverse is not defined \n");
		goto err;
	}

	MDrv_GPIO_Init();
	/* detect current backlight status */
	backlight_status = get_backlight();
	pr_info("Inital backlight_status is %d \n", backlight_status);

	proc_create("boot_completed", 0664, NULL, &boot_complete_fops);
	proc_create("backlight_status", 0664, NULL, &backlight_status_proc_fops);
	proc_create("silent_ota_status", 0444, NULL, &silent_ota_status_proc_fops);
	return 0;
err:
	pr_err("silentota:Init failed\n");
	WARN_ON(1);
	return ret;
}

static void __exit ota_backlight_exit(void)
{
}

early_initcall(ota_backlight_init);
module_exit(ota_backlight_exit);

MODULE_AUTHOR("Amazon.");
MODULE_DESCRIPTION("OTA backlight driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ota-backlight");
