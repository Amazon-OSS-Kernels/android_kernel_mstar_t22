/*
 * adf_debug.c
 *
 * Copyright 2020-2022 Amazon Technologies, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include "adf/adf_status.h"
#include "adf/adf_common.h"

static adf_mem_cpy adf_dbg_mem_read = NULL;
static adf_chk_run adf_dbg_chk_run = NULL;
// static struct task_struct *adfLogDumpThread = NULL;
static uint8_t *g_log_buf = NULL;
static uint32_t g_log_buf_size = 0;
static int32_t g_last_wptr = 0;
static int adfDspCoreNo = 0;
struct mutex adfDspLock;

/*
 * _adfDebug_readLog()
 * ----------------------------------------
 * read log from DSP and dump it to seq_file,
 * this func will be called when user cat debugfs to dump dsp log
 *
 * Input:
 *   struct seq_file *file - the pointer to the target seq_file
 *   void *data            - not used
 * Return:
 *   0 or Error VAL
 */
static int _adfDebug_readLog(struct seq_file *file, void *data)
{
	uint32_t tmp = 0;
    uint8_t dspId;

    if (file->private == NULL)
        return -EINVAL;

    dspId = *((int32_t *)file->private) & 0xFF;
	if (dspId != 0)
		return -EINVAL;

	if (g_log_buf == NULL)
		return -EINVAL;

	mutex_lock(&adfDspLock);
    if (adf_dbg_mem_read((uintptr_t)g_log_buf, g_log_buf_size) < 0) {
		seq_printf(file, "Warning! Failed to read logData!\n");
		mutex_unlock(&adfDspLock);
        return -EIO;
    }
	adfLog_dump((void*)g_log_buf, &tmp, file);
	mutex_unlock(&adfDspLock);

    return 0;
}

/*
 * _adfDebug_open()
 * ----------------------------------------
 * open API for log/cli debug node, link the readLog to seq_file hdls
 *
 * Input:
 *   struct inode *inode   - the pointer to the operation node
 *   struct file *file     - the file handler, seq_file is file->private_data
 * Return:
 *   0 or Error VAL
 */
static int _adfDebug_open(struct inode *inode, struct file *file)
{
    return single_open(file, _adfDebug_readLog, inode->i_private);
}


/* The FS operation handlers for log/cli and state debug nodes */
static struct file_operations adfDebugFops = {
    .owner = THIS_MODULE,
    .open = _adfDebug_open,
    .read = seq_read
};

/*
 * adfDebug_initFs()
 * ----------------------------------------
 * The entry to init the FS for the cli/log and state debug nodes
 * The read/write APIs may be different on each platform, considered as HAL
 *
 * Input:
 *   void *debugReadFunc   - the function pointer to the debug read API
 *   void *debugWriteFunc  - the function pointer to the debug write API
 * Return:
 *   0 or Error VAL
 */
int adfDebug_initFs(void)
{
    struct dentry *adfDebugDir = NULL;
    struct dentry *adfDebugFile = NULL;
    char debugFileName[12] = {0};
	int i = 0;

    adfDebugDir = debugfs_create_dir("adf_dbg_fs", NULL);
    if (!adfDebugDir)
        return -ENOMEM;

	snprintf(debugFileName, sizeof(debugFileName), "adf_debug_%d", i);
	adfDebugFile = debugfs_create_file(debugFileName, 0644,
					adfDebugDir, &adfDspCoreNo, &adfDebugFops);
	if (!adfDebugFile)
		goto fail;
	return 0;

fail:
	debugfs_remove_recursive(adfDebugDir);
	return -ENOMEM;
}

/*
 * adfDebug_printLog()
 * ----------------------------------------
 * read log from DSP and print it to kernel log (dmesg),
 * this func will be called periodically in driver, or when DSP is crashed
 *
 * Input:
 *   uint32_t mode         - the log print mode, 0 for RECENT and 1 for ALL
 * Return:
 *   None
 */
void adfDebug_printLog(uint32_t mode)
{
	adfRingbuf_t *debugLogHdr= NULL;
	int32_t wptr = 0;

	if (g_log_buf == NULL)
		return;

	mutex_lock(&adfDspLock);
	if (adf_dbg_mem_read((uintptr_t)g_log_buf, g_log_buf_size) < 0) {
		pr_err("%s failed to read debug log from DSP..#####\n",__func__);
		mutex_unlock(&adfDspLock);
		return;
	}

	debugLogHdr = (adfRingbuf_t*)g_log_buf;

	/* mode = 0 means periodical print dsp log to kernel log,
	 *     only the new log should be printed
	 * mode = 1 means print dsp log to kernel log when dsp is crashed,
	 *     all the valid log in the buffer should be printed */
	if (mode == ADF_LOG_DUMP_RECENT) {
		/*
		 * we will read the write pointer first,
		 * if it is not changed, then that means no new log.
		 * Note that, we understand the best way here should check
		 * both RP and WP and magic,
		 * but it's not necessary to do it because the log ring buf
		 * won't be full within several seconds.
		 *
		 * The definition of the uint32_t flag is that,
		 * higher 16 bits for wp offset, lower 16 bits for rp offset
		 */
		wptr = debugLogHdr->wp;
		if (wptr == g_last_wptr) {
			mutex_unlock(&adfDspLock);
			return;
		}
		adfLog_print((void*)g_log_buf, (uint32_t*)&g_last_wptr);
		g_last_wptr = wptr;
	} else if (mode == ADF_LOG_DUMP_ALL) {
		adfLog_print((void*)g_log_buf, NULL);
	}
	mutex_unlock(&adfDspLock);
}

