/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#ifndef __UNFD_M7221_SBOOT_H__
#define __UNFD_M7221_SBOOT_H__

#include <string.h>

#define TV_ROM

#define BL2_OFFSET	56*1024

#define REG_OFFSET_SHIFT_BITS               2

#define REG(Reg_Addr)                       (*(volatile U16*)(Reg_Addr))
#define GET_REG_ADDR(x, y)                  ((x)+((y) << REG_OFFSET_SHIFT_BITS))

#define REG_WRITE_UINT16(reg_addr, val)     REG((reg_addr)) = (val)
#define REG_READ_UINT16(reg_addr, val)      val = REG((reg_addr))
#define REG_SET_BITS_UINT16(reg_addr, val)  REG((reg_addr)) |= (val)
#define REG_CLR_BITS_UINT16(reg_addr, val)  REG((reg_addr)) &= ~(val)
#define REG_W1C_BITS_UINT16(reg_addr, val)  REG_WRITE_UINT16((reg_addr), REG((reg_addr))&(val))


#define RIU_PM_BASE                         0x1F000000U
#define RIU_BASE                            0x1F200000U
#define CONFIG_RIU_BASE_ADDRESS             0x1F000000

#ifdef CONFIG_ASIC_EMULATION
#define CONFIG_SRAM_BASE_ADDRESS            0x41000000
#else
#define CONFIG_SRAM_BASE_ADDRESS            0x1FC00000
#endif

#define CONFIG_SRAM_FCIE_OFFSET             0xE000


#define REG_BANK_CLKGEN                     0x0580U
#define REG_BANK_CHIPTOP                    0x0F00U
#define REG_BANK_FCIE0                      0x8980U
#define REG_BANK_FCIE1                      0x89E0U
#define REG_BANK_FCIE2                      0x8A00U
#define REG_BANK_FCIE3                      0x8A80U
#define REG_BANK_EMMC_PLL                   0x11F80
#define REG_BANK_TIMER0                     0x1800U
#define REG_BANK_MIU                        0x0900U
#define REG_BANK_MAILBOX                    0x19C0U

#define REG_BANK_PORTSTATUS                 0x0280U
//#define REG_BANK_PMSLEEP                    0x0700U

#define CHIPTOP_BASE                        GET_REG_ADDR(RIU_BASE, REG_BANK_CHIPTOP)
#define CLKGEN_BASE                         GET_REG_ADDR(RIU_BASE, REG_BANK_CLKGEN)
#define FCIE0_BASE                          GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE0)
#define FCIE1_BASE                          GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE1)
#define FCIE2_BASE                          GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE2)
#define FCIE3_BASE                          GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE3)
#define EMMC_PLL_BASE                       GET_REG_ADDR(RIU_BASE, REG_BANK_EMMC_PLL)
#define MAILBOX_BASE                        GET_REG_ADDR(RIU_BASE, REG_BANK_MAILBOX)

#define PORTSTATUS_BASE                     GET_REG_ADDR(RIU_BASE, REG_BANK_PORTSTATUS)
//#define PMSLEEP_BASE                        GET_REG_ADDR(RIU_PM_BASE, REG_BANK_PMSLEEP)

#define TIMER0_REG_BASE_ADDR                GET_REG_ADDR(CONFIG_RIU_BASE_ADDRESS, REG_BANK_TIMER0)

#define FCIE_REG_BASE_ADDR                  FCIE0_BASE
#define FCIE_REG_BASE1_ADDR                 FCIE1_BASE
#define FCIE_NC_CIFD_BASE                   FCIE2_BASE
#define FCIE_NC_WBUF_CIFD_BASE              GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE2)
#define FCIE_NC_RBUF_CIFD_BASE              GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE2 + 0x20)
#define EMMC_PLL_REG_BASE_ADDR              EMMC_PLL_BASE

#define DEBUG_REG                           (CONFIG_RIU_BASE_ADDRESS + (0x103380 << 1)) //0x1F206700
#define DEBUG_REG_SYS_L1                    (CONFIG_RIU_BASE_ADDRESS + (0x103392 << 1)) //0x1F206724
#define DEBUG_REG_SYS_L2                    (CONFIG_RIU_BASE_ADDRESS + (0x103394 << 1)) //0x1F206728
#define DEBUG_REG_SYS_L3                    (CONFIG_RIU_BASE_ADDRESS + (0x103396 << 1))

#define DUMMY_REG1                          (CONFIG_RIU_BASE_ADDRESS + (0x103382 << 1)) //0x1F206704
#define DUMMY_REG2                          (CONFIG_RIU_BASE_ADDRESS + (0x103384 << 1)) //0x1F206708
#define DUMMY_REG3                          (CONFIG_RIU_BASE_ADDRESS + (0x103386 << 1)) //0x1F20670C
#define DUMMY_REG4                          (CONFIG_RIU_BASE_ADDRESS + (0x103388 << 1)) //0x1F206710

