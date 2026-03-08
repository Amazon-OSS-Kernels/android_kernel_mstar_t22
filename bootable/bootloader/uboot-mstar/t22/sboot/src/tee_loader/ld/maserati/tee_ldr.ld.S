#include <autoconf.h>
#if (CONFIG_MSTAR_BUILD_TEE_LOADER_DAT == 1)
        *tee_loader.o       (.text*)
        *bootram.o          (Bootram_init)

        KEEP (*(SORT_BY_NAME(.rodata.padInitTbl)))

        *nandflash.o        (.text*)
        *nandflash.o        (.rodata*)
        *uart16550.o        (.text*)
        *setSPI.o           (.text*)
        *setSPI.o           (.rodata*)
        *apiBDMA.o          (.text*)
        *apiBDMA.o          (.rodata*)
        *apiBDMA.o          (.data*)
        *apiBDMA.o          (.bss*)

        *ns_uboot_loader.o          (.text*)
        *ns_uboot_loader.o          (.rodata*)
        *drvSEAL.o          (.text*)
        *halSEAL.o          (.text*)
        *halSEAL.o          (.rodata*)
        *halAESDMA_v2.o     (.text*)
        *drvAESDMA_v2.o     (.text*)
        *drvAESDMA_v2.o     (.rodata*)

        *MSDecompress.o     (.text*)
        *MSDecompress.o     (.rodata*)
        *MSDecompress.o     (.data*)
        *MSDecompress.o     (.bss*)
#if (CONFIG_MSTAR_FAST_STR == 1)
        *apiSecureR2.o      (.text*)
        *apiSecureR2.o      (.rodata*)
        *apiSecureR2.o      (.data*)
        *apiSecureR2.o      (.bss*)
        *strBootTEE.o       (.text*)
        *strBootTEE.o       (.rodata*)
        *strBootTEE.o       (.data*)
        *fastSTRSetting.o      (.text*)
        *fastSTRSetting.o      (.rodata*)
        *fastSTRSetting.o      (.data*)
        *fastSTRSetting.o      (.bss*)
#endif
        *drvRIU.o           (.text*)
        *drvAESDMA.o        (.text*)
        *drvAESDMA.o        (.rodata*)
        *halAESDMA.o        (.text*)
        *eMMC_prg.o         (.text*)
        *eMMC_hal.o         (.text*)
        *eMMC_platform.o    (.text*)
        *eMMC_RPMB.o        (.text*)
        *eMMC_prg.o         (.data*)
        *eMMC_hal.o         (.data*)
        *eMMC_platform.o    (.data*)
        *eMMC_RPMB.o        (.data*)
        *eMMC_prg.o         (.bss*)
        *eMMC_hal.o         (.bss*)
        *eMMC_platform.o    (.bss*)
        *eMMC_RPMB.o        (.bss*)

        *(EMMCCODE)
        *(.ARM.exidx*)

        _ld_teeloader_data_start = ABSOLUTE(.);
        *(.data*)
        . = ALIGN(16);
        _ld_teeloader_data_end = ABSOLUTE(.);
        . = ALIGN(16);
        _ld_bootbss_start = ABSOLUTE(.);
        *(.bss*)
        . = ALIGN(16);
        _ld_bootbss_end = ABSOLUTE(.);
        /* Reserved 0x100 for signature */
        . = LENGTH(tee_loader) - 0x300;
#endif
