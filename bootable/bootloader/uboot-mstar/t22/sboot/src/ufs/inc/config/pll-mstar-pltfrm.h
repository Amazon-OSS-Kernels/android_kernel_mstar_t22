#ifndef I2C_MSTAR_H_
#define I2C_MSTAR_H_

#include "datatype.h"
#include "drvTimer.h"

#define SCL_PORT				30
#define SDA_PORT				31

#define ADDR_PHY				0xE0 /* 1110 000 x */
#define I2C_DELAY_TIME			10 /* us */

#define RIU_PM_BASE				0x1F000000UL

#define REG_I2C_SCL_ADDR		(RIU_PM_BASE+(0x102BA2<<1)) /* bank 0x102B offset 0x51 */
#define REG_I2C_SDA_ADDR		(RIU_PM_BASE+(0x102BA2<<1)) /* bank 0x102B offset 0x51 */
#define REG_ALL_PAD_IN_ADDR		(RIU_PM_BASE+(0x101EA0<<1)) /* bank 0x101E offset 0x50 */

#define BIT_SCL_OUT_SHIFT		0
#define BIT_SCL_OEN_SHIFT		1
#define BIT_SCL_IN_SHIFT		2
#define BIT_SCL_OUT				(1<<BIT_SCL_OUT_SHIFT)
#define BIT_SCL_OEN				(1<<BIT_SCL_OEN_SHIFT)
#define BIT_SCL_IN				(1<<BIT_SCL_IN_SHIFT)

#define BIT_SDA_OUT_SHIFT		8
#define BIT_SDA_OEN_SHIFT		9
#define BIT_SDA_IN_SHIFT		10
#define BIT_SDA_OUT				(1<<BIT_SDA_OUT_SHIFT)
#define BIT_SDA_OEN				(1<<BIT_SDA_OEN_SHIFT)
#define BIT_SDA_IN				(1<<BIT_SDA_IN_SHIFT)

#define REG(Reg_Addr)			(*(volatile unsigned short*)(Reg_Addr))
#define REG_OFFSET_SHIFT_BITS	2
#define GET_REG_ADDR(x, y)		(x+((y)<<REG_OFFSET_SHIFT_BITS))
#define REG_WRITE_UINT16(reg_addr, val)		REG(reg_addr) = val
#define REG_READ_UINT16(reg_addr, val)		val = REG(reg_addr)
#define REG_SET_BITS_UINT16(reg_addr, val)	REG(reg_addr) |= (val)
#define REG_CLR_BITS_UINT16(reg_addr, val)	REG(reg_addr) &= ~(val)

#define REG_BANK_TIMER0			(0x1800)
#define TIMER0_REG_BASE_ADDR	GET_REG_ADDR(RIU_PM_BASE, REG_BANK_TIMER0)
#define TIMER0_ENABLE			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x20)
#define TIMER0_HIT				GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x21)
#define TIMER0_MAX_LOW			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x22)
#define TIMER0_MAX_HIGH			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x23)
#define TIMER0_CAP_LOW			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x24)
#define TIMER0_CAP_HIGH			GET_REG_ADDR(TIMER0_REG_BASE_ADDR, 0x25)

extern int phy_init(void);

#endif /* I2C_MSTAR_H_ */
