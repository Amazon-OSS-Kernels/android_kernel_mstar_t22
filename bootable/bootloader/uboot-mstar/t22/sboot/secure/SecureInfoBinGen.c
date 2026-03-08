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

/* arguments:   "MBoot file path" "MBoot address" "sensitive file path" "Sensitive address"
                "str_rsa_loader_key" "str_signature_root" "str_signature_loader" "str_signature_fw"
*/
#include <stdio.h>
#include <string.h>
#include <memory.h>

unsigned char t_buf[512];
unsigned char t_buf_chunk[1024];

#define SIGNATURE_LEN	256

typedef struct
{
    unsigned char RSA_loader_public_key_n[256];
    unsigned char RSA_loader_public_key_e[4];
    unsigned char signature_loader[SIGNATURE_LEN];
}SECURITY_INFO;

int str2hex(char* pInput, unsigned char* pOutput, int* len)
{
    char c1, c2;
    int i, length;
    
    length = strlen(pInput);
    if(length%2)
    {
        return 0;
    }
    
    for(i=0; i<length; i++)
    {
        pInput[i] = toupper(pInput[i]);
    }
    
    for(i = 0; i< strlen(pInput)/2; i++)
    {
        c1 = pInput[2*i];
        c2 = pInput[2*i+1];
        if(c1<'0' || (c1 > '9' && c1 <'A') || c1 > 'F')
        {
            continue;
        }
        if(c2<'0' || (c2 > '9' && c2 <'A') || c2 > 'F')
        {
            continue;
        }
        c1 = c1>'9' ? c1-'A'+10 : c1 -'0';
        c2 = c2>'9' ? c2-'A'+10 : c2 -'0';
        pOutput[i] = c1<<4 | c2;
    }
    *len = i;
    return 1;
}

int main(int argc, char *argv[])
{
    FILE	*t_fpin;
    FILE 	*t_fpout;
    SECURITY_INFO sec;
    
    int len;
    int t_filesize = 0;
    
    if (4!=argc)
    {
        printf("[usage]:\n");
        printf("%s ChunkHeader RsaPublicLoaderKey SignatureLoader \n", argv[0]);
        return -1;
    }

		/* init security structure */
    memset(&sec, 0, sizeof(SECURITY_INFO));
    
    /* set Chunk Header start address and size */
    t_fpin = fopen(argv[1], "rb");
    if (NULL == t_fpin)
    {
        printf("[Error]cannot open %s\n", argv[1]);
        return -1;
    }
    
    memset(t_buf_chunk, 0, sizeof(unsigned char)*sizeof(t_buf_chunk));
    fread(t_buf_chunk, sizeof(unsigned char), sizeof(t_buf_chunk), t_fpin);
    
    fclose(t_fpin);

		//start to parse Public_Key into PubKey_E and PubKey_N
		t_fpin = fopen(argv[2],"rb");
		if (NULL == t_fpin)
    {
        printf("[Error]cannot open %s\n", argv[2]);
        return -1;
    }
		fseek(t_fpin,(33*2),SEEK_SET);//skip 33 bytes header

		//Public_Key_N
		memset(t_buf, 0, sizeof(unsigned char)*512);
		fread(t_buf, sizeof(char), 256*2, t_fpin);
		str2hex(t_buf, sec.RSA_loader_public_key_n, &len);
	
		fseek(t_fpin,((33*2)+(256*2)),SEEK_SET);//33+256 bytes header
		
		//Public_Key_E
    memset(t_buf, 0, sizeof(unsigned char)*512);
    fread(t_buf,1,2,t_fpin);
		str2hex(t_buf, &t_buf[2], &len);
		if (t_buf[2]==0x02) // 2 byte
		{
        memset(t_buf, 0, sizeof(unsigned char)*512);
	 		  fread(t_buf,1,2,t_fpin);
			  str2hex(t_buf, &t_buf[2], &len);
			  if(t_buf[2]==0x01)
			  {
	          // KEY_E: {0x00,0x00,0x00,0x11}
	          memset(t_buf, 0, sizeof(unsigned char)*512);
	 			    fread(t_buf,1,2,t_fpin);
				    str2hex(t_buf, &sec.RSA_loader_public_key_e[3], &len);
				    sec.RSA_loader_public_key_e[0]=sec.RSA_loader_public_key_e[1]=sec.RSA_loader_public_key_e[2]=0x00;
        }
    }
		fclose(t_fpin);

    /* get the signature_loader */
    t_fpin = fopen(argv[3], "rb");
    if (NULL == t_fpin)
    {
        printf("[Error]cannot open %s\n", argv[3]);
        return -1;
    }
    
    fseek(t_fpin, 0, SEEK_END);
    if (512 != ftell(t_fpin))
    {
        printf("[Error]the size of Signature_loader is not equal to 256 bytes\n");
        return -1;
    }
    
    fseek(t_fpin, 0, SEEK_SET);
    memset(t_buf, 0, sizeof(unsigned char)*512);
    fread(t_buf, 1, 256*2, t_fpin);
    str2hex(t_buf, sec.signature_loader, &len);
    fclose(t_fpin);

		memcpy((void *)&t_buf_chunk[128], (void *)&sec, sizeof(sec));
		
		/* set Chunk Header start address and size */
    t_fpout = fopen(argv[1], "wb");
    if (NULL == t_fpout)
    {
        printf("[Error]cannot open %s\n", argv[1]);
        return -1;
    }
    
		fwrite(t_buf_chunk, 1, sizeof(t_buf_chunk), t_fpout);
		fclose(t_fpout);

    //printf("SecureInfoBinGen Done !!\n");
    return 0;
}
