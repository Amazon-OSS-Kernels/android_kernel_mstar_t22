#
# Makefile for the farfeild DSP power control
#

ccflags-y		+= -Idrivers/mstar2/include
ccflags-y		+= -Idrivers/mstar2/drv/gpio/
ccflags-y		+= -Idrivers/mstar2/hal/$(CONFIG_MSTAR_CHIP_NAME)/gpio/
obj-$(CONFIG_FFDSPCTRL)		+= ffdspctrl.o
obj-$(CONFIG_FFDSPCTRL)		+= ffdspdev.o
obj-$(CONFIG_FFDSPCTRL)		+= ffmt8570.o
obj-$(CONFIG_FFDSPCTRL)		+= ffsuecreek.o
