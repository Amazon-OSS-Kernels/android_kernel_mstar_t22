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
#include <linux/device.h>
#include <sound/soc.h>

#include "drvVoc.h"
#include "voc_pcm.h"
#include "voc_debug.h"

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//  Variables
//------------------------------------------------------------------------------
static struct voc_pcm_dma_data voc_pcm_dma_wr =
{
    .name		= "DMA writer",
    .channel	= 0,
};

//------------------------------------------------------------------------------
//  Function
//------------------------------------------------------------------------------
#if 0
static int voc_soc_dai_ops_startup(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}

static void voc_soc_dai_ops_shutdown(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
}

static int voc_soc_dai_ops_trigger(struct snd_pcm_substream *substream, int cmd, struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}

static int voc_soc_dai_ops_prepare(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}

static int voc_soc_dai_ops_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}

static int voc_soc_dai_ops_hw_free(struct snd_pcm_substream *substream, struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}

static int voc_soc_dai_ops_set_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}

static int voc_soc_dai_ops_set_clkdiv(struct snd_soc_dai *dai, int div_id, int div)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}

static int voc_soc_dai_ops_set_sysclk(struct snd_soc_dai *dai, int clk_id, unsigned int freq, int dir)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}

static struct snd_soc_dai_ops voc_soc_cpu_dai_ops =
{
    .set_sysclk = voc_soc_dai_ops_set_sysclk,
    .set_pll    = NULL,
    .set_clkdiv = voc_soc_dai_ops_set_clkdiv,

    .set_fmt    = voc_soc_dai_ops_set_fmt,

    .startup		= voc_soc_dai_ops_startup,
    .shutdown		= voc_soc_dai_ops_shutdown,
    .trigger		= voc_soc_dai_ops_trigger,
    .prepare      = voc_soc_dai_ops_prepare,
    .hw_params	= voc_soc_dai_ops_hw_params,
    .hw_free      = voc_soc_dai_ops_hw_free,
};
#endif

static int voc_soc_dai_probe(struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    dai->capture_dma_data = (void *)&voc_pcm_dma_wr;
    return 0;
}

static int voc_soc_dai_remove(struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    return 0;
}


static int voc_soc_dai_suspend(struct snd_soc_dai *dai)
{
    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);

    /* stop CM4, reload CM4 binary, finally start CM4 */
   // VocUpdateCm4Fw();
   //VocDmaResetAudio();

    return 0;
}

static int voc_soc_dai_resume(struct snd_soc_dai *dai)
{
    struct snd_pcm_substream *substream;
    struct snd_pcm_runtime *runtime;
    struct voc_pcm_runtime_data *prtd;
    //struct voc_pcm_dma_data *dma_data;
    struct snd_soc_pcm_runtime *rtd;
    //struct snd_soc_codec *codec;
    struct snd_soc_card *card = dai->component->card;

    VOC_PRINTF(TRACE_LEVEL, "%s: dai = %s\n", __FUNCTION__, dai->name);
    VocDmaReset();
    VOC_PRINTF(DEBUG_LEVEL, "Dma reset\n");


    list_for_each_entry(rtd, &card->rtd_list, list) {

		if (rtd->dai_link->ignore_suspend)
			continue;

        /* transfer to VD mode */


        for (substream = rtd->pcm->streams[SNDRV_PCM_STREAM_CAPTURE].substream;
                substream; substream = substream->next)
        {
            /* FIXME: the open/close code should lock this as well */
            if (substream->runtime == NULL)
                continue;

            runtime = substream->runtime;
            prtd = runtime->private_data;

            if (prtd->dma_data)
            {
                //memset(runtime->dma_area, 0, runtime->dma_bytes);
                prtd->dma_level_count = 0;
                prtd->state = DMA_EMPTY; //EMPTY
            }
        }
    }

    return 0;
}

struct snd_soc_dai_driver voc_soc_cpu_dai_drv =
{
    .probe				= voc_soc_dai_probe,
    .remove             = voc_soc_dai_remove,
    .suspend			= voc_soc_dai_suspend,
    .resume				= voc_soc_dai_resume,

    .capture			=
    {
        .channels_min	= 2,
        .channels_max	= 8,
        .rates			= SNDRV_PCM_RATE_8000_48000,
        .formats		= SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S32_LE,
    },
//  .ops				= &voc_soc_cpu_dai_ops,
};

static const struct snd_soc_component_driver voc_soc_component =
{
    .name		= "voc-pcm",
};

static int voc_cpu_dai_probe(struct platform_device *pdev)
{
    int ret;

    VOC_PRINTF(TRACE_LEVEL, "%s enter\r\n", __FUNCTION__);

    ret = snd_soc_register_component(&pdev->dev, &voc_soc_component, &voc_soc_cpu_dai_drv, 1);
    if (ret)
    {
        return ret;
    }

    return 0;
}

static int voc_cpu_dai_remove(struct platform_device *pdev)
{
    VOC_PRINTF(TRACE_LEVEL, "%s enter\r\n", __FUNCTION__);
    snd_soc_unregister_component(&pdev->dev);
    return 0;
}


static struct platform_driver voc_cpu_dai_driver =
{
    .probe = voc_cpu_dai_probe,
    .remove = (voc_cpu_dai_remove),
    .driver = {
        .name = "voc-cpu-dai",
        .owner = THIS_MODULE,
    },
};


static struct platform_device *voc_cpu_dai_device = NULL;
static int __init voc_cpu_dai_init(void)
{

    int ret = 0;

    struct device_node *np;

    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__)

    voc_cpu_dai_device = platform_device_alloc("voc-cpu-dai", -1);
    if (!voc_cpu_dai_device)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_device_alloc voc-cpu-dai error\r\n", __FUNCTION__);
        return -ENOMEM;
    }
#if CONFIG_OF
    np = of_find_compatible_node(NULL, NULL, "mstar,voc-audio");
    if (np)
    {
        voc_cpu_dai_device->dev.of_node = of_node_get(np);
        of_node_put(np);
    }
#endif
    ret = platform_device_add(voc_cpu_dai_device);
    if (ret)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_device_add voc_cpu_dai_device error\r\n", __FUNCTION__);
        platform_device_put(voc_cpu_dai_device);
    }

    ret = platform_driver_register(&voc_cpu_dai_driver);
    if (ret)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_driver_register voc_cpu_dai_driver error\r\n", __FUNCTION__);
        platform_device_unregister(voc_cpu_dai_device);
    }

    return ret;
}

static void __exit voc_cpu_dai_exit(void)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    platform_device_unregister(voc_cpu_dai_device);
    platform_driver_unregister(&voc_cpu_dai_driver);
}

module_init(voc_cpu_dai_init);
module_exit(voc_cpu_dai_exit);


/* Module information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trevor Wu, trevor.wu@mstarsemi.com");
MODULE_DESCRIPTION("Voc Audio ALSA SoC Dai");
