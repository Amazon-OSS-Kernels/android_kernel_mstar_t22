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
#include "mali_kbase_platform_mstar.h"

extern ulong mtk_miu0_length;
extern ulong mtk_miu1_length;
extern ulong mtk_miu2_length;

/* Platform functions */
int mstar_platform_init(struct kbase_device *kbdev)
{
    u32 miu0_length = 0;
    u32 miu1_length = 0;
    u32 miu2_length = 0;

    KBASE_DEBUG_ASSERT(NULL != kbdev);

    init_registers();

#ifdef CONFIG_MALI_MIDGARD_DVFS
    {
        int err = dvfs_init(kbdev);

        if (0 != err)
        {
            return err;
        }
    }
#endif

#ifdef MSTAR_DISABLE_SHADER_CORES
    kbdev->get_num_disabled_cores = get_num_disabled_cores;
#endif

#if defined(MSTAR_M7621) && defined(MSTAR_DEGLITCH_PATCH)
    kbdev->set_deglitch_mux_to_216mhz = set_deglitch_mux_to_216mhz;
#endif

    get_dram_length(&miu0_length, &miu1_length, &miu2_length);

    mtk_miu0_length = (ulong) miu0_length;
    mtk_miu1_length = (ulong) miu1_length;
    mtk_miu2_length = (ulong) miu2_length;

    return 0;
}

void mstar_platform_term(struct kbase_device *kbdev)
{
#ifdef CONFIG_MALI_MIDGARD_DVFS
    dvfs_term(kbdev);
#endif
}

/* PM callbacks */
void mstar_pm_off(struct kbase_device *kbdev)
{
    power_off();
}

int mstar_pm_on(struct kbase_device *kbdev)
{
    power_on();

    /* If the GPU state has been lost then this function must return 1, otherwise it should return 0. */
    return 1; /* TODO: check if the GPU state has been lost */
}

void mstar_pm_suspend(struct kbase_device *kbdev)
{
#if defined(MSTAR_M7821)
    /* Patch HW Bug: gpu software reset will cause FIFO points in disarray*/
    power_suspend();
    return;
#endif
    power_off();
}

void mstar_pm_resume(struct kbase_device *kbdev)
{
#if defined(MSTAR_M7821)
    /* Patch HW Bug: gpu software reset will cause FIFO points in disarray*/
    power_resume();
    return;
#endif
    init_registers();
    power_on();
}

#ifdef CONFIG_MALI_MIDGARD_DVFS

/* XXX: these two values are copied from mali_kbase_pm_metrics.c */
#define KBASE_PM_NO_VSYNC_MIN_UTILISATION 10
#define KBASE_PM_NO_VSYNC_MAX_UTILISATION 40

int kbase_platform_dvfs_event(struct kbase_device *kbdev,
                              u32 utilisation,
                              u32 util_gl_share,
                              u32 util_cl_share[2])
{
    KBASE_DEBUG_ASSERT(NULL != kbdev);

    /* implementation is copied from Midgard r6p0-02rel0 */
    if (kbdev->dvfs.enabled)
    {
        if (utilisation < KBASE_PM_NO_VSYNC_MIN_UTILISATION)
        {
            if (NULL != kbdev->dvfs.clock_down)
            {
                kbdev->dvfs.clock_down(kbdev);
            }
        }
        else if (utilisation > KBASE_PM_NO_VSYNC_MAX_UTILISATION)
        {
            if (NULL != kbdev->dvfs.clock_up)
            {
                kbdev->dvfs.clock_up(kbdev);
            }
        }
        else
        {
            /* nop */
        }
    }

    return 0;
}

#endif /* CONFIG_MALI_MIDGARD_DVFS */
