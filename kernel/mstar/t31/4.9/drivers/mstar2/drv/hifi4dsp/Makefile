#
# Copyright (C) 2018 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

EXTRA_CFLAGS      +=  -Idrivers/mstar2/include
subdir-ccflags-y += -I$(srctree)/drivers/mstar2/drv/hifi4dsp/
subdir-ccflags-y += -I$(srctree)/drivers/mstar2/drv/hifi4dsp/hifi4dsp_audio_ipi/common/framework

obj-$(CONFIG_MTK_HIFI4DSP_SUPPORT) += hifi4dsp_spi/
obj-$(CONFIG_MTK_HIFI4DSP_SUPPORT) += hifi4dsp_load/
obj-$(CONFIG_MTK_HIFI4DSP_SUPPORT) += hifi4dsp_ipi/
obj-$(CONFIG_MTK_HIFI4DSP_SUPPORT) += hifi4dsp_audio_ipi/
obj-$(CONFIG_MTK_HIFI4DSP_SUPPORT) += hifi4dsp_wdt/
