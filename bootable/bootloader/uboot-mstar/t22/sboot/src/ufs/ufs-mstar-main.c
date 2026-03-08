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
#include "inc/common/ufshcd.h"
#include "inc/common/ufs-mstar.h"

//USER LUN: 0
//BOOT LUN: 1
//offset -> byte offset
int ufs_load_boot(unsigned int dwRamAddress, unsigned int dwSramSize, unsigned int offset)
{
	int err = 0; 
	unsigned int LBA_512byte = offset >> 9;
	unsigned int LBA_4k = LBA_512byte >> 3;
	unsigned int LBA_512_in_4k = LBA_512byte & 0x7;	

	err = ufs_init();
	if(err)
	{
		REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xDEA1);
		while(1);
	}
	if(LBA_512_in_4k)	//read partial
	{
		err = ufs_read10(0x1, CONFIG_MSTAR_DRAM_TEMP_BUF, LBA_4k, 1);
		ufs_memcpy(dwRamAddress, CONFIG_MSTAR_DRAM_TEMP_BUF + (LBA_512_in_4k << 9), (8 - LBA_512_in_4k) << 9);
		dwRamAddress += (8 - LBA_512_in_4k) << 9; 
		dwSramSize -= (8 - LBA_512_in_4k) << 9;
		LBA_4k ++;
	}
	
	err = ufs_read10(0x1, (void*)dwRamAddress, LBA_4k, (dwSramSize + 4095) >> 12);	//fake 16k byte alignment 12k + 4k overlapped.
	if(err)
	{
		REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xDEA2);		
		while(1);
	}
	return err;
}

//offset -> byte offset
int ufs_load_user(unsigned int dwRamAddress, unsigned int dwSize, unsigned int LBA_512byte )
{
	int err = 0;
	unsigned int LBA_4k = LBA_512byte >> 3;
	unsigned int LBA_512_in_4k = LBA_512byte & 0x7;

	if(LBA_512_in_4k)	//read partial
	{
		err = ufs_read10(0x0, CONFIG_MSTAR_DRAM_TEMP_BUF, LBA_4k, 1);
		ufs_memcpy(dwRamAddress, CONFIG_MSTAR_DRAM_TEMP_BUF + (LBA_512_in_4k << 9), (8 - LBA_512_in_4k) << 9);
		dwRamAddress += (8 - LBA_512_in_4k) << 9; 
		dwSize -= (8 - LBA_512_in_4k) << 9;
		LBA_4k ++;
	}
	
	err = ufs_read10(0x0, (void*)dwRamAddress, LBA_4k, (dwSize + 4095) >> 12);	//fake 16k byte alignment 12k + 4k overlapped.
	if(err)
	{
		REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0xDEA3);
		while(1);
	}
	return err;
}


