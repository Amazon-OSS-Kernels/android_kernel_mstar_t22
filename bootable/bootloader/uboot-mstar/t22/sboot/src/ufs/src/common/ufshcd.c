/*
 * Universal Flash Storage Host controller driver Core
 *
 * This code is based on drivers/scsi/ufs/ufshcd.c
 * Copyright (C) 2011-2013 Samsung India Software Operations
 * Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * Authors:
 *	Santosh Yaraganavi <santosh.sy@samsung.com>
 *	Vinayak Holikatti <h.vinayak@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * See the COPYING file in the top-level directory or visit
 * <http://www.gnu.org/licenses/gpl-2.0.html>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This program is provided "AS IS" and "WITH ALL FAULTS" and
 * without warranty of any kind. You are solely responsible for
 * determining the appropriateness of using and distributing
 * the program and assume all risks associated with your exercise
 * of rights with respect to the program, including but not limited
 * to infringement of third party rights, the risks and costs of
 * program errors, damage to or loss of data, programs or equipment,
 * and unavailability or interruption of operations. Under no
 * circumstances will the contributor of this Program be liable for
 * any damages of any kind arising from your use or distribution of
 * this program.
 *
 * The Linux Foundation chooses to take subject only to the GPLv2
 * license terms, and distributes only under these terms.
 */

#include "../../inc/common/ufshcd.h"
#include "../../inc/common/unipro.h"
#ifdef CONFIG_MSTAR_UFS
#include "../../inc/common/ufs-mstar.h"
#endif

#define UFSHCD_ENABLE_INTRS	(UTP_TRANSFER_REQ_COMPL |\
				 UTP_TASK_REQ_COMPL |\
				 UFSHCD_ERROR_MASK)
/* UIC command timeout, unit: ms */
#define UIC_CMD_TIMEOUT	500

/* NOP OUT retries waiting for NOP IN response */
#define NOP_OUT_RETRIES    10
/* Timeout after 30 msecs if NOP OUT hangs without response */
#define NOP_OUT_TIMEOUT    30 /* msecs */

/* Query request retries */
#define QUERY_REQ_RETRIES 10
/* Query request timeout */
#define QUERY_REQ_TIMEOUT 30 /* msec */
/*
 * Query request timeout for fDeviceInit flag
 * fDeviceInit query response time for some devices is too large that default
 * QUERY_REQ_TIMEOUT may not be enough for such devices.
 */
#define QUERY_FDEVICEINIT_REQ_TIMEOUT 600 /* msec */

/* Task management command timeout */
#define TM_CMD_TIMEOUT	100 /* msecs */

/* maximum number of link-startup retries */
#define DME_LINKSTARTUP_RETRIES 3

/* maximum number of reset retries before giving up */
#define MAX_HOST_RESET_RETRIES 5

/* Expose the flag value from utp_upiu_query.value */
#define MASK_QUERY_UPIU_FLAG_LOC 0xFF

/* Interrupt aggregation default timeout, unit: 40us */
#define INT_AGGR_DEF_TO	0x02


enum {
	UFSHCD_MAX_CHANNEL	= 0,
	UFSHCD_MAX_ID		= 1,
	UFSHCD_CMD_PER_LUN	= 32,
	UFSHCD_CAN_QUEUE	= 32,
};

/* UFSHCD states */
enum {
	UFSHCD_STATE_RESET,
	UFSHCD_STATE_ERROR,
	UFSHCD_STATE_OPERATIONAL,
};

/* UFSHCD error handling flags */
enum {
	UFSHCD_EH_IN_PROGRESS = (1 << 0),
};

/* UFSHCD UIC layer error flags */
enum {
	UFSHCD_UIC_DL_PA_INIT_ERROR = (1 << 0), /* Data link layer error */
	UFSHCD_UIC_NL_ERROR = (1 << 1), /* Network layer error */
	UFSHCD_UIC_TL_ERROR = (1 << 2), /* Transport Layer error */
	UFSHCD_UIC_DME_ERROR = (1 << 3), /* DME error */
};

/* Interrupt configuration options */
enum {
	UFSHCD_INT_DISABLE,
	UFSHCD_INT_ENABLE,
	UFSHCD_INT_CLEAR,
};

#define ufshcd_set_eh_in_progress(h) \
	(h->eh_flags |= UFSHCD_EH_IN_PROGRESS)
#define ufshcd_eh_in_progress(h) \
	(h->eh_flags & UFSHCD_EH_IN_PROGRESS)
#define ufshcd_clear_eh_in_progress(h) \
	(h->eh_flags &= ~UFSHCD_EH_IN_PROGRESS)

#define ufshcd_set_ufs_dev_active(h) \
	((h)->curr_dev_pwr_mode = UFS_ACTIVE_PWR_MODE)
#define ufshcd_set_ufs_dev_sleep(h) \
	((h)->curr_dev_pwr_mode = UFS_SLEEP_PWR_MODE)
#define ufshcd_set_ufs_dev_poweroff(h) \
	((h)->curr_dev_pwr_mode = UFS_POWERDOWN_PWR_MODE)
#define ufshcd_is_ufs_dev_active(h) \
	((h)->curr_dev_pwr_mode == UFS_ACTIVE_PWR_MODE)
#define ufshcd_is_ufs_dev_sleep(h) \
	((h)->curr_dev_pwr_mode == UFS_SLEEP_PWR_MODE)
#define ufshcd_is_ufs_dev_poweroff(h) \
	((h)->curr_dev_pwr_mode == UFS_POWERDOWN_PWR_MODE)


static int ufshcd_probe_hba(struct ufs_hba *hba);
#if 1
static void ufshcd_uic_cmd_compl(struct ufs_hba *hba, u32 intr_status);
static void ufshcd_transfer_req_compl(struct ufs_hba *hba);
extern int ufs_mstar_pwr_change_notify(struct ufs_hba *hba,
                  bool stage, struct ufs_pa_layer_attr *desired,
                  struct ufs_pa_layer_attr *final);
#endif

/*
 * ufshcd_wait_for_register - wait for register value to change
 * @hba - per-adapter interface
 * @reg - mmio register offset
 * @mask - mask to apply to read register value
 * @val - wait condition
 * @interval_us - polling interval in microsecs
 * @timeout_ms - timeout in millisecs
 *
 * Returns -ETIMEDOUT on error, zero on success
 */
static int ufshcd_wait_for_register(struct ufs_hba *hba, u32 reg, u32 mask,
		u32 val, unsigned long interval_us, unsigned long timeout_ms)
{
	int err = 0;
	unsigned long timeout = (timeout_ms*1000)/interval_us;
	unsigned long i = 0;
    u32 reg_status;

	/* ignore bits that we don't intend to wait on */
	val = val & mask;
    do
    {
		udelay(interval_us);
        reg_status = ufshcd_readl(hba, reg) & mask;
        if((++i) >= timeout) {
			if (reg_status != val)
				err = -ETIMEDOUT;
			break;
		}
    }while(reg_status != val);

	return err;
}

static int ufshcd_wait_for_intr_status(struct ufs_hba *hba, u32 mask, unsigned long timeout_ms)
{
	int err = 0;
	u32 intr_status;
	unsigned long interval_us = 1;
	unsigned long timeout = (timeout_ms*1000)/interval_us;
	unsigned long i = 0;

	intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
	while ((intr_status&mask) == 0) {
		udelay(interval_us);

		intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
		if ((++i) >= timeout) {
			if ((intr_status&mask) == 0)
				err = -ETIMEDOUT;
			break;
		}
	}

	if ((intr_status&mask)) {
		if (intr_status & UFSHCD_UIC_MASK)
			ufshcd_uic_cmd_compl(hba, intr_status);

		if (intr_status & UTP_TRANSFER_REQ_COMPL)
			ufshcd_transfer_req_compl(hba);
	}
	if (intr_status) {
		ufshcd_writel(hba, intr_status, REG_INTERRUPT_STATUS);
	}

	return err;
}

/**
 * ufshcd_enable_run_stop_reg - Enable run-stop registers,
 *			When run-stop registers are set to 1, it indicates the
 *			host controller that it can process the requests
 * @hba: per adapter instance
 */
static void ufshcd_enable_run_stop_reg(struct ufs_hba *hba)
{
	ufshcd_writel(hba, UTP_TASK_REQ_LIST_RUN_STOP_BIT,
		      REG_UTP_TASK_REQ_LIST_RUN_STOP);
	ufshcd_writel(hba, UTP_TRANSFER_REQ_LIST_RUN_STOP_BIT,
		      REG_UTP_TRANSFER_REQ_LIST_RUN_STOP);
}

/**
 * ufshcd_send_uic_cmd - Send UIC commands and retrieve the result
 * @hba: per adapter instance
 * @uic_cmd: UIC command
 *
 * Returns 0 only if success.
 */

static int
ufshcd_send_uic_cmd(struct ufs_hba *hba, struct uic_command *uic_cmd)
{
	int ret = -ETIMEDOUT;

	hba->active_uic_cmd = uic_cmd;

    /*Dispatch*/
	ufshcd_writel(hba, uic_cmd->argument1, REG_UIC_COMMAND_ARG_1);
	ufshcd_writel(hba, uic_cmd->argument2, REG_UIC_COMMAND_ARG_2);
	ufshcd_writel(hba, uic_cmd->argument3, REG_UIC_COMMAND_ARG_3);

	/* Write UIC Cmd */
	ufshcd_writel(hba, uic_cmd->command & COMMAND_OPCODE_MASK,
		      REG_UIC_COMMAND);
    /*===Dispatch===*/

    /*===Wait===*/
	if(ufshcd_wait_for_intr_status(hba, UIC_COMMAND_COMPL, UIC_CMD_TIMEOUT) == 0)
		ret = uic_cmd->argument2 & MASK_UIC_COMMAND_RESULT;

	hba->active_uic_cmd = NULL;

	return ret;
}

