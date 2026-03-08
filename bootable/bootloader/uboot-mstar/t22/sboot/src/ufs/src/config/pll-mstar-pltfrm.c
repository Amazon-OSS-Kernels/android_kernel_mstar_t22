
#include "pll-mstar-pltfrm.h"

struct PHY_INIT_DATA
{
	U8 reg;
	U8 mask;
	U8 value;
}PHY_INIT_DATA;

struct PHY_INIT_DATA phy_init_table[] =
{
	{0xFF, 0xFF,							0x00},
	{0x10, BIT0,							0x01<<0},
	{0x10, BIT1,							0x01<<1},
	
	{0xFF, 0xFF,							0x20},
	{0x9C, 0xFF,							0x80},
	{0x9D, 0xFF,							0x01},	
	{0xFF, 0xFF,							0x21},
	{0x9C, 0xFF,							0x80},
	{0x9D, 0xFF,							0x01},
	
	{0xFF, 0xFF,							0x20},
	{0xA9, BIT2,							0x01<<2},
	{0xA9, BIT7|BIT6|BIT5|BIT4,				0x0F<<4},
	{0xFF, 0xFF,							0x21},
	{0xA9, BIT2,							0x01<<2},
	{0xA9, BIT7|BIT6|BIT5|BIT4,				0x0F<<4},
	{0xFF, 0xFF,							0x22},
	{0xA9, BIT2,							0x01<<2},
	{0xA9, BIT7|BIT6|BIT5|BIT4,				0x0F<<4},
	{0xFF, 0xFF,							0x23},
	{0xA9, BIT2,							0x01<<2},
	{0xA9, BIT7|BIT6|BIT5|BIT4,				0x0F<<4},
	{0xFF, 0xFF,							0x28},
	{0x02, BIT3|BIT2|BIT1|BIT0,				0x06<<0},
	{0x11, BIT0,							0x01<<0},
	{0xFF, 0xFF,							0x29},
	{0x02, BIT3|BIT2|BIT1|BIT0,				0x06<<0},
	{0x11, BIT0,							0x01<<0},
	{0xFF, 0xFF,							0x2A},
	{0x02, BIT3|BIT2|BIT1|BIT0,				0x06<<0},
	{0x11, BIT0,							0x01<<0},
	{0xFF, 0xFF,							0x2B},
	{0x02, BIT3|BIT2|BIT1|BIT0,				0x06<<0},
	{0x11, BIT0,							0x01<<0},
	{0xFF, 0xFF,							0x01},
	{0x01, BIT7|BIT6|BIT5|BIT4|BIT3|BIT2,	0x25<<2},
	{0xFF, 0xFF,							0x18},
	{0x00, BIT5|BIT4|BIT3|BIT2|BIT1|BIT0,	0x0F<<0},
	{0xFF, 0xFF,							0x19},
	{0x00, BIT5|BIT4|BIT3|BIT2|BIT1|BIT0,	0x0F<<0},
	{0xFF, 0xFF,							0x28},
	{0x03, BIT7|BIT6|BIT5|BIT4|BIT3|BIT2,	0x2E<<2},
	{0xFF, 0xFF, 							0x29},
	{0x03, BIT7|BIT6|BIT5|BIT4|BIT3|BIT2,	0x2D<<2},
	{0xFF, 0xFF, 							0x2A},
	{0x03, BIT7|BIT6|BIT5|BIT4|BIT3|BIT2,	0x2D<<2},
	{0xFF, 0xFF, 							0x2B},
	{0x03, BIT7|BIT6|BIT5|BIT4|BIT3|BIT2,	0x2E<<2},
};

static U32 HAL_DELAY_US(U32 u32usTick)
{

//	drvTimer_Enable(u32usTick, 1);
	// Use PIU timer
#if 0
	U32 u32HWTimer = 0;
	volatile U16 u16TimerLow = 0;
	volatile U16 u16TimerHigh = 0;

	// reset HW timer
	REG_WRITE_UINT16(TIMER0_MAX_LOW, 0xFFFF);
	REG_WRITE_UINT16(TIMER0_MAX_HIGH, 0xFFFF);
	REG_WRITE_UINT16(TIMER0_ENABLE, 0);

	// start HW timer
	REG_SET_BITS_UINT16(TIMER0_ENABLE, 0x0001);

	while( u32HWTimer < 12*u32usTick ) // wait for u32usTick micro seconds
	{
		REG_READ_UINT16(TIMER0_CAP_LOW, u16TimerLow);
		REG_READ_UINT16(TIMER0_CAP_HIGH, u16TimerHigh);

		u32HWTimer = (u16TimerHigh<<16) | u16TimerLow;
	}

	REG_WRITE_UINT16(TIMER0_ENABLE, 0);
#endif
    return u32usTick+1;
}

