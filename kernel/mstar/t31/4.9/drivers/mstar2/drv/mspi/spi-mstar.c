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

/* dts example for maxim 107B */
/*
	    spi@0x153A {
			compatible = "mstar,mstar-mspi";
			reg = <0 0 0x2A7400 0x1000>;
            interrupts = <43>;
            mspi_channel = <0>;
			#address-cells = <1>;
			#size-cells = <0>;
			status = "okay";
				spidev@1{
					compatible = "spidev";
					reg = <0x0>;
					spi-max-frequency = <2000000>;
				};
	    };

 */

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/spi/spi.h>
#include <linux/version.h>
#include <linux/spinlock.h>

#ifdef CONFIG_AMAZON_METRICS_LOG
#include <linux/metricslog.h>
#endif

#define DEBUG_SPI_TIMEOUT 0
#if DEBUG_SPI_TIMEOUT
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#endif

#define MSPI_PRINT(fmt, args...)        //printk("[MSPI][%05d] " fmt, __LINE__, ## args)
/*
 * CONFIG_DEBUG_MSPI:
 *      1:map MAXIM pins of mspi to  reg_ld_spi3_config
 *      0:require pins mapping at sboot/mboot stage.
 */
#define CONFIG_DEBUG_MSPI 0

/*
 Threshold value for SPI pkt length check.
 This is to detect invalid SPI pkt length by memory corrption or unknown reason.
*/
#define MSPI_PKT_LEN_CHK    (100 * 1024)

/* SPI register offsets */
#define MSPI_WD0_1                      0x40
#define MSPI_WD2_3                      0x41
#define MSPI_WD4_5                      0x42
#define MSPI_WD6_7                      0x43
#define MSPI_WD8_9                      0x00
#define MSPI_WD10_11                      0x01
#define MSPI_WD12_13                      0x02
#define MSPI_WD14_15                      0x03
#define MSPI_WD16_17                      0x04
#define MSPI_WD18_19                      0x05
#define MSPI_WD20_21                      0x06
#define MSPI_WD22_23                      0x07
#define MSPI_WD24_25                      0x08
#define MSPI_WD26_27                      0x09
#define MSPI_WD28_29                      0x0A
#define MSPI_WD30_31                      0x0B

#define MSPI_SINGLE_MODE_HALF_DUPLEX      0
#define MSPI_SINGLE_MODE 1
#define MSPI_DUAL_MODE 2
int MSPI_SPI_MODE = MSPI_SINGLE_MODE_HALF_DUPLEX;
/*
#define MSPI_FRD0_1          0x78
#define MSPI_FRD2_3          0x79
#define MSPI_FRD4_5          0x7A
#define MSPI_FRD6_7          0x7B
#define MSPI_FRD8_9          0x7C
#define MSPI_FRD10_11      0x7D
#define MSPI_FRD12_13       0x7E
#define MSPI_FRD14_15       0x7F
#define MSPI_WBF_RBF_SIZE_MAX           0x10

#define MSPI_RD0_1                     0x44
#define MSPI_RD2_3                     0x45
#define MSPI_RD4_5                     0x46
#define MSPI_RD6_7                     0x47
#define MSPI_RD8_9                     0x10
#define MSPI_RD10_11                      0x11
#define MSPI_RD12_13                      0x12
#define MSPI_RD14_15                      0x13
#define MSPI_RD16_17                      0x14
#define MSPI_RD18_19                      0x15
#define MSPI_RD20_21                      0x16
#define MSPI_RD22_23                      0x17
#define MSPI_RD24_25                      0x18
#define MSPI_RD26_27                      0x19
#define MSPI_RD28_29                      0x1A
#define MSPI_RD30_31                      0x1B
*/
#define MSPI_WBF_RBF_SIZE               0x48

static const u16 mspi_txfifoaddr[] = {
    MSPI_WD0_1,
    MSPI_WD2_3,
    MSPI_WD4_5,
    MSPI_WD6_7,
    MSPI_WD8_9,
    MSPI_WD10_11,
    MSPI_WD12_13,
    MSPI_WD14_15,
    MSPI_WD16_17,
    MSPI_WD18_19,
    MSPI_WD20_21,
    MSPI_WD22_23,
    MSPI_WD24_25,
    MSPI_WD26_27,
    MSPI_WD28_29,
    MSPI_WD30_31,
};
static const u16 *mspi_rxfifoaddr;
static const u16 mspi_rxfifoaddr_fullduplex[] = {
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F};
static const u16 mspi_rxfifoaddr_halfduplex[] = {
    0x44, 0x45, 0x46, 0x47, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B};
static int MSPI_WBF_RBF_SIZE_MAX = 0x10;  // default to single mode
#define MAX_HALF_TX_BUF_SIZE           0x20
#define MAX_FULL_TX_BUF_SIZE           0x10
#define MSPI_HALF_DUPLEX    1
#define MSPI_FULL_DUPLEX    0

/*
#define WB16_INDEX                     0x4
#define WB8_INDEX                      0x3
#define MSPI_WRITE_BUF_OFFSET          0x40
#define MSPI_WRITE_EXT_BUF_OFFSET      0x00
#define MAX_WRITE_BUF_SIZE             0x20
#define MSPI_READ_EXT_BUF_OFFSET       0x10
#define MSPI_READ_BUF_OFFSET           0x44
*/

#define MSPI_CTRL_CLOCK_RATE            0x49
#define MSPI_CLK_CLOCK_MASK            0xFF
#define MSPI_ENABLE_BIT                 BIT(0)
#define MSPI_RESET_BIT                  BIT(1)
#define MSPI_ENABLE_INT_BIT             BIT(2)
#define MSPI_3WARE_MODE_BIT             BIT(4)
#define MSPI_CPHA_BIT                   BIT(6)
#define MSPI_CPOL_BIT                   BIT(7)
#define MSPI_CLK_CLOCK_BIT_OFFSET   8
#define MSPI_CLK_P1_MAX     0x8
#define MSPI_CLK_P2_MAX     0x6
#define MSPI_CLK_DIV_INDEX_MAX     0x8

#define MSPI_TR_START_END_TIME          0x4a
#define MSPI_TBYTE_INTERVAL_AROUND_TIME 0x4b
#define MSPI_WD0_3_BIT_SEL              0x4c
#define MSPI_WD4_7_BIT_SEL              0x4d
#define MSPI_RD0_3_BIT_SEL              0x4e
#define MSPI_RD4_7_BIT_SEL              0x4f
#define MSPI_LSB_FIRST                  0x50
#define MSPI_LSB_FIRST_BIT              BIT(0)
#define MSPI_TRIGGER                    0x5a
#define MSPI_TRIGGER_BIT                BIT(0)
#define MSPI_DONE_FLAG                  0x5b
#define MSPI_CLEAR_DONE_FLAG            0x5c
#define MSPI_CLEAR_DONE_FLAG_BIT        BIT(0)
#define MSPI_CHIP_SELECT                0x5f
#define MSPI_CHIP_SELECT_BIT            BIT(0)

#define MSPI_CTR2                0x38
#define MSPI_DUAL_MODE_BIT            BIT(0)

