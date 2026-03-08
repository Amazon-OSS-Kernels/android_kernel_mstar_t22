/*
 *
 * (C) COPYRIGHT 2014-2017 MStar Semiconductor, Inc. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */

#include <mali_kbase.h>
#include <linux/delay.h>
#include "mali_kbase_platform_mstar.h"
#include "mali_kbase_platform_mstar_m7632_reg.h"

#ifdef CONFIG_DEBUG_FS
extern int mali_gpu_clock;
extern int mali_debug_property;

static void update_frequency(u32 frequency)
{
    mali_gpu_clock = frequency;
    /* set GPU clock */
    SET_REG(gpu_pll, ctrl1, (frequency/GPU_CLOCK_STEP));
    udelay(1);
}

static ssize_t gpu_clock_read(struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos)
{
    char buf[64];
    size_t r;
    r = snprintf(buf, 64, "%u\n", mali_gpu_clock);

    return simple_read_from_buffer(ubuf, cnt, ppos, buf, r);
}

static ssize_t gpu_clock_write(struct file *filp, const char __user *ubuf, size_t cnt, loff_t *ppos)
{
    int ret;
    char buffer[32];
    unsigned long val;

    if (cnt >= sizeof(buffer))
    {
        return -ENOMEM;
    }

    if (copy_from_user(&buffer[0], ubuf, cnt))
    {
        return -EFAULT;
    }

    buffer[cnt] = '\0';
    ret = kstrtoul(&buffer[0], 10, &val);

    if (0 != ret)
    {
        return -EINVAL;
    }

    if (mali_debug_property == MALI_DEBUG_PROPERTY_PASSWORD)
    {
        update_frequency(val);
    }

    return cnt;
}

static const struct file_operations kbasep_gpu_clock_debugfs_fops = {
    .read = gpu_clock_read,
    .write = gpu_clock_write,
};

/*
 *  Initialize debugfs entry for gpu clock
 */
void kbasep_gpu_clock_debugfs_init(struct kbase_device *kbdev)
{
    debugfs_create_file("gpu_clock", S_IRUGO,
            kbdev->mali_debugfs_directory, NULL,
            &kbasep_gpu_clock_debugfs_fops);
    return;
}

#else
/*
 * Stub functions for when debugfs is disabled
 */
void kbasep_gpu_clock_debugfs_init(struct kbase_device *kbdev)
{
    return;
}
#endif
