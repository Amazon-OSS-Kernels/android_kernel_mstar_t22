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

#if (ENABLE_NON_OS)

#if defined(__aeon__)
U32 __loader_2nd Loader_MDrv_Flash_Get_IOAddr(void);
U32 __loader_2nd Loader_MDrv_Flash_Get_IOAddr(void)
{
    U32 u32FlashIOAddr = msRead2Bytes(0x1002AC);

    u32FlashIOAddr <<= 16;

    return u32FlashIOAddr;
}
#endif

#if defined(__aeon__)
static U32 s_u32FlashDataPortBase = 0;
#endif

#if ( defined(__aeon__)||defined(__mips__)||(defined(__arm__) || defined(__arm64__)) )
U8 __loader_2nd Ld_MDrv_Flash_ReadU8(U32 u32FlashAddr);
U8 __loader_2nd Ld_MDrv_Flash_ReadU8(U32 u32FlashAddr)
{
#if( defined(__mips__)||(defined(__arm__) || defined(__arm64__)))
    U32 u32FlashIOBase;

    if( u32FlashAddr >= FLASH_SIZE_MAX )
    {
        return 0;
    }

    if( u32FlashAddr < 0x400000 )
    {
        u32FlashIOBase = MIPS_FLASH_IO_BASE_1;
    }
    else
    {
        u32FlashIOBase = MIPS_FLASH_IO_BASE_2;
    }

    return *((U8*)(u32FlashIOBase+u32FlashAddr));

#else

    if( u32FlashAddr >= FLASH_SIZE_MAX )
    {
        return 0;
    }

    if( s_u32FlashDataPortBase == 0 )
    {
        s_u32FlashDataPortBase = Loader_MDrv_Flash_Get_IOAddr();//RIU[0x1002AC];
    }

    return *((U8*)(s_u32FlashDataPortBase+u32FlashAddr));
#endif
}

U16 __loader_2nd Ld_MDrv_Flash_ReadU16_BE(U32 u32FlashAddr);
U16 __loader_2nd Ld_MDrv_Flash_ReadU16_BE(U32 u32FlashAddr)
{
    U16 u16Result = 0;

    if( u32FlashAddr >= FLASH_SIZE_MAX )
    {
        return 0;
    }

    u16Result = Ld_MDrv_Flash_ReadU8(u32FlashAddr) << 8;
    u16Result += Ld_MDrv_Flash_ReadU8(u32FlashAddr+1);

    return u16Result;
}

U32 __loader_2nd Ld_MDrv_Flash_ReadU32_BE(U32 u32FlashAddr);
U32 __loader_2nd Ld_MDrv_Flash_ReadU32_BE(U32 u32FlashAddr)
{
    U32 u32Result = 0;


    if( u32FlashAddr >= FLASH_SIZE_MAX )
    {
        return 0;
    }

    u32Result = Ld_MDrv_Flash_ReadU8(u32FlashAddr);
    u32Result <<= 8;

    u32Result += Ld_MDrv_Flash_ReadU8(u32FlashAddr+1);
    u32Result <<= 8;

    u32Result += Ld_MDrv_Flash_ReadU8(u32FlashAddr+2);
    u32Result <<= 8;

    u32Result += Ld_MDrv_Flash_ReadU8(u32FlashAddr+3);

    return u32Result;
}

