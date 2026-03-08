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
#define MAPP_IR_C

//***********************************************************
//              Header Files
//***********************************************************
#include "datatype.h"
#include "msIR.h"
#include "msKeypad.h"
#include "MApp_IR.h"
#include "MApp_Key.h"
#include "MApp_Wakeup.h"

//***********************************************************
//              Macro Definitions
//***********************************************************
#define KEY_DEBUG(y)            y
#define KEY_NULL                0xFF

//***********************************************************
//              External Veriables
//***********************************************************


//***********************************************************
//              Local Veriables
//***********************************************************
static KEYSTAT stKeyStatus;

//***********************************************************
//              Global Veriables
//***********************************************************


//***********************************************************
//              External Functions
//***********************************************************
extern void SystemWholeChipReset(void);
extern BOOL MDrv_IR_IsKeypadWakeup(void);


//***********************************************************
//              Local Functions Definition
//***********************************************************
static void MApp_CheckKeyStatus(void)
{
    U8 key = NULL;
    U8 KeyRepeatStatus = NULL;

    if (msIR_GetIRKeyCode(&key, &KeyRepeatStatus) == MSRET_OK)
    {
        stKeyStatus.keytype = KEY_TYPE_IR;
        stKeyStatus.keydown = TRUE;
        stKeyStatus.keydata = key;
        stKeyStatus.keyrepeat = KeyRepeatStatus;
        KEY_DEBUG(printf("[IR]: Key = 0x%02X, Repeat = %d\r\n", key, KeyRepeatStatus));
    }
  #if (KEYPAD_TYPE_SEL != KEYPAD_TYPE_NONE)
    else if (MDrv_IR_IsKeypadWakeup() && msKeypad_GetKey(&key, &KeyRepeatStatus)== MSRET_OK)
    {
        stKeyStatus.keytype = KEY_TYPE_KEYPAD;
        stKeyStatus.keydown = TRUE;
        stKeyStatus.keydata = key;
        stKeyStatus.keyrepeat = KeyRepeatStatus;
        KEY_DEBUG(printf("[Keypad]: Key = 0x%02X, Repeat = %d \r\n", key, KeyRepeatStatus));

    }
  #endif
    else
    {
        stKeyStatus.keydown = FALSE;
        stKeyStatus.keydata = KEY_NULL;
        stKeyStatus.keyrepeat = FALSE;
    }

}

