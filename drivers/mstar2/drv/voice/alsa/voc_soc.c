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
//  Include files
//------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/of.h>
#include <linux/kernel.h>
#include <sound/soc.h>
#ifdef __HWDEP__
#include <sound/asound.h>
#include <sound/hwdep.h>
#include <sound/soc-dapm.h>
#include "mdrv_voc_io.h"
#include "mdrv_voc_io_st.h"
#endif
#include "drvVoc.h"
#include "drvAec.h"
#include "voc_debug.h"



#define SOC_ENUM_VOC(xname, xenum) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = snd_soc_info_enum_double, \
	.get = voc_soc_card_get_enum, .put = voc_soc_card_put_enum, \
	.private_value = (unsigned long)&xenum }

enum
{
    VOC_REG_CM4_ONOFF = 0,
    VOC_REG_VQ_ONOFF,
    VOC_REG_SEAMLESS_ONOFF,
    VOC_REG_SIGEN_ONOFF,
    VOC_REG_DMIC_NUMBER,
    VOC_REG_DMIC_GAIN,
    VOC_REG_MIC_BITWIDTH,
    VOC_REG_AEC_MODE,
    VOC_REG_HW_AEC_ONOFF,
    VOC_REG_I2S_ONOFF,
    VOC_REG_DA_ONOFF,
    VOC_REG_HPF_ONOFF,
    VOC_REG_HPF_CONFIG,
    VOC_REG_UART_ONOFF,
    VOC_REG_LEN,
};


static unsigned int card_reg_backup[VOC_REG_LEN] =
{
    0x0,
    0x0,    //VOC_REG_VQ_ONOFF
    0x0,    //VOC_REG_SEAMLESS_ONOFF
    0x0,    //VOC_REG_SIGEN_ONOFF
    0x0,    //VOC_REG_DMIC_NUMBER
    0x0,    //VOC_REG_DMIC_GAIN
    0x0,    //VOC_REG_MIC_BITWIDTH
    0x0,   //VOC_REG_AEC_MODE
    0x0,
    0x0,   //VOC_REG_I2S_ONOFF
    0x0,   //VOC_REG_DA_ONOFF
    0x2,   //VOC_REG_HPF_ONOFF
    0x7,   //VOC_REG_HPF_CONFIG
    0x0,   //VOC_REG_UART_ONOFF
};

static unsigned int card_reg[VOC_REG_LEN] =
{
    0x0,
    0x0,    //VOC_REG_VQ_ONOFF
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,   //VOC_REG_AEC_MODE
    0x0,
    0x0,   //VOC_REG_I2S_ONOFF
    0x0,   //VOC_REG_DA_ONOFF
    0x2,   //VOC_REG_HPF_ONOFF
    0x7,   //VOC_REG_HPF_CONFIG
    0x0,   //VOC_REG_UART_ONOFF
};

static const char *vq_onoff_text[] = {"Off", "On"};
static const char *seamless_onoff_text[] = {"Off", "On"};
static const char *da_onoff_text[] = {"Off", "On"};
static const char *sigen_onoff_text[] = {"Off", "On"};
static const char *cm4_onoff_text[] = {"Off", "On"};

static const char *dmic_number_text[] = {"2", "4", "6", "8"};
static const char *mic_bitwidth_text[] = {"16", "24", "32"};
static const char *aec_mode_text[] = {"None", "48K Mono", "48k Stereo", "16K Mono", "16K Stereo"};
static const char *hw_aec_onoff_text[] = {"Off", "On"};
static const char *i2s_onoff_text[] = {"Off", "On"};
static const char *hpf_onoff_text[] = {"Off", "1-stage", "2-stage"};
static const char *hpf_config_text[] = {"-2", "-1", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
static const char *uart_onoff_text[] = {"Off", "On"};

static const struct soc_enum cm4_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_CM4_ONOFF, 0, 2, cm4_onoff_text);

static const struct soc_enum vq_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_VQ_ONOFF, 0, 2, vq_onoff_text);

