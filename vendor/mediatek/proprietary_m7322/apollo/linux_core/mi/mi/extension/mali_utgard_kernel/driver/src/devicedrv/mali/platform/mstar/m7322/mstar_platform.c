/*
 * Copyright (C) 2010-2018 MStar Semiconductor, Inc. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mstar_platform.c
 * MStar platform specific driver functions
 */
#include "mstar/mstar_platform.h"
#include <linux/delay.h>
#include "mali_kernel_common.h"

#include "mstar/mstar_chip.h"

/* REG_GPU_PLL */
#define REG_GPU_PLL_BASE            0x160f00
#define REG_GPUPLL_CTRL0            0x03
#define MSK_GPUPLL_PD               0x0002
#define REG_GPUPLL_CTRL1            0x04
#define MSK_GPU_CLOCK               0x00ff

#define REG_GPU_BASE                0x110800
#define REG_SOFT_RESET              0x00
#define MSK_SOFT_RESET              0x0001
#define MSK_SECURITY_SOFT_RESET     0x0002
#define REG_GPU_SRAM_SD_EN          0x43
#define MSK_GPU_SRAM_SD_EN          0x0008

#define REG_MFDEC_RREQ_RD_LEN         0x41
#define MSK_MFDEC_RREQ_RD_LEN         0x0007
#define VAL_MFDEC_RREQ_RD_LEN_08      0x0
#define VAL_MFDEC_RREQ_RD_LEN_16      0x1
#define VAL_MFDEC_RREQ_RD_LEN_32      0x2
#define VAL_MFDEC_RREQ_RD_LEN_40      0x3
#define VAL_MFDEC_RREQ_RD_LEN_48      0x4
#define VAL_MFDEC_RREQ_RD_LEN_56      0x5
#define VAL_MFDEC_RREQ_RD_LEN__48     0x6
#define VAL_MFDEC_RREQ_RD_LEN_54      0x7

#define REG_GPU_RREQ_RD_LEN         0x61
#define MSK_GPU_RREQ_RD_LEN         0x00e0
#define SHIFT_GPU_RREQ_RD_LEN       5
#define VAL_GPU_RREQ_RD_LEN_32      0x0
#define VAL_GPU_RREQ_RD_LEN_36      0x1
#define VAL_GPU_RREQ_RD_LEN_40      0x2
#define VAL_GPU_RREQ_RD_LEN_44      0x3
#define VAL_GPU_RREQ_RD_LEN_48      0x4
#define VAL_GPU_RREQ_RD_LEN_52      0x5
#define VAL_GPU_RREQ_RD_LEN_56      0x6
#define VAL_GPU_RREQ_RD_LEN_60      0x7

#define REG_RIU_APB_EN              0x6a
#define MSK_RIU_APB_EN              0x0001

#define REG_MIU0_START_ADDR         0x75
#define REG_MIU0_END_ADDR           0x76
#define REG_MIU1_START_ADDR         0x77
#define REG_MIU1_END_ADDR           0x78
#define MIU0_PHY_BASE_ADDR_HIGH     0x0000

#define REG_CLKGEN1_BASE            0x103300
#define REG_CKG_GPU                 0x20
#define DISABLE_CLK_GPU             0x0001

/* REG_MIU */
#define REG_MIU0_BASE               0x101200
#define REG_MIU1_BASE               0x100600
/* REG_MIU_LEN is set by MIU driver as is:
 * 0:for test, 1:2MB, 2:4MB, 3:8MB,
 * 4:16MB,  5:32MB,  6:64MB, 7:128MB,
 * 8:256MB, 9:512MB, A:1GB,  B:2GB
 */
#define REG_MIU_LEN                 0x69
#define MSK_MIU_LEN                 0xf000
#define REG_MIU1_LEN                0x3F
#define MSK_MIU1_LEN                0x00f0
#define SHIFT_MIU_LEN               12
#define SHIFT_MIU1_LEN              4

/* WROR */
#define REG_GPU_ROR_BASE            0x161000
#define REG_ROR_INORDER             0x01
#define MSK_ROR_INORDER             0x0001

/* RIU mapping */
#define GPU_PLL_REG(addr)           RIU[(REG_GPU_PLL_BASE) + ((addr) << 1)]
#define GPU_REG(addr)               RIU[REG_GPU_BASE + ((addr) << 1)]
#define CLKGEN1_REG(addr)           RIU[REG_CLKGEN1_BASE + ((addr) << 1)]
#define GPU_ROR_REG(addr)           RIU[REG_GPU_ROR_BASE + ((addr) << 1)]

#define MIU0_REG(addr)              RIU[(REG_MIU0_BASE) + ((addr) << 1)]
#define MIU1_REG(addr)              RIU[(REG_MIU1_BASE) + ((addr) << 1)]

#define GPU_CLOCK_STEP              24