static void MApp_ParseKey(void)
{
    //check IR wakeup key
    if(stKeyStatus.keytype == KEY_TYPE_IR)
    {
        // Check if it is wakeup key
        if(MDrv_IR_IsWakeup(stKeyStatus.keydata))
        {
            //force it to power key
            u8KeyCode = KEY_WAKEUP;
            return;
        }
    }


    switch ( stKeyStatus.keydata )
    {
        case IRKEY_POWER:               u8KeyCode = KEY_POWER;              break;
        case IRKEY_UP:                  u8KeyCode = KEY_UP;                 break;
        case IRKEY_DOWN:                u8KeyCode = KEY_DOWN;               break;
        case IRKEY_RIGHT:               u8KeyCode = KEY_RIGHT;              break;
        case IRKEY_LEFT:                u8KeyCode = KEY_LEFT;               break;
        case IRKEY_SELECT:              u8KeyCode = KEY_SELECT;             break;

        case IRKEY_CHANNEL_PLUS:
            if ((stKeyStatus.keytype == KEY_TYPE_KEYPAD) ) //ZUI_TODO: &&
                u8KeyCode = KEY_UP;
            else
                u8KeyCode = KEY_CHANNEL_PLUS;
            break;

        case IRKEY_CHANNEL_MINUS:
            if ((stKeyStatus.keytype == KEY_TYPE_KEYPAD) )//ZUI_TODO: &&
                u8KeyCode = KEY_DOWN;
            else
                u8KeyCode = KEY_CHANNEL_MINUS;
            break;

        case IRKEY_VOLUME_PLUS:
            // special case : when in input select mode, the KEYPAD LEFT/RIGHT is interpreted as VolUP/VolDOWN
            if ((stKeyStatus.keytype == KEY_TYPE_KEYPAD) )//ZUI_TODO: &&
                u8KeyCode = KEY_VOLUME_PLUS;
            else if((stKeyStatus.keytype == KEY_TYPE_KEYPAD) )//ZUI_TODO: &&
                u8KeyCode = KEY_SELECT;
            else if((stKeyStatus.keytype == KEY_TYPE_KEYPAD) )//ZUI_TODO: &&
                u8KeyCode = KEY_RIGHT;
            else
                u8KeyCode = KEY_VOLUME_PLUS;
            break;

        case IRKEY_VOLUME_MINUS:
            if ((stKeyStatus.keytype == KEY_TYPE_KEYPAD) )//ZUI_TODO: &&
                u8KeyCode = KEY_VOLUME_MINUS;
            else if((stKeyStatus.keytype == KEY_TYPE_KEYPAD) )//ZUI_TODO: &&
                u8KeyCode = KEY_SELECT;
            else if((stKeyStatus.keytype == KEY_TYPE_KEYPAD) )//ZUI_TODO: &&
                u8KeyCode = KEY_LEFT;
            else
                u8KeyCode = KEY_VOLUME_MINUS;
            break;

        case IRKEY_MENU:                u8KeyCode = KEY_MENU;               break;
        case IRKEY_INPUT_SOURCE:        u8KeyCode = KEY_INPUT_SOURCE;       break;

        case IRKEY_NUM_0:               u8KeyCode = KEY_0;                  break;
        case IRKEY_NUM_1:               u8KeyCode = KEY_1;                  break;
        case IRKEY_NUM_2:               u8KeyCode = KEY_2;                  break;
        case IRKEY_NUM_3:               u8KeyCode = KEY_3;                  break;
        case IRKEY_NUM_4:               u8KeyCode = KEY_4;                  break;
        case IRKEY_NUM_5:               u8KeyCode = KEY_5;                  break;
        case IRKEY_NUM_6:               u8KeyCode = KEY_6;                  break;
        case IRKEY_NUM_7:               u8KeyCode = KEY_7;                  break;
        case IRKEY_NUM_8:               u8KeyCode = KEY_8;                  break;
        case IRKEY_NUM_9:               u8KeyCode = KEY_9;                  break;

        case IRKEY_EXIT:                u8KeyCode = KEY_EXIT;               break;
        case IRKEY_EPG:                 u8KeyCode = KEY_EPG;                break;
        case IRKEY_CHANNEL_RETURN:      u8KeyCode = KEY_CHANNEL_RETURN;     break;
        case IRKEY_BACK:                u8KeyCode = KEY_BACK;               break;
        case IRKEY_CHANNEL_FAV_LIST:    u8KeyCode = KEY_CHANNEL_FAV_LIST;   break;
        case IRKEY_MUTE:                u8KeyCode = KEY_MUTE;               break;
        case IRKEY_FREEZE:              u8KeyCode = KEY_FREEZE;             break;
        case IRKEY_INFO:                u8KeyCode = KEY_INFO;               break;
        case IRKEY_AUDIO:               u8KeyCode = KEY_AUDIO;              break;
        case IRKEY_MTS:                 u8KeyCode = KEY_MTS;                break;
        case IRKEY_ZOOM:                u8KeyCode = KEY_ZOOM;               break;
        case IRKEY_SUBTITLE:            u8KeyCode = KEY_SUBTITLE;           break;
        case IRKEY_UPDATE:              u8KeyCode = KEY_UPDATE;             break;        // add UPDATE key
        case IRKEY_TTX_MODE:            u8KeyCode = KEY_TTX_MODE;           break;
        case IRKEY_MIX:                 u8KeyCode = KEY_MIX;                break;
        case IRKEY_TTX:                 u8KeyCode = KEY_TTX;                break;
        case IRKEY_SIZE:                u8KeyCode = KEY_SIZE;               break;
        case IRKEY_HOLD:                u8KeyCode = KEY_HOLD;               break;
        case IRKEY_INDEX:               u8KeyCode = KEY_INDEX;              break;
        case IRKEY_CHANNEL_LIST:        u8KeyCode = KEY_CHANNEL_LIST;       break;
        case IRKEY_SLEEP:               u8KeyCode = KEY_SLEEP;              break;
        case IRKEY_DASH:                u8KeyCode = KEY_DASH;               break;
        //Because some IR key codes are defined un-correctly, so we have to change key mapping below.
        case IRKEY_BACKWARD:            u8KeyCode = KEY_PREVIOUS;           break;
        case IRKEY_FORWARD:             u8KeyCode = KEY_NEXT;               break;
        case IRKEY_PAGE_DOWN:           u8KeyCode = KEY_PAGE_DOWN;          break;
        case IRKEY_PREVIOUS:            u8KeyCode = KEY_FF;                 break;
        case IRKEY_NEXT:                u8KeyCode = KEY_REWIND;             break;
        case IRKEY_CC:                  u8KeyCode = KEY_CC;                 break;
        case IRKEY_ADJUST:              u8KeyCode = KEY_ADJUST;             break;
        case IRKEY_KEY_DISABLE_KEYPAD:  u8KeyCode = KEY_DISABLE_KEYPAD;     break;
        case IRKEY_REVEAL:              u8KeyCode = KEY_REVEAL;             break;
        case IRKEY_RECORD:              u8KeyCode = KEY_RECORD;             break;
        case IRKEY_STOP:                u8KeyCode = KEY_STOP;               break;
        case IRKEY_PLAY:                u8KeyCode = KEY_PLAY;               break;
        case IRKEY_PAUSE:               u8KeyCode = KEY_PAUSE;              break;
        case IRKEY_SUBPAGE:             u8KeyCode = KEY_SUBPAGE;            break;
        case IRKEY_RED:                 u8KeyCode = KEY_RED;                break;
        case IRKEY_GREEN:               u8KeyCode = KEY_GREEN;              break;
        case IRKEY_YELLOW:              u8KeyCode = KEY_YELLOW;             break;
        case IRKEY_BLUE:                u8KeyCode = KEY_BLUE;               break;
        case IRKEY_RED2:                u8KeyCode = KEY_RED;                break;
        case IRKEY_GREEN2:              u8KeyCode = KEY_GREEN;              break;
        case IRKEY_TV_INPUT:            u8KeyCode = KEY_TV_INPUT;           break;
        case IRKEY_DTV:                 u8KeyCode = KEY_DTV;                break;
        case IRKEY_TV:                  u8KeyCode = KEY_TV;                 break;
        case IRKEY_PC:                  u8KeyCode = KEY_PC;                 break;
        case IRKEY_COMPONENT:           u8KeyCode = KEY_COMPONENT;          break;
        case IRKEY_SV:                  u8KeyCode = KEY_SV;                 break;

    #if (IR_MODE_SEL != IR_TYPE_SWDECODE_SHA_MODE)
        case IRKEY_HOME:                u8KeyCode = KEY_HOME;               break;
        case IRKEY_PICTURE:             u8KeyCode = KEY_PICTURE;            break;
        case IRKEY_CLOCK:               u8KeyCode = KEY_CLOCK;              break;
        case IRKEY_SCART:               u8KeyCode = KEY_SCART;              break;
        case IRKEY_HDMI:                u8KeyCode = KEY_HDMI;               break;
        case IRKEY_AV:                  u8KeyCode = KEY_AV;                 break;
    #endif

        default:                        u8KeyCode = KEY_NULL;               break;
    }

}


