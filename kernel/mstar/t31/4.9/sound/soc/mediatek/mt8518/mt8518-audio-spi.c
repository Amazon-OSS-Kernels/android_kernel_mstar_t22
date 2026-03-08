/*
 * mt8518-audio-spi.c  --  MT8518 Audio SPI driver
 *
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Hidalgo Huang <hidalgo.huang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/kthread.h>
#include <linux/sched/rt.h>
#include <linux/atomic.h>
#include <linux/timekeeping.h>
#include <linux/dma-mapping.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "hifi4dsp_load/hifi4dsp_load.h"
#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
#include "hifi4dsp_wdt/hifi4dsp_wdt.h"
#endif
#include "mt8518-adsp-common.h"
#include "mt8518-adsp-controls.h"
#include "mt8518-adsp-debug.h"
#include "mt8518-afe-common.h"
#include "mt8518-snd-utils.h"
#ifdef CONFIG_SND_SOC_COMPRESS
#include "mt8518-adsp-compress.h"
#endif

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
#include "pinctrl-mt8518-ipi.h"
#endif

#define DSP_READY_TIMEOUT_MS		(3000)
#define DSP_INIT_TIMEOUT_MS		(500)
/* TODO: need to fine tune overhead further */
#define SPI_RW_OVERHEAD_NS		(2500000)
#define SPI_RW_BITS_PER_SEC		(52*1000*1000)
#define IRQ_NOTIFY_OVERHEAD_NS		(5000000)
#define COPY_WORKER_OVERHEAD_NS		(5000000)
#define VA_UL_PREROLL_PERIOD_COUNT	(10)
#define DMA_HRT_JITTER_THRESHOLD_NS	(50)

static struct mt8518_audio_spi_priv *g_priv;

static int mt8518_init_dai_memory(struct snd_pcm_substream *substream, int id)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	struct mt8518_adsp_dai_memory *dai_mem = &priv->dai_mem[id];

	dai_mem->id = id;

	dai_mem->substream = substream;

	mt8518_adsp_init_dsp_dmab(&dai_mem->dsp_dmab);

	mt8518_adsp_init_host_dmab(&dai_mem->host_dmab);

	mt8518_adsp_init_preroll_dmab(&dai_mem->preroll_dmab);

	return 0;
}

static void mt8518_reset_dai_memory(struct mt8518_adsp_dai_memory *dai_mem)
{
	dai_mem->substream = NULL;

	memset(&dai_mem->dsp_dmab, 0, sizeof(struct mt8518_dsp_dma_buffer));

	memset(&dai_mem->host_dmab, 0, sizeof(struct mt8518_host_dma_buffer));

	memset(&dai_mem->preroll_dmab, 0,
	       sizeof(struct mt8518_preroll_dma_buffer));
}

static void mt8518_reset_dai_dma_offset(struct mt8518_adsp_dai_memory *dai_mem)
{
	mt8518_adsp_reset_dsp_dmab_offset(&dai_mem->dsp_dmab);

	mt8518_adsp_reset_host_dmab_offset(&dai_mem->host_dmab);

	mt8518_adsp_reset_preroll_dmab_offset(&dai_mem->preroll_dmab);
}

static bool mt8518_wait_dsp_ready(struct mt8518_audio_spi_priv *priv)
{
	wait_event_interruptible_timeout(priv->wait_dsp,
		priv->dsp_ready, msecs_to_jiffies(DSP_READY_TIMEOUT_MS));

	return priv->dsp_ready;
}

static bool mt8518_wait_dsp_init(struct mt8518_audio_spi_priv *priv)
{
	wait_event_interruptible_timeout(priv->wait_dsp,
		(priv->dsp_ready && priv->dsp_init),
		msecs_to_jiffies(DSP_INIT_TIMEOUT_MS));

	return priv->dsp_init;
}

#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
static void mt8518_reset_dsp_ready(struct mt8518_audio_spi_priv *priv)
{
	priv->dsp_ready = false;
}

static void mt8518_reset_dsp_init(struct mt8518_audio_spi_priv *priv)
{
	priv->dsp_init = false;
}
#endif

static void mt8518_cancel_dai_works(struct mt8518_adsp_dai_memory *dai_mem)
{
	if (mt8518_adsp_need_dl_dma_copy(dai_mem->id)) {
		hrtimer_cancel(&dai_mem->dma_elapse_hrt);
		kthread_cancel_work_sync(&dai_mem->dma_work);
		atomic_set(&dai_mem->dma_pending_elapse, 0);
	} else if (mt8518_adsp_need_ul_dma_copy(dai_mem->id)) {
		kthread_cancel_work_sync(&dai_mem->dma_work);
	}
}

#define AUDIO_SPI_DEFAULT_INFO \
	(SNDRV_PCM_INFO_MMAP | \
	 SNDRV_PCM_INFO_MMAP_VALID | \
	 SNDRV_PCM_INFO_INTERLEAVED)

#define AUDIO_SPI_DEFAULT_BUFFER_BYTES_MAX	(10 * 1024)
#define AUDIO_SPI_DEFAULT_PERIOD_BYTES_MAX	(5 * 1024)
#define AUDIO_SPI_DEFAULT_PERIOD_BYTES_MIN	(64)
#define AUDIO_SPI_DEFAULT_PERIODS_MIN		(2)
#define AUDIO_SPI_DEFAULT_PERIODS_MAX		(160)

#define AUDIO_SPI_DEFAULT_PCM_HW \
	.info = AUDIO_SPI_DEFAULT_INFO, \
	.buffer_bytes_max = AUDIO_SPI_DEFAULT_BUFFER_BYTES_MAX, \
	.period_bytes_min = AUDIO_SPI_DEFAULT_PERIOD_BYTES_MIN, \
	.period_bytes_max = AUDIO_SPI_DEFAULT_PERIOD_BYTES_MAX, \
	.periods_min = AUDIO_SPI_DEFAULT_PERIODS_MIN, \
	.periods_max = AUDIO_SPI_DEFAULT_PERIODS_MAX,

static struct snd_pcm_hardware mt8518_audio_spi_pcm_hw[MT8518_ADSP_FE_CNT] = {
	[MT8518_ADSP_FE_MIC_RECORD] = {
		AUDIO_SPI_DEFAULT_PCM_HW
	},
#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	[MT8518_ADSP_FE_VA_HOSTLESS] = {
		AUDIO_SPI_DEFAULT_PCM_HW
	},
	[MT8518_ADSP_FE_VA_UPLOAD] = {
		AUDIO_SPI_DEFAULT_PCM_HW
	},
#endif
#ifdef CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK
	[MT8518_ADSP_FE_PCM_PLAYBACK1] = {
		AUDIO_SPI_DEFAULT_PCM_HW
	},
#endif
#ifdef CONFIG_SND_SOC_MT8570_ADSP_XAF_2ND_CAPTURE
	[MT8570_ADSP_FE_XAF_2ND_RECORD] = {
		AUDIO_SPI_DEFAULT_PCM_HW
	},
#endif
};

static int mt8518_update_mem_infos(struct mt8518_audio_spi_priv *priv)
{
	int scene;
	struct dsp_mem_ifo *mem_info;
	struct ipi_msg_t ipi_msg;
	struct snd_pcm_hardware *pcm_hw;
	int ret = 0;
	int i;

	for (i = 0; i < MT8518_ADSP_FE_CNT; i++) {
		scene = mt8518_adsp_get_scene_by_dai_id(i);
		if (scene < 0)
			continue;

		memset(&ipi_msg, 0, sizeof(ipi_msg));

		ret = mt8518_adsp_send_ipi_cmd(&ipi_msg,
					       scene,
					       AUDIO_IPI_LAYER_TO_DSP,
					       AUDIO_IPI_MSG_ONLY,
					       AUDIO_IPI_MSG_NEED_ACK,
					       MSG_TO_DSP_HOST_QUERY_MEM_INFO,
					       0, 0, NULL);
		if (ret)
			return ret;

		mem_info = &priv->mem_info[i];
		if (!mem_info)
			continue;

		AUDIO_COPY_DSP_MEM_INFO(ipi_msg.payload, mem_info);

		pcm_hw = &mt8518_audio_spi_pcm_hw[i];
		if (!pcm_hw)
			continue;

		if (mem_info->mem_size > 0) {
			pcm_hw->buffer_bytes_max = mem_info->mem_size;
			pcm_hw->period_bytes_max = pcm_hw->buffer_bytes_max /
						   pcm_hw->periods_min;
			pcm_hw->periods_max = pcm_hw->buffer_bytes_max /
					      pcm_hw->period_bytes_min;
		}
	}

	return ret;
}

static int dma_fetch_pending_and_update(struct mt8518_adsp_dai_memory *dai_mem)
{
	struct mt8518_host_dma_buffer *host_dmab = &dai_mem->host_dmab;
	int pend = 0;
	unsigned long flags;
	struct timespec ts1;

	spin_lock_irqsave(&dai_mem->host_dma_lock, flags);

	pend = atomic_read(&dai_mem->dma_pending_elapse);
	if (pend <= 0) {
		spin_unlock_irqrestore(&dai_mem->host_dma_lock, flags);
		return pend;
	}

	host_dmab->hw_offset += host_dmab->transfer;
	host_dmab->hw_offset %= host_dmab->buf_size;

	snd_pcm_gettime(dai_mem->substream->runtime, &ts1);

	dai_mem->prev_elapse_ts = ts1;

	atomic_sub(1, &dai_mem->dma_pending_elapse);

	spin_unlock_irqrestore(&dai_mem->host_dma_lock, flags);

	return pend;
}

static int dma_peek_pending_hw_offset(struct mt8518_adsp_dai_memory *dai_mem,
	uint32_t *hw_off)
{
	struct mt8518_host_dma_buffer *host_dmab = &dai_mem->host_dmab;
	int pend = 0;
	unsigned long flags;

	spin_lock_irqsave(&dai_mem->host_dma_lock, flags);

	pend = atomic_read(&dai_mem->dma_pending_elapse);

	if (hw_off)
		*hw_off = host_dmab->hw_offset;

	spin_unlock_irqrestore(&dai_mem->host_dma_lock, flags);

	return pend;
}

static enum hrtimer_restart dma_elapse_hrtimer_callback(struct hrtimer *hrt)
{
	struct mt8518_adsp_dai_memory *dai_mem = container_of(hrt,
		struct mt8518_adsp_dai_memory, dma_elapse_hrt);
	struct snd_pcm_substream *substream = dai_mem->substream;
	int elapse = 0;
	ktime_t now;

	if (!substream) {
		dev_info(dai_mem->dev, "%s [%d] invalid substream\n",
			 __func__, dai_mem->id);
		return HRTIMER_NORESTART;
	}

	if (!snd_pcm_running(substream)) {
		dev_info(dai_mem->dev,
			 "%s [%d] skip process in non-running state\n",
			 __func__, dai_mem->id);
		return HRTIMER_NORESTART;
	}

	now = ktime_get();

	if (ktime_before(ktime_add_ns(now, DMA_HRT_JITTER_THRESHOLD_NS),
			 dai_mem->target_elapse_kt)) {
		hrtimer_start_range_ns(&dai_mem->dma_elapse_hrt,
			dai_mem->target_elapse_kt,
			DMA_HRT_JITTER_THRESHOLD_NS,
			HRTIMER_MODE_ABS);
		return HRTIMER_RESTART;
	}

	elapse = dma_fetch_pending_and_update(dai_mem);
	if (elapse <= 0) {
		struct timespec ts1 = ktime_to_timespec(now);

		dev_info(dai_mem->dev,
		"%s [%d] no pend elapse(adsp hw 0x%x appl 0x%x)\n",
		__func__, dai_mem->id,
		dai_mem->dsp_dmab.hw_offset,
		dai_mem->dsp_dmab.appl_offset);

		dev_info(dai_mem->dev,
		"%s [%d] elapse(%llu ns) transfer %llu data 0x%x\n",
		__func__, dai_mem->id,
		(timespec_to_ns(&ts1) -
		timespec_to_ns(&dai_mem->trigger_start_ts)),
		dai_mem->total_transfer_bytes,
		atomic_read(&dai_mem->host_dmab.data_size));

		snd_pcm_stop_xrun(substream);

		return HRTIMER_NORESTART;
	}

