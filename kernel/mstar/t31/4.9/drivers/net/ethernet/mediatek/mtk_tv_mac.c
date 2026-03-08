/* MediaTek TV MAC network driver.
 *
 * Copyright (c) 2016-2017 MediaTek Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/mii.h>
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_net.h>
#include <linux/dmapool.h>
#include <linux/ethtool.h>
//#include "chip_setup.h"

#define MTKTV_PR_ERR(fmt...) pr_err("mtktv mac: " fmt)
#define MTKTV_PR_INFO(fmt...) pr_info("mtktv mac: " fmt)
#define MTKTV_PR_DEBUG(fmt...) /*pr_info("mtktv mac debug: " fmt)*/

#define RX_DESC_SIZE_DEFAULT 8
#define RX_DESC_NUM_DEFAULT 64
/* rx descriptor pointer must be 16K alignment */
#define RX_DESC_PTR_ALIGNMENT 16384
#define RX_DESC_DONE 0x00000001UL
#define RX_NAPI_WEIGHT 64
#if 0 /* tx descriptor */
#define TX_DESC_SIZE_DEFAULT 16
#define TX_DESC_NUM_DEFAULT 64
#define TX_DESC_PTR_ALIGNMENT 16384
#endif /* tx descriptor */
/* tx memcpy */
#define TX_MEMCPY_SIZE_DEFAULT MAC_TX_MAX_LEN
#define TX_MEMCPY_NUM_DEFAULT 8
#define TX_MEMCPY_PTR_ALIGNMENT 2

#define MAC_MTU 1518
/* hw limit tx max length */
#define MAC_TX_MAX_LEN 1580
/* hw limit rx max length */
#define MAC_RX_MAX_LEN 1522
#define MAC_EXTRA_PKT_LEN 36
/* hw descriptor version, default is v2 */
#define MAC_HW_DESC_VER_DEFAULT 2
#define MAC_UPDATE_LINK_TIME msecs_to_jiffies(1000)

/* workaround, it should be from dts */
#define MIU0_BUS_BASE 0x20000000UL
#define MIU1_BUS_BASE 0xA0000000UL

/* registers */
#define IRQ_BIT_DONE (0x1UL)
#define IRQ_BIT_RCOM (0x1UL << 1)
#define IRQ_BIT_RBNA (0x1UL << 2)
#define IRQ_BIT_TOVR (0x1UL << 3)
#define IRQ_BIT_TUND (0x1UL << 4)
#define IRQ_BIT_RTRY (0x1UL << 5)
#define IRQ_BIT_TBRE (0x1UL << 6)
#define IRQ_BIT_TCOM (0x1UL << 7)
#define IRQ_BIT_TIDLE (0x1UL << 8)
#define IRQ_BIT_LINK (0x1UL << 9)
#define IRQ_BIT_ROVR (0x1UL << 10)
#define IRQ_BIT_HRESP (0x1UL << 11)
#define IRQ_BIT_EEE (0x1UL << 12)

#define IRQ_RSR_DNA (0x1UL)
#define IRQ_RSR_REC (0x1UL << 1)
#define IRQ_RSR_OVR_RSR (0x1UL << 2)
#define IRQ_RSR_BNA (0x1UL << 3)

#define IRQ_TSR_OVER (0x1UL)
#define IRQ_TSR_COL (0x1UL << 1)
#define IRQ_TSR_RLE (0x1UL << 2)
#define IRQ_TSR_IDLE (0x1UL << 3)
#define IRQ_TSR_BNQ (0x1UL << 4)
#define IRQ_TSR_COMP (0x1UL << 5)
#define IRQ_TSR_UND (0x1UL << 6)
#define IRQ_TSR_TBNQ (0x1UL << 7)
#define IRQ_TSR_FBNQ (0x1UL << 8)
#define IRQ_TSR_FIFO1_IDLE (0x1UL << 9)
#define IRQ_TSR_FIFO2_IDLE (0x1UL << 10)
#define IRQ_TSR_FIFO3_IDLE (0x1UL << 11)
#define IRQ_TSR_FIFO4_IDLE (0x1UL << 12)

/* rbna/tovr/tund/rtry/rovr */
#define IRQ_ENABLE_BIT 0x43CUL
/* rbna/tovr/tund/rtry/rovr/tcom */
//#define IRQ_ENABLE_BIT 0x4BCUL
#define IRQ_DISABLE_ALL 0xFFFFFFFFUL

/* mac descriptor */
#define MAC_DESC_V1_IPV4 (0x1UL << 11)
#define MAC_DESC_V1_UDP (0x1UL << 18)
#define MAC_DESC_V1_TCP (0x1UL << 19)
#define MAC_DESC_V1_TCP_UDP_CSUM (0x1UL << 20)
#define MAC_DESC_V1_IP_CSUM (0x1UL << 21)
#define MAC_DESC_V1_IPV6 (0x1UL << 27)

#define MAC_DESC_V2_PKT_TYPE_BIT2 (0x1UL << 19)
#define MAC_DESC_V2_PKT_TYPE_BIT1 (0x1UL << 18)
#define MAC_DESC_V2_PKT_TYPE_BIT0 (0x1UL << 11)
#define MAC_DESC_V2_TCP_UDP_CSUM (0x1UL << 20)
#define MAC_DESC_V2_IP_CSUM (0x1UL << 21)

#define BANK_BASE_MAC_0 (priv->mac_bank_0)
#define BANK_BASE_MAC_1 (priv->mac_bank_1)
#define BANK_BASE_ALBANY_0 (priv->albany_bank_0)
#define BANK_BASE_ALBANY_1 (priv->albany_bank_0 + 0x200)
#define BANK_BASE_ALBANY_2 (priv->albany_bank_0 + 0x400)
#define BANK_BASE_CLKGEN_0 (priv->clkgen_bank_0)
#define BANK_BASE_CHIP (priv->chip_bank)

#define REG_OFFSET_00_L(base) (base + 0x0000)
#define REG_OFFSET_00_H(base) (base + 0x0001)
#define REG_OFFSET_01_L(base) (base + 0x0004)
#define REG_OFFSET_01_H(base) (base + 0x0005)
#define REG_OFFSET_02_L(base) (base + 0x0008)
#define REG_OFFSET_02_H(base) (base + 0x0009)
#define REG_OFFSET_03_L(base) (base + 0x000c)
#define REG_OFFSET_03_H(base) (base + 0x000d)
#define REG_OFFSET_04_L(base) (base + 0x0010)
#define REG_OFFSET_04_H(base) (base + 0x0011)
#define REG_OFFSET_05_L(base) (base + 0x0014)
#define REG_OFFSET_05_H(base) (base + 0x0015)
#define REG_OFFSET_06_L(base) (base + 0x0018)
#define REG_OFFSET_06_H(base) (base + 0x0019)
#define REG_OFFSET_07_L(base) (base + 0x001c)
#define REG_OFFSET_07_H(base) (base + 0x001d)
#define REG_OFFSET_08_L(base) (base + 0x0020)
#define REG_OFFSET_08_H(base) (base + 0x0021)
#define REG_OFFSET_09_L(base) (base + 0x0024)
#define REG_OFFSET_09_H(base) (base + 0x0025)
#define REG_OFFSET_0A_L(base) (base + 0x0028)
#define REG_OFFSET_0A_H(base) (base + 0x0029)
#define REG_OFFSET_0B_L(base) (base + 0x002c)
#define REG_OFFSET_0B_H(base) (base + 0x002d)
#define REG_OFFSET_0C_L(base) (base + 0x0030)
#define REG_OFFSET_0C_H(base) (base + 0x0031)
#define REG_OFFSET_0D_L(base) (base + 0x0034)
#define REG_OFFSET_0D_H(base) (base + 0x0035)
#define REG_OFFSET_0E_L(base) (base + 0x0038)
#define REG_OFFSET_0E_H(base) (base + 0x0039)
#define REG_OFFSET_0F_L(base) (base + 0x003c)
#define REG_OFFSET_0F_H(base) (base + 0x003d)

#define REG_OFFSET_10_L(base) (base + 0x0040)
#define REG_OFFSET_10_H(base) (base + 0x0041)
#define REG_OFFSET_11_L(base) (base + 0x0044)
#define REG_OFFSET_11_H(base) (base + 0x0045)
#define REG_OFFSET_12_L(base) (base + 0x0048)
#define REG_OFFSET_12_H(base) (base + 0x0049)
#define REG_OFFSET_13_L(base) (base + 0x004c)
#define REG_OFFSET_13_H(base) (base + 0x004d)
#define REG_OFFSET_14_L(base) (base + 0x0050)
#define REG_OFFSET_14_H(base) (base + 0x0051)
#define REG_OFFSET_15_L(base) (base + 0x0054)
#define REG_OFFSET_15_H(base) (base + 0x0055)
#define REG_OFFSET_16_L(base) (base + 0x0058)
#define REG_OFFSET_16_H(base) (base + 0x0059)
#define REG_OFFSET_17_L(base) (base + 0x005c)
#define REG_OFFSET_17_H(base) (base + 0x005d)
#define REG_OFFSET_18_L(base) (base + 0x0060)
#define REG_OFFSET_18_H(base) (base + 0x0061)
#define REG_OFFSET_19_L(base) (base + 0x0064)
#define REG_OFFSET_19_H(base) (base + 0x0065)
#define REG_OFFSET_1A_L(base) (base + 0x0068)
#define REG_OFFSET_1A_H(base) (base + 0x0069)
#define REG_OFFSET_1B_L(base) (base + 0x006c)
#define REG_OFFSET_1B_H(base) (base + 0x006d)
#define REG_OFFSET_1C_L(base) (base + 0x0070)
#define REG_OFFSET_1C_H(base) (base + 0x0071)
#define REG_OFFSET_1D_L(base) (base + 0x0074)
#define REG_OFFSET_1D_H(base) (base + 0x0075)
#define REG_OFFSET_1E_L(base) (base + 0x0078)
#define REG_OFFSET_1E_H(base) (base + 0x0079)
#define REG_OFFSET_1F_L(base) (base + 0x007c)
#define REG_OFFSET_1F_H(base) (base + 0x007d)

#define REG_OFFSET_20_L(base) (base + 0x0080)
#define REG_OFFSET_20_H(base) (base + 0x0081)
#define REG_OFFSET_21_L(base) (base + 0x0084)
#define REG_OFFSET_21_H(base) (base + 0x0085)
#define REG_OFFSET_22_L(base) (base + 0x0088)
#define REG_OFFSET_22_H(base) (base + 0x0089)
#define REG_OFFSET_23_L(base) (base + 0x008c)
#define REG_OFFSET_23_H(base) (base + 0x008d)
#define REG_OFFSET_24_L(base) (base + 0x0090)
#define REG_OFFSET_24_H(base) (base + 0x0091)
#define REG_OFFSET_25_L(base) (base + 0x0094)
#define REG_OFFSET_25_H(base) (base + 0x0095)
#define REG_OFFSET_26_L(base) (base + 0x0098)
#define REG_OFFSET_26_H(base) (base + 0x0099)
#define REG_OFFSET_27_L(base) (base + 0x009c)
#define REG_OFFSET_27_H(base) (base + 0x009d)
#define REG_OFFSET_28_L(base) (base + 0x00a0)
#define REG_OFFSET_28_H(base) (base + 0x00a1)
#define REG_OFFSET_29_L(base) (base + 0x00a4)
#define REG_OFFSET_29_H(base) (base + 0x00a5)
#define REG_OFFSET_2A_L(base) (base + 0x00a8)
#define REG_OFFSET_2A_H(base) (base + 0x00a9)
#define REG_OFFSET_2B_L(base) (base + 0x00ac)
#define REG_OFFSET_2B_H(base) (base + 0x00ad)
#define REG_OFFSET_2C_L(base) (base + 0x00b0)
#define REG_OFFSET_2C_H(base) (base + 0x00b1)
#define REG_OFFSET_2D_L(base) (base + 0x00b4)
#define REG_OFFSET_2D_H(base) (base + 0x00b5)
#define REG_OFFSET_2E_L(base) (base + 0x00b8)
#define REG_OFFSET_2E_H(base) (base + 0x00b9)
#define REG_OFFSET_2F_L(base) (base + 0x00bc)
#define REG_OFFSET_2F_H(base) (base + 0x00bd)

#define REG_OFFSET_30_L(base) (base + 0x00c0)
#define REG_OFFSET_30_H(base) (base + 0x00c1)
#define REG_OFFSET_31_L(base) (base + 0x00c4)
#define REG_OFFSET_31_H(base) (base + 0x00c5)
#define REG_OFFSET_32_L(base) (base + 0x00c8)
#define REG_OFFSET_32_H(base) (base + 0x00c9)
#define REG_OFFSET_33_L(base) (base + 0x00cc)
#define REG_OFFSET_33_H(base) (base + 0x00cd)
#define REG_OFFSET_34_L(base) (base + 0x00d0)
#define REG_OFFSET_34_H(base) (base + 0x00d1)
#define REG_OFFSET_35_L(base) (base + 0x00d4)
#define REG_OFFSET_35_H(base) (base + 0x00d5)
#define REG_OFFSET_36_L(base) (base + 0x00d8)
#define REG_OFFSET_36_H(base) (base + 0x00d9)
#define REG_OFFSET_37_L(base) (base + 0x00dc)
#define REG_OFFSET_37_H(base) (base + 0x00dd)
#define REG_OFFSET_38_L(base) (base + 0x00e0)
#define REG_OFFSET_38_H(base) (base + 0x00e1)
#define REG_OFFSET_39_L(base) (base + 0x00e4)
#define REG_OFFSET_39_H(base) (base + 0x00e5)
#define REG_OFFSET_3A_L(base) (base + 0x00e8)
#define REG_OFFSET_3A_H(base) (base + 0x00e9)
#define REG_OFFSET_3B_L(base) (base + 0x00ec)
#define REG_OFFSET_3B_H(base) (base + 0x00ed)
#define REG_OFFSET_3C_L(base) (base + 0x00f0)
#define REG_OFFSET_3C_H(base) (base + 0x00f1)
#define REG_OFFSET_3D_L(base) (base + 0x00f4)
#define REG_OFFSET_3D_H(base) (base + 0x00f5)
#define REG_OFFSET_3E_L(base) (base + 0x00f8)
#define REG_OFFSET_3E_H(base) (base + 0x00f9)
#define REG_OFFSET_3F_L(base) (base + 0x00fc)
#define REG_OFFSET_3F_H(base) (base + 0x00fd)

