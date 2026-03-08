ccflags-y += -I$(srctree)/drivers/mstar2/include/
ccflags-y += -I$(srctree)/drivers/pinctrl/mediatek/
ifeq ($(CONFIG_MTK_HIFI4DSP_SUPPORT),y)
ccflags-y += -I$(srctree)/drivers/mstar2/drv/hifi4dsp/hifi4dsp_audio_ipi/common/framework
ccflags-y += -I$(srctree)/drivers/mstar2/drv/hifi4dsp/hifi4dsp_ipi/mt8570
ccflags-y += -I$(srctree)/drivers/mstar2/drv/hifi4dsp/hifi4dsp_ipi/common
ccflags-y += -I$(srctree)/drivers/mstar2/drv/hifi4dsp/
endif

snd-soc-mt8518-spi-objs := \
    mt8518-audio-spi.o mt8518-adsp-utils.o mt8518-adsp-controls.o \
    mt8518-adsp-debug.o mt8518-snd-utils.o


# MTK Platform Support
obj-$(CONFIG_MTK_HIFI4DSP_SUPPORT) += snd-soc-mt8518-spi.o
# Machine support
obj-$(CONFIG_SND_SOC_MT8518_EVB) += mt8518-evb.o
obj-y				+= pinctrl-mt8518-ipi.o