#define  MSPI2				2
// clk_spi_m_p2
#define  MSPI2_CLK_DIV                  0x1D
#define  MSPI2_CLK_DIV_MASK             0x0F
#define  MSPI2_CLK_CFG                  0x1D
#define  MSPI2_CLK_CFG_OFFSET           10
#define  MSPI2_CLK_MASK                 0x1F00
#define  MSPI2_CLK_DEFAULT              0x20
#define  MSPI2_MAXCLKLEVEL		0x07
#define  MSPI2_CLK_ENABLE               13

#define  MSPI_TSLINK			3
// ckg_mspi_cilink
#define  MSPI_TSLINK_CLK_DIV                  0x12
#define  MSPI_TSLINK_CLK_DIV_OFFSET           8
#define  MSPI_TSLINK_CLK_DIV_MASK             0xF00
#define  MSPI_TSLINK_CLK_CFG                  0x12
#define  MSPI_TSLINK_CLK_CFG_OFFSET           2
#define  MSPI_TSLINK_CLK_MASK                 0x1F
#define  MSPI_TSLINK_CLK_DEFAULT              0x20
#define  MSPI_TSLINK_MAXCLKLEVEL	      0x07
#define  MSPI_TSLINK_CLK_ENABLE                  5
#define MSPI_CLK_INDEX_MAX     0x400

#define MSTAR_SPI_TIMEOUT_MS	1000
#define MSTAR_SPI_MODE_BITS	(SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | SPI_NO_CS | SPI_LSB_FIRST | SPI_RX_DUAL |SPI_TX_DUAL)

#define DRV_NAME	"mspi-mstar"

#if defined(CONFIG_IDME)
extern char *idme_get_config_name(void);
#define DTS_STRING_LENGTH 64
#endif

struct mstar_spi {
    u64 regs;
    u64 clkgen;
    u32 mspi_channel;
    struct clk *clk;
    int irq;
    struct completion done;
    const u8 *tx_buf;
    u8 *rx_buf;
    int len;
    int current_trans_len;
    int num_chipselect;
    int bus_num;
    u8 fg_half_duplex;
    u8 print_irq;
    spinlock_t lock;
};

struct mstar_spi_data {
	u32 regs;
    u64 clkgen;
	u32 irq;
    u32 mspi_channel;
        int num_chipselect;
        int bus_num;
};

#if defined(CONFIG_ARM64)
extern ptrdiff_t mstar_pm_base;
#define REG_ADDR(addr)  (*((volatile u16 *)((mstar_pm_base + (addr )))))
#define BASEREG_ADDR(addr)  ((mstar_pm_base + (addr )))
#else
#define REG_RIU_BASE 0xFD000000
#define REG_ADDR(addr)  (*((volatile u16 *)((REG_RIU_BASE + (addr )))))
#define BASEREG_ADDR(addr)  (REG_RIU_BASE + (addr ))
#endif
// read 2 byte
#define MSPI_READ(_reg_)          (REG_ADDR(bs->regs + ((_reg_)<<2)))

// write 2 byte
#define MSPI_WRITE(_reg_, _val_)    \
        do{ REG_ADDR(bs->regs + ((_reg_)<<2)) =(_val_) ; }while(0)

#define CLK_READ(_reg_)                       (REG_ADDR(bs->clkgen + ((_reg_)<<2)))
#define CLK_WRITE(_reg_, _val_)    \
        do{ REG_ADDR(bs->clkgen + ((_reg_)<<2)) =(_val_) ; }while(0)

static inline u16 mstar_rd(struct mstar_spi *bs, u32 reg)
{
	return MSPI_READ(reg);
}
static inline u8 mstar_rdh(struct mstar_spi *bs, u32 reg)
{
	return mstar_rd(bs,reg)>>8;
}
static inline u8 mstar_rdl(struct mstar_spi *bs, u16 reg)
{
	return mstar_rd(bs,reg)&0xff;
}
static inline void mstar_wr(struct mstar_spi *bs, u16 reg, u32 val)
{
	MSPI_WRITE(reg,val);
}
static inline void mstar_wrh(struct mstar_spi *bs, u16 reg, u8 val)
{
    u16 val16 = mstar_rd(bs,reg)&0xff;
    val16 |= ((u16)val)<<8;
	mstar_wr(bs,reg,val16);
}
static inline void mstar_wrl(struct mstar_spi *bs, u16 reg, u8 val)
{
    u16 val16 = mstar_rd(bs,reg)&0xff00;
    val16 |= val;
	mstar_wr(bs,reg,val16);
}

/* HIFI4DSP SPI xfer speed */
typedef struct
{
    u8 u8ClkSpi_P1;
    u8 u8ClkSpi_P2;
    u8 u8ClkSpi_DIV;
    u32 u32ClkSpi;
}ST_DRV_MSPI_CLK;

extern void mtk_dsp_wdt_disable(void);
extern void hifidsp_hw_pull_low(void);

//#define SPI_LOAD_IMAGE_SPEED		(18*1000*1000)
//#define SPI_SPEED_LOW				(12*1000*1000)
//#define SPI_SPEED_HIGH				(18*1000*1000)
#define SPI_SPEED_LOW 1
static u8 g_lowu8ClkSpi_P1=0;
static u8 g_lowu8ClkSpi_P2=0;
static u8 g_lowu8ClkSpi_DIV=0;
static u8 g_u8ClkSpi_P1=0;
static u8 g_u8ClkSpi_P2=0;
static u8 g_u8ClkSpi_DIV=0;
static void mstar_hw_parsing_clock_table(u8 u8speed_low,u32 u32MspiClk)
{
    u16 i = 0;
    u16 j = 0;
    u16 k= 0;
    u32 clk =0;
    ST_DRV_MSPI_CLK temp;
    static ST_DRV_MSPI_CLK clk_buffer[MSPI_CLK_INDEX_MAX];
    u8 clk_spi_m_p1[8] = {27, 48, 62, 72, 86, 108, 123, 160};
    u16 clk_spi_div[8] = {2, 4, 8, 16, 32, 64, 128, 256};
    memset(&temp,0,sizeof(ST_DRV_MSPI_CLK));
    memset(&clk_buffer,0,sizeof(ST_DRV_MSPI_CLK)*MSPI_CLK_INDEX_MAX);

    for(i = 0;i < MSPI_CLK_P1_MAX;i++)//clk_spi_m_p1
    {
        for(j =0 ;j < MSPI_CLK_P2_MAX;j++)//clk_spi_m_p2
        {
            for(k = 0;k < MSPI_CLK_DIV_INDEX_MAX;k++)//spi div
            {
                clk = clk_spi_m_p1[i] * 1000000;
                clk_buffer[k+MSPI_CLK_P1_MAX*j+MSPI_CLK_P2_MAX*i].u8ClkSpi_P1 = i;
                clk_buffer[k+MSPI_CLK_P1_MAX*j+MSPI_CLK_P2_MAX*i].u8ClkSpi_P2 = j;
                clk_buffer[k+MSPI_CLK_P1_MAX*j+MSPI_CLK_P2_MAX*i].u8ClkSpi_DIV = k ;
                clk_buffer[k+MSPI_CLK_P1_MAX*j+MSPI_CLK_P2_MAX*i].u32ClkSpi = clk/((j+1)*(clk_spi_div[k]));
            }
        }
    }
    for(i = 0;i<MSPI_CLK_INDEX_MAX;i++)
    {
        for(j = i;j<MSPI_CLK_INDEX_MAX;j++)
        {
            if(clk_buffer[i].u32ClkSpi > clk_buffer[j].u32ClkSpi)
            {
                memcpy(&temp,&clk_buffer[i],sizeof(ST_DRV_MSPI_CLK));

                memcpy(&clk_buffer[i],&clk_buffer[j],sizeof(ST_DRV_MSPI_CLK));

                memcpy(&clk_buffer[j],&temp,sizeof(ST_DRV_MSPI_CLK));
            }
        }
    }
for(i = 0;i<MSPI_CLK_INDEX_MAX;i++)
    {
        if(u32MspiClk <= clk_buffer[i].u32ClkSpi)
        {
            break;
        }
    }
    //match Closer clk
    if((i>0)&&(i<MSPI_CLK_INDEX_MAX))
    {
        if((u32MspiClk - clk_buffer[i-1].u32ClkSpi)<(clk_buffer[i].u32ClkSpi - u32MspiClk))
        {
            i -= 1;
        }
    }
    else
    {
        printk("Wrong Index \n");
        return;
    }
    printk("u8ClkSpi_P1 =%d\n", clk_buffer[i].u8ClkSpi_P1);
    printk("u8ClkSpi_P2 =%d\n", clk_buffer[i].u8ClkSpi_P2);
    printk("u8ClkSpi_DIV =%d\n", clk_buffer[i].u8ClkSpi_DIV);
    printk("u32ClkSpi = %ld\n", (long int)clk_buffer[i].u32ClkSpi);
    if (u8speed_low) {
        g_lowu8ClkSpi_P1=clk_buffer[i].u8ClkSpi_P1;;
        g_lowu8ClkSpi_P2=clk_buffer[i].u8ClkSpi_P2;
        g_lowu8ClkSpi_DIV=clk_buffer[i].u8ClkSpi_DIV;
    }
    else {
        g_u8ClkSpi_P1 = clk_buffer[i].u8ClkSpi_P1;
        g_u8ClkSpi_P2 = clk_buffer[i].u8ClkSpi_P2;
        g_u8ClkSpi_DIV = clk_buffer[i].u8ClkSpi_DIV;
    }
}