#define REG_OFFSET_40_L(base) (base + 0x0100)
#define REG_OFFSET_40_H(base) (base + 0x0101)
#define REG_OFFSET_41_L(base) (base + 0x0104)
#define REG_OFFSET_41_H(base) (base + 0x0105)
#define REG_OFFSET_42_L(base) (base + 0x0108)
#define REG_OFFSET_42_H(base) (base + 0x0109)
#define REG_OFFSET_43_L(base) (base + 0x010c)
#define REG_OFFSET_43_H(base) (base + 0x010d)
#define REG_OFFSET_44_L(base) (base + 0x0110)
#define REG_OFFSET_44_H(base) (base + 0x0111)
#define REG_OFFSET_45_L(base) (base + 0x0114)
#define REG_OFFSET_45_H(base) (base + 0x0115)
#define REG_OFFSET_46_L(base) (base + 0x0118)
#define REG_OFFSET_46_H(base) (base + 0x0119)
#define REG_OFFSET_47_L(base) (base + 0x011c)
#define REG_OFFSET_47_H(base) (base + 0x011d)
#define REG_OFFSET_48_L(base) (base + 0x0120)
#define REG_OFFSET_48_H(base) (base + 0x0121)
#define REG_OFFSET_49_L(base) (base + 0x0124)
#define REG_OFFSET_49_H(base) (base + 0x0125)
#define REG_OFFSET_4A_L(base) (base + 0x0128)
#define REG_OFFSET_4A_H(base) (base + 0x0129)
#define REG_OFFSET_4B_L(base) (base + 0x012c)
#define REG_OFFSET_4B_H(base) (base + 0x012d)
#define REG_OFFSET_4C_L(base) (base + 0x0130)
#define REG_OFFSET_4C_H(base) (base + 0x0131)
#define REG_OFFSET_4D_L(base) (base + 0x0134)
#define REG_OFFSET_4D_H(base) (base + 0x0135)
#define REG_OFFSET_4E_L(base) (base + 0x0138)
#define REG_OFFSET_4E_H(base) (base + 0x0139)
#define REG_OFFSET_4F_L(base) (base + 0x013c)
#define REG_OFFSET_4F_H(base) (base + 0x013d)

#define REG_OFFSET_50_L(base) (base + 0x0140)
#define REG_OFFSET_50_H(base) (base + 0x0141)
#define REG_OFFSET_51_L(base) (base + 0x0144)
#define REG_OFFSET_51_H(base) (base + 0x0145)
#define REG_OFFSET_52_L(base) (base + 0x0148)
#define REG_OFFSET_52_H(base) (base + 0x0149)
#define REG_OFFSET_53_L(base) (base + 0x014c)
#define REG_OFFSET_53_H(base) (base + 0x014d)
#define REG_OFFSET_54_L(base) (base + 0x0150)
#define REG_OFFSET_54_H(base) (base + 0x0151)
#define REG_OFFSET_55_L(base) (base + 0x0154)
#define REG_OFFSET_55_H(base) (base + 0x0155)
#define REG_OFFSET_56_L(base) (base + 0x0158)
#define REG_OFFSET_56_H(base) (base + 0x0159)
#define REG_OFFSET_57_L(base) (base + 0x015c)
#define REG_OFFSET_57_H(base) (base + 0x015d)
#define REG_OFFSET_58_L(base) (base + 0x0160)
#define REG_OFFSET_58_H(base) (base + 0x0161)
#define REG_OFFSET_59_L(base) (base + 0x0164)
#define REG_OFFSET_59_H(base) (base + 0x0165)
#define REG_OFFSET_5A_L(base) (base + 0x0168)
#define REG_OFFSET_5A_H(base) (base + 0x0169)
#define REG_OFFSET_5B_L(base) (base + 0x016c)
#define REG_OFFSET_5B_H(base) (base + 0x016d)
#define REG_OFFSET_5C_L(base) (base + 0x0170)
#define REG_OFFSET_5C_H(base) (base + 0x0171)
#define REG_OFFSET_5D_L(base) (base + 0x0174)
#define REG_OFFSET_5D_H(base) (base + 0x0175)
#define REG_OFFSET_5E_L(base) (base + 0x0178)
#define REG_OFFSET_5E_H(base) (base + 0x0179)
#define REG_OFFSET_5F_L(base) (base + 0x017c)
#define REG_OFFSET_5F_H(base) (base + 0x017d)

#define REG_OFFSET_60_L(base) (base + 0x0180)
#define REG_OFFSET_60_H(base) (base + 0x0181)
#define REG_OFFSET_61_L(base) (base + 0x0184)
#define REG_OFFSET_61_H(base) (base + 0x0185)
#define REG_OFFSET_62_L(base) (base + 0x0188)
#define REG_OFFSET_62_H(base) (base + 0x0189)
#define REG_OFFSET_63_L(base) (base + 0x018c)
#define REG_OFFSET_63_H(base) (base + 0x018d)
#define REG_OFFSET_64_L(base) (base + 0x0190)
#define REG_OFFSET_64_H(base) (base + 0x0191)
#define REG_OFFSET_65_L(base) (base + 0x0194)
#define REG_OFFSET_65_H(base) (base + 0x0195)
#define REG_OFFSET_66_L(base) (base + 0x0198)
#define REG_OFFSET_66_H(base) (base + 0x0199)
#define REG_OFFSET_67_L(base) (base + 0x019c)
#define REG_OFFSET_67_H(base) (base + 0x019d)
#define REG_OFFSET_68_L(base) (base + 0x01a0)
#define REG_OFFSET_68_H(base) (base + 0x01a1)
#define REG_OFFSET_69_L(base) (base + 0x01a4)
#define REG_OFFSET_69_H(base) (base + 0x01a5)
#define REG_OFFSET_6A_L(base) (base + 0x01a8)
#define REG_OFFSET_6A_H(base) (base + 0x01a9)
#define REG_OFFSET_6B_L(base) (base + 0x01ac)
#define REG_OFFSET_6B_H(base) (base + 0x01ad)
#define REG_OFFSET_6C_L(base) (base + 0x01b0)
#define REG_OFFSET_6C_H(base) (base + 0x01b1)
#define REG_OFFSET_6D_L(base) (base + 0x01b4)
#define REG_OFFSET_6D_H(base) (base + 0x01b5)
#define REG_OFFSET_6E_L(base) (base + 0x01b8)
#define REG_OFFSET_6E_H(base) (base + 0x01b9)
#define REG_OFFSET_6F_L(base) (base + 0x01bc)
#define REG_OFFSET_6F_H(base) (base + 0x01bd)

#define REG_OFFSET_70_L(base) (base + 0x01c0)
#define REG_OFFSET_70_H(base) (base + 0x01c1)
#define REG_OFFSET_71_L(base) (base + 0x01c4)
#define REG_OFFSET_71_H(base) (base + 0x01c5)
#define REG_OFFSET_72_L(base) (base + 0x01c8)
#define REG_OFFSET_72_H(base) (base + 0x01c9)
#define REG_OFFSET_73_L(base) (base + 0x01cc)
#define REG_OFFSET_73_H(base) (base + 0x01cd)
#define REG_OFFSET_74_L(base) (base + 0x01d0)
#define REG_OFFSET_74_H(base) (base + 0x01d1)
#define REG_OFFSET_75_L(base) (base + 0x01d4)
#define REG_OFFSET_75_H(base) (base + 0x01d5)
#define REG_OFFSET_76_L(base) (base + 0x01d8)
#define REG_OFFSET_76_H(base) (base + 0x01d9)
#define REG_OFFSET_77_L(base) (base + 0x01dc)
#define REG_OFFSET_77_H(base) (base + 0x01dd)
#define REG_OFFSET_78_L(base) (base + 0x01e0)
#define REG_OFFSET_78_H(base) (base + 0x01e1)
#define REG_OFFSET_79_L(base) (base + 0x01e4)
#define REG_OFFSET_79_H(base) (base + 0x01e5)
#define REG_OFFSET_7A_L(base) (base + 0x01e8)
#define REG_OFFSET_7A_H(base) (base + 0x01e9)
#define REG_OFFSET_7B_L(base) (base + 0x01ec)
#define REG_OFFSET_7B_H(base) (base + 0x01ed)
#define REG_OFFSET_7C_L(base) (base + 0x01f0)
#define REG_OFFSET_7C_H(base) (base + 0x01f1)
#define REG_OFFSET_7D_L(base) (base + 0x01f4)
#define REG_OFFSET_7D_H(base) (base + 0x01f5)
#define REG_OFFSET_7E_L(base) (base + 0x01f8)
#define REG_OFFSET_7E_H(base) (base + 0x01f9)
#define REG_OFFSET_7F_L(base) (base + 0x01fc)
#define REG_OFFSET_7F_H(base) (base + 0x01fd)

