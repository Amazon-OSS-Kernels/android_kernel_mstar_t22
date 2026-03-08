#
# Makefile for MStar CMA Pool device drivers.
#

ifdef CONFIG_MP_MMA_ENABLE
# general options
EXTRA_CFLAGS      +=  -Idrivers/mstar2/include/sys/common
EXTRA_CFLAGS      +=  -Idrivers/mstar2/include
EXTRA_CFLAGS      +=  -Idrivers/mstar2/drv/miu_protect
EXTRA_CFLAGS      +=  -Idrivers/mstar2/drv/miu
# specific options
EXTRA_CFLAGS      +=  -Idrivers/staging/android/ion

# files
ifeq ($(CONFIG_CMA),y)
obj-$(CONFIG_MSTAR_MIUPROTECT)     += mdrv-miu_protect.o
mdrv-miu_protect-objs                       += mdrv_miu_protect.o
endif

endif