static void mstar_hw_set_clock(struct mstar_spi *bs,struct spi_device *spi,struct spi_transfer *tfr)
{
    /* Get mspi clock from below table or clock tree.
	DivLevel	0(2)	1(4)	2(8)
	ClkLevel
	0(27)		13.5	6.75	3.375
	1(48)		24	12	6
	2(62)		31	15.5	7.75
	3(72)		36	18	9
	4(86)		43	21.5	10.75
	5(108)		54	27	13.5
	6(123)		61.5	30.75	15.375
	7(160)		80	40	20
    */
    u16 TempData = 0;
    if (tfr->speed_hz==SPI_SPEED_LOW) {
        //mstar_hw_parsing_clock_table(tfr->speed_hz);
        TempData = CLK_READ(MSPI_TSLINK_CLK_CFG);
        TempData &= ~(MSPI_TSLINK_CLK_MASK);
        TempData |= g_lowu8ClkSpi_P1 << MSPI_TSLINK_CLK_CFG_OFFSET;
        TempData |= 1 << MSPI_TSLINK_CLK_ENABLE;
        CLK_WRITE(MSPI_TSLINK_CLK_CFG, TempData);

        TempData = CLK_READ(MSPI_TSLINK_CLK_DIV);
        TempData &= ~(MSPI_TSLINK_CLK_DIV_MASK);
        TempData |= g_lowu8ClkSpi_P2 << MSPI_TSLINK_CLK_DIV_OFFSET;
        CLK_WRITE(MSPI_TSLINK_CLK_DIV, TempData);

        TempData = MSPI_READ(MSPI_CTRL_CLOCK_RATE);
        TempData &= MSPI_CLK_CLOCK_MASK;
        TempData |= g_lowu8ClkSpi_DIV << MSPI_CLK_CLOCK_BIT_OFFSET;
        MSPI_WRITE(MSPI_CTRL_CLOCK_RATE, TempData);
        }
    else {  // control by bootarg
        //mstar_hw_parsing_clock_table(tfr->speed_hz);
        TempData = CLK_READ(MSPI_TSLINK_CLK_CFG);
        TempData &= ~(MSPI_TSLINK_CLK_MASK);
        TempData |= g_u8ClkSpi_P1 << MSPI_TSLINK_CLK_CFG_OFFSET;
        TempData |= 1 << MSPI_TSLINK_CLK_ENABLE;
        CLK_WRITE(MSPI_TSLINK_CLK_CFG, TempData);

        TempData = CLK_READ(MSPI_TSLINK_CLK_DIV);
        TempData &= ~(MSPI_TSLINK_CLK_DIV_MASK);
        TempData |= g_u8ClkSpi_P2 << MSPI_TSLINK_CLK_DIV_OFFSET;
        CLK_WRITE(MSPI_TSLINK_CLK_DIV, TempData);

        TempData = MSPI_READ(MSPI_CTRL_CLOCK_RATE);
        TempData &= MSPI_CLK_CLOCK_MASK;
        TempData |= g_u8ClkSpi_DIV << MSPI_CLK_CLOCK_BIT_OFFSET;
        MSPI_WRITE(MSPI_CTRL_CLOCK_RATE, TempData);

    }
#if 0
    u16 u16ClkDiv = 0;
    u16 u16ClkLevel = 0;
    u16 u16TempDiv = 0;
    u32 u32Clock = 0;
    u32 u32TempClock = 0;
    u32 u32ClockMatch = 0;
    u8  u8ClkIdx = 0;
    u8  u8DivIdx = 0;
    u32 u32MaxClock = tfr->speed_hz;
    if (u32MaxClock==SPI_SPEED_LOW) {
        u16TempDiv = 1 << MSPI_CLK_CLOCK_BIT_OFFSET;
        MSPI_WRITE(MSPI_CTRL_CLOCK_RATE, u16TempDiv);
        if (bs->mspi_channel == MSPI2) { //SPI SPEED LOW to 12M
            u16ClkLevel = 1 << MSPI2_CLK_CFG_OFFSET;
            u16ClkLevel |= 1 << MSPI2_CLK_ENABLE;
            CLK_WRITE(MSPI2_CLK_CFG, u16ClkLevel);
        } else if (bs->mspi_channel == MSPI_TSLINK) { //SPI SPEED LOW to 12M
            u16ClkLevel = 1 << MSPI_TSLINK_CLK_CFG_OFFSET;
            u16ClkLevel |= 1 << MSPI_TSLINK_CLK_ENABLE;
            CLK_WRITE(MSPI_TSLINK_CLK_CFG, u16ClkLevel);
        }
    } else {
        if (bs->mspi_channel == MSPI2) { //SPI SPEED HIGH to 18M
            u16TempDiv =  1 << MSPI_CLK_CLOCK_BIT_OFFSET;
            MSPI_WRITE(MSPI_CTRL_CLOCK_RATE, u16TempDiv);
            u16ClkLevel = 3 << MSPI2_CLK_CFG_OFFSET;
            u16ClkLevel |= 1 <<MSPI2_CLK_ENABLE;
            CLK_WRITE(MSPI2_CLK_CFG, u16ClkLevel);
        } else if (bs->mspi_channel == MSPI_TSLINK) { //MSPI_TSLINK to 18M
            u16TempDiv = 1 <<MSPI_CLK_CLOCK_BIT_OFFSET;
            MSPI_WRITE(MSPI_CTRL_CLOCK_RATE, u16TempDiv);
            u16ClkLevel = 3 << MSPI_TSLINK_CLK_CFG_OFFSET;
            u16ClkLevel |= 1 << MSPI_TSLINK_CLK_ENABLE;
            CLK_WRITE(MSPI_TSLINK_CLK_CFG, u16ClkLevel);
        }
    }
#endif
}
#if CONFIG_DEBUG_MSPI
static void mstar_hw_set_pin_mode(struct mstar_spi *bs,struct spi_device *spi)
{
   /*
    * FIXME:
    *   Setup mspi pin mode from pinctrl.
    */
    u64 PINCTL_REG = 0x101E<<9;
    u64 PM_SLEEP_REG = 0x0E<<9;
    REG_ADDR(PM_SLEEP_REG + (0x72<<2)) = BIT(15);
    REG_ADDR(PINCTL_REG + (0x4e<<2)) = BIT(5);

}
#endif
static void mstar_hw_enable_interrupt(struct mstar_spi *bs,bool enable)
{
    u8 val = mstar_rdl(bs,MSPI_CTRL_CLOCK_RATE);
    if (enable){
        val |= MSPI_ENABLE_INT_BIT;
    }
    else{
        val &= ~MSPI_ENABLE_INT_BIT;
    }
    mstar_wrl(bs,MSPI_CTRL_CLOCK_RATE,val);
}
static void mstar_hw_set_bits(struct mstar_spi *bs,struct spi_device *spi)
{
    int bits = spi->bits_per_word - 1;
    bits = bits|(bits<<3)|(bits<<6)|(bits<<9);
    mstar_wr(bs,MSPI_WD0_3_BIT_SEL,bits);
    mstar_wr(bs,MSPI_WD4_7_BIT_SEL,bits);
    mstar_wr(bs,MSPI_RD0_3_BIT_SEL,bits);
    mstar_wr(bs,MSPI_RD4_7_BIT_SEL,bits);
}
static void mstar_hw_set_mode(struct mstar_spi *bs,struct spi_device *spi)
{
    u8 val = mstar_rdl(bs,MSPI_CTRL_CLOCK_RATE);
    if(spi->mode&SPI_CPOL){
        val|= MSPI_CPOL_BIT;
    }else{
        val&= ~MSPI_CPOL_BIT;
    }
    if(spi->mode&SPI_CPHA){
        val|= MSPI_CPHA_BIT;
    }else{
        val&= ~MSPI_CPHA_BIT;
    }
    mstar_wrl(bs,MSPI_CTRL_CLOCK_RATE,val);
    val = mstar_rdl(bs,MSPI_LSB_FIRST);
    if (spi->mode&SPI_LSB_FIRST){
        val |= MSPI_LSB_FIRST_BIT;
    }else{
        val &= ~MSPI_LSB_FIRST_BIT;
    }
    mstar_wrl(bs,MSPI_LSB_FIRST,val);

}
static inline void mstar_hw_chip_select(struct mstar_spi *bs,struct spi_device *spi,bool enable)
{
    u8 val;
    if (spi->mode&SPI_NO_CS){
        return ;
    }

    val = mstar_rdl(bs, MSPI_CHIP_SELECT);

    if (enable != ((spi->mode&SPI_CS_HIGH))){
        val &= ~MSPI_CHIP_SELECT_BIT;
    }else{
        val |= MSPI_CHIP_SELECT_BIT;
    }

    mstar_wrl(bs,MSPI_CHIP_SELECT,val);
}
static inline void mstar_hw_clear_done(struct mstar_spi *bs)
{
    mstar_wrl(bs,MSPI_CLEAR_DONE_FLAG,MSPI_CLEAR_DONE_FLAG_BIT);
}
static inline void mstar_hw_enable(struct mstar_spi *bs,bool enable)
{
    u8 val;
    val = mstar_rdl(bs,MSPI_CTRL_CLOCK_RATE);
    if (enable){
        val |= MSPI_ENABLE_BIT;
        val |= MSPI_RESET_BIT;
    }else{
        val &= ~MSPI_ENABLE_BIT;
        val &= ~MSPI_RESET_BIT;
    }
    mstar_wrl(bs,MSPI_CTRL_CLOCK_RATE,val);
}
static inline void mstar_hw_transfer_trigger(struct mstar_spi *bs)
{
    mstar_wr(bs,MSPI_TRIGGER,MSPI_TRIGGER_BIT);
}
static void mstar_hw_txdummy(struct mstar_spi *bs,u8 len)
{
    int cnt;
    for (cnt = 0; cnt < len>>1;cnt++)
    {
        mstar_wr(bs,mspi_txfifoaddr[cnt],0xffff);
    }
    if (len&1)
    {
        mstar_wrl(bs,mspi_txfifoaddr[cnt],0xff);
    }
	mstar_wrh(bs, MSPI_WBF_RBF_SIZE, 0);
	mstar_wrl(bs, MSPI_WBF_RBF_SIZE, len);
}
static void mstar_hw_txfillfifo(struct mstar_spi *bs,const u8*buffer,u8 len)
{
    int cnt;
    for (cnt = 0; cnt < len>>1;cnt++)
    {
        mstar_wr(bs,mspi_txfifoaddr[cnt],buffer[cnt<<1]|(buffer[(cnt<<1)+1]<<8));
    }
    if (len&1)
    {
        mstar_wrl(bs,mspi_txfifoaddr[cnt],buffer[cnt<<1]);
    }
	mstar_wrh(bs, MSPI_WBF_RBF_SIZE, 0);
	mstar_wrl(bs, MSPI_WBF_RBF_SIZE, len);
}
static void mstar_hw_rxgetfullfifo(struct mstar_spi *bs, u8 *buffer, u8 len)//full
{
    int cnt;
	for (cnt = 0; cnt < (len>>1); cnt++) {
		u16 val = mstar_rd(bs, mspi_rxfifoaddr_fullduplex[cnt]);
		buffer[cnt<<1] = val & 0xff;
		buffer[(cnt<<1)+1] = val>>8;
}
	if (len & 1) {
		buffer[cnt<<1] = mstar_rdl(bs, mspi_rxfifoaddr_fullduplex[cnt]);
    }
}
static void mstar_hw_rxgetfifo(struct mstar_spi *bs, u8 *buffer, u8 len)//half
{
    int cnt;
    for (cnt = 0; cnt < (len>>1);cnt++)
    {
		u16 val = mstar_rd(bs, mspi_rxfifoaddr_halfduplex[cnt]);
        buffer[cnt<<1] = val &0xff;
        buffer[(cnt<<1)+1] = val>>8;
    }
    if (len&1)
    {
		buffer[cnt<<1] = mstar_rdl(bs, mspi_rxfifoaddr_halfduplex[cnt]);
    }
}
static void mstar_spi_hw_receive(struct mstar_spi *bs)//full
{
    MSPI_PRINT("%s\n",__func__);
    if (bs->rx_buf != NULL)
    {
		mstar_hw_rxgetfullfifo(bs, bs->rx_buf, bs->current_trans_len);
        bs->rx_buf += bs->current_trans_len;
    }
}
static void mstar_spi_hw_transfer(struct mstar_spi *bs)//full
{
    int len;

    len = bs->len;
    MSPI_PRINT("%s start len=%d \n",__func__,len);
    if (len >= MAX_FULL_TX_BUF_SIZE) {
		len = MAX_FULL_TX_BUF_SIZE;
    }
	if (bs->tx_buf != NULL) {
        mstar_hw_txfillfifo(bs, bs->tx_buf, len);
        bs->tx_buf += len;
    }else{
        mstar_hw_txdummy(bs,len);
    }
    bs->current_trans_len = len;
    bs->len -= len;

    mstar_hw_transfer_trigger(bs);
    MSPI_PRINT("%s:%d end\n",__func__,__LINE__);
}

