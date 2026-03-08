#ifndef rpmb
#define rpmb
#include "eMMC_RPMB.h"

#define RPMB_NONCE_SIZE (16)
#define RPMB_DATA_SIZE (256)

#ifndef SHA256_HASH_SIZE
#define SHA256_HASH_SIZE (32)
#endif

#define RPMB_BLOCK_SIZE (512)
#define RPMB_EMMC_CID_SIZE (16)
#define RPMB_NONCE_SIZE (16)

#define RPMB_UBOOT_MAX_SIZE (128)

#define UNIQUE_ITVID 0

#define FLAG_ANTIROLLBACK_INITIALIZED (0x42525468)

typedef struct RPMB_DATAFRAM {
    unsigned short type;
    unsigned short result;
    unsigned short blkCnt;
    unsigned short addr;
    uint32_t writeCnt;
    uint8_t nonce[RPMB_NONCE_SIZE];
    uint8_t data[RPMB_DATA_SIZE];
    uint8_t mac[SHA256_HASH_SIZE];
    uint8_t stuff[196];
} RPMB_DATAFRAM;

struct rpmb_fs_partition {
    uint32_t rpmb_fs_magic;
    uint32_t fs_version;
    uint32_t write_counter;
    uint32_t fat_start_address;
    /* Do not use reserved[] for other purpose than partition data. */
    uint8_t reserved[40];
    uint8_t  unique_itvid[24];
    uint8_t  magicString[16];
    uint32_t uboot_version;
    uint32_t hash1_version;
    uint32_t teekeybank_version;
    uint32_t teeloader_version;
    uint32_t reeloader_version;
    uint32_t optee_version;
    uint32_t armfw_version;
    uint32_t anti_rollback_init_flag;
};

typedef enum {
    REQ_SET_KEY = 0x1,
    REQ_GET_CNT = 0x2,
    REQ_WR_DATA = 0x3,
    REQ_RD_DATA = 0x4,
    REQ_RD_RESULT = 0x5,
} RPMB_REQ;

typedef enum {
    RESP_SET_KEY = 0x100,
    RESP_GET_CNT = 0x200,
    RESP_WR_DATA = 0x300,
    RESP_RD_DATA = 0x400,
} RPMB_RESP;

typedef enum {
    RESULT_OK = 0x0,
    RESULT_NG = 0x1,
    RESULT_ATH_NG = 0x2,
    RESULT_CNT_NG = 0x3,
    RESULT_ADR_NG = 0x4,
    RESULT_WR_NG = 0x5,
    RESULT_RD_NG = 0x6,
    RESULT_NO_KEY = 0x7,
    RESULT_TRYLOCK_FAIL = 0x8,
    RESULT_DECRYPT_FAIL = 0x80,
} RPMB_RESULT;

typedef struct{
    uint32_t uboot_nonce_magic;
    uint8_t nonce[RPMB_NONCE_SIZE];
} UBOOT_READ_NONCE;

typedef struct{
    uint32_t uboot_read_magic;
    uint8_t cid[RPMB_EMMC_CID_SIZE];
    uint8_t block_data[RPMB_BLOCK_SIZE];
} UBOOT_READ_RPMB;

typedef struct{
    uint32_t uboot_write_magic;
    uint32_t writer_counter;
    uint8_t cid[RPMB_EMMC_CID_SIZE];
    uint8_t block_data[RPMB_BLOCK_SIZE];
    uint8_t data[RPMB_DATA_SIZE];
	uint32_t datalen;
	uint32_t offset;
} UBOOT_WRITE_RPMB_0_1;

uint32_t uboot_rpmb_read_data_cus(uint8_t *data, uint32_t datalen, uint32_t operation_id);
uint32_t uboot_rpmb_write_data_cus(uint8_t *data, uint32_t datalen, uint32_t operation_id);
uint32_t uboot_rpmb_write_data(uint8_t *data, uint32_t len, uint32_t offset, uint16_t blk_addr);
uint32_t uboot_rpmb_read_data(uint8_t *data, uint32_t len, uint32_t offset, uint16_t blk_addr);
uint32_t uboot_disable_smc();
#endif