#define NANDDrv_ADDR                        (CONFIG_SRAM_BASE_ADDRESS + CONFIG_SRAM_FCIE_OFFSET)
#define ROM_PPM_ADDR                        (NANDDrv_ADDR + sizeof(NAND_DRIVER))	//note: sizeof NAND_DRIVER should be the same with that of ROM
#define SPARE_BUFFER_ADDR                   (ROM_PPM_ADDR + 256)

//#define ROM_PPM_ADDR            (CONFIG_SRAM_BASE_ADDRESS + CONFIG_SRAM_FCIE_OFFSET + 128)                              //check the elf.map for rom code binary

//#define SPARE_BUFFER_ADDR                   (CONFIG_SRAM_BASE_ADDRESS + CONFIG_SRAM_FCIE_OFFSET + 256)

#define BOOTSTAGE_REG                       GET_REG_ADDR(PORTSTATUS_BASE, 0x7)

#define BOOT_NONSECURE                      0x00
#define BOOT_HASH0                          0x80
#define BOOT_HASH1                          0x81
#define BOOT_HASH2                          0x82

#define REG_ALL_PAD_IN                      0x50

#define REG_NAND_MODE                       0x6F
#define REG_NAND_MODE_MASK                  (BIT7|BIT6)
#define NAND_MODE1                          (BIT6)

#define NC_DQS_PULL_H()
#define NC_DQS_PULL_L()

#define REG_NAND_AD_PE                      GET_REG_ADDR(CHIPTOP_BASE, 0x0D)
#define REG_NAND_AD_PS                      GET_REG_ADDR(CHIPTOP_BASE, 0x0F)

#define FCIE5_DDR                           0

#if defined(FCIE5_DDR) && FCIE5_DDR
#define ENABLE_TOGGLE_MODE_SUPPORT          1
#define ENABLE_8BIT_MACRO                   1
#define ENABLE_32BIT_MACRO                  0
#endif

#define NAND_PAD_BYPASS_MODE                1
#define NAND_PAD_TOGGLE_MODE                2

#define FCIE_LFSR							1

#define MICRON_READ_RETRY					1

#define SUPPORT_3D_NAND						1


#if defined(ENABLE_TOGGLE_MODE_SUPPORT) && ENABLE_TOGGLE_MODE_SUPPORT
#define NC_SET_DDR_MODE()                   REG_WRITE_UINT16(NC_DDR_CTRL, pNandDrv->u16_Reg58_DDRCtrl);
#define NC_CLR_DDR_MODE()                   REG_CLR_BITS_UINT16(NC_DDR_CTRL, BIT_DDR_MASM);
#else
#define NC_SET_DDR_MODE()
#define NC_CLR_DDR_MODE()
#endif

#define NC_CLR_SRAM_SEL()                   REG_CLR_BITS_UINT16(NC_BOOT_MODE, BIT_BOOTSRAM_ACCESS)
#define NC_SET_SRAM_SEL()                   REG_SET_BITS_UINT16(NC_BOOT_MODE, BIT_BOOTSRAM_ACCESS)

//-------------------------------

#define REG50_ECC_CTRL_INIT_VALUE           0 //(BIT_NC_SHARE_PAD_EN)

#define UNFD_ST_PLAT                        0x80000000
#define IF_IP_VERIFY                        0 // [CAUTION]: to verify IP and HAL code, defaut 0

#define NC_SEL_FCIE5                        1
#if NC_SEL_FCIE5
#include "drvNAND_reg_v5.h"
#else
#error "Error! no FCIE registers selected."
#endif

#define IF_FCIE_SHARE_PINS                  0 // 1: need to nand_pads_switch at HAL's functions.
#define IF_FCIE_SHARE_CLK                   0 // 1: need to nand_clock_setting at HAL's functions.
#define IF_FCIE_SHARE_IP                    0

#define ENABLE_NAND_INTERRUPT_MODE          0

#define ENABLE_ROM_DMA_FUNCTION     0

#if defined(ENABLE_ROM_DMA_FUNCTION) && ENABLE_ROM_DMA_FUNCTION
#define ENABLE_NAND_RIU_MODE                0
#else
#define ENABLE_NAND_RIU_MODE                1
#endif

#if defined(ENABLE_NAND_RIU_MODE) && ENABLE_NAND_RIU_MODE
#undef ENABLE_NAND_INTERRUPT_MODE
#define ENABLE_NAND_INTERRUPT_MODE          0
#endif