U32 __loader_2nd Ld_MDrv_Flash_ReadU32(U32 u32FlashAddr);
U32 __loader_2nd Ld_MDrv_Flash_ReadU32(U32 u32FlashAddr)
{

    U32 u32FlashIOBase;
    U32 u32Tmp = 0;

    if( u32FlashAddr >= FLASH_SIZE_MAX )
    {
        return 0;
    }
#if( defined(__mips__)||(defined(__arm__) || defined(__arm64__)))

    if( u32FlashAddr < 0x400000 )
    {
        u32FlashIOBase = MIPS_FLASH_IO_BASE_1;
    }
    else
    {
        u32FlashIOBase = MIPS_FLASH_IO_BASE_2;
    }
#else

    if( s_u32FlashDataPortBase == 0 )
    {
        s_u32FlashDataPortBase = Loader_MDrv_Flash_Get_IOAddr();//RIU[0x1002AC];
    }
    u32FlashIOBase = s_u32FlashDataPortBase;
#endif


    if( ((u32FlashIOBase + u32FlashAddr) & 0x3) == 0 ) // Align to 4
    {
        u32Tmp = *((U32*)(u32FlashIOBase+u32FlashAddr));
    }
    else
    {
        u32Tmp = *((U8*)(u32FlashIOBase+u32FlashAddr + 3));
        u32Tmp <<= 8;
        u32Tmp |= *((U8*)(u32FlashIOBase+u32FlashAddr + 2));
        u32Tmp <<= 8;
        u32Tmp |= *((U8*)(u32FlashIOBase+u32FlashAddr + 1));
        u32Tmp <<= 8;
        u32Tmp |= *((U8*)(u32FlashIOBase+u32FlashAddr + 0));
    }

    return u32Tmp;
}

#endif

// Test by using Whisky
U32 __loader_2nd Loader_MDrv_PIU_Get_Time(void);
U32 __loader_2nd Loader_MDrv_PIU_Get_Time(void)
{
    U32 u32PiuTick = msRead2Bytes(0x3048);
    u32PiuTick |= (msRead2Bytes(0x304a) << 16);

    //U32 u32PiuTime = u32PiuTick / 12000ul;  //ms
    //return u32PiuTime;
    return u32PiuTick / 12000ul;
}

U32 __loader_2nd Ld_MDrv_PIU_Get_TimeUs(void);
U32 __loader_2nd Ld_MDrv_PIU_Get_TimeUs(void)
{
    U32 u32PiuTick = msRead2Bytes(0x3048);
    u32PiuTick |= (msRead2Bytes(0x304a) << 16);
    return u32PiuTick / 12;
}

