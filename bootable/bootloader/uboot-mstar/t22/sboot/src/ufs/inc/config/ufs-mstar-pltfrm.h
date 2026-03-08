/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef UFS_MSTAR_PLTFRM_H_
#define UFS_MSTAR_PLTFRM_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "../common/datatype.h"
#include "../common/compat.h"
#include "../common/scsi_cmnd.h"
//#include "drvTimer.h"


#define CONFIG_MSTAR_UFS	1

#define CONFIG_MSTAR_UFS_IPVERIFY	1

#define CONFIG_MSTAR_UFS_SBOOT_VERSION    1

//#define CONFIG_MSTAR_UFS_HIGHER_POWER_MODE	0

#define RIU_PM_BASE				0x1F000000UL
#define RIU_BASE                0x1F200000U

#define REG_UFSHCI_ADDR			(RIU_PM_BASE+(0x380200<<1))
#define REG_UFSHCI_MSTAR_ADDR	(RIU_PM_BASE+(0x301200<<1))
#define REG_UFSHCI_PAD_ADDR 	(RIU_PM_BASE+(0x302100<<1))
#define REG_CHIPTOP_ADDR		(RIU_PM_BASE+(0x101E00<<1))
#define REG_CLKGEN_ADDR			(RIU_PM_BASE+(0x100B00<<1))
#define REG_CLKGENB_ADDR		(RIU_PM_BASE+(0x103300<<1))
#define REG_EFUSE_ADDR			(RIU_PM_BASE+(0x002000<<1))

#define REG_BANK_PORTSTATUS                 0x0280U

#define CAP_MSTAR_MAX_RX_PWM_GEAR   3
#define CAP_MSTAR_MAX_TX_PWM_GEAR   3
#define CAP_MSTAR_MAX_RX_HS_GEAR    3
#define CAP_MSTAR_MAX_TX_HS_GEAR    3

#define CONFIG_MSTAR_UFS_PWR        FASTAUTO_MODE
#define CONFIG_MSTAR_UFS_GEAR       1

#define REG(Reg_Addr)			(*(volatile unsigned short*)(Reg_Addr))
#define REG_OFFSET_SHIFT_BITS	2
#define GET_REG_ADDR(x, y)		(x+((y)<<REG_OFFSET_SHIFT_BITS))
#define REG_WRITE_UINT16(reg_addr, val)		REG(reg_addr) = val
#define REG_READ_UINT16(reg_addr, val)		val = REG(reg_addr)
#define REG_SET_BITS_UINT16(reg_addr, val)	REG(reg_addr) |= (val)
#define REG_CLR_BITS_UINT16(reg_addr, val)	REG(reg_addr) &= ~(val)

#define PORTSTATUS_BASE         GET_REG_ADDR(RIU_BASE, REG_BANK_PORTSTATUS)

#define REG_UFSHCI_MI0_START_L	GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x22)
#define REG_UFSHCI_MI0_START_H	GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x23)
#define REG_UFSHCI_MI0_END_L	GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x24)
#define REG_UFSHCI_MI0_END_H	GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x25)
#define REG_UFSHCI_DUMMY0		GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x36)
#define REG_UFSHCI_PWM_10B		GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x42)

#define REG_UFSHCI_RESET_N		GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x44)

#define REG_UFSHCI_0x70			GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x70)
#define REG_UFSHCI_0x71			GET_REG_ADDR(REG_UFSHCI_MSTAR_ADDR, 0x71)

#define REG_CLKGENB_0x3E		GET_REG_ADDR(REG_CLKGENB_ADDR, 0x3E)

#define REG_BANK3021_0x00		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x00)
#define REG_BANK3021_0x01		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x01)
#define REG_BANK3021_0x02		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x02)
#define REG_BANK3021_0x03		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x03)
#define REG_BANK3021_0x05		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x05)
#define REG_BANK3021_0x06		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x06)
#define REG_BANK3021_0x14		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x14)
#define REG_BANK3021_0x19		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x19)
#define REG_BANK3021_0x1A		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x1A)
#define REG_BANK3021_0x1C		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x1C)
#define REG_BANK3021_0x1F		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x1F)
#define REG_BANK3021_0x22		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x22)
#define REG_BANK3021_0x3D		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x3D)
#define REG_BANK3021_0x3E		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x3E)
#define REG_BANK3021_0x4A		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x4A)
#define REG_BANK3021_0x48		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x48)
#define REG_BANK3021_0x69		GET_REG_ADDR(REG_UFSHCI_PAD_ADDR, 0x69)


#define REG_BANKeFuse_0x28		GET_REG_ADDR(REG_EFUSE_ADDR, 0x28)
#define REG_BANKeFuse_0x2D		GET_REG_ADDR(REG_EFUSE_ADDR, 0x2D)


