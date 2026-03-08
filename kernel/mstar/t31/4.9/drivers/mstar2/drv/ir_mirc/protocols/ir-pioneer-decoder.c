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
 * Copyright(C) 2021 MediaTek Inc.
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
 * Copyright(C) 2021 MediaTek Inc.
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

/* ir-pioneer-decoder.c - handle Pioneer IR Pulse/Space protocol
 *
 * Copyright (C) 2021 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/bitrev.h>
#include <linux/module.h>
#include "../ir_core.h"
#include "../ir_common.h"

static u8 u8InitFlag_pio = FALSE;
static u32 scancode = 0;
static u32 speed = 0;
static u32 mapnum = 0;
static u32 repeatCount = 0;
static unsigned long KeyTime = 0;
static unsigned long decodeTime = 0;
static bool bFirstReceived = TRUE;
static IR_PIO_Spec_t pio;

/**
 * ir_pioneer_decode() - Decode one Pioneer pulse or space
 * @dev:    the struct rc_dev descriptor of the device
 * @duration:   the struct ir_raw_event descriptor of the pulse/space
 *
 * This function returns -EINVAL if the pulse violates the state machine
 */
static int ir_pioneer_decode(struct mstar_ir_dev *dev, struct ir_raw_data ev)
{
    IR_PIO_Spec_t *data = &pio;
    struct ir_scancode *sc = &dev->raw->this_sc;
    struct ir_scancode *prevsc = &dev->raw->prev_sc;
    u8 i = 0;
    u8 u8Addr[2] = {0, 0};
    u8 u8AddrInv[2] = {0, 0};
    u8 u8Cmd[2] = {0, 0};
    u8 u8CmdInv[2] = {0, 0};
    u8 u8Verify = 0;
    if (!(dev->enabled_protocols & (1<<IR_TYPE_PIONEER)))
        return -EINVAL;
    switch (data->eStatus)
    {

    case STATE_INACTIVE:
        if (!ev.pulse)
            break;
        if (eq_margin(ev.duration,PIO_HEADER_PULSE_LWB,PIO_HEADER_PULSE_UPB))
        {
            if((data->u8BitCount >= 32) && (data->u8BitCount < 64))
                ;
            else
                data->u8BitCount = 0;
            data->eStatus = STATE_HEADER_SPACE;
            //IRDBG_INFO("PIO_HEADER_PULSE\n");
            return 0;
        }
        else
            break;

    case STATE_HEADER_SPACE:
        if (ev.pulse)
            break;
        if ((u32)(MIRC_Get_System_Time()- decodeTime) > PIO_DATA_TIMEOUT)
        {
            data->u8BitCount = 0;
            data->u64DataBits = 0;
            repeatCount = 0;
            scancode = 0;
            bFirstReceived = TRUE;
        }
        if (eq_margin(ev.duration,PIO_HEADER_SPACE_LWB,PIO_HEADER_SPACE_UPB))
        {
            data->eStatus = STATE_BIT_PULSE;
            return 0;
        }
#if 0
        else if (eq_margin(ev.duration,PIO_REPEAT_SPACE_LWB,PIO_REPEAT_SPACE_UPB))
        {//repeat
            //IRDBG_INFO("[PIO] TIME =%ld\n",(MIRC_Get_System_Time()-KeyTime));
            if (prevsc->scancode_protocol == (1<<IR_TYPE_PIONEER) && ((u32)(MIRC_Get_System_Time()- KeyTime) <= PIO_REPEAT_TIMEOUT))
            {
                KeyTime = MIRC_Get_System_Time();
                if (((speed != 0)&&( data->u8RepeatTimes >= (speed - 1)))
                        || ((speed == 0)&&(data->u8RepeatTimes >= dev->speed)))
                {
#ifdef CONFIG_MIRC_INPUT_DEVICE
                    sc->scancode = scancode;
                    sc->scancode_protocol = (1<<IR_TYPE_PIONEER);
                    dev->map_num = mapnum;
                    dev->raw->u8RepeatFlag = 1;
#else
                    sc->scancode_protocol = (1<<IR_TYPE_PIONEER);
                    sc->scancode = scancode|0x01;//repeat
#endif
                    data->eStatus = STATE_INACTIVE;
                    data->u32DataBits = 0;
                    data->u8BitCount = 0;
                    return 1;

                }
                data->u8RepeatTimes ++;

                //IRDBG_INFO("[PIO] repeattimes =%d \n",data->u8RepeatTimes);
            }
            else
            {
                scancode = 0;
                mapnum = NUM_KEYMAP_MAX;
            }
            data->eStatus = STATE_INACTIVE;
            return 0;
        }
#endif
        break;

    case STATE_BIT_PULSE:
        if (!ev.pulse)
            break;
        if (!eq_margin(ev.duration,PIO_BIT_PULSE_LWB,PIO_BIT_PULSE_UPB))
            break;

        data->eStatus = STATE_BIT_SPACE;
        return 0;

    case STATE_BIT_SPACE:
        if (ev.pulse)
            break;
        data->u64DataBits <<= 1;

        if (eq_margin(ev.duration,PIO_BIT_1_SPACE_LWB,PIO_BIT_1_SPACE_UPB))
            data->u64DataBits |= 1;
        else if (!eq_margin(ev.duration,PIO_BIT_0_SPACE_LWB,PIO_BIT_0_SPACE_UPB))
            break;
        data->u8BitCount++;

        decodeTime = MIRC_Get_System_Time();

        if (data->u8BitCount == PIO_NBITS)
        {
            u8Addr[0]    = bitrev8((data->u64DataBits >> 56) & 0xff);
            u8AddrInv[0] = bitrev8((data->u64DataBits >> 48) & 0xff);
            u8Cmd[0]     = bitrev8((data->u64DataBits >> 40) & 0xff);
            u8CmdInv[0]  = bitrev8((data->u64DataBits >> 32) & 0xff);
            u8Addr[1]    = bitrev8((data->u64DataBits >> 24) & 0xff);
            u8AddrInv[1] = bitrev8((data->u64DataBits >> 16) & 0xff);
            u8Cmd[1]     = bitrev8((data->u64DataBits >>  8) & 0xff);
            u8CmdInv[1]  = bitrev8((data->u64DataBits >>  0) & 0xff);


            IRDBG_INFO("[PIO] u8Addr0 = %x u8AddrInv0 = %x u8Cmd0 = %x u8CmdInv0 = %x\n",u8Addr[0],u8AddrInv[0],u8Cmd[0],u8CmdInv[0]);
            IRDBG_INFO("[PIO] u8Addr1 = %x u8AddrInv1 = %x u8Cmd1 = %x u8CmdInv1 = %x\n",u8Addr[1],u8AddrInv[1],u8Cmd[1],u8CmdInv[1]);

            if (repeatCount >= 1000)
                repeatCount == 3;

            if (u8Addr[0] == u8Addr[1])
                repeatCount += 2;
            else
                repeatCount += 1;

            for (i= 0;i<dev->support_num;i++)
            {
                if(dev->support_ir[i].eIRType == IR_TYPE_PIONEER)
                {
                    if((((u8Addr[0]<<8) | u8AddrInv[0]) == dev->support_ir[i].u32HeadCode)&&(dev->support_ir[i].u8Enable == TRUE)
                       && ((bFirstReceived == TRUE)||(repeatCount >3)))
                    {
                        if ((u8Addr[0] == (u8)~u8AddrInv[0])&&(u8Addr[1] == (u8)~u8AddrInv[1]))
                        { 
                          if ((u8Cmd[0] == (u8)~u8CmdInv[0])&&(u8Cmd[1] == (u8)~u8CmdInv[1]))
                          {
#ifdef CONFIG_MIRC_INPUT_DEVICE
                            if (u8Addr[0] != u8Addr[1])
                                u8Cmd[0] += u8Cmd[1];
                            sc->scancode = (u8Addr[0]<<16) | (u8AddrInv[0]<<8) | u8Cmd[0];
                            sc->scancode_protocol = (1<<IR_TYPE_PIONEER);

                            if (scancode == sc->scancode && repeatCount > 3)
                            {
                                prevsc->scancode = scancode;
                                dev->map_num = mapnum;
                                dev->raw->u8RepeatFlag = 1;
                                data->eStatus = STATE_INACTIVE;
                                data->u64DataBits = 0;
                                data->u8BitCount = 0;
                            }
                            else
                            {
                                scancode = sc->scancode;
                                speed = dev->support_ir[i].u32IRSpeed;
                                dev->map_num = dev->support_ir[i].u32HeadCode;
                                mapnum = dev->map_num;
                                dev->raw->u8RepeatFlag = 0;
                            }
#else
                            sc->scancode = (u8Cmd<<8) |0x00;
                            if(dev->support_ir[i].u32HeadCode == 0x40DF)
                            {
                                sc->scancode |= (0x04UL << 28);
                            }
                            else
                            {
                                sc->scancode |= (0x01UL << 28);
                            }
                            sc->scancode_protocol = (1<<IR_TYPE_PIONEER;
                            scancode = sc->scancode;
                            speed = dev->support_ir[i].u32IRSpeed;
#endif
                            KeyTime = MIRC_Get_System_Time();
                            memset(data,0,sizeof(IR_PIO_Spec_t));
                            bFirstReceived = FALSE;
                            return 1;
                          }
                        }
                    }
                }
            }
        }
        else if (data->u8BitCount == 32)
            data->eStatus = STATE_INACTIVE;
        else
            data->eStatus = STATE_BIT_PULSE;

        return 0;
    default:
        break;
    }

    data->eStatus = STATE_INACTIVE;
    return -EINVAL;
}

static struct ir_decoder_handler pio_handler = {
    .protocols  = (1<<IR_TYPE_PIONEER),
    .decode     = ir_pioneer_decode,
};

int pio_decode_init(void)
{
    if(u8InitFlag_pio == FALSE)
    {
        scancode = 0;
        mapnum = 0;
        KeyTime = 0;
        memset(&pio,0,sizeof(IR_PIO_Spec_t));
        MIRC_Decoder_Register(&pio_handler);
        IR_PRINT("[IR Log] PIO Spec Protocol Init\n");
        u8InitFlag_pio = TRUE;
    }
    else
    {
        IR_PRINT("[IR Log] PIO Spec Protocol Has been Initialized\n");
    }
    return 0;
}

void pio_decode_exit(void)
{
    if(u8InitFlag_pio == TRUE)
    {
        MIRC_Decoder_UnRegister(&pio_handler);
        u8InitFlag_pio = FALSE;
    }
}
