EXTRA_CFLAGS      +=  -Idrivers/mstar2/include
EXTRA_CFLAGS      +=  -Idrivers/mstar2/drv/hifi4dsp/hifi4dsp_spi

ccflags-y += -I$(srctree)/drivers/mstar2/drv/hifi4dsp/hifi4dsp_audio_ipi/common/framework
ccflags-$(CONFIG_AMAZON_DSP_FRAMEWORK) += \
	-I$(srctree)/drivers/staging/amazon/dspframework/include

obj-y += adsp_ipi.o
obj-y += adsp_ipi_platform.o

ifeq ($(CONFIG_AMAZON_DSP_IPC_AGENT), y)
# hailey used hifi4dsp/hifi4dsp_agent/mt8570/acs_audio_ipc_hw_impl.c
obj-y += acs_audio_ipc_hwio.o
endif
