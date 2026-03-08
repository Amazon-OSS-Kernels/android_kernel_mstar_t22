dtb-$(CONFIG_ARCH_VEXPRESS) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_ramdisk).dtb
dtb-$(CONFIG_ARCH_VEXPRESS) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an).dtb

dtb-$(CONFIG_MSTAR_M7632) += \
	m7632_MediaTek_m7632_MT164B.dtb \
	m7632_MediaTek_m7632_MTV19006.dtb \
	m7632_MediaTek_m7632_MTV19008.dtb \
	m7632_ramdisk.dtb \
	m7632_an_midgard.dtb \
	m7632_an.dtb \
	m7632_an.main_dtb \
	m7632_overlay_28.dtbo \
	m7632_overlay_28_shelly-sf.dtbo \
	m7632_overlay_28_shelly-dm.dtbo \
	m7632_overlay_avb28.dtbo \
	m7632_overlay_avb.dtbo \
	m7632_overlay_dto.dtbo \
	m7632_an_avb.dtb \
	m7632_eu_an.main_dtb \
	m7632_eu_overlay_28.dtbo \
	m7632_eu_overlay_28_anna-am.dtbo \
	m7632_eu_overlay_28_anna-tb.dtbo \
	m7632_eu_overlay_28_anna-bt.dtbo \
	m7632_eu_overlay_28_anna-kt.dtbo \
	m7632_eu_overlay_28_anna-mt.dtbo \
	m7632_eu_overlay_28_anna-ct.dtbo \
	m7632_eu_overlay_28_anna-cl.dtbo \
	m7632_eu_overlay_28_juliana.dtbo \
	m7632_eu_an.dtb \

dtb-$(CONFIG_MSTAR_M7332) += \
	m7332_an.dtb \
	m7332_an_avb.dtb \
	m7332_ramdisk.dtb \
	m7332_an_midgard.dtb \
	m7332_MediaTek_m7332_MT165B.dtb \
	m7332_MediaTek_m7332_MT168B.dtb \
	m7332_MediaTek_m7332_MT169B-2K.dtb \
	m7332_MediaTek_m7332_MT169B-4K.dtb \
	m7332_MediaTek_m7332_MTV19007.dtb \
	m7332_MediaTek_m7332_MTV19013.dtb \
	m7332_overlay_28.dtbo \
	m7332_overlay_avb28.dtbo \
	m7332_overlay_avb.dtbo \
	m7332_overlay_dto.dtbo \

dtb-$(CONFIG_MSTAR_M7642) += \
	m7642_an.dtb \
	m7642_ramdisk.dtb \
	m7642_an_midgard.dtb \
	m7642_MediaTek_m7642_MTV19020.dtb \
	m7642_MediaTek_m7642_MTV19024.dtb \
	m7642_MediaTek_m7642_MT166B.dtb \

ifeq ($(vendor_overlay),true)
ifeq ($(CONFIG_MSTAR_CHIP_NAME), "m7322")
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an_avb).dtb
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an).main_dtb
ifeq ($(T22_PCB_V20C),true)
$(info *vendor_overlay info use T22_PCB_V20C)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_T22_PCB_V20C_overlay).dtbo
else
ifeq ($(PRODUCT_BRAND),)
$(info *vendor_overlay info PRODUCT_BRAND is empty)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_28).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_avb28).dtbo
else
$(info ---vendor_overlay info (CONFIG_MSTAR_CHIP_NAME,PRODUCT_BRAND,TARGET_PRODUCT)=($(CONFIG_MSTAR_CHIP_NAME),$(PRODUCT_BRAND),$(TARGET_PRODUCT)))
ifeq ($(PRODUCT_BRAND),MediaTek)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_28).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_avb28).dtbo
else
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_$(PRODUCT_BRAND)_$(TARGET_PRODUCT)_overlay).dtbo
endif
endif
endif
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_1).dtbo
endif

ifeq ($(CONFIG_MSTAR_CHIP_NAME), "m5621")
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an_avb).dtb
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an).main_dtb
ifeq ($(PRODUCT_BRAND),)
$(info *vendor_overlay info PRODUCT_BRAND is empty)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_28).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_avb28).dtbo
else
$(info ---vendor_overlay info (CONFIG_MSTAR_CHIP_NAME,PRODUCT_BRAND,TARGET_PRODUCT)=($(CONFIG_MSTAR_CHIP_NAME),$(PRODUCT_BRAND),$(TARGET_PRODUCT)))
ifeq ($(PRODUCT_BRAND),MediaTek)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_28).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_avb28).dtbo
else
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_$(PRODUCT_BRAND)_$(TARGET_PRODUCT)_overlay).dtbo
endif
endif
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_1).dtbo
endif
else
ifeq ($(CONFIG_MSTAR_CHIP_NAME), "m7322")
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an_avb).dtb
ifeq ($(PRODUCT_BRAND),)
$(info *vendor info PRODUCT_BRAND is empty)
ifeq ($(T22_PCB_V20C),true)
$(info *vendor info use T22_V20C_PCB)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an_T22_V20C_PCB).main_dtb
else
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an).main_dtb
endif
else
$(info ---vendor info (CONFIG_MSTAR_CHIP_NAME,PRODUCT_BRAND,TARGET_PRODUCT)=($(CONFIG_MSTAR_CHIP_NAME),$(PRODUCT_BRAND),$(TARGET_PRODUCT)))
ifeq ($(PRODUCT_BRAND),MediaTek)
ifeq ($(T22_PCB_V20C),true)
$(info ---vendor info use T22_V20C_PCB)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an_T22_V20C_PCB).main_dtb
else
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an).main_dtb
endif
else
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_$(PRODUCT_BRAND)_$(TARGET_PRODUCT)).main_dtb
endif
endif
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_1).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_avb).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_dto).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_28).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_avb28).dtbo
endif

ifeq ($(CONFIG_MSTAR_CHIP_NAME), "m5621")
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an_avb).dtb
ifeq ($(PRODUCT_BRAND),)
$(info *vendor info PRODUCT_BRAND is empty)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an).main_dtb
else
$(info ---vendor info (CONFIG_MSTAR_CHIP_NAME,PRODUCT_BRAND,TARGET_PRODUCT)=($(CONFIG_MSTAR_CHIP_NAME),$(PRODUCT_BRAND),$(TARGET_PRODUCT)))
ifeq ($(PRODUCT_BRAND),MediaTek)
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_an).main_dtb
else
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_$(PRODUCT_BRAND)_$(TARGET_PRODUCT)).main_dtb
endif
endif
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_1).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_avb).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_dto).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_28).dtbo
dtb-$(CONFIG_OF) += $(subst ",,$(CONFIG_MSTAR_CHIP_NAME)_overlay_avb28).dtbo
endif
endif

targets += dtbs

DTB_NAMES := $(subst $\",,$(CONFIG_BUILD_ARM64_APPENDED_DTB_IMAGE_NAMES))
ifneq ($(DTB_NAMES),)
DTB_LIST := $(addsuffix .dtb,$(DTB_NAMES))
else
DTB_LIST := $(dtb-y)
endif
targets += $(DTB_LIST)

dtbs: $(addprefix $(obj)/, $(DTB_LIST))

clean-files := dts/*.dtb *.dtb dts/*.dtbo *.dtbo dts/*.main_dtb *.main_dtb