#define IF_SPARE_AREA_DMA                   0 // [CAUTION]

#define NAND_DRIVER_ROM_VERSION             0 // to save code size
#define AUTO_FORMAT_FTL                     0

#define ENABLE_CUS_READ_ENHANCEMENT         0

#undef BIT_NC_ECC_CNT_MASK
#undef BIT_NC_ECC_SEL_LOC_MASK
#undef BIT_NC_ECC_SEL_LOC_SHIFT
#define BIT_NC_ECC_CNT_MASK        (BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2)
#define BIT_NC_ECC_SEL_LOC_MASK    (BIT15|BIT14|BIT13|BIT12|BIT11|BIT10|BIT9)
#define BIT_NC_ECC_SEL_LOC_SHIFT   9

//=====================================================
#define ENABLE_BLANK_CHECK_SUPPORT          1

#define ENABLE_READ_DISTURBANCE_SUPPORT     1
#define DEBUG_REG_ECC_STATUS                NC_RX5E // choose dummy reg from FCIE or CHIPTOP

// 2 bits ECC status
#define ECC_STATUS_NORMAL                   0
#define ECC_STATUS_BITFLIP                  1
#define ECC_STATUS_ERROR                    2

//---------------------------------------------------------------------
//         RESERVED          |   BL   | UBOOT | HASH2 | HASH1 | HASH0 |
//---------------------------------------------------------------------
#define BIT_ECC_STATUS_HASH0_SHIFT          0
#define BIT_ECC_STATUS_HASH1_SHIFT          2
#define BIT_ECC_STATUS_HASH2_SHIFT          4
#define BIT_ECC_STATUS_UBOOT_SHIFT          6
#define BIT_ECC_STATUS_BL_SHIFT             8
//=====================================================

#define __VER_UNFD_FTL__                    0
//=====================================================
// Nand Driver configs
//=====================================================
#define NAND_BUF_USE_STACK                  0
#define NAND_ENV_FPGA                       1
#define NAND_ENV_ASIC                       2
#ifdef __FPGA_MODE__        // currently G1 don't care this macro, even in ROM code
#define NAND_DRIVER_ENV                     NAND_ENV_FPGA
#else
#define NAND_DRIVER_ENV                     NAND_ENV_ASIC
#endif

#if defined(__ASIC_SIM__)   // && __ASIC_SIM__
#define NAND_DRIVER_FOR_ROM_SIM             1
#else
#define NAND_DRIVER_FOR_ROM_SIM             0
#endif

#define UNFD_CACHE_LINE                     0x20
//=====================================================
// tool-chain attributes
//=====================================================
#define UNFD_PACK0
#define UNFD_PACK1                          __attribute__((__packed__))
#define UNFD_ALIGN0
#define UNFD_ALIGN1                         __attribute__((aligned(UNFD_CACHE_LINE)))

//=====================================================
// debug option
//=====================================================
#define NAND_TEST_IN_DESIGN                 0      /* [CAUTION] */

#ifndef NAND_DEBUG_MSG
#define NAND_DEBUG_MSG                      0
#endif

/* Define trace levels. */
#define UNFD_DEBUG_LEVEL_ERROR              (1)    /* Error condition debug messages. */
#define UNFD_DEBUG_LEVEL_WARNING            (2)    /* Warning condition debug messages. */
#define UNFD_DEBUG_LEVEL_HIGH               (3)    /* Debug messages (high debugging). */
#define UNFD_DEBUG_LEVEL_MEDIUM             (4)    /* Debug messages. */
#define UNFD_DEBUG_LEVEL_LOW                (5)    /* Debug messages (low debugging). */

/* Higer debug level means more verbose */
#ifndef UNFD_DEBUG_LEVEL
#define UNFD_DEBUG_LEVEL                    UNFD_DEBUG_LEVEL_WARNING
#endif