#define BDMA_REG(addr)  (BDMA_REG_BASE + (addr))
void __loader_2nd Loader2_MDrv_BDMA_CopyFlash2Dram(U32 u32FlashAddr, U32 u32DramAddr, U32 u32Size, BOOL bWaitDone);
void __loader_2nd Loader2_MDrv_BDMA_CopyFlash2Dram(U32 u32FlashAddr, U32 u32DramAddr, U32 u32Size, BOOL bWaitDone)
{
#if 0//( defined(__arm__) )

    if( bWaitDone )
        __BDMA_FlashCopy2MIU0(u32FlashAddr, u32DramAddr, u32Size);
    else
        __BDMA_FlashCopy2MIU0_No_Wait(u32FlashAddr, u32DramAddr, u32Size);

#else
    // Wait HW done
    //while (RIU8[(BDMA_REG_BASE + 0x02)*2] & 0x01);
    while( msReadByte( BDMA_REG(0x02) ) & 0x01 )
    {
    }

    // Source addr
    //RIU[(BDMA_REG_BASE + 0x08 )] = (U16)(u32FlashAddr);
    msWrite2Bytes( BDMA_REG(0x08), (u32FlashAddr) );
    //RIU[(BDMA_REG_BASE + 0x0A )] = (U16)(u32FlashAddr >> 16);
    msWrite2Bytes( BDMA_REG(0x0A), (u32FlashAddr >> 16) );

    // Dest addr
    //RIU[(BDMA_REG_BASE + 0x0C )] = (U16)(u32DramAddr);
    msWrite2Bytes( BDMA_REG(0x0C), (u32DramAddr) );
    //RIU[(BDMA_REG_BASE + 0x0E )] = (U16)(u32DramAddr >> 16);
    msWrite2Bytes( BDMA_REG(0x0E), (u32DramAddr >> 16) );


    // align to 16, Need ?
#if(ENABLE_MSTAR_MACAW12)
    u32Size = (u32Size + 0x0f ) & (~ 0x0ful);
#else
    // No need alignment
#endif

    // Set Size
    //RIU[(BDMA_REG_BASE + 0x10 )] = (U16)u32Size;
    msWrite2Bytes( BDMA_REG(0x10), (u32Size) );
    //RIU[(BDMA_REG_BASE + 0x12 )] = (U16)( u32Size >> 16 );
    msWrite2Bytes( BDMA_REG(0x12), (u32Size >> 16) );


    // Set increasing?
    //RIU8[(BDMA_REG_BASE + 0x06)*2] = 0x00;   //increasing
    msWriteByte( BDMA_REG(0x06), 0x00);

    // Set src/dst sel
#if (ENABLE_MSTAR_EULER || ENABLE_MSTAR_MAYA || ENABLE_MSTAR_WHISKY)
    //RIU[(BDMA_REG_BASE+0x04)] = 0x4045;  //((DstDEv|DstDataWidth)  <<8) |(SrcDataWidth|SrcDEv) ;
    msWrite2Bytes( BDMA_REG(0x04), 0x4045 );
#else
    //RIU[(BDMA_REG_BASE+0x04)] = 0x3035;  //((DstDEv|DstDataWidth)  <<8) |(SrcDataWidth|SrcDEv) ;
    msWrite2Bytes( BDMA_REG(0x04), 0x3035 );
#endif

    // Trigger
    //RIU8[ (BDMA_REG_BASE*2)] = 0x01 ;
    msWriteByte( BDMA_REG(0x00), 0x01);

    // Wait done
    if( bWaitDone )
    {
        while( 1 )
        {
            //if ( (RIU8[(BDMA_REG_BASE+0x02)*2] & 0x08) == 0x08 )
            if( msReadByte(BDMA_REG(0x02)) & 0x08 )
                break;
        }
    }
#endif
}

BOOL __loader_2nd Loader2_MDrv_BDMA_Is_JobDone(void);
BOOL __loader_2nd Loader2_MDrv_BDMA_Is_JobDone(void)
{
#if 0//( defined(__arm__) )

    return FALSE;

#else

    if( msReadByte(BDMA_REG(0x02)) & 0x08 )
        return TRUE;

    return FALSE;
#endif
}


#define BDMA_CRC_TYPE_FLASH 0
#define BDMA_CRC_TYPE_DRAM  1