static const struct soc_enum seamless_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_SEAMLESS_ONOFF, 0, 2, seamless_onoff_text);

static const struct soc_enum da_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_DA_ONOFF, 0, 2, da_onoff_text);

static const struct soc_enum hpf_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_HPF_ONOFF, 0, 3, hpf_onoff_text);

static const struct soc_enum hpf_config_enum =
    SOC_ENUM_SINGLE(VOC_REG_HPF_CONFIG, 0, 12, hpf_config_text);

static const struct soc_enum uart_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_UART_ONOFF, 0, 2, uart_onoff_text);

static const struct soc_enum sigen_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_SIGEN_ONOFF, 0, 2, sigen_onoff_text);

static const struct soc_enum dmic_number_enum =
    SOC_ENUM_SINGLE(VOC_REG_DMIC_NUMBER, 0, 4, dmic_number_text);

static const struct soc_enum mic_bitwidth_enum =
    SOC_ENUM_SINGLE(VOC_REG_MIC_BITWIDTH, 0, 3, mic_bitwidth_text);

static const struct soc_enum aec_mode_enum =
    SOC_ENUM_SINGLE(VOC_REG_AEC_MODE, 0, 5, aec_mode_text);

static const struct soc_enum hw_aec_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_HW_AEC_ONOFF, 0, 2, hw_aec_onoff_text);

static const struct soc_enum i2s_onoff_enum =
    SOC_ENUM_SINGLE(VOC_REG_I2S_ONOFF, 0, 2, i2s_onoff_text);

unsigned int voc_soc_card_read(struct snd_soc_card *card, unsigned int reg)
{
    //VOC_PRINTF(DEBUG_LEVEL, "%s: reg = 0x%x, val = 0x%x\n", __FUNCTION__, reg, card_reg[reg]);
    return card_reg[reg];
}
EXPORT_SYMBOL_GPL(voc_soc_card_read);

/* TO DO : AMIC */
unsigned int voc_soc_get_mic_num(void)
{
    switch(card_reg[VOC_REG_DMIC_NUMBER])
    {
        case 0://E_MBX_AUD_DMA_CHN2
            return 2;
        case 1://E_MBX_AUD_DMA_CHN4
            return 4;
        case 2://E_MBX_AUD_DMA_CHN6
            return 6;
        case 3://E_MBX_AUD_DMA_CHN8
            return 8;
        default:
            return 2;
    }

}
EXPORT_SYMBOL_GPL(voc_soc_get_mic_num);

unsigned int voc_soc_get_mic_bitwidth(void)
{
    switch(card_reg[VOC_REG_MIC_BITWIDTH])
    {
        case 0://E_MBX_AUD_BITWIDTH_16
            return 16;
        case 1://E_MBX_AUD_BITWIDTH_24
            return 24;
        case 2://E_MBX_AUD_BITWIDTH_32
            return 32;
        default:
            return 16;
    }

}
EXPORT_SYMBOL_GPL(voc_soc_get_mic_bitwidth);


