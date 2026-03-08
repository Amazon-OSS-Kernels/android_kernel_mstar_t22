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

#include "../../inc/common/ufshcd.h"
#include "../../inc/common/ufshcd-pltfrm.h"
#include "../../inc/common/unipro.h"
#include "../../inc/common/ufshci.h"
#include "../../inc/common/ufs.h"
#include "../../inc/common/ufs-mstar.h"

int ufs_mstar_pwr_change_notify(struct ufs_hba *hba,
                  bool stage, struct ufs_pa_layer_attr *desired,
                  struct ufs_pa_layer_attr *final)

{
    if (PRE_CHANGE == stage) {  /* before power mode change */
        ufs_memcpy(final, desired, sizeof(struct ufs_pa_layer_attr));
        if(final->pwr_rx == SLOWAUTO_MODE || final->pwr_rx == SLOW_MODE)
        {
            if(final->gear_rx > CAP_MSTAR_MAX_RX_PWM_GEAR)
                final->gear_rx = CAP_MSTAR_MAX_RX_PWM_GEAR;
        }
        else if (final->pwr_rx == FASTAUTO_MODE || final->pwr_rx == FAST_MODE)
        {
            if(final->gear_rx > CAP_MSTAR_MAX_RX_HS_GEAR)
                final->gear_rx = CAP_MSTAR_MAX_RX_HS_GEAR;
        }
        if(final->pwr_tx == SLOWAUTO_MODE || final->pwr_tx == SLOW_MODE)
        {
            if(final->gear_tx > CAP_MSTAR_MAX_TX_PWM_GEAR)
                final->gear_tx = CAP_MSTAR_MAX_TX_PWM_GEAR;
        }
        else if (final->pwr_tx == FASTAUTO_MODE || final->pwr_tx == FAST_MODE)
        {
            if(final->gear_tx > CAP_MSTAR_MAX_TX_HS_GEAR)
                final->gear_tx = CAP_MSTAR_MAX_TX_HS_GEAR;
        }
    }
    else
    {
		if(final->pwr_rx == FASTAUTO_MODE || final->pwr_rx == FAST_MODE)
		{
			ufs_mstar_pltfrm_clock(final->gear_rx);
		}
    }

    return 0;
}



static int ufs_request_sense(u32 lun)
{
	int err = 0;
	struct ufs_hba *hba;
	struct ufs_mstar_host *host;
	u8 cmd[MAX_COMMAND_SIZE];
	u8 sense_buffer[SCSI_SENSE_BUFFERSIZE];
	struct scsi_cmnd cmnd;
	struct request request;
	struct scsi_device device;
	int tag;
	u8 sense_data[32];

	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8521);
	hba = ufshcd_get_host();
	host = (struct ufs_mstar_host *)ufshcd_get_variant(hba);

	if(lun >= MAX_UFS_MSTAR_LUN)
	{
		dev_err(hba->dev, "%s:Invalid lun\n", __func__);
		return -EINVAL;
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8522);
	//while(1);

	ufs_memset((void *) &cmnd, 0, sizeof(struct scsi_cmnd));
	ufs_memset((void *) &request, 0, sizeof(struct request));
	ufs_memset((void *) &device, 0, sizeof(struct scsi_device));
	ufs_memset((void *) &cmd[0], 0, MAX_COMMAND_SIZE);
	cmd[0] = REQUEST_SENSE;
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8523);
	request.tag = 0;
	device.lun = lun;
	cmnd.cmnd = cmd;
	cmnd.cmd_len = 6;
	cmnd.request = &request;
	cmnd.sense_buffer = sense_buffer;
	cmnd.device = &device;
	cmnd.sc_data_direction = DMA_FROM_DEVICE;
	cmnd.sdb.length = 32;
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8524);
	err = ufs_mstar_pltfrm_map_sg(&cmnd, sense_data, cmnd.sdb.length);
	if(err)
	{
		dev_err(hba->dev, "%s:Failed maping sg err = %d\n", __func__, err);
		err = -EIO;
		goto out;
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8525);
	err = ufshcd_queuecommand(hba, &cmnd);

out:

	return err;
}

int ufs_init(void)
{
	int err;
	struct platform_device *pdev;	

	pdev = (struct platform_device *)CONFIG_MSTAR_UFS_PDEV_VARIABLE_ADDR;//kmalloc(sizeof(struct platform_device), GFP_KERNEL);     //should be static memory
	if(!pdev)
	{
		dev_err(NULL, "%s:kmalloc failed\n", __func__);
		return -ENOMEM;
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8510);

	err = ufshcd_pltfrm_init(pdev);
	if(err)
	{
		dev_err(&pdev->dev, "Failed probing ufs err = %d\n", err);
		return err;
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8520);

    err = ufs_request_sense(0);
	if(err)
		return err;
    err = ufs_request_sense(1);

	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8526);
	return err;
}


static int ufs_rw10(u32 lun, void *pBuf, u32 dLba, u16 wLength, u8 bOp)
{
	int err = 0;
	struct ufs_hba *hba;
	struct ufs_mstar_host *host;
	u8 cmd[MAX_COMMAND_SIZE];
	struct scsi_cmnd cmnd;
	struct request request;
	struct scsi_device device;

	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8530);

	hba = ufshcd_get_host();
	host = (struct ufs_mstar_host *)ufshcd_get_variant(hba);

	if(lun >= MAX_UFS_MSTAR_LUN)
	{
		dev_err(hba->dev, "%s:Invalid lun\n", __func__);
		return -EINVAL;
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8531);
	ufs_memset((void *) &cmnd, 0, sizeof(struct scsi_cmnd));
	ufs_memset((void *) &request, 0, sizeof(struct request));
	ufs_memset((void *) &device, 0, sizeof(struct scsi_device));
	ufs_memset((void *) &cmd[0], 0, MAX_COMMAND_SIZE);
	cmd[0] = bOp;
	//cmd[1] = 0;
	cmd[2] = (u8) (dLba >> 24) & 0xff;
	cmd[3] = (u8) (dLba >> 16) & 0xff;
	cmd[4] = (u8) (dLba >> 8) & 0xff;
	cmd[5] = (u8) (dLba) & 0xff;
	//cmd[6] = 0;
	cmd[7] = (u8) (wLength >> 8) & 0xff;
	cmd[8] = (u8) wLength & 0xff;	
	//cmd[9] = 0;

	request.tag = 0;
	device.lun = lun;
	cmnd.cmnd = cmd;
	cmnd.cmd_len = 10;
	cmnd.request = &request;
	cmnd.device = &device;
	cmnd.sc_data_direction = (bOp==READ_10)?DMA_FROM_DEVICE:DMA_TO_DEVICE;
	cmnd.sdb.length = /*host->dLogicalBlkSize[lun]*/ 4096 * wLength;
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8540);
	err = ufs_mstar_pltfrm_map_sg(&cmnd, pBuf, cmnd.sdb.length);
	if(err)
	{
		dev_err(hba->dev, "%s:Failed maping sg err = %d\n", err, __func__);
		err = -EIO;
		goto out;
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L1, 0x8550);

	err = ufshcd_queuecommand(hba, &cmnd);

out:
	return err;
}

int ufs_read10(u32 lun, void *pBuf, u32 dLba, u16 wLength)
{
	int err = 0;

	err = ufs_rw10(lun, pBuf, dLba, wLength, READ_10);
	if(err)
	{
		//pr_err("Failed reading10 err = %d\n", err);
		return err;
	}

	return err;
}

module_platform_driver(ufs_mstar_pltform);