EXPORT_SYMBOL_GPL(adfDebug_printLog);

#if 0
/*
 * _adfDebug_logDumpHandler()
 * ----------------------------------------
 * dump dsp log into kernel log periodically
 *
 * Input:
 *   void *arg             - arg of the adf_log_dump thread
 * Return:
 *   0 for OK, or < 0 for error code
 */
static int _adfDebug_logDumpHandler(void *arg)
{
	int interval = (int)arg;

	while (!kthread_should_stop()) {
		msleep(interval);

		/* dump dsp log of each core */
		if (adf_dbg_chk_run()) {
			adfDebug_printLog(ADF_LOG_DUMP_RECENT);
		}
	}

	return 0;
}

/*
 * adfDebug_initLogDumpThread()
 * ----------------------------------------
 * init the kthread (or workqueue) to dump dsp log into kernel log periodically
 * this API should be called after ADF load successfully
 *
 * Input:
 *   void *debugCheckRunFunc - the funcptr that check whether dsp is running
 *   int32_t interval        - the log dump interval in ms
 * Return:
 *   None
 */
int adfDebug_initLogDumpThread(int32_t interval, uint32_t log_buf_size)
{
	/* if the log dump thread has been created, then return directly */
	if (adfLogDumpThread) {
		pr_info("adf_dump_log thread has been created already!\n");
		return 0;
	}

	/* allocate 16K log buffer */
	g_log_buf = vmalloc(log_buf_size);
	if(g_log_buf == NULL) {
		pr_err("%s: Error! cannot allocate %d bytes debug buffer\n",__func__, log_buf_size);
		return - EINVAL;
	} else {
		g_log_buf_size = log_buf_size;
		pr_notice("%s allocated debug log buffer of size %d bytes\n",__func__,log_buf_size);
	}

	/* create and wake the thread here,
	 * this API will be called only when ADF header is detected,
	 * so not necessary to check again here */
	adfLogDumpThread = kthread_run(_adfDebug_logDumpHandler,
			(void *)((uintptr_t)interval), "adf_dump_log");
	if (IS_ERR(adfLogDumpThread)) {
		pr_err("Error! cannot create thread for adf_dump_log, %d\n",
				(int)PTR_ERR(adfLogDumpThread));
		adfLogDumpThread = NULL;
	}

	return 0;
}
#endif

void adfDebug_init(void *debugCheckRunFunc, void *debugReadFunc,
                   int32_t interval, uint32_t log_buf_size)
{
	int ret = 0;

    if ((debugCheckRunFunc == NULL) || (debugReadFunc == NULL)) {
        pr_err("Invalid param chk run func %p, read func %p\n", debugCheckRunFunc, debugReadFunc);
        return;
    }
	
	if (log_buf_size == 0) {
        pr_err("%s Invalid log buffer size: 0\n",__func__);
        return;
	}

    if (adf_dbg_chk_run && adf_dbg_mem_read) {
        pr_notice("%s: ignore as already initialized\n",__func__);
        return;
    }

    adf_dbg_chk_run = debugCheckRunFunc;
    adf_dbg_mem_read = debugReadFunc;

    if (interval < ADF_DEBUG_LOG_DUMP_MIN_PERIOD) {
        pr_warn("Warning! log dump interval too small, %d, set to %d\n",
            interval, ADF_DEBUG_LOG_DUMP_MIN_PERIOD);
        interval = ADF_DEBUG_LOG_DUMP_MIN_PERIOD;
    }

	mutex_init(&adfDspLock);

#if 0
	ret = adfDebug_initLogDumpThread(interval,log_buf_size);
	if (ret != 0) {
		pr_notice("%s failed to init back ground dump task\n",__func__);
		return;
	}
#else
    /* allocate 16K log buffer */
	g_log_buf = vmalloc(log_buf_size);
	if(g_log_buf == NULL) {
		pr_err("%s: Error! cannot allocate %d bytes debug buffer\n",__func__, log_buf_size);
	} else {
		g_log_buf_size = log_buf_size;
		pr_notice("%s allocated debug log buffer of size %d bytes\n",__func__,log_buf_size);
		adfDebug_initFs();
	}
#endif
	return;
}
EXPORT_SYMBOL_GPL(adfDebug_init);