#define MACADDR_FORMAT "XX:XX:XX:XX:XX:XX"
static u8 mac_addr_env[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static u8 mac_addr_dts[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

struct mtktv_mac_reg_ops {
    void (*write_32)(void __iomem *reg, u32 val);
    u32 (*read_32)(void __iomem *reg);
    void (*write_8)(void __iomem *reg, u8 val);
    u8 (*read_8)(void __iomem *reg);
    void (*write_phy)(struct net_device *ndev, u8 phy_addr, u8 reg_addr, u32 val);
    u32 (*read_phy)(struct net_device *ndev, u8 phy_addr, u8 reg_addr);
    bool (*rx_checksum)(u32 desc_val);
};

struct rx_desc {
    u32 addr;
    u32 size;
};

#if 0 /* tx descriptor */
struct tx_desc {
    u32 addr;
    u32 low_tag;
    u32 reserve0;
    u32 reserve1;
};
#endif /* tx descriptor */

struct mtktv_mac_private {
    struct net_device *ndev;
    struct platform_device *pdev;
    //struct net_device_stats stats;
    struct napi_struct napi;
    void __iomem *mac_bank_0;
    void __iomem *mac_bank_1;
    void __iomem *albany_bank_0;
    void __iomem *clkgen_bank_0;
    void __iomem *chip_bank;
    struct mtktv_mac_reg_ops *reg_ops;

    dma_addr_t rx_desc_dma_addr;
    uintptr_t rx_desc_vir_addr;
    u32 rx_desc_size;
    u32 rx_desc_num;
    u32 rx_desc_index;
    struct rx_desc *rx_desc;
    struct dma_pool *rx_desc_dma_pool;
    struct sk_buff_head rx_skb_q;
    struct napi_struct napi_rx;

#if 0 /* tx descriptor */
    dma_addr_t tx_desc_dma_addr;
    uintptr_t tx_desc_vir_addr;
    u32 tx_desc_size;
    u32 tx_desc_num;
    u32 tx_desc_index;
    struct tx_desc *tx_desc;
    struct dma_pool *tx_desc_dma_pool;
    struct sk_buff_head tx_skb_q;
    u32 tx_desc_write_index;
    u32 tx_desc_read_index;
    u32 tx_desc_queued_number;
    u32 tx_desc_count;
    u32 tx_desc_full_count;
    u32 tx_desc_start_free;
#endif /* tx descriptor */

    /* tx memcpy */
    dma_addr_t tx_memcpy_dma_addr;
    uintptr_t tx_memcpy_vir_addr;
    u32 tx_memcpy_index;
    struct dma_pool *tx_memcpy_dma_pool;

    bool is_internal_phy;
    u8 hw_desc_ver;
    u8 phy_addr;
    u32 phy_restart_cnt;
    struct timer_list link_timer;

    spinlock_t tx_lock;
};

/* tools */
static inline void mtktv_mac_reg_write_8(void __iomem *reg, u8 val)
{
    iowrite8(val, reg);
    mb();
}

static inline u8 mtktv_mac_reg_read_8(void __iomem *reg)
{
    u8 data;

    data = ioread8(reg);
    mb();
    return data;
}

static inline void mtktv_mac_reg_write_32_xiu16(void __iomem *reg, u32 val)
{
    iowrite32((val & 0x0000FFFFUL), reg);
    iowrite32((val >> 0x10UL), (reg + 4));
    mb();
}

static inline u32 mtktv_mac_reg_read_32_xiu16(void __iomem *reg)
{
    u32 data_h, data_l;

    data_l = (ioread32(reg) & 0x0000FFFFUL);
    data_h = (ioread32(reg + 4) << 0x10UL);
    mb();
    return (data_l | data_h);
}

static inline void mtktv_mac_reg_write_32(void __iomem *reg, u32 val)
{
    iowrite32(val, reg);
    mb();
}

static inline u32 mtktv_mac_reg_read_32(void __iomem *reg)
{
    u32 data;

    data = ioread32(reg);
    mb();
    return data;
}

static void mtktv_mac_hw_power_on_clk(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u8 reg_read;

    /* swith rx discriptor format to mode 1 */
    reg_ops->write_8(REG_OFFSET_1D_L(BANK_BASE_MAC_1), 0x00);
    reg_ops->write_8(REG_OFFSET_1D_H(BANK_BASE_MAC_1), 0x01);

    /* gain shift */
    reg_ops->write_8(REG_OFFSET_5A_L(BANK_BASE_ALBANY_1), 0x02);

    /* det max */
    reg_ops->write_8(REG_OFFSET_27_H(BANK_BASE_ALBANY_1), 0x02);

    /* det min */
    reg_ops->write_8(REG_OFFSET_28_H(BANK_BASE_ALBANY_1), 0x01);

    /* snr len (emc noise) */
    reg_ops->write_8(REG_OFFSET_3B_H(BANK_BASE_ALBANY_1), 0x18);

    /* lpbk_enable set to 0 */
    reg_ops->write_8(REG_OFFSET_39_L(BANK_BASE_ALBANY_0), 0xa0);

    /* power-on LD0 */
    reg_ops->write_8(REG_OFFSET_7E_L(BANK_BASE_ALBANY_1), 0x00);
    reg_ops->write_8(REG_OFFSET_7E_H(BANK_BASE_ALBANY_1), 0x00);
    /* power-on SADC */
    reg_ops->write_8(REG_OFFSET_50_H(BANK_BASE_ALBANY_2), 0x80);
    /* Power-on ADCPL */
    reg_ops->write_8(REG_OFFSET_66_L(BANK_BASE_ALBANY_1), 0x40);
    /* Power-on REF */
    reg_ops->write_8(REG_OFFSET_5D_H(BANK_BASE_ALBANY_1), 0x04);
    /* Power-on TX */
    reg_ops->write_8(REG_OFFSET_1D_L(BANK_BASE_ALBANY_2), 0x00);
    /* Power-on TX */
    reg_ops->write_8(REG_OFFSET_78_H(BANK_BASE_ALBANY_2), 0x00);
    /* CLKO_ADC_SEL */
    reg_ops->write_8(REG_OFFSET_45_L(BANK_BASE_ALBANY_2), 0x01);
    /* reg_adc_clk_select */
    reg_ops->write_8(REG_OFFSET_1D_H(BANK_BASE_ALBANY_1), 0x01);
    /* Test */
    reg_ops->write_8(REG_OFFSET_62_L(BANK_BASE_ALBANY_1), 0x44);
    /* sadc timer */
    reg_ops->write_8(REG_OFFSET_40_L(BANK_BASE_ALBANY_2), 0x30);

    /* 100 gat */
    reg_ops->write_8(REG_OFFSET_62_H(BANK_BASE_ALBANY_2), 0x00);

    /* 200 gat */
    reg_ops->write_8(REG_OFFSET_18_L(BANK_BASE_ALBANY_2), 0x43);

    /* en_100t_phase */
    reg_ops->write_8(REG_OFFSET_1C_H(BANK_BASE_ALBANY_2), 0x41);

    /* LP mode, DAC OFF */
    reg_ops->write_8(REG_OFFSET_79_L(BANK_BASE_ALBANY_2), 0xf5);
    reg_ops->write_8(REG_OFFSET_79_H(BANK_BASE_ALBANY_2), 0x0d);

    /* Prevent packet drop by inverted waveform */
    reg_ops->write_8(REG_OFFSET_3C_H(BANK_BASE_ALBANY_0), 0xd0);
    reg_ops->write_8(REG_OFFSET_3B_H(BANK_BASE_ALBANY_0), 0x5a);

    /* Disable eee */
    reg_ops->write_8(REG_OFFSET_16_H(BANK_BASE_ALBANY_0), 0x7c);

    /* 10T waveform */
    reg_ops->write_8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), 0x06);
    reg_ops->write_8(REG_OFFSET_15_H(BANK_BASE_ALBANY_0), 0x00);
    reg_ops->write_8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), 0x00);
    reg_ops->write_8(REG_OFFSET_15_H(BANK_BASE_ALBANY_0), 0x00);
    /* shadow ctrl */
    reg_ops->write_8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), 0x06);
    /* tin17_s2 */
    reg_ops->write_8(REG_OFFSET_55_L(BANK_BASE_ALBANY_0), 0x19);
    /* tin18_s2 */
    reg_ops->write_8(REG_OFFSET_56_L(BANK_BASE_ALBANY_0), 0x19);
    reg_ops->write_8(REG_OFFSET_56_H(BANK_BASE_ALBANY_0), 0x19);
    /* tin19_s2 */
    reg_ops->write_8(REG_OFFSET_57_L(BANK_BASE_ALBANY_0), 0x19);
    reg_ops->write_8(REG_OFFSET_57_H(BANK_BASE_ALBANY_0), 0x19);
    /* shadow ctrl */
    reg_ops->write_8(REG_OFFSET_74_L(BANK_BASE_ALBANY_2), 0x00);
    /* tin17_s0 */
    reg_ops->write_8(REG_OFFSET_55_H(BANK_BASE_ALBANY_0), 0x28);
    reg_ops->write_8(REG_OFFSET_55_L(BANK_BASE_ALBANY_0), 0x19);

    /* Set MII Mode */
    reg_ops->write_8(REG_OFFSET_60_L(BANK_BASE_CLKGEN_0), 0x00);
    reg_ops->write_8(REG_OFFSET_60_H(BANK_BASE_CLKGEN_0), 0x00);
    reg_ops->write_8(REG_OFFSET_61_L(BANK_BASE_CLKGEN_0), 0x00);
    reg_ops->write_8(REG_OFFSET_61_H(BANK_BASE_CLKGEN_0), 0x00);
    reg_ops->write_8(REG_OFFSET_62_L(BANK_BASE_CLKGEN_0), 0x01);
    reg_ops->write_8(REG_OFFSET_62_H(BANK_BASE_CLKGEN_0), 0x00);

    /* speed up timing recovery */
    reg_ops->write_8(REG_OFFSET_7A_H(BANK_BASE_ALBANY_1), 0x02);

    /* signal_det_k */
    reg_ops->write_8(REG_OFFSET_07_H(BANK_BASE_ALBANY_1), 0xc9);

    /* snr_h */
    reg_ops->write_8(REG_OFFSET_44_H(BANK_BASE_ALBANY_1), 0x50);
    reg_ops->write_8(REG_OFFSET_45_H(BANK_BASE_ALBANY_1), 0x80);
    reg_ops->write_8(REG_OFFSET_47_L(BANK_BASE_ALBANY_1), 0x0e);
    reg_ops->write_8(REG_OFFSET_48_L(BANK_BASE_ALBANY_1), 0x04);

    /* snr check threshold define when snr locked */
    reg_ops->write_8(REG_OFFSET_49_H(BANK_BASE_ALBANY_2), 0x04);

    /* fix eee tx issue */
    reg_ops->write_8(REG_OFFSET_76_L(BANK_BASE_ALBANY_2), 0x10);

    /* 10t_8bt */
    reg_ops->write_8(REG_OFFSET_5A_L(BANK_BASE_ALBANY_0), 0x5a);
    reg_ops->write_8(REG_OFFSET_5B_L(BANK_BASE_ALBANY_0), 0x50);
    reg_ops->write_8(REG_OFFSET_7F_H(BANK_BASE_ALBANY_0), 0x1a);

    /* release snr setting after 1ms */
    reg_ops->write_8(REG_OFFSET_4A_L(BANK_BASE_ALBANY_2), 0x18);

    /* TR K1 for 28nm process */
    reg_ops->write_8(REG_OFFSET_19_H(BANK_BASE_ALBANY_1), 0x55);
    reg_ops->write_8(REG_OFFSET_08_L(BANK_BASE_ALBANY_1), 0x56);

    /* chiptop [15] allpad_in */
    reg_read = reg_ops->read_8(REG_OFFSET_50_H(BANK_BASE_CHIP));
    reg_read &= 0x7f;
    reg_ops->write_8(REG_OFFSET_50_H(BANK_BASE_CHIP), reg_read);

    /* chiptop pad_top [9:8] */
    reg_read = reg_ops->read_8(REG_OFFSET_6F_H(BANK_BASE_CHIP));
    reg_read &= 0xfe;
    reg_ops->write_8(REG_OFFSET_6F_H(BANK_BASE_CHIP), reg_read);
}

static void mtktv_mac_hw_patch(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;

    reg_ops->write_8(REG_OFFSET_1B_L(BANK_BASE_MAC_1), 0x44);
    reg_ops->write_8(REG_OFFSET_1B_H(BANK_BASE_MAC_1), 0x32);
    reg_ops->write_8(REG_OFFSET_1C_L(BANK_BASE_MAC_1), 0x05);
    reg_ops->write_8(REG_OFFSET_1C_H(BANK_BASE_MAC_1), 0x01);
}

static u32 mtktv_mac_phy_read_internal(struct net_device *ndev, u8 phy_addr, u8 reg_addr)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;

    return reg_ops->read_32(BANK_BASE_ALBANY_0 + (reg_addr << 2));
}

static u32 mtktv_mac_phy_read_external(struct net_device *ndev, u8 phy_addr, u8 reg_addr)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u32 reg_man, reg_ctl, reg_stat, reg_read;

    reg_man = ((0x60020000) | ((phy_addr & 0x1f) << 23) | (reg_addr << 18));

    /* enable mdio */
    reg_ctl = reg_ops->read_32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
    reg_ctl |= 0x10;
    reg_ops->write_32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_ctl);

    reg_ops->write_32(REG_OFFSET_1A_L(BANK_BASE_MAC_0), reg_man);

    /* wait until IDLE bit in Network Status register is cleared */
    do {
        reg_stat = reg_ops->read_32(REG_OFFSET_04_L(BANK_BASE_MAC_0));
    } while (!(reg_stat & 0x04));

    reg_read = reg_ops->read_32(REG_OFFSET_1A_L(BANK_BASE_MAC_0));
    reg_read &= 0xffff;

    /* disable mdio */
    reg_ctl &= 0xef;
    reg_ops->write_32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_ctl);

    return reg_read;
}

static void mtktv_mac_phy_write_internal(struct net_device *ndev, u8 phy_addr, u8 reg_addr, u32 val)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;

    reg_ops->write_32((BANK_BASE_ALBANY_0 + (reg_addr << 2)), val);
}

static void mtktv_mac_phy_write_external(struct net_device *ndev, u8 phy_addr, u8 reg_addr, u32 val)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u32 reg_man, reg_ctl, reg_stat;

    reg_man = ((0x50020000) | ((phy_addr & 0x1f) << 23) |
               (reg_addr << 18) | (val & 0xffff));

    /* enable mdio */
    reg_ctl = reg_ops->read_32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
    reg_ctl |= 0x10;
    reg_ops->write_32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_ctl);

    reg_ops->write_32(REG_OFFSET_1A_L(BANK_BASE_MAC_0), reg_man);

    /* wait until IDLE bit in Network Status register is cleared */
    do {
        reg_stat = reg_ops->read_32(REG_OFFSET_04_L(BANK_BASE_MAC_0));
    } while (!(reg_stat & 0x04));

    /* disable mdio */
    reg_ctl &= 0xef;
    reg_ops->write_32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_ctl);
}

static int mtktv_mac_phy_addr_scan(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u8 addr = 0;
    u32 val;

    do {
        val = reg_ops->read_phy(ndev, addr, MII_BMSR);
        if ((val != 0) && (val != 0xffff)) {
            MTKTV_PR_INFO("phy addr(%d)\n", addr);
            break;
        }
        addr++;
    } while (addr < 32);

    if (addr >= 32) {
        MTKTV_PR_ERR("bad phy addr(%d), set phy addr to 0\n", addr);
        priv->phy_addr = 0;
        return -1;
    }

    priv->phy_addr = addr;

    return 0;
}

/* checksum function */
bool mtktv_mac_rx_checksum_v1(u32 desc_val)
{
    if (((desc_val & MAC_DESC_V1_TCP ) || (desc_val & MAC_DESC_V1_UDP )) &&
        (desc_val & MAC_DESC_V1_IP_CSUM) &&
        (desc_val & MAC_DESC_V1_TCP_UDP_CSUM))
        return true;
    else
        return false;
}

bool mtktv_mac_rx_checksum_v2(u32 desc_val)
{
    if (!(desc_val & MAC_DESC_V2_PKT_TYPE_BIT2) &&
        !(desc_val & MAC_DESC_V2_PKT_TYPE_BIT1) &&
        !(desc_val & MAC_DESC_V2_PKT_TYPE_BIT0)) {
        /* not ip packet */
        return false;
    } else {
        /* ip packet */
        if (!(desc_val & MAC_DESC_V2_IP_CSUM)) {
            return false;
        } else {
            /* ip checksum ok */
            if ((!(desc_val & MAC_DESC_V2_PKT_TYPE_BIT2) &&
                 (desc_val & MAC_DESC_V2_PKT_TYPE_BIT1) &&
                 (desc_val & MAC_DESC_V2_PKT_TYPE_BIT0)) ||
                ((desc_val & MAC_DESC_V2_PKT_TYPE_BIT2) &&
                 (desc_val & MAC_DESC_V2_PKT_TYPE_BIT1) &&
                 !(desc_val & MAC_DESC_V2_PKT_TYPE_BIT0))) {
                /* not tcp or udp */
                return true;
            } else {
                /* tcp or udp */
                if (!(desc_val & MAC_DESC_V2_TCP_UDP_CSUM))
                    return false;
                else
                    return true;
            }
        }
    }
}