int voc_soc_card_write(struct snd_soc_card *card, unsigned int reg, unsigned int value)
{

    //VQ_CONFIG_S tVqConf;

    VOC_PRINTF(DEBUG_LEVEL, "%s: reg = 0x%x, val = 0x%x\n", __FUNCTION__, reg, value);

    if (!(card_reg[reg] ^ value)) //no change
        return 0;

    if ((reg != VOC_REG_CM4_ONOFF) && !card_reg[VOC_REG_CM4_ONOFF])
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: CM4 not power on, reg = 0x%x, val = 0x%x\n",
                   __FUNCTION__, reg, value);
        return -1;
    }

    switch(reg)
    {
        case VOC_REG_CM4_ONOFF:
            if (value)
            {
                VocUpdateCm4Fw();
                //switch Uart disable
//                voc_soc_card_write(card,VOC_REG_UART_ONOFF,0);
            }
            else
            {
                //TODO:
            }
            break;
        case VOC_REG_VQ_ONOFF:
          /*
            VocEnableVp(value);
            tVqConf.nMode = 1; //loop
            VocConfigVq(tVqConf);
            VocEnableVq(value);
          */
            break;
        case VOC_REG_SEAMLESS_ONOFF:
            VOC_PRINTF(ERROR_LEVEL, "%s:  Seamless is not support\n",__FUNCTION__);
            break;
        case VOC_REG_DA_ONOFF:
            VocEnableDa(value);
            break;
        case VOC_REG_SIGEN_ONOFF:
            VocDmaEnableSinegen(value);
            break;
        case VOC_REG_DMIC_NUMBER:
            VocDmicNumber(value);
            VocAecSetCapMode(value);
            break;
        case VOC_REG_DMIC_GAIN:
            VocDmicGain(value);
            break;
        case VOC_REG_MIC_BITWIDTH:
            VocDmicBitwidth(value);
            break;
        case VOC_REG_AEC_MODE:
            VocAecSetRefMode(value);
            break;
        case VOC_REG_I2S_ONOFF:
            VocI2sEnable(value);
            break;
        case VOC_REG_HW_AEC_ONOFF:
            VocHwAecEnable(value);
         case VOC_REG_HPF_ONOFF:
            VocEnableHpf(value);
            break;
         case VOC_REG_HPF_CONFIG:
         {
            long tmp;
            if(value>=sizeof(hpf_config_text)/sizeof(hpf_config_text[0]))
            {
                VOC_PRINTF(ERROR_LEVEL, "%s:  val = 0x%x not support\n",__FUNCTION__,value );
                return -1;
            }
            if(!kstrtol(hpf_config_text[value],10,&tmp))
            {
              VocConfigHpf((int)tmp);
              //VOC_PRINTF(ERROR_LEVEL, "%s:  val = %ld\n",__FUNCTION__,tmp );
            }
            break;
         }
         case VOC_REG_UART_ONOFF:
            VocEnableUart(value);
            break;
        default:
            VOC_PRINTF(ERROR_LEVEL, "%s: error parameter, reg = 0x%x, val = 0x%x\n",
                       __FUNCTION__, reg, value);
            return -1;
    }

    card_reg[reg] = value;
    return 1;
}
EXPORT_SYMBOL_GPL(voc_soc_card_write);

static int voc_soc_card_get_enum(struct snd_kcontrol *kcontrol,
                                 struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    ucontrol->value.integer.value[0] = voc_soc_card_read(card, e->reg);
    return 0;
}

static int voc_soc_card_put_enum(struct snd_kcontrol *kcontrol,
                                 struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    return voc_soc_card_write(card, e->reg, ucontrol->value.integer.value[0]);
}

static int voc_soc_card_get_volsw(struct snd_kcontrol *kcontrol,
                                  struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
    struct soc_mixer_control *mc =
        (struct soc_mixer_control *)kcontrol->private_value;
    ucontrol->value.integer.value[0] = voc_soc_card_read(card, mc->reg);
    return 0;
}

static int voc_soc_card_put_volsw(struct snd_kcontrol *kcontrol,
                                  struct snd_ctl_elem_value *ucontrol)
{
    struct snd_soc_card *card = snd_kcontrol_chip(kcontrol);
    struct soc_mixer_control *mc =
        (struct soc_mixer_control *)kcontrol->private_value;
    return voc_soc_card_write(card, mc->reg, ucontrol->value.integer.value[0]);
}


#ifdef __HWDEP__
static int voc_hwdep_open(struct snd_hwdep * hw, struct file *file)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    return 0;
}

