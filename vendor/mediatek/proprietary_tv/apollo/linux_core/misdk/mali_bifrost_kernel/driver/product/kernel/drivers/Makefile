#
# (C) COPYRIGHT 2014-2017 MStar Semiconductor, Inc. All rights reserved.
#
# This program is free software and is provided to you under the terms of the
# GNU General Public License version 2 as published by the Free Software
# Foundation, and any use by you of this program is subject to the terms
# of such GNU licence.
#
# A copy of the licence is included with the program, and can also be obtained
# from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301, USA.
#
#

# References: bldsys/sconstruct and base/sconscript

#MSTAR: -Wno-format-security is added for clang chnaged of ANQ kernel
SCONS_CFLAGS = -Wall -Werror -Wno-format-security
SCONS_CONFIGS =
MALI_KBASE_DEP =

#
# Include project config
#
CONFIG_DIR = config

ifeq ($(CONFIG),)
$(error CONFIG is not specified)
endif

ifeq ($(KDIR),)
$(error KDIR is not specified)
endif

include $(CONFIG_DIR)/$(CONFIG).mak

#
# Set up defaults of MStar specific configs
#
mstar_platform_name = $(shell echo $(mstar_platform))

ifeq ($(os),android)
use_fixed_devid = 0
endif

ifeq ($(os),linux)
# fred MALI_KBASE_DEP += kds
endif

enable_gpu_mfdec ?= 0

#
# Set up ARCH and CROSS_COMPILE
#
ifeq ($(arch),arm_v7)
	ARCH = arm
	toolchain ?= arm-none-linux-gnueabi-
else ifeq ($(arch),arm_v8)
	ARCH = arm64
	toolchain ?= aarch64-linux-gnu-
endif

export CROSS_COMPILE=$(toolchain)

#
# Set up ARM native flags and configs
#
SCONS_CONFIGS += CONFIG_MALI_MIDGARD=m

ifneq ($(os),android)
SCONS_CFLAGS += -DCONFIG_KDS
SCONS_CONFIGS += CONFIG_KDS=m
endif

ifeq ($(debug),1)
SCONS_CFLAGS += -DCONFIG_MALI_DEBUG
SCONS_CONFIGS += CONFIG_MALI_DEBUG=y
endif

SCONS_CFLAGS += -DCONFIG_MALI_PLATFORM_FAKE
SCONS_CONFIGS += CONFIG_MALI_PLATFORM_FAKE=y

SCONS_CFLAGS += -DCONFIG_MALI_PLATFORM_THIRDPARTY
SCONS_CONFIGS += CONFIG_MALI_PLATFORM_THIRDPARTY=y CONFIG_MALI_PLATFORM_THIRDPARTY_NAME=mstar

ifeq ($(dvfs),1)
SCONS_CFLAGS += -DCONFIG_MALI_MIDGARD_DVFS
SCONS_CONFIGS += CONFIG_MALI_MIDGARD_DVFS=y
endif

#
# Set up MStar specific flags and configs
#
SCONS_CFLAGS += -DMSTAR
SCONS_CFLAGS += -DMSTAR_$(strip $(shell echo $(mstar_platform) | tr a-z A-Z))
SCONS_CFLAGS += -DMSTAR_$(strip $(shell echo $(project) | tr a-z A-Z))
SCONS_CFLAGS += -DMALI_MAX_FREQ=$(shell echo $(mali_max_freq))
SCONS_CFLAGS += -DMALI_MIN_FREQ=$(shell echo $(mali_min_freq))
SCONS_CFLAGS += -DMALI_BOOST_FREQ=$(shell echo $(mali_boost_freq))
SCONS_CFLAGS += -DMALI_IRQ=$(shell echo $(mali_irq))
SCONS_CFLAGS += -DMALI_REG_OFFSET=$(shell echo $(mali_reg_offset))
SCONS_CFLAGS += -DMSTAR_RIU_ENABLED
SCONS_CFLAGS += -DMSTAR_PM_CALLBACKS
SCONS_CFLAGS += -DMSTAR_DISABLE_CONFIG_OF
# XXX: kernel enables CONFIG_NEED_SG_DMA_LENGTH, but dma_length is not set correctly in kernel.
#      we disable this config internally in mali driver to use length instead of dma_length.
SCONS_CFLAGS += -DMSTAR_DISABLE_CONFIG_NEED_SG_DMA_LENGTH
SCONS_CFLAGS += -DMSTAR_GPU_UTILIZATION
SCONS_CFLAGS += -DMSTAR_MEMORY_USAGE
SCONS_CFLAGS += -DMSTAR_DISABLE_SHADER_CORES

ifeq ($(mstar_platform_name),m7221)
SCONS_CFLAGS += -DMALI_MAX_FREQ_U01=$(shell echo $(mali_max_freq_u01))
SCONS_CFLAGS += -DMALI_MAX_FREQ_U02=$(shell echo $(mali_max_freq_u02))
endif

ifeq ($(use_fixed_devid),1)
SCONS_CFLAGS += -DMSTAR_USE_FIXED_DEVID
endif

SCONS_CFLAGS += -DMSTAR_UMM_EXPORT=$(umm_export)

ifeq ($(skip_jobs),1)
SCONS_CFLAGS += -DMSTAR_SKIP_JOBS
endif

ifeq ($(dvfs_freq_adjustable),1)
SCONS_CFLAGS += -DMSTAR_DVFS_FREQ_ADJUSTABLE
endif

ifeq ($(deglitch_patch),1)
SCONS_CFLAGS += -DMSTAR_DEGLITCH_PATCH
endif

SCONS_CFLAGS += -DMSTAR_CLOCK_DEBUG_ADJUSTABLE=$(clock_debug_adjustable)

SCONS_CONFIGS += MSTAR_PLATFORM_NAME=$(mstar_platform_name)
SCONS_CONFIGS += MSTAR_UMM_EXPORT=$(umm_export)
SCONS_CONFIGS += MSTAR_CLOCK_DEBUG_ADJUSTABLE=$(clock_debug_adjustable)

SCONS_CFLAGS += -DENABLE_GPU_MFDEC=$(enable_gpu_mfdec)
#
# Targets
#

ifndef MTK_EXTMOD_OBJ
OUT_DIR = modules
MAKE_OUT_DIR = mkdir -p $(OUT_DIR)
CLEAN_OUT_DIR = rm -rf $(OUT_DIR)
endif

all: kbase

kbase: $(MALI_KBASE_DEP)
	$(MAKE_OUT_DIR)
	SCONS_CONFIGS="$(SCONS_CONFIGS)" SCONS_CFLAGS="$(SCONS_CFLAGS)" $(MAKE) -C gpu/arm/midgard
ifndef MTK_EXTMOD_OBJ
	cp gpu/arm/midgard/mali_kbase.ko $(OUT_DIR)
endif

kds:
	$(MAKE_OUT_DIR)
	ARCH=$(ARCH) SCONS_CONFIGS="$(SCONS_CONFIGS)" SCONS_CFLAGS="$(SCONS_CFLAGS)" $(MAKE) -C base/kds
ifndef MTK_EXTMOD_OBJ
	cp base/kds/kds.ko $(OUT_DIR)
endif

clean: $(addsuffix -clean, $(MALI_KBASE_DEP))
	$(MAKE) -C gpu/arm/midgard clean
ifndef MTK_EXTMOD_OBJ
	$(CLEAN_OUT_DIR)
endif

kds-clean:
	ARCH=$(ARCH) $(MAKE) -C base/kds clean