static void mstar_spi_hw_rx_ext(struct mstar_spi *bs)
{
    int  j = 0;
    MSPI_PRINT("%s %d\n",__func__,__LINE__);
    j =  bs->current_trans_len;
    if (bs->rx_buf != NULL)
    {
        mstar_hw_rxgetfifo(bs, bs->rx_buf, j);
	bs->rx_buf += j;
    }
}

static void mstar_spi_hw_xfer_ext(struct mstar_spi *bs)//half
{
    int  j = 0;

    j = bs->len;
	if (j >= MAX_HALF_TX_BUF_SIZE) {
		j = MAX_HALF_TX_BUF_SIZE;
    }

    if (bs->tx_buf != NULL)
    {
        mstar_hw_txfillfifo(bs, bs->tx_buf, j);
	(bs->tx_buf) += j;
	mstar_wrl(bs, MSPI_WBF_RBF_SIZE, j);
	mstar_wrh(bs, MSPI_WBF_RBF_SIZE, 0);
    } else {
	if (bs->rx_buf != NULL) {
		mstar_wrh(bs, MSPI_WBF_RBF_SIZE, (j));
		mstar_wrl(bs, MSPI_WBF_RBF_SIZE, 0);
        }
    }
    bs->len -= j;
    bs->current_trans_len = j;
    mstar_hw_transfer_trigger(bs);

}