	snd_pcm_period_elapsed(substream);

	dai_mem->target_elapse_kt = ktime_add_ns(dai_mem->target_elapse_kt,
		dai_mem->period_time_ns);

	hrtimer_start_range_ns(&dai_mem->dma_elapse_hrt,
		dai_mem->target_elapse_kt,
		DMA_HRT_JITTER_THRESHOLD_NS,
		HRTIMER_MODE_ABS);

	return HRTIMER_RESTART;
}

static int mt8518_audio_spi_fe_startup(struct snd_pcm_substream *substream,
				       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	struct snd_pcm_hardware *pcm_hw;
	struct dsp_mem_ifo *mem_info;
	int ret = 0;

	dev_dbg(rtd->dev, "%s [%d]\n", __func__, id);

	if (!mt8518_wait_dsp_init(priv)) {
		dev_info(priv->dev, "%s dsp init timeout\n", __func__);
		return -ENXIO;
	}

	pcm_hw = &mt8518_audio_spi_pcm_hw[id];
	mem_info = &priv->mem_info[id];

	if (mem_info->mem_alignment > 0)
		snd_pcm_hw_constraint_step(substream->runtime, 0,
			SNDRV_PCM_HW_PARAM_BUFFER_BYTES,
			mem_info->mem_alignment);

	if (mem_info->period_time_us_alignment > 0)
		snd_pcm_hw_constraint_step(substream->runtime, 0,
			SNDRV_PCM_HW_PARAM_PERIOD_TIME,
			mem_info->period_time_us_alignment);

	snd_soc_set_runtime_hwparams(substream, pcm_hw);

	if (scene >= 0) {
		struct host_startup_param startup_param;

		memset(&startup_param, 0, sizeof(startup_param));

		startup_param.host_handler = (uintptr_t)&priv->dai_mem[id];

		mt8518_adsp_notify_power_state(scene);

		ret = mt8518_adsp_send_ipi_cmd(NULL,
			scene,
			AUDIO_IPI_LAYER_TO_DSP,
			AUDIO_IPI_PAYLOAD,
			AUDIO_IPI_MSG_NEED_ACK,
			MSG_TO_DSP_HOST_PORT_STARTUP,
			sizeof(startup_param),
			0,
			(char *)&startup_param);
	}

	mt8518_init_dai_memory(substream, id);

	return ret;
}

static void mt8518_audio_spi_fe_shutdown(struct snd_pcm_substream *substream,
					 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);

	dev_dbg(rtd->dev, "%s [%d]\n", __func__, id);

	if (scene >= 0) {
		mt8518_cancel_dai_works(&priv->dai_mem[id]);

		mt8518_adsp_send_ipi_cmd(NULL,
					 scene,
					 AUDIO_IPI_LAYER_TO_DSP,
					 AUDIO_IPI_MSG_ONLY,
					 AUDIO_IPI_MSG_NEED_ACK,
					 MSG_TO_DSP_HOST_CLOSE,
					 0, 0, NULL);

		mt8518_adsp_clear_power_state(scene);
	}

	mt8518_reset_dai_memory(&priv->dai_mem[id]);
}

static int mt8518_audio_spi_fe_hw_params(struct snd_pcm_substream *substream,
					 struct snd_pcm_hw_params *params,
					 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;

	ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
	if (ret < 0)
		return ret;

	if (scene >= 0) {
		struct host_ipc_msg_hw_param ipc_hw_param;
		struct dsp_ipc_msg_hw_param ack_hw_param;
		struct io_ipc_ring_buf_shared *shared_buf;
		struct ipi_msg_t ipi_msg;
		struct mt8518_adsp_dai_memory *dai_mem = &priv->dai_mem[id];

		memset(&ipc_hw_param, 0, sizeof(ipc_hw_param));
		memset(&ipi_msg, 0, sizeof(ipi_msg));

		ipc_hw_param.sample_rate = params_rate(params);
		ipc_hw_param.channel_num = params_channels(params);
		ipc_hw_param.bitwidth = params_width(params);
		ipc_hw_param.period_size = params_period_size(params);
		ipc_hw_param.period_count = params_periods(params);

		dev_dbg(rtd->dev,
			"%s [%d] rate %u ch %u bits %u period %u-%u\n",
			__func__, id,
			ipc_hw_param.sample_rate,
			ipc_hw_param.channel_num,
			ipc_hw_param.bitwidth,
			ipc_hw_param.period_size,
			ipc_hw_param.period_count);

		ret = mt8518_adsp_send_ipi_cmd(&ipi_msg,
					       scene,
					       AUDIO_IPI_LAYER_TO_DSP,
					       AUDIO_IPI_PAYLOAD,
					       AUDIO_IPI_MSG_NEED_ACK,
					       MSG_TO_DSP_HOST_HW_PARAMS,
					       sizeof(ipc_hw_param),
					       0,
					       (char *)&ipc_hw_param);

		if (ret != 0)
			return ret;

		ret = mt8518_adsp_verify_ack_hw_param(&ipc_hw_param,
			&ipi_msg, &ack_hw_param);

		if (ret != 0)
			return ret;

		shared_buf = &ack_hw_param.SharedRingBuffer;

		dai_mem->dsp_dmab.buf_paddr = shared_buf->start_addr;
		dai_mem->dsp_dmab.buf_size = shared_buf->size_bytes;
		dai_mem->dsp_dmab.hw_offset_paddr =
			shared_buf->ptr_to_hw_offset_bytes;
		dai_mem->dsp_dmab.appl_offset_paddr =
			shared_buf->ptr_to_appl_offset_bytes;

		dai_mem->host_dmab.buf_vaddr = substream->runtime->dma_area;
		dai_mem->host_dmab.buf_size = substream->runtime->dma_bytes;
		dai_mem->host_dmab.transfer =
			params_period_bytes(params);

		{
			u64 tmp = (u64)ipc_hw_param.period_size * NSEC_PER_SEC;

			do_div(tmp, ipc_hw_param.sample_rate);

			dai_mem->period_time_ns = tmp;
		}

#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
		if ((dai_mem->id == MT8518_ADSP_FE_VA_UPLOAD) &&
		    (priv->va_ul_preroll_period_count > 0)) {
			dai_mem->preroll_dmab.buf_size =
				dai_mem->host_dmab.transfer *
				priv->va_ul_preroll_period_count;

			dai_mem->preroll_dmab.buf_vaddr =
				dma_alloc_coherent(rtd->card->dev,
					dai_mem->preroll_dmab.buf_size,
					&dai_mem->preroll_dmab.buf_paddr,
					GFP_KERNEL);

			if (!dai_mem->preroll_dmab.buf_vaddr) {
				dev_info(rtd->dev,
					 "%s [%d] fail to alloc preroll %u\n",
					 __func__, id,
					 dai_mem->preroll_dmab.buf_size);
				return -ENOMEM;
			}

			dai_mem->preroll_dmab.data_size = 0;
		}
#endif
	}

	return ret;
}

static int mt8518_audio_spi_fe_hw_free(struct snd_pcm_substream *substream,
				       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;
	struct mt8518_adsp_dai_memory *dai_mem = &priv->dai_mem[id];

	dev_dbg(rtd->dev, "%s [%d]\n", __func__, id);

	if (scene >= 0) {
		mt8518_cancel_dai_works(&priv->dai_mem[id]);

		ret = mt8518_adsp_send_ipi_cmd(NULL,
					       scene,
					       AUDIO_IPI_LAYER_TO_DSP,
					       AUDIO_IPI_MSG_ONLY,
					       AUDIO_IPI_MSG_NEED_ACK,
					       MSG_TO_DSP_HOST_HW_FREE,
					       0, 0, NULL);
	}

	if (dai_mem->preroll_dmab.buf_vaddr) {
		dma_free_coherent(rtd->card->dev,
			dai_mem->preroll_dmab.buf_size,
			dai_mem->preroll_dmab.buf_vaddr,
			dai_mem->preroll_dmab.buf_paddr);

		dai_mem->preroll_dmab.buf_vaddr = NULL;
	}

	return snd_pcm_lib_free_pages(substream);
}

static int mt8518_audio_spi_fe_prepare(struct snd_pcm_substream *substream,
				       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;

	dev_dbg(rtd->dev, "%s [%d]\n", __func__, id);

	if (scene >= 0) {
		mt8518_cancel_dai_works(&priv->dai_mem[id]);

		priv->dai_mem[id].total_transfer_bytes = 0;

		ret = mt8518_adsp_send_ipi_cmd(NULL,
					       scene,
					       AUDIO_IPI_LAYER_TO_DSP,
					       AUDIO_IPI_MSG_ONLY,
					       AUDIO_IPI_MSG_NEED_ACK,
					       MSG_TO_DSP_HOST_PREPARE,
					       0, 0, NULL);
	}

	return ret;
}

static int mt8518_audio_spi_fe_trigger(struct snd_pcm_substream *substream,
				       int cmd,
				       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;
	struct mt8518_adsp_dai_memory *dai_mem = &priv->dai_mem[id];
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;

	dev_dbg(rtd->dev, "%s [%d] cmd %d\n", __func__, id, cmd);

	if (scene < 0)
		return 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		mt8518_adsp_send_ipi_cmd(NULL,
					 scene,
					 AUDIO_IPI_LAYER_TO_DSP,
					 AUDIO_IPI_MSG_ONLY,
					 AUDIO_IPI_MSG_DIRECT_SEND,
					 MSG_TO_DSP_HOST_TRIGGER_START,
					 0, 0, NULL);

		if (mt8518_adsp_need_dl_dma_copy(dai_mem->id)) {
			snd_pcm_gettime(substream->runtime,
				&dai_mem->trigger_start_ts);

			dai_mem->target_elapse_kt =
				timespec_to_ktime(dai_mem->trigger_start_ts);
			dai_mem->target_elapse_kt =
				ktime_add_ns(dai_mem->target_elapse_kt,
					     dai_mem->period_time_ns);
			hrtimer_start_range_ns(&dai_mem->dma_elapse_hrt,
				dai_mem->target_elapse_kt,
				DMA_HRT_JITTER_THRESHOLD_NS,
				HRTIMER_MODE_ABS);
			dai_mem->prev_elapse_ts = dai_mem->trigger_start_ts;
			dai_mem->prev_hw_offset = 0;
		}

#if defined(MT8518_AUDIO_SPI_UL_COPY_LATENCY_DEBUG)
		if (mt8518_adsp_need_ul_dma_copy(dai_mem->id))
			ktime_get_ts64(&dai_mem->prev_copy_ts);
#endif

#if defined(MT8518_AUDIO_SPI_UL_IRQ_LATENCY_DEBUG) || \
	defined(MT8518_AUDIO_SPI_DL_IRQ_LATENCY_DEBUG)
		ktime_get_ts64(&dai_mem->prev_irq_ts);
#endif
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		mt8518_adsp_send_ipi_cmd(NULL,
					 scene,
					 AUDIO_IPI_LAYER_TO_DSP,
					 AUDIO_IPI_MSG_ONLY,
					 AUDIO_IPI_MSG_DIRECT_SEND,
					 MSG_TO_DSP_HOST_TRIGGER_STOP,
					 0, 0, NULL);

		mt8518_reset_dai_dma_offset(dai_mem);
		break;
	default:
		break;
	}

	return ret;
}

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
static int mt8518_afe_get_scene_by_dai_id(void)
{
	return TASK_SCENE_INTERLINK_HOST_TO_DSP;
}
#endif

static int mt8518_afe_etdm2_startup(void)
{
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	int scene = mt8518_afe_get_scene_by_dai_id();

	mt8518_adsp_notify_power_state(scene);

	mt8518_adsp_send_ipi_cmd(NULL,
		scene,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_NEED_ACK,
		MSG_TO_DSP_DSP_PORT_STARTUP,
		0, 0, NULL);

#endif

	return 0;
}

static void mt8518_afe_etdm2_shutdown(void)
{
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	int scene = mt8518_afe_get_scene_by_dai_id();

	mt8518_adsp_send_ipi_cmd(NULL,
		scene,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_NEED_ACK,
		MSG_TO_DSP_DSP_CLOSE,
		0, 0, NULL);

	mt8518_adsp_clear_power_state(scene);

#endif

}

