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
#include <memory.h>

unsigned char t_buf[512], hexbuf[256];
unsigned char PublicKeyE[4];
    
// Convert binary file into HEX bytes for being included in a .c file.
int str2hex(unsigned char* pInput, unsigned char *pOutput, int *len)
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
    
    for(i=0; i< length/2; i++)
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

int main(int argc, char* argv[])
{
    FILE* t_fpin;
    FILE *t_fpout;
    int len;
    int  t_i;

    if (3 != argc) 
    {
        printf("Usage: %s <input>\n", argv[0]);
        return 0;
    }

    t_fpin = fopen(argv[1], "rb");
    if (NULL == t_fpin)
    {
        printf("[Error]cannot open %s\n", argv[1]);
        return -1;
    }

    t_fpout = fopen(argv[2], "wb");
    if (NULL == t_fpout)
    {
        printf("[Error]cannot open %s\n", argv[2]);
        return -1;
    }

    fseek(t_fpin,(33*2),SEEK_SET);//skip 33 bytes header
    memset(t_buf, 0, sizeof(unsigned char)*512);
    fread(t_buf, 1, 256*2, t_fpin);
    str2hex(t_buf, hexbuf, &len);
    //write public_key_N(256 byte) into file
    fwrite(hexbuf, sizeof(unsigned char),256,t_fpout);

    //skip 33+256 bytes to public_key_e header position
    fseek(t_fpin,((33*2)+(256*2)),SEEK_SET);
    memset(t_buf, 0, sizeof(unsigned char)*512);
    fread(t_buf, 1, 2, t_fpin);
    str2hex(t_buf, hexbuf, &len);
    if (hexbuf[0] == 0x02)
    {
        fread(t_buf, 1, 2, t_fpin);
        str2hex(t_buf, hexbuf, &len);
        if (hexbuf[0] == 0x01)
        {
            fread(t_buf, 1, 2, t_fpin);
            str2hex(t_buf, &PublicKeyE[3], &len);
            PublicKeyE[0] = PublicKeyE[1] = PublicKeyE[2] = 0x00;
            //write public_key_E(4 byte) into file
            fwrite(PublicKeyE, sizeof(unsigned char), 4, t_fpout);
        }
    }
    fclose(t_fpin);
    fclose(t_fpout);
    printf("ExtractPublicKey Done !!\n");
    return 0;
}