static irqreturn_t mstar_spi_interrupt(int irq, void *dev_id)
{
    struct spi_master *master = dev_id;
    struct mstar_spi *bs = spi_master_get_devdata(master);
    unsigned long flags;

    MSPI_PRINT("%s %d  bs->current_trans_len=%d \n",__func__,__LINE__,bs->current_trans_len);

    spin_lock_irqsave(&bs->lock, flags);
    if (bs->print_irq)
        pr_err("Error !!!! unexpected irq occurs after timeout\n");
    if(mstar_rd(bs,MSPI_DONE_FLAG)) {
        mstar_hw_clear_done(bs);
        if (bs->current_trans_len != 0) {
            if (MSPI_SPI_MODE == MSPI_SINGLE_MODE) // for full-duplex fifo
                mstar_spi_hw_receive(bs);
            else { // for half-duplex fifo or dual mode used
                if (bs->fg_half_duplex == MSPI_FULL_DUPLEX) {
                    mstar_spi_hw_receive(bs);//full
                } else {
                    mstar_spi_hw_rx_ext(bs);//half
                }
            }
        }
        else {
            dev_err(&master->dev, "Error:data corruption or race condition issue, please investigate!\n");
            spin_unlock_irqrestore(&bs->lock, flags);
            return IRQ_HANDLED;
        }
        if (bs->len != 0) {
            if (MSPI_SPI_MODE == MSPI_SINGLE_MODE) { // for full-duplex fifo
                if (bs->len > MSPI_PKT_LEN_CHK) {
                    spin_unlock_irqrestore(&bs->lock, flags);
                    pr_err("%s: %d SPI pkt len is too big, bs->len = %d!\n",
                            __FUNCTION__, __LINE__, bs->len);
                    WARN_ON_ONCE(1);
                    return IRQ_HANDLED;
                }
                mstar_spi_hw_transfer(bs);
            }
            else  {// for half-duplex fifo or dual mode used
                if (bs->fg_half_duplex == MSPI_FULL_DUPLEX) {
                    mstar_spi_hw_transfer(bs);//full
                }
                else {
                    mstar_spi_hw_xfer_ext(bs);//half
                }
            }
        }
        else {
            bs->current_trans_len = 0;
            complete(&bs->done);
        }
        MSPI_PRINT("%s\n",__func__);
        spin_unlock_irqrestore(&bs->lock, flags);
        return IRQ_HANDLED;
    }
    dev_err(&master->dev, "Error:incorrect irq num!\n");
    mstar_hw_clear_done(bs);
    MSPI_PRINT("%s\n",__func__);
    spin_unlock_irqrestore(&bs->lock, flags);
    return IRQ_HANDLED;
}

static int mstar_spi_start_transfer(struct spi_device *spi,
		struct spi_transfer *tfr)
{
	struct mstar_spi *bs = spi_master_get_devdata(spi->master);

    MSPI_PRINT("%s:%d start\n",__func__,__LINE__);
#if CONFIG_DEBUG_MSPI
    mstar_hw_set_pin_mode(bs,spi);
#endif

    /*
    *   Setup mspi clock for this transfer.
    */
    mstar_hw_set_clock(bs,spi,tfr);

    /*
    *   Setup mspi cpol & cpha for this transfer.
    */
    mstar_hw_set_mode(bs,spi);

    /*
    *   Setup mspi transfer bits for this transfer.
    */
    mstar_hw_set_bits(bs,spi);

    /*
    * Enable SPI master controller&&Interrupt.
    */
    mstar_hw_enable(bs,true);
    mstar_hw_clear_done(bs);
    mstar_hw_enable_interrupt(bs,true);

    /*
    *   Setup mspi chip select for this transfer.
    */
    mstar_hw_chip_select(bs,spi,true);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 1)
    INIT_COMPLETION(bs->done);
#else
	reinit_completion(&bs->done);
#endif

    if (tfr->len > MSPI_PKT_LEN_CHK) {
        pr_err("%s: %d SPI pkt len is too big, tfr->len = %u!\n",
                __FUNCTION__, __LINE__, tfr->len);
        WARN_ON_ONCE(1);
        return -EINVAL;
    }

    bs->tx_buf = tfr->tx_buf;
    bs->rx_buf = tfr->rx_buf;
    bs->len = tfr->len;
	if ((bs->tx_buf == NULL) || (bs->rx_buf == NULL)) {
		bs->fg_half_duplex = MSPI_HALF_DUPLEX;
	} else {
		bs->fg_half_duplex = MSPI_FULL_DUPLEX;
	}

    /*
     *   Start transfer loop.
     */
    //Check if dual mode or single mode
    // Do not enable dual mode only run single mode
#if 0
    u8 val = mstar_rdl(bs,MSPI_CTR2);
    if(tfr->rx_nbits == SPI_NBITS_DUAL || tfr->tx_nbits == SPI_NBITS_DUAL) {
        val|= MSPI_DUAL_MODE_BIT;
    }
    else {
        val &= ~MSPI_DUAL_MODE_BIT;
    }
    mstar_wrl(bs,MSPI_CTR2,val);