/**
 * ufshcd_map_sg - Map scatter-gather list to prdt
 * @lrbp - pointer to local reference block
 *
 * Returns 0 in case of success, non-zero value in case of failure
 */
static int ufshcd_map_sg(struct ufshcd_lrb *lrbp)
{
	struct ufshcd_sg_entry *prd_table;
	struct scatterlist *sg;
	struct scsi_cmnd *cmd;
	int sg_segments;
	int i;

	cmd = lrbp->cmd;
	#if 0
	sg_segments = scsi_dma_map(cmd);
	#else
	sg_segments = cmd->sdb.table.nents;
	#endif
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8531);

	if (sg_segments) {
		REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8532);		
		lrbp->utr_descriptor_ptr->prd_table_length =
					cpu_to_le16((u16) (sg_segments));

		prd_table = (struct ufshcd_sg_entry *)lrbp->ucd_prdt_ptr;
		REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8533); 	

		#if 0
		scsi_for_each_sg(cmd, sg, sg_segments, i) {
		#else
		for(i=0 ; i<sg_segments ; i++) {
			sg = &cmd->sdb.table.sgl[i];
		#endif
			REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8534);
			prd_table[i].size  =
				cpu_to_le32(((u32) sg_dma_len(sg))-1);
			REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8535);			
			prd_table[i].base_addr =
				cpu_to_le32(lower_32_bits(sg->dma_address));
			REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8536);			
			prd_table[i].upper_addr =
				cpu_to_le32(upper_32_bits(sg->dma_address));
			REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8537);			
			prd_table[i].reserved = 0;
			REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8538);			
		}
	} else {
		lrbp->utr_descriptor_ptr->prd_table_length = 0;
		REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8539);		
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x853A);

	return 0;
}

/**
 * ufshcd_prepare_req_desc_hdr() - Fills the requests header
 * descriptor according to request
 * @lrbp: pointer to local reference block
 * @upiu_flags: flags required in the header
 * @cmd_dir: requests data direction
 */
static void ufshcd_prepare_req_desc_hdr(struct ufshcd_lrb *lrbp,
		u32 *upiu_flags, enum dma_data_direction cmd_dir)
{
	struct utp_transfer_req_desc *req_desc = lrbp->utr_descriptor_ptr;
	u32 data_direction;
	u32 dword_0;

	if (cmd_dir == DMA_FROM_DEVICE) {
		data_direction = UTP_DEVICE_TO_HOST;
		*upiu_flags = UPIU_CMD_FLAGS_READ;
	} else {
		data_direction = UTP_NO_DATA_TRANSFER;
		*upiu_flags = UPIU_CMD_FLAGS_NONE;
	}

	#ifdef CONFIG_MSTAR_UFS
	dword_0 = data_direction | (1
				<< UPIU_COMMAND_TYPE_OFFSET);
	#else
	dword_0 = data_direction | (lrbp->command_type
				<< UPIU_COMMAND_TYPE_OFFSET);
	#endif
	if (lrbp->intr_cmd)
		dword_0 |= UTP_REQ_DESC_INT_CMD;
	
	/* Transfer request descriptor header fields */
	req_desc->header.dword_0 = cpu_to_le32(dword_0);
	
	/* dword_1 is reserved, hence it is set to 0 */
	req_desc->header.dword_1 = 0;
	/*
	 * assigning invalid value for command status. Controller
	 * updates OCS on command completion, with the command
	 * status
	 */	 
	req_desc->header.dword_2 =
		cpu_to_le32(OCS_INVALID_COMMAND_STATUS);
	/* dword_3 is reserved, hence it is set to 0 */
	req_desc->header.dword_3 = 0;

	req_desc->prd_table_length = 0;
}

/**
 * ufshcd_prepare_utp_scsi_cmd_upiu() - fills the utp_transfer_req_desc,
 * for scsi commands
 * @lrbp - local reference block pointer
 * @upiu_flags - flags
 */
static
void ufshcd_prepare_utp_scsi_cmd_upiu(struct ufshcd_lrb *lrbp, u32 upiu_flags)
{
	struct utp_upiu_req *ucd_req_ptr = lrbp->ucd_req_ptr;

	/* command descriptor fields */
	
	ucd_req_ptr->header.dword_0 = UPIU_HEADER_DWORD(
				UPIU_TRANSACTION_COMMAND, upiu_flags,
				lrbp->lun, lrbp->task_tag);
	ucd_req_ptr->header.dword_1 = UPIU_HEADER_DWORD(
				UPIU_COMMAND_SET_TYPE_SCSI, 0, 0, 0);

	/* Total EHS length and Data segment length will be zero */
	ucd_req_ptr->header.dword_2 = 0;

	ucd_req_ptr->sc.exp_data_transfer_len =
		cpu_to_be32(lrbp->cmd->sdb.length);

	ufs_memset(ucd_req_ptr->sc.cdb, 0, MAX_CDB_SIZE);
	ufs_memcpy(ucd_req_ptr->sc.cdb, lrbp->cmd->cmnd,
		(min_t(unsigned short, lrbp->cmd->cmd_len, MAX_CDB_SIZE)));
	ufs_memset(lrbp->ucd_rsp_ptr, 0, sizeof(struct utp_upiu_rsp));
}

static void ufshcd_prepare_utp_nop_upiu(struct ufshcd_lrb *lrbp)
{
	struct utp_upiu_req *ucd_req_ptr = lrbp->ucd_req_ptr;

	ufs_memset(ucd_req_ptr, 0, sizeof(struct utp_upiu_req));

	/* command descriptor fields */
	ucd_req_ptr->header.dword_0 =
		UPIU_HEADER_DWORD(
			UPIU_TRANSACTION_NOP_OUT, 0, 0, lrbp->task_tag);
	/* clear rest of the fields of basic header */
	ucd_req_ptr->header.dword_1 = 0;
	ucd_req_ptr->header.dword_2 = 0;

	ufs_memset(lrbp->ucd_rsp_ptr, 0, sizeof(struct utp_upiu_rsp));
}

/**
 * ufshcd_prepare_utp_query_req_upiu() - fills the utp_transfer_req_desc,
 * for query requsts
 * @hba: UFS hba
 * @lrbp: local reference block pointer
 * @upiu_flags: flags
 */
static void ufshcd_prepare_utp_query_req_upiu(struct ufs_hba *hba,
				struct ufshcd_lrb *lrbp, u32 upiu_flags)
{
	struct utp_upiu_req *ucd_req_ptr = lrbp->ucd_req_ptr;
	struct ufs_query *query = &hba->dev_cmd.query;
	u16 len = be16_to_cpu(query->request.upiu_req.length);
	u8 *descp = (u8 *)lrbp->ucd_req_ptr + GENERAL_UPIU_REQUEST_SIZE;

	/* Query request header */
	ucd_req_ptr->header.dword_0 = UPIU_HEADER_DWORD(
			UPIU_TRANSACTION_QUERY_REQ, upiu_flags,
			lrbp->lun, lrbp->task_tag);
	ucd_req_ptr->header.dword_1 = UPIU_HEADER_DWORD(
			0, query->request.query_func, 0, 0);

	/* Data segment length */
	ucd_req_ptr->header.dword_2 = UPIU_HEADER_DWORD(
			0, 0, len >> 8, (u8)len);

	/* Copy the Query Request buffer as is */
	ufs_memcpy(&ucd_req_ptr->qr, &query->request.upiu_req,
			QUERY_OSF_SIZE);

	/* Copy the Descriptor */
	if (query->request.upiu_req.opcode == UPIU_QUERY_OPCODE_WRITE_DESC)
		ufs_memcpy(descp, query->descriptor, len);

	ufs_memset(lrbp->ucd_rsp_ptr, 0, sizeof(struct utp_upiu_rsp));
}

/**
 * ufshcd_compose_upiu - form UFS Protocol Information Unit(UPIU)
 * @hba - per adapter instance
 * @lrb - pointer to local reference block
 */
static int ufshcd_compose_upiu(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	u32 upiu_flags;
	int ret = 0;

	switch (lrbp->command_type) {
	case UTP_CMD_TYPE_SCSI:
		if (likely(lrbp->cmd)) {			
			ufshcd_prepare_req_desc_hdr(lrbp, &upiu_flags,
					lrbp->cmd->sc_data_direction);
			ufshcd_prepare_utp_scsi_cmd_upiu(lrbp, upiu_flags);
			
		} else {
			ret = -EINVAL;
		}
		break;
	case UTP_CMD_TYPE_DEV_MANAGE:
		ufshcd_prepare_req_desc_hdr(lrbp, &upiu_flags, DMA_NONE);
		if (hba->dev_cmd.type == DEV_CMD_TYPE_QUERY)
			ufshcd_prepare_utp_query_req_upiu(
					hba, lrbp, upiu_flags);
		else if (hba->dev_cmd.type == DEV_CMD_TYPE_NOP)
			ufshcd_prepare_utp_nop_upiu(lrbp);
		else
			ret = -EINVAL;
		break;
	case UTP_CMD_TYPE_UFS:
		/* For UFS native command implementation */
		ret = -ENOTSUPP;
		dev_err(hba->dev, "%s: UFS native command are not supported\n",
			__func__);
		break;
	default:
		ret = -ENOTSUPP;
		dev_err(hba->dev, "%s: unknown command type: 0x%x\n",
				__func__, lrbp->command_type);
		break;
	} /* end of switch */

	return ret;
}

/**
 * ufshcd_queuecommand - main entry point for SCSI requests
 * @cmd: command from SCSI Midlayer
 * @done: call back function
 *
 * Returns 0 for success, non-zero in case of failure
 */
