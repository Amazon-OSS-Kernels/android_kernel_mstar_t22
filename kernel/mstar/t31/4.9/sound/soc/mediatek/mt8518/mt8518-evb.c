/*
 * mt8518-evb.c  --  MT8518 machine driver
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
#include <linux/of_gpio.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include "mt8518-afe-common.h"
#include "mt8518-snd-utils.h"

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
#include "pinctrl-mt8518-ipi.h"
#endif

#define TEST_BACKEND_WITH_ENDPOINT

#define PREFIX	"mediatek,"
#define ENUM_TO_STR(enum) #enum

enum PINCTRL_PIN_STATE {
	PIN_STATE_DEFAULT = 0,
	PIN_STATE_EXTAMP_ON,
	PIN_STATE_EXTAMP_OFF,
	PIN_STATE_MAX
};

static const char * const mt8518_evb_pin_str[PIN_STATE_MAX] = {
	"default",
	"extamp_on",
	"extamp_off",
};

enum {
	/* FE */
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	DAI_LINK_SPI_MIC_CAPTURE,
	DAI_LINK_VA_HOSTLESS,
	DAI_LINK_VA_UPLOAD,
	DAI_LINK_FE_SPI_XAF_2ND_CAPTURE,
#endif
	/* BE */
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	DAI_LINK_SPI_MIC_BE,
	DAI_LINK_BE_SPI_XAF_2ND_CAPTURE,
#endif
	DAI_LINK_NUM,
	DAI_LINK_BE_NUM = 1,
};

struct mt8518_evb_be_ctrl_data {
	unsigned int mck_multp;
	unsigned int lrck_width;
	unsigned int fix_rate;
	unsigned int fix_channels;
	unsigned int fix_bit_width;
};

#ifdef CONFIG_SND_SOC_GAPP_AUDIO_CONTROL
enum mtkfile_pcm_state {
	MTKFILE_PCM_STATE_UNKNOWN = 0,
	MTKFILE_PCM_STATE_OPEN,
	MTKFILE_PCM_STATE_HW_PARAMS,
	MTKFILE_PCM_STATE_PREPARE,
	MTKFILE_PCM_STATE_START,
	MTKFILE_PCM_STATE_PAUSE,
	MTKFILE_PCM_STATE_RESUME,
	MTKFILE_PCM_STATE_DRAIN,
	MTKFILE_PCM_STATE_STOP,
	MTKFILE_PCM_STATE_HW_FREE,
	MTKFILE_PCM_STATE_CLOSE,
	MTKFILE_PCM_STATE_NUM,
};

static const char *const pcm_state_func[] = {
	ENUM_TO_STR(MTKFILE_PCM_STATE_UNKNOWN),
	ENUM_TO_STR(MTKFILE_PCM_STATE_OPEN),
	ENUM_TO_STR(MTKFILE_PCM_STATE_HW_PARAMS),
	ENUM_TO_STR(MTKFILE_PCM_STATE_PREPARE),
	ENUM_TO_STR(MTKFILE_PCM_STATE_START),
	ENUM_TO_STR(MTKFILE_PCM_STATE_PAUSE),
	ENUM_TO_STR(MTKFILE_PCM_STATE_RESUME),
	ENUM_TO_STR(MTKFILE_PCM_STATE_DRAIN),
	ENUM_TO_STR(MTKFILE_PCM_STATE_STOP),
	ENUM_TO_STR(MTKFILE_PCM_STATE_HW_FREE),
	ENUM_TO_STR(MTKFILE_PCM_STATE_CLOSE),
};

static SOC_ENUM_SINGLE_EXT_DECL(pcm_state_enums, pcm_state_func);

enum {
	MASTER_VOLUME_ID = 0,
	MASTER_VOLUMEX_ID,
	MASTER_SWITCH_ID,
	MASTER_SWITCHX_ID,
	PCM_STATE_ID,
	PCM_STATEX_ID,
	CTRL_NOTIFY_NUM,
	CTRL_NOTIFY_INVAL = 0xFFFF,
};

static const char *nfy_ctl_names[CTRL_NOTIFY_NUM] = {
	"Master Volume 1",
	"Master Volume X",
	"Master Switch",
	"Master Switch X",
	"PCM State",
	"PCM State X",
};

struct soc_ctlx_res {
	int master_volume;
	int master_switch;
	int pcm_state;
	struct snd_ctl_elem_id nfy_ids[CTRL_NOTIFY_NUM];
	struct mutex res_mutex;
	spinlock_t res_lock;
};
#endif