#endif

    if (MSPI_SPI_MODE == MSPI_SINGLE_MODE)  // for full-duplex fifo
        mstar_spi_hw_transfer(bs);
    else { // for half-duplex fifo or dual mode used
        if (bs->fg_half_duplex == MSPI_FULL_DUPLEX) {
            mstar_spi_hw_transfer(bs);//full
        }
        else {
            mstar_spi_hw_xfer_ext(bs);//half
        }
    }

    MSPI_PRINT("%s:%d  end\n",__func__,__LINE__);
    return 0;
}

static int mstar_spi_finish_transfer(struct spi_device *spi,
		struct spi_transfer *tfr, bool cs_change)
{
    struct mstar_spi *bs = spi_master_get_devdata(spi->master);
    MSPI_PRINT("%s:%d start\n",__func__,__LINE__);
    if (tfr->delay_usecs)
	udelay(tfr->delay_usecs);

    /* Cancel chip select.*/  // false: pull H,  ture: pull L
    mstar_hw_chip_select(bs,spi,cs_change);
    MSPI_PRINT("%s:%d end\n",__func__,__LINE__);
	return 0;
}

static struct platform_device *gpdev = NULL;

#if DEBUG_SPI_TIMEOUT
struct mstar_spi_dev {
    struct dentry *dent;
    int dbg_counter;
    int enable_timeout_dbg;
};

static struct mstar_spi_dev *mstar_spi_dbg;

static ssize_t mstar_spi_debug_cli_write(struct file *file,
        const char __user *ubuf,
        size_t len, loff_t *offp)
{
    char buf[48];
    int ret;

    if (len >= sizeof(buf))
        return -EINVAL;

    if (copy_from_user(buf, ubuf, len))
        return -EFAULT;

    buf[len] = '\0';

    if (strncmp(buf, "spi_timeout", strlen("spi_timeout")) == 0) {
        pr_info("Enable SPI bus timeout debugging.\n");
        mstar_spi_dbg->enable_timeout_dbg = 1;
        mstar_spi_dbg->dbg_counter = 0;
        return len;
    }
    else {
        pr_info("unsupported cli command.\n");
        return -EINVAL;
    }
    return len;
}

static const struct file_operations mstar_spi_debug_cli_fops = {
    .write = mstar_spi_debug_cli_write,
};

static int mstar_spi_debugfs_init(void)
{
    struct dentry *cli_dent;

    mstar_spi_dbg->dent = debugfs_create_dir("mstar_spi", NULL);
    if (!mstar_spi_dbg->dent)
        return -ENOMEM;

    cli_dent = debugfs_create_file("cli", 0200,
            mstar_spi_dbg->dent, NULL, &mstar_spi_debug_cli_fops);
    if (!cli_dent)
        goto out_err;

    return 0;

out_err:
    debugfs_remove_recursive(mstar_spi_dbg->dent);
    return -ENOMEM;
}
#endif

void trigger_dsp_wdt(void)
{
    char data[32], *envp[] = { data, NULL };
    pr_err("[%s] SPI timeout happens!\n", __func__);
    snprintf(data, sizeof(data), "ACTION=DSP_WTD_WHOLE");
    kobject_uevent_env(&gpdev->dev.kobj, KOBJ_CHANGE, envp);
    pr_err("[%s][Reload DSP]\n", __func__);
}

// Logs the reset metric. Called everytime a SPI transaction times out.
static void log_timeout_metric(unsigned count) {
#ifdef CONFIG_AMAZON_MINERVA_METRICS_LOG
    log_counter_to_vitals_v2(ANDROID_LOG_INFO, KERNEL_METRICS_GROUP_ID, KERNEL_METRICS_MSTAR_SPI_COUNTER_SCHEMA_ID,
		    "Kernel", "farfield", "spi", "timeouts", count, "count", NULL, VITALS_NORMAL, NULL, NULL);
#elif defined(CONFIG_AMAZON_METRICS_LOG)
    log_counter_to_vitals(ANDROID_LOG_INFO, "Kernel", "farfield", "spi", "timeouts", count, "count", NULL, VITALS_NORMAL);
#endif
}

static int mstar_spi_transfer_one(struct spi_master *master,
        struct spi_message *mesg)
{
    struct mstar_spi *bs = spi_master_get_devdata(master);
    struct spi_transfer *tfr;
    struct spi_device *spi = mesg->spi;
    int err = 0;
    int spi_timeout_val = MSTAR_SPI_TIMEOUT_MS;
    unsigned int timeout;
    bool cs_change = true;
    unsigned long flags;

    MSPI_PRINT("%s:%d start \n",__func__,__LINE__);
    list_for_each_entry(tfr, &mesg->transfers, transfer_list) {
        spin_lock_irqsave(&bs->lock, flags);
        err = mstar_spi_start_transfer(spi, tfr);
        if (err) {
            printk("%s:%d \n",__func__,__LINE__);
            spin_unlock_irqrestore(&bs->lock, flags);
            goto out;
        }
        bs->print_irq = 0;
#if DEBUG_SPI_TIMEOUT
        /* Simulate SPI timeout */
        if (unlikely(mstar_spi_dbg->enable_timeout_dbg)) {
            if (bs->len > 5000) {
                pr_err("[SPI_BUS_TIMEOUT_DEBUG]Skip the test logic for firmware loading, bs->len %d \n", bs->len);
            }
            else if (bs->len < 1024) {
                spi_timeout_val = MSTAR_SPI_TIMEOUT_MS; // don't change default timeout for small size of data
            }
            else {
                if (mstar_spi_dbg->dbg_counter > 2000) {
                    pr_info("[SPI_BUS_TIMEOUT_DEBUG]Change timeout to 0ms every 1000 times for large size of data\n");
                    spi_timeout_val = 0;
                    mstar_spi_dbg->dbg_counter = 0;
                }
                mstar_spi_dbg->dbg_counter++;
            }
        }
#endif
        spin_unlock_irqrestore(&bs->lock, flags);
        timeout = wait_for_completion_timeout(&bs->done,
                msecs_to_jiffies(spi_timeout_val));
        if (!timeout) {
            spin_lock_irqsave(&bs->lock, flags);
#if DEBUG_SPI_TIMEOUT
            mstar_spi_dbg->enable_timeout_dbg = 0; /* Disable SPI bus timeout debugging */
#endif
            mstar_hw_enable_interrupt(bs,false); //reset interrupt
            bs->print_irq = 1;
            bs->len = 0;
            spin_unlock_irqrestore(&bs->lock, flags);
            log_timeout_metric(1);
            pr_err("MSPI timeout!! %s:%d -- Reset DSP - len: %d, cur_len: %d, tfr_len: %u\n",
                    __func__, __LINE__, bs->len, bs->current_trans_len, tfr->len);
            err = -ETIMEDOUT;
#if WAR_MT8570_DSP
            mtk_dsp_wdt_disable(); /* disable DSP wdt interruption */
            hifidsp_hw_pull_low(); /* put DSP in dead state */
#endif
            disable_irq(bs->irq);
            mstar_hw_clear_done(bs);
            trigger_dsp_wdt();
            enable_irq(bs->irq);
            goto out;
        }
#if 1 //8570 read command for each spi_transfer not for each list
        cs_change = tfr->cs_change;  // false: pull H,  ture: pull L
#endif
        err = mstar_spi_finish_transfer(spi, tfr, cs_change);
        if (err) {
            printk("%s:%d \n", __func__, __LINE__);
            goto out;
        }
        mesg->actual_length += (tfr->len - bs->len);
#if 0 /* No need to touch SPI mode setting register. Disable below. */
        //Disable dual mode when finished.
        u8 val = mstar_rdl(bs,MSPI_CTR2);
        val &= ~MSPI_DUAL_MODE_BIT;
        mstar_wrl(bs,MSPI_CTR2,val);
#endif
    }
out:
    mesg->status = err;
    spi_finalize_current_message(master);
    MSPI_PRINT("%s:%d  end\n",__func__,__LINE__);
    /* make client know error like TIMEOUT */
    return err;
}
static const struct of_device_id mstar_mspi_match[] = {
    { .compatible = "mstar,mstar-mspi", },
    {}
};
MODULE_DEVICE_TABLE(of, mstar_mspi_match);