static inline u8 ufshcd_scsi_to_upiu_lun(unsigned int scsi_lun)
{
	if (scsi_is_wlun(scsi_lun))
		return (scsi_lun & UFS_UPIU_MAX_UNIT_NUM_ID)
			| UFS_UPIU_WLUN_ID;
	else
		return scsi_lun & UFS_UPIU_MAX_UNIT_NUM_ID;
}

int ufshcd_queuecommand(struct ufs_hba *hba, struct scsi_cmnd *cmd)
{
	struct ufshcd_lrb *lrbp;
	int tag;
	int err = 0;
	u32 intr_status;
	unsigned long i = 0;

	tag = cmd->request->tag;

	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8520);

	/* acquire the tag to make sure device cmds don't use it */
	if (test_and_set_bit_lock(tag, &hba->lrb_in_use)) {
		/*
		 * Dev manage command in progress, requeue the command.
		 * Requeuing the command helps in cases where the request *may*
		 * find different tag instead of waiting for dev manage command
		 * completion.
		 */
		err = SCSI_MLQUEUE_HOST_BUSY;
		goto out;
	}

	if (err) {
		err = SCSI_MLQUEUE_HOST_BUSY;
		clear_bit_unlock(tag, &hba->lrb_in_use);
		goto out;
	}
	lrbp = &hba->lrb[tag];

	lrbp->cmd = cmd;
	lrbp->sense_bufflen = SCSI_SENSE_BUFFERSIZE;
	lrbp->sense_buffer = cmd->sense_buffer;
	lrbp->task_tag = tag;
	lrbp->lun = ufshcd_scsi_to_upiu_lun(cmd->device->lun);;
	lrbp->intr_cmd = true;
	lrbp->command_type = UTP_CMD_TYPE_SCSI;
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8530);

	/* form UPIU before issuing the command */
	ufshcd_compose_upiu(hba, lrbp);
	err = ufshcd_map_sg(lrbp);
	if (err) {
		lrbp->cmd = NULL;
		clear_bit_unlock(tag, &hba->lrb_in_use);
		goto out;
	}

	/* issue command to the controller */

	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8540);

	REG_WRITE_UINT16(REG_UFSHCI_0x70, 0x0);

	//senc_commands
	__set_bit(tag, &hba->outstanding_reqs);
	ufshcd_writel(hba, 1 << tag, REG_UTP_TRANSFER_REQ_DOOR_BELL);

	//wait for intr
	while ((ufshcd_readl(hba, REG_INTERRUPT_STATUS) & UTP_TRANSFER_REQ_COMPL) == 0) {
		udelay(1);
		i ++;
		if (i >= BLK_DEFAULT_SG_TIMEOUT * 1000) {
			if ((ufshcd_readl(hba, REG_INTERRUPT_STATUS) & UTP_TRANSFER_REQ_COMPL) == 0)
				err = ETIMEDOUT;
			break;
		}
		if((ufshcd_readl(hba, REG_INTERRUPT_STATUS) & UIC_ERROR) == UIC_ERROR)
		{
			if(REG(REG_UFSHCI_0x70))
			{
				err = EIO;
				break;
			}
			else
			{
				REG_WRITE_UINT16(REG_UFSHCI_0x70, 0x1);
				ufshcd_writel(hba, UIC_ERROR, REG_INTERRUPT_STATUS);
			}
		}
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8550);
	intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);

	if ((intr_status&UTP_TRANSFER_REQ_COMPL)) {
		if (intr_status & UFSHCD_UIC_MASK)
			ufshcd_uic_cmd_compl(hba, intr_status);

		if (intr_status & UTP_TRANSFER_REQ_COMPL)
			ufshcd_transfer_req_compl(hba);
		err = cmd->result;
	}
	if (intr_status) {
		ufshcd_writel(hba, intr_status, REG_INTERRUPT_STATUS);
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8560);
	out:
	REG_WRITE_UINT16(DEBUG_REG_SYS_L3, err>>16);
	REG_WRITE_UINT16(DEBUG_REG_SYS_L4, err);	
		
	return err;
}

static int ufshcd_compose_dev_cmd(struct ufs_hba *hba,
		struct ufshcd_lrb *lrbp, enum dev_cmd_type cmd_type, int tag)
{
	lrbp->cmd = NULL;
	lrbp->sense_bufflen = 0;
	lrbp->sense_buffer = NULL;
	lrbp->task_tag = tag;
	lrbp->lun = 0; /* device management cmd is not specific to any LUN */
	lrbp->command_type = UTP_CMD_TYPE_DEV_MANAGE;
	lrbp->intr_cmd = true; /* No interrupt aggregation */
	hba->dev_cmd.type = cmd_type;

	return ufshcd_compose_upiu(hba, lrbp);
}

static int
ufshcd_clear_cmd(struct ufs_hba *hba, int tag)
{
	int err = 0;
	u32 mask = 1 << tag;

	/* clear outstanding transaction before retry */
	ufshcd_writel(hba, ~(1 << tag), REG_UTP_TRANSFER_REQ_LIST_CLEAR);

	/*
	 * wait for for h/w to clear corresponding bit in door-bell.
	 * max. wait is 1 sec.
	 */
	err = ufshcd_wait_for_register(hba,
			REG_UTP_TRANSFER_REQ_DOOR_BELL,
			mask, ~mask, 1000, 1000);

	return err;
}
static inline int
ufshcd_get_rsp_upiu_result(struct utp_upiu_rsp *ucd_rsp_ptr)
{
	return be32_to_cpu(ucd_rsp_ptr->header.dword_1) & MASK_RSP_UPIU_RESULT;
}

static int
ufshcd_check_query_response(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufs_query_res *query_res = &hba->dev_cmd.query.response;

	/* Get the UPIU response */
	query_res->response = ufshcd_get_rsp_upiu_result(lrbp->ucd_rsp_ptr) >>
				UPIU_RSP_CODE_OFFSET;
	return query_res->response;
}

/**
 * ufshcd_copy_query_response() - Copy the Query Response and the data
 * descriptor
 * @hba: per adapter instance
 * @lrb - pointer to local reference block
 */
