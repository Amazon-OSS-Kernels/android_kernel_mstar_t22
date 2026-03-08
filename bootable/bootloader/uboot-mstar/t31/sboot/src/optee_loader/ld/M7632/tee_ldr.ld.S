#include <autoconf.h>
        _ld_TEE_LDR_run_base = .;
        LONG(0x3052444C);                                       /* TAG:LDR0 */
        LONG(0x0);                                              /* Version value */
        LONG( _ld_TEE_LDR_run_base);                            /* ldr_entry ram start */
        LONG( _ld_TEE_LDR_run_start - _ld_TEE_LDR_run_base);    /* ldr_entry offset, header size */
        LONG( _ld_TEE_LDR_stack_end );                          /* ldr_stack_end */
        LONG( SIZEOF(.tee_loader)-1024 );                       /* For backward compatible  */
        LONG( _ld_TEE_LDR_arg_start );                          /* Argument start */
        LONG(0x0);                                              /* DEBUG LEVEL */
        LONG(CONFIG_TLOPTION);                                  /* Additional TL options */
        . = ALIGN(0x80);                                        /* Reserve space*/
        LONG( EmbeddedTL_D_TI_KEY - _ld_TEE_LDR_run_base );     /* TL_D_TI_KEY */
        LONG( EmbeddedTL_A_TI_KEY - _ld_TEE_LDR_run_base );     /* TL_A_TI_KEY */
        LONG( EmbeddedTL_D_REE_KEY - _ld_TEE_LDR_run_base );    /* TL_D_REE_KEY */
        LONG( EmbeddedTL_A_REE_KEY - _ld_TEE_LDR_run_base );    /* TL_A_REE_KEY */
        . = ALIGN(0x100);
        
        _ld_TEE_LDR_run_start = .;
        _ld_TEE_LDR_signature = .;
        *tee_loader.o                     (.text*)
        *tee_loader.o                     (.data*)
#if (CONFIG_OPTEE_TEELOADER==1)
        *ns_uboot_loader_optee.o          (.tee.entry)
        *ns_uboot_loader_optee.o          (.text*)
        *ns_uboot_loader_optee.o          (.data*)
        *ns_uboot_loader_optee.o          (.rodata*)
#elif (CONFIG_R2TEE_TEELOADER==1)
        *ns_uboot_loader_r2.o          (.tee.entry)
        *ns_uboot_loader_r2.o          (.text*)
        *ns_uboot_loader_r2.o          (.data*)
        *ns_uboot_loader_r2.o          (.rodata*)
#endif
        *ns_uboot_common.o          (.text*)
        *ns_uboot_common.o          (.data*)
        *ns_uboot_common.o          (.rodata*)
        *apiBDMA.o          (.text*)
        *apiBDMA.o          (.rodata*)
        *apiBDMA.o          (.data*)
        *bootram.o          (.ldr.teeloader.text*)
        KEEP (*(SORT_BY_NAME(.rodata.padInitTbl)))
        *MSDecompress.o     (.text*)
        *MSDecompress.o     (.rodata*)
        *MSDecompress.o     (.data*)
        *drvSEAL.o          (.text*)
        *drvSEAL.o          (.rodata*)
        *drvSEAL.o          (.data*)
        *halSEAL.o          (.text*)
        *halSEAL.o          (.rodata*)
        *halSEAL.o          (.data*)
        *halAESDMA_v2.o     (.text*)
        *drvAESDMA_v2.o     (.text*)
        *drvAESDMA_v2.o     (.data*)
        *drvAESDMA_v2.o     (.rodata*)
        *halAESDMA.o        (.text*)
        *drvAESDMA.o        (.text*)
        *drvAESDMA.o        (.rodata*)
        *_dvmd_tls.o        (.text*)
        *_udivsi3.o         (.text*)
#if (CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH==1)
        *eMMC_prg.o             (.text*)
        *eMMC_hal.o             (.text*)
        *eMMC_platform.o        (.text*)
        *eMMC_RPMB.o            (.text*)
        *eMMC_prg.o             (.data*)
        *eMMC_hal.o             (.data*)
        *eMMC_platform.o        (.data*)
        *eMMC_RPMB.o            (.data*)
        *(EMMCCODE)
#elif (CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH==1)
        *drvNAND_prg.o          (.text*)
        *drvNAND_hal.o          (.text*)
        *drvNAND_hal_wp.o       (.text*)
        *drvNAND_hal_v5.o       (.text*)
        *drvNAND_hal_v5_tee.o   (.text*)
        *drvNAND_platform.o     (.text*)
        *drvNAND_utl.o          (.text*)
        *drvNAND_prg.o          (.data*)
        *drvNAND_utl.o          (.data*)
        *ftlsd_api.o            (.text*)
        *ftlsd_cfg_sboot.o      (.text*)
        *ftlsd_data.o           (.text*)
        *ftlsd_dbg.o            (.text*)
        *ftlsd_hal.o            (.text*)
        *ftlsd_hal_plane.o      (.text*)
        *ftlsd_test.o           (.text*)
        *ftlsd_utl.o            (.text*)
        *ftlsd_api.o            (.data*)
        *ftlsd_cfg_sboot.o      (.data*)
        *ftlsd_data.o           (.data*)
        *ftlsd_dbg.o            (.data*)
        *ftlsd_hal.o            (.data*)
        *ftlsd_hal_plane.o      (.data*)
        *ftlsd_test.o           (.data*)
        *ftlsd_utl.o            (.data*)
        *ftlsd_api.o            (.rodata*)
        *ftlsd_cfg_sboot.o      (.rodata*)
        *ftlsd_data.o           (.rodata*)
        *ftlsd_dbg.o            (.rodata*)
        *ftlsd_hal.o            (.rodata*)
        *ftlsd_hal_plane.o      (.rodata*)
        *ftlsd_test.o           (.rodata*)
        *ftlsd_utl.o            (.rodata*)
        *(NANDCODE)*(.rodata*)
#endif
        *(.ARM.exidx*)
        . = ALIGN(0x10);
        _ld_TEE_LDR_arg_start = .;
        . += 0x100;
        _ld_TEE_LDR_arg_end = .;
        . = ALIGN(0x400);
        . += 2k;
        _ld_TEE_LDR_stack_end = .;
        _ld_TEE_LDR_run_end = .;
        . = ALIGN(0x800);
        _ld_TEE_LDR_vector_start = .;
        *bootram.o                   (.ldr.teepostvector.text*)
        _ld_TEE_LDR_vector_end = .;
        _ld_TEE_LDR_postloader_start = .;
        *bootram.o                   (.ldr.teepostloader.text*)
        *drvSEAL.o                   (.ldr.teepostloader.text*)
        *halSEAL.o                   (.ldr.teepostloader.text*)
        *_ashldi3.o                  (.text*)
#if (CONFIG_OPTEE_TEELOADER==1)
        *ns_uboot_common.o           (.ldr.teepostloader.text*)
        *ns_uboot_loader_optee.o     (.ldr.teepostloader.text*)
        *ns_uboot_loader_optee.o     (.ldr.teepostloader.static*)
#endif
        _ld_TEE_LDR_postloader_end = .;
#if (CONFIG_OPTEE_TEELOADER==1)
        . = LENGTH(tee_loader) - 1024 - 16;
        _ld_TEE_MCT_Address = .;
        LONG(0x0);
#endif
        . = LENGTH(tee_loader) - 1024;
        /* Reserved for Secure Booting */
        . = LENGTH(tee_loader);