struct mt8518_evb_priv {
	struct pinctrl *pinctrl;
	struct pinctrl_state *pin_states[PIN_STATE_MAX];
	struct mt8518_evb_be_ctrl_data be_data[DAI_LINK_BE_NUM];
	struct device_node *afe_plat_node;
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	struct device_node *spi_plat_node;
#endif
#ifdef CONFIG_SND_SOC_GAPP_AUDIO_CONTROL
	struct soc_ctlx_res ctlx_res;
#endif
};

#ifdef CONFIG_SND_SOC_GAPP_AUDIO_CONTROL
static inline int soc_ctlx_init(struct soc_ctlx_res *ctlx_res,
	struct snd_soc_card *soc_card)
{
	int i;
	struct snd_card *card = soc_card->snd_card;
	struct snd_kcontrol *control;

	ctlx_res->master_volume = 100;
	ctlx_res->master_switch = 1;
	ctlx_res->pcm_state = MTKFILE_PCM_STATE_UNKNOWN;

	mutex_init(&ctlx_res->res_mutex);
	spin_lock_init(&ctlx_res->res_lock);

	for (i = 0; i < CTRL_NOTIFY_NUM; i++) {
		list_for_each_entry(control, &card->controls, list) {
			if (strcmp(control->id.name, nfy_ctl_names[i]))
				continue;
			ctlx_res->nfy_ids[i] = control->id;
		}
	}

	return 0;
}

static int soc_ctlx_get(struct snd_kcontrol *kctl,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kctl);
	struct mt8518_evb_priv *card_data = snd_soc_card_get_drvdata(card);
	struct soc_ctlx_res *res_mgr = &card_data->ctlx_res;
	int type;

	for (type = 0; type < CTRL_NOTIFY_NUM; type++) {
		if (kctl->id.numid == res_mgr->nfy_ids[type].numid)
			break;
	}

	if (type == CTRL_NOTIFY_NUM) {
		pr_notice("%s invalid mixer control(numid:%d)\n",
			  __func__, kctl->id.numid);
		return -EINVAL;
	}

	mutex_lock(&res_mgr->res_mutex);

	switch (type) {
	case MASTER_VOLUME_ID:
	case MASTER_VOLUMEX_ID:
		ucontrol->value.integer.value[0] = res_mgr->master_volume;
		break;
	case MASTER_SWITCH_ID:
	case MASTER_SWITCHX_ID:
		ucontrol->value.integer.value[0] = res_mgr->master_switch;
		break;
	default:
		break;
	}

	mutex_unlock(&res_mgr->res_mutex);

	pr_debug("%s (%s) value is:%ld\n",
		 __func__, kctl->id.name, ucontrol->value.integer.value[0]);
	return 0;
}

static int soc_ctlx_put(struct snd_kcontrol *kctl,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kctl);
	struct mt8518_evb_priv *card_data = snd_soc_card_get_drvdata(card);
	struct soc_ctlx_res *res_mgr = &card_data->ctlx_res;
	int type;
	int nfy_type;
	int need_notify_self = 0;
	int *value = NULL;

	for (type = 0; type < CTRL_NOTIFY_NUM; type++) {
		if (kctl->id.numid == res_mgr->nfy_ids[type].numid)
			break;
	}

	if (type == CTRL_NOTIFY_NUM) {
		pr_notice("%s invalid mixer control(numid:%d)\n",
			  __func__, kctl->id.numid);
		return -EINVAL;
	}

	mutex_lock(&res_mgr->res_mutex);

	switch (type) {
	case MASTER_VOLUME_ID:
		if ((res_mgr->master_switch == 1) ||
			(ucontrol->value.integer.value[0] != 0)) {
			nfy_type = MASTER_VOLUMEX_ID;
			value = &res_mgr->master_volume;
			need_notify_self = 1;
		}
		break;
	case MASTER_VOLUMEX_ID:
		nfy_type = MASTER_VOLUME_ID;
		value = &res_mgr->master_volume;
		break;
	case MASTER_SWITCH_ID:
		nfy_type = MASTER_SWITCHX_ID;
		value = &res_mgr->master_switch;
		need_notify_self = 1;
		break;
	case MASTER_SWITCHX_ID:
		nfy_type = MASTER_SWITCH_ID;
		value = &res_mgr->master_switch;
		break;
	default:
		break;
	}

	if (value != NULL) {
		*value = ucontrol->value.integer.value[0];
		snd_ctl_notify(card->snd_card, SNDRV_CTL_EVENT_MASK_VALUE,
			       &(res_mgr->nfy_ids[nfy_type]));
	} else {
		nfy_type = CTRL_NOTIFY_INVAL;
	}

	if (need_notify_self) {
		snd_ctl_notify(card->snd_card, SNDRV_CTL_EVENT_MASK_VALUE,
			       &(kctl->id));
	}

	mutex_unlock(&res_mgr->res_mutex);

	pr_debug("%s (%s) value is:%ld, notify id:%x, notify self:%d\n",
		 __func__, kctl->id.name, ucontrol->value.integer.value[0],
		 nfy_type, need_notify_self);
	return 0;
}

