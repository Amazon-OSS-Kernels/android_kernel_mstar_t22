///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2017 MStar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/spinlock.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "voc_pcm.h"
#include "voc_debug.h"

#include "drvVoc.h"
#include "drvAec.h"
#include "drvMBX.h"
#include "halCPUINT.h"


//#ifdef CONFIG_OF
static struct platform_device *voc_dma_device = NULL;
//#endif


//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------

#define params_period_bytes(p) \
    (hw_param_interval_c((p), SNDRV_PCM_HW_PARAM_PERIOD_BYTES)->min)

//------------------------------------------------------------------------------
//  Variables
//------------------------------------------------------------------------------

static const struct snd_pcm_hardware voc_pcm_capture_hardware =
{
    .info				= SNDRV_PCM_INFO_INTERLEAVED,
    .formats			= SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S32_LE,
    .rates			= SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
    SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_48000,
    .rate_min			= 8000,
    .rate_max			= 48000,
    .channels_min		= 2,
    .channels_max		= 8,
    .buffer_bytes_max	= 512 * 1024,
    .period_bytes_min	= 1 * 1024,
    .period_bytes_max	= 40 * 1024,
    .periods_min		= 4,
    .periods_max		= 16,
};

#define E_FIQ_VOC_IRQ   E_FIQEXPL_ARM_TO_SECURE_R2 //FIQ40 //E_FIQEXPL_ARM_TO_SECURE_R2
static unsigned int VOC_IRQ_ID = E_FIQ_VOC_IRQ;

#define __NO_TASKLET__

//------------------------------------------------------------------------------
//  Function definition
//------------------------------------------------------------------------------
#ifndef __NO_TASKLET__
static void mbx_handler(unsigned long data);
DECLARE_TASKLET(mbx_task, mbx_handler, 0);
#endif


//------------------------------------------------------------------------------
//  Function
//------------------------------------------------------------------------------
#ifdef __NO_TASKLET__
static irqreturn_t  voc_pcm_dma_irq(int irq, void *dev_id)
{
    struct snd_pcm_substream *substream = (struct snd_pcm_substream *)dev_id;
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    struct voc_pcm_dma_data *dma_data = prtd->dma_data;
    MBX_Msg tMsg;
    unsigned long flag1,flag2;
    unsigned int state = 0;
    unsigned long u32PeriodCount,u32PeriodSize,u32nLevelCount;
    unsigned long u32HighTime,u32LowTime;
    unsigned long long u64TimeStamp;

    //VOC_PRINTF(IRQ_LEVEL, "%s\n", __FUNCTION__);
    memset(&tMsg, 0, sizeof(tMsg));
    tMsg.eRoleID = E_MBX_ROLE_CP;

    MHal_CPUINT_Clear(E_CPUINT_HK0, E_CPUINT_CP);
    spin_lock_irqsave(&prtd->lock, flag1);
    if (SNDRV_PCM_STREAM_CAPTURE == substream->stream)  // CAPTURE device
    {
        state = prtd->state;
        if(E_MBX_SUCCESS == MDrv_MBX_RecvMsgExt(&tMsg))
        {

            switch(tMsg.u8Index)
            {
                case E_MBX_MSG_PERIOD_NOTIFY:
                    if(!snd_pcm_running(substream))
                    {
                        spin_unlock_irqrestore(&prtd->lock, flag1);
                        return IRQ_HANDLED;
                    }

                    if(tMsg.u8ParameterCount == 8)
                    {
                        MBX_MSG_GET_2U32(tMsg, u32PeriodCount,u32PeriodSize);
                        u32nLevelCount = VocDmaUpdateLevelCnt(u32PeriodCount,u32PeriodSize);
                        VOC_PRINTF(IRQ_LEVEL, "level count %ld\n",u32nLevelCount);
                    }
                    else
                        VOC_PRINTF(IRQ_LEVEL, "[ERROR]E_MBX_MSG_PERIOD_NOTIFY pararameter count %d\n",tMsg.u8ParameterCount);
                    break;

                case E_MBX_MSG_STARTTIME_NOTIFY:
                    if(tMsg.u8ParameterCount == 8)
                    {
                        MBX_MSG_GET_2U32(tMsg, u32LowTime,u32HighTime);
                        u64TimeStamp = (unsigned long long)(u32HighTime)<<32 ;
                        u64TimeStamp += u32LowTime;

                        VocAecSetCapStartTime(u64TimeStamp);
                        VOC_PRINTF(IRQ_LEVEL, "1st time %lld\n",u64TimeStamp);
                    }
                    else
                        VOC_PRINTF(IRQ_LEVEL, "[ERROR]E_MBX_MSG_STARTTIME_NOTIFY pararameter count %d\n",tMsg.u8ParameterCount);
                    spin_unlock_irqrestore(&prtd->lock, flag1);
                    return IRQ_HANDLED;
                default:
                    VOC_PRINTF(ERROR_LEVEL, "unknown MBX, mbx = %d\n", tMsg.u8Index);
                    spin_unlock_irqrestore(&prtd->lock, flag1);
                    return IRQ_HANDLED;

            }
        }

        if(prtd->state!=DMA_FULL)
        {
            if(VocDmaIsXrun())
            {
                prtd->state = DMA_FULL;
                wmb();

                spin_unlock_irqrestore(&prtd->lock, flag1);

                snd_pcm_stream_lock_irqsave(substream, flag2);
                if (snd_pcm_running(substream))
                    snd_pcm_stop(substream, SNDRV_PCM_STATE_XRUN);
                snd_pcm_stream_unlock_irqrestore(substream, flag2);
                VOC_PRINTF(ERROR_LEVEL, "FULL: chanId = %ld, previous state = %d\n", dma_data->channel, state);
                VOC_PRINTF(IRQ_LEVEL, "FULL: chanId = %ld, previous state = %d\n", dma_data->channel, state);
                spin_lock_irqsave(&prtd->lock, flag1);
            }
            else if((prtd->state!=DMA_OVERRUN) && VocDmaIsDataReady())//(VocDmaGetLevelCnt()> frames_to_bytes(runtime, runtime->period_size)))
            {
                prtd->state = DMA_OVERRUN;

                spin_unlock_irqrestore(&prtd->lock, flag1);
                snd_pcm_period_elapsed(substream);

                VOC_PRINTF(IRQ_LEVEL, "OVER: chanId = %ld, previous state = %d, remainder = 0x%x\n",
                           dma_data->channel, state, (unsigned int)bytes_to_frames(runtime, VocDmaGetLevelCnt()));
                spin_lock_irqsave(&prtd->lock, flag1);
            }
        }
    }
    else
    {
        VOC_PRINTF(ERROR_LEVEL, "unknown IRQ, pcm running = %d\n", snd_pcm_running(substream));
    }
    spin_unlock_irqrestore(&prtd->lock, flag1);

    return IRQ_HANDLED;
}