static void set_MSPI_DRV(void)
{
	/* Configure T31 SPI Driving (default: 4 mA*/
	/* 0 :2 mA */
	/* 1 :4 mA */
	/* 2 :8 mA */
	/* 3 :16 mA */

	u32 mspi3_drv = 3; // default value
	u32 reg_val = 0;
	char *ptr = strstr(saved_command_line, "MSPI3_DRV");

	if (ptr != NULL) {
		sscanf(ptr + strlen("MSPI3_DRV="), "%u",&mspi3_drv);
	}

	if (mspi3_drv > 3) {
		printk ("MSPI3_DRV=%d in wrong range, should be (0-3)\n Set to 3 here",mspi3_drv);
		mspi3_drv = 3;
	}
	printk ("MSPI3_DRV=%d ==> %d mA\n",mspi3_drv,1<<(mspi3_drv+1));
	//SPI_CK
	reg_val = REG_ADDR((0x322E<<9) + (0x00<<2));
	printk ("SPI_CK reg_val = 0x%lx --> ",(long unsigned int)reg_val);
	reg_val = (reg_val & ~(0x3 << 4)) | (mspi3_drv << 4);
	REG_ADDR((0x322E<<9) + (0x00<<2)) = reg_val;
	printk ("0x%lx \n",(long unsigned int)reg_val);
	//SPI_MOSI
	reg_val = REG_ADDR((0x322E<<9) + (0x02<<2));
	printk ("SPI_MOSI reg_val = 0x%lx --> ",(long unsigned int)reg_val);
	reg_val = (reg_val & ~(0x3 << 4)) | (mspi3_drv << 4);
	REG_ADDR((0x322E<<9) + (0x02<<2)) = reg_val;
	printk ("0x%lx \n",(long unsigned int)reg_val);
}

