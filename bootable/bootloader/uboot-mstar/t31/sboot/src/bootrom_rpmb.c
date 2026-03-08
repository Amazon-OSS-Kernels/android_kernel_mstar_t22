
#include "aesdma/src/drvAESDMA.h"
#include "mmc/inc/api/drv_eMMC.h"


struct SW_RPMB_DATA {
    unsigned int CID;
    char reserved[12];
    unsigned char key[16];
    char reserved2[12];
    unsigned int magic; // 0x626d7072
} ;

struct bootargs_teeloader {
    unsigned int ree_entry;
    unsigned int ree_size;
    unsigned int ree_opt;
    unsigned int CKB_Offset;
    unsigned int CKB_address;
    unsigned int CKB_size;
    unsigned int CustomizeID;
    unsigned int nand_opts;
    unsigned int ree_arg_addr;
    unsigned int mmc_opts;
    unsigned int mmc_rpmbswkey[4];
    unsigned int ModelID;
    unsigned int reserve[49];
} ;


#ifdef CONFIG_SW_RPMB_KEY
// below data lies in SRAM
unsigned char gData[] __attribute__((aligned(16)))= {
    #include "sw_rpmb.dat"
};

unsigned char* bootrom_copy_sw_key(unsigned char* buf);
unsigned char* bootrom_copy_sw_key(unsigned char* buf)
{
    struct SW_RPMB_DATA* pData = (struct SW_RPMB_DATA*)(gData);
    unsigned char* dst = ((struct bootargs_teeloader*)buf)->mmc_rpmbswkey;
    int i;
    for (i=0; i<16; i++)
        dst[i] = pData->key[i]; // pass SW key to TEE loader (in secure memory)

    for (i=0; i<sizeof(gData); i++)
        gData[i] = 0; // clear data on SRAM

    return buf;
}
#endif

// prototype
unsigned int bootrom_read_rpmb_data(unsigned char* buf);
unsigned int bootrom_read_rpmb_data(unsigned char* buf)
{
#ifdef CONFIG_SW_RPMB_KEY
    struct SW_RPMB_DATA* pData = (struct SW_RPMB_DATA*)(gData);
    int i;
    unsigned char* dataOnSram = (unsigned char*)pData;
    unsigned char* dataOnDram = (unsigned char*)buf;

    for (i=0; i<sizeof(struct SW_RPMB_DATA); i++)
        dataOnDram[i] = dataOnSram[i];

    MDrv_AESDMA_Decrypt((unsigned int)(buf+16)-CONFIG_MIU0_BUSADDR, 32, 0x0, AESDMA_ENGINE_CBC, E_AESDMA_UNIFORMKEY0);

    for (i=0; i<sizeof(struct SW_RPMB_DATA); i++)
    {
        dataOnSram[i] = dataOnDram[i];
        dataOnDram[i] = 0;  // clear data on DRAM
    }

    if (pData->magic != 0x626d7072)
        return eMMC_RPMB_Read_data(buf, 0x100, 0x0, 0x0); //// magic ID error, fallback to efuse key
    else
        return eMMC_RPMB_Read_data(buf, 0x100, 0x0, pData->key) == 0 ? 0 : eMMC_RPMB_Read_data(buf, 0x100, 0x0, 0x0);  //// try SW key first, if key is not correct, fallback to efuse key for backward compatible
#else
    return eMMC_RPMB_Read_data(buf, 0x100, 0x0, 0x0);
#endif
}
