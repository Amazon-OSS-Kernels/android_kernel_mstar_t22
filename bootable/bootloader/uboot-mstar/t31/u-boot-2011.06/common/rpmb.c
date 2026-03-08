/* ============================================================================
    Includes
============================================================================ */
#include "rpmb.h"
#include "MsOS.h"

/* ============================================================================
    Constant definitions
============================================================================ */
#ifndef UINT_MAX
#define UINT_MAX 4294967295U
#endif

#define UBOOT_READ_RPMB_MAGIC 0x5552524d
#define UBOOT_WRITE_RPMB_MAGIC_0_1 0x55575200  //for rpmb block 0/1

#define UBOOT_GET_NONCE_MAGIC 0x55474e4d

#define RPMB_OPTEE_ACCESS_ADDR 0xb200585F
#define RPMB_OPTEE_DISABLE_SMC 0xb2005862

#define STD_RPMB_OPTEE_ACCESS_ADDR 0x3200ff04
#define STD_RPMB_OPTEE_DISABLE_SMC 0x3200ff05

#define OPTEE_SMC_CALL_RETURN_FROM_RPC 0x32000003

#define GET_OPTEE_VERSION_NUMBER_SMC_CMD (0xb2000001)

/* ============================================================================
    Global Variable
============================================================================ */
static uint32_t g_optee_version_number;
static int is_std_init = 0;
static uint32_t g_optee_shm_adr;

/* ============================================================================
    Internal Functions
============================================================================ */

struct smc_param {
    U32 a0;
    U32 a1;
    U32 a2;
    U32 a3;
    U32 a4;
    U32 a5;
    U32 a6;
    U32 a7;
};


static void init_std(void)
{
    int ret = 0;
    if (is_std_init)
        return;

    is_std_init = 1;

    ret = get_addr_from_mmap("E_MMAP_ID_OPTEE_MEM_SHM", &g_optee_shm_adr);
    if( ret != 0)
        printf("Get map E_MMAP_ID_OPTEE_MEM_SHM failed\n");

    g_optee_shm_adr += CONFIG_SYS_MIU0_BUS;

    struct smc_param get_optee_version_smc_params;
    memset(&get_optee_version_smc_params, 0, sizeof(get_optee_version_smc_params));
    get_optee_version_smc_params.a0 = GET_OPTEE_VERSION_NUMBER_SMC_CMD;

#if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
    tee_smc_call(&get_optee_version_smc_params);
#endif

   g_optee_version_number =  get_optee_version_smc_params.a0;
   printf("[%s] OPTEE version is %d\n", __func__, g_optee_version_number);
}

static int _ReverseEndian(void *data, size_t size)
{
    uint32_t i;
    char tmp;
    char *swp = (char *)data;
    for (i = 0; i < (size / 2); ++i) {
        tmp = swp[i];
        swp[i] = swp[size - 1 - i];
        swp[size - 1 - i] = tmp;
    }
    return 0;
}

static inline void reg_pair_from_64(u32 *reg0, u32 *reg1, u64 val)
{
    *reg0 = val >> 32;
    *reg1 = val;
}