static int mstar_spi_probe(struct platform_device *pdev)
{
    struct spi_master *master;
    struct mstar_spi *bs;
    int err = -ENODEV;
    int i = 0;
    char project_name[DTS_STRING_LENGTH];
    char property_name[DTS_STRING_LENGTH];
    char *hw_build_id = NULL;
    char *ptr = NULL;
    u32 spi_cfg = 0;
    u32 prop;

    if (strstr(saved_command_line, "farfield.dsp.name=mt8570") == NULL) {
        pr_info("mt8570 is not supported\n");
        return -EINVAL;
    }
#if DEBUG_SPI_TIMEOUT
    mstar_spi_dbg = devm_kzalloc(&pdev->dev, sizeof(struct mstar_spi_dev), GFP_KERNEL);
    if (!mstar_spi_dbg) {
        pr_err("allocation for mstar spi debugfs failed\n");
        return -ENOMEM;
    }
#endif
    REG_ADDR((0x0F<<9) + (0x08<<2)) &= ~(BIT(1));
    printk("DSP_ON set On %s:%d \n",__func__,__LINE__);
    msleep(200);

    master = spi_alloc_master(&pdev->dev, sizeof(*bs));
    if (!master) {
        dev_err(&pdev->dev, "spi_alloc_master() failed\n");
#if DEBUG_SPI_TIMEOUT
        if (mstar_spi_dbg)
            devm_kfree(&pdev->dev, mstar_spi_dbg);
#endif
        return -ENOMEM;
    }

#if DEBUG_SPI_TIMEOUT
    ret = mstar_spi_debugfs_init();
    if (ret) {
        pr_err("Failed to initialize mstar_spi debugfs, err = %d\n", ret);
    }
#endif

    platform_set_drvdata(pdev, master);

    master->mode_bits = MSTAR_SPI_MODE_BITS;
    master->bits_per_word_mask = BIT(8 - 1)|BIT(7 - 1)
        |BIT(6 - 1)|BIT(5 - 1)
        |BIT(4 - 1)|BIT(3 - 1)
        |BIT(2 - 1)|BIT(1 - 1);

    master->transfer_one_message = mstar_spi_transfer_one;
    master->dev.of_node = pdev->dev.of_node;

    bs = spi_master_get_devdata(master);
    // Used for debugging purposes to make sure metrics work.
    log_timeout_metric(0);

    snprintf((char *)project_name, DTS_STRING_LENGTH, "%s", idme_get_config_name());
    hw_build_id = memchr(project_name, '_', sizeof(project_name));
    if (hw_build_id) {
        /*Remove hw_specific string*/
        *hw_build_id = '\0';
    }

    init_completion(&bs->done);
    if (of_match_device(mstar_mspi_match,&pdev->dev)){
        snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "reg_", project_name);
        if (!of_property_read_u32_index(pdev->dev.of_node, property_name, 2, (u32 *)&bs->regs)) {
            printk("[mspi] bs->regs_%s: is 0x%x \n", property_name, (unsigned int)bs->regs);
        } else if (!of_property_read_u32_index(pdev->dev.of_node, "reg",2, (u32 *)&bs->regs)) {
            printk("[mspi] bs->regs = 0x%x\n",(unsigned int)bs->regs);
        } else {
            dev_err(&pdev->dev, "could not get resource\n");
            return -EINVAL;
        }

        snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "interrupts_", project_name);
        if (!of_property_read_u32(pdev->dev.of_node, property_name, &prop)) {
            bs->irq = prop;
            printk("[mspi] bs->irq_%s: is %d \n", property_name, bs->irq);
        } else if (!of_property_read_u32(pdev->dev.of_node, "interrupts",&bs->irq)) {
            printk("[mspi] bs->irq = %d\n",bs->irq);
        } else {
            dev_err(&pdev->dev, "could not get resource\n");
            return -EINVAL;
        }

        snprintf((char *)property_name, DTS_STRING_LENGTH, "%s%s", "mspi_channel_", project_name);
        if (!of_property_read_u32(pdev->dev.of_node, property_name, &prop)) {
            bs->mspi_channel = prop;
            printk("[mspi] bs->mspi_channel_%s: is %d \n", property_name, bs->mspi_channel);
        } else if (!of_property_read_u32(pdev->dev.of_node, "mspi_channel",&bs->mspi_channel)) {
            printk("[mspi] bs->mspi_channel = %d\n",bs->mspi_channel);
        } else {
            dev_err(&pdev->dev, "could not get resource\n");
            return -EINVAL;
        }

        err = of_property_read_u32(pdev->dev.of_node, "num_chipselect",&bs->num_chipselect);
        if (err){
            dev_err(&pdev->dev, "could not get resource num_chipselect\n");
            return -EINVAL;
        } else {
            printk("[mspi] bs->num_chipselect = %d\n",bs->num_chipselect);
        }
        err = of_property_read_u32(pdev->dev.of_node, "bus_num",&bs->bus_num);
        if (err){
            dev_err(&pdev->dev, "could not get resource bus_num\n");
            return -EINVAL;
        } else {
            printk("[mspi] bs->bus_num = %d\n",bs->bus_num);
        }
        err = of_property_read_u32_index(pdev->dev.of_node, "clockreg",2, (u32 *)&bs->clkgen);
        if (err){
            dev_err(&pdev->dev, "could not get resource\n");
            return -EINVAL;
        } else {
            printk("[mspi] bs->clkgen = 0x%x\n",(u32)bs->clkgen);
        }
    } else {
        struct mstar_spi_data *data = dev_get_platdata(&pdev->dev);
        if (!data){
            dev_err(&pdev->dev, "could not get resource %d\n",__LINE__);
            return -EINVAL;
        }
        bs->regs = data->regs;
        bs->irq = data->irq;
        bs->mspi_channel = data->mspi_channel;
        bs->num_chipselect = data->num_chipselect;
        bs->bus_num = data->bus_num;
        bs->clkgen = data->clkgen;
    }

    printk("%s::%d TSLINK pad = %d\n", __func__, __LINE__, (REG_ADDR((0x3229<<9) + (0x12<<2))));
    printk("%s::%d TDM pad = %d\n", __func__, __LINE__, (REG_ADDR((0x3229<<9) + (0x10<<2))));

    if (!bs->regs || !bs->irq) {
        dev_err(&pdev->dev, "could not get resource  %d\n",__LINE__);
        return -EINVAL;
    }
    bs->fg_half_duplex = MSPI_HALF_DUPLEX;
    err = devm_request_irq(&pdev->dev,bs->irq, mstar_spi_interrupt, 0,
            dev_name(&pdev->dev), master);
    if (err) {
        dev_err(&pdev->dev, "could not request IRQ: %d:%d\n", bs->irq,err);
        return err;
    }

    err = spi_register_master(master);
    if (err) {
        dev_err(&pdev->dev, "could not register SPI master: %d\n", err);
        return err;
    }

    if (strstr(saved_command_line, "SPI_MODE=2")) {  // dual mode xfer
        MSPI_SPI_MODE = MSPI_DUAL_MODE;
        mspi_rxfifoaddr = mspi_rxfifoaddr_halfduplex;
        MSPI_WBF_RBF_SIZE_MAX = 2 * (int) (sizeof(mspi_rxfifoaddr_halfduplex) / sizeof(mspi_rxfifoaddr_halfduplex[0]));
    } else if (strstr(saved_command_line, "SPI_MODE=1")) {  // single mode xfer full duplex
        MSPI_SPI_MODE = MSPI_SINGLE_MODE;
        mspi_rxfifoaddr = mspi_rxfifoaddr_fullduplex;
        MSPI_WBF_RBF_SIZE_MAX = 2 * (int) (sizeof(mspi_rxfifoaddr_fullduplex) / sizeof(mspi_rxfifoaddr_fullduplex[0]));
    } else { // single mode & half duplex
        MSPI_SPI_MODE = MSPI_SINGLE_MODE_HALF_DUPLEX;
        mspi_rxfifoaddr = mspi_rxfifoaddr_halfduplex;
        MSPI_WBF_RBF_SIZE_MAX = 2 * (int) (sizeof(mspi_rxfifoaddr_halfduplex) / sizeof(mspi_rxfifoaddr_halfduplex[0]));
    }

    printk ("MSPI_WBF_RBF_SIZE_MAX=%d \n",MSPI_WBF_RBF_SIZE_MAX);
    for(i=0;i<MSPI_WBF_RBF_SIZE_MAX/2;i++)
        printk ("mspi_rxfifoaddr[%d]=0x%lx \n",i, (long unsigned int)mspi_rxfifoaddr[i]);
    set_MSPI_DRV();
    ptr = strstr(saved_command_line, "SPI_SPEED_LOW=");
    if (ptr) {
        sscanf(ptr + strlen("SPI_SPEED_LOW="), "%u",&spi_cfg);
    }
    else { //set default low speed as 12000000
        spi_cfg = 12000000;
    }
    printk ("SPI_SPEED_LOW=%u \n",spi_cfg);
    mstar_hw_parsing_clock_table(SPI_SPEED_LOW,spi_cfg);

    ptr = strstr(saved_command_line, "SPI_SPEED_HIGH=");
    if (ptr) {
        sscanf(ptr + strlen("SPI_SPEED_HIGH="), "%u",&spi_cfg);
    }
    else { //set default high speed as 18000000
        spi_cfg = 18000000;
    }
    printk ("SPI_SPEED_HIGH=%u \n",spi_cfg);
    mstar_hw_parsing_clock_table((!SPI_SPEED_LOW),spi_cfg);
    gpdev = pdev;
    return 0;
}

static int mstar_spi_remove(struct platform_device *pdev)
{
    struct spi_master *master = platform_get_drvdata(pdev);
    struct mstar_spi *bs = spi_master_get_devdata(master);

    devm_free_irq(&pdev->dev,bs->irq, master);
    spi_unregister_master(master);

    spi_master_put(master);
#if DEBUG_SPI_TIMEOUT
    debugfs_remove_recursive(mstar_spi_dbg->dent);
    devm_kfree(&pdev->dev, mstar_spi_dbg);
#endif
    return 0;
}

static int mstar_spi_pm_resume(struct device *pdev)
{
    if (strstr(saved_command_line, "farfield.dsp.name=mt8570") == NULL) {
        pr_info("mt8570 is not supported\n");
        return -ENOMEM;
    }
    printk("%s is resume!\n",__func__);

    set_MSPI_DRV();

    //printk("DSP reset pull high %s:%d \n",__func__,__LINE__);
    /* fix me: Should move to dsp host driver */
    //REG_ADDR((0x14<<9) + (0x12<<2)) |= (BIT(0));

    return 0;
}

static int mstar_spi_pm_suspend(struct device *pdev)
{
    if (strstr(saved_command_line, "farfield.dsp.name=mt8570") == NULL) {
        pr_info("mt8570 is not supported\n");
        return -ENOMEM;
    }

    printk("%s is suspend!\n",__func__);
    //printk("DSP reset pull low  %s:%d \n",__func__,__LINE__);
    /* fix me: Should move to dsp host driver */
    //REG_ADDR((0x14<<9) + (0x12<<2)) &= ~(BIT(0));
    return 0;
}

struct dev_pm_ops const mstar_spi_pm_ops = {
	.suspend         = mstar_spi_pm_suspend,
	.resume          = mstar_spi_pm_resume,
};

static struct platform_driver mstar_spi_driver = {
	.driver		= {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= mstar_mspi_match,
		.pm = &mstar_spi_pm_ops,
	},
	.probe		= mstar_spi_probe,
	.remove		= mstar_spi_remove,
};

module_platform_driver(mstar_spi_driver);

MODULE_DESCRIPTION("MSPI controller driver for MSTAR");
MODULE_AUTHOR("Vick Sun <vick.sun@mstarsemi.com>");
MODULE_LICENSE("GPL v2");