/* packet data dump for debug usage */
static void mtktv_mac_skb_dump(phys_addr_t addr, u32 len)
{
    u8 *ptr = (u8 *)addr;
    u32 i;

    printk(KERN_ERR "===== Dump %lx, len %d(%02x) =====\n",
           (long unsigned int)ptr, len, len);
    printk(KERN_ERR "              00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f\n");
    for (i = 0; i < len; i++) {
        printk(KERN_ERR "%lx(%02x): %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x\n",
               (long unsigned int)ptr, i,
               *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5), *(ptr + 6), *(ptr + 7),
               *(ptr + 8), *(ptr + 9), *(ptr + 10), *(ptr + 11), *(ptr + 12), *(ptr + 13), *(ptr + 14), *(ptr + 15));
        ptr += 16;
        i += 15;
    }
    printk(KERN_ERR "\n");
}

static int __init mtktv_mac_mac_addr_auto_setup(char *addrs)
{
    u8 mac_str[20] = {};

    strncpy(mac_str, addrs, strlen(addrs));
    if ((strlen(mac_str) == strlen(MACADDR_FORMAT)) &&
        (':' == mac_str[2]) &&
        (':' == mac_str[5]) &&
        (':' == mac_str[8]) &&
        (':' == mac_str[11]) &&
        (':' == mac_str[14])) {
        mac_str[2]  = '\0';
        mac_str[5]  = '\0';
        mac_str[8]  = '\0';
        mac_str[11] = '\0';
        mac_str[14] = '\0';

        mac_addr_env[0] = (u8)simple_strtoul(&(mac_str[0]),  NULL, 16);
        mac_addr_env[1] = (u8)simple_strtoul(&(mac_str[3]),  NULL, 16);
        mac_addr_env[2] = (u8)simple_strtoul(&(mac_str[6]),  NULL, 16);
        mac_addr_env[3] = (u8)simple_strtoul(&(mac_str[9]),  NULL, 16);
        mac_addr_env[4] = (u8)simple_strtoul(&(mac_str[12]), NULL, 16);
        mac_addr_env[5] = (u8)simple_strtoul(&(mac_str[15]), NULL, 16);
    }

    MTKTV_PR_DEBUG("mac_addr_env=%pM\n", mac_addr_env);

    return 1;
}
__setup("macaddr=", mtktv_mac_mac_addr_auto_setup);

static void mtktv_mac_hw_mac_addr_set(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u32 val, reg_read;

    val = ((ndev->dev_addr[3] << 24) | (ndev->dev_addr[2] << 16) |
           (ndev->dev_addr[1] << 8) | (ndev->dev_addr[0]));
    reg_ops->write_32(REG_OFFSET_4C_L(BANK_BASE_MAC_0), val);
    reg_read = reg_ops->read_32(REG_OFFSET_4E_L(BANK_BASE_MAC_0));
    val = (((ndev->dev_addr[5] << 8) | (ndev->dev_addr[4])) & 0xffff) |
           (reg_read & 0xffff0000);
    reg_ops->write_32(REG_OFFSET_4E_L(BANK_BASE_MAC_0), val);

    MTKTV_PR_DEBUG("set ndev->dev_addr=%pM\n", ndev->dev_addr);
}

static void mtktv_mac_mac_addr_get(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u32 mac_addr_h, mac_addr_l;
    u8 addr[6] = {};

    mac_addr_l = reg_ops->read_32(REG_OFFSET_4C_L(BANK_BASE_MAC_0));
    mac_addr_h = reg_ops->read_32(REG_OFFSET_4E_L(BANK_BASE_MAC_0));

    addr[0] = (mac_addr_l & 0xff);
    addr[1] = (mac_addr_l & 0xff00) >> 8;
    addr[2] = (mac_addr_l & 0xff0000) >> 16;
    addr[3] = (mac_addr_l & 0xff000000) >> 24;
    addr[4] = (mac_addr_h & 0xff);
    addr[5] = (mac_addr_h & 0xff00) >> 8;

    if (is_valid_ether_addr(addr)) {
        ether_addr_copy(ndev->dev_addr ,addr);
        MTKTV_PR_DEBUG("ndev->dev_addr=%pM, sa1=%pM\n", ndev->dev_addr, addr);
        return;
    }

    mac_addr_l = reg_ops->read_32(REG_OFFSET_50_L(BANK_BASE_MAC_0));
    mac_addr_h = reg_ops->read_32(REG_OFFSET_52_L(BANK_BASE_MAC_0));

    addr[0] = (mac_addr_l & 0xff);
    addr[1] = (mac_addr_l & 0xff00) >> 8;
    addr[2] = (mac_addr_l & 0xff0000) >> 16;
    addr[3] = (mac_addr_l & 0xff000000) >> 24;
    addr[4] = (mac_addr_h & 0xff);
    addr[5] = (mac_addr_h & 0xff00) >> 8;

    if (is_valid_ether_addr(addr)) {
        ether_addr_copy(ndev->dev_addr ,addr);
        MTKTV_PR_DEBUG("ndev->dev_addr=%pM, sa2=%pM\n", ndev->dev_addr, addr);
        return;
    }

    mac_addr_l = reg_ops->read_32(REG_OFFSET_54_L(BANK_BASE_MAC_0));
    mac_addr_h = reg_ops->read_32(REG_OFFSET_56_L(BANK_BASE_MAC_0));

    addr[0] = (mac_addr_l & 0xff);
    addr[1] = (mac_addr_l & 0xff00) >> 8;
    addr[2] = (mac_addr_l & 0xff0000) >> 16;
    addr[3] = (mac_addr_l & 0xff000000) >> 24;
    addr[4] = (mac_addr_h & 0xff);
    addr[5] = (mac_addr_h & 0xff00) >> 8;

    if (is_valid_ether_addr(addr)) {
        ether_addr_copy(ndev->dev_addr ,addr);
        MTKTV_PR_DEBUG("ndev->dev_addr=%pM, sa3=%pM\n", ndev->dev_addr, addr);
        return;
    }

    mac_addr_l = reg_ops->read_32(REG_OFFSET_58_L(BANK_BASE_MAC_0));
    mac_addr_h = reg_ops->read_32(REG_OFFSET_5A_L(BANK_BASE_MAC_0));

    addr[0] = (mac_addr_l & 0xff);
    addr[1] = (mac_addr_l & 0xff00) >> 8;
    addr[2] = (mac_addr_l & 0xff0000) >> 16;
    addr[3] = (mac_addr_l & 0xff000000) >> 24;
    addr[4] = (mac_addr_h & 0xff);
    addr[5] = (mac_addr_h & 0xff00) >> 8;

    if (is_valid_ether_addr(addr)) {
        ether_addr_copy(ndev->dev_addr ,addr);
        MTKTV_PR_DEBUG("ndev->dev_addr=%pM, sa4=%pM\n", ndev->dev_addr, addr);
        return;
    }

    if (is_valid_ether_addr(mac_addr_env)) {
        ether_addr_copy(ndev->dev_addr ,mac_addr_env);
        MTKTV_PR_DEBUG("ndev->dev_addr=%pM, mac_addr_env=%pM\n",
                       ndev->dev_addr, mac_addr_env);
        return;
    }

    if (is_valid_ether_addr(mac_addr_dts)) {
        ether_addr_copy(ndev->dev_addr ,mac_addr_dts);
        MTKTV_PR_DEBUG("ndev->dev_addr=%pM, mac_addr_env=%pM\n",
                       ndev->dev_addr, mac_addr_dts);
        return;
    }

    eth_hw_addr_random(ndev);
    MTKTV_PR_INFO("use random mac address(%pM)\n", ndev->dev_addr);
}

/* phy link status */
static bool mtktv_mac_link_status_update(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u32 bmsr, bmcr, lpa, adv, neg, speed, duplex, reg;
    u32 hcd_link_st_ok, an_100t_link_st;

    /* latch link status bit to 1 */
    bmsr = reg_ops->read_phy(ndev, priv->phy_addr, MII_BMSR);
    bmsr |= 0x4UL;
    reg_ops->write_phy(ndev, priv->phy_addr, MII_BMSR, bmsr);
    bmsr = reg_ops->read_phy(ndev, priv->phy_addr, MII_BMSR);
    /* check hcd link status */
    hcd_link_st_ok = reg_ops->read_phy(ndev, priv->phy_addr, 0x21);
    if (!(hcd_link_st_ok & 0x100UL)) {
        /* link down */
        if (netif_carrier_ok(ndev)) {
            netif_carrier_off(ndev);
        }

        return false;
    }

    bmcr = reg_ops->read_phy(ndev, priv->phy_addr, MII_BMCR);

    if (bmcr & BMCR_ANENABLE) {
        /* auto-negotiation */
        if (!(bmsr & BMSR_ANEGCOMPLETE)) {
            MTKTV_PR_INFO("auto-negotiation still running\n");
            if (netif_carrier_ok(ndev)){
                netif_carrier_off(ndev);
            }

            return false;
        }

        /* get link partner and advertisement from the phy not from the mac */
        adv = reg_ops->read_phy(ndev, priv->phy_addr, MII_ADVERTISE);
        lpa = reg_ops->read_phy(ndev, priv->phy_addr, MII_LPA);

        /* for link parterner adopts force mode and ephy used,
         * ephy lpa reveals all zero value.
         * ephy would be forced to full-duplex mode.
         */
        if (!lpa) {
            /* 100Mbps full-duplex */
            if (bmcr & BMCR_SPEED100)
                lpa |= LPA_100FULL;
            else /* 10Mbps full-duplex */
                lpa |= LPA_10FULL;
        }

        neg = (adv & lpa);

        if (neg & LPA_100FULL) {
            speed = SPEED_100;
            duplex = DUPLEX_FULL;
        } else if (neg & LPA_100HALF) {
            speed = SPEED_100;
            duplex = DUPLEX_HALF;
        } else if (neg & LPA_10FULL) {
            speed = SPEED_10;
            duplex = DUPLEX_FULL;
        } else if (neg & LPA_10HALF) {
            speed = SPEED_10;
            duplex = DUPLEX_HALF;
        } else {
            speed = SPEED_10;
            duplex = DUPLEX_HALF;
            MTKTV_PR_INFO("no speed and mode found (lpa=0x%x, adv=0x%x)\n",
                          lpa, adv);
        }
    } else {
        speed = (bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10;
        duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
    }

    /* get status from mac */
    reg = reg_ops->read_32(REG_OFFSET_02_L(BANK_BASE_MAC_0));
    if (speed == SPEED_100)
        reg |= 0x1UL;
    else
        reg &= ~(0x1UL);

    if (duplex == DUPLEX_FULL)
        reg |= 0x2UL;
    else
        reg &= ~(0x2UL);

    /* update status to mac */
    reg_ops->write_32(REG_OFFSET_02_L(BANK_BASE_MAC_0), reg);

    /* link up */
    if (!netif_carrier_ok(ndev)) {
        netif_carrier_on(ndev);
    }

    /* phy restart patch */
    if (speed == SPEED_100) {
        hcd_link_st_ok = reg_ops->read_phy(ndev, priv->phy_addr, 0x21);
        an_100t_link_st = reg_ops->read_phy(ndev, priv->phy_addr, 0x22);
        if (((hcd_link_st_ok & 0x100) && !(an_100t_link_st & 0x300)) ||
            (!(hcd_link_st_ok & 0x100) && ((an_100t_link_st & 0x300) == 0x200))) {
            priv->phy_restart_cnt++;
            if (priv->phy_restart_cnt > 10) {
                MTKTV_PR_INFO("restart auto-negotiation\n");
                reg_ops->write_phy(ndev, priv->phy_addr, MII_BMCR, 0x1200UL);
                priv->phy_restart_cnt = 0;
            }
        } else {
            priv->phy_restart_cnt = 0;
        }
    }

    return true;
}

static void mtktv_mac_link_timer_callback(unsigned long data)
{
    struct net_device *ndev = (struct net_device *)data;
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    bool ret;
    static u32 cnt = 0;

    ret = mtktv_mac_link_status_update(ndev);
    if (ret == true) {
        /* link up */
        priv->link_timer.expires = jiffies + MAC_UPDATE_LINK_TIME;
        cnt = 0;
    } else {
        /* link down */
        if (cnt < 10) {
            /* check phy status quickly */
            priv->link_timer.expires = jiffies + (MAC_UPDATE_LINK_TIME / 10);
            cnt++;
        } else {
            /* check phy status normally */
            priv->link_timer.expires = jiffies + MAC_UPDATE_LINK_TIME;
        }
    }

    add_timer(&priv->link_timer);
}

#if 0 /* tx descriptor */
static void mtktv_mac_tx_desc_reset(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);

    priv->tx_desc_write_index = 0;
    priv->tx_desc_read_index = 0;
    priv->tx_desc_queued_number = 0;
    priv->tx_desc_count = 0;
    priv->tx_desc_full_count = 0;
}