#define DEBUG_REG               GET_REG_ADDR(PORTSTATUS_BASE, 0x01)
#define DEBUG_REG_SYS_L1        GET_REG_ADDR(PORTSTATUS_BASE, 0x02)
#define DEBUG_REG_SYS_L2        GET_REG_ADDR(PORTSTATUS_BASE, 0x03)
#define DEBUG_REG_SYS_L3        GET_REG_ADDR(PORTSTATUS_BASE, 0x04)
#define DEBUG_REG_SYS_L4        GET_REG_ADDR(PORTSTATUS_BASE, 0x05)


#define REG_TSP_BOOT_NEW_CLK	GET_REG_ADDR(REG_CHIPTOP_ADDR, 0x4B)

#define REG_CKG_UFSHCI			GET_REG_ADDR(REG_CLKGEN_ADDR, 0x6A)

#define BIT_UFSHCI_PWM_10B		BIT2
#define USE_MTK_PHY				0

#define UFS_BOOT_SEL_EN()		{REG_SET_BITS_UINT16(REG_UFSHCI_PWM_10B, BIT1);}

#define UFS_BOOT_SEL_DIS()		{REG_CLR_BITS_UINT16(REG_UFSHCI_PWM_10B, BIT1);}

#define UFS_DUMMY_ZERO()		REG_WRITE_UINT16(REG_UFSHCI_DUMMY0, 0)

#define UFS_INCREASE_DUMMY()	REG_WRITE_UINT16(REG_UFSHCI_DUMMY0, REG(REG_UFSHCI_DUMMY0) + 1)

#define UFS_DUMMY()				REG(REG_UFSHCI_DUMMY0)

#define REG_BANK_TIMER0			(0x1800)
#define TIMER0_REG_BASE_ADDR	GET_REG_ADDR(RIU_PM_BASE, REG_BANK_TIMER0)
#define TIMER0_ENABLE			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x20)
#define TIMER0_HIT				GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x21)
#define TIMER0_MAX_LOW			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x22)
#define TIMER0_MAX_HIGH			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x23)
#define TIMER0_CAP_LOW			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x24)
#define TIMER0_CAP_HIGH			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x25)

//#define CONFIG_MSTAR_UFS_DEBUG

#if defined(CONFIG_MSTAR_UFS_SBOOT_VERSION) && CONFIG_MSTAR_UFS_SBOOT_VERSION
#define CONFIG_MSTAR_DRAM_TEMP_BUF             (CONFIG_BOOTRAM_LOADADDR-0x5000)
#define CONFIG_MSTAR_DRAM_BASE_ADDR             (CONFIG_BOOTRAM_LOADADDR-0x4000)

#define CONFIG_MSTAR_UFS_PDEV_VARIABLE_ADDR     (CONFIG_MSTAR_DRAM_BASE_ADDR+0x1000)		//size of int
#define CONFIG_MSTAR_UFS_HBA_VARIABLE_ADDR      (CONFIG_MSTAR_DRAM_BASE_ADDR+0x2000)
#define CONFIG_MSTAR_UFS_SBOOT_MEMORY_ST_ADDR     (CONFIG_MSTAR_DRAM_BASE_ADDR+0x3000)	//size 0x1000

#define CONFIG_MSTAR_SRAM_BASE_ADDR             0x1fc00000

#define CONFIG_MSTAR_UFS_ROM_HBA_VARIABLE_ADDR      (CONFIG_MSTAR_SRAM_BASE_ADDR+0x3900)


#define BOOT_INFO_REGISTER_DMA_ADDR_HIGH	GET_REG_ADDR(PORTSTATUS_BASE, 0xC)
#define BOOT_INFO_REGISTER_DMA_ADDR_LOW		GET_REG_ADDR(PORTSTATUS_BASE, 0xD)
#endif

#define udelay(TimerCount) 	ufs_hw_timer_delay(TimerCount)

#define writel(value, addr)	(*(volatile u32*)addr) = cpu_to_le32(value)

#define readl(addr)	le32_to_cpu(*(volatile U32*)addr)

extern int ufs_mstar_pltfrm_init(void);
extern int ufs_mstar_pltfrm_clock(int gear_rx);
extern int ufs_mstar_pltfrm_map_sg(struct scsi_cmnd *cmndp, void *pBuf, u32 dLength);

enum {
	DUMP_PREFIX_NONE,
	DUMP_PREFIX_ADDRESS,
	DUMP_PREFIX_OFFSET
};
/* Byte order for UTF-16 strings */
enum utf16_endian {
	UTF16_HOST_ENDIAN,
	UTF16_LITTLE_ENDIAN,
	UTF16_BIG_ENDIAN
};
//extern void writel(u32 value, volatile void __iomem *addr);
//extern u32 readl(const volatile void __iomem *addr);

extern void *ufs_memset(void *s, int c, size_t count);
extern void *ufs_memcpy(void *__dest, __const void *__src, size_t __n);

#endif /* UFS_MSTAR_PLTFRM_H_ */