static
int ufshcd_copy_query_response(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct ufs_query_res *query_res = &hba->dev_cmd.query.response;

	ufs_memcpy(&query_res->upiu_res, &lrbp->ucd_rsp_ptr->qr, QUERY_OSF_SIZE);

	/* Get the descriptor */
	if (lrbp->ucd_rsp_ptr->qr.opcode == UPIU_QUERY_OPCODE_READ_DESC) {
		u8 *descp = (u8 *)lrbp->ucd_rsp_ptr +
				GENERAL_UPIU_REQUEST_SIZE;
		u16 resp_len;
		u16 buf_len;

		/* data segment length */
		resp_len = be32_to_cpu(lrbp->ucd_rsp_ptr->header.dword_2) &
						MASK_QUERY_DATA_SEG_LEN;
		buf_len = be16_to_cpu(
				hba->dev_cmd.query.request.upiu_req.length);
		if (likely(buf_len >= resp_len)) {
			ufs_memcpy(hba->dev_cmd.query.descriptor, descp, resp_len);
		} else {
			dev_warn(hba->dev,
				"%s: Response size is bigger than buffer",
				__func__);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * ufshcd_dev_cmd_completion() - handles device management command responses
 * @hba: per adapter instance
 * @lrbp: pointer to local reference block
 */
static int
ufshcd_dev_cmd_completion(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	int resp;
	int err = 0;

	resp = be32_to_cpu(lrbp->ucd_rsp_ptr->header.dword_0) >> 24;

	switch (resp) {
	case UPIU_TRANSACTION_NOP_IN:
        /*
		if (hba->dev_cmd.type != DEV_CMD_TYPE_NOP) {
			err = -EINVAL;
			dev_err(hba->dev, "%s: unexpected response %x\n",
					__func__, resp);
		}
		*/
		break;
	case UPIU_TRANSACTION_QUERY_RSP:
		err = ufshcd_check_query_response(hba, lrbp);
		if (!err)
			err = ufshcd_copy_query_response(hba, lrbp);
		break;		
	case UPIU_TRANSACTION_REJECT_UPIU:
		/* TODO: handle Reject UPIU Response */
		err = -EPERM;
		dev_err(hba->dev, "%s: Reject UPIU not fully implemented\n",
				__func__);
		break;
	default:
		err = -EINVAL;
		dev_err(hba->dev, "%s: Invalid device management cmd response: %x\n",
				__func__, resp);
		break;
	}

	return err;
}

static int ufshcd_wait_for_dev_cmd(struct ufs_hba *hba,
		struct ufshcd_lrb *lrbp, int time_left)
{
	int err = 0;

	#if 0
	err = ufshcd_wait_for_intr_status(hba, UTP_TRANSFER_REQ_COMPL, max_timeout);
	if(err)
		time_left = 0;
	else
		time_left = 1;
	#endif

	hba->dev_cmd.complete = NULL;
	if (likely(time_left)) {
		err = le32_to_cpu(lrbp->utr_descriptor_ptr->header.dword_2) & MASK_OCS;
		if (!err)
			err = ufshcd_dev_cmd_completion(hba, lrbp);
		#ifdef CONFIG_MSTAR_UFS_DEBUG
		else
		    dev_err(hba->dev, "ufshcd_wait_for_dev_cmd - ufshcd_get_tr_ocs fails: %x\n", err);
		#endif
	}

	if (!time_left) {
		err = -ETIMEDOUT;
		dev_dbg(hba->dev, "%s: dev_cmd request timedout, tag %d\n",
			__func__, lrbp->task_tag);
		if (!ufshcd_clear_cmd(hba, lrbp->task_tag))
			/* successfully cleared the command, retry if needed */
			err = -EAGAIN;
	}

	return err;
}

/**
 * ufshcd_get_dev_cmd_tag - Get device management command tag
 * @hba: per-adapter instance
 * @tag: pointer to variable with available slot value
 *
 * Get a free slot and lock it until device management command
 * completes.
 *
 * Returns false if free slot is unavailable for locking, else
 * return true with tag value in @tag.
 */
bool ufshcd_get_dev_cmd_tag(struct ufs_hba *hba, int *tag_out)
{
	bool ret = false;

	if (!tag_out)
		goto out;

    test_and_set_bit_lock(0, &hba->lrb_in_use);
	*tag_out = 0;
	ret = true;
out:
	return ret;
}

void ufshcd_put_dev_cmd_tag(struct ufs_hba *hba, int tag)
{
	clear_bit_unlock(tag, &hba->lrb_in_use);
    return;
}

/**
 * ufshcd_exec_dev_cmd - API for sending device management requests
 * @hba - UFS hba
 * @cmd_type - specifies the type (NOP, Query...)
 * @timeout - time in seconds
 *
 * NOTE: Since there is only one available tag for device management commands,
 * it is expected you hold the hba->dev_cmd.lock mutex.
 */
static int ufshcd_exec_dev_cmd(struct ufs_hba *hba,
		enum dev_cmd_type cmd_type, int timeout)
{
	struct ufshcd_lrb *lrbp;
	int err;
	int tag;
	int timeleft = 1;
	u32 intr_status;

	/*
	 * Get free slot, sleep if slots are unavailable.
	 * Even though we use wait_event() which sleeps indefinitely,
	 * the maximum wait time is bounded by SCSI request timeout.
	 */
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8579);
	#if 0
	wait_event(hba->dev_cmd.tag_wq, ufshcd_get_dev_cmd_tag(hba, &tag));
	#else
	tag = -1;
	do {
		if(ufshcd_get_dev_cmd_tag(hba, &tag) == true)
			break;
	} while(1);
	#endif
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x857A);

	lrbp = &hba->lrb[tag];
	err = ufshcd_compose_dev_cmd(hba, lrbp, cmd_type, tag);
	if (unlikely(err))
		goto out_put_tag;
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x857B);

	__set_bit(tag, &hba->outstanding_reqs);
	ufshcd_writel(hba, 1 << tag, REG_UTP_TRANSFER_REQ_DOOR_BELL);
//	writel( 1<< tag, (volatile void __iomem *) (REG_UFSHCI_ADDR + REG_UTP_TRANSFER_REQ_DOOR_BELL)); 	//avoid using sram


	//wait for intr
//	while (( readl((const volatile void __iomem *)( REG_UFSHCI_ADDR + REG_INTERRUPT_STATUS) ) & UTP_TRANSFER_REQ_COMPL) == 0) {

	while ((ufshcd_readl(hba, REG_INTERRUPT_STATUS) & UTP_TRANSFER_REQ_COMPL) == 0) {
		udelay(1);

		UFS_INCREASE_DUMMY();
		if (UFS_DUMMY() >= BLK_DEFAULT_SG_TIMEOUT * 1000) {
			if (( readl((const volatile void __iomem *)( REG_UFSHCI_ADDR + REG_INTERRUPT_STATUS) ) & UTP_TRANSFER_REQ_COMPL) == 0)
				err = -ETIMEDOUT;
			break;
		}
	}
	intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x857C);

	if ((intr_status&UTP_TRANSFER_REQ_COMPL)) {
		REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x857D);
		if (intr_status & UFSHCD_UIC_MASK)
			ufshcd_uic_cmd_compl(hba, intr_status);

		if (intr_status & UTP_TRANSFER_REQ_COMPL)
			ufshcd_transfer_req_compl(hba);
	}
	if (intr_status) {
		ufshcd_writel(hba, intr_status, REG_INTERRUPT_STATUS);
	}
	if(err)
		timeleft = 0;

	err = ufshcd_wait_for_dev_cmd(hba, lrbp, timeleft);
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, err);

out_put_tag:
	clear_bit_unlock(tag, &hba->lrb_in_use);
	return err;
}

/**
 * ufshcd_memory_alloc - allocate memory for host memory space data structures
 * @hba: per adapter instance
 *
 * 1. Allocate DMA memory for Command Descriptor array
 *	Each command descriptor consist of Command UPIU, Response UPIU and PRDT
 * 2. Allocate DMA memory for UTP Transfer Request Descriptor List (UTRDL).
 * 3. Allocate DMA memory for UTP Task Management Request Descriptor List
 *	(UTMRDL)
 * 4. Allocate memory for local reference block(lrb).
 *
 * Returns 0 for success, non-zero in case of failure
 */
static int ufshcd_memory_alloc(struct ufs_hba *hba)
{
    hba->ucdl_base_addr = (struct utp_transfer_cmd_desc*)(CONFIG_MSTAR_UFS_SBOOT_MEMORY_ST_ADDR);
    hba->utrdl_base_addr = (struct utp_transfer_req_desc*)(CONFIG_MSTAR_UFS_SBOOT_MEMORY_ST_ADDR + 0x800);
//    hba->utmrdl_base_addr = &ufs_memory_st->utmrdl_base_addr;
    hba->ucdl_dma_addr =(dma_addr_t) (CONFIG_MSTAR_UFS_SBOOT_MEMORY_ST_ADDR);
    hba->utrdl_dma_addr =(dma_addr_t) (CONFIG_MSTAR_UFS_SBOOT_MEMORY_ST_ADDR + 0x800);
//    hba->utmrdl_dma_addr =(dma_addr_t) &ufs_memory_st->utmrdl_base_addr;
    hba->lrb = (struct ufshcd_lrb *)(CONFIG_MSTAR_UFS_SBOOT_MEMORY_ST_ADDR + 0x400);
	ufs_memset((void*) hba->lrb, 0, sizeof(struct ufshcd_lrb));
	return 0;
}

/**
 * ufshcd_host_memory_configure - configure local reference block with
 *				memory offsets
 * @hba: per adapter instance
 *
 * Configure Host memory space
 * 1. Update Corresponding UTRD.UCDBA and UTRD.UCDBAU with UCD DMA
 * address.
 * 2. Update each UTRD with Response UPIU offset, Response UPIU length
 * and PRDT offset.
 * 3. Save the corresponding addresses of UTRD, UCD.CMD, UCD.RSP and UCD.PRDT
 * into local reference block.
 */
static void ufshcd_host_memory_configure(struct ufs_hba *hba)
{
	struct utp_transfer_cmd_desc *cmd_descp;
	struct utp_transfer_req_desc *utrdlp;
	dma_addr_t cmd_desc_dma_addr;
	dma_addr_t cmd_desc_element_addr;
	u16 response_offset;
	u16 prdt_offset;
	int cmd_desc_size;

	utrdlp = hba->utrdl_base_addr;
	cmd_descp = hba->ucdl_base_addr;

	response_offset =
		offsetof(struct utp_transfer_cmd_desc, response_upiu);
	prdt_offset =
		offsetof(struct utp_transfer_cmd_desc, prd_table);

	cmd_desc_size = sizeof(struct utp_transfer_cmd_desc);
	cmd_desc_dma_addr = hba->ucdl_dma_addr;
	/* Configure UTRD with command descriptor base address */
	cmd_desc_element_addr =cmd_desc_dma_addr;
	utrdlp[0].command_desc_base_addr_lo =
			cpu_to_le32(lower_32_bits(cmd_desc_element_addr));
	utrdlp[0].command_desc_base_addr_hi =
			cpu_to_le32(upper_32_bits(cmd_desc_element_addr));

	/* Response upiu and prdt offset should be in double words */
	utrdlp[0].response_upiu_offset =
			cpu_to_le16((response_offset >> 2));
	utrdlp[0].prd_table_offset =
			cpu_to_le16((prdt_offset >> 2));
	utrdlp[0].response_upiu_length =
			cpu_to_le16(ALIGNED_UPIU_SIZE >> 2);

	hba->lrb[0].utr_descriptor_ptr = (utrdlp);
	hba->lrb[0].ucd_req_ptr =
		(struct utp_upiu_req *)(cmd_descp);
	hba->lrb[0].ucd_rsp_ptr =
		(struct utp_upiu_rsp *) ((U32)(cmd_descp) + 32); //cmd_descp[0].response_upiu;
	hba->lrb[0].ucd_prdt_ptr =
		(struct ufshcd_sg_entry *)cmd_descp[0].prd_table;
}

/**
 * ufshcd_dme_link_startup - Notify Unipro to perform link startup
 * @hba: per adapter instance
 *
 * UIC_CMD_DME_LINK_STARTUP command must be issued to Unipro layer,
 * in order to initialize the Unipro link startup procedure.
 * Once the Unipro links are up, the device connected to the controller
 * is detected.
 *
 * Returns 0 on success, non-zero value on failure
 */
static int ufshcd_dme_link_startup(struct ufs_hba *hba)
{
	struct uic_command uic_cmd;

	ufs_memset((void*)&uic_cmd,0, sizeof(uic_cmd));
	uic_cmd.command = UIC_CMD_DME_LINK_STARTUP;

	return ufshcd_send_uic_cmd(hba, &uic_cmd);
}
/**
 * ufshcd_dme_set_attr - UIC command for DME_SET, DME_PEER_SET
 * @hba: per adapter instance
 * @attr_sel: uic command argument1
 * @attr_set: attribute set type as uic command argument2
 * @mib_val: setting value as uic command argument3
 * @peer: indicate whether peer or local
 *
 * Returns 0 on success, non-zero value on failure
 */
