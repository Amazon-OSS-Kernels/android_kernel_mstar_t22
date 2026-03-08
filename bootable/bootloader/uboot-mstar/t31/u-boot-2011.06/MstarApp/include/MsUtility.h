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

#ifndef __CMD_MS_Utility__
#define __CMD_MS_Utility__
#include <command.h>
#include <ShareType.h>

struct smc_param {
	U32 a0;
	U32 a1;
	U32 a2;
	U32 a3;
	U32 a4;
	U32 a5;
	U32 a6;
	U32 a7;
};

int do_mscompress7 (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_mdelay(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
int do_showVersion(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

#if ((CONFIG_TV_CHIP == 1) || (ENABLE_RESERVED_CHUNK_HEADER == 1))
int do_showBoard(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif

#if defined(CONFIG_MMC)
int get_mmc_part_number(const char* partition, int *partNum);
#endif

//-------------------------------------------------------------------------------------------------
/// Check TEE binary before programming it
//-------------------------------------------------------------------------------------------------
#if defined(CONFIG_MSTAR_TOOL_CHECK_TEE)
int do_checkTEE(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
#endif

#if ((ENABLE_MODULE_USB == 1)&&(ENABLE_MODULE_FAT==1))
int do_spi2usb ( cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);
int do_usb2spi ( cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);
#endif

#if ((ENABLE_MODULE_USB == 1)&&(ENABLE_MODULE_EEPROM==1))
int do_usb2eeprom ( cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);
int do_eeprom2usb ( cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);
#endif

#if defined(CONFIG_MSTAR_TOOL_ROM_PROGRAM_NAND_BIN) && defined (CONFIG_MSTAR_TOOL_PROGRAM)
int do_nandprogramforrom ( cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);
#endif

#if defined(CONFIG_MSTAR_TOOL_ROM_PROGRAM_NAND_BIN) && defined (CONFIG_MSTAR_TOOL_CMDLINE)
int do_skipnandprogramforrom ( cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[]);
#endif

#if (ENABLE_UTEST == 1)
int appInitUsbDisk_Single(char idx);
#endif

//-------------------------------------------------------------------------------------------------
/// Jump to uboot's console from anywhere
//-------------------------------------------------------------------------------------------------
void jump_to_console(void);


#if (ENABLE_MODULE_USB == 1)
//-------------------------------------------------------------------------------------------------
/// Init usb storage in application layer
/// @return  int                              \b OUT: 0: successful. -1: fail
//-------------------------------------------------------------------------------------------------
int appInitUsbDisk(void);
#endif

//-------------------------------------------------------------------------------------------------
/// get next line in script, and the input data will be modified in this function
/// @return  char*                            \b OUT: a string in the script
//-------------------------------------------------------------------------------------------------
char *get_script_next_line(char **line_buf_ptr);

//-------------------------------------------------------------------------------------------------
/// load entire script to return buffer.
/// @return  char*                            \b OUT: script content.
/// remember free the memory when use vfsfile_loadscript load some script
/// ex:  char *script = vfsfile_loadscript("/config/set_env");
///      free(script);
//-------------------------------------------------------------------------------------------------
char* loadscript(char *filedir,U32 *filesize);

//-------------------------------------------------------------------------------------------------
/// load entire script to "get next line in script"
/// @return  void                            \b OUT: NULL
//-------------------------------------------------------------------------------------------------
void runscript_linebyline(char *scriptdir);


//-------------------------------------------------------------------------------------------------
/// searc the specific pattern in a specific buffer
/// @param buf                                  \b IN: buffer addres
/// @param bus_size                           \b IN: The size of input buffer
/// @param pattern                             \b IN: pattern that you want to find out
/// @return char *                              \b OUT: NULL:FAIL, non-null:Sucess
//-------------------------------------------------------------------------------------------------
char *pattern_search(char *buf, unsigned int buf_size, char *pattern);

void tee_smc_call(struct smc_param *param);

// temp for usb secure upgrade used
void Reset_System(void);

#if defined(CONFIG_MCU_ARM)
void get_temp_used_addr(unsigned int *temp_addr);
#endif

//-------------------------------------------------------------------------------------------------
/// check if RPMB is programed
//-------------------------------------------------------------------------------------------------
int anti_rb_enabled(void);

//-------------------------------------------------------------------------------------------------
/// check if the device is in lockdown
//-------------------------------------------------------------------------------------------------
int is_lockdown(void);


//-------------------------------------------------------------------------------------------------
/// check if a command is supported in lockdown
//-------------------------------------------------------------------------------------------------
int chk_cmd_lockdown(const char* command);

#endif

