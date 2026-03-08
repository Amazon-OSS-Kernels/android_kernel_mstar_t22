#include "drvSEAL.h"

int MDrv_SEAL_SetSecureRange(unsigned long long startAddr, unsigned long long endAddr, int attr)
{
    return HAL_SEAL_SetSecureRange(startAddr, endAddr, attr);
}

int MDrv_SEAL_DisableRIUBridges(void)
{
    return HAL_SEAL_DisableRIUBridges();
}

int MDrv_SEAL_Init(void)
{
    return HAL_SEAL_Init();
}

int MDrv_SEAL_SetMIUHost(int swIdx, int bSecure)
{
    return HAL_SEAL_SetMIUHost(swIdx, bSecure);
}

int MDrv_SEAL_SetRIUBank(int swIdx, int bSecure)
{
    return HAL_SEAL_SetRIUBank(swIdx, bSecure);
}

int MDrv_SEAL_SetRIURegister(int swIdx, int bSecure)
{
    return HAL_SEAL_SetRIURegister(swIdx, bSecure);
}

int MDrv_SEAL_Cleanup(void)
{
    return HAL_SEAL_Cleanup();
}

#ifdef CONFIG_SEAL_SELF_TEST
#include "../../common/src/common_print.h"

int MDrv_SEAL_SelfTest(void)
{
    unsigned int BufferAddr = 0x100000;
    unsigned int BufferLenth = 0x2000;
    unsigned int Attribute = 0x0;
    unsigned int PatternSize = 0x20;

    LDR_PUTS("=========================\n");
    LDR_PUTS("    SEAL Self Test\n");
    LDR_PUTS("=========================\n");
    LDR_PUTS("Test Init:\t");
    if( MDrv_SEAL_Init() != 1 )
        LDR_PUTS("Fail\n");
    LDR_PUTS("Pass\n");

    LDR_PUTS("Test Secure Range:\n");
    LDR_PUTS("\tSet Sercure Range ");
    if( MDrv_SEAL_SetSecureRange(BufferAddr, BufferAddr+BufferLenth, Attribute) != 1 )
        LDR_PUTS("Fail\n");
    else
        LDR_PUTS("Pass\n");

    LDR_PUTS("\tTEE touch Secure buffer ");
    *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + BufferAddr) = 0x0000FFFFFFFF0300ULL;
    *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + BufferAddr + 8) = 0x0000000000000000ULL;
    /*
    LDR_PUTDW(*(volatile unsigned long*)(CONFIG_MIU0_BUSADDR + BufferAddr));
    LDR_PUTS("\t");
    LDR_PUTDW(*(volatile unsigned long*)(CONFIG_MIU0_BUSADDR + BufferAddr + 4));
    LDR_PUTS("\t");
    LDR_PUTDW(*(volatile unsigned long*)(CONFIG_MIU0_BUSADDR + BufferAddr + 8));
    LDR_PUTS("\t");
    LDR_PUTDW(*(volatile unsigned long*)(CONFIG_MIU0_BUSADDR + BufferAddr + 12));
    LDR_PUTS("\t");
    */
    if( (*(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + BufferAddr) != 0x0000FFFFFFFF0300ULL) &&
        (*(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + BufferAddr + 8) != 0x0000000000000000ULL) )
        LDR_PUTS("Fail\n");
    else
        LDR_PUTS("Pass\n");

    LDR_PUTS("\tBDMA touch Secure buffer ");
#if defined(CONFIG_MSTAR_M7221) || defined(CONFIG_MSTAR_M7322) || defined(CONFIG_MSTAR_M7642)
    __BDMA_MIU0Copy2MIU0((unsigned int)BufferAddr, (unsigned int)(BufferAddr+PatternSize), PatternSize);
    /*
    LDR_PUTDW(*(volatile unsigned long*)(CONFIG_MIU0_BUSADDR + BufferAddr + PatternSize));
    LDR_PUTS("\t");
    LDR_PUTDW(*(volatile unsigned long*)(CONFIG_MIU0_BUSADDR + BufferAddr + PatternSize + 4));
    LDR_PUTS("\t");
    LDR_PUTDW(*(volatile unsigned long*)(CONFIG_MIU0_BUSADDR + BufferAddr + PatternSize + 8));
    LDR_PUTS("\t");
    LDR_PUTDW(*(volatile unsigned long*)(CONFIG_MIU0_BUSADDR + BufferAddr + PatternSize + 12));
    LDR_PUTS("\t");
    */
    if( (*(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + BufferAddr + PatternSize) == 0x0000FFFFFFFF0300ULL) &&
        (*(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + BufferAddr + PatternSize + 8) == 0x0000000000000000ULL) )
        LDR_PUTS("Fail\n");
    else
        LDR_PUTS("Pass\n");
#else
    LDR_PUTS("\n\t\tNot Support BDMA to test\n");
#endif

    LDR_PUTS("\tRelease Sercure Range ");
    Attribute = 0xf;
    if( MDrv_SEAL_SetSecureRange(BufferAddr, BufferAddr+BufferLenth, Attribute) != 1 )
        LDR_PUTS("Fail\n");
    else
        LDR_PUTS("Pass\n");

    LDR_PUTS("=========================\n");
    LDR_PUTS("    The End\n");
    LDR_PUTS("=========================\n");
    return 0;
}
#endif
