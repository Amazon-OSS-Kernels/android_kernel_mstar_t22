/*
 *  drivers/irblaster/irblaster_detection.c
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/kdev_t.h>
#include <linux/of.h>
#include <linux/kernel.h>

#ifdef CONFIG_AMAZON_METRICS_LOG
#include <linux/vmalloc.h>
#endif

#include "mdrv_gpio.h"
#include "mhal_gpio.h"
#include "mhal_gpio_reg.h"
#include "mdrv_mstypes.h"
#include "mdrv_mbx.h"

#define DEVICE_NAME		"irblaster"

extern char *idme_get_config_name(void);
#define DTS_STRING_LENGTH 64
#define DTS_IRBLASTER_ENABLE_DEFAULT "irblaster-enable"
#define DTS_IRBLASTER_ENABLE_PREFIX "irblaster-enable_"
#define DTS_IRBLASTER_PWM_DEFAULT "irblaster-pwm"
#define DTS_IRBLASTER_PWM_PREFIX "irblaster-pwm_"
#define PM_CMDIDX_IR_OUT_PWM_CHANNEL (0x81)
#define PM_MBX_TIMEOUT      (1500)

static struct task_struct *irblaster_poll_tsk;
static struct device *irblaster_dev;
static U8 irblaster_pwm = 0xFF;
struct class *irblaster_class;


enum IRBLASTER_STATUS {
	IRBLASTER_UNPLUGGED		= 0,
	IRBLASTER_PLUG_IN		= 1,
};


struct irblaster_detection_dev {
	struct device *dev;
	struct work_struct work;
	u32 irdetect_gpio;
	u32 irdetect_inverse;
	int ir_ctrl;
	int	state;
	bool is_suspend;
	int	pwm;
};


static struct platform_device irblaster_detection_device = {
	.name	= "irblaster-detection",
};

static int irblaster_SetupMbx(void)
{
	MBX_Result enMbxResult = E_MBX_UNKNOW_ERROR;

	if (E_MBX_SUCCESS != MDrv_MBX_Startup()) {
		pr_err("!! MDrv_MBX_Startup fail \n");
		return enMbxResult;
	}
	enMbxResult = MDrv_MBX_Init(E_MBX_CPU_MIPS, E_MBX_ROLE_HK, PM_MBX_TIMEOUT);
	if (E_MBX_SUCCESS != enMbxResult) {
		pr_err("!! MDrv_MBX_Init fail nMbxResult:%d \n", enMbxResult);
	} else {
		MDrv_MBX_Enable(TRUE);
	}
	return enMbxResult;
}
static int irblaster_SendParaMbx2PM(void)
{
	unsigned char count = 0;
	MBX_Msg stMbxCommand;
	MBX_Result enMbxResult = E_MBX_UNKNOW_ERROR;
	/*send msg to PM */
	memset((void *)&stMbxCommand, 0, sizeof(MBX_Msg));
	stMbxCommand.u8Index = PM_CMDIDX_IR_OUT_PWM_CHANNEL;
	stMbxCommand.eRoleID = E_MBX_ROLE_PM;               /* Do Not Change */
	stMbxCommand.eMsgType = E_MBX_MSG_TYPE_INSTANT;     /* Do Not Change */
	stMbxCommand.u8Ctrl = 0;                            /* Do Not Change */
	stMbxCommand.u8MsgClass = E_MBX_CLASS_PM_NOWAIT;    /* Do Not Change */
	stMbxCommand.u8ParameterCount = sizeof(irblaster_pwm);                  /* Implement by yourself Between ARM & PM */
	stMbxCommand.u8Parameters[0] = irblaster_pwm;

	printk("\033[45;37m  %s pwm=[%x]  \033[0m\n", __FUNCTION__, irblaster_pwm);
	enMbxResult = irblaster_SetupMbx();
	if (enMbxResult != E_MBX_SUCCESS) {
		pr_err("\033[45;37m  %s MailBox init fail err=%x \033[0m\n", __FUNCTION__, enMbxResult);
		return -1;
	}
	enMbxResult = MDrv_MBX_SendMsg(&stMbxCommand);
	if (enMbxResult != E_MBX_SUCCESS) {
		pr_err("\033[45;37m  %s MailBox send coomand fail enMbxResult=%x  \033[0m\n", __FUNCTION__, enMbxResult);
	}
}
int irblaster_SetPWMInfo (void)
{
	if (irblaster_pwm != 0xFF) {
		irblaster_SendParaMbx2PM();
	}
}
EXPORT_SYMBOL(irblaster_SetPWMInfo);