static void mtktv_mac_tx_desc_mode_set(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u32 val1,val2;

    memset((void *)priv->tx_desc_vir_addr, 0x0, (priv->tx_desc_num * priv->tx_desc_size));
    skb_queue_purge(&priv->tx_skb_q);
    priv->tx_desc = (struct tx_desc *) priv->tx_desc_vir_addr;
    /* set tx low priority descriptor address */
    reg_ops->write_32(REG_OFFSET_2F_L(BANK_BASE_MAC_1), (priv->tx_desc_dma_addr & 0xffffUL));
    reg_ops->write_32(REG_OFFSET_30_L(BANK_BASE_MAC_1), ((priv->tx_desc_dma_addr & 0xffff0000UL) >> 16));
    val1 = reg_ops->read_32(REG_OFFSET_2F_L(BANK_BASE_MAC_1));
    val2 = reg_ops->read_32(REG_OFFSET_30_L(BANK_BASE_MAC_1));
    MTKTV_PR_INFO("mtktv_mac_tx_desc_mode_set emac1 0x2f=0x%lx 0x30=0x%lx\n", val1, val2);
    /* set tx threshold and enable it */
    reg_ops->write_8(REG_OFFSET_33_L(BANK_BASE_MAC_1), (priv->tx_desc_num & 0xff));
    reg_ops->write_8(REG_OFFSET_33_H(BANK_BASE_MAC_1), (((priv->tx_desc_num & 0x3f00) >> 8) | 0x80));


}

static void mtktv_mac_tx_desc_clear(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;

    skb_queue_purge(&priv->tx_skb_q);
    reg_ops->write_8(REG_OFFSET_33_L(BANK_BASE_MAC_1), 0);
    reg_ops->write_8(REG_OFFSET_33_H(BANK_BASE_MAC_1), 0);
}

static u32 mtktv_mac_tx_desc_free_skb(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    struct tx_desc *ptx;
    struct sk_buff *skb;
    u32 free_sk_buffer_number, hw_queue_number, free_count, flags;

    if(!priv->tx_desc_start_free)
        return 0;

    spin_lock_irqsave(&priv->tx_lock, flags);

    free_count = 0;
    hw_queue_number = ((reg_ops->read_32(REG_OFFSET_32_L(BANK_BASE_MAC_1))) & 0x3FFFUL);

    if(priv->tx_desc_queued_number > (8 + hw_queue_number))
        free_sk_buffer_number = priv->tx_desc_queued_number - hw_queue_number - 8;
    else
        free_sk_buffer_number = 0;

    while (free_sk_buffer_number > free_count) {
        ptx = priv->tx_desc + priv->tx_desc_read_index;
        skb = __skb_dequeue(&priv->tx_skb_q);

        dev_kfree_skb_any(skb);
//MTKTV_PR_INFO("mtktv_mac_tx_desc_free_skb freenum(%d) freecnt(%d) readidx(%d) descnum(%d) qednum(%d)\n", free_sk_buffer_number, free_count, priv->tx_desc_read_index, priv->tx_desc_num, priv->tx_desc_queued_number);
        //priv->stats.tx_bytes += (ptx->low_tag & 0x3FFFUL);
        priv->tx_desc_read_index++;
        if(priv->tx_desc_read_index >= priv->tx_desc_num) {
            priv->tx_desc_read_index = 0;
        }

        priv->tx_desc_queued_number--;

        free_count++;
    }
    spin_unlock_irqrestore(&priv->tx_lock, flags);
    //MHal_GMAC_Write_IDR(GMAC_INT_TCOM);

    return free_count;
}
#endif /* tx descriptor */






static void mtktv_mac_mem_init(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    struct rx_desc *prx;
    struct sk_buff *skb;
    int i;
    u32 size;
    dma_addr_t dma_addr, rx_desc_dma_addr;

    /* tx */
    priv->tx_memcpy_index = 0;
    memset((void *)priv->tx_memcpy_vir_addr, 0x0,
           (TX_MEMCPY_SIZE_DEFAULT * TX_MEMCPY_NUM_DEFAULT));

    /* rx */
    /* set descriptor pointer */
    rx_desc_dma_addr = priv->rx_desc_dma_addr - MIU0_BUS_BASE;
    reg_ops->write_32(REG_OFFSET_0C_L(BANK_BASE_MAC_0), rx_desc_dma_addr);

    memset((void *)priv->rx_desc_vir_addr, 0x0,
           (priv->rx_desc_num * priv->rx_desc_size));
    skb_queue_purge(&priv->rx_skb_q);
    priv->rx_desc_index = 0;
    priv->rx_desc = (struct rx_desc *) priv->rx_desc_vir_addr;

    for (i = 0; i < priv->rx_desc_num; i++) {
        prx = priv->rx_desc + i;
        /* create skb */
        size = MAC_RX_MAX_LEN + MAC_EXTRA_PKT_LEN;
        skb = netdev_alloc_skb(ndev, size);
        if (!skb) {
            MTKTV_PR_ERR("allocate skb fail at %d\n", i);
            prx--;
            break;
        }
        //skb_reserve(skb, NET_IP_ALIGN);
        //skb_queue_tail(&priv->rx_skb_q, skb);
        dma_addr = dma_map_single(ndev->dev.parent, skb->data, size, DMA_FROM_DEVICE);
        if (dma_mapping_error(ndev->dev.parent, dma_addr)) {
            MTKTV_PR_ERR("dma map fail at %d\n", i);
            prx--;
            break;
        }

        skb_queue_tail(&priv->rx_skb_q, skb);

        dma_addr -= MIU0_BUS_BASE;

        if (dma_addr >= MIU1_BUS_BASE) {
            printk(KERN_EMERG "\x1b[37;41m [%s %d] dma_addr >= MIU1_BUS_BASE\x1b[0m\n ", __FUNCTION__, __LINE__);
            dma_addr |= 0x80000000UL;
        }

        prx->addr = dma_addr;
        wmb();
    }
    /* last descriptor */
    prx->addr |= 0x2UL;
    wmb();
}

static void mtktv_mac_mem_release(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);

    /* tx */
    priv->tx_memcpy_index = 0;
    memset((void *)priv->tx_memcpy_vir_addr, 0x0,
           (TX_MEMCPY_SIZE_DEFAULT * TX_MEMCPY_NUM_DEFAULT));

    /* rx */
    memset((void *)priv->rx_desc_vir_addr, 0x0,
           (priv->rx_desc_num * priv->rx_desc_size));
    skb_queue_purge(&priv->rx_skb_q);
    priv->rx_desc_index = 0;
}

static int mtktv_mac_hw_init(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    int ret;
    u32 reg_read;

    mtktv_mac_hw_power_on_clk(ndev);

    if (priv->is_internal_phy == true) {
        /* 0xf011 */
        reg_ops->write_8(REG_OFFSET_00_L(BANK_BASE_MAC_1), 0x11);
        reg_ops->write_8(REG_OFFSET_00_H(BANK_BASE_MAC_1), 0xf0);
    } else {
        /* 0xf017 */
        reg_ops->write_8(REG_OFFSET_00_L(BANK_BASE_MAC_1), 0x17);
        reg_ops->write_8(REG_OFFSET_00_H(BANK_BASE_MAC_1), 0xf0);
    }

    ret = mtktv_mac_phy_addr_scan(ndev);
    if (ret)
        return -1;

    /* 0x8f */
    reg_ops->write_8(REG_OFFSET_01_L(BANK_BASE_MAC_1), 0x00);
    /* 0x8f */
    reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), 0x8f);

    /* delay interrupt: 0x0402 */
    reg_ops->write_8(REG_OFFSET_03_L(BANK_BASE_MAC_1), 0x02);
    reg_ops->write_8(REG_OFFSET_03_H(BANK_BASE_MAC_1), 0x04);

    /* mac address */
    mtktv_mac_mac_addr_get(ndev);
    mtktv_mac_hw_mac_addr_set(ndev);

    /* default speed-duplex 100-full */
    reg_read = reg_ops->read_32(REG_OFFSET_02_L(BANK_BASE_MAC_0));
    reg_read |= 0x03;
    reg_ops->write_32(REG_OFFSET_02_L(BANK_BASE_MAC_0), reg_read);

    /* hw patch */
    mtktv_mac_hw_patch(ndev);

    return 0;
}

static int mtktv_mac_rx(struct net_device *ndev, int loop)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    struct rx_desc *prx;
    struct sk_buff *skb, *skb_new;
    dma_addr_t dma_addr, dma_addr_new;
    u32 len, size;
    int i, rx_pkt_cnt = 0;

    for (i = 0; i < loop; i++) {
        prx = priv->rx_desc + priv->rx_desc_index;
        dma_addr = prx->addr;
        if (dma_addr & RX_DESC_DONE) {
            len = (prx->size & 0x7ffUL);
            if ((len > MAC_RX_MAX_LEN) || (len < 64)) {
                MTKTV_PR_ERR("packet length error(%d)\n", len);
                prx->addr &= ~(0x1UL);
                ndev->stats.rx_length_errors++;
                goto rx_end;
            }
            len -= 4;
            skb = skb_dequeue(&priv->rx_skb_q);
            if (skb) {
                /* create skb */
                size = MAC_RX_MAX_LEN + MAC_EXTRA_PKT_LEN;
                skb_new = netdev_alloc_skb(ndev, size);
                if (!skb_new) {
                    MTKTV_PR_ERR("allocate skb fail, use old skb\n");
                    skb_queue_head(&priv->rx_skb_q, skb);
                    prx->addr &= ~(0x1UL);
                    ndev->stats.rx_missed_errors++;
                    goto rx_end;
                }
                //skb_reserve(skb, NET_IP_ALIGN);
                skb_queue_tail(&priv->rx_skb_q, skb_new);
                dma_addr_new = dma_map_single(ndev->dev.parent, skb_new->data, size,
                                              DMA_FROM_DEVICE);
                if (dma_mapping_error(ndev->dev.parent, dma_addr_new)) {
                    MTKTV_PR_ERR("dma map fail, use old skb\n");
                    skb_dequeue_tail(&priv->rx_skb_q);
                    skb_queue_head(&priv->rx_skb_q, skb);
                    dev_kfree_skb_any(skb_new);
                    prx->addr &= ~(0x1UL);
                    ndev->stats.rx_missed_errors++;
                    goto rx_end;
                }

                if (dma_addr_new >= MIU1_BUS_BASE) {
                    printk(KERN_EMERG "\x1b[37;41m [%s %d] dma_addr >= MIU1_BUS_BASE\x1b[0m\n ", __FUNCTION__, __LINE__);
                    dma_addr_new |= 0x80000000UL;
                }

                dma_addr_new -= MIU0_BUS_BASE;

                prx->addr = dma_addr_new;
                wmb();
                if (priv->rx_desc_index >= (priv->rx_desc_num - 1)) {
                    /* last descriptor */
                    prx->addr |= 0x2UL;
                    wmb();
                }
                /* net rx process */
                dma_addr += MIU0_BUS_BASE;
                dma_unmap_single(ndev->dev.parent, dma_addr, size, DMA_FROM_DEVICE);
                //MTKTV_PR_ERR("debug data_new(0x%p), dma_new(0x%p), data(0x%p), dma(0x%p)\n", skb_new->data, dma_addr_new, skb->data, dma_addr);
                skb_put(skb, len);
                //MTKTV_PR_INFO("start recv:\n");
                //mtktv_mac_skb_dump(skb->data, len);
                if (reg_ops->rx_checksum(prx->size) == true) {
                    skb->ip_summed = CHECKSUM_UNNECESSARY;
                    //MTKTV_PR_ERR("CHECKSUM_UNNECESSARY (0x%x)\n", prx->size);
                } else {
                    skb->ip_summed = CHECKSUM_NONE;
                    //MTKTV_PR_ERR("CHECKSUM_NONE (0x%x)\n", prx->size);
                }
                skb->protocol = eth_type_trans(skb, ndev);
                netif_receive_skb(skb);
                rx_pkt_cnt++;
                ndev->stats.rx_packets++;
                ndev->stats.rx_bytes += len;
            } else {
                MTKTV_PR_ERR("skb from queue is bad, what the hell\n");
                ndev->stats.rx_missed_errors++;
            }

rx_end:
            priv->rx_desc_index++;
            if (priv->rx_desc_index >= priv->rx_desc_num)
                priv->rx_desc_index = 0;
        } else {
            break;
        }
    }

    return rx_pkt_cnt;
}

static int mtktv_mac_napi_poll_rx(struct napi_struct *napi, int budget)
{
    struct mtktv_mac_private *priv = container_of(napi, struct mtktv_mac_private, napi_rx);
    struct net_device *ndev = priv->ndev;
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    int work_done;
    u8 reg_irq;

    work_done = mtktv_mac_rx(ndev, budget);

    if (work_done < budget) {
        local_irq_disable();
        napi_complete(napi);
        /* enable rx irq */
        reg_irq = reg_ops->read_8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
        reg_irq |= 0x80;
        reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);
        local_irq_enable();
    }

    return work_done;
}