static int soc_pcm_state_get(struct snd_kcontrol *kctl,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kctl);
	struct mt8518_evb_priv *card_data = snd_soc_card_get_drvdata(card);
	struct soc_ctlx_res *res_mgr = &card_data->ctlx_res;
	unsigned long flags;

	spin_lock_irqsave(&res_mgr->res_lock, flags);
	ucontrol->value.integer.value[0] = res_mgr->pcm_state;
	spin_unlock_irqrestore(&res_mgr->res_lock, flags);

	pr_debug("%s (%s) value is:%ld\n",
		 __func__, kctl->id.name,
		 ucontrol->value.integer.value[0]);
	return 0;
}

static int soc_pcm_state_put(struct snd_kcontrol *kctl,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_card *card = snd_kcontrol_chip(kctl);
	struct mt8518_evb_priv *card_data = snd_soc_card_get_drvdata(card);
	struct soc_ctlx_res *res_mgr = &card_data->ctlx_res;
	unsigned long flags;

	spin_lock_irqsave(&res_mgr->res_lock, flags);
	if (ucontrol->value.integer.value[0] != res_mgr->pcm_state) {
		res_mgr->pcm_state = ucontrol->value.integer.value[0];
		snd_ctl_notify(card->snd_card, SNDRV_CTL_EVENT_MASK_VALUE,
			       &(res_mgr->nfy_ids[PCM_STATEX_ID]));
	}
	spin_unlock_irqrestore(&res_mgr->res_lock, flags);

	pr_debug("%s (%s) value is:%ld\n",
		 __func__, kctl->id.name,
		 ucontrol->value.integer.value[0]);
	return 0;
}

static int dlm_playback_state_set(struct snd_pcm_substream *substream,
	int state)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;
	struct mt8518_evb_priv *card_data = snd_soc_card_get_drvdata(card);
	struct soc_ctlx_res *res_mgr = &card_data->ctlx_res;
	int nfy_type;
	unsigned long flags;

	nfy_type = PCM_STATEX_ID;

	spin_lock_irqsave(&res_mgr->res_lock, flags);
	if (res_mgr->pcm_state != state) {
		res_mgr->pcm_state = state;
		snd_ctl_notify(card->snd_card, SNDRV_CTL_EVENT_MASK_VALUE,
			       &(res_mgr->nfy_ids[nfy_type]));
	} else {
		nfy_type = CTRL_NOTIFY_INVAL;
	}
	spin_unlock_irqrestore(&res_mgr->res_lock, flags);

	return 0;
}

static int dlm_playback_startup(struct snd_pcm_substream *substream)
{
	dlm_playback_state_set(substream, MTKFILE_PCM_STATE_OPEN);
	return 0;
}

static void dlm_playback_shutdown(struct snd_pcm_substream *substream)
{
	dlm_playback_state_set(substream, MTKFILE_PCM_STATE_CLOSE);
}

static int dlm_playback_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	dlm_playback_state_set(substream, MTKFILE_PCM_STATE_HW_PARAMS);
	return 0;
}

static int dlm_playback_hw_free(struct snd_pcm_substream *substream)
{
	dlm_playback_state_set(substream, MTKFILE_PCM_STATE_HW_FREE);
	return 0;
}

static int dlm_playback_trigger(struct snd_pcm_substream *substream, int cmd)
{
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		dlm_playback_state_set(substream, MTKFILE_PCM_STATE_START);
		break;
	default:
		break;
	}
	return 0;
}

static struct snd_soc_ops dlm_playback_ops = {
	.startup = dlm_playback_startup,
	.shutdown = dlm_playback_shutdown,
	.hw_params = dlm_playback_hw_params,
	.hw_free = dlm_playback_hw_free,
	.trigger = dlm_playback_trigger,
};
#endif