U32 __loader_2nd Loader2_MDrv_BDMA_CalCRC(U32 u32DestAddr, U32 u32Size, U8 u8DestType);
U32 __loader_2nd Loader2_MDrv_BDMA_CalCRC(U32 u32DestAddr, U32 u32Size, U8 u8DestType)
{
#if 0//( defined(__arm__) )

    return 0x1234;

#else

    U32 u32CrcVal = 0;


    // Wait HW done
    //while (RIU8[(BDMA_REG_BASE + 0x02)*2] & 0x01);
    while( msReadByte( BDMA_REG(0x02) ) & 0x01 )
    {
    }


    // Source addr
    //RIU[(BDMA_REG_BASE + 0x08 )] = (U16)(u32DestAddr&0xffff);
    msWrite2Bytes( BDMA_REG(0x08), (u32DestAddr&0xFFFF) );
    //RIU[(BDMA_REG_BASE + 0x0A )] = (U16)(u32DestAddr >> 16);
    msWrite2Bytes( BDMA_REG(0x0A), (u32DestAddr >> 16) );

    // Dest addr
    //RIU[(BDMA_REG_BASE + 0x0C )] = 0;
    //RIU[(BDMA_REG_BASE + 0x0E )] = 0;
    msWrite2Bytes( BDMA_REG(0x0C), 0 );
    msWrite2Bytes( BDMA_REG(0x0E), 0 );

    // Set Size
    //RIU[(BDMA_REG_BASE + 0x10 )] = (U16)(u32Size&0xFFFF);
    msWrite2Bytes( BDMA_REG(0x10), (u32Size&0xFFFF) );
    //RIU[(BDMA_REG_BASE + 0x12 )] = (U16)(u32Size >> 16);
    msWrite2Bytes( BDMA_REG(0x12), (u32Size >> 16) );


    //polynomial
    //RIU[(BDMA_REG_BASE + 0x14 )] = (U16)(0x1DB7);
    msWrite2Bytes( BDMA_REG(0x14), 0x1DB7 );
    //RIU[(BDMA_REG_BASE + 0x16 )] = (U16)(0x04C1);
    msWrite2Bytes( BDMA_REG(0x16), 0x04C1 );

    //seed
    //RIU[(BDMA_REG_BASE + 0x18 )] = 0;
    msWrite2Bytes( BDMA_REG(0x18), 0 );
    //RIU[(BDMA_REG_BASE + 0x1a )] = 0;
    msWrite2Bytes( BDMA_REG(0x1A), 0 );

    // Set increasing?
    //RIU8[(BDMA_REG_BASE + 0x06)*2] = 0x00;   //increasing
    msWriteByte( BDMA_REG(0x06), 0x00);


    if( u8DestType == BDMA_CRC_TYPE_FLASH ) //Calculate CRC32 from flash
    {
        // Set src/dst sel
    #if (ENABLE_MSTAR_EULER || ENABLE_MSTAR_MAYA || ENABLE_MSTAR_WHISKY)
        //RIU[(BDMA_REG_BASE+0x04)] = 0x0345;  //((DstDEv|DstDataWidth)  <<8) |(SrcDataWidth|SrcDEv) ;
        msWrite2Bytes( BDMA_REG(0x04), 0x0345 );
    #else
        //RIU[(BDMA_REG_BASE+0x04)] = 0x0335;  //((DstDEv|DstDataWidth)  <<8) |(SrcDataWidth|SrcDEv) ;
        msWrite2Bytes( BDMA_REG(0x04), 0x0335 );
    #endif
    }
    else if( u8DestType == BDMA_CRC_TYPE_DRAM)
    {
        // Set src/dst sel
    #if (ENABLE_MSTAR_EULER || ENABLE_MSTAR_MAYA || ENABLE_MSTAR_WHISKY)
        msWrite2Bytes( BDMA_REG(0x04), 0x0340 );
    #else
        msWrite2Bytes( BDMA_REG(0x04), 0x0330 );
    #endif
    }

    // Trigger
    //RIU8[ (BDMA_REG_BASE*2)] = 0x01 ;
    msWriteByte( BDMA_REG(0x00), 0x01);

    // Wait done
    while( 1 )
    {
        //if ( (RIU8[(BDMA_REG_BASE+0x02)*2] & 0x08) == 0x08 )
        if( msReadByte(BDMA_REG(0x02)) & 0x08 )
            break;
    }


    //Get CRC32 result
    //u32CrcVal = RIU[(BDMA_REG_BASE + 0x18 )] ;        //low word
    u32CrcVal = msRead2Bytes( BDMA_REG(0x18) );        //low word
    //u32CrcVal |= (RIU[(BDMA_REG_BASE + 0x1a )]  <<  16);      //high word
    u32CrcVal |= (msRead2Bytes( BDMA_REG(0x1A) )  <<  16); //high word

#endif

    return u32CrcVal;
}

void __loader_2nd Ld_Timer_DelayUs(U32 u32DelayUs);
void __loader_2nd Ld_Timer_DelayUs(U32 u32DelayUs)
{
    U32 u32TargetTime = Ld_MDrv_PIU_Get_TimeUs() + u32DelayUs + 1;

    while(1)
    {
        if( Ld_MDrv_PIU_Get_TimeUs() >= u32TargetTime )
            break;
    }
}

