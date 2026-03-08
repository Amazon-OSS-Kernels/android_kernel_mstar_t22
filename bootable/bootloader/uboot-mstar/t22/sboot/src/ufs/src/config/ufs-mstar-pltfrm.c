/*
 * Copyright (c) 2013-2016, Linux Foundation. All rights reserved.
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

#include "../../inc/config/ufs-mstar-pltfrm.h"

int ufs_mstar_pltfrm_init(void)
{
    // by system design, hardcode now
    REG_WRITE_UINT16(REG_UFSHCI_MI0_START_L, 0x0200);
    REG_WRITE_UINT16(REG_UFSHCI_MI0_START_H, 0x0000);
    REG_WRITE_UINT16(REG_UFSHCI_MI0_END_L, 0x0800);
    REG_WRITE_UINT16(REG_UFSHCI_MI0_END_H, 0x0000);

	return 0;
}

int ufs_mstar_pltfrm_clock(int gear_rx)		//rom code only set series A
{
	REG_SET_BITS_UINT16(REG_CKG_UFSHCI, BIT5);

	if(gear_rx == 1)
	{
		REG_CLR_BITS_UINT16(REG_BANK3021_0x3D, (BIT14-1));//[0:13] = 0x1D4
		REG_SET_BITS_UINT16(REG_BANK3021_0x3D, 0x1D4);
		REG_CLR_BITS_UINT16(REG_BANK3021_0x3E, (BIT8-1));//[0:7] = 3
		REG_SET_BITS_UINT16(REG_BANK3021_0x3E, 3);
		REG_CLR_BITS_UINT16(REG_BANK3021_0x4A, (BIT8-1) << 8);//[15:8] = 5
		REG_SET_BITS_UINT16(REG_BANK3021_0x4A, (5) << 8);
	}
	else if(gear_rx == 2)
	{
		REG_CLR_BITS_UINT16(REG_BANK3021_0x3D, (BIT14-1));//[0:13] = 0x3A8
		REG_SET_BITS_UINT16(REG_BANK3021_0x3D, 0x3A8);
		REG_CLR_BITS_UINT16(REG_BANK3021_0x3E, (BIT8-1));//[0:7] = 5
		REG_SET_BITS_UINT16(REG_BANK3021_0x3E, 5);
		REG_CLR_BITS_UINT16(REG_BANK3021_0x4A, (BIT8-1) << 8);//[15:8] = 10
		REG_SET_BITS_UINT16(REG_BANK3021_0x4A, (10) << 8);
	}
	else if(gear_rx == 3)
	{
		REG_CLR_BITS_UINT16(REG_BANK3021_0x3D, (BIT14-1));//[0:13] = 0x750
		REG_SET_BITS_UINT16(REG_BANK3021_0x3D, 0x750);
		REG_CLR_BITS_UINT16(REG_BANK3021_0x3E, (BIT8-1));//[0:7] = 10
		REG_SET_BITS_UINT16(REG_BANK3021_0x3E, 10);
		REG_CLR_BITS_UINT16(REG_BANK3021_0x4A, (BIT8-1) << 8);//[15:8] = 19
		REG_SET_BITS_UINT16(REG_BANK3021_0x4A, (19) << 8);
	}
	REG_SET_BITS_UINT16(REG_BANK3021_0x69, BIT3);

	return 0;
}

int ufs_mstar_pltfrm_map_sg(struct scsi_cmnd *cmndp, void *pBuf, u32 dLength)
{
	#define MAX_ENTRY_SIZE	262144
	int nprd = 0;
	int i;
	u32 len;
	dma_addr_t addr;

	// note1:non-os code , va == ba
	// note2:223C->DBC shall be Dword granularity

	if(dLength)
	{
	if(dLength%4 != 0)
		{
		return -EINVAL;
		}
	len = dLength;
	addr = (dma_addr_t)pBuf;
	nprd = (dLength+MAX_ENTRY_SIZE-1)>>18;

	for(i=0 ; i<nprd ; i++)
	{
		cmndp->sdb.table.sgl[i].dma_address = addr;
		// 0 based
		cmndp->sdb.table.sgl[i].length= len>=MAX_ENTRY_SIZE?(MAX_ENTRY_SIZE):(len);
		addr += (cmndp->sdb.table.sgl[i].length);
		len -= (cmndp->sdb.table.sgl[i].length);
	}

	// flush cache here
	//MsOS_Dcache_Flush(pBuf, dLength);
	}

	cmndp->sdb.table.nents = nprd;

	return 0;

}

u32 ufs_hw_timer_delay(u32 u32usTick)
{
	u32 u32HWTimer = 0;
	volatile u16 u16TimerLow = 0;
	volatile u16 u16TimerHigh = 0;

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

    return u32usTick+1;
}


void __set_bit(int nr, volatile void * addr)
{
	unsigned long * m = ((unsigned long *) addr) + (nr >> 5);

	*m |= 1UL << (nr & 31);
}

void __clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long * m = ((unsigned long *) addr) + (nr >> 5);

	*m &= ~(1UL << (nr & 31));
}


int test_and_set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long old;

	old = (*addr);
	(*addr) |= (1<<nr);

	return (old & (1<<nr)) != 0;
}

void *ufs_memset(void *s, int c, size_t count)
{
	int *Ws = (int *)s;
	char *xs;
	int Wc = (c|(c<<8)|(c<<16)|(c<<24));

	while(count > 4)
	{
		*Ws++ = Wc;
		count-=4;
	}
	xs = (char*) Ws;
	while (count--)
			*xs++ = c;
	return s;
}

void *ufs_memcpy(void *__dest, __const void *__src, size_t __n)
{
        unsigned char *d = (unsigned char *)__dest, *s = (unsigned char *)__src;
		int i;
		#if 0
        while(__n --)
            *d++ = *s++;
        #else

        for (i = __n >> 3; i > 0; i--) {
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
        }

        if (__n & 1 << 2) {
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
        }

        if (__n & 1 << 1) {
                *d++ = *s++;
                *d++ = *s++;
        }

        if (__n & 1)
                *d++ = *s++;
        #endif
        return __dest;
}

