#
# Makefile for MStar GFLIP device drivers.
CONFIG_MSTAR_CHIP_NAME := $(subst ",,$(CONFIG_MSTAR_CHIP_NAME))
ifdef CONFIG_MSTAR_PROJECT_NAME
    CONFIG_MSTAR_PROJECT_NAME := $(subst ",,$(CONFIG_MSTAR_PROJECT_NAME))
endif


# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar2/include
EXTRA_CFLAGS      +=  -Idrivers/mstar2/drv/mspi/
EXTRA_CFLAGS      +=  -Idrivers/mstar2/hal/$(CONFIG_MSTAR_CHIP_NAME)/mspi/

EXTRA_CFLAGS      +=  -DRED_LION

# specific options
ifeq ($(TARGET_PRODUCT), $(filter $(TARGET_PRODUCT), brandenburg anna hailey))
EXTRA_CFLAGS      +=  -DWAR_MT8570_DSP
endif

# files
obj-$(CONFIG_MSTAR_MSPI)   := mdrv-mspi.o
mdrv-mspi-objs   +=  spi-mstar.o
mdrv-mspi-objs   +=  mdrv_mspi.o
mdrv-mspi-objs   +=  ../../hal/$(CONFIG_MSTAR_CHIP_NAME)/mspi/mhal_mspi.o