#if(ENABLE_MSTAR_WHISKY)

#if(ENABLE_MSTAR_WHISKY)
#define AEON_REG_BASE       0x100F00
#define AEON_REG_CHIPTOP    0x100B00

#define AEON_REG_CLK        (AEON_REG_CHIPTOP+0x0024)

#define AEON_REG_STOP       (AEON_REG_BASE+0x00E6)
#define AEON_REG_CTRL       (AEON_REG_BASE+0x00F0)
#define AEON_REG_ORDER      (AEON_REG_BASE+0x0080)

#define AEON_SDR_ADDR0      (AEON_REG_BASE+0x00E4)
#define AEON_SDR_ADDR1      (AEON_REG_BASE+0x00E3)
#define AEON_SDR_ADDR2      (AEON_REG_BASE+0x00E7)

#define AEON_SDR_ADDR_DATA_L    (AEON_REG_BASE+0x00F4)
#define AEON_SDR_ADDR_DATA_H    (AEON_REG_BASE+0x00F6)

#define AEON_CTRL_EN            BIT0
#define AEON_CTRL_RST           BIT1
#define AEON_CTRL_MIUHIPRI      (BIT5|BIT4)
#endif

void __loader_2nd Ld_HAL_COPRO_Disable(void);
void __loader_2nd Ld_HAL_COPRO_Disable(void)
{
#if(ENABLE_MSTAR_WHISKY)
    // Disable aeon cpu
    msWriteByte(AEON_REG_STOP, 0x03); //stop aeon
    Ld_Timer_DelayUs(10);
    msWriteByte(AEON_REG_CTRL, (msReadByte(AEON_REG_CTRL) & ~(AEON_CTRL_EN + AEON_CTRL_RST)));

#else
    #error "Need check!"

#endif
}

void __loader_2nd Ld_HAL_COPRO_Enable(MS_U32 u32_ADR);
void __loader_2nd Ld_HAL_COPRO_Enable(MS_U32 u32_ADR)
{
#if(ENABLE_MSTAR_WHISKY)

    MS_U32 u32_BitOffsetAddr = 0;
    MS_U16 u16_SdrAddr0 = 0, u16_SdrAddr1 = 0;
    MS_U8 u8_SdrAddr1= 0;
    MS_U8 u8_SdrAddr2= 0;

    //##############################################
    //
    // ### Please Note here ###
    //
    // 1. Extral Aeon Internal Configuration must be done before releasing
    //      Aeon itself ==> refer to (1)
    // 2. Due to the fact that both Aeon and R2 can be House Keeper
    //      Therefore, release SOC Hold Processor Aeon first before releasing
    //      Aeon itself ==> refer to (2)
    // 3. Releasing Aeon is at the final step. ==> refer to (3)
    //
    //##############################################

    //(0) Basic Aeon Configuration
    msWriteByte(AEON_REG_ORDER, 0x02); // Let aeon command ¡§in order¡¨

  #if 1//!defined(CONFIG_FPGA_VERIFY)
    msWriteByte(AEON_REG_CLK,  0x08);  // step_1/2 turn aeon clk to 172.8Mhz
    msWriteByte(AEON_REG_CLK,  0x88);  // step_2/2 turn aeon clk to 172.8Mhz
  #endif

    //(1) Config SDRAM base address for instruction
    u32_BitOffsetAddr = u32_ADR >> 4; //(128bits = 16 bytes)
    u16_SdrAddr0 = (MS_U16)(u32_BitOffsetAddr);
    u8_SdrAddr1 = (MS_U8)(u32_BitOffsetAddr >> 16);
    u8_SdrAddr2 = (MS_U8)(u32_BitOffsetAddr >> 24);
    msWrite2Bytes(AEON_SDR_ADDR0, u16_SdrAddr0);
    msWriteByte(AEON_SDR_ADDR1, u8_SdrAddr1);
    msWriteByte(AEON_SDR_ADDR2, (msReadByte(AEON_SDR_ADDR2) & 0x03)|(u8_SdrAddr2&0x03));

    //(2) Config SDRAM base address for data
    u32_BitOffsetAddr = u32_ADR >> 3; //(64bits = 8 bytes)
    u16_SdrAddr0 = (MS_U16)(u32_BitOffsetAddr);
    u16_SdrAddr1 = (MS_U16)(u32_BitOffsetAddr>>16);
    msWrite2Bytes(AEON_SDR_ADDR_DATA_L, u16_SdrAddr0);
    msWrite2Bytes(AEON_SDR_ADDR_DATA_H, u16_SdrAddr1);


    //(3) Extral Aeon Internal Configuration
    msWriteByte(AEON_REG_STOP, 0x06); //aeon start & enable ECO

    //(2) release aeon at chiptop
    msWriteByte(0x2E53, msReadByte(0x2E53)&(~BIT6)); //R2 sw rst Aeon
    msWriteByte(0x2E53, msReadByte(0x2E53)|BIT6);

    //(3) relaese aeon at local
    msWriteByte(AEON_REG_CTRL, msReadByte(AEON_REG_CTRL)&(~AEON_CTRL_EN));
    msWriteByte(AEON_REG_CTRL, msReadByte(AEON_REG_CTRL)|(AEON_CTRL_EN)|(AEON_CTRL_RST)|(AEON_CTRL_MIUHIPRI));  // reset + enable aeon

#else

    #error "Need check!"

#endif

}