struct mt8518_dai_link_prop {
	char *name;
	unsigned int link_id;
};
#if 0
static int mt8518_evb_ext_spk_amp_wevent(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_dapm_context *dapm = w->dapm;
	struct snd_soc_card *card = dapm->card;
	struct mt8518_evb_priv *card_data = snd_soc_card_get_drvdata(card);
	int ret = 0;

	dev_dbg(card->dev, "%s event %d\n", __func__, event);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		if (!IS_ERR(card_data->pin_states[PIN_STATE_EXTAMP_ON])) {
			ret = pinctrl_select_state(card_data->pinctrl,
				card_data->pin_states[PIN_STATE_EXTAMP_ON]);
			if (ret)
				dev_err(card->dev,
					"%s failed to select state %d\n",
					__func__, ret);
		} else {
			dev_info(card->dev,
				 "%s invalid pin state %s\n",
				 __func__,
				 mt8518_evb_pin_str[PIN_STATE_EXTAMP_ON]);
		}
		break;
	case SND_SOC_DAPM_PRE_PMD:
		if (!IS_ERR(card_data->pin_states[PIN_STATE_EXTAMP_OFF])) {
			ret = pinctrl_select_state(card_data->pinctrl,
				card_data->pin_states[PIN_STATE_EXTAMP_OFF]);
			if (ret)
				dev_err(card->dev,
					"%s failed to select state %d\n",
					__func__, ret);
		} else {
			dev_info(card->dev,
				 "%s invalid pin state %s\n",
				 __func__,
				 mt8518_evb_pin_str[PIN_STATE_EXTAMP_OFF]);
		}
		break;
	default:
		break;
	}

	return 0;
}
#endif
#if 0
static const struct snd_soc_dapm_widget mt8518_evb_widgets[] = {
	SND_SOC_DAPM_OUTPUT("HFP Out"),
	SND_SOC_DAPM_INPUT("HFP In"),
	SND_SOC_DAPM_INPUT("DMIC In"),
	SND_SOC_DAPM_SPK("Ext Spk Amp", mt8518_evb_ext_spk_amp_wevent),
#ifdef TEST_BACKEND_WITH_ENDPOINT
	SND_SOC_DAPM_OUTPUT("ETDM1 Out"),
	SND_SOC_DAPM_INPUT("ETDM1 In"),
	SND_SOC_DAPM_OUTPUT("ETDM2 Out"),
	SND_SOC_DAPM_INPUT("ETDM2 In"),
#endif
};

static const struct snd_soc_dapm_route mt8518_evb_routes[] = {
	{"HFP Out", NULL, "PCM1 Playback"},
	{"PCM1 Capture", NULL, "HFP In"},
#ifdef TEST_BACKEND_WITH_ENDPOINT
	{"ETDM1 Out", NULL, "ETDM1 Playback"},
	{"ETDM1 Capture", NULL, "ETDM1 In"},
	{"ETDM2 Out", NULL, "ETDM2 Playback"},
	{"ETDM2 Capture", NULL, "ETDM2 In"},
#endif
	{"DMIC Capture", NULL, "DMIC In"},

#ifdef CONFIG_SND_SOC_MT8518_CODEC
	{"DIG_DAC_CLK", NULL, "AFE_DAC_CLK"},
	{"DIG_ADC_CLK", NULL, "AFE_ADC_CLK"},
	{"Ext Spk Amp", NULL, "AU_LOL"},
#endif
};
#endif

static int mt8518_evb_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct mt8518_evb_priv *priv = snd_soc_card_get_drvdata(rtd->card);
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int id = rtd->dai_link->id;
	struct mt8518_evb_be_ctrl_data *be;
	unsigned int mclk_multiplier = 0;
	unsigned int mclk = 0;
	unsigned int lrck_width = 0;
	int slot = 0;
	int slot_width = 0;
	unsigned int slot_bitmask = 0;
	unsigned int idx;
	int ret;

	if (id < DAI_LINK_SPI_MIC_CAPTURE || id >= DAI_LINK_NUM)
		return -EINVAL;

	idx = id - DAI_LINK_SPI_MIC_CAPTURE;
	be = &priv->be_data[idx];

	mclk_multiplier = be->mck_multp;
	lrck_width = be->lrck_width;

	if (mclk_multiplier > 0) {
		mclk = mclk_multiplier * params_rate(params);

		ret = snd_soc_dai_set_sysclk(cpu_dai, 0, mclk,
					     SND_SOC_CLOCK_OUT);
		if (!ret)
			return ret;
	}

	slot_width = lrck_width;
	if (slot_width > 0) {
		slot = params_channels(params);
		slot_bitmask = GENMASK(slot - 1, 0);

		ret = snd_soc_dai_set_tdm_slot(cpu_dai,
					       slot_bitmask,
					       slot_bitmask,
					       slot,
					       slot_width);
		if (!ret)
			return ret;
	}

	return 0;
}

static int mt8518_evb_be_hw_params_fixup(struct snd_soc_pcm_runtime *rtd,
	struct snd_pcm_hw_params *params)
{
	struct mt8518_evb_priv *priv = snd_soc_card_get_drvdata(rtd->card);
	int id = rtd->dai_link->id;
	struct mt8518_evb_be_ctrl_data *be;
	unsigned int fix_rate = 0;
	unsigned int fix_bit_width = 0;
	unsigned int fix_channels = 0;
	unsigned int idx;