static void gpio_enable(U16 port)
{
	if(port == SCL_PORT)
		REG_CLR_BITS_UINT16(REG_I2C_SCL_ADDR, BIT_SCL_OEN);
	else if(port == SDA_PORT)
		REG_CLR_BITS_UINT16(REG_I2C_SDA_ADDR, BIT_SDA_OEN);
}

static void gpio_set(U16 port, U8 value)
{
	if(port == SCL_PORT)
	{
		if(value == 1)
			REG_SET_BITS_UINT16(REG_I2C_SCL_ADDR, BIT_SCL_OUT);
		else
			REG_CLR_BITS_UINT16(REG_I2C_SCL_ADDR, BIT_SCL_OUT);
	}
	else if(port == SDA_PORT)
	{
		if(value == 1)
			REG_SET_BITS_UINT16(REG_I2C_SDA_ADDR, BIT_SDA_OUT);
		else
			REG_CLR_BITS_UINT16(REG_I2C_SDA_ADDR, BIT_SDA_OUT);
	}
}

static U8 gpio_get(U16 port)
{
	U8 value = 1;

	if(port == SCL_PORT)
	{
		value = (U8)((REG(REG_I2C_SCL_ADDR)&BIT_SCL_IN)>>BIT_SCL_IN_SHIFT);
	}
	else if(port == SDA_PORT)
	{
		value = (U8)((REG(REG_I2C_SDA_ADDR)&BIT_SDA_IN)>>BIT_SDA_IN_SHIFT);
	}

	return value;
}

static void i2c_init(void)
{
	
	REG_CLR_BITS_UINT16(REG_ALL_PAD_IN_ADDR, (1<<15));

	/* Set SDA & SCL to High */
	gpio_set(SDA_PORT, 1);
	gpio_set(SCL_PORT, 1);

	/* Enable SDA & SCL gpio port */
	gpio_enable(SDA_PORT);
	gpio_enable(SCL_PORT);
}

static void i2c_start(void)
{
	/* I2C start sequence is defined as 
 	* a High to Low Transition on the data
 	* line as the CLK pin is high */

	gpio_set(SDA_PORT, 1);  /* SDA: High */
	gpio_set(SCL_PORT, 1);  /* SCL: High */
	HAL_DELAY_US(I2C_DELAY_TIME);

	gpio_set(SDA_PORT, 0);  /* SDA: Low */
	gpio_set(SCL_PORT, 0);  /* SCL: Low */
	HAL_DELAY_US(I2C_DELAY_TIME);
}

static void i2c_stop(void)
{  
	/* I2C stop sequence is defined as 
	* data pin is low, then CLK pin is high,
	* finally data pin is high. */

	gpio_set(SDA_PORT, 0);  /* SDA: Low */
	gpio_set(SCL_PORT, 1);  /* SCL: High */
	gpio_set(SDA_PORT, 1);  /* SDA: High */
}

static U8 i2c_write(U8 data)
{  
	/* An I2C output byte is bits 7-0
	* (MSB to LSB).  Shift one bit at a time
	* to the MDO output, and then clock the
	* data to the I2C Slave */

	U8 i;
	U8 ack;

	/* Write to slave */
	for(i = 0; i < 8; i++) 
	{
		gpio_set(SDA_PORT, (data&0x80)?1:0); /* Send data bit */
		data <<= 1;   /* Shift one bit */
		gpio_set(SCL_PORT, 1); /* SCL: High */
		HAL_DELAY_US(I2C_DELAY_TIME);
		gpio_set(SCL_PORT, 0); /* SCL: Low */
		HAL_DELAY_US(I2C_DELAY_TIME);
	}

	/* Read ACK bit from slave */
	REG_SET_BITS_UINT16(REG_I2C_SDA_ADDR, BIT_SDA_OEN);
	gpio_set(SCL_PORT, 1);  /* SCL: High */
	HAL_DELAY_US(I2C_DELAY_TIME);
	ack = gpio_get(SDA_PORT);
	gpio_set(SCL_PORT, 0);  /* SCL: Low */
	REG_CLR_BITS_UINT16(REG_I2C_SDA_ADDR, BIT_SDA_OEN);
	HAL_DELAY_US(I2C_DELAY_TIME);

	return ack;
}