int ufshcd_dme_set_attr(struct ufs_hba *hba, u32 attr_sel,
			u8 attr_set, u32 mib_val, u8 peer)
{
	struct uic_command uic_cmd;
//	static const char *const action[] = {
//		"dme-set",
//		"dme-peer-set"
//	};
//	const char *set = action[!!peer];
	int ret;
	ufs_memset((void*)&uic_cmd,0, sizeof(uic_cmd));

	uic_cmd.command = peer ?
		UIC_CMD_DME_PEER_SET : UIC_CMD_DME_SET;
	uic_cmd.argument1 = attr_sel;
	uic_cmd.argument2 = UIC_ARG_ATTR_TYPE(attr_set);
	uic_cmd.argument3 = mib_val;

	ret = ufshcd_send_uic_cmd(hba, &uic_cmd);
//	if (ret)
//		dev_err(hba->dev, "%s: attr-id 0x%x val 0x%x error code %d\n",
//			set, UIC_GET_ATTR_ID(attr_sel), mib_val, ret);

	return ret;
}
EXPORT_SYMBOL_GPL(ufshcd_dme_set_attr);

/**
 * ufshcd_dme_get_attr - UIC command for DME_GET, DME_PEER_GET
 * @hba: per adapter instance
 * @attr_sel: uic command argument1
 * @mib_val: the value of the attribute as returned by the UIC command
 * @peer: indicate whether peer or local
 *
 * Returns 0 on success, non-zero value on failure
 */
int ufshcd_dme_get_attr(struct ufs_hba *hba, u32 attr_sel,
			u32 *mib_val, u8 peer)
{
	struct uic_command uic_cmd;
//	static const char *const action[] = {
//		"dme-get",
//		"dme-peer-get"
//	};
//	const char *get = action[!!peer];
	int ret;
	ufs_memset((void*)&uic_cmd,0, sizeof(uic_cmd));

	uic_cmd.command = peer ?
		UIC_CMD_DME_PEER_GET : UIC_CMD_DME_GET;
	uic_cmd.argument1 = attr_sel;

	ret = ufshcd_send_uic_cmd(hba, &uic_cmd);
	if (ret) {
//		dev_err(hba->dev, "%s: attr-id 0x%x error code %d\n",
//			get, UIC_GET_ATTR_ID(attr_sel), ret);
		goto out;
	}

	if (mib_val)
		*mib_val = uic_cmd.argument3;
out:
	return ret;
}
EXPORT_SYMBOL_GPL(ufshcd_dme_get_attr);

#if defined(CONFIG_MSTAR_UFS_HIGHER_POWER_MODE)
/**
 * ufshcd_uic_pwr_ctrl - executes UIC commands (which affects the link power
 * state) and waits for it to take effect.
 *
 * @hba: per adapter instance
 * @cmd: UIC command to execute
 *
 * DME operations like DME_SET(PA_PWRMODE), DME_HIBERNATE_ENTER &
 * DME_HIBERNATE_EXIT commands take some time to take its effect on both host
 * and device UniPro link and hence it's final completion would be indicated by
 * dedicated status bits in Interrupt Status register (UPMS, UHES, UHXS) in
 * addition to normal UIC command completion Status (UCCS). This function only
 * returns after the relevant status bits indicate the completion.
 *
 * Returns 0 on success, non-zero value on failure
 */
static int ufshcd_uic_pwr_ctrl(struct ufs_hba *hba, struct uic_command *cmd)
{
	//struct completion uic_async_done;
//	unsigned long flags;
	u8 status;
	int ret;

//	mutex_lock(&hba->uic_cmd_mutex);
//	init_completion(&uic_async_done);

//	spin_lock_irqsave(hba->host->host_lock, flags);
	//hba->uic_async_done = &uic_async_done;


	ret = ufshcd_send_uic_cmd(hba, cmd);
	/*
	ret = __ufshcd_send_uic_cmd(hba, cmd);
//	spin_unlock_irqrestore(hba->host->host_lock, flags);
	if (ret) {
		dev_err(hba->dev,
			"pwr ctrl cmd 0x%x with mode 0x%x uic error %d\n",
			cmd->command, cmd->argument3, ret);
		goto out;
	}
	ret = ufshcd_wait_for_uic_cmd(hba, cmd);
	if (ret) {
		dev_err(hba->dev,
			"pwr ctrl cmd 0x%x with mode 0x%x uic error %d\n",
			cmd->command, cmd->argument3, ret);
		goto out;
	}
	*/
	#if 0
	if (!wait_for_completion_timeout(hba->uic_async_done,
					 msecs_to_jiffies(UIC_CMD_TIMEOUT))) {
	#else
	if(ufshcd_wait_for_intr_status(hba, UFSHCD_UIC_PWR_MASK, UIC_CMD_TIMEOUT)) {
	#endif
		dev_err(hba->dev,
			"pwr ctrl cmd 0x%x with mode 0x%x completion timeout\n",
			cmd->command, cmd->argument3);
		ret = -ETIMEDOUT;
		goto out;
	}

	status = (ufshcd_readl(hba, REG_CONTROLLER_STATUS) >> 8) & 0x7;
	if (status != PWR_LOCAL) {
		dev_err(hba->dev,
			"pwr ctrl cmd 0x%0x failed, host umpcrs:0x%x\n",
			cmd->command, status);
		ret = (status != PWR_OK) ? status : -1;
	}
out:
//	spin_lock_irqsave(hba->host->host_lock, flags);
//	hba->uic_async_done = NULL;
//	spin_unlock_irqrestore(hba->host->host_lock, flags);
//	mutex_unlock(&hba->uic_cmd_mutex);

	return ret;
}


/**
 * ufshcd_uic_change_pwr_mode - Perform the UIC power mode chage
 *				using DME_SET primitives.
 * @hba: per adapter instance
 * @mode: powr mode value
 *
 * Returns 0 on success, non-zero value on failure
 */


static int ufshcd_uic_change_pwr_mode(struct ufs_hba *hba, u8 mode)
{
	struct uic_command uic_cmd;
	int ret;
	ufs_memset((void*)&uic_cmd,0, sizeof(uic_cmd));

	uic_cmd.command = UIC_CMD_DME_SET;
	uic_cmd.argument1 = UIC_ARG_MIB(PA_PWRMODE);
	uic_cmd.argument3 = mode;
//	ufshcd_hold(hba, false);
	ret = ufshcd_uic_pwr_ctrl(hba, &uic_cmd);
//	ufshcd_release(hba);

	return ret;
}
#endif


 /**
 * ufshcd_init_pwr_info - setting the POR (power on reset)
 * values in hba power info
 * @hba: per-adapter instance
 */

static void ufshcd_init_pwr_info(struct ufs_hba *hba)
{
	hba->pwr_info.gear_rx = UFS_PWM_G1;
	hba->pwr_info.gear_tx = UFS_PWM_G1;
	hba->pwr_info.lane_rx = 1;
	hba->pwr_info.lane_tx = 1;
	hba->pwr_info.pwr_rx = SLOWAUTO_MODE;
	hba->pwr_info.pwr_tx = SLOWAUTO_MODE;
	hba->pwr_info.hs_rate = 0;
}
/**
 * ufshcd_get_max_pwr_mode - reads the max power mode negotiated with device
 * @hba: per-adapter instance
 */
#if defined(CONFIG_MSTAR_UFS_HIGHER_POWER_MODE)
static int ufshcd_get_max_pwr_mode(struct ufs_hba *hba)
{
	static bool is_valid = false;
	static struct ufs_pa_layer_attr spwr_info;
	struct ufs_pa_layer_attr *pwr_info = &hba->max_pwr_info.info;

	if(is_valid == true)
	{
		if((--spwr_info.gear_rx) == 0)
		{
			if(spwr_info.pwr_rx == SLOWAUTO_MODE ||
				spwr_info.pwr_rx == SLOW_MODE)
				spwr_info.gear_rx = 1;
			else
			{
				spwr_info.pwr_rx = SLOWAUTO_MODE;
				spwr_info.gear_rx = CAP_MSTAR_MAX_RX_PWM_GEAR;
			}
		}

		if((--spwr_info.gear_tx) == 0)
		{
			if(spwr_info.pwr_tx == SLOWAUTO_MODE ||
				spwr_info.pwr_tx == SLOW_MODE)
				spwr_info.gear_tx = 1;
			else
			{
				spwr_info.pwr_tx = SLOWAUTO_MODE;
				spwr_info.gear_tx = CAP_MSTAR_MAX_TX_PWM_GEAR;
			}
		}
		ufs_memcpy(pwr_info, &spwr_info, sizeof(struct ufs_pa_layer_attr));

		return 0;
	}

	pwr_info->pwr_tx = FASTAUTO_MODE;
	pwr_info->pwr_rx = FASTAUTO_MODE;
	pwr_info->hs_rate = PA_HS_MODE_A;

	/* Get the connected lane count */
	ufshcd_dme_get_attr(hba, UIC_ARG_MIB(PA_CONNECTEDRXDATALANES),
			&pwr_info->lane_rx, DME_LOCAL);
	ufshcd_dme_get_attr(hba, UIC_ARG_MIB(PA_CONNECTEDTXDATALANES),
			&pwr_info->lane_tx, DME_LOCAL);

	if (!pwr_info->lane_rx || !pwr_info->lane_tx) {
		dev_err(hba->dev, "%s: invalid connected lanes value. rx=%d, tx=%d\n",
				__func__,
				pwr_info->lane_rx,
				pwr_info->lane_tx);
		return -EINVAL;
	}

	/*
	 * First, get the maximum gears of HS speed.
	 * If a zero value, it means there is no HSGEAR capability.
	 * Then, get the maximum gears of PWM speed.
	 */
	ufshcd_dme_get_attr(hba, UIC_ARG_MIB(PA_MAXRXHSGEAR), &pwr_info->gear_rx, DME_LOCAL);
	if (!pwr_info->gear_rx) {
		ufshcd_dme_get_attr(hba, UIC_ARG_MIB(PA_MAXRXPWMGEAR),
				&pwr_info->gear_rx, DME_LOCAL);
		if (!pwr_info->gear_rx) {
			dev_err(hba->dev, "%s: invalid max pwm rx gear read = %d\n",
				__func__, pwr_info->gear_rx);
			return -EINVAL;
		}
		pwr_info->pwr_rx = SLOWAUTO_MODE;
	}

	ufshcd_dme_get_attr(hba, UIC_ARG_MIB(PA_MAXRXHSGEAR),
			&pwr_info->gear_tx, DME_PEER);
	if (!pwr_info->gear_tx) {
		ufshcd_dme_get_attr(hba, UIC_ARG_MIB(PA_MAXRXPWMGEAR),
				&pwr_info->gear_tx, DME_PEER);
		if (!pwr_info->gear_tx) {
			dev_err(hba->dev, "%s: invalid max pwm tx gear read = %d\n",
				__func__, pwr_info->gear_tx);
			return -EINVAL;
		}
		pwr_info->pwr_tx = SLOWAUTO_MODE;
	}

	hba->max_pwr_info.is_valid = true;

	is_valid = true;
	ufs_memcpy(&spwr_info, pwr_info, sizeof(struct ufs_pa_layer_attr));


	return 0;
}
static int ufshcd_change_power_mode(struct ufs_hba *hba,
			     struct ufs_pa_layer_attr *pwr_mode)
{
	int ret;

	/* if already configured to the requested pwr_mode */
	if (pwr_mode->gear_rx == hba->pwr_info.gear_rx &&
	    pwr_mode->gear_tx == hba->pwr_info.gear_tx &&
	    pwr_mode->lane_rx == hba->pwr_info.lane_rx &&
	    pwr_mode->lane_tx == hba->pwr_info.lane_tx &&
	    pwr_mode->pwr_rx == hba->pwr_info.pwr_rx &&
	    pwr_mode->pwr_tx == hba->pwr_info.pwr_tx &&
	    pwr_mode->hs_rate == hba->pwr_info.hs_rate) {
		dev_dbg(hba->dev, "%s: power already configured\n", __func__);
		return 0;
	}

	/*
	 * Configure attributes for power mode change with below.
	 * - PA_RXGEAR, PA_ACTIVERXDATALANES, PA_RXTERMINATION,
	 * - PA_TXGEAR, PA_ACTIVETXDATALANES, PA_TXTERMINATION,
	 * - PA_HSSERIES
	 */

	ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_RXGEAR), ATTR_SET_NOR, pwr_mode->gear_rx, DME_LOCAL);
	ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_ACTIVERXDATALANES), ATTR_SET_NOR,
			pwr_mode->lane_rx, DME_LOCAL);
	if (pwr_mode->pwr_rx == FASTAUTO_MODE ||
			pwr_mode->pwr_rx == FAST_MODE)
		ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_RXTERMINATION), ATTR_SET_NOR, TRUE, DME_LOCAL);
	else
		ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_RXTERMINATION), ATTR_SET_NOR, FALSE, DME_LOCAL);

	ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_TXGEAR), ATTR_SET_NOR, pwr_mode->gear_tx, DME_LOCAL);
	ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_ACTIVETXDATALANES), ATTR_SET_NOR,
			pwr_mode->lane_tx, DME_LOCAL);
	if (pwr_mode->pwr_tx == FASTAUTO_MODE ||
			pwr_mode->pwr_tx == FAST_MODE)
		ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_TXTERMINATION), ATTR_SET_NOR, TRUE, DME_LOCAL);
	else
		ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_TXTERMINATION), ATTR_SET_NOR, FALSE, DME_LOCAL);

	if (pwr_mode->pwr_rx == FASTAUTO_MODE ||
	    pwr_mode->pwr_tx == FASTAUTO_MODE ||
	    pwr_mode->pwr_rx == FAST_MODE ||
	    pwr_mode->pwr_tx == FAST_MODE)
		ufshcd_dme_set_attr(hba, UIC_ARG_MIB(PA_HSSERIES), ATTR_SET_NOR,
						pwr_mode->hs_rate, DME_LOCAL);

	ret = ufshcd_uic_change_pwr_mode(hba, pwr_mode->pwr_rx << 4
			| pwr_mode->pwr_tx);

	if (ret) {
		dev_err(hba->dev,
			"%s: power mode change failed %d\n", __func__, ret);
	} else {
//		if (hba->vops && hba->vops->pwr_change_notify)
//			hba->vops->pwr_change_notify(hba,
//				POST_CHANGE, NULL, pwr_mode);
        ufs_mstar_pwr_change_notify(hba,
				POST_CHANGE, NULL, pwr_mode);

		ufs_memcpy(&hba->pwr_info, pwr_mode,
			sizeof(struct ufs_pa_layer_attr));
	}

	return ret;
}