static ssize_t show_plug(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct irblaster_detection_dev *irdev = dev_get_drvdata(dev);
	sprintf(buf, "%d\n", irdev->state);
	return strlen(buf);
}
static DEVICE_ATTR(plug, S_IRUGO, show_plug, NULL);

static ssize_t enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct irblaster_detection_dev *irdev = dev_get_drvdata(dev);
	if (irdev->ir_ctrl == -1) {
		sprintf(buf, "%d\n", -1);
	} else {
		sprintf(buf, "%d\n", MDrv_GPIO_Pad_Read(irdev->ir_ctrl));
	}
	return strlen(buf);
}

static ssize_t enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t len)
{
	int ctrl;
	struct irblaster_detection_dev *irdev = dev_get_drvdata(dev);

	if (irdev->ir_ctrl == -1) {
		return -1;
	}
	kstrtoint(buf, 10, &ctrl);
	if (ctrl == 1) {
		MDrv_GPIO_Set_High(irdev->ir_ctrl);
	} else {
		MDrv_GPIO_Set_Low(irdev->ir_ctrl);
	}

	return -1;
}
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP, enable_show, enable_store);

static void set_state(struct irblaster_detection_dev *dev, int state)
{
	char event_string[10];
	char *envp[] = { event_string, NULL };

	if (dev->state != state) {
		dev->state = state;
		snprintf(event_string, sizeof(event_string), "plug=%d", state);
		pr_info("irblaster: generate IR detect uevent %s\n", envp[0]);
		kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE, envp);
	}
}


static int irblaster_poll(void *arg)
{
	struct irblaster_detection_dev *dev =
		(struct irblaster_detection_dev *)arg;
	int state = 0;

	while (1) {
		schedule_timeout_interruptible(msecs_to_jiffies(100));
		if (!dev->is_suspend) {
			if (dev->irdetect_inverse == 1)
				state = !(MDrv_GPIO_Pad_Read(dev->irdetect_gpio));
			else
				state = (MDrv_GPIO_Pad_Read(dev->irdetect_gpio));
			set_state(dev, state);
		}
	}
	return 0;
}