static int voc_hwdep_ioctl(struct snd_hwdep * hw, struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)(arg);
    int nArg;
    char n8Arg;
    int err = 0;
    VQ_CONFIG_S sVqConfig;
    VP_CONFIG_S sVpConfig;

    if (_IOC_TYPE(cmd) != VOICE_IOCTL_MAGIC)
        return -ENOTTY;
    if (_IOC_NR(cmd) > VOICE_IOCTL_MAXNR)
        return -ENOTTY;

    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (err)
    {
        return -EFAULT;
    }

    switch (cmd)
    {
        case MDRV_VOICE_IOCTL_LOAD_IMAGE:
            VocUpdateCm4Fw();
            break;

        case MDRV_VOICE_IOCTL_VQ_ENABLE:
            if (get_user(nArg, (int __user *)argp))
                return -EFAULT;
            VOC_PRINTF(TRACE_LEVEL, "MDRV_VOICE_IOCTL_VQ_ENABLE %d\r\n",nArg);
            VocEnableVq(nArg);
            break;

        case MDRV_VOICE_IOCTL_VQ_CONFIG:
            if (copy_from_user(&sVqConfig, argp, sizeof(VQ_CONFIG_S)))
                return -EFAULT;
            VOC_PRINTF(TRACE_LEVEL, "MDRV_VOICE_IOCTL_VQ_CONFIG mode:%d\r\n",sVqConfig.nMode);
            VocConfigVq(sVqConfig);
            break;

        case MDRV_VOICE_IOCTL_VP_ENABLE:
            if (get_user(nArg, (int __user *)argp))
                return -EFAULT;
            VOC_PRINTF(TRACE_LEVEL, "MDRV_VOICE_IOCTL_VP_ENABLE %d\r\n",nArg);
            VocEnableVp(nArg);
            break;

        case MDRV_VOICE_IOCTL_VP_CONFIG:
            if (copy_from_user(&sVpConfig, argp, sizeof(VP_CONFIG_S)))
            //if (get_user(sVpConfig, (VP_CONFIG_S __user *)argp))
                return -EFAULT;
            VOC_PRINTF(TRACE_LEVEL, "MDRV_VOICE_IOCTL_VP_CONFIG scale:%d\r\n",sVpConfig.nScale);
            VocConfigVp(sVpConfig);
            break;

        case MDRV_VOICE_IOCTL_SLEEP_CM4:
            if (get_user(nArg, (int __user *)argp))
                return -EFAULT;
            VOC_PRINTF(TRACE_LEVEL, "MDRV_VOICE_IOCTL_SLEEP_CM4 %d\r\n",nArg);
            VocSleepCm4(nArg);
            break;

        case MDRV_VOICE_IOCTL_KEYWORD_MATCH:
            /*nArg = VocDmaGetFlag();
            VOC_PRINTF(TRACE_LEVEL, "MDRV_VOICE_IOCTL_KEYWORD_MATCH %d\r\n",nArg);
            if (put_user(nArg, (int __user *)argp))
                return -EFAULT;
*/
            break;
        case MDRV_VOICE_IOCTL_SINEGEN_ENABLE:
            if (get_user(nArg, (int __user *)argp))
                return -EFAULT;
            VOC_PRINTF(TRACE_LEVEL, "MDRV_VOICE_IOCTL_SINEGEN_ENABLE %d\r\n",nArg);
            VocDmaEnableSinegen(nArg);
            break;
        case MDRV_VOICE_IOCTL_DMIC_GAIN:
            if (get_user(n8Arg, (char __user *)argp))
                return -EFAULT;
            VOC_PRINTF(TRACE_LEVEL, "MDRV_VOICE_IOCTL_DMIC_GAIN %d\r\n",n8Arg);
            VocDmicGain(n8Arg);
            break;
        default:
            VOC_PRINTF(ERROR_LEVEL, "Not supported ioctl for VOC-HWDEP\r\n");
            return -ENOIOCTLCMD;

            //......
    }
    return 0;

}
#endif
static int voc_soc_card_probe(struct snd_soc_card *card)
{
#ifdef __HWDEP__
    struct snd_hwdep *hwdep;
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    if (snd_hwdep_new(card->snd_card, "VOC-HWDEP", 0, &hwdep) < 0)
    {
        VOC_PRINTF(ERROR_LEVEL, "create VOC-HWDEP fail\n");
        return 0;
    }

    sprintf(hwdep->name, "VOC-HWDEP %d", 0);

    // hwdep->iface = SNDRV_HWDEP_IFACE_MS_VOC;
    hwdep->ops.open = voc_hwdep_open;
    hwdep->ops.ioctl = voc_hwdep_ioctl;
#else
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
#endif
    return 0;
}