#if defined(NAND_DEBUG_MSG) && NAND_DEBUG_MSG
#define nand_printf    printf
#define nand_debug(dbg_lv, tag, str, ...)           \
    do {                                            \
        if (dbg_lv > UNFD_DEBUG_LEVEL)              \
            break;                                  \
        else {                                      \
            if (tag)                                \
                nand_printf("[ %s() ] ", __func__); \
                                                    \
            nand_printf(str, ##__VA_ARGS__);        \
        }                                           \
    } while(0)
#else /* NAND_DEBUG_MSG */
#define nand_printf(...)
#define nand_debug(enable, tag, str, ...)   {}
#endif /* NAND_DEBUG_MSG */

#define REG_BANK_UART		(RIU_PM_BASE+(0x100900<<1))
#define UART_BASE			GET_REG_ADDR(REG_BANK_UART, 0x40)
#define PUTK(val)			(*(volatile U8*)(UART_BASE)) = val
extern void print_err(U32 Err);

extern void nand_reset_WatchDog(void);

#define nand_die(msg)                           \
    REG_WRITE_UINT16(DEBUG_REG_SYS_L3, 0xDEAD); \
    while(1);

#define nand_stop() \
    while(1)  nand_reset_WatchDog();

//=====================================================
// HW Timer for Delay
//=====================================================
#define TIMER0_ENABLE                       GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x10)
#define TIMER0_HIT                          GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x11)
#define TIMER0_MAX_LOW                      GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x12)
#define TIMER0_MAX_HIGH                     GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x13)
#define TIMER0_CAP_LOW                      GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x14)
#define TIMER0_CAP_HIGH                     GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x15)


#define HW_TIMER_DELAY_1us                  (1)
#define HW_TIMER_DELAY_10us                 (10 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_100us                (100 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_1ms                  (1000 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_5ms                  (5 * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_10ms                 (10 * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_100ms                (100 * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_500ms                (500 * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_1s                   (1000 * HW_TIMER_DELAY_1ms)

extern void delay_us( unsigned us );
extern U32  nand_hw_timer_delay(U32 u32usTick);

//=====================================================
// Pads Switch
//=====================================================
extern U32 nand_pads_switch(U32 u32EnableFCIE);
extern U32 NC_ReadOneID(void);
extern U32 nand_pad_detect(void);

#define REG_CLK_NFIE                        GET_REG_ADDR(CLKGEN_BASE, 0x64)
#define BIT_CLK_ENABLE                      BIT6

//=====================================================
// set FCIE clock
//=====================================================
#define NFIE_CLK_MASK                       ((BIT4-1)<<2)
#define NFIE_CLK_XTAL                       (0<<2)
#define NFIE_CLK_20M                        (1<<2)
#define NFIE_CLK_32M                        (2<<2)
#define NFIE_CLK_36M                        (3<<2)
#define NFIE_CLK_40M                        (4<<2)
#define NFIE_CLK_43_2M                      (5<<2)
#define NFIE_CLK_54M                        (6<<2)
#define NFIE_CLK_62M                        (7<<2)
#define NFIE_CLK_72M                        (8<<2)
#define NFIE_CLK_86M                        (9<<2)
#define NFIE_CLK_48M                        (15<<2)

#define DUTY_CYCLE_PATCH                    0 // 1: to enlarge low width for tREA's worst case of 25ns
#if DUTY_CYCLE_PATCH
#define FCIE3_SW_DEFAULT_CLK                NFIE_CLK_86M
#define FCIE_REG41_VAL                      ((2<<9)|(2<<3)) // RE,WR pulse, Low:High=3:1
#else
#define FCIE3_SW_DEFAULT_CLK                NFIE_CLK_36M
#define FCIE_REG41_VAL                      0               // RE,WR pulse, Low:High=1:1
#endif
#define FCIE3_SW_SLOWEST_CLK                NFIE_CLK_XTAL

#define NAND_SEQ_ACC_TIME_TOL               10 //in unit of ns

extern void drvTimer_Enable(unsigned int dwMicroSecondNumber, unsigned char bNeedWaitComplete);
extern U32  nand_clock_setting(U32 u32ClkParam);

#define usleep(TimerCount, WaitComplete)    drvTimer_Enable(TimerCount, WaitComplete)

//=====================================================
// transfer DMA Address
//=====================================================
#define MIU_BUS_WIDTH_BITS                  3 // Need to confirm
/*
 * Important:
 * The following buffers should be large enough for a whole NAND block
 */
// FIXME, this is only for verifing IP
#define DMA_W_ADDR                          0x80C00000
#define DMA_R_ADDR                          0x80D00000
#define DMA_W_SPARE_ADDR                    0x80E00000
#define DMA_R_SPARE_ADDR                    0x80E80000
#define DMA_BAD_BLK_BUF                     0x80F00000

//=====================================================
// misc
//=====================================================
//#define BIG_ENDIAN
#define LITTLE_ENDIAN

typedef struct NAND_DRIVER_PLATFORM_DEPENDENT
{
    U8      *pu8_PageSpareBuf;
    U8      *pu8_PageDataBuf;
    U32     u32_DMAAddrOffset;
    U32     u32_RAMBufferOffset;
    U32     u32_RAMBufferLen;

}NAND_DRIVER_PLAT_CTX, *P_NAND_DRIVER_PLAT;

// locate API to DRAM area
#define __ATTR_DRAM_CODE__  __attribute__((section ("NANDCODE")))

#endif /* __UNFD_M7221_SBOOT_H__ */