static int irblaster_detection_probe(struct platform_device *pdev)
{
	struct irblaster_detection_dev *dev;
	struct device_node *np;
	u32 prop;
	int ret = 0;
	char buffer_suffix_fullconfigname[DTS_STRING_LENGTH]; //irblaster-enable_XXXX_XXX
	char buffer_suffix_partconfigname[DTS_STRING_LENGTH]; //irblaster-enable_XXXX
	char buffer_default[DTS_STRING_LENGTH]; //irblaster-enable
	char project_name[DTS_STRING_LENGTH];

	//1. create pattern with irblaster-enable_$(full_config_name)
	snprintf((char *)buffer_suffix_fullconfigname, DTS_STRING_LENGTH, "%s%s", DTS_IRBLASTER_ENABLE_PREFIX, idme_get_config_name());

	//2. create pattern with irblaster-enable_$(partial_config_name)
	snprintf((char *)buffer_suffix_partconfigname, DTS_STRING_LENGTH, "%s%s", DTS_IRBLASTER_ENABLE_PREFIX, idme_get_config_name());
	char *hw_build_id = memchr((buffer_suffix_partconfigname + sizeof(DTS_IRBLASTER_ENABLE_PREFIX)), '_', (sizeof(buffer_suffix_partconfigname) - sizeof(DTS_IRBLASTER_ENABLE_PREFIX)));
	if (hw_build_id)
	{
		//Remove hw_specific string
		*hw_build_id = '\0';
	}

	//3. create pattern with irblaster-enable
	snprintf((char *)buffer_default, DTS_STRING_LENGTH, "%s", DTS_IRBLASTER_ENABLE_DEFAULT);

	dev = kzalloc(sizeof(struct irblaster_detection_dev), GFP_KERNEL);
	if (!dev) {
		pr_err("irblaster: irblaster_detection_dev allocation fail");
		return -ENOMEM;
	}

	np = of_find_node_by_name(NULL, "irblaster_gpio");
	if (np == NULL) {
		pr_err("irblaster: irblaster_detection is not defined in dts\n");
		goto err_class_create;
	}

	//When the first one success of_property_read_u32,then loop will break and get the value with the following priority.
	if( of_property_read_u32(np, buffer_suffix_fullconfigname, &prop) && \
	    of_property_read_u32(np, buffer_suffix_partconfigname, &prop) && \
	    of_property_read_u32(np, buffer_default, &prop))
	{
		pr_err("irblaster: can't find irblaster_enable [%s][%d] full=%s partial=%s default=%s\n", __func__, __LINE__, buffer_suffix_fullconfigname, buffer_suffix_partconfigname, buffer_default);
		pr_err("irblaster: ir blaster is not supported on this product \n");
		goto err_class_create;
	}
	if (prop == 0) {
		pr_info("ir blaster is not supported \n");
		goto err_class_create;
	}

	memset (buffer_suffix_fullconfigname, 0, sizeof(buffer_suffix_fullconfigname));
	memset (buffer_suffix_partconfigname, 0, sizeof(buffer_suffix_partconfigname));
	memset (buffer_default, 0, sizeof(buffer_default));

	snprintf((char *)buffer_suffix_fullconfigname, DTS_STRING_LENGTH, "%s%s", DTS_IRBLASTER_PWM_PREFIX, idme_get_config_name());
	snprintf((char *)buffer_suffix_partconfigname, DTS_STRING_LENGTH, "%s%s", DTS_IRBLASTER_PWM_PREFIX, idme_get_config_name());

	hw_build_id = memchr((buffer_suffix_partconfigname + sizeof(DTS_IRBLASTER_PWM_PREFIX)), '_', (sizeof(buffer_suffix_partconfigname) - sizeof(DTS_IRBLASTER_PWM_PREFIX)));
	if (hw_build_id) {
		/*Remove hw_specific string*/
		*hw_build_id = '\0';
	}

	snprintf((char *)buffer_default, DTS_STRING_LENGTH, "%s", DTS_IRBLASTER_PWM_DEFAULT);

	if (of_property_read_u32(np, buffer_suffix_fullconfigname, &prop) && \
		of_property_read_u32(np, buffer_suffix_partconfigname, &prop) && \
		of_property_read_u32(np, buffer_default, &prop)) {
		pr_err("irblaster: can't find irblaster_pwm [%s][%d] full=%s partial=%s default=%s\n", __func__, __LINE__, buffer_suffix_fullconfigname, buffer_suffix_partconfigname, buffer_default);
		pr_err("irblaster: ir blaster is not supported on this product \n");
		goto err_class_create;
	} else {
		pr_info("irblaster: irblaster_pwm is %d \n", prop);
		dev->pwm = prop;
		irblaster_pwm = (U8)prop;
	}

	snprintf((char *)project_name, DTS_STRING_LENGTH, "%s", idme_get_config_name());
	hw_build_id = memchr(project_name, '_', sizeof(project_name));
	if (hw_build_id) {
		/*Remove hw_specific string*/
		*hw_build_id = '\0';
	}

	/*specific PROJECT irblaster-gpio is prior*/
	snprintf((char *)buffer_default, DTS_STRING_LENGTH, "%s%s", "irblaster-gpio_", project_name);
	if (!of_property_read_u32(np, buffer_default, &prop)) {
		dev->irdetect_gpio = prop;
		pr_info("irblaster: irblaster_detection %s %d \n", buffer_default, prop);
	}
	else if (!of_property_read_u32(np, "irblaster-gpio", &prop)) {
		dev->irdetect_gpio = prop;
		pr_info("irblaster: irblaster_detection gpio is %d \n", prop);
	} else {
		pr_err("irblaster: irblaster_detection gpio is not defined \n");
		goto err_class_create;
	}

	/*specific PROJECT irblaster-inverse is prior*/
	snprintf((char *)buffer_default, DTS_STRING_LENGTH, "%s%s", "irblaster-inverse_", project_name);
	if (!of_property_read_u32(np, buffer_default, &prop)) {
		dev->irdetect_inverse = prop;
		pr_info("irblaster: irblaster_detection %s %d \n", buffer_default, prop);
	}
	else if (!of_property_read_u32(np, "irblaster-inverse", &prop)) {
		dev->irdetect_inverse = prop;
		pr_info("irblaster: irblaster_detection gpio inverse is %d \n", prop);
	} else {
		pr_err("irblaster: irblaster_detection gpio inverse is not defined \n");
		goto err_class_create;
	}

	/*specific PROJECT irblaster-ctrl is prior*/
	snprintf((char *)buffer_default, DTS_STRING_LENGTH, "%s%s", "irblaster-ctrl_", project_name);
	if (!of_property_read_u32(np, buffer_default, &prop)) {
		dev->ir_ctrl = prop;
		pr_info("irblaster: irblaster %s %d \n", buffer_default, prop);
	}
	else if (!of_property_read_u32(np, "irblaster-ctrl", &prop)) {
		dev->ir_ctrl = prop;
		pr_info("irblaster: irblaster control gpio  is %d \n", prop);
	} else {
		pr_err("irblaster: irblaster control  gpio is not defined \n");
		dev->ir_ctrl = -1;
	}


	platform_set_drvdata(pdev, dev);
	MDrv_GPIO_Init();

	irblaster_poll_tsk = kthread_create(irblaster_poll, dev, "Irblaster poll Task");
	if (IS_ERR(irblaster_poll_tsk)) {
		pr_err("irblaster: create kthread for GPIO poll Task fail\n");
		ret = PTR_ERR(irblaster_poll_tsk);
		goto err_kthread_create;
	} else
		wake_up_process(irblaster_poll_tsk);

	irblaster_class = class_create(THIS_MODULE, DEVICE_NAME);
	if (IS_ERR(irblaster_class)) {
		pr_err("irblaster: create class fail\n");
		ret = PTR_ERR(irblaster_class);
		goto err_class_create;
	}
	irblaster_dev = device_create(irblaster_class, NULL,
			MKDEV(0, 0), dev, "irblaster%d", 1);
	dev->dev = irblaster_dev;

	ret = device_create_file(irblaster_dev, &dev_attr_plug);
	if (ret < 0)
		goto err_create_file;

	ret = device_create_file(irblaster_dev, &dev_attr_enable);
	if (ret < 0)
		goto err_create_file;

	irblaster_SendParaMbx2PM();

	if (dev->irdetect_inverse == 1)
		set_state(dev, !MDrv_GPIO_Pad_Read(dev->irdetect_gpio));
	else
		set_state(dev, MDrv_GPIO_Pad_Read(dev->irdetect_gpio));
	return 0;

err_create_file:
	device_destroy(irblaster_class, MKDEV(0, 0));
	class_destroy(irblaster_class);
err_class_create:
err_kthread_create:
	kfree(dev);
	return ret;
}

