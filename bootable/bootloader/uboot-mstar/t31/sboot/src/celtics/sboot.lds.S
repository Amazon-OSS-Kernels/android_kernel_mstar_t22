/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2019 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2019 MediaTek Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/


OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH("aarch64")

ENTRY(Boot_Entry)

MEMORY
{
#if (CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH == 1)
    boot :  ORIGIN = 0x1fc00000,                LENGTH = 14K
    boot2 : ORIGIN = 0x1fc00000 + 7K,           LENGTH = 7K
    boot3 : ORIGIN = 0x1fc00000 + 7K,           LENGTH = 7K
    ram :   ORIGIN = CONFIG_BOOTRAM_LOADADDR,   LENGTH = 9K
#elif (CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH == 1)
    boot :  ORIGIN = 0x1fc00000,                LENGTH = 14K
    boot2 : ORIGIN = 0x1fc00000 + 7K,           LENGTH = 7K
    boot3 : ORIGIN = 0x1fc00000 + 7K,           LENGTH = 7K
    boot4 : ORIGIN = 0x1fc00000 + 7K,           LENGTH = 7K
    ram :   ORIGIN = CONFIG_BOOTRAM_LOADADDR,   LENGTH = 9K
#else
    boot :  ORIGIN = 0x00000000,                LENGTH = 8K
    rom :   ORIGIN = 0x00000000+0x2000,         LENGTH = 18K
    ram :   ORIGIN = CONFIG_BOOTRAM_LOADADDR,   LENGTH = 128K
#endif
}

