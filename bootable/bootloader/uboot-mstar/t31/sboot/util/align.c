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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    FILE *t_fpin;
    FILE *t_fpout;
    unsigned int t_filesize = 0;
    unsigned int align_size = 0;	
	unsigned int padding_size = 0;
    unsigned char *t_buf = NULL;
	unsigned char i;
    int t_i;
	
	if (argc < 3)
	{
	    printf("[Error]please enter the correct input!\n");
        return -1;
	}

	//get align size
	align_size=strtoul(argv[2], NULL, 16);
	//printf("align_size : 0x%x\n",align_size);
   
    if(align_size == 0)
    {
        printf("[Warning] Ignore Alignment Size : 0x%x\n",align_size);
        return 0;
    }    

    t_fpin = fopen(argv[1], "rb");
    if (NULL == t_fpin)
    {
        printf("[Error]cannot open %s\n", argv[1]);
        return -1;
    }
	
	//get file size
	fseek(t_fpin, 0, SEEK_END);
	t_filesize = ftell(t_fpin);
	fseek(t_fpin, 0, SEEK_SET);
	fclose(t_fpin);
	//printf("t_filesize : 0x%x\n",t_filesize);
	
	//get padding size
	if(t_filesize > align_size)
	{
        if(t_filesize % align_size != 0)
        {    
		    padding_size = align_size - (t_filesize % align_size);
        }
        else
        {
            padding_size = 0;
        }
		//printf("padding_size = 0x%x\n",padding_size);			
	}
	else if(t_filesize == align_size)
	{
	    //unnecessary padding		
		padding_size = 0;
		//printf("padding_size = 0x%x\n",padding_size);		
		return 0;
	}
	else
	{		
		padding_size = align_size - t_filesize;
    	//printf("padding_size = 0x%x\n",padding_size);		
	}
	
	// put pending data to target file
	t_buf = (unsigned char*)malloc(padding_size);
	if (NULL == t_buf)
	{
		printf("[Error]memory allocation.\n");
		return -1;
	}
	t_fpout = fopen(argv[1], "ab");
	if (NULL == t_fpout)
	{
		printf("[Error]cannot open %s\n", argv[1]);
		return -1;
	}
	memset(t_buf,0,padding_size);
	fwrite(t_buf, sizeof(unsigned char), padding_size, t_fpout);

    free(t_buf);
    fclose(t_fpout);
    return 0;
}