/* helper functions */
static void power_on(void)
{
    /* GPU clock */
    GPU_PLL_REG(REG_GPUPLL_CTRL0) &= ~MSK_GPUPLL_PD;   /* set PLL PD to 0*/
    GPU_PLL_REG(REG_GPUPLL_CTRL1) = (GPU_PLL_REG(REG_GPUPLL_CTRL1)&~MSK_GPU_CLOCK) | ((GPU_MAX_CLOCK/GPU_CLOCK_STEP) & MSK_GPU_CLOCK); /* Set GPU Clock */
    udelay(100);

    /* Set MIU0 base and end address */
    GPU_REG(REG_MIU0_START_ADDR) = MIU0_PHY_BASE_ADDR_HIGH;
    GPU_REG(REG_MIU0_END_ADDR) = MIU0_PHY_BASE_ADDR_HIGH + (1 << (MIU0_REG(REG_MIU_LEN) >> SHIFT_MIU_LEN));
    /* Set MIU1 base and end address */
    GPU_REG(REG_MIU1_START_ADDR) = MIU1_PHY_BASE_ADDR_HIGH;
    GPU_REG(REG_MIU1_END_ADDR) = MIU1_PHY_BASE_ADDR_HIGH + (1 << ((MIU1_REG(REG_MIU1_LEN) & MSK_MIU1_LEN) >> SHIFT_MIU1_LEN));
    udelay(100);

    /* enable RIU access */
#ifdef MSTAR_RIU_ENABLED
    GPU_REG(REG_RIU_APB_EN) |= MSK_RIU_APB_EN;
    udelay(100);
#endif

    /* Set MFDEC read request length to 32 */
    GPU_REG(REG_MFDEC_RREQ_RD_LEN) &= ~MSK_MFDEC_RREQ_RD_LEN;
    GPU_REG(REG_MFDEC_RREQ_RD_LEN) |= VAL_MFDEC_RREQ_RD_LEN_32;

    /* Set read request length to 32 */
    GPU_REG(REG_GPU_RREQ_RD_LEN) &= ~MSK_GPU_RREQ_RD_LEN;
    GPU_REG(REG_GPU_RREQ_RD_LEN) |= (VAL_GPU_RREQ_RD_LEN_32 << SHIFT_GPU_RREQ_RD_LEN);

    /* Disable WROR */
    GPU_ROR_REG(REG_ROR_INORDER) |= MSK_ROR_INORDER;

    udelay(100);

    /* disable GPU clock gating */
    CLKGEN1_REG(REG_CKG_GPU) &= ~DISABLE_CLK_GPU;
    udelay(100);

    /* power up SRAM */
    GPU_REG(REG_GPU_SRAM_SD_EN) &= ~MSK_GPU_SRAM_SD_EN;
    udelay(100);

    /* reset mali */
    GPU_REG(REG_SOFT_RESET) &= ~MSK_SOFT_RESET;
    GPU_REG(REG_SOFT_RESET) |= MSK_SOFT_RESET;
    udelay(100); /*delay for run-time suspend*/
    GPU_REG(REG_SOFT_RESET) &= ~MSK_SOFT_RESET;
    udelay(100);
}

static void power_off(void)
{
    /* Pull up (enable) GPU soft reset */
    GPU_REG(REG_SOFT_RESET) |= MSK_SOFT_RESET;
    udelay(100);

    /* power down SRAM */
    GPU_REG(REG_GPU_SRAM_SD_EN) |= MSK_GPU_SRAM_SD_EN;
    udelay(100);

    /* enable GPU clock gating */
    CLKGEN1_REG(REG_CKG_GPU) |= DISABLE_CLK_GPU;
    udelay(100);
}

#if defined(CONFIG_MALI_DVFS)
static void dvfs_init(void)
{
    u32 i;

    mali_clock_items.num_of_steps = (GPU_MAX_CLOCK - GPU_MIN_CLOCK) / GPU_CLOCK_STEP + 1;
    mali_clock_items.item = _mali_osk_valloc(sizeof(struct mali_gpu_clk_item) * mali_clock_items.num_of_steps);

    if (NULL == mali_clock_items.item)
    {
        mali_clock_items.num_of_steps = 0;
        return;
    }

    for (i = 0; i < mali_clock_items.num_of_steps; ++i)
    {
        mali_clock_items.item[i].clock = GPU_MIN_CLOCK + GPU_CLOCK_STEP * i;
        mali_clock_items.item[i].vol = 0;
    }
}

static void dvfs_deinit(void)
{
    if (mali_clock_items.item)
    {
        _mali_osk_vfree(mali_clock_items.item);
        mali_clock_items.item = NULL;
        mali_clock_items.num_of_steps = 0;
    }
}
#endif

void adjust_frequency(int frequency)
{
    mali_gpu_clock = frequency;

    /* GPU clock */
    GPU_PLL_REG(REG_GPUPLL_CTRL0) &= ~MSK_GPUPLL_PD;   /* set PLL PD to 0*/
    GPU_PLL_REG(REG_GPUPLL_CTRL1) = (GPU_PLL_REG(REG_GPUPLL_CTRL1)&~MSK_GPU_CLOCK) | ((frequency/GPU_CLOCK_STEP) & MSK_GPU_CLOCK); /* Set GPU Clock */
    udelay(1);
}

/* platform functions */
void mstar_platform_init(void)
{
    mali_gpu_clock = GPU_MAX_CLOCK;
#ifdef CONFIG_MALI_DVFS
    dvfs_init();
#endif
    power_on();
}

void mstar_platform_deinit(void)
{
    power_off();
#ifdef CONFIG_MALI_DVFS
    dvfs_deinit();
#endif
}

void mstar_platform_suspend(void)
{
    power_off(); /* just power off */
}

void mstar_platform_resume(void)
{
    power_on(); /* just power on */
}

void mstar_platform_runtime_suspend(void)
{
    power_off();
}

void mstar_platform_runtime_resume(void)
{
    /* disable GPU clock gating */
    CLKGEN1_REG(REG_CKG_GPU) &= ~DISABLE_CLK_GPU;
    udelay(100);

    /* power up SRAM */
    GPU_REG(REG_GPU_SRAM_SD_EN) &= ~MSK_GPU_SRAM_SD_EN;
    udelay(100);

    /* Pull up (enable) GPU soft reset */
    GPU_REG(REG_SOFT_RESET) &= ~MSK_SOFT_RESET;
    udelay(100);
}

#if defined(MSTAR_DISABLE_GPU_CORES)
int mstar_platform_get_efuse_core_num(void)
{
    return 0;
}
#endif
