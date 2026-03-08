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

#ifndef UFS_MSTAR_H_
#define UFS_MSTAR_H_

#include "../config/ufs-mstar-pltfrm.h"

struct ufshcd_lrb;

int ufs_init(void);
int ufs_read6(u32 lun, void *pBuf, u32 dLba, u16 wLength);
int ufs_read10(u32 lun, void *pBuf, u32 dLba, u16 wLength);
int ufs_rom_read10(u32 lun, void *pBuf, u32 dLba, u16 wLength);
int ufs_write6(u32 lun, void *pBuf, u32 dLba, u16 wLength);
int ufs_write10(u32 lun, void *pBuf, u32 dLba, u16 wLength);
void ufs_mstar_print_request(struct ufshcd_lrb *lrbp);
void ufs_mstar_print_response(struct ufshcd_lrb *lrbp);

#define MAX_UFS_MSTAR_HOSTS	1
#define MAX_UFS_MSTAR_LUN	3
// 0:user lun
// 1:boot lun 1
// 2:boot lun 2

struct ufs_mstar_host {

	struct ufs_hba *hba;
	u32 dLogicalBlkSize[MAX_UFS_MSTAR_LUN];
	u64 qLogicalBlkCnt[MAX_UFS_MSTAR_LUN];
};

#endif /* UFS_MSTAR_H_ */
