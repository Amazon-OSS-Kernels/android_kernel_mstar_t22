O_TARGET    = $(TOPDIR)/$(DRIVER_PATH)/drvUfs.a

UFS_ROOT	= .

CFLAGS	    += -I$(UFS_ROOT)/inc/api -I$(UFS_ROOT)/inc/common -I$(UFS_ROOT)/inc/config

obj-y		+= src/config/pll-mstar-pltfrm.o    \
				src/common/ufshcd.o    \
				src/common/ufshcd-pltfrm.o    \
				src/common/ufs-mstar.o    \
				src/config/ufs-mstar-pltfrm.o \
				ufs-mstar-main.o


include $(TOPDIR)/Rules.make