int voc_soc_card_suspend_pre(struct snd_soc_card *card)
{
  VQ_CONFIG_S tVqConf;

  VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);

  // load cm4
  voc_soc_card_write(card,VOC_REG_CM4_ONOFF,0);
  voc_soc_card_write(card,VOC_REG_CM4_ONOFF,1);

    int i;
    int tmp_reg;
    VOC_PRINTF(TRACE_LEVEL, "%s reload CM4 setting\r\n", __FUNCTION__);
    for(i=0;i<VOC_REG_LEN;i++)
    {
        tmp_reg = voc_soc_card_read(card,i);
        voc_soc_card_write(card,i,0);
        voc_soc_card_write(card,i,tmp_reg);
    }
  // enable VQ
  voc_soc_card_write(card,VOC_REG_VQ_ONOFF,1);

  if(voc_soc_card_read(card, VOC_REG_CM4_ONOFF)==1)
  {
   if (voc_soc_card_read(card, VOC_REG_VQ_ONOFF))
   {
      VocEnableVp(1);
      tVqConf.nMode = 2; //PM
      VocConfigVq(tVqConf);
      VocEnableVq(1);
   }
  //switch Uart disable
//  voc_soc_card_write(card,VOC_REG_UART_ONOFF,1);
  //switch DMIC_GAIN
  voc_soc_card_write(card,VOC_REG_DMIC_GAIN,5);
  }
  return 0;
}
int voc_soc_card_suspend_post(struct snd_soc_card *card)
{
  VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
  VocDcOff();
  return 0;
}

int voc_soc_card_resume_pre(struct snd_soc_card *card)
{
  VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
  VocDcOn();
  // reload uart setting
  int tmp_reg;
  tmp_reg = voc_soc_card_read(card,VOC_REG_UART_ONOFF);
  voc_soc_card_write(card,VOC_REG_UART_ONOFF,0);
  voc_soc_card_write(card,VOC_REG_UART_ONOFF,tmp_reg);
  return 0;
}

int voc_soc_card_resume_post(struct snd_soc_card *card)
{
  VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
  if(voc_soc_card_read(card, VOC_REG_CM4_ONOFF)==1)
  {
   if (voc_soc_card_read(card, VOC_REG_VQ_ONOFF))
    {
      VocEnableVp(0);
      VocEnableVq(0);
    }
  }
  return 0;
}

static struct snd_soc_dai_link voc_soc_dais[] =
{
    {
        .name           = "Voc Soc Dai Link",
        .codec_name     = "snd-soc-dummy",
        .codec_dai_name = "snd-soc-dummy-dai",
        .cpu_dai_name   = "voc-cpu-dai",
        .platform_name  = "voc-platform",
    },
};


/* sound card controls */
static const struct snd_kcontrol_new voc_snd_controls[] =
{
    SOC_ENUM_VOC("CM4 switch", cm4_onoff_enum),
    SOC_ENUM_VOC("Voice WakeUp Switch", vq_onoff_enum),
    SOC_ENUM_VOC("Seamless Mode", seamless_onoff_enum),
#if defined(__VOICE_DA__)
    SOC_ENUM_VOC("Mic data arrange Switch", da_onoff_enum),
#endif
#if defined(__VOICE_AEC_SW_SYNC__)
    SOC_ENUM_VOC("AEC Mode", aec_mode_enum),
#endif
#if defined(__VOICE_I2S__)
    SOC_ENUM_VOC("I2S Switch", i2s_onoff_enum),
#endif
#if defined(__VOICE_HW_AEC__)
    SOC_ENUM_VOC("HW AEC Switch", hw_aec_onoff_enum),
#endif
    SOC_ENUM_VOC("Mic Number", dmic_number_enum),
    SOC_ENUM_VOC("Mic Bitwidth", mic_bitwidth_enum),
    SOC_ENUM_VOC("HPF Switch", hpf_onoff_enum),
    SOC_ENUM_VOC("HPF Coef", hpf_config_enum),
    SOC_ENUM_VOC("Uart enable Switch", uart_onoff_enum),
    SOC_ENUM_VOC("Sigen Switch", sigen_onoff_enum),
    SOC_SINGLE_EXT("Mic Gain Step (+6db)", VOC_REG_DMIC_GAIN, 0, 7, 0, voc_soc_card_get_volsw, voc_soc_card_put_volsw),
};