/**
 * ufshcd_config_pwr_mode - configure a new power mode
 * @hba: per-adapter instance
 * @desired_pwr_mode: desired power configuration
 */
static int ufshcd_config_pwr_mode(struct ufs_hba *hba,
		struct ufs_pa_layer_attr *desired_pwr_mode)
{
	struct ufs_pa_layer_attr final_params ;
	int ret;

	ufs_memset((void*)&final_params, 0, sizeof(struct ufs_pa_layer_attr));
//	if (hba->vops && hba->vops->pwr_change_notify)
//		hba->vops->pwr_change_notify(hba,
//		     PRE_CHANGE, desired_pwr_mode, &final_params);
//	else
//		memcpy(&final_params, desired_pwr_mode, sizeof(final_params));

    ufs_mstar_pwr_change_notify(hba,
		     PRE_CHANGE, desired_pwr_mode, &final_params);

	ret = ufshcd_change_power_mode(hba, &final_params);

	return ret;
}
#endif
/**
 * ufshcd_make_hba_operational - Make UFS controller operational
 * @hba: per adapter instance
 *
 * To bring UFS host controller to operational state,
 * 1. Enable required interrupts
 * 2. Configure interrupt aggregation
 * 3. Program UTRL and UTMRL base address
 * 4. Configure run-stop-registers
 *
 * Returns 0 on success, non-zero value on failure
 */
static int ufshcd_make_hba_operational(struct ufs_hba *hba)
{
	int err = 0;

	/* Enable required interrupts */
//	ufshcd_enable_intr(hba, UFSHCD_ENABLE_INTRS);

//    ufshcd_writel(hba, ufshcd_readl(hba, REG_INTERRUPT_ENABLE) | UFSHCD_ENABLE_INTRS, REG_INTERRUPT_ENABLE);


	/* Configure interrupt aggregation */
//	ufshcd_writel(hba, INT_AGGR_ENABLE | INT_AGGR_PARAM_WRITE |
//		      INT_AGGR_COUNTER_THLD_VAL(hba->nutrs - 1) |
//		      INT_AGGR_TIMEOUT_VAL(INT_AGGR_DEF_TO),
//		      REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL);

	/* Configure UTRL and UTMRL base address registers */
	ufshcd_writel(hba, lower_32_bits(hba->utrdl_dma_addr),
			REG_UTP_TRANSFER_REQ_LIST_BASE_L);
	ufshcd_writel(hba, upper_32_bits(hba->utrdl_dma_addr),
			REG_UTP_TRANSFER_REQ_LIST_BASE_H);

	/*
	 * UCRDY, UTMRLDY and UTRLRDY bits must be 1
	 * DEI, HEI bits must be 0
	 */
	 /*
	if (!((((ufshcd_readl(hba, REG_CONTROLLER_STATUS)) & (0xFF)) >> 1) ^ (0x07))) {
		ufshcd_enable_run_stop_reg(hba);
	} else {
		dev_err(hba->dev,
			"Host controller not ready to process requests\n");
		err = -EIO;
		goto out;
	}
	*/

out:
	return err;
}

/**
 * ufshcd_hba_enable - initialize the controller
 * @hba: per adapter instance
 *
 * The controller resets itself and controller firmware initialization
 * sequence kicks off. When controller is ready it will set
 * the Host Controller Enable bit to 1.
 *
 * Returns 0 on success, non-zero value on failure
 */
static int ufshcd_hba_enable(struct ufs_hba *hba)
{
	int retry;

	/*
	 * msleep of 1 and 5 used in this function might result in msleep(20),
	 * but it was necessary to send the UFS FPGA to reset mode during
	 * development and testing of this driver. msleep can be changed to
	 * mdelay and retry count can be reduced based on the controller.
	 */
	if (!!(ufshcd_readl(hba, REG_CONTROLLER_ENABLE) & 0x1)) {

		/* change controller state to "reset state" */
        ufshcd_writel(hba, CONTROLLER_DISABLE,  REG_CONTROLLER_ENABLE);

		/*
		 * This delay is based on the testing done with UFS host
		 * controller FPGA. The delay can be changed based on the
		 * host controller used.
		 */
		msleep(5);
	}

	/* UniPro link is disabled at this point */
//	ufshcd_set_link_off(hba);

//	if (hba->vops && hba->vops->hce_enable_notify)
//		hba->vops->hce_enable_notify(hba, PRE_CHANGE);

	/* start controller initialization sequence */
	ufshcd_writel(hba, CONTROLLER_ENABLE, REG_CONTROLLER_ENABLE);

	/*
	 * To initialize a UFS host controller HCE bit must be set to 1.
	 * During initialization the HCE bit value changes from 1->0->1.
	 * When the host controller completes initialization sequence
	 * it sets the value of HCE bit to 1. The same HCE bit is read back
	 * to check if the controller has completed initialization sequence.
	 * So without this delay the value HCE = 1, set in the previous
	 * instruction might be read back.
	 * This delay can be changed based on the controller.
	 */
	msleep(1);

	/* wait for the host controller to complete initialization */
	retry = 10;
	while (!(ufshcd_readl(hba, REG_CONTROLLER_ENABLE) & 0x1)) {
		if (retry) {
			retry--;
		} else {
			dev_err(hba->dev,
				"Controller enable failed\n");
			return -EIO;
		}
		msleep(5);
	}

	/* enable UIC related interrupts */
//	ufshcd_enable_intr(hba, UFSHCD_UIC_MASK);
    ufshcd_writel(hba, ufshcd_readl(hba, REG_INTERRUPT_ENABLE) | UFSHCD_UIC_MASK, REG_INTERRUPT_ENABLE);

//	if (hba->vops && hba->vops->hce_enable_notify)
//		hba->vops->hce_enable_notify(hba, POST_CHANGE);

	return 0;
}