	if (id < DAI_LINK_SPI_MIC_CAPTURE || id >= DAI_LINK_NUM)
		return -EINVAL;

	idx = id - DAI_LINK_SPI_MIC_CAPTURE;
	be = &priv->be_data[idx];

	fix_rate = be->fix_rate;
	fix_bit_width = be->fix_bit_width;
	fix_channels = be->fix_channels;

	if (fix_rate > 0) {
		struct snd_interval *rate =
			hw_param_interval(params, SNDRV_PCM_HW_PARAM_RATE);

		rate->max = rate->min = fix_rate;
	}

	if (fix_bit_width > 0) {
		struct snd_mask *mask =
			hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);

		if (fix_bit_width == 32) {
			snd_mask_none(mask);
			snd_mask_set(mask, SNDRV_PCM_FORMAT_S32_LE);
		} else if (fix_bit_width == 16) {
			snd_mask_none(mask);
			snd_mask_set(mask, SNDRV_PCM_FORMAT_S16_LE);
		}
	}

	if (fix_channels > 0) {
		struct snd_interval *channels = hw_param_interval(params,
			SNDRV_PCM_HW_PARAM_CHANNELS);

		channels->min = channels->max = fix_channels;
	}

	return 0;
}

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
static struct snd_soc_ops mt8518_evb_spi_be_ops = {
	.hw_params = mt8518_evb_hw_params,
};
#endif

#define RSV_DAI_LNIK(x) \
{ \
	.name = #x "_FE", \
	.stream_name = #x, \
	.cpu_dai_name = "snd-soc-dummy-dai", \
	.codec_name = "snd-soc-dummy", \
	.codec_dai_name = "snd-soc-dummy-dai", \
	.platform_name = "snd-soc-dummy" \
}