#ifdef CONFIG_SND_SOC_MT8518
static int mt8518_spi_mt8570_gpio_probe(void)
{
	int ret = 0;

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	ret = gpio_send_ipi_msg(11, SET_MODE_IPI, GPIO_MODE_02);
	if (ret < 0)
		printk("%s fail to set gpio 11\n", __FUNCTION__);
	ret = gpio_send_ipi_msg(12, SET_MODE_IPI, GPIO_MODE_02);
	if (ret < 0)
		printk("%s fail to set gpio 12\n", __FUNCTION__);
#endif

	return ret;
}
#endif

static int mt8518_afe_etdm2_hw_params(struct snd_pcm_hw_params *params)
{
	int ret = 0;
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	int scene = mt8518_afe_get_scene_by_dai_id();

	struct host_ipc_msg_hw_param ipc_hw_param;
	struct dsp_ipc_msg_hw_param ack_hw_param;
	struct ipi_msg_t ipi_msg;

	memset(&ipc_hw_param, 0, sizeof(ipc_hw_param));
	memset(&ipi_msg, 0, sizeof(ipi_msg));

	ipc_hw_param.sample_rate = 48000;
	ipc_hw_param.channel_num = 2;
	ipc_hw_param.bitwidth = 32;
	ipc_hw_param.period_size = 2048;//params_period_size(params);
	ipc_hw_param.period_count = 2;//params_periods(params);

	ipc_hw_param.tdm_config.mclk_freq = 128*48000;
	ipc_hw_param.tdm_config.lrck_width = 32;
	ipc_hw_param.tdm_config.slave_mode = 1;
	ipc_hw_param.tdm_config.format = mt8518_adsp_etdm_format(MT8518_ETDM_FORMAT_I2S);
	ipc_hw_param.tdm_config.lrck_invert = 0;
	ipc_hw_param.tdm_config.bck_invert = 0;
	ipc_hw_param.tdm_config.data_mode = 0;
	ipc_hw_param.tdm_config.clock_mode = 0;
	ipc_hw_param.tdm_config.bck_per_channel = ipc_hw_param.bitwidth;
#if 0
	dev_dbg(afe->dev,
		"%s [%u] rate %u ch %u bit %u period %u-%u\n",
		__func__, stream,
		ipc_hw_param.sample_rate,
		ipc_hw_param.channel_num,
		ipc_hw_param.bitwidth,
		ipc_hw_param.period_size,
		ipc_hw_param.period_count);
#endif
	ret = mt8518_adsp_send_ipi_cmd(&ipi_msg,
		scene,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_PAYLOAD,
		AUDIO_IPI_MSG_NEED_ACK,
		MSG_TO_DSP_DSP_HW_PARAMS,
		sizeof(ipc_hw_param),
		0,
		(char *)&ipc_hw_param);

	if (ret != 0)
		return ret;

	ret = mt8518_adsp_verify_ack_hw_param(&ipc_hw_param,
		&ipi_msg, &ack_hw_param);

	if (ret != 0)
		return ret;

	mt8518_spi_mt8570_gpio_probe();

#endif

	return ret;
}


int mt8518_afe_etdm2_hw_free(void)
{
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	int scene = mt8518_afe_get_scene_by_dai_id();

	mt8518_adsp_send_ipi_cmd(NULL,
		scene,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_NEED_ACK,
		MSG_TO_DSP_DSP_HW_FREE,
		0, 0, NULL);

#endif

	return 0;
}

static int mt8518_afe_etdm2_prepare(void)
{
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	int scene = mt8518_afe_get_scene_by_dai_id();

	mt8518_adsp_send_ipi_cmd(NULL,
		scene,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_NEED_ACK,
		MSG_TO_DSP_DSP_PREPARE,
		0, 0, NULL);

#endif

	return 0;
}

int mt8518_afe_etdm2_trigger(int cmd)
{
	int ret = 0;
	int scene = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	scene = mt8518_afe_get_scene_by_dai_id();

	mt8518_adsp_send_ipi_cmd(NULL,
		scene,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_DIRECT_SEND,
		MSG_TO_DSP_DSP_TRIGGER_START,
		0, 0, NULL);
#endif
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	scene = mt8518_afe_get_scene_by_dai_id();

	mt8518_adsp_send_ipi_cmd(NULL,
		scene,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_DIRECT_SEND,
		MSG_TO_DSP_DSP_TRIGGER_STOP,
		0, 0, NULL);
#endif
		break;
	default:
		break;
	}

	return ret;
}

static int mt8518_audio_spi_be_startup(struct snd_pcm_substream *substream,
				       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;

	dev_dbg(rtd->dev, "%s [%d]\n", __func__, id);

	if (scene >= 0) {
		if (scene != TASK_SCENE_VA)
			mt8518_adsp_notify_power_state(scene);

		ret = mt8518_adsp_send_ipi_cmd(NULL,
					       scene,
					       AUDIO_IPI_LAYER_TO_DSP,
					       AUDIO_IPI_MSG_ONLY,
					       AUDIO_IPI_MSG_NEED_ACK,
					       MSG_TO_DSP_DSP_PORT_STARTUP,
					       0, 0, NULL);
	}

	mt8518_afe_etdm2_startup();

	return ret;
}

static void mt8518_audio_spi_be_shutdown(struct snd_pcm_substream *substream,
					 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);

	dev_dbg(rtd->dev, "%s [%d]\n", __func__, id);

	if (scene >= 0) {
		mt8518_adsp_send_ipi_cmd(NULL,
					 scene,
					 AUDIO_IPI_LAYER_TO_DSP,
					 AUDIO_IPI_MSG_ONLY,
					 AUDIO_IPI_MSG_NEED_ACK,
					 MSG_TO_DSP_DSP_CLOSE,
					 0, 0, NULL);

		if (scene != TASK_SCENE_VA)
			mt8518_adsp_clear_power_state(scene);
	}

	mt8518_afe_etdm2_shutdown();
}

static int mt8518_audio_spi_be_hw_params(struct snd_pcm_substream *substream,
					 struct snd_pcm_hw_params *params,
					 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;
	struct host_ipc_msg_hw_param ipc_hw_param;
	struct dsp_ipc_msg_hw_param ack_hw_param;
	struct ipi_msg_t ipi_msg;

	if (scene < 0)
		return 0;

	memset(&ipc_hw_param, 0, sizeof(ipc_hw_param));
	memset(&ipi_msg, 0, sizeof(ipi_msg));

	ipc_hw_param.sample_rate = params_rate(params);
	ipc_hw_param.channel_num = params_channels(params);
	ipc_hw_param.bitwidth = params_width(params);
	ipc_hw_param.period_size = params_period_size(params);
	ipc_hw_param.period_count = params_periods(params);

	dev_dbg(rtd->dev,
		"%s [%d] rate %u ch %u bits %u period %u-%u\n",
		__func__, id,
		ipc_hw_param.sample_rate,
		ipc_hw_param.channel_num,
		ipc_hw_param.bitwidth,
		ipc_hw_param.period_size,
		ipc_hw_param.period_count);

	if ((scene == TASK_SCENE_MIXER_PRIMARY) ||
		(scene == TASK_SCENE_VA) ||
		(scene == TASK_SCENE_XAF_2ND_CAPTURE)) {
		struct mt8518_audio_spi_be_data *be_data =
			&priv->be_data[id - MT8518_ADSP_BE_START];
		struct TDM_config_shared *config =
			&ipc_hw_param.tdm_config;
		unsigned int mclk = be_data->mclk_freq;
		unsigned int lrck_width = be_data->lrck_width;

		if (mclk == 0)
			mclk = 256 * ipc_hw_param.sample_rate;

		if (lrck_width == 0)
			lrck_width = ipc_hw_param.bitwidth;

		dev_dbg(rtd->dev,
			"%s [%d] mclk %u lrck_width %u slave %d format %u\n",
			__func__, id,
			mclk,
			lrck_width,
			be_data->slave_mode,
			be_data->format);

		dev_dbg(rtd->dev,
			"%s [%d] lrck_inv %d bck_inv %d data %u clock %u\n",
			__func__, id,
			be_data->lrck_inv,
			be_data->bck_inv,
			be_data->data_mode,
			be_data->clock_mode);

		config->mclk_freq = mclk;
		config->lrck_width = lrck_width;
		config->slave_mode = be_data->slave_mode;
		config->format = mt8518_adsp_etdm_format(be_data->format);
		config->lrck_invert = be_data->lrck_inv;
		config->bck_invert = be_data->bck_inv;
		config->data_mode =
			mt8518_adsp_etdm_data_mode(be_data->data_mode);
		config->clock_mode =
			mt8518_adsp_etdm_clock_mode(be_data->clock_mode);
		config->bck_per_channel = ipc_hw_param.bitwidth;
	}

	ret = mt8518_adsp_send_ipi_cmd(&ipi_msg,
				       scene,
				       AUDIO_IPI_LAYER_TO_DSP,
				       AUDIO_IPI_PAYLOAD,
				       AUDIO_IPI_MSG_NEED_ACK,
				       MSG_TO_DSP_DSP_HW_PARAMS,
				       sizeof(ipc_hw_param),
				       0,
				       (char *)&ipc_hw_param);

	if (ret != 0)
		return ret;

	ret = mt8518_adsp_verify_ack_hw_param(&ipc_hw_param,
		&ipi_msg, &ack_hw_param);
	if (ret != 0)
		return ret;

	mt8518_afe_etdm2_hw_params(params);

	return ret;
}

static int mt8518_audio_spi_be_hw_free(struct snd_pcm_substream *substream,
				       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;

	dev_dbg(rtd->dev, "%s [%d]\n", __func__, id);

	if (scene >= 0) {
		mt8518_adsp_send_ipi_cmd(NULL,
					 scene,
					 AUDIO_IPI_LAYER_TO_DSP,
					 AUDIO_IPI_MSG_ONLY,
					 AUDIO_IPI_MSG_NEED_ACK,
					 MSG_TO_DSP_DSP_HW_FREE,
					 0, 0, NULL);
	}

	mt8518_afe_etdm2_hw_free();

	return ret;
}

static int mt8518_audio_spi_be_prepare(struct snd_pcm_substream *substream,
				       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;

	dev_dbg(rtd->dev, "%s [%d]\n", __func__, id);

	if (scene >= 0) {
		mt8518_adsp_send_ipi_cmd(NULL,
					 scene,
					 AUDIO_IPI_LAYER_TO_DSP,
					 AUDIO_IPI_MSG_ONLY,
					 AUDIO_IPI_MSG_NEED_ACK,
					 MSG_TO_DSP_DSP_PREPARE,
					 0, 0, NULL);
	}

	mt8518_afe_etdm2_prepare();

	return ret;
}

static int mt8518_audio_spi_be_trigger(struct snd_pcm_substream *substream,
				       int cmd,
				       struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	int id = rtd->cpu_dai->id;
	int scene = mt8518_adsp_get_scene_by_dai_id(id);
	int ret = 0;

	dev_dbg(rtd->dev, "%s [%d] cmd %d\n", __func__, id, cmd);

	if (scene < 0)
		return 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		mt8518_adsp_send_ipi_cmd(NULL,
					 scene,
					 AUDIO_IPI_LAYER_TO_DSP,
					 AUDIO_IPI_MSG_ONLY,
					 AUDIO_IPI_MSG_DIRECT_SEND,
					 MSG_TO_DSP_DSP_TRIGGER_START,
					 0, 0, NULL);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		mt8518_adsp_send_ipi_cmd(NULL,
					 scene,
					 AUDIO_IPI_LAYER_TO_DSP,
					 AUDIO_IPI_MSG_ONLY,
					 AUDIO_IPI_MSG_DIRECT_SEND,
					 MSG_TO_DSP_DSP_TRIGGER_STOP,
					 0, 0, NULL);
		break;
	default:
		break;
	}

	mt8518_afe_etdm2_trigger(cmd);

	return ret;
}