static irqreturn_t mtktv_mac_isr(int irq, void *dev_id)
{
    struct net_device *ndev = (struct net_device *)dev_id;
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u32 intr_stat, rx_stat, tx_stat;
    //dma_addr_t rx_desc_dma_addr;
    u8 delay_intr_stat, reg_irq;

    /* reg is read clear, read delay intr first */
    delay_intr_stat = reg_ops->read_8(REG_OFFSET_04_H(BANK_BASE_MAC_1));
    intr_stat = reg_ops->read_32(REG_OFFSET_12_L(BANK_BASE_MAC_0));
    rx_stat = reg_ops->read_32(REG_OFFSET_10_L(BANK_BASE_MAC_0));
    tx_stat = reg_ops->read_32(REG_OFFSET_0A_L(BANK_BASE_MAC_0));

    //MTKTV_PR_INFO("mtktv_mac_isr, intr_stat(0x%x), rx_stat(0x%x), tx_stat(0x%x), delay_intr_stat(0x%x)\n", intr_stat, rx_stat, tx_stat, delay_intr_stat);

    if (intr_stat & IRQ_BIT_RBNA) {
        //MTKTV_PR_ERR("RBNA, isr(0x%x), rsr(0x%x), tsr(0x%x), delay(0x%x)\n",
        //             intr_stat, rx_stat, tx_stat, delay_intr_stat);
        //rx_desc_dma_addr = priv->rx_desc_dma_addr - MIU0_BUS_BASE;
        //reg_ops->write_32(REG_OFFSET_0C_L(BANK_BASE_MAC_0), rx_desc_dma_addr);
        ndev->stats.rx_fifo_errors++;
        if (rx_stat & IRQ_RSR_DNA)
            reg_ops->write_32(REG_OFFSET_10_L(BANK_BASE_MAC_0), IRQ_RSR_DNA);
        if (rx_stat & IRQ_RSR_BNA)
            reg_ops->write_32(REG_OFFSET_10_L(BANK_BASE_MAC_0), IRQ_RSR_BNA);
    }

    if (intr_stat & IRQ_BIT_ROVR) {
        //MTKTV_PR_ERR("ROVR, isr(0x%x), rsr(0x%x), tsr(0x%x), delay(0x%x)\n",
        //             intr_stat, rx_stat, tx_stat, delay_intr_stat);
        ndev->stats.rx_over_errors++;
        if (rx_stat & IRQ_RSR_OVR_RSR)
            reg_ops->write_32(REG_OFFSET_10_L(BANK_BASE_MAC_0), IRQ_RSR_OVR_RSR);
    }

    if (intr_stat & IRQ_BIT_TUND) {
        //MTKTV_PR_ERR("TUND, isr(0x%x), rsr(0x%x), tsr(0x%x), delay(0x%x)\n",
        //             intr_stat, rx_stat, tx_stat, delay_intr_stat);
        ndev->stats.tx_fifo_errors++;
        if (tx_stat & IRQ_TSR_UND)
            reg_ops->write_32(REG_OFFSET_0A_L(BANK_BASE_MAC_0), IRQ_TSR_UND);
    }

    if (intr_stat & IRQ_BIT_TOVR) {
        //MTKTV_PR_ERR("TOVR, isr(0x%x), rsr(0x%x), tsr(0x%x), delay(0x%x)\n",
        //             intr_stat, rx_stat, tx_stat, delay_intr_stat);
        ndev->stats.tx_fifo_errors++;
        if (tx_stat & IRQ_TSR_OVER)
            reg_ops->write_32(REG_OFFSET_0A_L(BANK_BASE_MAC_0), IRQ_TSR_OVER);
    }

    if (intr_stat & IRQ_BIT_RTRY) {
        //MTKTV_PR_ERR("RTRY, isr(0x%x), rsr(0x%x), tsr(0x%x), delay(0x%x)\n",
        //             intr_stat, rx_stat, tx_stat, delay_intr_stat);
        ndev->stats.tx_errors++;
        if (tx_stat & IRQ_TSR_RLE)
            reg_ops->write_32(REG_OFFSET_0A_L(BANK_BASE_MAC_0), IRQ_TSR_RLE);
    }

    if (delay_intr_stat & 0x80) {
        /* do rx */
        /* napi */
        //MTKTV_PR_INFO("mtktv_mac_isr rx\n");
        if (likely(napi_schedule_prep(&priv->napi_rx))) {
            //MTKTV_PR_INFO("mtktv_mac_isr rx napi\n");
            /* disable rx irq */
            reg_irq = reg_ops->read_8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
            reg_irq &= 0x7f;
            reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);
            __napi_schedule(&priv->napi_rx);
        }
    }

#if 0 /* tx descriptor */
    mtktv_mac_tx_desc_free_skb(ndev);
#endif /* tx descriptor */

    return IRQ_HANDLED;
}

static int mtktv_mac_open(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u8 reg_irq;
    u32 reg_trx;

    MTKTV_PR_INFO("mtktv_mac_open\n");
    netif_carrier_off(ndev);

#if 0 /* tx descriptor */
    priv->tx_desc_start_free = 1;
#endif /* tx descriptor */

    /* memory init */
    mtktv_mac_mem_init(ndev);
    /* enable napi */
    napi_enable(&priv->napi_rx);

    /* disable all irq first */
    reg_ops->write_32(REG_OFFSET_16_L(BANK_BASE_MAC_0), IRQ_DISABLE_ALL);
    reg_irq = reg_ops->read_8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
    reg_irq &= 0x7f;
    reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);
    /* enable irq */
    reg_ops->write_32(REG_OFFSET_14_L(BANK_BASE_MAC_0), IRQ_ENABLE_BIT);
    reg_irq = reg_ops->read_8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
    reg_irq |= 0x80;
    reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);

    /* enable hw tx rx */
    reg_trx = reg_ops->read_32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
    reg_trx |= 0x0c;
    reg_ops->write_32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_trx);

    mtktv_mac_link_status_update(ndev);
    add_timer(&priv->link_timer);
    netif_start_queue(ndev);

    return 0;
}

static int mtktv_mac_stop(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u8 reg_irq;
    u32 reg_trx;

    MTKTV_PR_INFO("mtktv_mac_stop\n");
    netif_stop_queue(ndev);
    del_timer(&priv->link_timer);
    netif_carrier_off(ndev);

    /* disable hw tx rx */
    reg_trx = reg_ops->read_32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
    reg_trx &= 0xfffffff3;
    reg_ops->write_32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_trx);

    /* disable all irq first */
    reg_ops->write_32(REG_OFFSET_16_L(BANK_BASE_MAC_0), IRQ_DISABLE_ALL);
    reg_irq = reg_ops->read_8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
    reg_irq &= 0x7f;
    reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);

    /* disable napi */
    napi_disable(&priv->napi_rx);

    /* memory release */
    mtktv_mac_mem_release(ndev);

#if 0 /* tx descriptor */
    priv->tx_desc_start_free = 0;
#endif /* tx descriptor */

    return 0;
}

static int mtktv_mac_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    uintptr_t skb_addr;
    dma_addr_t dma_addr, tx_memcpy_dma_addr;
    u32 tsr_val, flags;
    u8 tx_fifo[8] = {0};
    u8 i, tx_fifo_token = 0;

    spin_lock_irqsave(&priv->tx_lock, flags);
    if (skb->len > MAC_TX_MAX_LEN) {
        MTKTV_PR_ERR("bad tx length(%d)\n", skb->len);
        spin_unlock_irqrestore(&priv->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    tsr_val = reg_ops->read_32(REG_OFFSET_0A_L(BANK_BASE_MAC_0));

    tx_fifo[0] = (((tsr_val & IRQ_TSR_IDLE) != 0) ? 1 : 0);
    tx_fifo[1] = (((tsr_val & IRQ_TSR_BNQ) != 0) ? 1 : 0);
    tx_fifo[2] = (((tsr_val & IRQ_TSR_TBNQ) != 0) ? 1 : 0);
    tx_fifo[3] = (((tsr_val & IRQ_TSR_FBNQ) != 0) ? 1 : 0);
    tx_fifo[4] = (((tsr_val & IRQ_TSR_FIFO1_IDLE) != 0) ? 1 : 0);
    tx_fifo[5] = (((tsr_val & IRQ_TSR_FIFO2_IDLE) != 0) ? 1 : 0);
    tx_fifo[6] = (((tsr_val & IRQ_TSR_FIFO3_IDLE) != 0) ? 1 : 0);
    tx_fifo[7] = (((tsr_val & IRQ_TSR_FIFO4_IDLE) != 0) ? 1 : 0);

    for (i = 0; i < 8; i++) {
        tx_fifo_token += tx_fifo[i];
    }

    if (tx_fifo_token <= 4) {
        spin_unlock_irqrestore(&priv->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    skb_addr = priv->tx_memcpy_vir_addr + (TX_MEMCPY_SIZE_DEFAULT * priv->tx_memcpy_index);
    if (!skb_addr) {
        MTKTV_PR_ERR("bad skb_addr\n");
        spin_unlock_irqrestore(&priv->tx_lock, flags);
        return -ENOMEM;
    }
    memcpy((void*)skb_addr, skb->data, skb->len);

    tx_memcpy_dma_addr = priv->tx_memcpy_dma_addr - MIU0_BUS_BASE;

    dma_addr = tx_memcpy_dma_addr + (TX_MEMCPY_SIZE_DEFAULT * priv->tx_memcpy_index);
    //MTKTV_PR_ERR("tx packet dma: 0x%p\n", dma_addr);
    //mtktv_mac_skb_dump(skb_addr, skb->len);

    reg_ops->write_32(REG_OFFSET_06_L(BANK_BASE_MAC_0), dma_addr);
    reg_ops->write_32(REG_OFFSET_08_L(BANK_BASE_MAC_0), skb->len);

    priv->tx_memcpy_index++;
    if (priv->tx_memcpy_index >= TX_MEMCPY_NUM_DEFAULT)
        priv->tx_memcpy_index = 0;

    ndev->stats.tx_bytes += skb->len;
    ndev->stats.tx_packets++;

    dev_kfree_skb_any(skb);

    spin_unlock_irqrestore(&priv->tx_lock, flags);
    return NETDEV_TX_OK;

#if 0 /* tx descriptor */
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u32 dma_addr, flags, addr, len;
    struct tx_desc *ptx;

    //MTKTV_PR_INFO("start xmit before dma:\n");
//mtktv_mac_skb_dump(skb->data, skb->len);
    if (skb->len > MAC_TX_MAX_LEN) {
        MTKTV_PR_ERR("bad tx length(%d)\n", skb->len);
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    if (reg_ops->read_8(REG_OFFSET_32_H(BANK_BASE_MAC_1)) & 0x80) {
        MTKTV_PR_ERR("tx descriptor overrun\n");
        netif_stop_queue(ndev);
        //MDev_EMAC_TX_Free_sk_buff(ndev, 0);
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    mtktv_mac_tx_desc_free_skb(ndev);
    spin_lock_irqsave(&priv->tx_lock, flags);

    __skb_queue_tail(&priv->tx_skb_q, skb);
        //MTKTV_PR_INFO("start xmit before dma 2:\n");
//mtktv_mac_skb_dump(skb->data, skb->len);
    dma_addr = dma_map_single(ndev->dev.parent, skb->data, skb->len, DMA_TO_DEVICE);
    if (unlikely(dma_mapping_error(ndev->dev.parent, dma_addr))) {
        MTKTV_PR_ERR("tx dma_mapping_error\n");
        ndev->stats.tx_dropped++;
        goto out_tx;
    }
    ptx = priv->tx_desc + priv->tx_desc_write_index;

    ptx->addr = dma_addr;
    wmb();


    if(priv->tx_desc_write_index >= (priv->tx_desc_num - 1)) {
        ptx->low_tag = ((skb->len & 0x3FFFUL) | 0x4000UL);
        priv->tx_desc_write_index = 0;
    } else {
        ptx->low_tag = (skb->len & 0x3FFFUL);
        priv->tx_desc_write_index++;
    }
    //mb();

    //MTKTV_PR_INFO("start xmit after dma:\n");
//mtktv_mac_skb_dump(skb->data, skb->len);
    //mdelay(1);
    //Chip_Flush_Cache_Range((unsigned long)(&ptx), sizeof(struct tx_desc));
    //addr = *((volatile u32*)&(ptx->addr));
#if 0
    len = *((volatile u32*)&(ptx->low_tag));
    if (len <= 0) {
        MTKTV_PR_ERR("bad len(%d)\n", len);
        ndev->stats.tx_dropped++;
        goto out_tx;
    }
#endif
    if ((priv->tx_desc_count % 2) == 0) {
        reg_ops->write_8(REG_OFFSET_40_L(BANK_BASE_MAC_1), 0x1);
    } else {
        reg_ops->write_8(REG_OFFSET_41_L(BANK_BASE_MAC_1), 0x1);
    }
    priv->tx_desc_count++;
    priv->tx_desc_queued_number++;
    ndev->stats.tx_bytes += skb->len;
    ndev->stats.tx_packets++;
out_tx:
    spin_unlock_irqrestore(&priv->tx_lock, flags);
    mtktv_mac_tx_desc_free_skb(ndev);

    return NETDEV_TX_OK;
#endif /* tx descriptor */
}

static int mtktv_mac_set_mac_address(struct net_device *ndev, void *addr)
{
    struct sockaddr *skaddr = addr;

    if (!is_valid_ether_addr(skaddr->sa_data))
        return -EADDRNOTAVAIL;

    memcpy(ndev->dev_addr, skaddr->sa_data, ndev->addr_len);
    mtktv_mac_hw_mac_addr_set(ndev);

    return 0;
}



static void mtktv_mac_set_rx_mode(struct net_device *ndev)
{
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    struct netdev_hw_addr *ha;
    u32 reg_read, mc_filter_h, mc_filter_l, hash_bit, hash_val, i, tmp_crc, idx;
    u64 mac[6];
    u64 mac_addr;

    reg_read  = reg_ops->read_32(REG_OFFSET_02_L(BANK_BASE_MAC_0));

    if (ndev->flags & IFF_PROMISC) {
        /* enable promiscuous mode */
        reg_read |= 0x10UL;
    } else if (ndev->flags & (~IFF_PROMISC)) {
        /* disable promiscuous mode */
        reg_read &= ~(0x10UL);
    }

    if (ndev->flags & IFF_ALLMULTI) {
        /* enable all multicast mode */
        reg_ops->write_32(REG_OFFSET_48_L(BANK_BASE_MAC_0), 0xffffffffUL);
        reg_ops->write_32(REG_OFFSET_4A_L(BANK_BASE_MAC_0), 0xffffffffUL);
        reg_read |= 0x40UL;
    } else if (ndev->flags & IFF_MULTICAST) {
        /* enable specific multicasts */
        reg_ops->write_32(REG_OFFSET_48_L(BANK_BASE_MAC_0), 0x0);
        reg_ops->write_32(REG_OFFSET_4A_L(BANK_BASE_MAC_0), 0x0);

        netdev_for_each_mc_addr(ha, ndev) {
            hash_val = 0;
            mac_addr = 0;
            for (i = 0; i < 6; i++) {
                mac[i] = (u64)ha->addr[i];
            }
            mac_addr |= (mac[0] | (mac[1] << 8) | (mac[2] << 16) |
                         (mac[3] << 24) | (mac[4] << 32) | (mac[5] << 40));
            /* hash value */
            for (hash_bit = 0; hash_bit < 6; hash_bit++) {
                tmp_crc = ((mac_addr & (0x1UL << hash_bit)) >> hash_bit);
                for (i = 1; i < 8; i++) {
                    idx = hash_bit + (i * 6);
                    tmp_crc = (tmp_crc ^ ((mac_addr >> idx) & 0x1));
                }
                hash_val |= (tmp_crc << hash_bit);
            }
            mc_filter_l = reg_ops->read_32(REG_OFFSET_48_L(BANK_BASE_MAC_0));
            mc_filter_h = reg_ops->read_32(REG_OFFSET_4A_L(BANK_BASE_MAC_0));

            if (hash_val < 32) {
                mc_filter_l |= (0x1UL << hash_val);
                reg_ops->write_32(REG_OFFSET_48_L(BANK_BASE_MAC_0), mc_filter_l);
            } else {
                mc_filter_h |= (0x1UL << (hash_val - 32));
                reg_ops->write_32(REG_OFFSET_4A_L(BANK_BASE_MAC_0), mc_filter_h);
            }
        }
        reg_read |= 0x40UL;
    } else if (ndev->flags & ~(IFF_ALLMULTI | IFF_MULTICAST)) {
        /* disable all multicast mode */
        reg_ops->write_32(REG_OFFSET_48_L(BANK_BASE_MAC_0), 0x0);
        reg_ops->write_32(REG_OFFSET_4A_L(BANK_BASE_MAC_0), 0x0);
        reg_read &= ~(0x40UL);
    }

    reg_ops->write_32(REG_OFFSET_02_L(BANK_BASE_MAC_0), reg_read);
}

static int mtktv_mac_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
    return 0;
}

static struct net_device_stats *mtktv_mac_get_stats(struct net_device *ndev)
{
    return &ndev->stats;
}

static const struct net_device_ops mtktv_mac_netdev_ops = {
    .ndo_open = mtktv_mac_open,
    .ndo_stop = mtktv_mac_stop,
    .ndo_start_xmit = mtktv_mac_start_xmit,
    .ndo_set_mac_address = mtktv_mac_set_mac_address,
    .ndo_set_rx_mode = mtktv_mac_set_rx_mode,
    .ndo_do_ioctl = mtktv_mac_ioctl,
    .ndo_get_stats = mtktv_mac_get_stats,
};

static int mtktv_mac_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct net_device *ndev = platform_get_drvdata(pdev);
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u8 reg_irq;
    u32 reg_trx;

    MTKTV_PR_INFO("enter mac suspend\n");

    netif_stop_queue(ndev);
    del_timer(&priv->link_timer);
    netif_carrier_off(ndev);

    /* disable hw tx rx */
    reg_trx = reg_ops->read_32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
    reg_trx &= 0xfffffff3;
    reg_ops->write_32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_trx);

    /* disable all irq first */
    reg_ops->write_32(REG_OFFSET_16_L(BANK_BASE_MAC_0), IRQ_DISABLE_ALL);
    reg_irq = reg_ops->read_8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
    reg_irq &= 0x7f;
    reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);

    /* disable napi */
    napi_disable(&priv->napi_rx);

    /* memory release */
    mtktv_mac_mem_release(ndev);

    return 0;
}

static int mtktv_mac_resume(struct platform_device *pdev)
{
    struct net_device *ndev = platform_get_drvdata(pdev);
    struct mtktv_mac_private *priv = netdev_priv(ndev);
    struct mtktv_mac_reg_ops *reg_ops = priv->reg_ops;
    u8 reg_irq;
    u32 reg_trx;
    int ret;

    MTKTV_PR_INFO("enter mac resume\n");

    ret = mtktv_mac_hw_init(ndev);
    if (ret)
        MTKTV_PR_ERR("hw init fail in resume\n");

    netif_carrier_off(ndev);

    /* memory init */
    mtktv_mac_mem_init(ndev);
    /* enable napi */
    napi_enable(&priv->napi_rx);

    /* disable all irq first */
    reg_ops->write_32(REG_OFFSET_16_L(BANK_BASE_MAC_0), IRQ_DISABLE_ALL);
    reg_irq = reg_ops->read_8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
    reg_irq &= 0x7f;
    reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);
    /* enable irq */
    reg_ops->write_32(REG_OFFSET_14_L(BANK_BASE_MAC_0), IRQ_ENABLE_BIT);
    reg_irq = reg_ops->read_8(REG_OFFSET_02_L(BANK_BASE_MAC_1));
    reg_irq |= 0x80;
    reg_ops->write_8(REG_OFFSET_02_L(BANK_BASE_MAC_1), reg_irq);

    /* enable hw tx rx */
    reg_trx = reg_ops->read_32(REG_OFFSET_00_L(BANK_BASE_MAC_0));
    reg_trx |= 0x0c;
    reg_ops->write_32(REG_OFFSET_00_L(BANK_BASE_MAC_0), reg_trx);

    mtktv_mac_link_status_update(ndev);
    add_timer(&priv->link_timer);
    netif_start_queue(ndev);

    return 0;
}