static int32_t ubootRpmbWriteCounter(void) {
    int32_t ret = 0;
    volatile U32 write_cnt = 0;

    if(eMMC_RPMB_IfKeyWritten() != 1)
    {
        printf("%s:%s:%d There is no key in emmc\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    ret = eMMC_RPMB_Get_Counter_for_cmd(&write_cnt);
    if( ret != eMMC_ST_SUCCESS )
    {
        printf("%s:%s:%d Fail to get rpmb write counter\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return write_cnt;
}

static uint32_t ubootGetNonce(u8 *nonce)
{
    uint32_t ret = 0;
    UBOOT_READ_NONCE* uboot_read_nonce = NULL;
    uboot_read_nonce = (UBOOT_READ_NONCE*)malloc(sizeof(UBOOT_READ_NONCE));
    memset(uboot_read_nonce, 0, sizeof(UBOOT_READ_NONCE));

    uboot_read_nonce->uboot_nonce_magic = UBOOT_GET_NONCE_MAGIC;
    flush_cache(uboot_read_nonce, sizeof(UBOOT_READ_NONCE));

    struct smc_param rpmb_get_nonce_smc_params;
    memset(&rpmb_get_nonce_smc_params, 0, sizeof(rpmb_get_nonce_smc_params));

    init_std();

    if (g_optee_version_number == 1)
    {
        rpmb_get_nonce_smc_params.a0 = RPMB_OPTEE_ACCESS_ADDR;
        rpmb_get_nonce_smc_params.a1 = uboot_read_nonce;

    #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
        tee_smc_call(&rpmb_get_nonce_smc_params);
    #endif

        ret = rpmb_get_nonce_smc_params.a0;
        if( ret != 0 )
            printf("%s:%s:%d rpmb get nonce fail, ret=0x%x\n", __FILE__, __FUNCTION__, __LINE__, ret);
        else
            memcpy(nonce, rpmb_get_nonce_smc_params.a1, RPMB_NONCE_SIZE);
    }
    else if (g_optee_version_number >= 2)
    {
        rpmb_get_nonce_smc_params.a0 = STD_RPMB_OPTEE_ACCESS_ADDR;
        rpmb_get_nonce_smc_params.a6 = uboot_read_nonce;

    #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
        printf("%s\n", __func__); // this printf is for timing issue ???? without this, rpmb std call will fail
        tee_smc_call(&rpmb_get_nonce_smc_params);
    #endif

        if (g_optee_version_number == 2)
        {
            rpmb_get_nonce_smc_params.a0 = OPTEE_SMC_CALL_RETURN_FROM_RPC;
            rpmb_get_nonce_smc_params.a6 = uboot_read_nonce;

            reg_pair_from_64(&(rpmb_get_nonce_smc_params.a1), &(rpmb_get_nonce_smc_params.a2), g_optee_shm_adr);
        #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
            printf("%s\n", __func__);
            tee_smc_call(&rpmb_get_nonce_smc_params);
        #endif
        }
        else//g_optee_version_number == 3
        {
            invalidate_dcache_range(rpmb_get_nonce_smc_params.a0, sizeof(rpmb_get_nonce_smc_params.a0));
            invalidate_dcache_range(rpmb_get_nonce_smc_params.a6, RPMB_NONCE_SIZE);
        }

        ret = rpmb_get_nonce_smc_params.a0;
        if( ret != 0 )
            printf("%s:%s:%d rpmb read fail, ret=0x%x\n", __FILE__, __FUNCTION__, __LINE__, ret);
        else
            memcpy(nonce, rpmb_get_nonce_smc_params.a6, RPMB_NONCE_SIZE);
    }
    else
    {
        printf("%s:%s:%d Incorrect OPTEE version number\n", __FILE__, __FUNCTION__, __LINE__);
    }

    free(uboot_read_nonce);
    return ret;
}

/* ============================================================================
    Exported Entry Points
============================================================================ */
static uint32_t get_rpmb_address_cus(uint32_t *plen, uint32_t *poffset, uint16_t *pblk_addr, uint32_t operation_id)
{
	int32_t ret = 0;

	switch(operation_id){
		case UNIQUE_ITVID:
			*pblk_addr = 0;
			*poffset = 56;
			*plen = 24;
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}

uint32_t uboot_rpmb_read_data_cus(uint8_t *data, uint32_t datalen, uint32_t operation_id)
{
	int32_t ret = 0;
	uint16_t blk_addr = 0;
	uint32_t offset = 0;
	uint32_t len = 0;

	ret = get_rpmb_address_cus(&len, &offset, &blk_addr, operation_id);
	if( ret != 0 )
	{
		printf("%s:%s:%d can not support this operation_id=%d\n", __FILE__, __FUNCTION__, __LINE__, operation_id);
		return -1;
	}

	if(datalen > len)
	{
		printf("%s:%s:%d datalen:%d bytes is too long syetem only allocate %d bytes\n", __FILE__, __FUNCTION__, __LINE__, datalen, len);
		return -1;
	}

	ret = uboot_rpmb_read_data(data, datalen, offset, blk_addr);
	if( ret != 0 )
	{
		printf("%s:%s:%d rpmb read data fail, ret=0x%x\n", __FILE__, __FUNCTION__, __LINE__, ret);
		return -1;
	}
	return 0;
}

uint32_t uboot_rpmb_write_data_cus(uint8_t *data, uint32_t datalen, uint32_t operation_id)
{
	int32_t ret = 0;
	uint16_t blk_addr = 0;
	uint32_t offset = 0;
	uint32_t len = 0;

	ret = get_rpmb_address_cus(&len, &offset, &blk_addr, operation_id);
	if( ret != 0 )
	{
		printf("%s:%s:%d can not support this operation_id=%d\n", __FILE__, __FUNCTION__, __LINE__, operation_id);
		return -1;
	}

	if(datalen > len)
	{
		printf("%s:%s:%d datalen:%d bytes is too long syetem only allocate %d bytes\n", __FILE__, __FUNCTION__, __LINE__, datalen, len);
		return -1;
	}

	ret = uboot_rpmb_write_data(data, datalen, offset, blk_addr);
	if( ret != 0 )
	{
		printf("%s:%s:%d rpmb read data fail, ret=0x%x\n", __FILE__, __FUNCTION__, __LINE__, ret);
		return -1;
	}
	return 0;
}

uint32_t uboot_rpmb_write_data(uint8_t *data, uint32_t len, uint32_t offset, uint16_t blk_addr)
{
    int32_t ret = 0;
    uint8_t cid[RPMB_EMMC_CID_SIZE] = {0};
    uint8_t nonce[RPMB_NONCE_SIZE] = {0};
    uint8_t block_data[RPMB_BLOCK_SIZE]={0};
    uint32_t write_cnt = 0;

    write_cnt = ubootRpmbWriteCounter();
    if(write_cnt < 0)
    {
        printf("%s:%s:%d Fail to get rpmb write counter\n", __FILE__, __FUNCTION__, __LINE__);
        return 1;
    }

    ret = eMMC_GetCID(&cid);
    if(ret != eMMC_ST_SUCCESS)
    {
        printf("%s:%s:%d Fail to get emmc cid\n", __FILE__, __FUNCTION__, __LINE__);
        return 1;
    }

    ret = ubootGetNonce(nonce);
    if(ret != eMMC_ST_SUCCESS)
    {
        printf("%s:%s:%d Fail to get nonce\n", __FILE__, __FUNCTION__, __LINE__);
        return 1;
    }

    _ReverseEndian(nonce, RPMB_NONCE_SIZE);

    ret = eMMC_RPMB_Read_Blk(block_data, nonce, blk_addr);
    if(ret != eMMC_ST_SUCCESS)
    {
        printf("%s:%s:%d Fail to read rpmb block\n", __FILE__, __FUNCTION__, __LINE__);
        return 1;
    }

    UBOOT_WRITE_RPMB_0_1 *rmpb_write_data = NULL;
    rmpb_write_data = (UBOOT_WRITE_RPMB_0_1 *)malloc(sizeof(UBOOT_WRITE_RPMB_0_1));
    rmpb_write_data->uboot_write_magic = UBOOT_WRITE_RPMB_MAGIC_0_1;
    rmpb_write_data->writer_counter = write_cnt;
    memcpy(rmpb_write_data->cid, cid, RPMB_EMMC_CID_SIZE);
    memcpy(rmpb_write_data->block_data, block_data, RPMB_BLOCK_SIZE);
    memcpy(rmpb_write_data->data, data, len);
    rmpb_write_data->datalen = len;
    rmpb_write_data->offset = offset;

    flush_cache(rmpb_write_data, sizeof(UBOOT_WRITE_RPMB_0_1));

    struct smc_param rpmb_write_smc_params;
    memset(&rpmb_write_smc_params, 0, sizeof(rpmb_write_smc_params));

    init_std();

    if (g_optee_version_number == 1)
    {
        rpmb_write_smc_params.a0 = RPMB_OPTEE_ACCESS_ADDR;
        rpmb_write_smc_params.a1 = rmpb_write_data;

    #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
        tee_smc_call(&rpmb_write_smc_params);
    #endif

        ret = rpmb_write_smc_params.a0;
        if(ret != eMMC_ST_SUCCESS)
        {
            printf("%s:%s:%d Fail to get rpmb block(with MAC)\n", __FILE__, __FUNCTION__, __LINE__);
            free(rmpb_write_data);
            return 1;
        }

        memcpy(block_data, rpmb_write_smc_params.a1, RPMB_BLOCK_SIZE);
    }
    else if (g_optee_version_number >= 2)
    {
        rpmb_write_smc_params.a0 = STD_RPMB_OPTEE_ACCESS_ADDR;
        rpmb_write_smc_params.a6 = rmpb_write_data;

    #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
        printf("%s\n", __func__); // this printf is for timing issue ???? without this, rpmb std call will fail
        tee_smc_call(&rpmb_write_smc_params);
    #endif

        if (g_optee_version_number == 2)
        {
            rpmb_write_smc_params.a0 = OPTEE_SMC_CALL_RETURN_FROM_RPC;
            rpmb_write_smc_params.a6 = rmpb_write_data;
            reg_pair_from_64(&(rpmb_write_smc_params.a1), &(rpmb_write_smc_params.a2), g_optee_shm_adr);
        #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
            printf("%s\n", __func__);
            tee_smc_call(&rpmb_write_smc_params);
        #endif
        }
        else//g_optee_version_number == 3
        {
            invalidate_dcache_range(rpmb_write_smc_params.a0, sizeof(rpmb_write_smc_params.a0));
            invalidate_dcache_range(rpmb_write_smc_params.a6, RPMB_BLOCK_SIZE);
        }

        ret = rpmb_write_smc_params.a0;
        if(ret != eMMC_ST_SUCCESS)
        {
            printf("%s:%s:%d Fail to get rpmb block(with MAC)\n", __FILE__, __FUNCTION__, __LINE__);
            free(rmpb_write_data);
            return 1;
        }

        memcpy(block_data, rpmb_write_smc_params.a6, RPMB_BLOCK_SIZE);
    }
    else
    {
        printf("%s:%s:%d Incorrect OPTEE version number\n", __FILE__, __FUNCTION__, __LINE__);
    }
    _ReverseEndian(block_data, RPMB_BLOCK_SIZE);
    ret = eMMC_RPMB_Write_Blk(block_data);
    if(ret != eMMC_ST_SUCCESS)
    {
        printf("%s:%s:%d Fail to write RPMB\n", __FILE__, __FUNCTION__, __LINE__);
        free(rmpb_write_data);
        return 1;
    }

    free(rmpb_write_data);
    return 0;
}

uint32_t uboot_rpmb_read_data(uint8_t *data, uint32_t len, uint32_t offset, uint16_t blk_addr)
{
    int32_t ret = 0;
    uint8_t cid[RPMB_EMMC_CID_SIZE] = {0};
    uint8_t nonce[RPMB_NONCE_SIZE] = {0};
    uint8_t block_data[RPMB_BLOCK_SIZE] = {0};
    uint8_t rpmb_data[RPMB_DATA_SIZE] = {0};

    if(offset + len > RPMB_DATA_SIZE || offset >= RPMB_DATA_SIZE || len > RPMB_DATA_SIZE || len == 0)
    {
        printf("%s:%s:%d bad parameter offset=%d len=%d\n", __FILE__, __FUNCTION__, __LINE__,offset,len);
        return -1;
    }

    if(blk_addr > 1)
    {
        printf("%s:%s:%d uboot rpmb only support rw blk0,blk1 blk_addr=%d\n", __FILE__, __FUNCTION__, __LINE__,blk_addr);
        return -1;
    }

    ret = eMMC_GetCID(&cid);
    if(ret != eMMC_ST_SUCCESS)
    {
        printf("%s:%s:%d Fail to get emmc cid\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    ret = ubootGetNonce(nonce);
    if(ret != eMMC_ST_SUCCESS)
    {
        printf("%s:%s:%d Fail to get nonce\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    _ReverseEndian(nonce, RPMB_NONCE_SIZE);

    ret = eMMC_RPMB_Read_Blk(block_data, nonce, blk_addr);
    if(ret != eMMC_ST_SUCCESS)
    {
        printf("%s:%s:%d Fail to read rpmb block\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    UBOOT_READ_RPMB rmpb_read_data;
    uint32_t rmpb_read_data_size = sizeof(rmpb_read_data);
    memset(rmpb_read_data.block_data, 0, RPMB_BLOCK_SIZE);
    rmpb_read_data.uboot_read_magic = UBOOT_READ_RPMB_MAGIC;
    memcpy(rmpb_read_data.cid, cid, RPMB_EMMC_CID_SIZE);
    memcpy(rmpb_read_data.block_data, block_data, RPMB_BLOCK_SIZE);

    flush_cache(&rmpb_read_data, rmpb_read_data_size);

    struct smc_param rpmb_read_smc_params;
    memset(&rpmb_read_smc_params, 0, sizeof(rpmb_read_smc_params));

    init_std();

    if (g_optee_version_number == 1)
    {
        rpmb_read_smc_params.a0 = RPMB_OPTEE_ACCESS_ADDR;
        rpmb_read_smc_params.a1 = &rmpb_read_data;

    #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
        tee_smc_call(&rpmb_read_smc_params);
    #endif

        ret = rpmb_read_smc_params.a0;
        if( ret != 0 )
        {
            printf("%s:%s:%d rpmb read data fail, ret=0x%x\n", __FILE__, __FUNCTION__, __LINE__, ret);
                return -1;
        }
    }
    else if (g_optee_version_number >= 2)
    {
        rpmb_read_smc_params.a0 = STD_RPMB_OPTEE_ACCESS_ADDR;
        rpmb_read_smc_params.a6 = &rmpb_read_data;

    #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
        printf("%s\n", __func__); // this printf is for timing issue ???? without this, rpmb std call will fail
        tee_smc_call(&rpmb_read_smc_params);
    #endif

        if (g_optee_version_number == 2)
        {
            rpmb_read_smc_params.a0 = OPTEE_SMC_CALL_RETURN_FROM_RPC;
            rpmb_read_smc_params.a6 = &rmpb_read_data;
            reg_pair_from_64(&(rpmb_read_smc_params.a1), &(rpmb_read_smc_params.a2), g_optee_shm_adr);
        #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
            printf("%s\n", __func__);
            tee_smc_call(&rpmb_read_smc_params);
        #endif
        }
        else//g_optee_version_number == 3
        {
            invalidate_dcache_range(rpmb_read_smc_params.a0, sizeof(rpmb_read_smc_params.a0));
            invalidate_dcache_range(rpmb_read_smc_params.a6, RPMB_BLOCK_SIZE);
        }

        ret = rpmb_read_smc_params.a0;
        if( ret != 0 )
        {
            printf("%s:%s:%d rpmb read data fail, ret=0x%x\n", __FILE__, __FUNCTION__, __LINE__, ret);
                return -1;
        }
    }
    else
    {
        printf("%s:%s:%d Incorrect OPTEE version number\n", __FILE__, __FUNCTION__, __LINE__);
    }

    _ReverseEndian(block_data, RPMB_BLOCK_SIZE);

    eMMC_RPMB_DATA *p = block_data;
    memcpy(rpmb_data, p->u8_data, RPMB_DATA_SIZE);

    // data need to reverse
    ret = _ReverseEndian(rpmb_data, RPMB_DATA_SIZE);
    if(ret != 0)
    {
        printf("%s:%s:%d Fail to ReverseEndian\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
    memcpy(data, rpmb_data+offset, len);
    return 0;
}

uint32_t uboot_disable_smc()
{
   uint32_t ret = 0;
    struct smc_param uboot_disable_smc_params;
    memset(&uboot_disable_smc_params, 0, sizeof(uboot_disable_smc_params));

    init_std();

    if (g_optee_version_number == 1)
    {
        uboot_disable_smc_params.a0 = RPMB_OPTEE_DISABLE_SMC;

    #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
        tee_smc_call(&uboot_disable_smc_params);
    #endif

        ret = uboot_disable_smc_params.a0;
        if( ret != 0 )
        {
            printf("%s:%s:%d uboot disable smc fail, ret=0x%x\n", __FILE__, __FUNCTION__, __LINE__, ret);
            return -1;
        }
    }
    else if (g_optee_version_number >= 2)
    {
        uboot_disable_smc_params.a0 = STD_RPMB_OPTEE_DISABLE_SMC;

    #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
        printf("%s\n", __func__); // this printf is for timing issue ???? without this, rpmb std call will fail
        tee_smc_call(&uboot_disable_smc_params);
    #endif

        if (g_optee_version_number == 2)
        {
            uboot_disable_smc_params.a0 = OPTEE_SMC_CALL_RETURN_FROM_RPC;

            reg_pair_from_64(&(uboot_disable_smc_params.a1), &(uboot_disable_smc_params.a2), g_optee_shm_adr);
        #if defined(CONFIG_OPTEE_TEELOADER) || defined(CONFIG_MSTAR_NS_UBOOT)
            printf("%s\n", __func__);
            tee_smc_call(&uboot_disable_smc_params);
        #endif
        }
        else//g_optee_version_number == 3
        {
            invalidate_dcache_range(uboot_disable_smc_params.a0, sizeof(uboot_disable_smc_params.a0));
        }

        ret = uboot_disable_smc_params.a0;
        if( ret != 0 )
        {
             printf("%s:%s:%d uboot disable smc fail, ret=0x%x\n", __FILE__, __FUNCTION__, __LINE__, ret);
             return -1;
        }
    }
    else
    {
        printf("%s:%s:%d Incorrect OPTEE version number\n", __FILE__, __FUNCTION__, __LINE__);
    }

    return ret;

}

#if 0
static void rpmb_test()
{
	char data[256]={0};
	int i = 0;
	int ret = 0;

	printf("rpmb block0 data :\n");

	memset(data, 0, 256);
	ret = uboot_rpmb_read_data(data, 256, 0, 0);
	if(ret == -1)
		printf("uboot_rpmb_read_data fail\n");
	printf("rpmb block0 data :\n");
	for(i=0; i<16; i++)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data[16*i+0],data[16*i+1],data[16*i+2],data[16*i+3],
			data[16*i+4],data[16*i+5],data[16*i+6],data[16*i+7],
			data[16*i+8],data[16*i+9],data[16*i+10],data[16*i+11],
			data[16*i+12],data[16*i+13],data[16*i+14],data[16*i+15]);

	memset(data, 0, 256);
	ret = uboot_rpmb_read_data_cus(data, 24, UNIQUE_ITVID);
	if(ret == -1)
		printf("uboot_rpmb_read_data_cus fail\n");
	printf("rpmb block0 data off 0x38 len 0x18 :\n");
	for(i=0; i<2; i++)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data[16*i+0],data[16*i+1],data[16*i+2],data[16*i+3],
			data[16*i+4],data[16*i+5],data[16*i+6],data[16*i+7],
			data[16*i+8],data[16*i+9],data[16*i+10],data[16*i+11],
			data[16*i+12],data[16*i+13],data[16*i+14],data[16*i+15]);

	memset(data, 0x11, 24);
	ret = uboot_rpmb_write_data_cus(data, 24, UNIQUE_ITVID);
	if(ret == -1)
		printf("uboot_rpmb_write_data_cus fail\n");


	memset(data, 0, 256);
	ret = uboot_rpmb_read_data(data, 256, 0, 0);
	if(ret == -1)
		printf("uboot_rpmb_read_data fail\n");
	printf("rpmb block0 data :\n");
	for(i=0; i<16; i++)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data[16*i+0],data[16*i+1],data[16*i+2],data[16*i+3],
			data[16*i+4],data[16*i+5],data[16*i+6],data[16*i+7],
			data[16*i+8],data[16*i+9],data[16*i+10],data[16*i+11],
			data[16*i+12],data[16*i+13],data[16*i+14],data[16*i+15]);

	memset(data, 0, 256);
	ret = uboot_rpmb_read_data_cus(data, 24, UNIQUE_ITVID);
	if(ret == -1)
		printf("uboot_rpmb_read_data_cus fail\n");
	printf("rpmb block0 data off 0x38 len 0x18 :\n");
	for(i=0; i<2; i++)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data[16*i+0],data[16*i+1],data[16*i+2],data[16*i+3],
			data[16*i+4],data[16*i+5],data[16*i+6],data[16*i+7],
			data[16*i+8],data[16*i+9],data[16*i+10],data[16*i+11],
			data[16*i+12],data[16*i+13],data[16*i+14],data[16*i+15]);

	memset(data, 0x22, 24);
	ret = uboot_rpmb_write_data(data, 24, 56, 0);
	if(ret == -1)
		printf("uboot_rpmb_write_data_cus fail\n");
	memset(data, 0, 256);
	ret = uboot_rpmb_read_data(data, 256, 0, 0);
	if(ret == -1)
		printf("uboot_rpmb_read_data fail\n");
	printf("rpmb block0 data :\n");
	for(i=0; i<16; i++)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data[16*i+0],data[16*i+1],data[16*i+2],data[16*i+3],
			data[16*i+4],data[16*i+5],data[16*i+6],data[16*i+7],
			data[16*i+8],data[16*i+9],data[16*i+10],data[16*i+11],
			data[16*i+12],data[16*i+13],data[16*i+14],data[16*i+15]);


	memset(data, 0, 256);
	ret = uboot_rpmb_read_data(data, 256, 0, 1);
	if(ret == -1)
		printf("uboot_rpmb_read_data fail\n");
	printf("rpmb block1 data :\n");
	for(i=0; i<16; i++)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data[16*i+0],data[16*i+1],data[16*i+2],data[16*i+3],
			data[16*i+4],data[16*i+5],data[16*i+6],data[16*i+7],
			data[16*i+8],data[16*i+9],data[16*i+10],data[16*i+11],
			data[16*i+12],data[16*i+13],data[16*i+14],data[16*i+15]);

	memset(data, 0x22, 128);
	ret = uboot_rpmb_write_data(data, 128, 0, 1);
	if(ret == -1)
		printf("uboot_rpmb_write_data_cus fail\n");
	memset(data, 0, 256);
	ret = uboot_rpmb_read_data(data, 256, 0, 1);
	if(ret == -1)
		printf("uboot_rpmb_read_data fail\n");
	printf("rpmb block1 data :\n");
	for(i=0; i<16; i++)
		printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			data[16*i+0],data[16*i+1],data[16*i+2],data[16*i+3],
			data[16*i+4],data[16*i+5],data[16*i+6],data[16*i+7],
			data[16*i+8],data[16*i+9],data[16*i+10],data[16*i+11],
			data[16*i+12],data[16*i+13],data[16*i+14],data[16*i+15]);

	ret = uboot_disable_smc();
	if(ret == 0)
		printf("disable smc success\n");
	else
		printf("disable smc failed\n");

}

U_BOOT_CMD(rpmbtest, 1, 0, rpmb_test,
       "rpmbtest",
       "rpmbtest");
#endif