static int mt8518_audio_spi_be_set_fmt(struct snd_soc_dai *dai,
	unsigned int fmt)
{
	struct mt8518_audio_spi_priv *priv = snd_soc_dai_get_drvdata(dai);
	struct mt8518_audio_spi_be_data *be_data =
		&priv->be_data[dai->id - MT8518_ADSP_BE_START];
	int ret;

	ret = mt8518_snd_get_etdm_format(fmt & SND_SOC_DAIFMT_FORMAT_MASK);
	if (ret >= 0)
		be_data->format = ret;
	else
		return -EINVAL;

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		be_data->bck_inv = false;
		be_data->lrck_inv = false;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		be_data->bck_inv = false;
		be_data->lrck_inv = true;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		be_data->bck_inv = true;
		be_data->lrck_inv = false;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		be_data->bck_inv = true;
		be_data->lrck_inv = true;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		be_data->slave_mode = true;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		be_data->slave_mode = false;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int mt8518_audio_spi_be_set_tdm_slot(struct snd_soc_dai *dai,
					    unsigned int tx_mask,
					    unsigned int rx_mask,
					    int slots,
					    int slot_width)
{
	struct mt8518_audio_spi_priv *priv = snd_soc_dai_get_drvdata(dai);
	struct mt8518_audio_spi_be_data *be_data =
		&priv->be_data[dai->id - MT8518_ADSP_BE_START];

	be_data->lrck_width = slot_width;

	return 0;
}

static int mt8518_audio_spi_be_set_sysclk(struct snd_soc_dai *dai,
					  int clk_id,
					  unsigned int freq,
					  int dir)
{
	struct mt8518_audio_spi_priv *priv = snd_soc_dai_get_drvdata(dai);
	struct mt8518_audio_spi_be_data *be_data =
			&priv->be_data[dai->id - MT8518_ADSP_BE_START];

	be_data->mclk_freq = freq;

	return 0;
}


/* FE DAIs */
static const struct snd_soc_dai_ops mt8518_audio_spi_fe_dai_ops = {
	.startup	= mt8518_audio_spi_fe_startup,
	.shutdown	= mt8518_audio_spi_fe_shutdown,
	.hw_params	= mt8518_audio_spi_fe_hw_params,
	.hw_free	= mt8518_audio_spi_fe_hw_free,
	.prepare	= mt8518_audio_spi_fe_prepare,
	.trigger	= mt8518_audio_spi_fe_trigger,
};

/* BE DAIs */
static const struct snd_soc_dai_ops mt8518_audio_spi_be_dai_ops = {
	.startup	= mt8518_audio_spi_be_startup,
	.shutdown	= mt8518_audio_spi_be_shutdown,
	.hw_params	= mt8518_audio_spi_be_hw_params,
	.hw_free	= mt8518_audio_spi_be_hw_free,
	.prepare	= mt8518_audio_spi_be_prepare,
	.trigger	= mt8518_audio_spi_be_trigger,
	.set_fmt	= mt8518_audio_spi_be_set_fmt,
	.set_tdm_slot	= mt8518_audio_spi_be_set_tdm_slot,
	.set_sysclk	= mt8518_audio_spi_be_set_sysclk,
};

static struct snd_soc_dai_driver mt8518_audio_spi_dais[] = {
	/* FE DAIs */
	{
		.name = "FE_MICR",
		.id = MT8518_ADSP_FE_MIC_RECORD,
		.capture = {
			.stream_name = "FE_MICR",
			.channels_min = 1,
			.channels_max = 16,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8518_audio_spi_fe_dai_ops,
#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	}, {
		.name = "FE_VA_HL",
		.id = MT8518_ADSP_FE_VA_HOSTLESS,
		.capture = {
			.stream_name = "FE_VA_HL",
			.channels_min = 1,
			.channels_max = 16,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8518_audio_spi_fe_dai_ops,
	}, {
		.name = "FE_VA_UL",
		.id = MT8518_ADSP_FE_VA_UPLOAD,
		.capture = {
			.stream_name = "FE_VA_UL",
			.channels_min = 1,
			.channels_max = 16,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8518_audio_spi_fe_dai_ops,
#endif
#ifdef CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK
	}, {
		.name = "FE_PCMP1",
		.id = MT8518_ADSP_FE_PCM_PLAYBACK1,
		.playback = {
			.stream_name = "FE_PCMP1",
			.channels_min = 1,
			.channels_max = 16,
			.rates = SNDRV_PCM_RATE_8000_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8518_audio_spi_fe_dai_ops,
#endif
#ifdef CONFIG_SND_SOC_COMPRESS
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 0
	}, {
		.name = "FE_COMPRP1",
		.id = MT8518_ADSP_FE_COMPR_PLAYBACK1,
		.playback = {
			.stream_name = "FE_COMPRP1",
			.channels_min = 1,
		},
		.compress_new = snd_soc_new_compress,
#endif
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 1
	}, {
		.name = "FE_COMPRP2",
		.id = MT8518_ADSP_FE_COMPR_PLAYBACK2,
		.playback = {
			.stream_name = "FE_COMPRP2",
			.channels_min = 1,
		},
		.compress_new = snd_soc_new_compress,
#endif
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 2
	}, {
		.name = "FE_COMPRP3",
		.id = MT8518_ADSP_FE_COMPR_PLAYBACK3,
		.playback = {
			.stream_name = "FE_COMPRP3",
			.channels_min = 1,
		},
		.compress_new = snd_soc_new_compress,
#endif
#endif
#ifdef CONFIG_SND_SOC_MT8570_ADSP_XAF_2ND_CAPTURE
	}, {
		.name = "FE_XAF_2NDR",
		.id = MT8570_ADSP_FE_XAF_2ND_RECORD,
		.capture = {
			.stream_name = "FE_XAF_2NDR",
			.channels_min = 1,
			.channels_max = 16,
			.rates = SNDRV_PCM_RATE_8000_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8518_audio_spi_fe_dai_ops,
#endif
	}, {
	/* BE DAIs */
		.name = "BE_MICR",
		.id = MT8518_ADSP_BE_MIC_RECORD,
		.capture = {
			.stream_name = "SPI MIC Capture",
			.channels_min = 1,
			.channels_max = 16,
			.rates = SNDRV_PCM_RATE_8000_48000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8518_audio_spi_be_dai_ops,
#if defined(CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK) || \
	defined(CONFIG_SND_SOC_COMPRESS)
	}, {
	/* BE DAIs */
		.name = "BE_PRIP",
		.id = MT8518_ADSP_BE_PRIMARY_PLAYBACK,
		.playback = {
			.stream_name = "Primary Playback",
			.channels_min = 1,
			.channels_max = 16,
			.rates = SNDRV_PCM_RATE_8000_192000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8518_audio_spi_be_dai_ops,
#endif
#ifdef CONFIG_SND_SOC_MT8570_ADSP_XAF_2ND_CAPTURE
	}, {
    /* BE DAIs */
		.name = "BE_XAF_2NDR",
		.id = MT8570_ADSP_BE_XAF_2ND_RECORD,
		.capture = {
			.stream_name = "SPI XAF 2ND Capture",
			.channels_min = 1,
			.channels_max = 16,
			.rates = SNDRV_PCM_RATE_8000_96000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE |
				   SNDRV_PCM_FMTBIT_S32_LE,
		},
		.ops = &mt8518_audio_spi_be_dai_ops,
#endif
	},
};

static const struct snd_soc_dapm_route mt8518_audio_spi_routes[] = {
	{"FE_MICR", NULL, "SPI MIC Capture"},
#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	{"FE_VA_HL", NULL, "SPI MIC Capture"},
	{"FE_VA_UL", NULL, "SPI MIC Capture"},
#endif
#ifdef CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK
	{"Primary Playback", NULL, "FE_PCMP1"},
#endif
#ifdef CONFIG_SND_SOC_COMPRESS
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 0
	{"Primary Playback", NULL, "FE_COMPRP1"},
#endif
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 1
	{"Primary Playback", NULL, "FE_COMPRP2"},
#endif
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 2
	{"Primary Playback", NULL, "FE_COMPRP3"},
#endif
#endif
#ifdef CONFIG_SND_SOC_MT8570_ADSP_XAF_2ND_CAPTURE
	{"FE_XAF_2NDR", NULL, "SPI XAF 2ND Capture"},
#endif
};

static const struct snd_soc_component_driver mt8518_audio_spi_dai_comp_drv = {
	.name = "mt8518-audio-spi-dai",
	.dapm_routes = mt8518_audio_spi_routes,
	.num_dapm_routes = ARRAY_SIZE(mt8518_audio_spi_routes),
};

static uint32_t copy_data_from_dsp_to_host(
	struct mt8518_adsp_dai_memory *dai_mem)
{
	struct mt8518_dsp_dma_buffer *dsp_dmab = &dai_mem->dsp_dmab;
	struct mt8518_host_dma_buffer *host_dmab = &dai_mem->host_dmab;
	uint32_t adsp_dma_buf_paddr = dsp_dmab->buf_paddr;
	uint32_t adsp_dma_buf_size = dsp_dmab->buf_size;
	uint32_t adsp_dma_hw_off = 0;
	uint32_t adsp_dma_appl_off = dsp_dmab->appl_offset;
	unsigned char *host_dma_buf_vaddr = host_dmab->buf_vaddr;
	uint32_t host_dma_buf_size = host_dmab->buf_size;
	uint32_t host_dma_hw_off = host_dmab->hw_offset;
	uint32_t transfer = host_dmab->transfer;
	uint32_t avail;
	uint32_t copy;
	uint32_t copied = 0;
	bool overtake_dst_buff = true;
#ifdef MT8518_AUDIO_SPI_UL_DMA_READ_LATENCY_DEBUG
	struct timespec64 ts1, ts2, ts_delta;
	s64 spi_rw_time_ns;
	s64 spi_rw_expect_time_ns;
#endif

#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	if (dai_mem->id == MT8518_ADSP_FE_VA_UPLOAD)
		overtake_dst_buff = false;
#endif

	dsp_spi_read(dsp_dmab->hw_offset_paddr,
		     &adsp_dma_hw_off,
		     sizeof(adsp_dma_hw_off), SPI_SPEED_HIGH);

	if (adsp_dma_hw_off >= adsp_dma_appl_off) {
		avail = adsp_dma_hw_off - adsp_dma_appl_off;
	} else {
		avail = adsp_dma_buf_size - adsp_dma_appl_off +
			adsp_dma_hw_off;
	}

#ifdef MT8518_AUDIO_SPI_UL_DMA_DEBUG
	dev_info(dai_mem->dev,
		 "%s [%d] adsp hw 0x%x appl 0x%x size 0x%x\n",
		 __func__, dai_mem->id, adsp_dma_hw_off,
		 adsp_dma_appl_off, adsp_dma_buf_size);

	dev_info(dai_mem->dev,
		 "%s [%d] host hw 0x%x appl 0x%x size 0x%x\n",
		 __func__, dai_mem->id, host_dma_hw_off,
		 frames_to_bytes(dai_mem->substream->runtime,
		 (dai_mem->substream->runtime->control->appl_ptr %
		 dai_mem->substream->runtime->buffer_size)),
		 host_dma_buf_size);
#endif

	if (avail < transfer) {
		dev_dbg(dai_mem->dev,
			"%s [%d] src not available (0x%x < 0x%x)\n",
			__func__, dai_mem->id, avail, transfer);
		return 0;
	}

	copy = transfer;

	if (!overtake_dst_buff) {
		struct snd_pcm_runtime *runtime = dai_mem->substream->runtime;
		ssize_t dst_avail = frames_to_bytes(runtime,
			snd_pcm_capture_hw_avail(runtime));

		if (dst_avail < copy) {
			dev_dbg(dai_mem->dev,
				"%s [%d] dst not available (0x%x < 0x%x)\n",
				__func__, dai_mem->id, avail, copy);
			return 0;
		}
	}

#ifdef MT8518_AUDIO_SPI_UL_COPY_LATENCY_DEBUG
	{
		struct timespec64 cur_ts, ts_delta;
		s64 copy_interval_time_ns;
		s64 copy_interval_expect_time_ns;

		ktime_get_ts64(&cur_ts);

		ts_delta = timespec64_sub(dai_mem->prev_copy_ts, cur_ts);

		copy_interval_time_ns = timespec64_to_ns(&ts_delta);

		copy_interval_expect_time_ns = dai_mem->period_time_ns;
		copy_interval_expect_time_ns += COPY_WORKER_OVERHEAD_NS;

		if (copy_interval_time_ns > copy_interval_expect_time_ns)
			dev_info(dai_mem->dev,
			"%s [%d] copy interval %lld ns expect:%lld ns\n",
			__func__, dai_mem->id,
			copy_interval_time_ns,
			copy_interval_expect_time_ns);

		dai_mem->prev_copy_ts = cur_ts;
	}
#endif

	while (copy > 0) {
		uint32_t from = 0;

		if (adsp_dma_hw_off >= adsp_dma_appl_off)
			from = adsp_dma_hw_off - adsp_dma_appl_off;
		else
			from = adsp_dma_buf_size - adsp_dma_appl_off;

		if (from > copy)
			from = copy;

		while (from > 0) {
			uint32_t to = 0;

			if (host_dma_hw_off + from <= host_dma_buf_size)
				to = from;
			else
				to = host_dma_buf_size - host_dma_hw_off;

#ifdef MT8518_AUDIO_SPI_UL_DMA_DEBUG
			dev_info(dai_mem->dev,
				 "%s [%d] copy %u bytes from 0x%x to %p\n",
				 __func__, dai_mem->id, to,
				 adsp_dma_buf_paddr + adsp_dma_appl_off,
				 host_dma_buf_vaddr + host_dma_hw_off);
#endif

#ifdef MT8518_AUDIO_SPI_UL_DMA_READ_LATENCY_DEBUG
			ktime_get_ts64(&ts1);
#endif

			dsp_spi_read_ex(adsp_dma_buf_paddr + adsp_dma_appl_off,
					host_dma_buf_vaddr + host_dma_hw_off,
					to, SPI_SPEED_HIGH);

#ifdef MT8518_AUDIO_SPI_UL_DMA_READ_LATENCY_DEBUG
			ktime_get_ts64(&ts2);

			ts_delta = timespec64_sub(ts2, ts1);
			spi_rw_time_ns = timespec64_to_ns(&ts_delta);

			spi_rw_expect_time_ns = NSEC_PER_SEC * to * 8 /
				SPI_RW_BITS_PER_SEC;
			spi_rw_expect_time_ns += SPI_RW_OVERHEAD_NS;

			if (spi_rw_time_ns > spi_rw_expect_time_ns)
				dev_info(dai_mem->dev,
				"%s [%d] take %lld ns to copy %u bytes\n",
				__func__, dai_mem->id, spi_rw_time_ns,
				to);
#endif

			from -= to;
			copy -= to;
			copied += to;

			host_dma_hw_off = (host_dma_hw_off + to) %
				host_dma_buf_size;
			adsp_dma_appl_off = (adsp_dma_appl_off + to) %
				adsp_dma_buf_size;
		}
	}

	dsp_spi_write(dsp_dmab->appl_offset_paddr,
		      &adsp_dma_appl_off,
		      sizeof(adsp_dma_appl_off), SPI_SPEED_HIGH);
	dsp_dmab->appl_offset = adsp_dma_appl_off;
	dsp_dmab->hw_offset = adsp_dma_hw_off;
	host_dmab->hw_offset = host_dma_hw_off;

	return copied;
}

static uint32_t upload_data_from_dsp_to_host(
	struct mt8518_adsp_dai_memory *dai_mem)
{
	struct mt8518_dsp_dma_buffer *dsp_dmab = &dai_mem->dsp_dmab;
	struct mt8518_host_dma_buffer *host_dmab = &dai_mem->host_dmab;
	struct mt8518_preroll_dma_buffer *preroll_dmab = &dai_mem->preroll_dmab;
	uint32_t adsp_dma_buf_paddr = dsp_dmab->buf_paddr;
	uint32_t adsp_dma_buf_size = dsp_dmab->buf_size;
	uint32_t adsp_dma_hw_off = 0;
	uint32_t adsp_dma_appl_off = dsp_dmab->appl_offset;
	unsigned char *host_dma_buf_vaddr = host_dmab->buf_vaddr;
	uint32_t host_dma_buf_size = host_dmab->buf_size;
	uint32_t host_dma_hw_off = host_dmab->hw_offset;
	uint32_t transfer = host_dmab->transfer;
	unsigned char *preroll_dma_buf_vaddr = preroll_dmab->buf_vaddr;
	uint32_t preroll_dma_buf_size = preroll_dmab->buf_size;
	uint32_t preroll_dma_hw_off = preroll_dmab->hw_offset;
	uint32_t preroll_dma_appl_off = preroll_dmab->appl_offset;
	uint32_t preroll_data_size = preroll_dmab->data_size;
	uint32_t preroll_remain_space = preroll_dma_buf_size -
					preroll_data_size;
	uint32_t avail;
	uint32_t copy;
	uint32_t copied = 0;
	bool overtake_dst_buff = false;
#ifdef MT8518_AUDIO_SPI_UL_DMA_READ_LATENCY_DEBUG
	struct timespec64 ts1, ts2, ts_delta;
	s64 spi_rw_time_ns;
	s64 spi_rw_expect_time_ns;
#endif

#ifdef MT8518_AUDIO_SPI_UL_COPY_LATENCY_DEBUG
	{
		struct timespec64 cur_ts, ts_delta;
		s64 copy_interval_time_ns;
		s64 copy_interval_expect_time_ns;

		ktime_get_ts64(&cur_ts);

		ts_delta = timespec64_sub(dai_mem->prev_copy_ts, cur_ts);

		copy_interval_time_ns = timespec64_to_ns(&ts_delta);

		copy_interval_expect_time_ns = dai_mem->period_time_ns;
		copy_interval_expect_time_ns += COPY_WORKER_OVERHEAD_NS;

		if (copy_interval_time_ns > copy_interval_expect_time_ns)
			dev_info(dai_mem->dev,
			"%s [%d] copy interval %lld ns expect:%lld ns\n",
			__func__, dai_mem->id,
			copy_interval_time_ns,
			copy_interval_expect_time_ns);

		dai_mem->prev_copy_ts = cur_ts;
	}
#endif

	if (preroll_data_size >= transfer)
		goto copy_from_preroll_to_host;

	dsp_spi_read(dsp_dmab->hw_offset_paddr,
		     &adsp_dma_hw_off,
		     sizeof(adsp_dma_hw_off),
		     SPI_SPEED_HIGH);

	if (adsp_dma_hw_off >= adsp_dma_appl_off) {
		avail = adsp_dma_hw_off - adsp_dma_appl_off;
	} else {
		avail = adsp_dma_buf_size - adsp_dma_appl_off +
			adsp_dma_hw_off;
	}

	if (avail < transfer) {
		dev_dbg(dai_mem->dev,
			"%s [%d] src not available (0x%x < 0x%x)\n",
			__func__, dai_mem->id, avail, transfer);
		return 0;
	}

	if (preroll_remain_space <= avail)
		copy = preroll_remain_space;
	else
		copy = (avail / transfer) * transfer;

	/* copy data from dsp to preroll */
	while (copy > 0) {
		uint32_t from = 0;

		if (adsp_dma_hw_off >= adsp_dma_appl_off)
			from = adsp_dma_hw_off - adsp_dma_appl_off;
		else
			from = adsp_dma_buf_size - adsp_dma_appl_off;

		if (from > copy)
			from = copy;

		while (from > 0) {
			uint32_t to = 0;

			to = preroll_remain_space;
			if (to > from)
				to = from;

#ifdef MT8518_AUDIO_SPI_UL_DMA_READ_LATENCY_DEBUG
			ktime_get_ts64(&ts1);
#endif

			dsp_spi_read_ex(adsp_dma_buf_paddr +
					adsp_dma_appl_off,
					preroll_dma_buf_vaddr +
					preroll_dma_hw_off,
					to, SPI_SPEED_HIGH);

#ifdef MT8518_AUDIO_SPI_UL_DMA_READ_LATENCY_DEBUG
			ktime_get_ts64(&ts2);

			ts_delta = timespec64_sub(ts2, ts1);
			spi_rw_time_ns = timespec64_to_ns(&ts_delta);

			spi_rw_expect_time_ns = NSEC_PER_SEC * to * 8 /
				SPI_RW_BITS_PER_SEC;
			spi_rw_expect_time_ns += SPI_RW_OVERHEAD_NS;

			if (spi_rw_time_ns > spi_rw_expect_time_ns)
				dev_info(dai_mem->dev,
				"%s [%d] take %lld ns to copy %u bytes\n",
				__func__, dai_mem->id, spi_rw_time_ns,
				to);
#endif

			from -= to;
			copy -= to;
			copied += to;

			preroll_dma_hw_off = (preroll_dma_hw_off + to) %
				preroll_dma_buf_size;
			preroll_data_size += to;
			preroll_remain_space = preroll_dma_buf_size -
				preroll_data_size;
			adsp_dma_appl_off = (adsp_dma_appl_off + to) %
				adsp_dma_buf_size;
		}
	}

	dsp_spi_write(dsp_dmab->appl_offset_paddr,
		      &adsp_dma_appl_off,
		      sizeof(adsp_dma_appl_off),
		      SPI_SPEED_HIGH);

	dsp_dmab->appl_offset = adsp_dma_appl_off;
	dsp_dmab->hw_offset = adsp_dma_hw_off;

	preroll_dmab->data_size = preroll_data_size;
	preroll_dmab->hw_offset = preroll_dma_hw_off;

	copied = 0;

copy_from_preroll_to_host:
	if (preroll_data_size < transfer) {
		dev_dbg(dai_mem->dev,
			"%s [%d] preroll not available (0x%x < 0x%x)\n",
			__func__, dai_mem->id,
			preroll_data_size,
			transfer);
		return 0;
	}

	copy = transfer;

	if (!overtake_dst_buff) {
		struct snd_pcm_runtime *runtime = dai_mem->substream->runtime;
		ssize_t dst_avail = frames_to_bytes(runtime,
			snd_pcm_capture_hw_avail(runtime));

		if (dst_avail < copy) {
			dev_dbg(dai_mem->dev,
				"%s [%d] dst not available (0x%x < 0x%x)\n",
				__func__, dai_mem->id,
				(unsigned int)dst_avail,
				copy);
			return 0;
		}
	}

	while (copy > 0) {
		uint32_t from = 0;

		from = preroll_data_size;
		if (from > copy)
			from = copy;

		while (from > 0) {
			uint32_t to = 0;

			if (host_dma_hw_off + from <= host_dma_buf_size)
				to = from;
			else
				to = host_dma_buf_size - host_dma_hw_off;

			memcpy(host_dma_buf_vaddr + host_dma_hw_off,
			       preroll_dma_buf_vaddr + preroll_dma_appl_off,
			       to);

			from -= to;
			copy -= to;
			copied += to;

			host_dma_hw_off = (host_dma_hw_off + to) %
				host_dma_buf_size;
			preroll_dma_appl_off = (preroll_dma_appl_off + to) %
				preroll_dma_buf_size;
			preroll_data_size -= to;
		}
	}

	preroll_dmab->data_size = preroll_data_size;
	preroll_dmab->appl_offset = preroll_dma_appl_off;
	host_dmab->hw_offset = host_dma_hw_off;

	return copied;
}

static uint32_t copy_data_from_host_to_dsp(
	struct mt8518_adsp_dai_memory *dai_mem,
	bool delay_host_dma_update)
{
#ifdef MT8518_AUDIO_SPI_DL_DMA_DEBUG
	struct snd_pcm_substream *substream = dai_mem->substream;
#endif
	struct mt8518_dsp_dma_buffer *dsp_dmab = &dai_mem->dsp_dmab;
	struct mt8518_host_dma_buffer *host_dmab = &dai_mem->host_dmab;
	uint32_t adsp_dma_buf_paddr = dsp_dmab->buf_paddr;
	uint32_t adsp_dma_buf_size = dsp_dmab->buf_size;
	uint32_t adsp_dma_hw_off = 0;
	uint32_t adsp_dma_appl_off = dsp_dmab->appl_offset;
	unsigned char *host_dma_buf_vaddr = host_dmab->buf_vaddr;
	uint32_t host_dma_buf_size = host_dmab->buf_size;
	uint32_t host_dma_hw_off = host_dmab->hw_offset;
	uint32_t transfer = host_dmab->transfer;
	uint32_t host_avail = atomic_read(&host_dmab->data_size);
	uint32_t avail;
	uint32_t copy;
	uint32_t copied = 0;
	int pending_elapse_size;
#ifdef MT8518_AUDIO_SPI_DL_DMA_WRITE_LATENCY_DEBUG
	struct timespec64 ts1, ts2, ts_delta;
	s64 spi_rw_time_ns;
	s64 spi_rw_expect_time_ns;
#endif

	dsp_spi_read(dsp_dmab->hw_offset_paddr,
		     &adsp_dma_hw_off,
		     sizeof(adsp_dma_hw_off), SPI_SPEED_HIGH);

	if (adsp_dma_hw_off > adsp_dma_appl_off) {
		avail = adsp_dma_hw_off - adsp_dma_appl_off;
	} else {
		avail = adsp_dma_buf_size - adsp_dma_appl_off +
			adsp_dma_hw_off;
	}

	pending_elapse_size = transfer *
		dma_peek_pending_hw_offset(dai_mem, &host_dma_hw_off);

#ifdef MT8518_AUDIO_SPI_DL_DMA_DEBUG
	dev_info(dai_mem->dev,
		 "%s [%d] adsp hw 0x%x appl 0x%x size 0x%x\n",
		 __func__, dai_mem->id, adsp_dma_hw_off, adsp_dma_appl_off,
		 adsp_dma_buf_size);

	dev_info(dai_mem->dev,
		 "%s [%d] host hw 0x%x appl 0x%x size 0x%x\n",
		 __func__, dai_mem->id, host_dma_hw_off,
		 frames_to_bytes(substream->runtime,
		 (substream->runtime->control->appl_ptr %
		 substream->runtime->buffer_size)),
		 host_dma_buf_size);

	dev_info(dai_mem->dev,
		 "%s [%d] transfer 0x%x elapse 0x%x host_avail 0x%x\n",
		 __func__, dai_mem->id, transfer,
		 pending_elapse_size, host_avail);
#endif

	if (avail < transfer) {
		dev_dbg(dai_mem->dev,
			"%s [%d] dst not available (0x%x < 0x%x)\n",
			__func__, dai_mem->id, avail, transfer);
		return 0;
	}

	if (pending_elapse_size > 0) {
		host_dma_hw_off += pending_elapse_size;
		host_dma_hw_off %= host_dma_buf_size;
	}

	if (avail > host_avail)
		avail = host_avail;

	copy = (avail / transfer) * transfer;

	if (copy == 0)
		return 0;

#ifdef MT8518_AUDIO_SPI_DL_COPY_LATENCY_DEBUG
	{
		struct timespec64 cur_ts, ts_delta;
		s64 copy_interval_time_ns;
		s64 copy_interval_expect_time_ns;

		ktime_get_ts64(&cur_ts);

		ts_delta = timespec64_sub(dai_mem->prev_copy_ts, cur_ts);

		copy_interval_time_ns = timespec64_to_ns(&ts_delta);

		copy_interval_expect_time_ns = dai_mem->period_time_ns;
		copy_interval_expect_time_ns += COPY_WORKER_OVERHEAD_NS;

		if (copy_interval_time_ns > copy_interval_expect_time_ns)
			dev_info(dai_mem->dev,
			"%s [%d] copy interval %lld ns expect:%lld ns\n",
			__func__, dai_mem->id,
			copy_interval_time_ns,
			copy_interval_expect_time_ns);

		dai_mem->prev_copy_ts = cur_ts;
	}
#endif

	while (copy > 0) {
		uint32_t from = 0;

		if (host_dma_hw_off + copy <= host_dma_buf_size)
			from = copy;
		else
			from = host_dma_buf_size - host_dma_hw_off;

		while (from > 0) {
			uint32_t to = 0;

			if (adsp_dma_appl_off + from <= adsp_dma_buf_size)
				to = from;
			else
				to = adsp_dma_buf_size - adsp_dma_appl_off;

#ifdef MT8518_AUDIO_SPI_DL_DMA_DEBUG
			dev_info(dai_mem->dev,
				 "%s [%d] copy %u bytes from %p to 0x%x\n",
				 __func__, dai_mem->id, to,
				 host_dma_buf_vaddr + host_dma_hw_off,
				 adsp_dma_buf_paddr + adsp_dma_appl_off);
#endif

#ifdef MT8518_AUDIO_SPI_DL_DMA_WRITE_LATENCY_DEBUG
			ktime_get_ts64(&ts1);
#endif

			dsp_spi_write_ex(adsp_dma_buf_paddr +
					 adsp_dma_appl_off,
					 host_dma_buf_vaddr +
					 host_dma_hw_off,
					 to, SPI_SPEED_HIGH);

#ifdef MT8518_AUDIO_SPI_DL_DMA_WRITE_LATENCY_DEBUG
			ktime_get_ts64(&ts2);

			ts_delta = timespec64_sub(ts2, ts1);
			spi_rw_time_ns = timespec64_to_ns(&ts_delta);

			spi_rw_expect_time_ns = NSEC_PER_SEC * to * 8 /
				SPI_RW_BITS_PER_SEC;
			spi_rw_expect_time_ns += SPI_RW_OVERHEAD_NS;

			if (spi_rw_time_ns > spi_rw_expect_time_ns)
				dev_info(dai_mem->dev,
				"%s [%d] take %lld ns to copy %u bytes\n",
				__func__, dai_mem->id, spi_rw_time_ns,
				to);
#endif

			from -= to;
			copy -= to;
			copied += to;

			host_dma_hw_off = (host_dma_hw_off + to) %
				host_dma_buf_size;
			adsp_dma_appl_off = (adsp_dma_appl_off + to) %
				adsp_dma_buf_size;
		}
	}

	dsp_spi_write(dsp_dmab->appl_offset_paddr,
		      &adsp_dma_appl_off,
		      sizeof(adsp_dma_appl_off), SPI_SPEED_HIGH);
	dsp_dmab->appl_offset = adsp_dma_appl_off;
	dsp_dmab->hw_offset = adsp_dma_hw_off;

	if (!delay_host_dma_update)
		host_dmab->hw_offset = host_dma_hw_off;

	return copied;
}

static void ul_dma_process(struct kthread_work *work)
{
	struct mt8518_adsp_dai_memory *dai_mem = container_of(work,
			struct mt8518_adsp_dai_memory, dma_work);
	struct snd_pcm_substream *substream = dai_mem->substream;
	uint32_t copied;
	bool do_upload = false;

	if (!substream || !snd_pcm_running(substream)) {
		dev_info(dai_mem->dev,
			 "%s [%d] skip process in non-running state\n",
			 __func__, dai_mem->id);
		return;
	}

#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	if ((dai_mem->id == MT8518_ADSP_FE_VA_UPLOAD) &&
	    (dai_mem->preroll_dmab.buf_vaddr != NULL))
		do_upload = true;
#endif

	if (do_upload)
		copied = upload_data_from_dsp_to_host(dai_mem);
	else
		copied = copy_data_from_dsp_to_host(dai_mem);

	if (copied > 0)
		snd_pcm_period_elapsed(dai_mem->substream);
}

static void dl_dma_process(struct kthread_work *work)
{
	struct mt8518_adsp_dai_memory *dai_mem = container_of(work,
		struct mt8518_adsp_dai_memory, dma_work);
	struct snd_pcm_substream *substream = dai_mem->substream;
	uint32_t copied;

	if (!substream) {
		dev_info(dai_mem->dev,
			 "%s [%d] invalid substream\n",
			 __func__, dai_mem->id);
		return;
	}

	if (!snd_pcm_running(substream) &&
	    substream->runtime->status->state != SNDRV_PCM_STATE_PREPARED) {
		dev_info(dai_mem->dev,
			 "%s [%d] skip process in non-running state %d\n",
			 __func__, dai_mem->id,
			 substream->runtime->status->state);
		return;
	}

	copied = copy_data_from_host_to_dsp(dai_mem, true);
	if (copied > 0) {
		atomic_sub(copied, &dai_mem->host_dmab.data_size);
		atomic_add(copied / dai_mem->host_dmab.transfer,
			&dai_mem->dma_pending_elapse);
		dai_mem->total_transfer_bytes += copied;
	}
}

static int mt8518_audio_spi_pcm_copy(struct snd_pcm_substream *substream,
				     int channel,
				     snd_pcm_uframes_t pos,
				     void __user *buf,
				     snd_pcm_uframes_t count)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;
	ssize_t copy_bytes = frames_to_bytes(runtime, count);

	if (mt8518_adsp_need_ul_dma_copy(id)) {
		char *hwbuf = runtime->dma_area +
			frames_to_bytes(runtime, pos);

		if (copy_to_user_fromio(buf, hwbuf, copy_bytes))
			return -EFAULT;
	} else if (mt8518_adsp_need_dl_dma_copy(id)) {
		char *hwbuf = runtime->dma_area +
			frames_to_bytes(runtime, pos);
		struct mt8518_adsp_dai_memory *dai_mem = &priv->dai_mem[id];

		if (copy_from_user_toio(hwbuf, buf, copy_bytes))
			return -EFAULT;

		if (copy_bytes > 0)
			atomic_add(copy_bytes, &dai_mem->host_dmab.data_size);

		if (dai_mem->dma_worker_task && !snd_pcm_running(substream)) {
#ifdef MT8518_AUDIO_SPI_DL_COPY_LATENCY_DEBUG
			s64 pre;

			ktime_get_ts64(&dai_mem->prev_copy_ts);

			pre = timespec64_to_ns(&dai_mem->prev_copy_ts);

			pre -= dai_mem->period_time_ns;

			dai_mem->prev_copy_ts = ns_to_timespec64(pre);
#endif
			kthread_queue_work(&dai_mem->dma_worker, &dai_mem->dma_work);
		}
	}

	return 0;
}

static snd_pcm_uframes_t mt8518_audio_spi_pcm_pointer(
	struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;
	struct mt8518_adsp_dai_memory *dai_mem = &priv->dai_mem[id];
	struct mt8518_host_dma_buffer *host_dmab = &dai_mem->host_dmab;
	uint32_t hw_offset = host_dmab->hw_offset;

	if (mt8518_adsp_need_dl_dma_copy(id) && snd_pcm_running(substream)) {
		struct timespec ts;
		struct timespec prev_ts;
		s64 diff;
		unsigned long flags;

		spin_lock_irqsave(&dai_mem->host_dma_lock, flags);

		prev_ts = dai_mem->prev_elapse_ts;
		hw_offset = host_dmab->hw_offset;

		spin_unlock_irqrestore(&dai_mem->host_dma_lock, flags);

		snd_pcm_gettime(runtime, &ts);

		diff = timespec_to_ns(&ts) - timespec_to_ns(&prev_ts);
		if (diff > 0 && diff < dai_mem->period_time_ns) {
			u64 tmp = diff * runtime->rate;

			do_div(tmp, NSEC_PER_SEC);

			hw_offset += samples_to_bytes(runtime, tmp);
			hw_offset %= host_dmab->buf_size;
		} else if (diff > dai_mem->period_time_ns) {
			u64 tmp = (u64)dai_mem->period_time_ns * runtime->rate;

			do_div(tmp, NSEC_PER_SEC);

			tmp--;

			hw_offset += samples_to_bytes(runtime, tmp);
			hw_offset %= host_dmab->buf_size;
		} else {
			hw_offset = dai_mem->prev_hw_offset;
		}
	}

	dai_mem->prev_hw_offset = hw_offset;

	return bytes_to_frames(runtime, hw_offset);
}

static int mt8518_audio_spi_pcm_ack(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(rtd->platform);
	int id = rtd->cpu_dai->id;

	if (mt8518_adsp_check_dma_copy_by_ack(id)) {
		struct mt8518_adsp_dai_memory *dai_mem = &priv->dai_mem[id];

		if (!dai_mem->dma_worker_task) {
			dev_info(rtd->dev, "%s invalid dma_worker_task\n", __func__);
			return 0;
		}

		if (snd_pcm_running(substream))
			kthread_queue_work(&dai_mem->dma_worker, &dai_mem->dma_work);
	}

	return 0;
}

const struct snd_pcm_ops mt8518_audio_spi_pcm_ops = {
	.ioctl = snd_pcm_lib_ioctl,
	.copy = mt8518_audio_spi_pcm_copy,
	.pointer = mt8518_audio_spi_pcm_pointer,
	.ack = mt8518_audio_spi_pcm_ack,
};

static void handle_dsp_ul_irq(struct ipi_msg_t *p_ipi_msg)
{
	struct dsp_irq_param param;
	struct mt8518_adsp_dai_memory *dai_mem;

	AUDIO_COPY_DSP_IRQ_PARAM(p_ipi_msg->payload, &param);

	dai_mem = (void *)(uintptr_t)param.host_handler;
	if (!dai_mem) {
		pr_info("%s invalid handle\n", __func__);
		return;
	}

	if (!dai_mem->dma_worker_task) {
		dev_info(dai_mem->dev, "%s invalid dma_wq\n", __func__);
		return;
	}

#ifdef MT8518_AUDIO_SPI_UL_IRQ_LATENCY_DEBUG
	{
		struct timespec64 cur_ts, ts_delta;
		s64 irq_interval_time_ns;
		s64 irq_interval_expect_time_ns;

		ktime_get_ts64(&cur_ts);

		ts_delta = timespec64_sub(dai_mem->prev_irq_ts, cur_ts);

		irq_interval_time_ns = timespec64_to_ns(&ts_delta);

		irq_interval_expect_time_ns = dai_mem->period_time_ns;
		irq_interval_expect_time_ns += IRQ_NOTIFY_OVERHEAD_NS;

		if (irq_interval_time_ns > irq_interval_expect_time_ns)
			dev_info(dai_mem->dev,
			"%s [%d] irq interval %lld ns expect:%lld ns\n",
			__func__, dai_mem->id,
			irq_interval_time_ns,
			irq_interval_expect_time_ns);

		dai_mem->prev_irq_ts = cur_ts;
	}
#endif

	kthread_queue_work(&dai_mem->dma_worker, &dai_mem->dma_work);
}

#ifdef CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK
static void handle_dsp_dl_irq(struct ipi_msg_t *p_ipi_msg)
{
	struct dsp_irq_param param;
	struct mt8518_adsp_dai_memory *dai_mem;

	AUDIO_COPY_DSP_IRQ_PARAM(p_ipi_msg->payload, &param);

	dai_mem = (void *)(uintptr_t)param.host_handler;
	if (!dai_mem) {
		pr_info("%s invalid handle\n", __func__);
		return;
	}

	if (!dai_mem->dma_wq) {
		dev_info(dai_mem->dev, "%s invalid dma_wq\n", __func__);
		return;
	}

#ifdef MT8518_AUDIO_SPI_DL_IRQ_LATENCY_DEBUG
	{
		struct timespec64 cur_ts, ts_delta;
		s64 irq_interval_time_ns;
		s64 irq_interval_expect_time_ns;

		ktime_get_ts64(&cur_ts);

		ts_delta = timespec64_sub(dai_mem->prev_irq_ts, cur_ts);

		irq_interval_time_ns = timespec64_to_ns(&ts_delta);

		irq_interval_expect_time_ns = dai_mem->period_time_ns;
		irq_interval_expect_time_ns += IRQ_NOTIFY_OVERHEAD_NS;

		if (irq_interval_time_ns > irq_interval_expect_time_ns)
			dev_info(dai_mem->dev,
			"%s [%d] irq interval %lld ns expect:%lld ns\n",
			__func__, dai_mem->id,
			irq_interval_time_ns,
			irq_interval_expect_time_ns);

		dai_mem->prev_irq_ts = cur_ts;
	}
#endif

	kthread_queue_work(&dai_mem->dma_worker, &dai_mem->dma_work);
}
#endif

static void handle_va_event(struct ipi_msg_t *p_ipi_msg)
{
	struct dsp_ipc_va_notify event;
	struct mt8518_adsp_dai_memory *dai_mem;
	struct snd_pcm_substream *substream;

	AUDIO_COPY_DSP_VA_NOTIFY(p_ipi_msg->payload, &event);

	dai_mem = (void *)(uintptr_t)event.host_handler;
	if (!dai_mem) {
		pr_info("%s invalid handle\n", __func__);
		return;
	}

	substream = dai_mem->substream;
	if (!substream) {
		dev_info(dai_mem->dev, "%s invalid substream\n", __func__);
		return;
	}

	dev_dbg(dai_mem->dev, "%s event %u\n", __func__, event.type);

	switch (event.type) {
	case VA_NOTIFY_WAKEWORD_PASS:
		if (substream->pcm)
			mt8518_snd_ctl_notify(substream->pcm->card,
				WWE_DETECT_STS_CTL_NAME,
				SNDRV_CTL_EVENT_MASK_INFO);
		break;
	case VA_NOTIFY_VAD_PASS:
		break;
	default:
		break;
	}
}

static void mt8518_adsp_generic_recv_msg(struct ipi_msg_t *p_ipi_msg)
{
	if (!p_ipi_msg)
		return;

	if (p_ipi_msg->task_scene == TASK_SCENE_AUDIO_CONTROLLER) {
		switch (p_ipi_msg->msg_id) {
		case MSG_TO_HOST_DSP_AUDIO_READY:
			g_priv->dsp_ready = true;
			wake_up(&g_priv->wait_dsp);
			break;
		case MSG_TO_HOST_DSP_DEBUG_IRQ:
			mt8518_adsp_handle_debug_dump_irq(p_ipi_msg);
			break;
		default:
			break;
		}
	} else if (p_ipi_msg->task_scene == TASK_SCENE_VA_HOSTLESS) {
		switch (p_ipi_msg->msg_id) {
		case MSG_TO_HOST_VA_NOTIFY:
			handle_va_event(p_ipi_msg);
			break;
		default:
			break;
		}
	}
}

static void mt8518_adsp_generic_ul_recv_msg(struct ipi_msg_t *p_ipi_msg)
{
	if (!p_ipi_msg)
		return;

	switch (p_ipi_msg->msg_id) {
	case MSG_TO_HOST_DSP_IRQUL:
		handle_dsp_ul_irq(p_ipi_msg);
		break;
	default:
		break;
	}
}

#ifdef CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK
static void mt8518_adsp_generic_dl_recv_msg(struct ipi_msg_t *p_ipi_msg)
{
	if (!p_ipi_msg)
		return;

	switch (p_ipi_msg->msg_id) {
	case MSG_TO_HOST_DSP_IRQDL:
		handle_dsp_dl_irq(p_ipi_msg);
		break;
	default:
		break;
	}
}
#endif

static const struct {
	unsigned int scene;
	unsigned int msg_id;
} init_tasks[] = {
	{TASK_SCENE_AUDIO_CONTROLLER, MSG_TO_DSP_CREATE_VA_T},
#ifdef CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK
	{TASK_SCENE_AUDIO_CONTROLLER, MSG_TO_DSP_CREATE_PCM_PLAYBACK_T},
#endif
#ifdef CONFIG_SND_SOC_COMPRESS
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 0
	{TASK_SCENE_AUDIO_CONTROLLER, MSG_TO_DSP_CREATE_CMP_OFFLOAD_T},
#endif
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 1
	{TASK_SCENE_AUDIO_CONTROLLER, MSG_TO_DSP_CREATE_CMP_OFFLOAD_2_T},
#endif
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 2
	{TASK_SCENE_AUDIO_CONTROLLER, MSG_TO_DSP_CREATE_CMP_OFFLOAD_3_T},
#endif
#endif
#ifdef CONFIG_SND_SOC_MT8570_ADSP_XAF_2ND_CAPTURE
	{TASK_SCENE_AUDIO_CONTROLLER, MSG_TO_DSP_CREATE_XAF_2ND_CAPTURE_T},
#endif
};

static const struct {
	unsigned int scene;
	recv_message_t recv;
} task_callbacks[] = {
	{TASK_SCENE_VA, mt8518_adsp_generic_ul_recv_msg},
	{TASK_SCENE_INTERLINK_HOST_TO_DSP, mt8518_adsp_generic_recv_msg},
#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	{TASK_SCENE_VA_HOSTLESS, mt8518_adsp_generic_recv_msg},
	{TASK_SCENE_VA_UPLOAD, mt8518_adsp_generic_ul_recv_msg},
#endif
#ifdef CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK
	{TASK_SCENE_PCM_PLAYBACK, mt8518_adsp_generic_dl_recv_msg},
#endif
#ifdef CONFIG_SND_SOC_COMPRESS
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 0
	{TASK_SCENE_COMPRESS_PLAYBACK1, mt8518_adsp_compr_ipi_recv_msg},
#endif
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 1
	{TASK_SCENE_COMPRESS_PLAYBACK2, mt8518_adsp_compr_ipi_recv_msg},
#endif
#if CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS > 2
	{TASK_SCENE_COMPRESS_PLAYBACK3, mt8518_adsp_compr_ipi_recv_msg},
#endif
#endif
#ifdef CONFIG_SND_SOC_MT8570_ADSP_XAF_2ND_CAPTURE
	{TASK_SCENE_XAF_2ND_CAPTURE, mt8518_adsp_generic_ul_recv_msg},
#endif
};

#if defined(CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK) || \
	defined(CONFIG_SND_SOC_COMPRESS)
static int mt8518_audio_spi_init_dsp_volume(struct mt8518_audio_spi_priv *priv)
{
	struct mt8518_adsp_volume *vol = &priv->adsp_vol;
	struct mt8518_adsp_control_data *data = &priv->ctrl_data;
	struct ipi_msg_t ipi_msg;
	unsigned int scene;
#ifdef CONFIG_SND_SOC_COMPRESS
	size_t i;
#endif

	memset(&ipi_msg, 0, sizeof(ipi_msg));

	scene = mt8518_adsp_get_scene_by_dai_id(
		MT8518_ADSP_BE_PRIMARY_PLAYBACK);

	mt8518_adsp_send_ipi_cmd(&ipi_msg,
		TASK_SCENE_AUDIO_CONTROLLER,
		AUDIO_IPI_LAYER_TO_DSP,
		AUDIO_IPI_MSG_ONLY,
		AUDIO_IPI_MSG_NEED_ACK,
		MSG_TO_DSP_DSP_GET_VOLUME,
		scene, 0, NULL);

	mt8518_adsp_verify_ack_vol_info(&ipi_msg, &vol->master);

	mt8518_adsp_dsp_to_host_vol(&vol->master, &data->master_vol);

#ifdef CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK
	memset(&ipi_msg, 0, sizeof(ipi_msg));

	scene = mt8518_adsp_get_scene_by_dai_id(MT8518_ADSP_FE_PCM_PLAYBACK1);

	mt8518_adsp_send_ipi_cmd(&ipi_msg,
			TASK_SCENE_AUDIO_CONTROLLER,
			AUDIO_IPI_LAYER_TO_DSP,
			AUDIO_IPI_MSG_ONLY,
			AUDIO_IPI_MSG_NEED_ACK,
			MSG_TO_DSP_HOST_GET_VOLUME,
			scene, 0, NULL);

	mt8518_adsp_verify_ack_vol_info(&ipi_msg, &vol->pcm[0]);

	mt8518_adsp_dsp_to_host_vol(&vol->pcm[0], &data->pcm_vol[0]);
#endif

#ifdef CONFIG_SND_SOC_COMPRESS
	for (i = 0; i < CONFIG_SND_SOC_COMPRESS_NR_PLAYBACK_STREAMS; i++) {
		memset(&ipi_msg, 0, sizeof(ipi_msg));

		scene =	mt8518_adsp_get_scene_by_dai_id(
				MT8518_ADSP_FE_COMPR_BASE + i);

		mt8518_adsp_send_ipi_cmd(&ipi_msg,
			TASK_SCENE_AUDIO_CONTROLLER,
			AUDIO_IPI_LAYER_TO_DSP,
			AUDIO_IPI_MSG_ONLY,
			AUDIO_IPI_MSG_NEED_ACK,
			MSG_TO_DSP_HOST_GET_VOLUME,
			scene,
			0, NULL);

		mt8518_adsp_verify_ack_vol_info(&ipi_msg, &vol->compress[i]);

		mt8518_adsp_dsp_to_host_vol(&vol->compress[i],
			&data->compress_vol[i]);
	}
#endif

	return 0;
}
#endif

static void load_hifi4dsp_callback(void *arg)
{
	struct mt8518_audio_spi_priv *priv = arg;
	size_t i;

	if (!hifi4dsp_run_status())
		dev_warn(priv->dev,
			 "%s hifi4dsp_run_status not done\n",
			 __func__);

	if (!mt8518_wait_dsp_ready(priv)) {
		dev_info(priv->dev, "%s dsp ready timeout\n", __func__);
		return;
	}

#ifndef CONFIG_AMAZON_DSP_FRAMEWORK
	for (i = 0; i < ARRAY_SIZE(init_tasks); i++) {
		mt8518_adsp_send_ipi_cmd(NULL,
			init_tasks[i].scene,
			AUDIO_IPI_LAYER_TO_DSP,
			AUDIO_IPI_MSG_ONLY,
			AUDIO_IPI_MSG_NEED_ACK,
			init_tasks[i].msg_id,
			0, 0, NULL);
	}

	for (i = 0; i < ARRAY_SIZE(task_callbacks); i++) {
		audio_task_register_callback(task_callbacks[i].scene,
			task_callbacks[i].recv,
			NULL);
	}

	mt8518_update_mem_infos(priv);
#endif

#if defined(CONFIG_SND_SOC_MT8518_ADSP_PCM_PLAYBACK) || \
	defined(CONFIG_SND_SOC_COMPRESS)
	mt8518_audio_spi_init_dsp_volume(priv);
#endif

	priv->dsp_init = true;

	wake_up(&priv->wait_dsp);
}

static int mt8518_audio_spi_pcm_probe(struct snd_soc_platform *platform)
{
	size_t i;
	int ret = 0;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(platform);
	struct mt8518_adsp_dai_memory *dai_mem;
	struct sched_param param = { .sched_priority =  MAX_RT_PRIO / 2 + 1 };

	ret = audio_task_init_ipi_dma(priv->reserved_memory_paddr,
				      priv->reserved_memory_vaddr,
				      priv->reserved_memory_size);
	if (ret) {
		dev_err(platform->dev,
			"%s audio_task_init_ipi_dma fail %d\n",
			__func__, ret);
		return ret;
	}

	ret = audio_task_register_callback(TASK_SCENE_AUDIO_CONTROLLER,
					   mt8518_adsp_generic_recv_msg,
					   NULL);
	if (ret) {
		dev_err(platform->dev,
			"%s register callback for audio controller fail %d\n",
			__func__, ret);
		return ret;
	}

	for (i = 0; i < MT8518_ADSP_FE_CNT; i++) {
		char name[16];

		dai_mem = &priv->dai_mem[i];

		spin_lock_init(&dai_mem->host_dma_lock);

		dai_mem->dev = priv->dev;

		if (mt8518_adsp_need_ul_dma_copy(i)) {
			kthread_init_work(&dai_mem->dma_work, ul_dma_process);
			snprintf(name, sizeof(name), "ul_wq_%zu", i);
		} else if (mt8518_adsp_need_dl_dma_copy(i)) {
			kthread_init_work(&dai_mem->dma_work, dl_dma_process);

			snprintf(name, sizeof(name), "dl_wq_%zu", i);

			hrtimer_init(&dai_mem->dma_elapse_hrt,
				CLOCK_MONOTONIC, HRTIMER_MODE_ABS);

			dai_mem->dma_elapse_hrt.function =
				dma_elapse_hrtimer_callback;
		} else {
			continue;
		}
		kthread_init_worker(&dai_mem->dma_worker);
		dai_mem->dma_worker_task = kthread_run(kthread_worker_fn,
							&dai_mem->dma_worker,
							name);
		if (IS_ERR(dai_mem->dma_worker_task)) {
			dev_err(platform->dev, "Could not create dma task\n");
			return PTR_ERR(dai_mem->dma_worker_task);
		}
		sched_setscheduler(dai_mem->dma_worker_task, SCHED_FIFO, &param);
	}

	mt8518_adsp_init_debug_dump(priv);

	return mt8518_adsp_add_controls(platform);
}

static int mt8518_audio_spi_pcm_remove(struct snd_soc_platform *platform)
{
	size_t i;
	struct mt8518_audio_spi_priv *priv =
		snd_soc_platform_get_drvdata(platform);
	struct mt8518_adsp_dai_memory *dai_mem;

	for (i = 0; i < MT8518_ADSP_FE_CNT; i++) {
		dai_mem = &priv->dai_mem[i];
		if (dai_mem->dma_worker_task) {
				kthread_flush_worker(&dai_mem->dma_worker);
				kthread_stop(dai_mem->dma_worker_task);
				dai_mem->dma_worker_task = NULL;
		}
	}

	mt8518_adsp_cleanup_debug_dump(priv);

	return 0;
}

static int mt8518_audio_spi_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_card *card = rtd->card->snd_card;
	struct snd_pcm *pcm = rtd->pcm;
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;

	substream = pcm->streams[SNDRV_PCM_STREAM_PLAYBACK].substream;
	if (substream) {
		buf = &substream->dma_buffer;
		buf->dev.type = SNDRV_DMA_TYPE_DEV;
		buf->dev.dev = card->dev;
		buf->private_data = NULL;
	}

	substream = pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream;
	if (substream) {
		buf = &substream->dma_buffer;
		buf->dev.type = SNDRV_DMA_TYPE_DEV;
		buf->dev.dev = card->dev;
		buf->private_data = NULL;
	}

	return 0;
}

static void mt8518_audio_spi_pcm_free(struct snd_pcm *pcm)
{
}

const struct snd_soc_platform_driver mt8518_audio_spi_pcm_platform = {
	.probe = mt8518_audio_spi_pcm_probe,
	.remove = mt8518_audio_spi_pcm_remove,
	.ops = &mt8518_audio_spi_pcm_ops,
#ifdef CONFIG_SND_SOC_COMPRESS
	.compr_ops = &mt8518_adsp_compr_ops,
#endif
	.pcm_new = mt8518_audio_spi_pcm_new,
	.pcm_free = mt8518_audio_spi_pcm_free,
};

static int mt8518_audio_spi_parse_of(struct mt8518_audio_spi_priv *priv,
				     struct device_node *np)
{
	struct device_node *node = NULL;
	struct resource res;
	unsigned int temp;
	unsigned int index;
	int ret = 0;

	if (!priv || !np)
		return -EINVAL;

	node = of_parse_phandle(np, "memory-region", 0);
	if (!node) {
		dev_err(priv->dev,
			"%s No memory-region specified\n",
			__func__);
		ret = -EINVAL;
		goto error;
	}

	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(priv->dev,
			"%s No memory address assigned to the region %d\n",
			__func__, ret);
		goto error;
	}

	priv->reserved_memory_vaddr = devm_ioremap_wc(priv->dev,
		res.start, resource_size(&res));
	if (!priv->reserved_memory_vaddr) {
		dev_err(priv->dev,
			"%s Unable to remap addr(0x%llx) size(0x%llx)\n",
			__func__, (unsigned long long)res.start,
			(unsigned long long)resource_size(&res));
		ret = -ENXIO;
		goto error;
	}

	priv->reserved_memory_paddr = res.start;
	priv->reserved_memory_size = resource_size(&res);

	ret = of_property_read_u32(np, "mediatek,spi-mic-clock-mode", &temp);
	if (ret == 0) {
		index = MT8518_ADSP_BE_MIC_RECORD - MT8518_ADSP_BE_START;
		priv->be_data[index].clock_mode = temp;
	}

	ret = of_property_read_u32(np, "mediatek,spi-mic-data-mode", &temp);
	if (ret == 0) {
		index = MT8518_ADSP_BE_MIC_RECORD - MT8518_ADSP_BE_START;
		priv->be_data[index].data_mode = temp;
	}

	ret = of_property_read_u32(np, "mediatek,prip-be-clock-mode", &temp);
	if (ret == 0) {
		index = MT8518_ADSP_BE_PRIMARY_PLAYBACK - MT8518_ADSP_BE_START;
		priv->be_data[index].clock_mode = temp;
	}

	ret = of_property_read_u32(np, "mediatek,prip-be-data-mode", &temp);
	if (ret == 0) {
		index = MT8518_ADSP_BE_PRIMARY_PLAYBACK - MT8518_ADSP_BE_START;
		priv->be_data[index].data_mode = temp;
	}

#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	ret = of_property_read_u32(np, "mediatek,va-ul-preroll-period-count",
		&temp);
	if (ret == 0)
		priv->va_ul_preroll_period_count = temp;
	else
		priv->va_ul_preroll_period_count = VA_UL_PREROLL_PERIOD_COUNT;
#endif

	ret = 0;

error:
	if (node)
		of_node_put(node);

	return ret;
}

int mt8570_adsp_load_start(struct mt8518_audio_spi_priv *priv) {
	int ret;

	ret = async_load_hifi4dsp_bin_and_run(load_hifi4dsp_callback, priv);
	if (ret) {
		dev_info(priv->dev,
			 "%s async_load_hifi4dsp_bin_and_run fail %d\n",
			 __func__, ret);
	}
    return ret;
}

#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
static int mt8570_adsp_pcm_wdt_event(struct notifier_block *this,
		unsigned long event, void *data)
{
	struct mt8518_audio_spi_priv *priv = g_priv;

	dev_dbg(priv->dev, "%s\n", __func__);

	mt8518_reset_dsp_ready(priv);
	mt8518_reset_dsp_init(priv);
	mt8570_adsp_load_start(priv);

	return NOTIFY_DONE;
}

static struct notifier_block mt8570_adsp_pcm_event_notifier = {
	.notifier_call  = mt8570_adsp_pcm_wdt_event,
};
#endif

static int mt8518_audio_spi_dev_probe(struct platform_device *pdev)
{
	int ret;
	struct mt8518_audio_spi_priv *priv;
	struct device *dev = &pdev->dev;

	if (strstr(saved_command_line, "farfield.dsp.name=mt8570") == NULL) {
		pr_info("mt8570 is not supported\n");
		return -EINVAL;
	}

	priv = devm_kzalloc(dev,
			    sizeof(struct mt8518_audio_spi_priv),
			    GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	init_waitqueue_head(&priv->wait_dsp);

	priv->dev = dev;

	platform_set_drvdata(pdev, priv);

	ret = mt8518_audio_spi_parse_of(priv, dev->of_node);
	if (ret)
		return ret;

	ret = snd_soc_register_platform(dev, &mt8518_audio_spi_pcm_platform);
	if (ret < 0) {
		dev_err(dev, "Failed to register platform\n");
		return ret;
	}

	ret = snd_soc_register_component(dev,
					 &mt8518_audio_spi_dai_comp_drv,
					 mt8518_audio_spi_dais,
					 ARRAY_SIZE(mt8518_audio_spi_dais));
	if (ret < 0) {
		dev_err(dev, "Failed to register component\n");
		goto err_platform;
	}

	g_priv = priv;

#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
	register_adsp_wdt_notifier(&mt8570_adsp_pcm_event_notifier);
#endif

	dev_info(dev, "%s initialized.\n", __func__);
	return 0;

err_platform:
	snd_soc_unregister_platform(dev);
	return ret;
}

static int mt8518_audio_spi_dev_remove(struct platform_device *pdev)
{
#ifdef CONFIG_MTK_HIFI4DSP_WDT_RECOVER_SUPPORT
	unregister_adsp_wdt_notifier(&mt8570_adsp_pcm_event_notifier);
#endif
	snd_soc_unregister_component(&pdev->dev);
	snd_soc_unregister_platform(&pdev->dev);
	return 0;
}

static const struct of_device_id mt8518_audio_spi_dt_match[] = {
	{ .compatible = "mediatek,mt8518-audio-spi", },
	{ }
};
MODULE_DEVICE_TABLE(of, mt8518_audio_spi_dt_match);

static struct platform_driver mt8518_audio_spi_driver = {
	.driver = {
		.name = "mt8518-audio-spi",
		.of_match_table = mt8518_audio_spi_dt_match,
	},
	.probe = mt8518_audio_spi_dev_probe,
	.remove = mt8518_audio_spi_dev_remove,
};
module_platform_driver(mt8518_audio_spi_driver);

MODULE_DESCRIPTION("MT8518 Audio SPI driver");
MODULE_AUTHOR("Hidalgo Huang <hidalgo.huang@mediatek.com>");
MODULE_LICENSE("GPL v2");