/* Digital audio interface glue - connects codec <---> CPU */
static struct snd_soc_dai_link mt8518_evb_dais[] = {
	/* Front End DAI links */
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	[DAI_LINK_SPI_MIC_CAPTURE] = {
		.name = "SPI_MIC_FE",
		.stream_name = "SPI MIC Capture",
		.cpu_dai_name = "FE_MICR",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.id = DAI_LINK_SPI_MIC_CAPTURE,
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	[DAI_LINK_VA_HOSTLESS] = {
		.name = "VA_HL_FE",
		.stream_name = "VA Hostless FrontEnd",
		.cpu_dai_name = "FE_VA_HL",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.id = DAI_LINK_VA_HOSTLESS,
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
#else
	[DAI_LINK_VA_HOSTLESS] = RSV_DAI_LNIK(SPI_RSV0),
#endif

#ifdef CONFIG_SND_SOC_MT8518_ADSP_VOICE_ASSIST
	[DAI_LINK_VA_UPLOAD] = {
		.name = "VA_UL_FE",
		.stream_name = "VA Upload Capture",
		.cpu_dai_name = "FE_VA_UL",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.id = DAI_LINK_VA_UPLOAD,
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
#else
	[DAI_LINK_VA_UPLOAD] = RSV_DAI_LNIK(SPI_RSV1),
#endif
#ifdef CONFIG_SND_SOC_MT8570_ADSP_XAF_2ND_CAPTURE
	[DAI_LINK_FE_SPI_XAF_2ND_CAPTURE] = {
		.name = "SPI_XAF_2ND_CAPTURE_FE",
		.stream_name = "SPI XAF 2ND Capture",
		.cpu_dai_name = "FE_XAF_2NDR",
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.id = DAI_LINK_FE_SPI_XAF_2ND_CAPTURE,
		.trigger = {
			SND_SOC_DPCM_TRIGGER_POST,
			SND_SOC_DPCM_TRIGGER_POST
		},
		.dynamic = 1,
		.dpcm_capture = 1,
	},
#else
	[DAI_LINK_FE_SPI_XAF_2ND_CAPTURE] = RSV_DAI_LNIK(SPI_RSV2),
#endif
#endif
	/* Back End DAI links */
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	[DAI_LINK_SPI_MIC_BE] = {
		.name = "SPI MIC BE",
		.cpu_dai_name = "BE_MICR",
		.no_pcm = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.id = DAI_LINK_SPI_MIC_BE,
		.dai_fmt = SND_SOC_DAIFMT_I2S |
			   SND_SOC_DAIFMT_NB_NF |
			   SND_SOC_DAIFMT_CBS_CFS,
		.ops = &mt8518_evb_spi_be_ops,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
	[DAI_LINK_BE_SPI_XAF_2ND_CAPTURE] = {
		.name = "SPI XAF 2ND BE",
#ifdef CONFIG_SND_SOC_MT8570_ADSP_XAF_2ND_CAPTURE
		.cpu_dai_name = "BE_XAF_2NDR",
#else
		.cpu_dai_name = "snd-soc-dummy-dai",
#endif
		.no_pcm = 1,
		.codec_name = "snd-soc-dummy",
		.codec_dai_name = "snd-soc-dummy-dai",
		.id = DAI_LINK_BE_SPI_XAF_2ND_CAPTURE,
		.dai_fmt = SND_SOC_DAIFMT_I2S |
			   SND_SOC_DAIFMT_NB_NF |
			   SND_SOC_DAIFMT_CBS_CFS,
		.ops = &mt8518_evb_spi_be_ops,
		.dpcm_capture = 1,
		.ignore_suspend = 1,
	},
#endif
};

static const struct snd_kcontrol_new mt8518_evb_controls[] = {
#ifdef CONFIG_SND_SOC_GAPP_AUDIO_CONTROL
	SOC_SINGLE_EXT("Master Volume 1", 0, 0, 100, 0,
		       soc_ctlx_get, soc_ctlx_put),
	SOC_SINGLE_EXT("Master Volume X", 0, 0, 100, 0,
		       soc_ctlx_get, soc_ctlx_put),
	SOC_SINGLE_BOOL_EXT("Master Switch", 0,
			    soc_ctlx_get, soc_ctlx_put),
	SOC_SINGLE_BOOL_EXT("Master Switch X", 0,
			    soc_ctlx_get, soc_ctlx_put),
	SOC_ENUM_EXT("PCM State", pcm_state_enums,
		     soc_pcm_state_get, soc_pcm_state_put),
	SOC_ENUM_EXT("PCM State X", pcm_state_enums,
		     soc_pcm_state_get, 0),
#endif
};

#ifdef CONFIG_SND_SOC_MT8518
static int mt8518_evb_mt8570_gpio_probe(struct snd_soc_card *card,
	struct device_node *np)
{
	char prop[128];
	int ret = 0;

#if 0
	snprintf(prop, sizeof(prop), PREFIX"%s", "gpio_dsp_ctrl_pin");
	ret = of_property_read_u32(np, "mediatek,gpio_dsp_ctrl_pin", &gpio_dsp_ctrl_pin);
	if (ret) {
		dev_dbg(card->dev, "fail to get gpio_dsp_ctrl_pin\n");
		return ret;
	}
#endif
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	ret = gpio_send_ipi_msg(11, SET_MODE_IPI, GPIO_MODE_00);
	if (ret < 0)
		dev_dbg(card->dev, "fail to set gpio 11\n");
	ret = gpio_send_ipi_msg(12, SET_MODE_IPI, GPIO_MODE_00);
	if (ret < 0)
		dev_dbg(card->dev, "fail to set gpio 12\n");
#endif

	return ret;
}
#endif

static void mt8518_evb_parse_of_codec(struct device *dev,
	struct device_node *np,
	struct snd_soc_dai_link *dai_link,
	char *name)
{
	char prop[128];
	int ret;
	unsigned int i, num_codecs;
	struct device_node *codec_node;

	snprintf(prop, sizeof(prop), PREFIX"%s-audio-codec-num", name);
	ret = of_property_read_u32(np, prop, &num_codecs);
	if (ret)
		goto single_codec;

	if (num_codecs == 0)
		return;

	dai_link->codecs = devm_kzalloc(dev,
		num_codecs * sizeof(struct snd_soc_dai_link_component),
		GFP_KERNEL);

	dai_link->num_codecs = num_codecs;
	dai_link->codec_name = NULL;
	dai_link->codec_of_node = NULL;
	dai_link->codec_dai_name = NULL;

	for (i = 0; i < num_codecs; i++) {
		codec_node = NULL;

		// parse codec_of_node
		snprintf(prop, sizeof(prop),
			 PREFIX"%s-audio-codec%u",
			 name, i);
		codec_node = of_parse_phandle(np, prop, 0);
		if (codec_node)
			dai_link->codecs[i].of_node = codec_node;
		else {
			// parse codec name
			snprintf(prop, sizeof(prop),
				 PREFIX"%s-codec-name%u",
				 name, i);
			of_property_read_string(np, prop,
				&dai_link->codecs[i].name);
		}

		// parse codec dai name
		snprintf(prop, sizeof(prop),
			 PREFIX"%s-codec-dai-name%u",
			 name, i);
		of_property_read_string(np, prop,
			&dai_link->codecs[i].dai_name);
	}

	return;

single_codec:
	// parse codec_of_node
	snprintf(prop, sizeof(prop), PREFIX"%s-audio-codec", name);
	codec_node = of_parse_phandle(np, prop, 0);
	if (codec_node) {
		dai_link->codec_of_node = codec_node;
		dai_link->codec_name = NULL;
	}

	// parse codec dai name
	snprintf(prop, sizeof(prop), PREFIX"%s-codec-dai-name", name);
	of_property_read_string(np, prop, &dai_link->codec_dai_name);
}

static void mt8518_evb_parse_of(struct snd_soc_card *card,
				struct device_node *np)
{
	struct mt8518_evb_priv *priv = snd_soc_card_get_drvdata(card);
	size_t i;
	int ret;
	char prop[128];
	const char *str;
	unsigned int val;
	struct snd_soc_dai_link *dai_link;
	unsigned int link_id;

	static const struct mt8518_dai_link_prop of_dai_links_be[] = {
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
		{"spi-mic", DAI_LINK_SPI_MIC_BE},
#endif
	};

	snd_soc_of_parse_card_name(card, PREFIX"card-name");

	for (i = 0; i < ARRAY_SIZE(of_dai_links_be); i++) {
		struct mt8518_evb_be_ctrl_data *be;
		bool lrck_inverse = false;
		bool bck_inverse = false;
		bool hook_be_fixup_cb = false;

		link_id = of_dai_links_be[i].link_id;

		if (link_id < DAI_LINK_SPI_MIC_CAPTURE || link_id >= DAI_LINK_NUM)
			continue;

		dai_link = &mt8518_evb_dais[link_id];
		be = &priv->be_data[link_id - DAI_LINK_SPI_MIC_BE];

		// parse format
		snprintf(prop, sizeof(prop), PREFIX"%s-format",
			 of_dai_links_be[i].name);
		ret = of_property_read_string(np, prop, &str);
		if (ret == 0) {
			unsigned int format = 0;

			format = mt8518_snd_get_dai_format(str);

			dai_link->dai_fmt &= ~SND_SOC_DAIFMT_FORMAT_MASK;
			dai_link->dai_fmt |= format;
		}

		// parse clock mode
		snprintf(prop, sizeof(prop), PREFIX"%s-master-clock",
			 of_dai_links_be[i].name);
		ret = of_property_read_u32(np, prop, &val);
		if (ret == 0) {
			dai_link->dai_fmt &= ~SND_SOC_DAIFMT_MASTER_MASK;
			if (val)
				dai_link->dai_fmt |= SND_SOC_DAIFMT_CBS_CFS;
			else
				dai_link->dai_fmt |= SND_SOC_DAIFMT_CBM_CFM;
		}

		// parse lrck inverse
		snprintf(prop, sizeof(prop), PREFIX"%s-lrck-inverse",
			 of_dai_links_be[i].name);
		lrck_inverse = of_property_read_bool(np, prop);

		// parse bck inverse
		snprintf(prop, sizeof(prop), PREFIX"%s-bck-inverse",
			 of_dai_links_be[i].name);
		bck_inverse = of_property_read_bool(np, prop);

		dai_link->dai_fmt &= ~SND_SOC_DAIFMT_INV_MASK;

		if (lrck_inverse && bck_inverse)
			dai_link->dai_fmt |= SND_SOC_DAIFMT_IB_IF;
		else if (lrck_inverse && !bck_inverse)
			dai_link->dai_fmt |= SND_SOC_DAIFMT_IB_NF;
		else if (!lrck_inverse && bck_inverse)
			dai_link->dai_fmt |= SND_SOC_DAIFMT_NB_IF;
		else
			dai_link->dai_fmt |= SND_SOC_DAIFMT_NB_NF;

		// parse mclk multiplier
		snprintf(prop, sizeof(prop), PREFIX"%s-mclk-multiplier",
			 of_dai_links_be[i].name);
		ret = of_property_read_u32(np, prop, &val);
		if (ret == 0)
			be->mck_multp = val;

		// parse lrck width
		snprintf(prop, sizeof(prop), PREFIX"%s-lrck-width",
			 of_dai_links_be[i].name);
		ret = of_property_read_u32(np, prop, &val);
		if (ret == 0)
			be->lrck_width = val;

		if (hook_be_fixup_cb)
			dai_link->be_hw_params_fixup =
				mt8518_evb_be_hw_params_fixup;

		mt8518_evb_parse_of_codec(card->dev, np, dai_link,
			of_dai_links_be[i].name);

		// parse ignore pmdown time
		snprintf(prop, sizeof(prop), PREFIX"%s-ignore-pmdown-time",
			 of_dai_links_be[i].name);
		if (of_property_read_bool(np, prop))
			dai_link->ignore_pmdown_time = 1;

		// parse ignore suspend
		snprintf(prop, sizeof(prop), PREFIX"%s-ignore-suspend",
			 of_dai_links_be[i].name);
		if (of_property_read_bool(np, prop))
			dai_link->ignore_suspend = 1;
	}
}

static struct snd_soc_card mt8518_evb_card = {
	.name = "mt8518-evb-card",
	.owner = THIS_MODULE,
	.dai_link = mt8518_evb_dais,
	.num_links = ARRAY_SIZE(mt8518_evb_dais),
	.controls = mt8518_evb_controls,
	.num_controls = ARRAY_SIZE(mt8518_evb_controls),
#if 0
	.dapm_widgets = mt8518_evb_widgets,
	.num_dapm_widgets = ARRAY_SIZE(mt8518_evb_widgets),
	.dapm_routes = mt8518_evb_routes,
	.num_dapm_routes = ARRAY_SIZE(mt8518_evb_routes),
#endif
};

static void mt8518_evb_cleanup_of_resource(struct snd_soc_card *card)
{
	struct mt8518_evb_priv *priv = snd_soc_card_get_drvdata(card);
	struct snd_soc_dai_link *dai_link;
	int i, j;

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	of_node_put(priv->spi_plat_node);
#endif

	for (i = 0, dai_link = card->dai_link;
	     i < card->num_links; i++, dai_link++) {
		if (dai_link->num_codecs > 1) {
			struct snd_soc_dai_link_component *codec;

			for (j = 0, codec = dai_link->codecs;
			     j < dai_link->num_codecs; j++, codec++) {
				if (codec)
					of_node_put(codec->of_node);
			}
		} else if (dai_link->num_codecs == 1)
			of_node_put(dai_link->codec_of_node);
	}
}

static int mt8518_evb_dev_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &mt8518_evb_card;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	struct device_node *spi_plat_node;
#endif
	struct mt8518_evb_priv *priv;
	int ret, id;
	size_t i;
	size_t dais_num = ARRAY_SIZE(mt8518_evb_dais);

	if (strstr(saved_command_line, "farfield.dsp.name=mt8570") == NULL) {
		pr_info("mt8570 is not supported\n");
		return -EINVAL;
	}

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	spi_plat_node = of_parse_phandle(dev->of_node,
		"mediatek,spi-platform", 0);
	if (!spi_plat_node) {
		dev_info(dev, "Property 'spi-platform' missing or invalid\n");
		return -EINVAL;
	}
#endif

	for (i = 0; i < dais_num; i++) {
		if (mt8518_evb_dais[i].platform_name)
			continue;

		id = mt8518_evb_dais[i].id;

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
		if ((id >= DAI_LINK_SPI_MIC_CAPTURE) || (id <= DAI_LINK_SPI_MIC_BE)) {
			mt8518_evb_dais[i].platform_of_node = spi_plat_node;
		}
#endif
	}

	card->dev = dev;

	priv = devm_kzalloc(dev, sizeof(struct mt8518_evb_priv),
			    GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		dev_err(dev, "%s allocate card private data fail %d\n",
			__func__, ret);
		return ret;
	}

#ifdef CONFIG_MTK_HIFI4DSP_SUPPORT
	priv->spi_plat_node = spi_plat_node;
#endif

	snd_soc_card_set_drvdata(card, priv);

	mt8518_evb_parse_of(card, np);

	ret = devm_snd_soc_register_card(dev, card);
	if (ret) {
		dev_err(dev, "%s snd_soc_register_card fail %d\n",
			__func__, ret);
		return ret;
	}

#ifdef CONFIG_SND_SOC_GAPP_AUDIO_CONTROL
	soc_ctlx_init(&priv->ctlx_res, card);
#endif

	return ret;
}

static int mt8518_evb_dev_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	mt8518_evb_cleanup_of_resource(card);

	return 0;
}

static const struct of_device_id mt8518_evb_dt_match[] = {
	{ .compatible = "mediatek,mt8518-evb", },
	{ }
};
MODULE_DEVICE_TABLE(of, mt8518_evb_dt_match);

static struct platform_driver mt8518_evb_driver = {
	.driver = {
		   .name = "mt8518-evb",
		   .of_match_table = mt8518_evb_dt_match,
#ifdef CONFIG_PM
		   .pm = &snd_soc_pm_ops,
#endif
	},
	.probe = mt8518_evb_dev_probe,
	.remove = mt8518_evb_dev_remove,
};

module_platform_driver(mt8518_evb_driver);

/* Module information */
MODULE_DESCRIPTION("MT8518 EVB SoC machine driver");
MODULE_AUTHOR("Hidalgo Huang <hidalgo.huang@mediatek.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mt8518-evb");

