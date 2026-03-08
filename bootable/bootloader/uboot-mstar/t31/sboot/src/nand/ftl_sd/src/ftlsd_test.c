
#include "ftlsd_api.h"

#if (defined(MSFTLSd_UBOOT)&&MSFTLSd_UBOOT) || (defined(MSFTLSd_LINUX)&&MSFTLSd_LINUX)
U8 *pu8_FtlTestDatBuf = (U8*)DMA_W_ADDR; 
U8 *pu8_FtlTestTmpBuf = (U8*)DMA_R_ADDR; 
U8 *pu8_FtlTestRddBuf = (U8*)DMA_W_SPARE_ADDR;

#define DATA_LEN   500
// =============================================
U32 FtlSdTest_OutOfSecureRange(U16 PBA)
{
    volatile U32 u32_i, u32_err;
    volatile U32 *pu32_1=(U32*)pu8_FtlTestDatBuf, *pu32_2=(U32*)pu8_FtlTestTmpBuf;

    u32_err = (*FtlSdDev.pFnEraseBlk)(PBA);
    if(FTLSd_OK == u32_err) // out-of-range should not be ok
    {
        ftl_dbg(0,1,"erase: %Xh ok, abnormal!\n", PBA);
        //return FTLSd_ERR_TEST_FAIL;
    }
    else
        return u32_err;
    
    for(u32_i=0; u32_i<DATA_LEN; u32_i++)
    {
        pu32_1[u32_i] = u32_i;  pu32_2[u32_i] = ~u32_i;
    }

    u32_err = (*FtlSdDev.pFnWritePages)(BlkRowAddr(PBA), (U8*)pu32_1, pu8_FtlTestRddBuf, 1);
    if(FTLSd_OK == u32_err) // out-of-range should not be ok
    {
        ftl_dbg(0,1,"write: %Xh ok, abnormal!\n", BlkRowAddr(PBA));
        //return FTLSd_ERR_TEST_FAIL;
    }
    else
        return u32_err;

    u32_err = (*FtlSdDev.pFnReadPages)(BlkRowAddr(PBA), (U8*)pu32_2, pu8_FtlTestRddBuf, 1);
    if(FTLSd_OK == u32_err) // out-of-range should not be ok
    {
        ftl_dbg(0,1,"read: %Xh ok, abnormal!\n", BlkRowAddr(PBA));
        //return FTLSd_ERR_TEST_FAIL;
    }
    else
        return u32_err;

    if(FTLSd_OK == nand_CompareData(pu8_FtlTestDatBuf, pu8_FtlTestTmpBuf, DATA_LEN)) // out-of-range should not be ok
    {
        ftl_dbg(0,1,"cmp ok, abnormal!\n");
        return FTLSd_ERR_TEST_FAIL;
    }
    else
        return u32_err;
}


U32 FtlSdTest_DataWRC(U32 u32_TestLoop, U32 seed)
{
    volatile U32 u32_i;
    volatile U32 *pu32_1=(U32*)pu8_FtlTestDatBuf;//, *pu32_2=(U32*)pu8_FtlTestTmpBuf;

    for(u32_i=0; u32_i<DATA_LEN; u32_i++)
        pu32_1[u32_i] = u32_i+seed;

    for(u32_i=0; u32_i<u32_TestLoop; u32_i++)
    {
        //ftl_dbg(0,1,"loop: %Xh \n", u32_i);
        
        if(FTLSd_WriteData(pu8_FtlTestDatBuf, DATA_LEN))
            return FTLSd_ERR_TEST_FAIL;
            
        if(FTLSd_ReadData(pu8_FtlTestTmpBuf, DATA_LEN))
        //if(FTLSd_ReadData_Ex(pu8_FtlTestTmpBuf, 1))
            return FTLSd_ERR_TEST_FAIL;
            
        if(nand_CompareData(pu8_FtlTestDatBuf, pu8_FtlTestTmpBuf, DATA_LEN))
            return FTLSd_ERR_TEST_FAIL;
    }
    return FTLSd_OK;
}


U32 FtlSdTest_DataRC(U32 u32_TestLoop, U32 seed)
{
    volatile U32 u32_i;
    volatile U32 *pu32_1=(U32*)pu8_FtlTestDatBuf;//, *pu32_2=(U32*)pu8_FtlTestTmpBuf;

    for(u32_i=0; u32_i<DATA_LEN; u32_i++)
        pu32_1[u32_i] = u32_i+seed;

    for(u32_i=0; u32_i<u32_TestLoop; u32_i++)
    {
        ftl_dbg(0,1,"loop: %Xh \n", u32_i);
        
        //if(FTLSd_WriteData(pu8_FtlTestDatBuf, DATA_LEN))
        //    return FTLSd_ERR_TEST_FAIL;
            
        if(FTLSd_ReadData(pu8_FtlTestTmpBuf, DATA_LEN))
            return FTLSd_ERR_TEST_FAIL;
            
        if(nand_CompareData(pu8_FtlTestDatBuf, pu8_FtlTestTmpBuf, DATA_LEN))
            return FTLSd_ERR_TEST_FAIL;
    }
    return FTLSd_OK;
}


#endif // uboot || linux

