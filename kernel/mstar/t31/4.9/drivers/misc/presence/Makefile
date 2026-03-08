#
# Makefile for misc devices that really don't fit anywhere else.
#

ccflags-y		+= -Idrivers/mstar2/include
ccflags-y		+= -Idrivers/mstar2/drv/gpio/
ccflags-y		+= -Idrivers/mstar2/hal/$(CONFIG_MSTAR_CHIP_NAME)/gpio/

obj-$(CONFIG_SENSORS_SC1233A)	+= sinext_sc1233a.o