static U8 i2c_read(U8 send_ack)
{
	U8 i, data;

	data = 0x00;

	REG_SET_BITS_UINT16(REG_I2C_SDA_ADDR, BIT_SDA_OEN);
	HAL_DELAY_US(1);
	/* Read from slave */
	for(i = 0; i < 8; i++) 
	{
		data <<= 1;   /* Shift one bit */
		data |= gpio_get(SDA_PORT); /* Read data bit */
		gpio_set(SCL_PORT, 1); /* SCL: High */
		HAL_DELAY_US(I2C_DELAY_TIME);
		gpio_set(SCL_PORT, 0); /* SCL: Low */
		HAL_DELAY_US(I2C_DELAY_TIME);
	}
	
	/* Send ACK bit to slave */
	if(send_ack)
		gpio_set(SDA_PORT, 0); /* SDA: Low */
	else
		gpio_set(SDA_PORT, 1); /* SDA: High */

	REG_CLR_BITS_UINT16(REG_I2C_SDA_ADDR, BIT_SDA_OEN);

	gpio_set(SCL_PORT, 1);  /* SCL: High */
	HAL_DELAY_US(I2C_DELAY_TIME);
	gpio_set(SCL_PORT, 0);  /* SCL: Low */

	HAL_DELAY_US(I2C_DELAY_TIME);

	return data;
}

static int phy_write(U8 id , U8 reg, U8 val)
{
	i2c_start();

	if(i2c_write(id))
	{
		//printf("no ack, line %d\n", __LINE__);
    	return -1;
	}

	if(i2c_write(reg))
	{
		//printf("no ack, line %d\n", __LINE__);
    	return -1;
	}
	
	if(i2c_write(val))
	{
		//printf("no ack, line %d\n", __LINE__);
    	return -1;
	}	
	i2c_stop();

	return 0;
}

static int phy_read(U8 id , U8 reg)
{
	U8 val;

	i2c_start();
	if(i2c_write(id))
	{
		//printf("no ack, line %d\n", __LINE__);
		return -1;
	}
	if(i2c_write(reg))
	{
		//printf("no ack, line %d\n", __LINE__);
		return -1;
	}

	gpio_set(SDA_PORT, 1); /* Send data bit */
	gpio_set(SCL_PORT, 1);  /* SCL: High */
	HAL_DELAY_US(I2C_DELAY_TIME);
	gpio_set(SDA_PORT, 0); 
	gpio_set(SCL_PORT, 0);  /* SCL: Low */
	
	
	if(i2c_write(id+1))
	{
		//printf("no ack, line %d\n", __LINE__);
		return -1;
	}

	val = i2c_read(1);

	i2c_stop();

	return val;
}

int phy_init(void)
{
	U16 i;
	int ret = 0;

	i2c_init();

	for(i=0 ; i<sizeof(phy_init_table)/sizeof(struct PHY_INIT_DATA) ; i++)
	{

		ret = phy_read(ADDR_PHY, phy_init_table[i].reg);
		if(ret == -1)
		{
			//printf("phy_read %X fail\n", phy_init_table[i].reg);
	    	return ret;
		}
		
		ret &= ~(phy_init_table[i].mask);
		ret |= phy_init_table[i].value;
		
		ret = phy_write(ADDR_PHY, phy_init_table[i].reg, ret);
		if(ret == -1)
		{
			//printf("phy_write %X fail\n", phy_init_table[i].reg);
	    	return ret;
		}
		
		ret = phy_read(ADDR_PHY, phy_init_table[i].reg);
		if(ret == -1)
		{
			//printf("phy_read2 %X fail\n", phy_init_table[i].reg);
	    	return ret;
		}

		//printf("phy_read reg %X = %X\n", phy_init_table[i].reg, ret);
	}

	return 0;
}