#else
static irqreturn_t  voc_pcm_dma_irq(int irq, void *dev_id)
{
    struct snd_pcm_substream *substream = (struct snd_pcm_substream *)dev_id;
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    //unsigned long flag;

    //VOC_PRINTF(IRQ_LEVEL, "%s\n", __FUNCTION__);


    MHal_CPUINT_Clear(E_CPUINT_HK0, E_CPUINT_CP);
    spin_lock(&prtd->lock);
    mbx_task.data = (unsigned long)dev_id;
    tasklet_hi_schedule(&mbx_task);
    spin_unlock(&prtd->lock);

    return IRQ_HANDLED;
}


static void mbx_handler(unsigned long data)
{
    struct snd_pcm_substream *substream = (struct snd_pcm_substream *)data;
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    struct voc_pcm_dma_data *dma_data = prtd->dma_data;
    MBX_Msg tMsg;
    unsigned long flag1,flag2;
    unsigned int state = 0;
    unsigned long u32PeriodCount,u32PeriodSize,u32nLevelCount;
    unsigned long u32HighTime,u32LowTime;
    unsigned long long u64TimeStamp;

    //VOC_PRINTF(IRQ_LEVEL, "%s\n", __FUNCTION__);
    memset(&tMsg, 0, sizeof(tMsg));
    tMsg.eRoleID = E_MBX_ROLE_CP;

    if (SNDRV_PCM_STREAM_CAPTURE == substream->stream)  // CAPTURE device
    {
        state = prtd->state;
        if(E_MBX_SUCCESS == MDrv_MBX_RecvMsgExt(&tMsg))
        {

            switch(tMsg.u8Index)
            {
                case E_MBX_MSG_PERIOD_NOTIFY:
                    if(!snd_pcm_running(substream))
                    {
                        return;
                    }

                    if(tMsg.u8ParameterCount == 8)
                    {
                        MBX_MSG_GET_2U32(tMsg, u32PeriodCount,u32PeriodSize);
                        spin_lock_irqsave(&prtd->lock, flag1);
                        u32nLevelCount = VocDmaUpdateLevelCnt(u32PeriodCount,u32PeriodSize);
                        spin_unlock_irqrestore(&prtd->lock, flag1);
                        VOC_PRINTF(IRQ_LEVEL, "level count %ld\n",u32nLevelCount);
                    }
                    else
                        VOC_PRINTF(IRQ_LEVEL, "[ERROR]E_MBX_MSG_PERIOD_NOTIFY pararameter count %d\n",tMsg.u8ParameterCount);
                    break;

                case E_MBX_MSG_STARTTIME_NOTIFY:
                    if(tMsg.u8ParameterCount == 8)
                    {
                        MBX_MSG_GET_2U32(tMsg, u32LowTime,u32HighTime);
                        u64TimeStamp = (unsigned long long)(u32HighTime)<<32 ;
                        u64TimeStamp += u32LowTime;

                        VocAecSetCapStartTime(u64TimeStamp);

                        VOC_PRINTF(TRACE_LEVEL, "1st time %lld\n",u64TimeStamp);
                    }
                    else
                        VOC_PRINTF(IRQ_LEVEL, "[ERROR]E_MBX_MSG_STARTTIME_NOTIFY pararameter count %d\n",tMsg.u8ParameterCount);
                    return;
                default:
                    VOC_PRINTF(ERROR_LEVEL, "unknown MBX, mbx = %d\n", tMsg.u8Index);
                    return;

            }
        }

        spin_lock_irqsave(&prtd->lock, flag1);

        if(prtd->state!=DMA_FULL)
        {
            if(VocDmaIsXrun())
            {
                prtd->state = DMA_FULL;
                wmb();

                spin_unlock_irqrestore(&prtd->lock, flag1);

                snd_pcm_stream_lock_irqsave(substream, flag2);
                if (snd_pcm_running(substream))
                    snd_pcm_stop(substream, SNDRV_PCM_STATE_XRUN);
                snd_pcm_stream_unlock_irqrestore(substream, flag2);
                VOC_PRINTF(ERROR_LEVEL, "FULL: chanId = %ld, previous state = %d\n", dma_data->channel, state);
                VOC_PRINTF(IRQ_LEVEL, "FULL: chanId = %ld, previous state = %d\n", dma_data->channel, state);
                spin_lock_irqsave(&prtd->lock, flag1);
            }
            else if((prtd->state!=DMA_OVERRUN) && VocDmaIsDataReady())//(VocDmaGetLevelCnt()> frames_to_bytes(runtime, runtime->period_size)))
            {
                prtd->state = DMA_OVERRUN;
                wmb();
                spin_unlock_irqrestore(&prtd->lock, flag1);
                snd_pcm_period_elapsed(substream);

                VOC_PRINTF(IRQ_LEVEL, "OVER: chanId = %ld, previous state = %d, remainder = 0x%x\n",
                           dma_data->channel, state, (unsigned int)bytes_to_frames(runtime, VocDmaGetLevelCnt()));
                spin_lock_irqsave(&prtd->lock, flag1);
            }
        }
        spin_unlock_irqrestore(&prtd->lock, flag1);
    }
    else
    {
        VOC_PRINTF(ERROR_LEVEL, "unknown IRQ, pcm running = %d\n", snd_pcm_running(substream));
    }
}

