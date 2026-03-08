/**
* Copyright (c) 2006 – 2016 MStar Semiconductor, Inc.
* This program is free software. You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
int UartHandShake(void);
int UartLoadUBootSize(void);
int UartWaitRXSTop(void);
int UartLoadContent(void);
void UartGetRpmbUbootVersion(unsigned int uboot_version);
int UartCheckAntiRollbackInit(unsigned int anti_rollback_init_magic);

//-------------------------------------------------------------------------------------------------
//  Define
//-------------------------------------------------------------------------------------------------
#define CUS_UBOOT_PASSWORD_LENTH  (8)
#define CUS_UBOOT_CODE_LENTH      (4)
#define CUS_UBOOT_EA_KEY_SIZE     (0x104)
#define CUS_UBOOT_SIGN_SIZE       (0x110)
#define CUS_UBOOT_SIGN_OFFSET     (0x10)
#define CUS_UBOOT_CONTENT_OFFSET  (788)
#define CUS_UBOOT_VERSION_OFFSET  (16)
// Magic flag indicating anti-rollback fields in RPMB have been initialized
#define FLAG_ANTIROLLBACK_INITIALIZED (0x42525468)

typedef union
{
    unsigned char u8Code[CUS_UBOOT_CODE_LENTH];
    unsigned int u32Code;
} cus_uboot_header;

static unsigned char pwd[CUS_UBOOT_PASSWORD_LENTH] = {'C', 'U', 'S', 'M', 'S', 'T', 'A', 'R'};
static cus_uboot_header header[2];
static unsigned int rpmb_uboot_version = 0;
static unsigned int anti_rollback_enable = 0;

void UartGetRpmbUbootVersion(unsigned int uboot_version)
{
    rpmb_uboot_version = uboot_version;
}

int UartCheckAntiRollbackInit(unsigned int anti_rollback_init_magic)
{
    if (anti_rollback_init_magic == FLAG_ANTIROLLBACK_INITIALIZED)
    {
        anti_rollback_enable = 1;
		return 1;
    }

    anti_rollback_enable = 0;
    /* Not Init Anti-RollBack */
    return 0;
}

int UartLoadContent(void)
{
    unsigned int u32Ret = 0;
    unsigned int image_uboot_version = 0;
    unsigned char * uboot_loader = (unsigned char *)CONFIG_UBOOT_LOADADDR;

    //while (*(( volatile unsigned char* )(0x1f000c00)) != 0xAA);
    /* Enable UART RX */
    *(( volatile unsigned short* )(0x1f201308)) |= 0x0004;
    *(( volatile unsigned short* )(0x1f001c24)) |= 0x0800;

    /* Load Content image */
    while (header[0].u32Code--)
    {
        while ((*(( volatile unsigned short* )(0x1f201328)) & 0x0001) != 0x0001);
            *uboot_loader = *(( volatile unsigned char* )(0x1f201300));

        uboot_loader++;
    }

    /* Auth EA key vie MSTAR key */
    u32Ret = MDrv_AESDMA_SecureMain(CONFIG_UBOOT_LOADADDR + header[1].u32Code + CUS_UBOOT_SIGN_SIZE ,
                                    CUS_UBOOT_EA_KEY_SIZE ,
                                    CONFIG_UBOOT_LOADADDR + header[1].u32Code + CUS_UBOOT_SIGN_SIZE + CUS_UBOOT_EA_KEY_SIZE,
                                    0x0);
    if (u32Ret == 0)
        return 1;

    /* Auth Cus UBoot vie EA key */
    u32Ret = MDrv_AESDMA_SecureMain_v2(CONFIG_UBOOT_LOADADDR,
                                       header[1].u32Code,
                                       CONFIG_UBOOT_LOADADDR + header[1].u32Code + CUS_UBOOT_SIGN_OFFSET ,
                                       CONFIG_UBOOT_LOADADDR + header[1].u32Code + CUS_UBOOT_SIGN_SIZE);
    if (u32Ret == 0)
        return 1;

    if (anti_rollback_enable)
    {
        /* Uboot Version: Uboot Size - CUS_UBOOT_VERSION_OFFSET */
        image_uboot_version = *(( volatile unsigned short* )(CONFIG_UBOOT_LOADADDR + header[1].u32Code - CUS_UBOOT_VERSION_OFFSET));

        /* Compare uboot_version */
        if (image_uboot_version >= rpmb_uboot_version)
            return 0;
        else
            return 1;
    }

    /* Success */ 
    return 0;
}

int UartWaitRXSTop(void)
{

    delayus_ddr(5000000);
    return 0;
}

static unsigned int Uart_is_str(void)
{
    return ((*(volatile unsigned short*)(0x1F000000 + (0x0E70<<1))) & 0xF000) != 0xF000;
    // if STR resume then sboot will set 0xF000 in Bootrom.s
}


int UartHandShake(void)
{
    unsigned char i = 0;
    unsigned int u32Count = 0xFFFF;
    unsigned char u8result = 0;

    if(Uart_is_str()==1)
        return 1;

    /* Enable UART RX */
    *(( volatile unsigned short* )(0x1f001c24)) |= 0x0800;

    /* Wait for UART RX FIFO 32 times */
    while (i < (CUS_UBOOT_PASSWORD_LENTH * 4))
    {
        while ((*(( volatile unsigned short* )(0x1f201328)) & 0x0001) != 0x0001)
        {
            u32Count--;
            /* Hand-shake time-out */
            if (u32Count == 0)
            {
                u32Count = 0xFFFF;
                break;
            }
        }

        /* Judge Password */
        if (*(( volatile unsigned char* )(0x1f201300)) == pwd[u8result])
        {
            u8result++;
            if (u8result == CUS_UBOOT_PASSWORD_LENTH)
               break;
        }
        else
        {
            u8result = 0;
        }
        i++;
    }

    if (u8result == CUS_UBOOT_PASSWORD_LENTH)
    {
        /* Get Content Size */
        i = CUS_UBOOT_CODE_LENTH;
        while (i > 0)
        {
            while ((*(( volatile unsigned short* )(0x1f201328)) & 0x0001) != 0x0001);
            header[0].u8Code[i - 1] = *(( volatile unsigned char* )(0x1f201300));
            i--;
        }

        /* Get Uboot Size */
        i = CUS_UBOOT_CODE_LENTH;
        while (i > 0)
        {
            while ((*(( volatile unsigned short* )(0x1f201328)) & 0x0001) != 0x0001);
            header[1].u8Code[i - 1] = *(( volatile unsigned char* )(0x1f201300));
            i--;
        }

        /* Disable UART RX */
        *(( volatile unsigned short* )(0x1f201308)) |= 0x0004;
        *(( volatile unsigned short* )(0x1f001c24)) &= ~0x0800;
        *(( volatile unsigned short* )(0x1f201308)) |= 0x0004;

        if (header[0].u32Code == 0 ||
            header[1].u32Code == 0 ||
            header[0].u32Code > 0x300000 ||
            header[1].u32Code > 0x300000)
            return 1;

        if (header[0].u32Code < header[1].u32Code)
            return 1;

        if (header[0].u32Code - header[1].u32Code != CUS_UBOOT_CONTENT_OFFSET)
            return 1;

        return 0;
    }
    else
    {
         return 1;
    }
}
