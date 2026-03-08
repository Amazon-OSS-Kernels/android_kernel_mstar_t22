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
#include "mali_kbase_platform_mstar_m7632_reg.h"

#include <linux/delay.h>
#include "mstar/mstar_chip.h"

#define GPU_CLOCK_STEP              12

#if ENABLE_GPU_MFDEC
#include <mtk_dtv_mfdec_v1.h>
#endif

void setDisableRrorWror(void)
{

    //disable rror
    SET_REG(rror, ror_inorder, 0x1);

    //disable wror
    SET_REG(wror, ror_inorder, 0x1);

    return;
}

/* Platform functions */
void init_registers(void)
{
#define MIU_BASE_TO_MB(base)        ((base) >> 20)                   /* in MB */
#define DRAM_SIZE_REG_TO_MB(reg)    ((reg > 0) ? (1 << (reg)) : 0)   /* in MB */

    /* set GPU clock */

    SET_REG(gpu_pll, ctrl0, 0x0);
    SET_REG(gpu_pll, ctrl1, (MALI_MAX_FREQ/GPU_CLOCK_STEP));
    udelay(10);

    /* default is enable, disable for zebu test */
    //setDisableRrorWror();

    /* set MIU addr in asic*/
    SET_REG(gpu, miu0_start, MIU_BASE_TO_MB(MSTAR_MIU0_BUS_BASE));
    SET_REG(gpu, miu0_end, MIU_BASE_TO_MB(MSTAR_MIU0_BUS_BASE) + DRAM_SIZE_REG_TO_MB(GET_REG(miu0, dram_size)));
#if defined(MSTAR_MIU1_BUS_BASE) && defined(ARM_MIU1_BASE_ADDR)
    SET_REG(gpu, miu1_start, MIU_BASE_TO_MB(MSTAR_MIU1_BUS_BASE));
    SET_REG(gpu, miu1_end, MIU_BASE_TO_MB(MSTAR_MIU1_BUS_BASE) + DRAM_SIZE_REG_TO_MB(GET_REG(miu1, dram_size)));
#else
    SET_REG(gpu, miu1_start, 0xffff);
    SET_REG(gpu, miu1_end, 0xffff);
#endif

    /* enable RIU */
#ifdef MSTAR_RIU_ENABLED
    SET_REG(gpu, riu, 1);
#endif

    /* set read request length 0 to 64 */
    SET_REG(gpu, rreq_rd_len0, 0x4);
    /* set read request length 0 to 48 */
    SET_REG(gpu, rreq_rd_len1, 0x1);

    /* enable dynamic clock gating for gpu bridge */
    SET_REG(gpu, dynamic_clk_gating_en, 1);

    /* enable GPU clock */
    SET_REG(clkgen1, ckg_gpu, 0);
    udelay(1);

    SET_REG(gpu, soft_reset, 0);    /* disable MALI soft reset */
    SET_REG(gpu, soft_reset, 1);    /* enable MALI soft reset */
    SET_REG(gpu, soft_reset, 0);    /* disable MALI soft reset */
    udelay(1);

    /* NOTICE!!!!! WROR and RROR trigger must be set after bridge register is init or reset!!!! */
    /* trigger WROR hw arb update */
    SET_REG(rror, miu_hw_arb_update, 0x0);
    SET_REG(rror, miu_hw_arb_update, 0x1);
    /* trigger RROR hw arb update */
    SET_REG(wror, miu_hw_arb_update, 0x0);
    SET_REG(wror, miu_hw_arb_update, 0x1);

}

void power_on(void)
{
    SET_REG(mpu, mpu_enable, 0);    /* disbale mpu */
    SET_REG(clkgen1, ckg_gpu, 0);   /* enable GPU clock */
    SET_REG(gpu, sram_sd_en, 0);    /* power on SRAM */

    /* PULL DOWN GPU RESET */
    SET_REG(gpu, soft_reset, 0);
    udelay(1);

    /* NOTICE!!!!! WROR and RROR trigger must be set after bridge register is init or reset!!!! */
    /* trigger WROR hw arb update */
    SET_REG(rror, miu_hw_arb_update, 0x0);
    SET_REG(rror, miu_hw_arb_update, 0x1);
    /* trigger RROR hw arb update */
    SET_REG(wror, miu_hw_arb_update, 0x0);
    SET_REG(wror, miu_hw_arb_update, 0x1);

#if ENABLE_GPU_MFDEC
    init_mfdec();
#endif
}

void power_off(void)
{
    SET_REG(gpu, soft_reset, 1);    /* reset GPU */
    SET_REG(gpu, sram_sd_en, 1);    /* power off SRAM */
    SET_REG(clkgen1, ckg_gpu, 1);   /* disable GPU clock */
    udelay(1);
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

    *miu0_length = DRAM_SIZE_REG_TO_MB(GET_REG(miu0, dram_size));
#if defined(MSTAR_MIU1_BUS_BASE) && defined(ARM_MIU1_BASE_ADDR)
    *miu1_length = DRAM_SIZE_REG_TO_MB(GET_REG(miu1, dram_size));
#else
    *miu1_length = 0;
#endif
    *miu2_length = 0;

    return;
}
