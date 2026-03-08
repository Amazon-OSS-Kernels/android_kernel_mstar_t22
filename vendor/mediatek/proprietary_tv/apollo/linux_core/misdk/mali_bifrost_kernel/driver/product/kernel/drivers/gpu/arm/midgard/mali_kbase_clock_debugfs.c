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

#ifdef CONFIG_DEBUG_FS

int mali_debug_property = 0;

static void update_mali_debug_property(u32 property)
{
    if(property == MALI_DEBUG_PROPERTY_PASSWORD || property == 0)
    {
        mali_debug_property = property;
    }

    return;
}

static ssize_t gpu_property_read(struct file *filp, char __user *ubuf, size_t cnt, loff_t *ppos)
{
    char buf[64];
    size_t r;
    r = snprintf(buf, 64, "%u\n", mali_debug_property);

    return simple_read_from_buffer(ubuf, cnt, ppos, buf, r);
}

static ssize_t gpu_property_write(struct file *filp, const char __user *ubuf, size_t cnt, loff_t *ppos)
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

    update_mali_debug_property(val);
    return cnt;
}

static const struct file_operations kbasep_mali_debug_debugfs_fops = {
    .read = gpu_property_read,
    .write = gpu_property_write,
};

/*
 *  Initialize debugfs entry for mali_debug
 */
void kbasep_mali_debug_debugfs_init(struct kbase_device *kbdev)
{
    debugfs_create_file("mali_debug_level", S_IRUGO,
            kbdev->mali_debugfs_directory, NULL,
            &kbasep_mali_debug_debugfs_fops);
    return;
}

#else
/*
 * Stub functions for when debugfs is disabled
 */
void kbasep_mali_debug_debugfs_init(struct kbase_device *kbdev)
{
    return;
}
#endif
