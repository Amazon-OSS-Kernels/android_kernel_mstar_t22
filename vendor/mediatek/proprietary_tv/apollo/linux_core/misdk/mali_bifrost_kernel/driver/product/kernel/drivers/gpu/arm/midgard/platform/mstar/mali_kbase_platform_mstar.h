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

#ifndef _KBASE_PLATFORM_MSTAR_H_
#define _KBASE_PLATFORM_MSTAR_H_

#include <mali_kbase_defs.h>

/* RIU */
#ifdef CONFIG_ARM64
#define MSTAR_RIU_BASE mstar_pm_base
extern ptrdiff_t mstar_pm_base;
#else
#define MSTAR_RIU_BASE 0xfd000000
#endif

#if MSTAR_CLOCK_DEBUG_ADJUSTABLE
#define MALI_DEBUG_PROPERTY_PASSWORD 112233
#endif

#define RIU ((volatile unsigned short*)(MSTAR_RIU_BASE))

/* Macros to manage registers */
#define DEFINE_REG_BANK(bank, base) \
    static const u32 reg_##bank##_base = base;

#define DEFINE_REG(bank, name, addr, lsb, msb) \
    static const u16 reg_##bank##_##name = addr; \
    static const u16 shift_##bank##_##name = lsb; \
    static const u16 mask_##bank##_##name = ((1 << (msb - lsb + 1)) - 1) << lsb;

#define CLEAR_REG(bank, name, value) \
    RIU[reg_##bank##_base + (reg_##bank##_##name << 1)] &= ~mask_##bank##_##name;

#define CLEAR_REG_VAL(bank, name, value) \
    RIU[reg_##bank##_base + (reg_##bank##_##name << 1)] &= ((~value) << shift_##bank##_##name) & mask_##bank##_##name;

#define SET_REG(bank, name, value) \
    RIU[reg_##bank##_base + (reg_##bank##_##name << 1)] &= ~mask_##bank##_##name; \
    RIU[reg_##bank##_base + (reg_##bank##_##name << 1)] |= ((value) << shift_##bank##_##name) & mask_##bank##_##name;

#define SET_REG_NO_CLEAR(bank, name, value) \
    RIU[reg_##bank##_base + (reg_##bank##_##name << 1)] |= ((value) << shift_##bank##_##name) & mask_##bank##_##name;

#define GET_REG(bank, name) \
    (RIU[reg_##bank##_base + (reg_##bank##_##name << 1)] & mask_##bank##_##name) >> shift_##bank##_##name

/* Marcos to convert Mali frequency to register value */
#define MHZ_TO_REG(MHZ) (0x6C000000/(MHZ))

/* Platform and PM Callbacks */
int mstar_platform_init(struct kbase_device* kbdev);
void mstar_platform_term(struct kbase_device* kbdev);
void mstar_pm_off(struct kbase_device* kbdev);
int mstar_pm_on(struct kbase_device* kbdev);
void mstar_pm_suspend(struct kbase_device* kbdev);
void mstar_pm_resume(struct kbase_device* kbdev);

/* Internal platform functions */
extern void init_registers(void);
extern void power_on(void);
extern void power_off(void);
extern int dvfs_init(struct kbase_device* kbdev);
extern void dvfs_term(struct kbase_device* kbdev);
#ifdef MSTAR_DISABLE_SHADER_CORES
extern int get_num_disabled_cores(void);
#endif

#if ENABLE_GPU_MFDEC
/* define the prototype here for the access of extended mali ioctl */
void init_mfdec(void);
int set_mfdec_bridge(u8 id, u64 base, u32 height, u32 pitch, u8 is_tiled_yuv);
int set_mfdec_detile(u8 id, u64 video_ybase, u64 video_uvbase, u32 pitch, u32 tile_mode, u32 flags);
int set_mfdec_decode(u8 id, u64 video_ybase, u64 video_uvbase, u64 bld_base, u32 bld_pitch, u32 codec_type, u32 flags);
int reset_mfdec_slot(u8 id);
#endif

#if defined(MSTAR_M7621) && defined(MSTAR_DEGLITCH_PATCH)
extern int set_deglitch_mux_to_216mhz(u8 enable);
#endif

#if defined(MSTAR_M7821)
extern void power_suspend(void);
extern void power_resume(void);
#endif

extern void get_dram_length(u32* miu0_length, u32* miu1_length, u32* miu2_length);

#endif /* _KBASE_PLATFORM_MSTAR_H_ */
