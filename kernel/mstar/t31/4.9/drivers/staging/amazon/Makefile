ccflags-y		+= -Idrivers/mstar2/include
ccflags-y		+= -Idrivers/mstar2/drv/gpio/
ccflags-y		+= -Idrivers/mstar2/hal/$(CONFIG_MSTAR_CHIP_NAME)/gpio/
ccflags-y		+= -Idrivers/mstar2/drv/mspi/
ccflags-y		+= -Idrivers/mstar2/hal/$(CONFIG_MSTAR_CHIP_NAME)/mspi/

obj-$(CONFIG_AMAZON_DSP_FRAMEWORK)      += dspframework/
obj-$(CONFIG_AMAZON_SIGN_OF_LIFE)       += sign_of_life.o
obj-$(CONFIG_AMAZON_SIGN_OF_LIFE_MSTAR) += sign_of_life_mstar.o
ifeq ($(or $(strip $(CONFIG_AMAZON_METRICS_LOG)), $(strip $(CONFIG_AMAZON_MINERVA_METRICS_LOG))),y)
	obj-y += amazon_logger.o
endif
obj-$(CONFIG_AMAZON_METRICS_LOG_TEST)   += amazon_logger_test.o
obj-$(CONFIG_AMAZON_SILENT_OTA_BACKLIGHT) += silent_ota_backlight_mstar.o