static int irblaster_detection_remove(struct platform_device *pdev)
{

	device_remove_file(irblaster_dev, &dev_attr_plug);
	device_remove_file(irblaster_dev, &dev_attr_enable);
	device_destroy(irblaster_class, MKDEV(0, 0));
	class_destroy(irblaster_class);
	return 0;
}

static int irblaster_detection_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct irblaster_detection_dev *dev = platform_get_drvdata(pdev);
	if (dev)
		dev->is_suspend = true;
	return 0;
}

static int irblaster_detection_resume(struct platform_device *pdev)
{
	struct irblaster_detection_dev *dev = platform_get_drvdata(pdev);

	if (dev)
		dev->is_suspend = false;
	return 0;
}

static struct platform_driver irblaster_detection_driver = {
	.probe		= irblaster_detection_probe,
	.remove		= irblaster_detection_remove,
	.suspend	= irblaster_detection_suspend,
	.resume		= irblaster_detection_resume,
	.driver		= {
		.name	= "irblaster-detection",
		.owner	= THIS_MODULE,
	},
};

static int __init irblaster_detection_init(void)
{
	int ret;
	ret = platform_device_register(&irblaster_detection_device);
	if (ret < 0) {
		pr_err("failed to register irblaster device\n");
		return ret;
	}
	ret = platform_driver_register(&irblaster_detection_driver);
	if (ret < 0) {
		pr_err("failed to register irblaster driver\n");
		platform_device_unregister(&irblaster_detection_device);
	}
	return ret;
}

static void __exit irblaster_detection_exit(void)
{
	platform_driver_unregister(&irblaster_detection_driver);
	platform_device_unregister(&irblaster_detection_device);
}

module_init(irblaster_detection_init);
module_exit(irblaster_detection_exit);

MODULE_DESCRIPTION("IRblaster detection");
MODULE_LICENSE("GPL");