static int ufs_mstar_link_startup_post_change(struct ufs_hba *hba)
{
	int err = 0;
	ufshcd_dme_set_attr(hba,UIC_ARG_MIB(0x3000), ATTR_SET_NOR, 0, DME_LOCAL);

	ufshcd_dme_set_attr(hba,UIC_ARG_MIB(0x3001), ATTR_SET_NOR, 1, DME_LOCAL);

	ufshcd_dme_set_attr(hba,UIC_ARG_MIB(0x4025), ATTR_SET_NOR, 6, DME_LOCAL);

	ufshcd_dme_set_attr(hba,UIC_ARG_MIB(0x4021), ATTR_SET_NOR, 1, DME_LOCAL);

	ufshcd_dme_set_attr(hba,UIC_ARG_MIB(0x4020), ATTR_SET_NOR, 1, DME_LOCAL);
	return err;
}

/**
 * ufshcd_link_startup - Initialize unipro link startup
 * @hba: per adapter instance
 *
 * Returns 0 for success, non-zero in case of failure
 */
static int ufshcd_link_startup(struct ufs_hba *hba)
{
	int ret;
	int retries = DME_LINKSTARTUP_RETRIES;

	do {
//		if (hba->vops && hba->vops->link_startup_notify)
//			hba->vops->link_startup_notify(hba, PRE_CHANGE);

		ret = ufshcd_dme_link_startup(hba);

		/* check if device is detected by inter-connect layer */
		if (!ret && !(ufshcd_readl(hba, REG_CONTROLLER_STATUS) & DEVICE_PRESENT)) {
//			dev_err(hba->dev, "%s: Device not present\n", __func__);
			ret = -ENXIO;
			goto out;
		}

		/*
		 * DME link lost indication is only received when link is up,
		 * but we can't be sure if the link is up until link startup
		 * succeeds. So reset the local Uni-Pro and try again.
		 */
		if (ret && ufshcd_hba_enable(hba))
			goto out;
	} while (ret && retries--);

	if (ret)
		/* failed to get the link up... retire */
		goto out;
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8572);

	/* Include any host controller configuration via UIC commands */
//	if (hba->vops && hba->vops->link_startup_notify) {
//		ret = hba->vops->link_startup_notify(hba, POST_CHANGE);
//		if (ret)
//			goto out;
//	}
	ret = ufs_mstar_link_startup_post_change(hba);
	if(ret)
		goto out;
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8573);

	ret = ufshcd_make_hba_operational(hba);

	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8574);
out:
//	if (ret)
//		dev_err(hba->dev, "link startup failed %d\n", ret);
	return ret;
}

/**
 * ufshcd_verify_dev_init() - Verify device initialization
 * @hba: per-adapter instance
 *
 * Send NOP OUT UPIU and wait for NOP IN response to check whether the
 * device Transport Protocol (UTP) layer is ready after a reset.
 * If the UTP layer at the device side is not initialized, it may
 * not respond with NOP IN UPIU within timeout of %NOP_OUT_TIMEOUT
 * and we retry sending NOP OUT for %NOP_OUT_RETRIES iterations.
 */
static int ufshcd_verify_dev_init(struct ufs_hba *hba)
{
	int err = 0;
	int retries;

	for (retries = NOP_OUT_RETRIES; retries > 0; retries--) {
		err = ufshcd_exec_dev_cmd(hba, DEV_CMD_TYPE_NOP,
					       NOP_OUT_TIMEOUT);

		if (!err || err == -ETIMEDOUT)
			break;

		dev_dbg(hba->dev, "%s: error %d retrying\n", __func__, err);
	}

	return err;
}

/**
 * ufshcd_init_query() - init the query response and request parameters
 * @hba: per-adapter instance
 * @request: address of the request pointer to be initialized
 * @response: address of the response pointer to be initialized
 * @opcode: operation to perform
 * @idn: flag idn to access
 * @index: LU number to access
 * @selector: query/flag/descriptor further identification
 */
static inline void ufshcd_init_query(struct ufs_hba *hba,
		struct ufs_query_req **request, struct ufs_query_res **response,
		enum query_opcode opcode, u8 idn, u8 index, u8 selector)
{
	*request = &hba->dev_cmd.query.request;
	*response = &hba->dev_cmd.query.response;
	ufs_memset(*request, 0, sizeof(struct ufs_query_req));
	ufs_memset(*response, 0, sizeof(struct ufs_query_res));
	(*request)->upiu_req.opcode = opcode;
	(*request)->upiu_req.idn = idn;
	(*request)->upiu_req.index = index;
	(*request)->upiu_req.selector = selector;
}

/**
 * ufshcd_query_flag() - API function for sending flag query requests
 * hba: per-adapter instance
 * query_opcode: flag query to perform
 * idn: flag idn to access
 * flag_res: the flag value after the query request completes
 *
 * Returns 0 for success, non-zero in case of failure
 */
int ufshcd_query_flag(struct ufs_hba *hba, enum query_opcode opcode,
			enum flag_idn idn, bool *flag_res)
{
	struct ufs_query_req *request = NULL;
	struct ufs_query_res *response = NULL;
	int err, index = 0, selector = 0;
	int timeout = QUERY_REQ_TIMEOUT;

	BUG_ON(!hba);

	mutex_lock(&hba->dev_cmd.lock);
	ufshcd_init_query(hba, &request, &response, opcode, idn, index,
			selector);

	switch (opcode) {
	case UPIU_QUERY_OPCODE_SET_FLAG:
	case UPIU_QUERY_OPCODE_CLEAR_FLAG:
	case UPIU_QUERY_OPCODE_TOGGLE_FLAG:
		request->query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
		break;
	case UPIU_QUERY_OPCODE_READ_FLAG:
		request->query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
		if (!flag_res) {
			/* No dummy reads */
			dev_err(hba->dev, "%s: Invalid argument for read request\n",
					__func__);
			err = -EINVAL;
			goto out_unlock;
		}
		break;
	default:
		dev_err(hba->dev,
			"%s: Expected query flag opcode but got = %d\n",
			__func__, opcode);
		err = -EINVAL;
		goto out_unlock;
	}

	if (idn == QUERY_FLAG_IDN_FDEVICEINIT)
		timeout = QUERY_FDEVICEINIT_REQ_TIMEOUT;

	err = ufshcd_exec_dev_cmd(hba, DEV_CMD_TYPE_QUERY, timeout);

	if (err) {
		dev_err(hba->dev,
			"%s: Sending flag query for idn %d failed, err = %d\n",
			__func__, idn, err);
		goto out_unlock;
	}

	if (flag_res)
		*flag_res = (be32_to_cpu(response->upiu_res.value) &
				MASK_QUERY_UPIU_FLAG_LOC) & 0x1;

out_unlock:
	mutex_unlock(&hba->dev_cmd.lock);
	return err;
}

/**
 * ufshcd_complete_dev_init() - checks device readiness
 * hba: per-adapter instance
 *
 * Set fDeviceInit flag and poll until device toggles it.
 */
static int ufshcd_complete_dev_init(struct ufs_hba *hba)
{
	int i, retries, err = 0;
	bool flag_res = 1;
	
	for (retries = QUERY_REQ_RETRIES; retries > 0; retries--) {
		/* Set the fDeviceInit flag */
		err = ufshcd_query_flag(hba, UPIU_QUERY_OPCODE_SET_FLAG,
					QUERY_FLAG_IDN_FDEVICEINIT, NULL);
		if (!err || err == -ETIMEDOUT)
			break;
		dev_dbg(hba->dev, "%s: error %d retrying\n", __func__, err);
	}
	if (err) {
		dev_err(hba->dev,
			"%s setting fDeviceInit flag failed with error %d\n",
			__func__, err);
		goto out;
	}
	/* poll for max. 100 iterations for fDeviceInit flag to clear */
	for (i = 0; i < 100 && !err && flag_res; i++) {
		for (retries = QUERY_REQ_RETRIES; retries > 0; retries--) {
			err = ufshcd_query_flag(hba,
					UPIU_QUERY_OPCODE_READ_FLAG,
					QUERY_FLAG_IDN_FDEVICEINIT, &flag_res);
			if (!err || err == -ETIMEDOUT)
				break;
			dev_dbg(hba->dev, "%s: error %d retrying\n", __func__,
					err);
		}
	}
	if (err)
		dev_err(hba->dev,
			"%s reading fDeviceInit flag failed with error %d\n",
			__func__, err);
	else if (flag_res)
		dev_err(hba->dev,
			"%s fDeviceInit was not cleared by the device\n",
			__func__);
out:
	
	REG_WRITE_UINT16(DEBUG_REG_SYS_L3, err >>16);
	REG_WRITE_UINT16(DEBUG_REG_SYS_L4, err);
	return err;
}

/**
 * ufshcd_scsi_cmd_status - Update SCSI command result based on SCSI status
 * @lrb: pointer to local reference block of completed command
 * @scsi_status: SCSI command status
 *
 * Returns value base on SCSI command status
 */
