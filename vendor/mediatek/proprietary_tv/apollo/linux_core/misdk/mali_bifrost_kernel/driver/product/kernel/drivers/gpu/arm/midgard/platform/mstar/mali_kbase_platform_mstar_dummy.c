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

/* Platform functions */
void init_registers(void)
{

}

void power_on(void)
{

}

void power_off(void)
{

}

#ifdef MSTAR_DISABLE_SHADER_CORES
int get_num_disabled_cores(void)
{
    return 0;
}
#endif

void get_dram_length(u32* miu0_length, u32* miu1_length, u32* miu2_length)
{
#define DRAM_SIZE_REG_TO_MB(reg)    ((reg > 0) ? (1 << (reg)) : 0)      /* in MB */
#define DRAM_SIZE_REG_TO_B(reg)     (DRAM_SIZE_REG_TO_MB(reg) << 20)    /* in Byte */

    *miu0_length = 0;
    *miu1_length = 0;
    *miu2_length = 0;

    return;
}