SECTIONS
{
#if (CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH == 1)
    .text1 :
    {
        _ld_boot_start = ABSOLUTE(.);
        *boot.o             (.text*)

        . = ALIGN(16);
        _ld_boot_end = ABSOLUTE(.);
        _ld_boot_size = _ld_boot_end - _ld_boot_start;

        _ld_bootrom_start = ABSOLUTE(.);
        *bootrom.o          (.text*)
        *bootrom_cache.o    (.text*)
        *drvRIU.o           (.text*)
        *drvAESDMA.o        (.text*)
        *halAESDMA.o        (.text*)
        *drvNAND_prg.o      (.text*)
        *drvNAND_hal.o      (.text*)
        *drvNAND_hal_v5.o   (.text*)
        *drvNAND_platform.o (.text*)
        *drvNAND_prg.o      (.data*)
        *drvNAND_prg.o      (.bss*)
        *drvNAND_hal.o      (.bss*)
        *drvNAND_hal_v5.o   (.bss*)
        *drvNAND_platform.o (.bss*)
        *bootrom_data_miu.o (.rodata*)

        . = LENGTH(boot) - LENGTH(boot2);

        _ld_program_reload_addr = ABSOLUTE(.);
        *bootrom_pll.o      (.text*)
        *bootrom_clock.o    (.text*)
        *bootrom_peripheral.o   (.text*)
        *bootrom.o          (.rodata*)
        *drvPadConf.o       (.rodata*)

        . = LENGTH(boot) - 1024;
        /* Reserved for Secure Booting */
        . = LENGTH(boot);
        _ld_bootrom_end = ABSOLUTE(.);
        _ld_bootrom_size = _ld_bootrom_end - _ld_bootrom_start;

    } > boot

    .text2 : AT (LOADADDR(.text1) + LENGTH(boot))
    {
        _ld_miu_flow_flash_start = LENGTH(boot) - _ld_program_reload_addr;
        _ld_miu_flow_flash_size = LENGTH(boot2) - _ld_program_reload_addr;
        _ld_miu_flow_start = ABSOLUTE(.);

        *bootrom_miu.o         (.text*)

        . = LENGTH(boot2) - 1024;
        /* Reserved for Secure Booting */
        . = LENGTH(boot2);
        _ld_miu_flow_end = ABSOLUTE(.);
        _ld_miu_flow_size = _ld_miu_flow_end - _ld_miu_flow_start;

    } > boot2

    .text3 : AT (LOADADDR(.text2) + LENGTH(boot2))
    {
        _ld_miu_str_flow_flash_start = LENGTH(boot2) - _ld_program_reload_addr + _ld_miu_flow_flash_start;
        _ld_miu_str_flow_flash_size = LENGTH(boot3) - _ld_program_reload_addr;
        _ld_miu_str_flow_start = ABSOLUTE(.);

        *bootrom_miu_str.o      (.text*)

        . = LENGTH(boot3) - 1024;
        /* Reserved for Secure Booting */
        . = LENGTH(boot3);
        _ld_miu_str_flow_end = ABSOLUTE(.);
        _ld_miu_str_flow_size = _ld_miu_str_flow_end - _ld_miu_str_flow_start;

    } > boot3

    .text4 : AT (LOADADDR(.text1) + 0x8000)
    {
        _ld_bootram_start = ABSOLUTE(.);
        *bootram.o          (.text*)
        *nandflash.o        (.text*)
        *nandflash.o        (.rodata*)
        *uart16550.o        (.text*)
        *setSPI.o           (.text*)
        *setSPI.o           (.rodata*)
        *apiBDMA.o          (.text*)
        *apiBDMA.o          (.rodata*)
        *apiBDMA.o          (.data*)
        *apiBDMA.o          (.bss*)

        *MSDecompress.o     (.text*)
        *MSDecompress.o     (.rodata*)
        *MSDecompress.o     (.data*)
        *MSDecompress.o     (.bss*)

        *(NANDCODE)

        *(.ARM.exidx*)

        . = ALIGN(16);
        _ld_bootdata_start = .;
        *(.data*)
        . = ALIGN(16);
        _ld_bootdata_end = .;

        _ld_bootbss_start = ABSOLUTE(.);
        *(.bss*)
        . = ALIGN(16);
        _ld_bootbss_end = ABSOLUTE(.);

        . = LENGTH(ram) - 1024;
        /* Reserved for Secure Booting */
        . = LENGTH(ram);
        _ld_bootram_end = ABSOLUTE(.);
        _ld_bootram_size = _ld_bootram_end - _ld_bootram_start;

    } > ram

#elif (CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH == 1)
    .text1 :
    {
        *boot.o             (.text*)

        . = ALIGN(16);
        _ld_bootrom_start = ABSOLUTE(.);

        *bootrom.o          (.text*)
        *bootrom_cache.o    (.text*)
        *drvRIU.o           (.text*)
        *drvAESDMA.o        (.text*)
        *halAESDMA.o        (.text*)
        *eMMC_prg.o         (.text*)
        *eMMC_hal.o         (.text*)
        *eMMC_platform.o    (.text*)
        *eMMC_prg.o         (.data*)
        *eMMC_hal.o         (.data*)
        *eMMC_platform.o    (.data*)
        *eMMC_prg.o         (.bss*)
        *eMMC_hal.o         (.bss*)
        *eMMC_platform.o    (.bss*)
        *bootrom_data_miu.o (.rodata*)

        . = LENGTH(boot) - LENGTH(boot2);

        _ld_program_reload_addr = ABSOLUTE(.);
        *bootrom_pll.o      (.text*)
        *bootrom_clock.o    (.text*)
        *bootrom_peripheral.o   (.text*)
        *bootrom.o          (.rodata*)
        *drvPadConf.o       (.rodata*)

        . = LENGTH(boot) - 1024;
        . = LENGTH(boot);
        _ld_bootrom_end = ABSOLUTE(.);
        _ld_bootrom_size = _ld_bootrom_end - _ld_bootrom_start;

    } > boot

    .text2 : AT (LOADADDR(.text1) + LENGTH(boot))
    {
        _ld_miu_flow_flash_start = LENGTH(boot) - _ld_program_reload_addr;
        _ld_miu_flow_flash_size = LENGTH(boot2) - _ld_program_reload_addr;
        _ld_miu_flow_start = ABSOLUTE(.);

        *bootrom_miu.o         (.text*)
    
    #if (CONFIG_MIU_DDR4 == 1)
        *bootrom_dqs_ddr4.o      (.text*)
        *bootrom_dqs_ddr4.o     (.rodata*)
    #elif (CONFIG_MIU_DDR3 == 1)
        *bootrom_dqs_ddr3.o      (.text*)
        *bootrom_dqs_ddr3.o     (.rodata*)
    #else
    #endif

        . = LENGTH(boot2) - 1024;
        . = LENGTH(boot2);
        _ld_miu_flow_end = ABSOLUTE(.);
        _ld_miu_flow_size = _ld_miu_flow_end - _ld_miu_flow_start;

    } > boot2

    .text3 : AT (LOADADDR(.text2) + LENGTH(boot2))
    {
        _ld_miu_str_flow_flash_start = LENGTH(boot2) - _ld_program_reload_addr + _ld_miu_flow_flash_start;
        _ld_miu_str_flow_flash_size = LENGTH(boot3) - _ld_program_reload_addr;
        _ld_miu_str_flow_start = ABSOLUTE(.);

        *bootrom_miu_str.o      (.text*)

        . = LENGTH(boot3) - 1024;
        . = LENGTH(boot3);
        _ld_miu_str_flow_end = ABSOLUTE(.);
        _ld_miu_str_flow_size = _ld_miu_str_flow_end - _ld_miu_str_flow_start;

    } > boot3
    
    .text4 : AT (LOADADDR(.text3) + LENGTH(boot3))
    {
        _ld_miu_dqs_flow_start = ABSOLUTE(.);

#if 0
    #if (CONFIG_MIU_DDR4 == 1)
        *bootrom_dqs_ddr4.o      (.text*)
        *bootrom_dqs_ddr4.o     (.rodata*)
    #elif (CONFIG_MIU_DDR3 == 1)
        *bootrom_dqs_ddr3.o      (.text*)
        *bootrom_dqs_ddr3.o     (.rodata*)
    #else
    #endif
#endif

        . = LENGTH(boot4) - 1024;
        /* Reserved for Secure Booting */
        . = LENGTH(boot4);
        _ld_miu_dqs_flow_end = ABSOLUTE(.);
        _ld_miu_dqs_flow_size = _ld_miu_dqs_flow_end - _ld_miu_dqs_flow_start;

    } > boot4

    .text5 : AT (LOADADDR(.text1) + 0x8000)
    {
        _ld_bootram_start = .;
        *bootram.o          (.text*)
        *nandflash.o        (.text*)
        *nandflash.o        (.rodata*)
        *uart16550.o        (.text*)
        *setSPI.o           (.text*)
        *setSPI.o           (.rodata*)
        *apiBDMA.o          (.text*)
        *apiBDMA.o          (.rodata*)
        *apiBDMA.o          (.data*)
        *apiBDMA.o          (.bss*)

        *MSDecompress.o     (.text*)
        *MSDecompress.o     (.rodata*)
        *MSDecompress.o     (.data*)
        *MSDecompress.o     (.bss*)

        *(.ARM.exidx*)

        . = ALIGN(16);
        _ld_bootdata_start = .;
        *(.data*)
        . = ALIGN(16);
        _ld_bootdata_end = .;

        _ld_bootbss_start = ABSOLUTE(.);
        *(.bss*)
        . = ALIGN(16);
        _ld_bootbss_end = ABSOLUTE(.);

        . = LENGTH(ram) - 1024;
        /* Reserved for Secure Booting */
        . = LENGTH(ram);
        _ld_bootram_end = ABSOLUTE(.);
        _ld_bootram_size = _ld_bootram_end - _ld_bootram_start;

    } > ram

#else
    .text1 :
    {
        _ld_boot_start = ABSOLUTE(.);
        *boot.o             (.text*)
        . = ALIGN(16);
        _ld_boot_end = ABSOLUTE(.);
        _ld_boot_size = _ld_boot_end - _ld_boot_start;

    } > boot

    .text2 : AT ( LOADADDR(.text1) + 0x2000 )
    {
        _ld_bootrom_start = ABSOLUTE(.);
        *drvRIU.o           (.text*)
        *bootrom.o          (.text*)
        *bootrom_pll.o      (.text*)
        *bootrom_clock.o    (.text*)
        *bootrom_peripheral.o   (.text*)
        *bootrom_cache.o    (.text*)
        *bootrom_miu.o      (.text*)
        *bootrom_miu_str.o  (.text*)
    
    #if (CONFIG_MIU_DDR4 == 1)
        *bootrom_dqs_ddr4.o      (.text*)
    #elif (CONFIG_MIU_DDR3 == 1)
        *bootrom_dqs_ddr3.o      (.text*)
    #else
    #endif

        *bootrom.o          (.rodata*)
        *bootrom_data_miu.o (.rodata*)

        *drvPadConf.o       (.rodata*)

        . = ALIGN(16);
        _ld_bootrom_end = ABSOLUTE(.);
        _ld_bootrom_size = _ld_bootrom_end - _ld_bootrom_start;
    } > rom

    .text3 : AT (LOADADDR(.text2) + SIZEOF(.text2))
    {
        _ld_bootram_start = ABSOLUTE(.);
        *bootram.o          (.text*)
        *nandflash.o        (.text*)
        *nandflash.o        (.rodata*)
        *uart16550.o        (.text*)
        *setSPI.o           (.text*)
        *setSPI.o           (.rodata*)
        *apiBDMA.o          (.text*)
        *apiBDMA.o          (.rodata*)

        *MSDecompress.o     (.text*)
        *MSDecompress.o     (.rodata*)

        *(.ARM.exidx*)

    } > ram

    .data :
    {
        . = ALIGN(16);
        _ld_bootdata_start = ABSOLUTE(.);
        *(.data*)
        . = ALIGN(16);
        _ld_bootdata_end = ABSOLUTE(.);
        _ld_bootram_end = ABSOLUTE(.);
        _ld_bootram_size = _ld_bootram_end - _ld_bootram_start;
    } > ram

    .bss :
    {
        . = ALIGN(16);
        _ld_bootbss_start = ABSOLUTE(.);
        *(.bss*)
        . = ALIGN(16);
        _ld_bootbss_end = ABSOLUTE(.);
    } > ram

#endif

    /DISCARD/ :
    {
        /* ABI crap starts here */

        *(.reginfo)
        *(.note.gnu.build-id)
    }

#if (CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH == 1)
    .padding : AT(LOADADDR(.text1) + 0x8000 - 4)
    {
        /* LONG(0x0) */
    } > ram
#elif (CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH == 1)
    .padding : AT(LOADADDR(.text1) + 0x8000 - 4)
    {
        /* LONG(0x0) */
    } > ram
#else
    .padding : AT(LOADADDR(.text1) + 0x10000 - 4)
    {
        LONG(0x0)
    } > ram
#endif
}