static int
ufshcd_scsi_cmd_status(struct ufshcd_lrb *lrbp, int scsi_status)
{
	int result = 0;

	switch (scsi_status) {
	case SAM_STAT_CHECK_CONDITION:
//		ufshcd_copy_sense_data(lrbp);
	case SAM_STAT_GOOD:
		result |= DID_OK << 16 |
			  COMMAND_COMPLETE << 8 |
			  scsi_status;
		break;
	case SAM_STAT_TASK_SET_FULL:
	case SAM_STAT_BUSY:
	case SAM_STAT_TASK_ABORTED:
//		ufshcd_copy_sense_data(lrbp);
		result |= scsi_status;
		break;
	default:
		result |= DID_ERROR << 16;
		break;
	} /* end of switch */

	return result;
}

/**
 * ufshcd_transfer_rsp_status - Get overall status of the response
 * @hba: per adapter instance
 * @lrb: pointer to local reference block of completed command
 *
 * Returns result of the command to notify SCSI midlayer
 */
static int
ufshcd_transfer_rsp_status(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	int result = 0;
	int scsi_status;
	int ocs;

	/* overall command status of utrd */
	ocs = le32_to_cpu(lrbp->utr_descriptor_ptr->header.dword_2) & MASK_OCS;

	switch (ocs) {
	case OCS_SUCCESS:
		result = be32_to_cpu(lrbp->ucd_rsp_ptr->header.dword_0) >> 24;

		switch (result) {
		case UPIU_TRANSACTION_RESPONSE:
			/*
			 * get the response UPIU result to extract
			 * the SCSI command status
			 */
			result =be32_to_cpu(lrbp->ucd_rsp_ptr->header.dword_1) & MASK_RSP_UPIU_RESULT;

			/*
			 * get the result based on SCSI status response
			 * to notify the SCSI midlayer of the command status
			 */
			scsi_status = result & MASK_SCSI_STATUS;
			result = ufshcd_scsi_cmd_status(lrbp, scsi_status);
            /*
			if (ufshcd_is_exception_event(lrbp->ucd_rsp_ptr))
				#if 0
				schedule_work(&hba->eeh_work);
				#else
				ufshcd_exception_event_handler(hba);
				#endif
			*/
			break;
		default:
			result = DID_ERROR << 16;
			break;
		}
		break;
	case OCS_ABORTED:
		result |= DID_ABORT << 16;
		break;
	case OCS_INVALID_COMMAND_STATUS:
		result |= DID_REQUEUE << 16;
		break;
	default:
		result |= DID_ERROR << 16;
		dev_err(hba->dev,
		"OCS error from controller = %x\n", ocs);
		break;
	} /* end of switch */

	return result;
}

/**
 * ufshcd_uic_cmd_compl - handle completion of uic command
 * @hba: per adapter instance
 * @intr_status: interrupt status generated by the controller
 */
static void ufshcd_uic_cmd_compl(struct ufs_hba *hba, u32 intr_status)
{
	if ((intr_status & UIC_COMMAND_COMPL) && hba->active_uic_cmd) {
		hba->active_uic_cmd->argument2 |=
			ufshcd_readl(hba, REG_UIC_COMMAND_ARG_2) & MASK_UIC_COMMAND_RESULT;
		hba->active_uic_cmd->argument3 =
			ufshcd_readl(hba, REG_UIC_COMMAND_ARG_3);
		#if 0
		complete(&hba->active_uic_cmd->done);
		#endif
	}
}

/**
 * ufshcd_transfer_req_compl - handle SCSI and query command completion
 * @hba: per adapter instance
 */
static void ufshcd_transfer_req_compl(struct ufs_hba *hba)
{
	struct ufshcd_lrb *lrbp;
	struct scsi_cmnd *cmd;
	unsigned long completed_reqs;
	int result;

	/* Resetting interrupt aggregation counters first and reading the
	 * DOOR_BELL afterward allows us to handle all the completed requests.
	 * In order to prevent other interrupts starvation the DB is read once
	 * after reset. The down side of this solution is the possibility of
	 * false interrupt if device completes another request after resetting
	 * aggregation and before reading the DB.
	 */
    ufshcd_writel(hba, INT_AGGR_ENABLE |
              INT_AGGR_COUNTER_AND_TIMER_RESET,
              REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL);

	completed_reqs = ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL) ^ hba->outstanding_reqs;

//	for_each_set_bit(index, &completed_reqs, hba->nutrs) {
		lrbp = &hba->lrb[0];
		#ifdef CONFIG_MSTAR_UFS_DEBUG
		ufs_mstar_print_response(lrbp);
		#endif
		cmd = lrbp->cmd;
		if (cmd) {
			result = ufshcd_transfer_rsp_status(hba, lrbp);
			#if 0
			scsi_dma_unmap(cmd);
			#endif
			cmd->result = result;
			/* Mark completed command as NULL in LRB */
			lrbp->cmd = NULL;
			clear_bit_unlock(0, &hba->lrb_in_use);
			//hba->lrb_in_use &= ~BIT0;
			/* Do not touch lrbp after scsi done */
			#if 0
			cmd->scsi_done(cmd);
			#endif
		}
//	}

	/* clear corresponding bits of completed commands */
	hba->outstanding_reqs ^= completed_reqs;

	/* we might have free'd some tags above */
	wake_up(&hba->dev_cmd.tag_wq);
}

/**
 * ufshcd_probe_hba - probe hba to detect device and initialize
 * @hba: per-adapter instance
 *
 * Execute link-startup and verify device initialization
 */
static int ufshcd_probe_hba(struct ufs_hba *hba)
{
	int ret;
	#if 0
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8571);

	ret = ufshcd_link_startup(hba);
	if (ret)
		goto out;

	ufshcd_init_pwr_info(hba);

	/* UniPro link is active now */
//	ufshcd_set_link_active(hba);
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8578);

	ret = ufshcd_verify_dev_init(hba);
	if (ret)
		goto out;

	#if defined(CONFIG_MSTAR_UFS_HIGHER_POWER_MODE)
	if(efuse_fails == 0)
	{
	    if (ufshcd_get_max_pwr_mode(hba)) {
	        dev_err(hba->dev,
	            "%s: Failed getting max supported power mode\n",
	            __func__);
	    } else {
	        ret = ufshcd_config_pwr_mode(hba, &hba->max_pwr_info.info);
	        if (ret)
	            dev_err(hba->dev, "%s: Failed setting power mode, err = %d\n",
	                    __func__, ret);
	    }
	}
	#endif
	#endif
	
	ret = ufshcd_make_hba_operational(hba);
	
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8571);	
	
	ret = ufshcd_complete_dev_init(hba);
	if (ret)
		goto out;
	
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x857F);

out:
	return ret;
}

/**
 * ufshcd_alloc_host - allocate Host Bus Adapter (HBA)
 * @dev: pointer to device handle
 * @hba_handle: driver private handle
 * Returns 0 on success, non-zero value on failure
 */
int ufshcd_alloc_host(struct device *dev, struct ufs_hba **hba_handle)
{
	struct ufs_hba *hba;
	hba = (struct ufs_hba*)(CONFIG_MSTAR_UFS_HBA_VARIABLE_ADDR);
    ufs_memset((void*)hba, 0, sizeof(struct ufs_hba));	
//    ufs_memcpy((void*)pHostBusAdapter, (void*)CONFIG_MSTAR_UFS_ROM_HBA_VARIABLE_ADDR, sizeof(struct ufs_hba));
	hba->dev = dev;
	*hba_handle = hba;
	return 0;
}
EXPORT_SYMBOL(ufshcd_alloc_host);

struct ufs_hba* ufshcd_get_host(void)
{
	return (struct ufs_hba*) (CONFIG_MSTAR_UFS_HBA_VARIABLE_ADDR);
}
EXPORT_SYMBOL(ufshcd_get_host);

/**
 * ufshcd_init - Driver initialization routine
 * @hba: per-adapter instance
 * @mmio_base: base register address
 * @irq: Interrupt line of device
 * Returns 0 on success, non-zero value on failure
 */
int ufshcd_init(struct ufs_hba *hba, void __iomem *mmio_base, unsigned int irq)
{
	int err;

	hba->mmio_base = mmio_base;
//	err = ufshcd_hba_init(hba);
//	if (err)
//		goto out_error;
//	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8530);

    ufs_mstar_pltfrm_init();

	/* Read capabilities registers */
    hba->nutrs = 1;

//	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8540);

	/* Allocate memory for host memory space */
	err = ufshcd_memory_alloc(hba);
	if (err) {
		dev_err(hba->dev, "Memory allocation failed\n");
		goto out_error;
	}

	/* Configure LRB */
	ufshcd_host_memory_configure(hba);
//	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8550);

//	hba->max_pwr_info.is_valid = false;

	/*
	 * In order to avoid any spurious interrupt immediately after
	 * registering UFS controller interrupt handler, clear any pending UFS
	 * interrupt status and disable all the UFS interrupts.
	 */
//	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8560);
//	ufshcd_writel(hba, ufshcd_readl(hba, REG_INTERRUPT_STATUS),
//		      REG_INTERRUPT_STATUS);
//	ufshcd_writel(hba, 0, REG_INTERRUPT_ENABLE);

	/* Host controller enable */
/*
	err = ufshcd_hba_enable(hba);
	if (err) {
		dev_err(hba->dev, "Host controller enable failed\n");
		goto out_error;
	}
	REG_WRITE_UINT16(DEBUG_REG_SYS_L2, 0x8570);
*/
	err = ufshcd_probe_hba(hba);
	return err;

out_error:
	return err;
}
EXPORT_SYMBOL_GPL(ufshcd_init);

MODULE_AUTHOR("Santosh Yaragnavi <santosh.sy@samsung.com>");
MODULE_AUTHOR("Vinayak Holikatti <h.vinayak@samsung.com>");
MODULE_DESCRIPTION("Generic UFS host controller driver Core");
MODULE_LICENSE("GPL");
MODULE_VERSION(UFSHCD_DRIVER_VERSION);