//***********************************************************
//              Global Functions Definition
//***********************************************************
void MApp_ProcessUserInput(void)
{
    MApp_CheckKeyStatus();

    if ( stKeyStatus.keydown )
    {
        MApp_ParseKey();
    }

    switch (u8KeyCode)
    {
        case KEY_POWER:
            if ( stKeyStatus.keytype == KEY_TYPE_KEYPAD )
            {
                msAPI_Power_NotifyWakeupDeviceForAP(WakeUp_by_SAR);
                SystemWholeChipReset();
            }
            break;
        case KEY_WAKEUP:
            if ( stKeyStatus.keytype == KEY_TYPE_IR )
            {
                msAPI_Power_NotifyWakeupDeviceForAP(WakeUp_by_IR);
                SystemWholeChipReset();
            }
            break;
        default:
            break;

    }
    u8KeyCode = KEY_NULL;
}

//#############
//
//  IR, Keypad
//
//#############
U8 MApp_GetKeyCode(void)
{
    return u8KeyCode;
}

BOOLEAN MApp_IsKeyRepeat(void)
{
    return (stKeyStatus.keyrepeat) ? TRUE : FALSE;
}

void MApp_IR_Init(void)
{
//    printf(" Initialize IR \r\n");
#if 1
     msIR_Initialize_Isr();
#else
    msIR_Initialize(12);
#endif
    msIR_ProcessUserKey(TRUE);
}

void MApp_Keypad_Init(void)
{
//    printf(" Initialize Keypad \r\n");

    msKeypad_Init();
}



#undef MAPP_IR_C