static int mtktv_mac_probe(struct platform_device *pdev)
{
    struct mtktv_mac_private *priv = NULL;
    int ret = 0;
    struct device_node *np = NULL;
    struct net_device *ndev = NULL;
    struct mtktv_mac_reg_ops *reg_ops = NULL;
    const char *mac_addr;

    MTKTV_PR_INFO("enter mac probe\n");

    ndev = alloc_etherdev(sizeof(struct mtktv_mac_private));
    if (!ndev)
        return -ENOMEM;

    /* new chips support to 8G */
    pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
    pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

#ifdef CONFIG_ARM64
    arch_setup_dma_ops(&pdev->dev, 0, 0, NULL, 0);
#endif

    np = pdev->dev.of_node;
    if (!np) {
        MTKTV_PR_ERR("fail to find node\n");
        ret = -EINVAL;
        goto err_free_netdev;
    }

    ndev->netdev_ops = &mtktv_mac_netdev_ops;

    SET_NETDEV_DEV(ndev, &pdev->dev);

    priv = netdev_priv(ndev);
    priv->ndev = ndev;
    priv->pdev = pdev;

    /* phy restart patch */
    priv->phy_restart_cnt = 0;

    /* get of properties */
    priv->mac_bank_0 = of_iomap(np, 0);
    priv->mac_bank_1 = of_iomap(np, 1);
    priv->albany_bank_0 = of_iomap(np, 2);
    priv->clkgen_bank_0 = of_iomap(np, 3);
    priv->chip_bank = of_iomap(np, 4);

    if ((!priv->mac_bank_0) || (!priv->mac_bank_1) ||
        (!priv->albany_bank_0) || (!priv->clkgen_bank_0) ||
        (!priv->chip_bank)) {
        MTKTV_PR_ERR("fail to ioremap, mac0(0x%p), mac1(0x%p), "
                     "albany0(0x%p), clkgen0(0x%p), "
                     "chip(0x%p)\n",
                     priv->mac_bank_0, priv->mac_bank_1,
                     priv->albany_bank_0, priv->clkgen_bank_0,
                     priv->chip_bank);
        ret = -ENOMEM;
        goto err_iounmap;
    }

    ndev->irq = irq_of_parse_and_map(np, 0);

    if (ndev->irq <= 0) {
        MTKTV_PR_ERR("no IRQ found, irq(%d)\n", ndev->irq);
        ret = -EINVAL;
        goto err_iounmap;
    }

    MTKTV_PR_INFO("mac0(0x%p), mac1(0x%p), "
                  "albany0(0x%p), clkgen0(0x%p), "
                  "chip(0x%p), irq(%d)\n",
                  priv->mac_bank_0, priv->mac_bank_1,
                  priv->albany_bank_0, priv->clkgen_bank_0,
                  priv->chip_bank, ndev->irq);

    mac_addr = of_get_mac_address(np);
    if (mac_addr) {
        ether_addr_copy(mac_addr_dts, mac_addr);
        MTKTV_PR_INFO("mac_dts(%pM)\n", mac_addr_dts);
    }

    /* hook register ops */
    reg_ops = kmalloc(sizeof(struct mtktv_mac_reg_ops), GFP_KERNEL);
    if (!reg_ops) {
        MTKTV_PR_ERR("fail to malloc reg_ops\n");
        ret = -ENOMEM;
        goto err_iounmap;
    }

    if (of_property_read_bool(np, "xiu32-support") == true) {
        MTKTV_PR_INFO("support xiu32 mode\n");
        reg_ops->write_32 = mtktv_mac_reg_write_32;
        reg_ops->read_32 = mtktv_mac_reg_read_32;
    } else {
        MTKTV_PR_INFO("no xiu32 mode\n");
        reg_ops->write_32 = mtktv_mac_reg_write_32_xiu16;
        reg_ops->read_32 = mtktv_mac_reg_read_32_xiu16;
    }
    reg_ops->write_8 = mtktv_mac_reg_write_8;
    reg_ops->read_8 = mtktv_mac_reg_read_8;

    if (of_property_read_bool(np, "internal-phy") == true) {
        MTKTV_PR_INFO("use internal phy\n");
        priv->is_internal_phy = true;
        reg_ops->write_phy = mtktv_mac_phy_write_internal;
        reg_ops->read_phy = mtktv_mac_phy_read_internal;
    } else {
        MTKTV_PR_INFO("use external phy\n");
        priv->is_internal_phy = false;
        reg_ops->write_phy = mtktv_mac_phy_write_external;
        reg_ops->read_phy = mtktv_mac_phy_read_external;
    }

    /* hw mac descriptor version */
    if (of_property_read_u8(np, "hw-desc-ver", &priv->hw_desc_ver)) {
        priv->hw_desc_ver = MAC_HW_DESC_VER_DEFAULT;
        MTKTV_PR_INFO("set hw_desc_ver to default value (%d)\n",
                      priv->hw_desc_ver);
    } else {
        if (priv->hw_desc_ver <= 0) {
            MTKTV_PR_INFO("bad hw_desc_ver(%d), set it to default value(%d)\n",
                          priv->hw_desc_ver, MAC_HW_DESC_VER_DEFAULT);
            priv->hw_desc_ver = MAC_HW_DESC_VER_DEFAULT;
        }
    }
    if (priv->hw_desc_ver == 1)
        reg_ops->rx_checksum = mtktv_mac_rx_checksum_v1;
    else if (priv->hw_desc_ver == 2)
        reg_ops->rx_checksum = mtktv_mac_rx_checksum_v2;
    else
        reg_ops->rx_checksum = mtktv_mac_rx_checksum_v2;

    priv->reg_ops = reg_ops;

    /* allocate descriptor resource */
    if (of_property_read_u32(np, "rx-desc-size", &priv->rx_desc_size)) {
        priv->rx_desc_size = RX_DESC_SIZE_DEFAULT;
        MTKTV_PR_INFO("set rx_desc_size to default value (%d)\n",
                      priv->rx_desc_size);
    } else {
        if (priv->rx_desc_size <= 0) {
            MTKTV_PR_INFO("bad rx_desc_size(%d), set it to default value\n",
                          priv->rx_desc_size);
            priv->rx_desc_size = RX_DESC_SIZE_DEFAULT;
        }
    }

    if (of_property_read_u32(np, "rx-desc-num", &priv->rx_desc_num)) {
        priv->rx_desc_num = RX_DESC_NUM_DEFAULT;
        MTKTV_PR_INFO("set rx_desc_num to default value (%d)\n",
                      priv->rx_desc_num);
    } else {
        if (priv->rx_desc_num <= 0) {
            MTKTV_PR_INFO("bad rx_desc_num(%d), set it to default value\n",
                          priv->rx_desc_num);
            priv->rx_desc_num = RX_DESC_NUM_DEFAULT;
        }
    }

#if 0 /* tx descriptor */
    if (of_property_read_u32(np, "tx-desc-size", &priv->tx_desc_size)) {
        priv->tx_desc_size = TX_DESC_SIZE_DEFAULT;
        MTKTV_PR_INFO("set tx_desc_size to default value (%d)\n",
                      priv->tx_desc_size);
    } else {
        if (priv->tx_desc_size <= 0) {
            MTKTV_PR_INFO("bad tx_desc_size(%d), set it to default value\n",
                          priv->tx_desc_size);
            priv->tx_desc_size = TX_DESC_SIZE_DEFAULT;
        }
    }

    if (of_property_read_u32(np, "tx-desc-num", &priv->tx_desc_num)) {
        priv->tx_desc_num = TX_DESC_NUM_DEFAULT;
        MTKTV_PR_INFO("set tx_desc_num to default value (%d)\n",
                      priv->tx_desc_num);
    } else {
        if (priv->tx_desc_num <= 0) {
            MTKTV_PR_INFO("bad tx_desc_num(%d), set it to default value\n",
                          priv->tx_desc_num);
            priv->tx_desc_num = TX_DESC_NUM_DEFAULT;
        }
    }
#endif /* tx descriptor */

    priv->rx_desc_dma_pool = dma_pool_create("mtktv-mac-rx", &pdev->dev,
                                 (priv->rx_desc_size * priv->rx_desc_num),
                                 RX_DESC_PTR_ALIGNMENT, 0);
    if (!priv->rx_desc_dma_pool) {
        MTKTV_PR_ERR("fail to rx dma_pool_create\n");
        ret = -ENOMEM;
        goto err_free_kmalloc;
    }

#if 0 /* tx descriptor */
    priv->tx_desc_dma_pool = dma_pool_create("mtktv-mac-tx", &pdev->dev,
                                 (priv->tx_desc_size * priv->tx_desc_num),
                                 TX_DESC_PTR_ALIGNMENT, 0);
    if (!priv->tx_desc_dma_pool) {
        MTKTV_PR_ERR("fail to tx dma_pool_create\n");
        ret = -ENOMEM;
        goto err_free_kmalloc;
    }
#endif /* tx descriptor */

    /* tx memcpy */
    priv->tx_memcpy_dma_pool = dma_pool_create("mtktv-mac-tx", &pdev->dev,
                                 (TX_MEMCPY_SIZE_DEFAULT * TX_MEMCPY_NUM_DEFAULT),
                                 TX_MEMCPY_PTR_ALIGNMENT, 0);
    if (!priv->tx_memcpy_dma_pool) {
        MTKTV_PR_ERR("fail to tx dma_pool_create\n");
        ret = -ENOMEM;
        goto err_free_kmalloc;
    }

    priv->rx_desc_vir_addr = (uintptr_t)dma_pool_alloc(priv->rx_desc_dma_pool,
                                            GFP_KERNEL | GFP_DMA,
                                            &priv->rx_desc_dma_addr);
    if (!priv->rx_desc_vir_addr) {
        MTKTV_PR_ERR("fail to rx dma_poll_alloc\n");
        ret = -ENOMEM;
        goto err_free_dma;
    }

    if (priv->rx_desc_dma_addr & (RX_DESC_PTR_ALIGNMENT - 1)) {
        MTKTV_PR_ERR("rx dma_addr(0x%p) not %d aligned\n",
                     (void *)priv->rx_desc_dma_addr, RX_DESC_PTR_ALIGNMENT);
        ret = -EINVAL;
        goto err_free_dma;
    }

    MTKTV_PR_INFO("rx desc_size(%d), desc_num(%d), vir_addr(0x%p), rx dma_addr(0x%p)\n",
                  priv->rx_desc_size,
                  priv->rx_desc_num,
                  (void *)priv->rx_desc_vir_addr,
                  (void *)priv->rx_desc_dma_addr);

#if 0 /* tx descriptor */
    priv->tx_desc_vir_addr = (uintptr_t)dma_pool_alloc(priv->tx_desc_dma_pool,
                                            GFP_KERNEL | GFP_DMA,
                                            &priv->tx_desc_dma_addr);
    if (!priv->tx_desc_vir_addr) {
        MTKTV_PR_ERR("fail to tx dma_poll_alloc\n");
        ret = -ENOMEM;
        goto err_free_dma;
    }

    if (priv->tx_desc_dma_addr & (TX_DESC_PTR_ALIGNMENT - 1)) {
        MTKTV_PR_ERR("tx dma_addr(0x%p) not %d aligned\n",
                     (void *)priv->tx_desc_dma_addr, TX_DESC_PTR_ALIGNMENT);
        ret = -EINVAL;
        goto err_free_dma;
    }

    MTKTV_PR_INFO("tx desc_size(%d), desc_num(%d), vir_addr(0x%p), tx dma_addr(0x%p)\n",
                  priv->tx_desc_size,
                  priv->tx_desc_num,
                  (void *)priv->tx_desc_vir_addr,
                  (void *)priv->tx_desc_dma_addr);
#endif /* tx descriptor */

    /* tx memcpy */
    priv->tx_memcpy_vir_addr = (uintptr_t)dma_pool_alloc(priv->tx_memcpy_dma_pool,
                                            GFP_KERNEL | GFP_DMA,
                                            &priv->tx_memcpy_dma_addr);
    if (!priv->tx_memcpy_vir_addr) {
        MTKTV_PR_ERR("fail to tx dma_poll_alloc\n");
        ret = -ENOMEM;
        goto err_free_dma;
    }

    if (priv->tx_memcpy_dma_addr & (TX_MEMCPY_PTR_ALIGNMENT - 1)) {
        MTKTV_PR_ERR("tx dma_addr(0x%p) not %d aligned\n",
                     (void *)priv->tx_memcpy_dma_addr, TX_MEMCPY_PTR_ALIGNMENT);
        ret = -EINVAL;
        goto err_free_dma;
    }

    MTKTV_PR_INFO("tx memcpy_size(%d), memcpy_num(%d), vir_addr(0x%p), tx dma_addr(0x%p)\n",
                  TX_MEMCPY_SIZE_DEFAULT,
                  TX_MEMCPY_NUM_DEFAULT,
                  (void *)priv->tx_memcpy_vir_addr,
                  (void *)priv->tx_memcpy_dma_addr);

    /* init */
    skb_queue_head_init(&priv->rx_skb_q);
#if 0 /* tx descriptor */
    skb_queue_head_init(&priv->tx_skb_q);
#endif /* tx descriptor */

    //mtktv_mac_mem_init(ndev);
#if 0 /* tx descriptor */
    mtktv_mac_tx_desc_reset(ndev);
    mtktv_mac_tx_desc_mode_set(ndev);
#endif /* tx descriptor */

    /* tx memcpy */
    //priv->tx_memcpy_index = 0;

    ret = mtktv_mac_hw_init(ndev);
    if (ret)
        goto err_free_dma;

    /* register irq */
    if (request_irq(ndev->irq, mtktv_mac_isr, /*SA_INTERRUPT*/IRQF_ONESHOT, ndev->name, ndev) != 0) {
        MTKTV_PR_ERR("request irq %d fail\n", ndev->irq);
        ret = -ENODEV;
        goto err_free_dma;
    }

#if defined(CONFIG_MP_PLATFORM_GIC_SET_MULTIPLE_CPUS) && defined(CONFIG_MP_PLATFORM_INT_1_to_1_SPI)
    irq_set_affinity_hint(ndev->irq, cpu_online_mask);
    irq_set_affinity(ndev->irq, cpu_online_mask);
#endif /* CONFIG_MP_PLATFORM_GIC_SET_MULTIPLE_CPUS && CONFIG_MP_PLATFORM_INT_1_to_1_SPI */

    /* register napi */
    netif_napi_add(ndev, &priv->napi_rx, mtktv_mac_napi_poll_rx, RX_NAPI_WEIGHT);

    ret = register_netdev(ndev);
    if (ret)
        goto err_free_dma;

    /* timer */
    init_timer(&priv->link_timer);
    priv->link_timer.data = ((unsigned long)ndev);
    priv->link_timer.function = mtktv_mac_link_timer_callback;
    priv->link_timer.expires = jiffies + (MAC_UPDATE_LINK_TIME/20);

    /* lock */
    spin_lock_init(&priv->tx_lock);

    platform_set_drvdata(pdev, ndev);

    return 0;

err_free_dma:
    if (priv->rx_desc_vir_addr)
        dma_pool_free(priv->rx_desc_dma_pool,
                      (void *)priv->rx_desc_vir_addr,
                      priv->rx_desc_dma_addr);
    if (priv->rx_desc_dma_pool)
        dma_pool_destroy(priv->rx_desc_dma_pool);
#if 0 /* tx descriptor */
    if (priv->tx_desc_vir_addr)
        dma_pool_free(priv->tx_desc_dma_pool,
                      (void *)priv->tx_desc_vir_addr,
                      priv->tx_desc_dma_addr);
    if (priv->tx_desc_dma_pool)
        dma_pool_destroy(priv->tx_desc_dma_pool);
#endif /* tx descriptor */

    /* tx memcpy */
    if (priv->tx_memcpy_vir_addr)
        dma_pool_free(priv->tx_memcpy_dma_pool,
                      (void *)priv->tx_memcpy_vir_addr,
                      priv->tx_memcpy_dma_addr);
    if (priv->tx_memcpy_dma_pool)
        dma_pool_destroy(priv->tx_memcpy_dma_pool);

    skb_queue_purge(&priv->rx_skb_q);
#if 0 /* tx descriptor */
    skb_queue_purge(&priv->tx_skb_q);
#endif /* tx descriptor */

err_free_kmalloc:
    if (priv->reg_ops)
        kfree(priv->reg_ops);

err_iounmap:
    if (priv->mac_bank_0)
        iounmap(priv->mac_bank_0);
    if (priv->mac_bank_1)
        iounmap(priv->mac_bank_1);
    if (priv->albany_bank_0)
        iounmap(priv->albany_bank_0);
    if (priv->clkgen_bank_0)
        iounmap(priv->clkgen_bank_0);
    if (priv->chip_bank)
        iounmap(priv->chip_bank);

err_free_netdev:
    free_netdev(ndev);
    return ret;
}