#endif

U32 __loader_2nd Ld_Mem_ReadU32(U32 u32MemAddr);
U32 __loader_2nd Ld_Mem_ReadU32(U32 u32MemAddr)
{
    U32 u32Tmp = 0;

    if( ((u32MemAddr) & 0x3) == 0 ) // Align to 4
    {
        u32Tmp = *((U32*)(u32MemAddr));
    }
    else
    {
        u32Tmp = *((U8*)(u32MemAddr + 3));
        u32Tmp <<= 8;
        u32Tmp |= *((U8*)(u32MemAddr + 2));
        u32Tmp <<= 8;
        u32Tmp |= *((U8*)(u32MemAddr + 1));
        u32Tmp <<= 8;
        u32Tmp |= *((U8*)(u32MemAddr + 0));
    }

    return u32Tmp;
}

void __loader_2nd Ld_Mem_WriteU8(U32 u32MemAddr, U8 u8Val);
void __loader_2nd Ld_Mem_WriteU8(U32 u32MemAddr, U8 u8Val)
{
    ((U8*)(u32MemAddr))[0] = u8Val;
}

void __loader_2nd Ld_Mem_WriteU32(U32 u32MemAddr, U32 u32Val);
void __loader_2nd Ld_Mem_WriteU32(U32 u32MemAddr, U32 u32Val)
{
    if( ((u32MemAddr) & 0x3) == 0 ) // Align to 4
    {
        *((U32*)(u32MemAddr)) = u32Val;
    }
    else
    {
        ((U8*)(u32MemAddr))[0] = u32Val&0xFF;
        ((U8*)(u32MemAddr))[1] = (u32Val>>8)&0xFF;
        ((U8*)(u32MemAddr))[2] = (u32Val>>16)&0xFF;
        ((U8*)(u32MemAddr))[3] = (u32Val>>24)&0xFF;
    }
}

void __loader_2nd Loader_MemCpy(U8* pu8DstAddr, U8* pu8SrcAddr, U32 u32Size);
void __loader_2nd Loader_MemCpy(U8* pu8DstAddr, U8* pu8SrcAddr, U32 u32Size)
{
    U32 i;
    for( i = 0; i < u32Size; i += 1 )
    {
        pu8DstAddr[i] = pu8SrcAddr[i];
    }
}

#endif // ENABLE_NON_OS