#endif

static int voc_pcm_open(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai_link *dai_link = rtd->dai_link;
    struct snd_pcm *pcm = rtd->pcm;

    struct voc_pcm_runtime_data *prtd;
    int ret = 0;

    VOC_PRINTF(DEBUG_LEVEL, "%s: stream = %s, pcmC0D%d (substream = %s), dai_link = %s\n",
               __FUNCTION__, (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE"),
               pcm->device, substream->name, dai_link->name);

    if (SNDRV_PCM_STREAM_CAPTURE == substream->stream)
    {
        snd_soc_set_runtime_hwparams(substream, &voc_pcm_capture_hardware);
    }
    else
    {
        ret = -EINVAL;
        goto out;
    }
    /* Ensure that buffer size is a multiple of period size */
#if 0
    ret = snd_pcm_hw_constraint_integer(runtime,
                                        SNDRV_PCM_HW_PARAM_PERIODS);
    if (ret < 0)
        goto out;
#endif

    prtd = kzalloc(sizeof(*prtd), GFP_KERNEL);
    if (prtd == NULL)
    {
        ret = -ENOMEM;
        goto out;
    }

    spin_lock_init(&prtd->lock);
    runtime->private_data = prtd;

out:
    return ret;
}

static int voc_pcm_close(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;

    VOC_PRINTF(DEBUG_LEVEL, "%s: stream = %s\n", __FUNCTION__,
               (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE"));

    //VocDmaResetAudio();

    kfree(runtime->private_data); //free voc_pcm_runtime_data
    runtime->private_data = NULL;

    return 0;
}

extern unsigned int voc_soc_get_mic_bitwidth(void);
extern unsigned int voc_soc_get_mic_num(void);

/* this may get called several times by oss emulation */
static int voc_pcm_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *params)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
//    struct snd_soc_codec *codec = rtd->codec;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    struct voc_pcm_dma_data *dma_data;
    int err = 0;
    unsigned int frame_bits;

    VOC_PRINTF(TRACE_LEVEL, "%s: stream = %s\n", __FUNCTION__,
               (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE"));

    VOC_PRINTF(TRACE_LEVEL, "params_channels     = %d\n", params_channels(params));
    VOC_PRINTF(TRACE_LEVEL, "params_rate         = %d\n", params_rate(params));
    VOC_PRINTF(TRACE_LEVEL, "params_period_size  = %d\n", params_period_size(params));
    VOC_PRINTF(TRACE_LEVEL, "params_periods      = %d\n", params_periods(params));
    VOC_PRINTF(TRACE_LEVEL, "params_buffer_size  = %d\n", params_buffer_size(params));
    VOC_PRINTF(TRACE_LEVEL, "params_buffer_bytes = %d\n", params_buffer_bytes(params));
    VOC_PRINTF(TRACE_LEVEL, "params_sample_width = %d\n", snd_pcm_format_physical_width(params_format(params)));

    VOC_PRINTF(TRACE_LEVEL, "params_access = %d, params_format = %d, params_subformat = %d\n",
               params_access(params), params_format(params), params_subformat(params));

    frame_bits = snd_pcm_format_physical_width(params_format(params))/8*params_channels(params);


    dma_data = snd_soc_dai_get_dma_data(rtd->cpu_dai, substream);

    if (!dma_data)
        return 0;

    snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
    runtime->dma_bytes = params_buffer_bytes(params);
    if (prtd->dma_data)
        return 0;

    prtd->dma_data = dma_data;
    err = request_irq(VOC_IRQ_ID, //INT_MS_AUDIO_1,
                      voc_pcm_dma_irq,
                      SA_INTERRUPT | IRQF_ONESHOT,
                      // IRQF_SHARED,
                      "MBX_FIQ_R2toMIPS",
                      //(const char*)dma_data->name,
                      (void *)substream);
    if (!err)
    {
        /*
         * Link channel with itself so DMA doesn't need any
         * reprogramming while looping the buffer
         */
        VOC_PRINTF(TRACE_LEVEL, "dma name = %s, channel id = %lu\n", dma_data->name, dma_data->channel);
        VOC_PRINTF(TRACE_LEVEL, "dma buf phy addr = 0x%llx\n", (runtime->dma_addr - MIU_BASE));
        VOC_PRINTF(TRACE_LEVEL, "dma buf vir addr = %p\n", runtime->dma_area);
        VOC_PRINTF(TRACE_LEVEL, "dma buf size     = 0x%lx\n", runtime->dma_bytes);

        // Re-set up the underrun and overrun
        if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
        {
#if defined(__VOICE_AEC_SW_SYNC__)
            VocDmaInit(VocCapFramesToBytes(params_buffer_size(params)), VocCapFramesToBytes(params_period_size(params)));
#else
            VocDmaInit(params_buffer_bytes(params), params_period_bytes(params));
#endif

            VocDmaInitChannel((runtime->dma_addr - MIU_BASE),
                              runtime->dma_bytes,
                              params_channels(params),
                              snd_pcm_format_physical_width(params_format(params)),
                              params_rate(params));

#if defined(__VOICE_AEC_SW_SYNC__)
            if (!VocAecConfig(params_channels(params), params_rate(params), snd_pcm_format_physical_width(params_format(params))))
              return -EFAULT;
            //test
            //VocAecSetRefInfo((u32)runtime->dma_area, (u32)VocCapFramesToBytes(params_buffer_size(params)));
#else
            if(voc_soc_get_mic_bitwidth()>16)
            {
                if(snd_pcm_format_physical_width(params_format(params))!=32)
                {
                    VOC_PRINTF(ERROR_LEVEL, "bitwidth %d should be 32\n", snd_pcm_format_physical_width(params_format(params)));
                    return -EFAULT;
                }
            }
            else if(snd_pcm_format_physical_width(params_format(params))!=16)
            {
                VOC_PRINTF(ERROR_LEVEL, "bitwidth %d should be 16\n", snd_pcm_format_physical_width(params_format(params)));
                return -EFAULT;
            }
            if(voc_soc_get_mic_num()!=params_channels(params))
            {
                VOC_PRINTF(ERROR_LEVEL, "channel number should be %d\n", voc_soc_get_mic_num());
                return -EFAULT;
            }

            //VOC_PRINTF(TRACE_LEVEL, "%s: param OK!!\n", __FUNCTION__);
#endif
            /* enable sinegen for debug */
            //VocDmaEnableSinegen(true);
        }

        memset(runtime->dma_area, 0, runtime->dma_bytes);
    }
    return err;
}


static int voc_pcm_hw_free(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;


    VOC_PRINTF(TRACE_LEVEL, "%s: stream = %s\n", __FUNCTION__,
               (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE"));

    if (prtd->dma_data == NULL)
        return 0;

    free_irq(VOC_IRQ_ID, (void *)substream);

    prtd->dma_data = NULL;
    snd_pcm_set_runtime_buffer(substream, NULL);

    return 0;
}


static int voc_pcm_prepare(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    struct voc_pcm_dma_data *dma_data = prtd->dma_data;
    int ret = 0;
    VOC_PRINTF(TRACE_LEVEL, "%s: stream = %s, channel = %s\n", __FUNCTION__,
               (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE"), dma_data->name);

    return ret;
}

static int voc_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    struct voc_pcm_dma_data *dma_data = prtd->dma_data;
    unsigned long flags;
    int ret = 0;
//    struct timespec   tv;

    VOC_PRINTF(TRACE_LEVEL, "%s: stream = %s, channel = %s, cmd = %d\n", __FUNCTION__,
               (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE"), dma_data->name, cmd);

// AUD_PRINTF(TRACE_LEVEL, "!!BACH_DMA1_CTRL_0 = 0x%x,  BACH_DMA1_CTRL_8 = 0x%x, level count = %d\n",
// 		InfinityReadReg(BACH_REG_BANK1, BACH_DMA1_CTRL_0), InfinityReadReg(BACH_REG_BANK1, BACH_DMA1_CTRL_8), InfinityDmaGetLevelCnt(prtd->dma_data->channel));
    if(substream->stream != SNDRV_PCM_STREAM_CAPTURE)
        ret = -EINVAL;

    spin_lock_irqsave(&prtd->lock, flags);
    switch (cmd)
    {
        case SNDRV_PCM_TRIGGER_START:
        case SNDRV_PCM_TRIGGER_RESUME:
        case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
            //VocDmaReset();
            VocDmaStartChannel();

            if(cmd==SNDRV_PCM_TRIGGER_START)
            {
                //do_posix_clock_monotonic_gettime(&tv);
                //g_nCapStartTime = (unsigned long long)tv.tv_sec * 1000000LL + (unsigned long long) (tv.tv_nsec/1000LL);
            }
            break;

        case SNDRV_PCM_TRIGGER_STOP:
        case SNDRV_PCM_TRIGGER_SUSPEND:
        case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
            memset(runtime->dma_area, 0, runtime->dma_bytes);
            prtd->dma_level_count = 0;
            prtd->state = DMA_EMPTY;
            wmb();
            if(cmd==SNDRV_PCM_TRIGGER_STOP)
            {
                if(substream->stream == SNDRV_PCM_STREAM_CAPTURE)
                {
                    //g_nCapStartTime = 0;
                }
            }

            VocDmaStopChannel();
            VocDmaResetAudio();
            VocDmaReset();
            VocAecReset();
            break;
        default:
            ret = -EINVAL;
    }
    spin_unlock_irqrestore(&prtd->lock, flags);

    // AUD_PRINTF(TRACE_LEVEL, "!!Out BACH_DMA1_CTRL_0 = 0x%x,  BACH_DMA1_CTRL_8 = 0x%x, level count = %d\n",
    //		InfinityReadReg(BACH_REG_BANK1, BACH_DMA1_CTRL_0), InfinityReadReg(BACH_REG_BANK1, BACH_DMA1_CTRL_8), InfinityDmaGetLevelCnt(prtd->dma_data->channel));

    return ret;
}

#if defined(__VOICE_AEC_SW_SYNC__)
static snd_pcm_uframes_t voc_pcm_pointer(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    struct voc_pcm_dma_data *dma_data = prtd->dma_data;

    snd_pcm_uframes_t offset = 0;
    unsigned long flags;
    size_t last_dma_count_level = 0;

    if (SNDRV_PCM_STREAM_CAPTURE == substream->stream)
    {
        spin_lock_irqsave(&prtd->lock, flags);

        last_dma_count_level = VocDmaGetLevelCnt();
        //udelay(5000);

        /*
            if (last_dma_count_level <= 64)
              last_dma_count_level = 0;
            else
              last_dma_count_level = last_dma_count_level - 64;
        */

        //if (prtd->dma_level_count > runtime->dma_bytes * 2)
        //    prtd->dma_level_count -= runtime->dma_bytes;
        //offset = bytes_to_frames(runtime, ((prtd->dma_level_count + last_dma_count_level) % runtime->dma_bytes));
        if (prtd->dma_level_count > VocDmaBufferSize() * 2)
            prtd->dma_level_count -=  VocDmaBufferSize();
        offset = VocCapBytesToFrames((prtd->dma_level_count + last_dma_count_level) % VocDmaBufferSize());
        spin_unlock_irqrestore(&prtd->lock, flags);
    }

    VOC_PRINTF(DEBUG_LEVEL, "%s: stream id = %d, channel id = %lu, frame offset = 0x%x\n",
               __FUNCTION__, substream->stream, dma_data->channel, (unsigned int)offset);

    return offset;
}

static int voc_pcm_copy(struct snd_pcm_substream *substream,
                        int channel, snd_pcm_uframes_t pos,
                        void __user *dst, snd_pcm_uframes_t count)
{
    int i;
    unsigned char *aecbuf, *aecdst;

    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    // struct voc_pcm_dma_data *dma_data = prtd->dma_data;
    unsigned long flags;
    //unsigned char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
    unsigned char *hwbuf = runtime->dma_area + VocCapFramesToBytes(pos);
    unsigned long nCnt;

    volatile unsigned int state = prtd->state;
    wmb();

    if (SNDRV_PCM_STREAM_CAPTURE == substream->stream)
    {
        i = count;
        aecdst = (unsigned char *)dst;
        VocAecUpdateRefInfo();
        while (i > 0)
        {
            nCnt = VocAecCopy((unsigned short **)&aecbuf, (unsigned short *)hwbuf, i);
            if (copy_to_user(aecdst, aecbuf, frames_to_bytes(runtime, nCnt)))
            {
                VOC_PRINTF(ERROR_LEVEL, "%s: copy_to_user failed\n", __FUNCTION__);
                return -EFAULT;
            }
            i -= nCnt;
            aecdst += frames_to_bytes(runtime, nCnt);
            hwbuf += VocCapFramesToBytes(nCnt);
        }

        //if (copy_to_user(dst, hwbuf, frames_to_bytes(runtime, count)))
        //    return -EFAULT;

        spin_lock_irqsave(&prtd->lock, flags);

        if(prtd->state == DMA_EMPTY && state != prtd->state)
        {
            spin_unlock_irqrestore(&prtd->lock, flags);
            return -EPIPE;
        }
        //nCnt = VocDmaTrigLevelCnt( frames_to_bytes(runtime, count));
        nCnt = VocDmaTrigLevelCnt( VocCapFramesToBytes(count));
        /*if (prtd->state == DMA_FULL)
        {
            VocDmaClrStatus();
            VOC_PRINTF(DEBUG_LEVEL, "%s: FULL Clear\n", __FUNCTION__);
        }*/

        /*
            if ((prtd->dma_level_count%runtime->dma_bytes) != frames_to_bytes(runtime, pos))
              AUD_PRINTF(PCM_LEVEL, "%s: dma_level_count = %d, pos_bytes = %d\n",
                         __FUNCTION__, prtd->dma_level_count, frames_to_bytes(runtime, pos));
        */
        prtd->state = DMA_NORMAL;


        //prtd->dma_level_count += frames_to_bytes(runtime, count);
        prtd->dma_level_count += VocCapFramesToBytes(count);

        spin_unlock_irqrestore(&prtd->lock, flags);
    }

    //VOC_PRINTF(DEBUG_LEVEL, "frame_pos = 0x%x, frame_count = 0x%x,%d, framLevelCnt = %lu\n",
    //           (unsigned int)pos, (unsigned int)count,frames_to_bytes(runtime, count), VocDmaGetLevelCnt());

    return 0;
}
#else
static snd_pcm_uframes_t voc_pcm_pointer(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    struct voc_pcm_dma_data *dma_data = prtd->dma_data;

    snd_pcm_uframes_t offset = 0;
    unsigned long flags;
    size_t last_dma_count_level = 0;

    if (SNDRV_PCM_STREAM_CAPTURE == substream->stream)
    {
        spin_lock_irqsave(&prtd->lock, flags);

        last_dma_count_level = VocDmaGetLevelCnt();
        //udelay(5000);

        /*
            if (last_dma_count_level <= 64)
              last_dma_count_level = 0;
            else
              last_dma_count_level = last_dma_count_level - 64;
        */

        if (prtd->dma_level_count > runtime->dma_bytes * 2)
            prtd->dma_level_count -= runtime->dma_bytes;
        offset = bytes_to_frames(runtime, ((prtd->dma_level_count + last_dma_count_level) % runtime->dma_bytes));
        spin_unlock_irqrestore(&prtd->lock, flags);
    }

    VOC_PRINTF(DEBUG_LEVEL, "%s: stream id = %d, channel id = %lu, frame offset = 0x%x\n",
               __FUNCTION__, substream->stream, dma_data->channel, (unsigned int)offset);

    return offset;
}

static int voc_pcm_copy(struct snd_pcm_substream *substream,
                        int channel, snd_pcm_uframes_t pos,
                        void __user *dst, snd_pcm_uframes_t count)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct voc_pcm_runtime_data *prtd = runtime->private_data;
    // struct voc_pcm_dma_data *dma_data = prtd->dma_data;
    unsigned long flags;
    unsigned char *hwbuf = runtime->dma_area + frames_to_bytes(runtime, pos);
    unsigned long nCnt;

    volatile unsigned int state = prtd->state;
    wmb();

    if (SNDRV_PCM_STREAM_CAPTURE == substream->stream)
    {
        if (copy_to_user(dst, hwbuf, frames_to_bytes(runtime, count)))
            return -EFAULT;

        spin_lock_irqsave(&prtd->lock, flags);

        if(prtd->state == DMA_EMPTY && state != prtd->state)
        {
            spin_unlock_irqrestore(&prtd->lock, flags);
            return -EPIPE;
        }
        nCnt = VocDmaTrigLevelCnt( frames_to_bytes(runtime, count));
        /*if (prtd->state == DMA_FULL)
        {
            VocDmaClrStatus();
            VOC_PRINTF(DEBUG_LEVEL, "%s: FULL Clear\n", __FUNCTION__);
        }*/

        /*
            if ((prtd->dma_level_count%runtime->dma_bytes) != frames_to_bytes(runtime, pos))
              AUD_PRINTF(PCM_LEVEL, "%s: dma_level_count = %d, pos_bytes = %d\n",
                         __FUNCTION__, prtd->dma_level_count, frames_to_bytes(runtime, pos));
        */
        prtd->state = DMA_NORMAL;

        prtd->dma_level_count += frames_to_bytes(runtime, count);
        spin_unlock_irqrestore(&prtd->lock, flags);
    }

    //VOC_PRINTF(DEBUG_LEVEL, "frame_pos = 0x%x, frame_count = 0x%x,%d, framLevelCnt = %lu\n",
    //           (unsigned int)pos, (unsigned int)count,frames_to_bytes(runtime, count), VocDmaGetLevelCnt());

    return 0;
}
#endif


static struct snd_pcm_ops voc_pcm_ops =
{
    .open		= voc_pcm_open,
    .close		= voc_pcm_close,
    .ioctl		= snd_pcm_lib_ioctl,
    .hw_params		= voc_pcm_hw_params,
    .hw_free		= voc_pcm_hw_free,
    .prepare		= voc_pcm_prepare,
    .trigger		= voc_pcm_trigger,
    .pointer		= voc_pcm_pointer,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,53)
    .copy_user		= voc_pcm_copy,
#else
    .copy		= voc_pcm_copy,
#endif
};

static u64 voc_pcm_dmamask = DMA_BIT_MASK(32);

#if defined(__VOICE_MSYS_ALLOC__)
void* alloc_dmem(const char* name, unsigned int size, dma_addr_t *addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name,name,strlen(name)+1);
    dmem.length=size;
    if(0!=msys_request_dmem(&dmem))
    {
        return NULL;
    }
    *addr=dmem.phys;
    return (void *)((uintptr_t)dmem.kvirt);
}

void free_dmem(const char* name, unsigned int size, void *virt, dma_addr_t addr)
{
    MSYS_DMEM_INFO dmem;
    memcpy(dmem.name,name,strlen(name)+1);
    dmem.length=size;
    dmem.kvirt=(unsigned long long)((uintptr_t)virt);
    dmem.phys=(unsigned long long)((uintptr_t)addr);
    msys_release_dmem(&dmem);
}
#endif

static int voc_pcm_preallocate_dma_buffer(struct snd_pcm *pcm,
        int stream)
{
    struct snd_pcm_substream *substream = pcm->streams[stream].substream;
    struct snd_dma_buffer *buf = &substream->dma_buffer;
    size_t size = 0;

    VOC_PRINTF(TRACE_LEVEL, "%s: stream = %s\r\n", __FUNCTION__,
               (substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? "PLAYBACK" : "CAPTURE"));

    if (SNDRV_PCM_STREAM_CAPTURE == stream)  // CAPTURE device
    {
        size = voc_pcm_capture_hardware.buffer_bytes_max;
    }
    else
        return -EINVAL;

    buf->dev.type = SNDRV_DMA_TYPE_DEV;
    buf->dev.dev = pcm->card->dev;
    buf->private_data = NULL;

#if defined(__VOICE_MSYS_ALLOC__)
    {
      char name[16];
      snprintf(name, 16, "pcmC%dD%dc", pcm->card->number, pcm->device);
      buf->area = alloc_dmem(name, PAGE_ALIGN(size),&buf->addr);
    }
#else
    buf->area = dma_alloc_coherent(buf->dev.dev, PAGE_ALIGN(size), &buf->addr, GFP_KERNEL);
#endif
    if (!buf->area)
        return -ENOMEM;
    buf->bytes = PAGE_ALIGN(size);

    VOC_PRINTF(DEBUG_LEVEL, "dma buffer size 0x%lx\n", buf->bytes);
    VOC_PRINTF(DEBUG_LEVEL, "physical dma address 0x%llx\n", buf->addr);
    VOC_PRINTF(DEBUG_LEVEL, "virtual dma address,%p\n", buf->area);

    return 0;
}

static void voc_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
    struct snd_pcm_substream *substream;
    struct snd_dma_buffer *buf;
//    int stream;
    VOC_PRINTF(TRACE_LEVEL, "%s\n", __FUNCTION__);

    substream = pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream;
    if (!substream)
        return;

    buf = &substream->dma_buffer;
    if (!buf->area)
        return;

#if defined(__VOICE_MSYS_ALLOC__)
    {
      char name[16];
      snprintf(name, 16, "pcmC%dD%dc", pcm->card->number, pcm->device);
      free_dmem(name, buf->bytes, buf->area, buf->addr);
	}
#else
    dma_free_coherent(buf->dev.dev, buf->bytes,(void *)buf->area, buf->addr);
#endif
    buf->area = NULL;



}

static int voc_pcm_new(struct snd_soc_pcm_runtime *rtd)
{
    struct snd_card *card = rtd->card->snd_card;
    struct snd_pcm *pcm = rtd->pcm;
    int ret = 0;

    VOC_PRINTF(TRACE_LEVEL, "%s: snd_pcm device id = %d\r\n", __FUNCTION__, pcm->device);

    if (!card->dev->dma_mask)
        card->dev->dma_mask = &voc_pcm_dmamask;
    if (!card->dev->coherent_dma_mask)
        card->dev->coherent_dma_mask = DMA_BIT_MASK(32);


    if (pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream)
    {
#if 1
        ret = voc_pcm_preallocate_dma_buffer(pcm, SNDRV_PCM_STREAM_CAPTURE);
        if (ret)
            goto out;
#else
        ret = snd_pcm_lib_preallocate_pages(pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream, SNDRV_DMA_TYPE_CONTINUOUS, snd_dma_continuous_data(GFP_KERNEL), 64*1024, 64*1024);

        if(ret==0)
        {
            pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream->dma_buffer.addr = virt_to_phys(pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream->dma_buffer.area);

            VOC_PRINTF(TRACE_LEVEL, "%s: phy = 0x%llx, virt = 0x%p\r\n", __FUNCTION__, pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream->dma_buffer.addr, pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream->dma_buffer.area);
            VOC_PRINTF(TRACE_LEVEL, "%s: size = %ld\r\n", __FUNCTION__, virt_to_phys(pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream->dma_buffer.area),pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream->dma_buffer.bytes);

        }
        else
        {
            VOC_PRINTF(TRACE_LEVEL, "snd_pcm_lib_preallocate_pages failed!!\n");
            ret = 0;
        }
#endif
    }



out:
    /* free preallocated buffers in case of error */
    if (ret)
        voc_pcm_free_dma_buffers(pcm);

    return ret;
}

/*
static struct class * audio_class;
static char * audio_classname = "mstar_audio_class";
static struct device *audio_dev;
*/
static int voc_pcm_probe(struct snd_soc_platform *platform)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: platform = %s\n", __FUNCTION__, dev_name(platform->dev));

    MDrv_VOC_MBX_Init();
    VOC_PRINTF(DEBUG_LEVEL, "MBX init\n");
    return 0;
}

static int voc_pcm_remove(struct snd_soc_platform *platform)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: platform = %s\n", __FUNCTION__, dev_name(platform->dev));
    return 0;
}


static struct snd_soc_platform_driver voc_platform_driver =
{
    .probe = voc_pcm_probe,
    .remove = voc_pcm_remove,
    .ops		= &voc_pcm_ops,
    .pcm_new	= voc_pcm_new,
    .pcm_free	= voc_pcm_free_dma_buffers,
};

static int voc_platform_probe(struct platform_device *pdev)
{
   struct resource *res_irq;

    VOC_PRINTF(TRACE_LEVEL, "%s IRQ=0x%x\n", __FUNCTION__,VOC_IRQ_ID);

    VocCm4Init();

    res_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (res_irq)
    {
        VOC_IRQ_ID = res_irq->start;
        VOC_PRINTF(TRACE_LEVEL, "Platform get resource IORESOURCE_IRQ = 0x%x\n", VOC_IRQ_ID);
    }
    else
    {
#if CONFIG_OF
        VOC_IRQ_ID = of_irq_to_resource(pdev->dev.of_node, 0, NULL);
        if(!VOC_IRQ_ID)
        {
            VOC_IRQ_ID = E_FIQEXPL_SECURE_R2_TO_ARM;
        }

        VOC_PRINTF(TRACE_LEVEL, "of irq get resource IORESOURCE_IRQ = 0x%x\n", VOC_IRQ_ID);
#endif
    }

    VOC_PRINTF(DEBUG_LEVEL, "ASoC: platform register %s\n", dev_name(&pdev->dev));
    return snd_soc_register_platform(&pdev->dev, &voc_platform_driver);
}

static int voc_platform_remove(struct platform_device *pdev)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\n", __FUNCTION__);
    snd_soc_unregister_platform(&pdev->dev);
    return 0;
}
#if 0
static const struct of_device_id voc_audio_of_match[] =
{
    { .compatible = "mstar,voc-platform", },
    {},
};
MODULE_DEVICE_TABLE(of, voc_audio_of_match);

static struct platform_driver voc_dma_driver =
{
    .driver = {
        .name = "voc-platform",
        .owner = THIS_MODULE,
        .of_match_table = voc_audio_of_match,
    },

    .probe = voc_platform_probe,
    .remove = voc_platform_remove,
};


module_platform_driver(voc_dma_driver);
#else

static struct platform_driver voc_dma_driver =
{
    .driver = {
        .name = "voc-platform",
        .owner = THIS_MODULE,
    },

    .probe = voc_platform_probe,
    .remove = voc_platform_remove,
};

static int __init voc_dma_init(void)
{

    int ret = 0;

    struct device_node *np;

    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__)

    voc_dma_device = platform_device_alloc("voc-platform", -1);
    if (!voc_dma_device)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_device_alloc voc_dma_device error\r\n", __FUNCTION__);
        return -ENOMEM;
    }
#if CONFIG_OF
    np = of_find_compatible_node(NULL, NULL, "mstar,voc-audio");
    if(np)
    {
        voc_dma_device->dev.of_node = of_node_get(np);
        of_node_put(np);
    }
#endif
    ret = platform_device_add(voc_dma_device);
    if (ret)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_device_add voc_dma_device error\r\n", __FUNCTION__);
        platform_device_put(voc_dma_device);
    }

    ret = platform_driver_register(&voc_dma_driver);
    if (ret)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_driver_register voc_dma_driver error\r\n", __FUNCTION__);
        platform_device_unregister(voc_dma_device);
    }

    return ret;
}

static void __exit voc_dma_exit(void)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    platform_device_unregister(voc_dma_device);
    platform_driver_unregister(&voc_dma_driver);
}

module_init(voc_dma_init);
module_exit(voc_dma_exit);

#endif
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trevor Wu, trevor.wu@mstarsemi.com");
MODULE_DESCRIPTION("Voice PCM module");