static int mtktv_mac_remove(struct platform_device *pdev)
{
    struct net_device *ndev = platform_get_drvdata(pdev);
    struct mtktv_mac_private *priv = netdev_priv(ndev);

    unregister_netdev(ndev);

    if (priv->rx_desc_vir_addr)
        dma_pool_free(priv->rx_desc_dma_pool,
                      (void *)priv->rx_desc_vir_addr,
                      priv->rx_desc_dma_addr);
    if (priv->rx_desc_dma_pool)
        dma_pool_destroy(priv->rx_desc_dma_pool);
#if 0 /* tx descriptor */
    if (priv->tx_desc_vir_addr)
        dma_pool_free(priv->tx_desc_dma_pool,
                      (void *)priv->tx_desc_vir_addr,
                      priv->tx_desc_dma_addr);
    if (priv->tx_desc_dma_pool)
        dma_pool_destroy(priv->tx_desc_dma_pool);
#endif /* tx descriptor */

    /* tx memcpy */
    if (priv->tx_memcpy_vir_addr)
        dma_pool_free(priv->tx_memcpy_dma_pool,
                      (void *)priv->tx_memcpy_vir_addr,
                      priv->tx_memcpy_dma_addr);
    if (priv->tx_memcpy_dma_pool)
        dma_pool_destroy(priv->tx_memcpy_dma_pool);

    skb_queue_purge(&priv->rx_skb_q);
#if 0 /* tx descriptor */
    skb_queue_purge(&priv->tx_skb_q);
#endif /* tx descriptor */

    if (priv->reg_ops)
        kfree(priv->reg_ops);

    if (priv->mac_bank_0)
        iounmap(priv->mac_bank_0);
    if (priv->mac_bank_1)
        iounmap(priv->mac_bank_1);
    if (priv->albany_bank_0)
        iounmap(priv->albany_bank_0);
    if (priv->clkgen_bank_0)
        iounmap(priv->clkgen_bank_0);
    if (priv->chip_bank)
        iounmap(priv->chip_bank);

    free_netdev(ndev);

    return 0;
}

static const struct of_device_id mtktv_mac_match[] = {
    { .compatible = "mediatek,mtk-tv-mac" },
    {},
};
MODULE_DEVICE_TABLE(of, mtktv_mac_match);

static struct platform_driver mtktv_mac_driver = {
    .probe      = mtktv_mac_probe,
    .remove     = mtktv_mac_remove,
    .suspend    = mtktv_mac_suspend,
    .resume     = mtktv_mac_resume,
    .driver = {
        .name   = "mtk-tv-ethernet",
        .of_match_table = mtktv_mac_match,
        .owner  = THIS_MODULE,
    }
};

static int __init mtktv_mac_init(void)
{
    int err;

    MTKTV_PR_INFO("enter module init\n");
    err = platform_driver_register(&mtktv_mac_driver);
    if (err)
        return err;

    MTKTV_PR_INFO("driver loaded\n");
    return 0;
}

static void __exit mtktv_mac_exit(void)
{
    platform_driver_unregister(&mtktv_mac_driver);
}


module_init(mtktv_mac_init);
module_exit(mtktv_mac_exit);

MODULE_AUTHOR("MTKTV");
MODULE_DESCRIPTION("EMAC Ethernet driver");
MODULE_LICENSE("GPL");
