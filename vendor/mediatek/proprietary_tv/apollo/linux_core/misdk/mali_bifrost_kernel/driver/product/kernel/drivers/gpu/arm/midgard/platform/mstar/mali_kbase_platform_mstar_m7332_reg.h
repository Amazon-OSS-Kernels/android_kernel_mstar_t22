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

#include "mali_kbase_platform_mstar.h"

/* Registers */
DEFINE_REG_BANK(gpu,        0x110800);
DEFINE_REG_BANK(gpu_pll,    0x160f00);
DEFINE_REG_BANK(clkgen1,    0x103300);
DEFINE_REG_BANK(mpu,         0x313800);

DEFINE_REG_BANK(miu0,       0x101200);
DEFINE_REG_BANK(miu1,       0x100600);

//bank for WROR and RROR
DEFINE_REG_BANK(rror,       0x161200);
DEFINE_REG_BANK(wror,       0x161000);

//reg for RROR
DEFINE_REG(rror, ror_status_clear,     0x00, 8, 8);
DEFINE_REG(rror, ror_inorder,          0x01, 0, 0);
DEFINE_REG(rror, miu_hw_arb_update,    0x1f, 8, 8);

//req for WROR
DEFINE_REG(wror, ror_status_clear,     0x00, 8, 8);
DEFINE_REG(wror, ror_inorder,          0x01, 0, 0);
DEFINE_REG(wror, miu_hw_arb_update,    0x1f, 8, 8);

//reg for g3d_status_clr
DEFINE_REG(gpu,  g3d_status_clear,     0x61, 3, 3);

DEFINE_REG(gpu, soft_reset,     0x0, 0, 0);
DEFINE_REG(gpu, sram_sd_en,     0x43, 3, 3);
DEFINE_REG(gpu, dynamic_clk_gating_en,     0x4b, 8, 8);
DEFINE_REG(gpu, rreq_rd_len0,    0x60, 8, 10);
DEFINE_REG(gpu, rreq_rd_len1,    0x61, 5, 7);

DEFINE_REG(gpu, riu,            0x6a, 0, 0);
DEFINE_REG(gpu, miu0_start,     0x75, 0, 15);
DEFINE_REG(gpu, miu0_end,       0x76, 0, 15);
DEFINE_REG(gpu, miu1_start,     0x77, 0, 15);
DEFINE_REG(gpu, miu1_end,       0x78, 0, 15);
DEFINE_REG(gpu, nodefine_hit_r, 0x7b, 4, 4);
DEFINE_REG(mpu, mpu_enable, 0x60, 0, 0);

DEFINE_REG(gpu_pll, ctrl0,    0x3, 0, 0);
DEFINE_REG(gpu_pll, ctrl1,    0x4, 0, 7);


DEFINE_REG(clkgen1, ckg_gpu,    0x20, 0, 0);

DEFINE_REG(miu0, dram_size,     0x69, 12, 15);
DEFINE_REG(miu1, dram_size,     0x69, 12, 15);

#if ENABLE_GPU_MFDEC
DEFINE_REG_BANK(disp_mfdec0, 0x122d00);
DEFINE_REG_BANK(mfdec_clock, 0x100b00);

DEFINE_REG(mfdec_clock, CKG_DC_SRAM0_MFDEC_USE, 0x4f, 0, 3);
DEFINE_REG(mfdec_clock, CKG_DC_SRAM1_MFDEC_USE, 0x4f, 4, 7);

DEFINE_REG(gpu, mfdec_y_start_low,       0x18, 0, 15);
DEFINE_REG(gpu, mfdec_y_start_high,      0x19, 0, 5);
DEFINE_REG(gpu, mfdec_y_end_low,         0x1a, 0, 15);
DEFINE_REG(gpu, mfdec_y_end_high,        0x1b, 0, 5);
DEFINE_REG(gpu, mfdec_uv_start_low,      0x1c, 0, 15);
DEFINE_REG(gpu, mfdec_uv_start_high,     0x1d, 0, 5);
DEFINE_REG(gpu, mfdec_uv_end_low,        0x1e, 0, 15);
DEFINE_REG(gpu, mfdec_uv_end_high,       0x1f, 0, 5);
DEFINE_REG(gpu, mfdec_adr_idx,           0x3a, 0, 3);
DEFINE_REG(gpu, mfdec_adr_update,        0x3a, 4, 4);
DEFINE_REG(gpu, mfdec_valid,             0x52, 0, 15);
DEFINE_REG(gpu, video_pitch,             0x53, 0, 14);
DEFINE_REG(gpu, mfdec_16b_mode,          0x54, 0, 15);
DEFINE_REG(gpu, mfdec_off,               0x6a, 1, 1);

DEFINE_REG(disp_mfdec0, mfdec_clk_gated,               0x0,  12, 12);
DEFINE_REG(disp_mfdec0, mfdec_reset,                   0x0,  0,  0);
DEFINE_REG(disp_mfdec0, mfdec_xpu_sel,                 0x1,  0,  0);
DEFINE_REG(disp_mfdec0, gpu_mfdec_map,                 0x1,  4,  4);
DEFINE_REG(disp_mfdec0, cpu_mfdec_map,                 0x1,  5,  5);
DEFINE_REG(disp_mfdec0, prg_fb_type,                   0x8,  0,  1);
DEFINE_REG(disp_mfdec0, prg_fb_idx,                    0x8,  4,  7);
DEFINE_REG(disp_mfdec0, fb_para_data_low,              0x9,  0,  15);
DEFINE_REG(disp_mfdec0, fb_para_data_high,             0xa,  0,  15);
DEFINE_REG(disp_mfdec0, tagram_inv,                    0xe,  0,  0);
DEFINE_REG(disp_mfdec0, rt_tagram_inv,                 0xe,  1,  1);
DEFINE_REG(disp_mfdec0, vd_iommu_tagram_inv,           0xe,  4,  4);
DEFINE_REG(disp_mfdec0, mfd_l_cache_flush_timeout_lb,  0x12, 0,  5);
DEFINE_REG(disp_mfdec0, mfd_l_cache_flush_timeout_ub,  0x12, 8,  13);
DEFINE_REG(disp_mfdec0, mfd_ofs_cache,                 0x16, 15, 15);
DEFINE_REG(disp_mfdec0, mfd_cpx_pf_en,                 0x50, 12, 12);

#endif

#define GPU_CLOCK_STEP              12