struct snd_soc_card voc_soc_card =
{
  .name       = "voc_snd_card",
  .owner      = THIS_MODULE,
  .dai_link	  = voc_soc_dais,
  .num_links  = 1,
  .probe      = voc_soc_card_probe,

  .controls = voc_snd_controls,
  .num_controls = ARRAY_SIZE(voc_snd_controls),

  .suspend_pre = voc_soc_card_suspend_pre,
  .suspend_post = voc_soc_card_suspend_post,
  .resume_pre = voc_soc_card_resume_pre,
  .resume_post = voc_soc_card_resume_post,
};
//EXPORT_SYMBOL_GPL(voc_soc_card);

#if 1//#ifndef CONFIG_OF

static struct platform_device *voc_snd_device = NULL;


static int voc_audio_probe(struct platform_device *pdev)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    voc_soc_card.dev = &pdev->dev;
    return snd_soc_register_card(&voc_soc_card);
}

int voc_audio_remove(struct platform_device *pdev)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    return snd_soc_unregister_card(&voc_soc_card);
}

static struct platform_driver voc_audio_driver =
{
    .driver = {
        .name	= "voc-audio",
        .owner = THIS_MODULE,
        .pm     = &snd_soc_pm_ops,
    },
    .probe		= voc_audio_probe,
    .remove   = voc_audio_remove,
};

static int __init voc_audio_init(void)
{
    int ret = 0;

    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);

    //----------------------------------------------------------------


    //----------------------------------------------------------------
    voc_snd_device = platform_device_alloc("voc-audio", -1);
    if (!voc_snd_device)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_device_alloc soc-audio error\r\n", __FUNCTION__);
        return -ENOMEM;
    }
    platform_set_drvdata(voc_snd_device, &voc_soc_card);
    ret = platform_device_add(voc_snd_device);
    if (ret)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_device_add infinity_snd_device error\r\n", __FUNCTION__);
        platform_device_put(voc_snd_device);
    }

    ret = platform_driver_register(&voc_audio_driver);
    if (ret)
    {
        VOC_PRINTF(ERROR_LEVEL, "%s: platform_driver_register voc_dma_driver error\r\n", __FUNCTION__);
        platform_device_unregister(voc_snd_device);
    }


    return ret;
}

static void __exit voc_audio_exit(void)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    platform_device_unregister(voc_snd_device);


}

module_init(voc_audio_init);
module_exit(voc_audio_exit);

#else
static int voc_audio_probe(struct platform_device *pdev)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    voc_soc_card.dev = &pdev->dev;
    return snd_soc_register_card(&voc_soc_card);
}

int voc_audio_remove(struct platform_device *pdev)
{
    VOC_PRINTF(TRACE_LEVEL, "%s\r\n", __FUNCTION__);
    return snd_soc_unregister_card(&voc_soc_card);
}

static const struct of_device_id voc_audio_of_match[] =
{
    { .compatible = "mstar,voc-audio", },
    {},
};
MODULE_DEVICE_TABLE(of, voc_audio_of_match);

static struct platform_driver voc_audio_driver =
{
    .driver = {
        .name	= "voc-audio",
        .owner = THIS_MODULE,
        .pm     = &snd_soc_pm_ops,
        .of_match_table = voc_audio_of_match,
    },
    .probe		= voc_audio_probe,
    .remove   = voc_audio_remove,
};

module_platform_driver(voc_audio_driver);
#endif

/* Module information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Trevor Wu, trevor.wu@mstarsemi.com");
MODULE_DESCRIPTION("Voice Audio ASLA SoC Machine");

