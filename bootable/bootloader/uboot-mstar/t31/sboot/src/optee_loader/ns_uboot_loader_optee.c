#include "ns_uboot_common.h"
#include "../../../u-boot-2011.06/MstarApp/include/MsTrustZone.h"

#include "../MMAPInfo.h"
#include "../rpmb_config.h"

#if defined(__aarch64__)
#define UINT2PTR(x) ((void *)(MS_U64)x)
#define PTR2UINT(x) ((void *)(MS_U64)x)
#else
#define UINT2PTR(x) (void *)(x)
#define PTR2UINT(x) (U32)(x)
#endif

#define ALIGN(a,s) (((a)-((s) & ((a)-1))) & ((a)-1))


#define CONFIG_OPTEE_HEADER_LEN (sizeof(mstar_tee_t))
#define OPTEE_HEADER_MAGIC (0x4554504f)
#define EMMC_BLOCK_SIZE (512)
#define SEAL_ALIGN_BIT_WIDTH (12)
#define TL_BSSSECTION_SIZE 0x10000

#define SEAL_ATTR_SECURE_RW (0x03)
#define SEAL_ATTR_ALL_RW (0x0F)


#define SIGNATURE_LEN               (256)
#define RSA_PUBLIC_KEY_N_LEN (256)
#define RSA_PUBLIC_KEY_E_LEN (4)
#define RSA_PUBLIC_KEY_LEN          (RSA_PUBLIC_KEY_N_LEN+RSA_PUBLIC_KEY_E_LEN)


#define AES_IV_LEN (16)
#define AES_KEY_LEN (16)
#define HMAC_KEY_LEN (32)

// Magic flag indicating anti-rollback fields in RPMB have been initialized
#define FLAG_ANTIROLLBACK_INITIALIZED (0x42525468)
// Magic flag indicating encrypted magic string by eFuse in RPMB have been initialized
#define RMPBFSMAGIC (0x52504D42) //RPMB
#define CLEAR_MAGICSTRING "TEELOADERRPMBSTR"
#define ANTIROLLBACK_REG_ADDR (0x1f000c08)
#define ANTIROLLBACK_ENABLING_MAGIC (0xBABE)
#define ANTIROLLBACK_NOT_ENABLED_MAGIC (0xEBAB)

// Must be a positive integer less than 2^32-1
#define VERSTR_OFFSET 0x04
#define ADDITIONOPT_OFFSET 0x20

// Additional Option masks
#define MASK_DEVICELOCKMASK 0x1 /*bit 0*/

unsigned int ns_uboot_cleanup(void);
void ns_uboot_postboot(void) __attribute__((section(".ldr.teepostloader.text")));
void ns_uboot_DelayMS(unsigned int msDelay) __attribute__((__no_instrument_function__)) __attribute__((section(".ldr.teepostloader.text")));
void ns_uboot_self_clean(void) __attribute__((__no_instrument_function__)) __attribute__((section(".ldr.teepostloader.text")));
void ns_uboot_global_reset(void) __attribute__((__no_instrument_function__)) __attribute__((section(".ldr.teepostloader.text")));

struct rpmb_fs_partition {
    U32 rpmb_fs_magic;
    U32 fs_version;
    U32 write_counter;
    U32 fat_start_address;
    /* Do not use reserved[] for other purpose than partition data. */
    U8 reserved[40];
    U8  unique_itvid[24];
    U8  magicString[16];
    U32 uboot_version;
    U32 hash1_version;
    U32 teekeybank_version;
    U32 teeloader_version;
    U32 reeloader_version;
    U32 optee_version;
    U32 armfw_version;
    U32 anti_rollback_init_flag;
};

struct ftl_partition {
    U32 uboot_version;
    U32 hash1_version;
    U32 teekeybank_version;
    U32 teeloader_version;
    U32 reeloader_version;
    U32 optee_version;
    U32 armfw_version;
    U32 anti_rollback_init_flag;
    U8 reserved[224];
};//Maxim size for ftl_partition is 256 bytes

struct version_info {
    U32 hash1_version;
    U32 teeloader_version;
    U32 reeloader_version;
    U32 optee_version;
    U32 armfw_version;
};

struct ree_arg_data {
    U32 reeloader_version;
    U32 uboot_version;
    U32 rollback_enabled
};

typedef struct
{
    unsigned int ATFBase;
    unsigned int ATFSize;
    unsigned int ATFEnrty;
    unsigned int ATFFlag;
    unsigned char ATFMAC[16];
    unsigned int ATFDummy[4];
    unsigned int TLBase;
    unsigned int TLSize;
    unsigned int TLEnrty;
    unsigned int TLDummy;
} warmboot_table;

typedef struct
{
    unsigned int MultiCoreEnrty;
    unsigned int MultiCoreTrigger;
} multicore_table;

multicore_table _gMultiCoreTable __attribute__((aligned(4))) __attribute__((section(".ldr.teepostloader.static"))) = {0};

typedef struct
{
    U32 u32Num;
    U32 u32Size;
}IMAGE_INFO;

typedef struct
{
  U8 u8SecIdentify[8];
  IMAGE_INFO info;
  U8 u8Signature[SIGNATURE_LEN];
}_SUB_SECURE_INFO;


typedef struct
{
    unsigned char MagicID[12];
    unsigned int NumberOfConfig;
    unsigned int TotalConfigSize; // without descriptor
    unsigned int ConfigVersion;
    unsigned char Reserved[8];
} OPTEE_CONFIG_HEADER;

typedef struct
{
    unsigned int ConfigID;
    unsigned int Offset;
    unsigned int ConfigSize;
    unsigned int Reserved;
} OPTEE_CONFIG_DESCRIPTOR;

typedef struct
{
    unsigned int NumberOfSecureGroup;
    unsigned int NumberOfMmapID;
} OPTEE_CONFIG_MMAP_HEADER;

typedef struct
{
    unsigned int addr;
    unsigned int length;
} ST_MMAP_GROUP_ITEM;

/*
typedef struct
{
    unsigned char valid;
    unsigned int addr;
    unsigned int length;
    unsigned char MiuSel;
    unsigned int index;
} ST_MMAP_DB_ITEM;
*/
typedef struct
{
    unsigned char Dummy1[10];
    unsigned short index;
    unsigned char Dummy2[2];
} ST_MMAP_DB_ITEM;

typedef struct
{
    unsigned int          eAlgo;
    unsigned int          eSrc;   // Select KL root key source //
    unsigned int          eDst;   // Select KL output Key destination //
    unsigned int  eOutsize;  // Select ouput Key size. ex: Key size of CSA and DES are E_DSCMB_KL_64_BITS//
    unsigned int       eKeyType;
    unsigned int               u32Level;
    unsigned int               u32EngID;  // Select ESA/NSA as KL destination, Dscmb engine ID //
    unsigned int               u32DscID;  // Select ESA/NSA as KL destination, Dscmb ID //
    unsigned long long   u8KeyACPU;  // Select ACPU as KL root Key, KL root key //
    unsigned long long   pu8KeyKLIn;
    unsigned char              bDecrypt;
    unsigned char              bInverse;
    unsigned int      eKLSel;   //Select KeyLadder
    unsigned int               u32CAVid; //Set CAVid
    unsigned int         stKDF[3];
    unsigned int           eFSCB; // Key-specific FSCB
} DSCMB_KLCfg_All64;

unsigned int ns_uboot_boot(unsigned int uboot_addr);
unsigned int ns_uboot_load_optee(void);
unsigned int ns_uboot_load_armfw(void);
unsigned int ns_uboot_get_optee_addr(void);
unsigned int ns_uboot_nonsecure_handler(void);

Rpmb_Config RpmbConfig = {0};

// For STR
static int _gIsStr = 0;
static int _gStrMode = COLD_BOOT;
static int _gAuthMode = 0;

//For OpteeOS consistency checking
#if defined(CONFIG_OPTEE_ANTIBRICK)
static char _gOpteeConsisStatus = 0;
#define RTC_BANK 0x1F002440
typedef enum
{
	OPTEENEVER=0,
	OPTEECHECKIN,
	OPTEECHECKOUT,
}OpteeConsisStatus;
#endif
#define DBG_BANK 0x1F206704
#define DUALLOADER_BANK 0x1F002644

typedef enum
{
	TLRELOADED=0,
	TLRESUMED,
}TLRELOADSTATUS;

typedef enum
{
  DEFAULTTYPE=0x0,//MTK Default type
	CUSM1TYPE1=0x1,//M1 TYPE for E_AESDMA_RPMB_DRMMODE mode
	CUSM1TYPE2=0x2,//M1 TYPE for E_AESDMA_RPMB_CIDMODE mode
}TLCUSTOMIZEDTYPE;

typedef enum
{
  LOADMBOOTB=0x0,//Load MBOOTB
  LOADMBOOT=0x1,//Load MBOOT
}TLDUALLOADERSTATUS;

//For group secure range
static unsigned int _gGroupSecureMemFlag = FALSE;
static unsigned int _gGroupSecureMemAddr_lo = 0;
static unsigned int _gGroupSecureMemsize = 0;

static unsigned long long BIN0_entry_point = 0;
static unsigned long long BIN0_end_point = 0;
static unsigned short _gIsTLreloaded __attribute__((section(".ldr.teepostloader.static")))= TLRELOADED;
static char _gOpteeBootArgs[90] __attribute__((aligned(8))) __attribute__((section(".ldr.teepostloader.static")));
static mstar_boot_prameters_t boot  __attribute__((aligned(8))) __attribute__((section(".ldr.teepostloader.static")));
static int SecureBootTeeOK __attribute__((section(".ldr.teepostloader.static"))) = TRUE;
static unsigned long Fireware_entry __attribute__((section(".ldr.teepostloader.static"))) = 0;
static unsigned long Fireware_end __attribute__((section(".ldr.teepostloader.static"))) = 0;
const unsigned char EmbeddedTL_A_TI_KEY[] __attribute__((aligned(16)))= {
    #include "../../TL_A_TI_KEY.dat"
};
const unsigned char EmbeddedTL_A_REE_KEY[] __attribute__((aligned(16)))= {
    #include "../../TL_A_REE_KEY.dat"
};
unsigned char EmbeddedTL_D_TI_KEY[] __attribute__((aligned(16)))={
    #include "../../TL_D_TI_KEY.dat"
};
unsigned char EmbeddedTL_D_REE_KEY[] __attribute__((aligned(16)))= {
    #include "../../TL_D_REE_KEY.dat"
};

#if defined(CONFIG_TEE_LOADER_HWAES_TEST)
unsigned char EmbeddedAES_TestInput[] __attribute__((aligned(16)))= {
    #include "../../AES_TestInput.dat"
};
void AES_TestInput(void);
#endif

unsigned int setvector_code[]={
    0xaa0003fa,        //mov     x26, x0
    0xd51ec01a,        //msr     vbar_el3, x26
    0xd69f03e0         //eret
};
unsigned int setvector_code_TeeFail[]={
    0xaa0003fa,        //mov     x26, x0
    0xd51ec01a,        //msr     vbar_el3, x26
    0xd2800007,        //mov     x7, #0x0                        // #0
    0xb9400027,        //ldr     w7, [x1]
    0xd51e4027,        //msr     elr_el3, x7
    0xd53e4001,        //mrs     x1, spsr_el3
    0x927af821,        //and     x1, x1, #0xffffffffffffffdf
    0xd51e4001,        //msr     spsr_el3, x1
    0xd53e1101,        //mrs     x1, scr_el3
    0xb2400021,        //orr     x1, x1, #0x1 //Enable NS
    0x9278f821,        //and     x1, x1, #0xffffffffffffff7f //Enable SMD
    0xd51e1101,        //msr     scr_el3, x1
    0xd69f03e0         //eret
};

extern unsigned int _ld_TEE_LDR_run_base;
extern unsigned int _ld_TEE_LDR_load_start,_ld_TEE_LDR_load_end;
extern unsigned int _ld_TEE_LDR_run_start,_ld_TEE_LDR_run_end;
extern unsigned int _ld_TEE_LDR_size;
extern unsigned int _CONFIG_SRAM_START_ADDRESS,_CONFIG_SRAM_CODE_SIZE;
extern unsigned int _ld_TEE_LDR_vector_start;
extern unsigned int _ld_TEE_LDR_arg_start;
extern unsigned int _ld_TLbss_start,_ld_TLbss_end;

#define TPMBUFFERSIZE 512*3
static const U8 u8TmpBuf[TPMBUFFERSIZE] __attribute__((aligned(16)));

static U8 u8ATF_MAC[16] __attribute__((aligned(16)));
static U8 u8VerCtrlBuf[0x100] __attribute__((aligned(16)));

#define AUTH_KEYBANK_FAIL 1
#define AUTH_ARMFW_FAIL 2
#define AUTH_OPTEE_FAIL 3
#define MBOOT_PARTITION               "MBOOT"
#define MBOOTB_PARTITION              "MBOOTB"
#define TEE_PARTITION                 "optee"
#define ATF_PARTITION                 "armfw"
#define COMBINEDTEE_PARTITION          "tzfw"
#define MBOOT_PARTITION_LEN            5
#define MBOOTB_PARTITION_LEN           6
#define TEE_PARTITION_LEN              5
#define ATF_PARTITION_LEN              5
#define COMBINEDTEE_PARTITION_LEN      4
static unsigned int _gOpteePartOffset = 0;

static struct version_info _gVersionInfo = {0};
static struct ree_arg_data gRee_argData;

#define IS_MBOOTBAK() ((*(volatile unsigned int*)(CONFIG_RIU_BASE_ADDRESS + (0x103380<<1)))&0x0100)

unsigned int ns_uboot_anti_rollback(void)
{
    static U8 buf[0x100] __attribute__((aligned(16)));
    _gVersionInfo.hash1_version = *(volatile unsigned short*)(0x1F000000+(0x0600<<1));
    _gVersionInfo.teeloader_version = *((unsigned int*)((unsigned int)&_ld_TEE_LDR_run_base+VERSTR_OFFSET));
    bootargs_teeloader_t* argument = (void *)&_ld_TEE_LDR_arg_start;
    _memset(&gRee_argData,0xFFFFFFFF,sizeof(struct ree_arg_data));
    unsigned short anti_rollback_reg = *(volatile unsigned short *)ANTIROLLBACK_REG_ADDR;
#if defined(CONFIG_PROGRAM_EMMC_RPMB_KEY)
    struct rpmb_fs_partition *rpmb = (struct rpmb_fs_partition*)buf;
    U8* AESKey = RpmbConfig.select_key == 1 ? RpmbConfig.rootkey : NULL;

    if(eMMC_RPMB_Read_data(buf, 0x100, 0, AESKey) != eMMC_ST_SUCCESS)
    {
        LDR_PUTS("~eMMC_RPMB_Read_data fail!\n");
        return FALSE;
    }

    array_reverse(buf,0x100);
    _memcpy(u8VerCtrlBuf,buf,sizeof(u8VerCtrlBuf));
    gRee_argData.reeloader_version = rpmb->reeloader_version;
    gRee_argData.uboot_version = rpmb->uboot_version;
    gRee_argData.rollback_enabled = 1;
    _memcpy(argument->ree_arg_addr,&gRee_argData,sizeof(struct ree_arg_data));
    if ( rpmb->anti_rollback_init_flag == FLAG_ANTIROLLBACK_INITIALIZED &&
        (rpmb->teeloader_version > _gVersionInfo.teeloader_version || rpmb->optee_version > _gVersionInfo.optee_version || rpmb->armfw_version > _gVersionInfo.armfw_version  || rpmb->hash1_version > _gVersionInfo.hash1_version) )
    {
        LDR_PUTS("Dump RPMB\n");
        LDR_DUMP(rpmb,sizeof(struct rpmb_fs_partition));
        LDR_PUTS("Dump CurVerInfo\n");
        LDR_DUMP(&_gVersionInfo,sizeof(struct version_info));
        return FALSE;
    }
    else if ( rpmb->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED ||
              rpmb->teeloader_version < _gVersionInfo.teeloader_version || rpmb->optee_version < _gVersionInfo.optee_version || rpmb->armfw_version < _gVersionInfo.armfw_version || rpmb->hash1_version < _gVersionInfo.hash1_version)
    {
        if (rpmb->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED && anti_rollback_reg != ANTIROLLBACK_ENABLING_MAGIC)
        {
            gRee_argData.rollback_enabled = 0;
            _memcpy(argument->ree_arg_addr,&gRee_argData,sizeof(struct ree_arg_data));
            /* set the register to indicate that anti-rollback is not enabled */
            *(volatile unsigned short *)ANTIROLLBACK_REG_ADDR = ANTIROLLBACK_NOT_ENABLED_MAGIC;
            LDR_PUTS("\nAnti-rollback not enabled\n");
            return TRUE;
        }
        rpmb->hash1_version = _gVersionInfo.hash1_version;
        rpmb->teeloader_version = _gVersionInfo.teeloader_version;
        rpmb->optee_version = _gVersionInfo.optee_version;
        rpmb->armfw_version = _gVersionInfo.armfw_version;
        _memcpy(u8VerCtrlBuf,buf,sizeof(u8VerCtrlBuf));//Copy to u8VerCtrlBuf again since version numbers are initialized except anti_rollback_init_flag
        rpmb->anti_rollback_init_flag = FLAG_ANTIROLLBACK_INITIALIZED;
        LDR_PUTS("+");
        array_reverse(buf,0x100);
        if(eMMC_RPMB_Write_data(buf, 0x100, 0, AESKey) != eMMC_ST_SUCCESS)
        {
            LDR_PUTS("~eMMC_RPMB_Write_data fail!\n");
            return FALSE;
        }
    }

#elif defined(CONFIG_MSTAR_FTL_SD)
    static U8 buf_verify[0x100] __attribute__((aligned(16)));
    struct ftl_partition *ftl_part = (struct ftl_partition*)buf;

    if(FTLSd_ReadData(buf, 0x100)!=0)
    {
        LDR_PUTS("~FTLSd_ReadData fail!\n");
        return FALSE;
    }

    _memcpy(u8VerCtrlBuf,buf,sizeof(u8VerCtrlBuf));
    gRee_argData.reeloader_version = ftl_part->reeloader_version;
    gRee_argData.uboot_version = ftl_part->uboot_version;
    gRee_argData.rollback_enabled = 1;
    _memcpy(argument->ree_arg_addr,&gRee_argData,sizeof(struct ree_arg_data));
    if ( ftl_part->anti_rollback_init_flag == FLAG_ANTIROLLBACK_INITIALIZED &&
       ( ftl_part->teeloader_version > _gVersionInfo.teeloader_version || ftl_part->optee_version > _gVersionInfo.optee_version || ftl_part->armfw_version > _gVersionInfo.armfw_version || ftl_part->hash1_version > _gVersionInfo.hash1_version) )
    {
        LDR_PUTS("Dump FTL\n");
        LDR_DUMP(ftl_part,sizeof(struct ftl_partition));
        LDR_PUTS("Dump CurVerInfo\n");
        LDR_DUMP(&_gVersionInfo,sizeof(struct version_info));
        return FALSE;
    }
    else if (ftl_part->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED ||
             ftl_part->teeloader_version < _gVersionInfo.teeloader_version || ftl_part->optee_version < _gVersionInfo.optee_version || ftl_part->armfw_version < _gVersionInfo.armfw_version || ftl_part->hash1_version < _gVersionInfo.hash1_version)
    {
        if (ftl_part->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED && anti_rollback_reg != ANTIROLLBACK_ENABLING_MAGIC)
        {
            gRee_argData.rollback_enabled = 0;
            _memcpy(argument->ree_arg_addr,&gRee_argData,sizeof(struct ree_arg_data));
            /* set the register to indicate that anti-rollback is not enabled */
            *(volatile unsigned short *)ANTIROLLBACK_REG_ADDR = ANTIROLLBACK_NOT_ENABLED_MAGIC;
            LDR_PUTS("\nAnti-rollback not enabled\n");
            return TRUE;
        }
        _memset(ftl_part,0,0x100);
        ftl_part->hash1_version = _gVersionInfo.hash1_version;
        ftl_part->teeloader_version = _gVersionInfo.teeloader_version;
        ftl_part->optee_version = _gVersionInfo.optee_version;
        ftl_part->armfw_version = _gVersionInfo.armfw_version;
        _memcpy(u8VerCtrlBuf,buf,sizeof(u8VerCtrlBuf));//Copy to u8VerCtrlBuf again since version numbers are initialized except anti_rollback_init_flag
        ftl_part->anti_rollback_init_flag = FLAG_ANTIROLLBACK_INITIALIZED;
        LDR_PUTS("+");
        if(FTLSd_WriteData(buf, 0x100)!=0)
        {
            LDR_PUTS("~FTLSd_WriteData fail!\n");
            return FALSE;
        }
        //Read it out again to prevent arbitrary NNI size attack
        if(FTLSd_ReadData(buf_verify, 0x100)!=0)
        {
            LDR_PUTS("~FTLSd_ReadData verify fail!\n");
            //hang the system to prevent incorrect initialization of nand secure range
            ns_uboot_nonsecure_handler();
            ns_uboot_global_reset();
        }
        if ( 0 != _memcmp( buf, buf_verify, 0x100) )
        {
            LDR_PUTS("~FTLSd_WriteData verify fail!\n");
            //hang the system to prevent incorrect initialization of nand secure range
            ns_uboot_nonsecure_handler();
            ns_uboot_global_reset();
        }
        //
    }

#endif
    return TRUE;
}

static unsigned int ns_uboot_is_str(void)
{
    unsigned short val = *(volatile unsigned short*)(0x1F000000 + (0x3964<<1));
    *(volatile unsigned short*)(0x1F000000 + (0x3964<<1)) &= 0x00FF; // clear STR flag
    return ((val & 0xFF00) == 0x5500) || ((val & 0xFF00) == 0x6600);
}

static unsigned int ns_uboot_get_dualloader_status(void)
{
    return RREG16(DUALLOADER_BANK)&0x0001;
}

static void ns_uboot_set_dualloader_clear_status()
{
    unsigned short curVal = RREG16(DUALLOADER_BANK);
    curVal = curVal&0xFFFE;
    WREG16(DUALLOADER_BANK,curVal);
}

unsigned int ns_uboot_support_faststr(void)
{
    return _gIsStr && (_gStrMode == FAST_BOOT);
}

unsigned ns_uboot_check_boundary(unsigned int ree_entry,unsigned int ree_size)
{
	  //Check ree_size, maximum allowed size is 16MB
	  if(ree_size > 0x1000000)
	  {
	      return FALSE;
	  }
	  //Check overflow
	  if(ree_entry+ree_size < ree_entry)
    {
        return FALSE;
    }
	  //Check if ree_entry overlaps teeloader
	  if(!(ree_entry < (unsigned int)&_ld_TEE_LDR_load_start && ree_entry < (unsigned int)&_ld_TEE_LDR_load_end+TL_BSSSECTION_SIZE && ree_entry + ree_size <= (unsigned int)&_ld_TEE_LDR_load_start && ree_entry + ree_size < (unsigned int)&_ld_TEE_LDR_load_end+TL_BSSSECTION_SIZE ||
         ree_entry > (unsigned int)&_ld_TEE_LDR_load_start && ree_entry >= (unsigned int)&_ld_TEE_LDR_load_end+TL_BSSSECTION_SIZE && ree_entry + ree_size > (unsigned int)&_ld_TEE_LDR_load_start && ree_entry + ree_size > (unsigned int)&_ld_TEE_LDR_load_end+TL_BSSSECTION_SIZE))
	  {
	      return FALSE;
	  }
	  //Check if ree_entry overlaps armfw, armfw size is presumed to be 128KB(0x20000)
    if(!(ree_entry < (unsigned int)Fireware_entry && ree_entry < (unsigned int)Fireware_entry+0x20000 && ree_entry + ree_size <= (unsigned int)Fireware_entry && ree_entry + ree_size < (unsigned int)Fireware_entry+0x20000 ||
         ree_entry > (unsigned int)Fireware_entry && ree_entry >= (unsigned int)Fireware_entry+0x20000 && ree_entry + ree_size > (unsigned int)Fireware_entry && ree_entry + ree_size > (unsigned int)Fireware_entry+0x20000))
	  {
	      return FALSE;
	  }
	  //Check if ree_entry overlaps armfw, optee size is presumed to be 4MB(0x400000)
	  if(!(ree_entry < (unsigned int)BIN0_entry_point && ree_entry < (unsigned int)BIN0_entry_point+0x400000 && ree_entry + ree_size <= (unsigned int)BIN0_entry_point && ree_entry + ree_size < (unsigned int)BIN0_entry_point+0x400000 ||
         ree_entry > (unsigned int)BIN0_entry_point && ree_entry >= (unsigned int)BIN0_entry_point+0x400000 && ree_entry + ree_size > (unsigned int)BIN0_entry_point && ree_entry + ree_size > (unsigned int)BIN0_entry_point+0x400000))
	  {
	      return FALSE;
	  }
	  //Check if ree_entry overlaps secure group memory
	  if( _gGroupSecureMemFlag &&
       (!(ree_entry < (unsigned int)_gGroupSecureMemAddr_lo && ree_entry < (unsigned int)_gGroupSecureMemAddr_lo+_gGroupSecureMemsize && ree_entry + ree_size <= (unsigned int)_gGroupSecureMemAddr_lo && ree_entry + ree_size < (unsigned int)_gGroupSecureMemAddr_lo+_gGroupSecureMemsize ||
          ree_entry > (unsigned int)_gGroupSecureMemAddr_lo && ree_entry >= (unsigned int)_gGroupSecureMemAddr_lo+_gGroupSecureMemsize && ree_entry + ree_size > (unsigned int)_gGroupSecureMemAddr_lo && ree_entry + ree_size > (unsigned int)_gGroupSecureMemAddr_lo+_gGroupSecureMemsize)))
	  {
	      return FALSE;
	  }
    return TRUE;
}

unsigned ns_uboot_load_reeloader(unsigned int ree_entry,unsigned int ree_size,unsigned int ree_opt)
{
    //ree_opt bit0:bAuthReeloader
    //ree_opt bit1:bDecReeloader
    //ree_opt bit2:bBypassLoadReeloader
    unsigned int auth_ok = TRUE;
    bootargs_teeloader_t* argument = (void *)&_ld_TEE_LDR_arg_start;
    unsigned short anti_rollback_reg = *(volatile unsigned short *)ANTIROLLBACK_REG_ADDR;

    LDR_PUTS("ree_opt=");LDR_PUTDW(ree_opt);LDR_PUTS("\n");
    if(ree_opt & 0x04)
    {
        LDR_PUTS("Bypassing loading reeloader...\n");
        return auth_ok;
    }
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    eMMC_LoadImages(ree_entry, ree_size, (2 * (unsigned int)(&_CONFIG_SRAM_CODE_SIZE)+(unsigned int)(&_ld_TEE_LDR_size))/512);
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
    drvNAND_LoadBLOffsetDma(UINT2PTR(ree_entry), ((unsigned int)(&_CONFIG_SRAM_CODE_SIZE))+((unsigned int)(&_ld_TEE_LDR_size)), ree_size);
#endif
    if(auth_ok && ree_opt & 0x01)
    {
        LDR_PUTS("Auth reeloader...\n");
        auth_ok = MDrv_AESDMA_SecureMain_v2(UINT2PTR(ree_entry),UINT2PTR(ree_size-0x400),UINT2PTR(ree_entry+ree_size-0x100),EmbeddedTL_A_REE_KEY);
    }
    if(auth_ok && ree_opt & 0x02)
    {
        LDR_PUTS("Decrypt reeloader...\n");
        auth_ok = MDrv_AESDMA_Decrypt(__BA2PA(ree_entry), ree_size-0x400, EmbeddedTL_D_REE_KEY, AESDMA_ENGINE_CBC, E_AESDMA_SWKEY);
    }
    if(auth_ok == FALSE)
    {
        return FALSE;
    }

    _gVersionInfo.reeloader_version = *(unsigned int*)(ree_entry+ree_size-0x400-0x4);
    gRee_argData.reeloader_version = _gVersionInfo.reeloader_version;
    _memcpy(argument->ree_arg_addr,&gRee_argData,sizeof(struct ree_arg_data));
#if defined(CONFIG_PROGRAM_EMMC_RPMB_KEY)
    LDR_PUTS("Version check on reeloader...\n");
    U8* AESKey = RpmbConfig.select_key == 1 ? RpmbConfig.rootkey : NULL;
    struct rpmb_fs_partition *rpmb = (struct rpmb_fs_partition*)u8VerCtrlBuf;
    if ( rpmb->anti_rollback_init_flag == FLAG_ANTIROLLBACK_INITIALIZED && rpmb->reeloader_version > _gVersionInfo.reeloader_version )
    {
        LDR_PUTS("Dump RPMB\n");
        LDR_DUMP(rpmb,sizeof(struct rpmb_fs_partition));
        LDR_PUTS("Dump CurVerInfo\n");
        LDR_DUMP(&_gVersionInfo,sizeof(struct version_info));
        return FALSE;
    }
    else if ( rpmb->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED || rpmb->reeloader_version < _gVersionInfo.reeloader_version)
    {
        if (rpmb->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED && anti_rollback_reg != ANTIROLLBACK_ENABLING_MAGIC)
        {
            LDR_PUTS("\nAnti-rollback not enabled\n");
            return TRUE;
        }
        else if(rpmb->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED)
        {
            //Pass initialized uboot version to REE once again
            rpmb->uboot_version = 0;
            gRee_argData.uboot_version = 0;
            _memcpy(argument->ree_arg_addr,&gRee_argData,sizeof(struct ree_arg_data));
        }
        rpmb->reeloader_version = _gVersionInfo.reeloader_version;
        rpmb->anti_rollback_init_flag = FLAG_ANTIROLLBACK_INITIALIZED;
        LDR_PUTS("+");
        array_reverse(rpmb,0x100);
        if(eMMC_RPMB_Write_data(rpmb, 0x100, 0, AESKey) != eMMC_ST_SUCCESS)
        {
            LDR_PUTS("~eMMC_RPMB_Write_data fail!\n");
            return FALSE;
        }
    }
#elif defined(CONFIG_MSTAR_FTL_SD)
    LDR_PUTS("Version check on reeloader...\n");
    struct ftl_partition *ftl_part = (struct ftl_partition*)u8VerCtrlBuf;
    static U8 buf_verify[0x100] __attribute__((aligned(16)));
    if ( ftl_part->anti_rollback_init_flag == FLAG_ANTIROLLBACK_INITIALIZED && ftl_part->reeloader_version > _gVersionInfo.reeloader_version )
    {
        LDR_PUTS("Dump FTL\n");
        LDR_DUMP(ftl_part,sizeof(struct ftl_partition));
        LDR_PUTS("Dump CurVerInfo\n");
        LDR_DUMP(&_gVersionInfo,sizeof(struct version_info));
        return FALSE;
    }
    else if (ftl_part->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED || ftl_part->reeloader_version < _gVersionInfo.reeloader_version )
    {
        if (ftl_part->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED && anti_rollback_reg != ANTIROLLBACK_ENABLING_MAGIC)
        {
            LDR_PUTS("\nAnti-rollback not enabled\n");
            return TRUE;
        }
        else if(ftl_part->anti_rollback_init_flag != FLAG_ANTIROLLBACK_INITIALIZED )
        {
            //Pass initialized uboot version to REE once again
            ftl_part->uboot_version = 0;
            gRee_argData.uboot_version = 0;
            _memcpy(argument->ree_arg_addr,&gRee_argData,sizeof(struct ree_arg_data));
        }
        ftl_part->reeloader_version = _gVersionInfo.reeloader_version;
        ftl_part->anti_rollback_init_flag = FLAG_ANTIROLLBACK_INITIALIZED;
        LDR_PUTS("+");
        if(FTLSd_WriteData(ftl_part, 0x100)!=0)
        {
            LDR_PUTS("~FTLSd_WriteData fail!\n");
            return FALSE;
        }
        //Read it out again to prevent arbitrary NNI size attack
        if(FTLSd_ReadData(buf_verify, 0x100)!=0)
        {
            LDR_PUTS("~FTLSd_ReadData verify fail!\n");
            //hang the system to prevent incorrect initialization of nand secure range
            return FALSE;
        }
        if ( 0 != _memcmp( ftl_part, buf_verify, 0x100) )
        {
            LDR_PUTS("~FTLSd_WriteData verify fail!\n");
            //hang the system to prevent incorrect initialization of nand secure range
            return FALSE;
        }
        //
    }
#endif
    /* clear the register */
    *(volatile unsigned short *)ANTIROLLBACK_REG_ADDR = 0;
    return auth_ok;
}

unsigned ns_uboot_load_CKB(unsigned int CKB_offset,unsigned int CKB_addr,unsigned int CKB_size,unsigned int ree_opt)
{
    //ree_opt bit3:bAuthCKB
    //ree_opt bit4:bDecCKB
    //ree_opt bit5:bEnableCKB
    unsigned int auth_ok = TRUE;

    if(!(ree_opt & 0x20))
    {
        return auth_ok;
    }
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    emmc_partition_t mpart;
    mstar_tee_t* header = NULL;
    unsigned int loaderStatus = ns_uboot_get_dualloader_status();
    if(loaderStatus == LOADMBOOTB)
    {
        LDR_PUTS("Load MBOOTB\n");
    }
    if (ns_uboot_load_partition(loaderStatus == LOADMBOOTB?MBOOTB_PARTITION_LEN:MBOOT_PARTITION_LEN, loaderStatus == LOADMBOOTB?MBOOTB_PARTITION:MBOOT_PARTITION, &header, &mpart))
    {
        eMMC_ReadData_MIU((U8*)UINT2PTR(CKB_addr), CKB_size, mpart.start_block + (CKB_offset>>9));
    }
    else
    {
        LDR_PUTS("MBOOT part not found!\n");
        auth_ok = FALSE;
    }
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
    drvNAND_LoaduBoot((void*)CKB_addr, CKB_size+CKB_offset);//Read chunk header and CKB from NAND to CKB_addr
    _memcpy((void*)CKB_addr,(void*)CKB_addr+CKB_offset,CKB_size);//Copy CKB part to CKB_addr
    _memset((void*)CKB_addr+CKB_size,0,CKB_offset);//Clean the rest of redundent CKB data after CKB_addr+CKB_size
#endif
    if(auth_ok && ree_opt & 0x08)
    {
        LDR_PUTS("Auth CKB...\n");
        auth_ok = MDrv_AESDMA_SecureMain_v2(UINT2PTR(CKB_addr+0x110),CKB_size-0x110,UINT2PTR(CKB_addr+0x10),EmbeddedTL_A_REE_KEY);
        if(!auth_ok)
        {
            //Clear boot status bit0 if CKB auth fails
            ns_uboot_set_dualloader_clear_status();
        }
    }
    if(auth_ok && ree_opt & 0x10)
    {
        LDR_PUTS("Decrypt CKB...\n");
        auth_ok = MDrv_AESDMA_Decrypt(__BA2PA(CKB_addr+0x110), CKB_size-0x110, EmbeddedTL_D_REE_KEY, AESDMA_ENGINE_CBC, E_AESDMA_SWKEY);
    }

    return auth_ok;
}

unsigned int ns_uboot_load_optee(void)
{
    mstar_tee_t* header = NULL;
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    emmc_partition_t mpart;
#endif
    OPTEE_CONFIG_HEADER* opteeConfigHeader = NULL;
    U32 tmpBuf = (U32)u8TmpBuf;
    unsigned int u32HeadSize = sizeof(mstar_tee_t) + sizeof(OPTEE_CONFIG_HEADER) + sizeof(_SUB_SECURE_INFO);
    unsigned int u32HeadeMMCAlign = ALIGN(EMMC_BLOCK_SIZE, u32HeadSize);
    unsigned int u32HeadeMMCBlockNum = (u32HeadSize + u32HeadeMMCAlign) / EMMC_BLOCK_SIZE;
    bootargs_teeloader_t* argument = (void*)&_ld_TEE_LDR_arg_start;

    if(_gIsTLreloaded == TLRELOADED)
    {
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
        if (argument->CustomizeID == DEFAULTTYPE && ns_uboot_load_partition(TEE_PARTITION_LEN, TEE_PARTITION, &header, &mpart))
        {
            eMMC_ReadData_MIU((U8*)UINT2PTR(tmpBuf), u32HeadSize + u32HeadeMMCAlign, mpart.start_block);
        }
        else if ((argument->CustomizeID == CUSM1TYPE1 || argument->CustomizeID == CUSM1TYPE2) && ns_uboot_load_partition(COMBINEDTEE_PARTITION_LEN, COMBINEDTEE_PARTITION, &header, &mpart))
        {
            mpart.start_block += (_gOpteePartOffset>>9);
            eMMC_ReadData_MIU((U8*)UINT2PTR(tmpBuf), u32HeadSize + u32HeadeMMCAlign, mpart.start_block);
        }
        else
        {
            // partition 'optee' not found
            return FALSE;
        }
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
        if (UNFD_ST_SUCCESS != drvNAND_ReadPartition(UNFD_PART_OPTEE, 0, tmpBuf, 1)) // read 512-byte to get header and signature
        {
            return FALSE; // nand read fail
        }
#endif

        // verify header
        {
            if (FALSE == MDrv_AESDMA_SecureMain_v2(tmpBuf, CONFIG_OPTEE_HEADER_LEN+sizeof(OPTEE_CONFIG_HEADER), ((_SUB_SECURE_INFO*)(tmpBuf+CONFIG_OPTEE_HEADER_LEN+sizeof(OPTEE_CONFIG_HEADER)))->u8Signature, EmbeddedTL_A_TI_KEY))
            {
                // optee header auth fail
                return FALSE;
            }
            else
            {
                header = (mstar_tee_t*)UINT2PTR(tmpBuf);
                opteeConfigHeader = (OPTEE_CONFIG_HEADER*)(tmpBuf+CONFIG_OPTEE_HEADER_LEN);
            }
        }
        
        // make sure binary is really optee
        if ( 0 != _memcmp( &(header->magic), "OPTE", 4) )
        {
            return FALSE;
        }

#if defined(CONFIG_ARMv8_ARM_TRUSTED_FIRMWARE)
        //Check _gGroupSecureMemAddr_lo consistency
        if(_gGroupSecureMemAddr_lo != header->init_group_addr_lo)
        {
            LDR_PUTS("init_group_addr_lo is different between armfw and optee\n");
            return FALSE;
        }
        //Check _gGroupSecureMemsize consistency
        if(_gGroupSecureMemsize != header->group_size)
        {
            LDR_PUTS("group_size is different between armfw and optee\n");
            return FALSE;
        }
#else
        _gGroupSecureMemFlag = (header->init_group_addr_lo != 0);
        if(_gGroupSecureMemFlag)//Use secure group memory as secure memory
        {
            _gGroupSecureMemAddr_lo = header->init_group_addr_lo;
            _gGroupSecureMemsize = header->group_size;
        }
#endif

        //Consistency check between secure group memory and independent secure memory
        if(_gGroupSecureMemFlag && !(header->init_load_addr_lo >= _gGroupSecureMemAddr_lo && header->secure_range_end <= _gGroupSecureMemAddr_lo + _gGroupSecureMemsize))
        {
            LDR_PUTS("secure range consistency check failed\n");
            return FALSE;
        }
        
        BIN0_entry_point = header->init_load_addr_lo;
        BIN0_end_point = header->secure_range_end;
        _gVersionInfo.optee_version = header->cus_version;

#if !defined(CONFIG_ARMv8_ARM_TRUSTED_FIRMWARE)
        _gStrMode = ( header->warmboot_mode & STR_MODE_MASK );
#endif

#if defined(CONFIG_PROGRAM_EMMC_RPMB_KEY)
        if (argument->CustomizeID == CUSM1TYPE1)
        {
            LDR_PUTS("Set E_AESDMA_RPMB_DRMMODE\n");
            SetHmacKeyMode(E_AESDMA_RPMB_DRMMODE);
        }
#endif
    }

    if(!_gGroupSecureMemFlag && !MDrv_SEAL_SetSecureRange((unsigned long long)__BA2PA(BIN0_entry_point), (unsigned long long)__BA2PA(BIN0_end_point), SEAL_ATTR_SECURE_RW))
    {
        LDR_PUTS("secure range setup failed\n");
        return FALSE;
    }
    else if(_gGroupSecureMemFlag && !MDrv_SEAL_SetSecureRange(__BA2PA(_gGroupSecureMemAddr_lo), __BA2PA(_gGroupSecureMemAddr_lo+_gGroupSecureMemsize), SEAL_ATTR_SECURE_RW))
    {
        LDR_PUTS("secure range setup failed\n");
        return FALSE;
    }
    if ( _gIsTLreloaded != TLRELOADED || ( _gIsStr && _gStrMode ))
    {
        return TRUE;
    }

    // copy header part to load address
    _memcpy(UINT2PTR(header->init_load_addr_lo), UINT2PTR(tmpBuf), u32HeadSize + u32HeadeMMCAlign);

    // read other body data
    unsigned int u32BodyAlign = ALIGN(16,header->init_size) + ALIGN(16,opteeConfigHeader->TotalConfigSize) + opteeConfigHeader->NumberOfConfig*sizeof(OPTEE_CONFIG_DESCRIPTOR) + opteeConfigHeader->TotalConfigSize; // Body dummy data
    unsigned int u32BodyeMMCAlign = ALIGN(EMMC_BLOCK_SIZE, header->init_size + u32BodyAlign + sizeof(_SUB_SECURE_INFO) - u32HeadeMMCAlign);
    unsigned int u32BodyOtherSizeeMMC = header->init_size + u32BodyAlign + sizeof(_SUB_SECURE_INFO) - u32HeadeMMCAlign + u32BodyeMMCAlign;
    unsigned int u32CusConfigOffset = header->init_size + u32BodyAlign + u32HeadSize + ALIGN(16,sizeof(_SUB_SECURE_INFO)) + sizeof(_SUB_SECURE_INFO);
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    unsigned int u32CusConfigeMMCBlockOffset = mpart.start_block + (unsigned int)((u32CusConfigOffset+ALIGN(EMMC_BLOCK_SIZE,u32CusConfigOffset))>>9);
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
    unsigned int u32CusConfigeMMCBlockOffset = (unsigned int)((u32CusConfigOffset+ALIGN(EMMC_BLOCK_SIZE,u32CusConfigOffset))>>9);
#endif

#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    eMMC_ReadData_MIU(UINT2PTR(header->init_load_addr_lo + u32HeadSize + u32HeadeMMCAlign), u32BodyOtherSizeeMMC, mpart.start_block + u32HeadeMMCBlockNum);
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
    if (UNFD_ST_SUCCESS == drvNAND_ReadPartition(UNFD_PART_OPTEE, 0, header->init_load_addr_lo, u32BodyOtherSizeeMMC/EMMC_BLOCK_SIZE + u32HeadeMMCBlockNum)) // read header with body
    {
        // because here we have loaded header from flash again, to prevent TOCTOU issue, we first replace the header part with the previously verified copy, before verify header+body as a whole
        _memcpy(UINT2PTR(header->init_load_addr_lo), UINT2PTR(tmpBuf), u32HeadSize);
    }
    else
    {
        return FALSE; // read optee body fail
    }
#endif

    //verify optee body
    {
        int auth_ok = MDrv_AESDMA_SecureMain_v2(header->init_load_addr_lo, header->init_size + u32BodyAlign + u32HeadSize, ((_SUB_SECURE_INFO*)(header->init_load_addr_lo + header->init_size + u32BodyAlign + u32HeadSize))->u8Signature, EmbeddedTL_A_TI_KEY);
        if (!auth_ok)
        {
            // optee body auth fail
            return FALSE;
        }

        _memcpy(UINT2PTR(header->init_load_addr_lo), UINT2PTR(header->init_load_addr_lo + u32HeadSize), header->init_size + u32BodyAlign);
        if (MDrv_AESDMA_Decrypt(__BA2PA(header->init_load_addr_lo), header->init_size + u32BodyAlign, EmbeddedTL_D_TI_KEY, AESDMA_ENGINE_CBC, E_AESDMA_SWKEY) == FALSE)
        {
            // optee body decrypt fail
            return FALSE;
        }

        // parsing config data
        OPTEE_CONFIG_DESCRIPTOR* desc = (OPTEE_CONFIG_DESCRIPTOR*)(header->init_load_addr_lo + header->init_size + ALIGN(16, header->init_size) );
        unsigned char* dataOffset = (void*)(header->init_load_addr_lo + header->init_size + ALIGN(16, header->init_size) + opteeConfigHeader->NumberOfConfig*sizeof(OPTEE_CONFIG_DESCRIPTOR));

        int i;
        for (i=0; i<opteeConfigHeader->NumberOfConfig; i++, desc++)
        {
            if (desc->ConfigID==1) // mmap
            {
                OPTEE_CONFIG_MMAP_HEADER* mmapHeader = (OPTEE_CONFIG_MMAP_HEADER*)(dataOffset + desc->Offset);
                ST_MMAP_GROUP_ITEM* groupItem = (ST_MMAP_GROUP_ITEM*)(dataOffset + desc->Offset + sizeof(OPTEE_CONFIG_MMAP_HEADER));
                ST_MMAP_DB_ITEM* dbItem = (ST_MMAP_DB_ITEM*)(dataOffset + desc->Offset + sizeof(OPTEE_CONFIG_MMAP_HEADER) + mmapHeader->NumberOfSecureGroup*sizeof(ST_MMAP_GROUP_ITEM));

                // move data to HW AES BUF for secure OS
                if (dbItem[E_MMAP_ID_HW_AES_BUF].index != 0)
                {
                    unsigned char* dst;
                    unsigned char* src;
                    //Variables for CusConfig
                    unsigned char* cusconfig = (void*)tmpBuf + 512;
                    OPTEE_CONFIG_HEADER* cusconfig_header = (OPTEE_CONFIG_HEADER*)cusconfig;
                    unsigned int u32CusConfigHeadSize = sizeof(OPTEE_CONFIG_HEADER)+sizeof(_SUB_SECURE_INFO);
                    unsigned int u32CusConfigHeadeMMCAlign = ALIGN(EMMC_BLOCK_SIZE, u32CusConfigHeadSize);
                    unsigned int u32CusConfigHeadeMMCBlockNum = (u32CusConfigHeadSize + u32CusConfigHeadeMMCAlign) / EMMC_BLOCK_SIZE;
                    //

                    // get HW AES buffer information
                    groupItem = groupItem + dbItem[E_MMAP_ID_HW_AES_BUF].index - 1;

                    if(!_gGroupSecureMemFlag && !MDrv_SEAL_SetSecureRange(groupItem->addr, groupItem->addr + groupItem->length, SEAL_ATTR_SECURE_RW))
                    {
                        return FALSE;
                    }

                    //Consistency check between secure group memory and independent secure memory
                    if(_gGroupSecureMemFlag &&  !(__PA2BA(groupItem->addr) >= _gGroupSecureMemAddr_lo && __PA2BA(groupItem->addr + groupItem->length) <= _gGroupSecureMemAddr_lo + _gGroupSecureMemsize))
                    {
                        LDR_PUTS("HW AES BUF secure range consistency check failed\n");
                        return FALSE;
                    }

                    // copy customer keybank
                    dst = (unsigned char*)((unsigned int)__PA2BA(groupItem->addr));

                    // copy mmapDB.ini for backward-compatible
                    dst += 0x1000;
                    src = (unsigned char*)(dataOffset + desc->Offset);
                    _memcpy(dst, src, 0x1000);

                    // copy optee config header
                    dst += 0x1000;
                    src = (unsigned char*)(tmpBuf+CONFIG_OPTEE_HEADER_LEN);
                    _memcpy(dst, src, sizeof(OPTEE_CONFIG_HEADER));

                    // copy optee config decriptor and config data
                    dst += sizeof(OPTEE_CONFIG_HEADER);
                    src = (unsigned char*)(header->init_load_addr_lo + header->init_size + ALIGN(16, header->init_size));
                    _memcpy(dst, src, opteeConfigHeader->NumberOfConfig*sizeof(OPTEE_CONFIG_DESCRIPTOR) + opteeConfigHeader->TotalConfigSize);

                    //Get CusConfig header
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
                    eMMC_ReadData_MIU((void*)cusconfig_header, u32CusConfigHeadSize+u32CusConfigHeadeMMCAlign, u32CusConfigeMMCBlockOffset);
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
                    drvNAND_ReadPartition(UNFD_PART_OPTEE, u32CusConfigeMMCBlockOffset, PTR2UINT(cusconfig_header), (u32CusConfigHeadSize+u32CusConfigHeadeMMCAlign)/EMMC_BLOCK_SIZE);
#endif
                    //Process CusConfig.aes
                    //Authenticate CusConfig header
                    auth_ok=MDrv_AESDMA_SecureMain_v2(UINT2PTR(cusconfig_header),sizeof(OPTEE_CONFIG_HEADER),((_SUB_SECURE_INFO*)(cusconfig+sizeof(OPTEE_CONFIG_HEADER)))->u8Signature,EmbeddedTL_A_REE_KEY);
                    if(auth_ok && !_memcmp(cusconfig_header->MagicID,"OPTEE_CUSTOM",12))
                    {
                        unsigned int u32CusConfigBodySize = cusconfig_header->TotalConfigSize+cusconfig_header->NumberOfConfig*sizeof(OPTEE_CONFIG_DESCRIPTOR);
                        unsigned int u32CusConfigBodySizeAlign = ALIGN(16,u32CusConfigBodySize);
                        unsigned int u32CusConfigBodySizeeMMCAlign = ALIGN(EMMC_BLOCK_SIZE, u32CusConfigBodySize + u32CusConfigBodySizeAlign + sizeof(_SUB_SECURE_INFO) - u32CusConfigHeadeMMCAlign);
                        unsigned int u32CusConfigBodyOtherSizeeMMC = u32CusConfigBodySize + u32CusConfigBodySizeAlign + sizeof(_SUB_SECURE_INFO) - u32CusConfigHeadeMMCAlign + u32CusConfigBodySizeeMMCAlign;
                        LDR_PUTS("CusConfig\n");
                        dst += opteeConfigHeader->NumberOfConfig*sizeof(OPTEE_CONFIG_DESCRIPTOR) + opteeConfigHeader->TotalConfigSize;
                        dst += ALIGN(16,opteeConfigHeader->NumberOfConfig*sizeof(OPTEE_CONFIG_DESCRIPTOR) + opteeConfigHeader->TotalConfigSize);
                        // copy optee custom config header
                        _memcpy(dst,cusconfig_header,u32CusConfigHeadSize+u32CusConfigHeadeMMCAlign);

                        // Get CusConfig config decriptor and config data from CusConfig.aes
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
                        eMMC_ReadData_MIU(dst+u32CusConfigHeadSize+u32CusConfigHeadeMMCAlign, u32CusConfigBodyOtherSizeeMMC ,u32CusConfigeMMCBlockOffset +u32CusConfigHeadeMMCBlockNum);
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
                        drvNAND_ReadPartition(UNFD_PART_OPTEE, u32CusConfigeMMCBlockOffset +u32CusConfigHeadeMMCBlockNum, PTR2UINT(dst)+u32CusConfigHeadSize+u32CusConfigHeadeMMCAlign, (u32CusConfigBodyOtherSizeeMMC)/EMMC_BLOCK_SIZE);
#endif
                        //Authenticate CusConfig header and body
                        if(!MDrv_AESDMA_SecureMain_v2(dst, u32CusConfigBodySize + u32CusConfigBodySizeAlign + u32CusConfigHeadSize, ((_SUB_SECURE_INFO*)(dst + u32CusConfigBodySize + u32CusConfigBodySizeAlign + u32CusConfigHeadSize))->u8Signature, EmbeddedTL_A_REE_KEY))
                        {
                            LDR_PUTS("Auth CusConfig body failed \n");
                            return FALSE;
                        }

                        // copy optee custom config body
                        _memcpy(dst+sizeof(OPTEE_CONFIG_HEADER),dst+u32CusConfigHeadSize,u32CusConfigBodySize+u32CusConfigBodySizeAlign);
                        //Decrypt CusConfig body
                        if (MDrv_AESDMA_Decrypt(__BA2PA(PTR2UINT(dst)+sizeof(OPTEE_CONFIG_HEADER)), u32CusConfigBodySize+u32CusConfigBodySizeAlign, EmbeddedTL_D_REE_KEY, AESDMA_ENGINE_CBC, E_AESDMA_SWKEY) == FALSE)
                        {
                            LDR_PUTS("Decrypt CusConfig body failed \n");
                            return FALSE;
                        }
                    }
                    else if(!auth_ok && !_memcmp(cusconfig_header->MagicID,"OPTEE_CUSTOM",12))
                    {
                        LDR_PUTS("Auth CusConfig header failed \n");
                        return FALSE;
                    }
                }
            }
            else if (desc->ConfigID==2) // secure MIU host
            {
                unsigned char* data = (unsigned char*)(dataOffset + desc->Offset);
                unsigned int size = desc->ConfigSize;
                int i, j;

                for (i=0; i<size; i++)
                {
                    for (j=0; j<8; j++)
                    {
                        if ( (*(data+i)) & (1<<j) )
                        {
                            MDrv_SEAL_SetMIUHost(i*8+j, 1);
                        }
                    }
                }
            }
            else if (desc->ConfigID==3) // secure RIU bank
            {
                unsigned char* data = (unsigned char*)(dataOffset + desc->Offset);
                unsigned int size = desc->ConfigSize;
                int i, j;

                for (i=0; i<size; i++)
                {
                    for (j=0; j<8; j++)
                    {
                        if ( (*(data+i)) & (1<<j) )
                        {
                            MDrv_SEAL_SetRIUBank(i*8+j, 1);
                        }
                    }
                }
            }
            else if (desc->ConfigID==5) // secure RIU register
            {
                unsigned char* data = (unsigned char*)(dataOffset + desc->Offset);
                unsigned int size = desc->ConfigSize;
                int i, j;

                for (i=0; i<size; i++)
                {
                    for (j=0; j<8; j++)
                    {
                        if ( (*(data+i)) & (1<<j) )
                        {
                            MDrv_SEAL_SetRIURegister(i*8+j, 1);
                        }
                    }
                }
            }
#if defined(CONFIG_PROGRAM_EMMC_RPMB_KEY)
            else if (desc->ConfigID==9) // RPMB key config
            {
                DSCMB_KLCfg_All64 cfg;
                _memset(&cfg, 0, sizeof(DSCMB_KLCfg_All64));
                _memset(&RpmbConfig, 0, sizeof(RpmbConfig));
                unsigned char* data = (unsigned char*)(dataOffset + desc->Offset);
                _memcpy(&cfg, data, sizeof(DSCMB_KLCfg_All64));
                if (desc->ConfigSize < sizeof(DSCMB_KLCfg_All64) + 16 /*ACPU key size*/ + 16*cfg.u32Level /*KL input size*/)
                {
                    LDR_PUTS("Bad RPMB key\n");
                    return FALSE;
                }
                _memcpy(&RpmbConfig, data+sizeof(DSCMB_KLCfg_All64)+16 + cfg.u32Level*16, sizeof(Rpmb_Config));
            }
#endif
        }
#if defined(CONFIG_PROGRAM_EMMC_RPMB_KEY)
        LDR_PUTS("eMMC_RPMB_Check_Program_Key\n");
        eMMC_RPMB_Check_Program_Key(RpmbConfig.select_key == 1 ? RpmbConfig.rootkey : NULL);
#endif
    }

    return header->init_load_addr_lo;
}

unsigned int ns_uboot_load_armfw(void)
{
    mstar_tee_t* header = NULL;
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    emmc_partition_t mpart;
#endif
    unsigned int u32HeadSize = CONFIG_OPTEE_HEADER_LEN + sizeof(_SUB_SECURE_INFO);
    unsigned int u32HeadeMMCAlign = ALIGN(EMMC_BLOCK_SIZE, u32HeadSize);
    unsigned int u32HeadeMMCBlockNum = (u32HeadSize + u32HeadeMMCAlign) / EMMC_BLOCK_SIZE;
    U8* tmpBuf = (U8*)u8TmpBuf;
    bootargs_teeloader_t* argument = (void*)&_ld_TEE_LDR_arg_start;

    if(_gIsTLreloaded == TLRELOADED)
    {
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
        if (argument->CustomizeID == DEFAULTTYPE && ns_uboot_load_partition(ATF_PARTITION_LEN, ATF_PARTITION, &header, &mpart))
        {
            eMMC_ReadData_MIU(tmpBuf, u32HeadSize + u32HeadeMMCAlign, mpart.start_block);
        }
        else if ((argument->CustomizeID == CUSM1TYPE1 || argument->CustomizeID == CUSM1TYPE2) && ns_uboot_load_partition(COMBINEDTEE_PARTITION_LEN, COMBINEDTEE_PARTITION, &header, &mpart))
        {
            eMMC_ReadData_MIU(tmpBuf, u32HeadSize + u32HeadeMMCAlign, mpart.start_block);
        }
        else
        {
            // load partition failed
            return 0;
        }
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
        if (UNFD_ST_SUCCESS != drvNAND_ReadPartition(UNFD_PART_ARMFW , 0, PTR2UINT(tmpBuf), 1)) // read 512-byte to get header and signature
        {
            return FALSE; // nand read fail
        }
#endif

        // verify header
        {
            if (FALSE == MDrv_AESDMA_SecureMain_v2(tmpBuf, CONFIG_OPTEE_HEADER_LEN, ((_SUB_SECURE_INFO*)(tmpBuf+CONFIG_OPTEE_HEADER_LEN))->u8Signature, EmbeddedTL_A_TI_KEY))
            {
                return 0;
            }
            else
            {
                header = (mstar_tee_t*)tmpBuf;
            }
        }

        if(argument->CustomizeID == CUSM1TYPE1 || argument->CustomizeID == CUSM1TYPE2)
        {
            _gOpteePartOffset = u32HeadSize + header->init_size + sizeof(_SUB_SECURE_INFO);
            _gOpteePartOffset += ALIGN(EMMC_BLOCK_SIZE, _gOpteePartOffset);
        }
        
        // make sure binary is really armfw
        if ( 0 != _memcmp( &(header->magic), "ATFW", 4) )
        {
            return 0;
        }
        
        _gGroupSecureMemFlag = (header->init_group_addr_lo != 0);
        if(_gGroupSecureMemFlag)//Use secure group memory as secure memory
        {
            _gGroupSecureMemAddr_lo = header->init_group_addr_lo;
            _gGroupSecureMemsize = header->group_size;
            //Consistency check between secure group memory and independent secure memory
            if( !(header->init_load_addr_lo >= _gGroupSecureMemAddr_lo && header->secure_range_end <= _gGroupSecureMemAddr_lo + _gGroupSecureMemsize))
            {
                LDR_PUTS("secure range consistency check failed\n");
                return FALSE;
            }
        }
        
        Fireware_entry = header->init_load_addr_lo;
        Fireware_end = header->secure_range_end;
        _gVersionInfo.armfw_version = header->cus_version;
        
        // STR mode depends on armfw header
        _gStrMode = ( header->warmboot_mode & STR_MODE_MASK );
        _gAuthMode = ( header->warmboot_mode & AUTH_MODE_MASK );
    }

    if(!_gGroupSecureMemFlag && !MDrv_SEAL_SetSecureRange((unsigned long long)__BA2PA(Fireware_entry), (unsigned long long)__BA2PA(Fireware_end), SEAL_ATTR_SECURE_RW))
    {
        LDR_PUTS("secure range setup failed\n");
        return FALSE;
    }
    else if(_gGroupSecureMemFlag && !MDrv_SEAL_SetSecureRange(__BA2PA(_gGroupSecureMemAddr_lo), __BA2PA(_gGroupSecureMemAddr_lo+_gGroupSecureMemsize), SEAL_ATTR_SECURE_RW))
    {
        LDR_PUTS("secure range setup failed\n");
        return FALSE;
    }
    if ( _gIsTLreloaded != TLRELOADED || ( _gIsStr && _gStrMode ))
    {
        if ( _gAuthMode == AESCBC_MAC )
        {
            int i;
            unsigned int result = 0;
            warmboot_table* tbl;

            if ( MDrv_SYS_Query(E_SYS_QUERY_MOBF_KEY_SAVED, &result) == FALSE )
            {
                return FALSE;
            }

            if ( result ) // support mobf
            {
                do
                {
                    if ( MDrv_SYS_Query(E_SYS_QUERY_MOBF_ENABLED, &result) == FALSE )
                    {
                        return FALSE;
                    }

                    if ( result ) // MOBF init done
                    {
                        break;
                    }
                } while ( 1 );
            }

            if ( MDrv_SYS_Query(E_SYS_QUERY_TRNG_KEY_SAVED, &result) == FALSE )
            {
                return FALSE;
            }

            if ( !result ) // no TRNG key, CBC-MAC check fail
            {
                return FALSE;
            }

            // calculate CBC-MAC with TRNG key
            tbl = (warmboot_table*)(Fireware_end - sizeof(warmboot_table));
            if(!MDrv_AESDMA_STR_CBCMAC(__BA2PA(tbl->ATFBase), tbl->ATFSize, (void*)__BA2PA(PTR2UINT(u8ATF_MAC))))
            {
                return FALSE;
            }

            //Fix for constant time of memcmp to prevent timing attack
            result = 0;
            for ( i=0 ; i<16 ; i++ )
            {
                result |= (u8ATF_MAC[i] ^ tbl->ATFMAC[i]);
            }
            if ( result )
            {
                return FALSE;
            }
        }

        return Fireware_entry;
    }

    // copy header part to load address
    _memcpy(UINT2PTR(header->init_load_addr_lo), UINT2PTR(tmpBuf), u32HeadSize + u32HeadeMMCAlign);

    // read other body data
    unsigned int u32BodyAlign = ALIGN(16,header->init_size); // Body dummy data

    unsigned int u32BodyeMMCAlign = ALIGN(EMMC_BLOCK_SIZE, header->init_size + u32BodyAlign + sizeof(_SUB_SECURE_INFO) - u32HeadeMMCAlign);
    unsigned int u32BodyOtherSizeeMMC = header->init_size + u32BodyAlign + sizeof(_SUB_SECURE_INFO) - u32HeadeMMCAlign + u32BodyeMMCAlign;

#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    eMMC_ReadData_MIU(UINT2PTR(header->init_load_addr_lo + u32HeadSize + u32HeadeMMCAlign), u32BodyOtherSizeeMMC, mpart.start_block + u32HeadeMMCBlockNum);
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
    if (UNFD_ST_SUCCESS == drvNAND_ReadPartition(UNFD_PART_ARMFW, 0, header->init_load_addr_lo, u32BodyOtherSizeeMMC/EMMC_BLOCK_SIZE + u32HeadeMMCBlockNum)) // read header with body
    {
        // because here we have loaded header from flash again, to prevent TOCTOU issue, we first replace the header part with the previously verified copy, before verify header+body as a whole
        _memcpy(UINT2PTR(header->init_load_addr_lo), UINT2PTR(tmpBuf), u32HeadSize);
    }
    else
    {
        return FALSE; // read armfw body fail
    }
#endif
    // verify body
    {
        int auth_ok = MDrv_AESDMA_SecureMain_v2(header->init_load_addr_lo, header->init_size + u32BodyAlign + u32HeadSize, ((_SUB_SECURE_INFO*)(header->init_load_addr_lo + header->init_size + u32BodyAlign + u32HeadSize))->u8Signature, EmbeddedTL_A_TI_KEY);
        if (!auth_ok)
        {
            // armfw body auth fail
            return FALSE;
        }


        _memcpy(UINT2PTR(header->init_load_addr_lo), UINT2PTR(header->init_load_addr_lo + u32HeadSize), header->init_size + u32BodyAlign);

        if (MDrv_AESDMA_Decrypt(__BA2PA(header->init_load_addr_lo), header->init_size + u32BodyAlign, EmbeddedTL_D_TI_KEY, AESDMA_ENGINE_CBC, E_AESDMA_SWKEY) == FALSE)
        {
            // armfw body decrypt fail
            return FALSE;
        }
    }

    return header->init_load_addr_lo;
}

#ifdef CONFIG_DOUBLE_OPTEE
static U32 Double_OPTEE_Check(char* name, unsigned int len)
{
    U32 ret = 0;

    if (_memcmp(name,ATF_PARTITION,len) == 0 && \
        (*(volatile unsigned int*)(CONFIG_RIU_BASE_ADDRESS + (0x103380<<1)))&0x0400)
    {
        ret = 1; //armfw authen failed, try armfw_bak partition
    }
    else if (_memcmp(name,TEE_PARTITION,len) == 0 && \
             (*(volatile unsigned int*)(CONFIG_RIU_BASE_ADDRESS + (0x103380<<1)))&0x0800)
    {
        ret = 1; //optee authen failed, try optee_bak partition
    }

    return ret;
}
#endif

#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
int ns_uboot_load_partition(unsigned int len, char* name, mstar_tee_t** header, emmc_partition_t* mpart)
{
    int u32_i, u32_j;
    int found;
    U8* tmpBuf = (U8*)u8TmpBuf;
    U8* part_name = (U8*)u8TmpBuf;
    bootargs_teeloader_t* argument = (void*)&_ld_TEE_LDR_arg_start;
    //Variables for CustomizeID=1,M1
    U8* UpperBuff=&tmpBuf[512];//BuffA for swapping buffer
    U8* LowerBuff=&tmpBuf[1024];//BuffB for MMC loading buffer
    struct partition_info* M1PartInfoPt=NULL;
    int u32_k,numOfPart=0,emmcBlk=0,partIndex=0;
    //

    for(u32_i=0; u32_i < len; u32_i++)
    {
        part_name[u32_i] = name[u32_i];
    }

#ifdef CONFIG_DOUBLE_OPTEE
    if (0 != Double_OPTEE_Check(name, len)) //get armfw_bak, optee_bak partition info
    {
        part_name[len++] = '_';
        part_name[len++] = 'b';
        part_name[len++] = 'a';
        part_name[len++] = 'k';
    }
#endif

    // iterate to find specific partition
    for(u32_i=0; argument->CustomizeID == DEFAULTTYPE && u32_i<= EMMC_RESERVED_FOR_MAP_V2;u32_i++)//CustomizeID=0
    {
        if(eMMC_ReadData_MIU((U8*)mpart, EMMC_BLOCK_SIZE, u32_i))
            break; // emmc read fail

        found = 1;

        for (u32_j=0; u32_j<len; u32_j++)
        {
            if (mpart->name[u32_j]!=part_name[u32_j])
            {
                found = 0;
                break;
            }
        }

        if (found)
        {
            eMMC_ReadData_MIU(tmpBuf, EMMC_BLOCK_SIZE, mpart->start_block);
            *header = (mstar_tee_t*)tmpBuf;
            return 1;
        }
    }

    for(u32_i=0,emmcBlk=0; (argument->CustomizeID == CUSM1TYPE1 || argument->CustomizeID == CUSM1TYPE2) && u32_i<= sizeof(struct partmap_info);u32_i+=EMMC_BLOCK_SIZE,emmcBlk++)//M1 Customized Type
    {
        //Starting point
        if(u32_i==0)
        {
            //Read data to BuffA from EMMC for the first block
            if(eMMC_ReadData_MIU(UpperBuff, EMMC_BLOCK_SIZE, 0x2800))//starting point of partinfo is at offset of 0x500000
                break; // emmc read fail

            M1PartInfoPt=((struct partmap_info*)UpperBuff)->partition;
            numOfPart=(EMMC_BLOCK_SIZE-((void*)M1PartInfoPt-(void*)UpperBuff))/sizeof(struct partition_info);
        }
        else
        {
            //Read data to BuffB from EMMC after the first block is read
            if(eMMC_ReadData_MIU(LowerBuff, EMMC_BLOCK_SIZE, 0x2800+emmcBlk))//starting point of partinfo is at offset of 0x500000
                break; // emmc read fail

            numOfPart=(EMMC_BLOCK_SIZE+((void*)LowerBuff-(void*)M1PartInfoPt))/sizeof(struct partition_info);
        }

        //Parsing partition_info
        for (u32_j=0; u32_j<numOfPart&&partIndex<PARTITION_MAX; u32_j++)
        {
            found = 1;
            for (u32_k=0; u32_k<len; u32_k++)
            {
                if (M1PartInfoPt->name[u32_k]!=part_name[u32_k])
                {
                    found = 0;
                    break;
                }
            }
            if (found)
            {
                mpart->start_block=(U32)(M1PartInfoPt->offset>>9);
                LDR_PUTS("M1 Partition found!\n");
                return 1;
            }
            M1PartInfoPt++;
            partIndex++;
        }

        if(u32_i>0)
        {
            //Swap BuffB to BuffA
            _memcpy(UpperBuff,LowerBuff,EMMC_BLOCK_SIZE);
            M1PartInfoPt=(void*)M1PartInfoPt-EMMC_BLOCK_SIZE;
        }
    }

    return 0;
}
#endif

static char byte_to_char(char val) __attribute__((__no_instrument_function__));
static char byte_to_char(char val)
{
    if (val > 0x9)
        return (0x41+(val-10));
    else
        return (0x30+val);
}

static void u32_to_string(unsigned int val, char* dst) __attribute__((__no_instrument_function__));
static void u32_to_string(unsigned int val, char* dst)
{
    dst[0] = byte_to_char((val>>28)&0xF);
    dst[1] = byte_to_char((val>>24)&0xF);
    dst[2] = byte_to_char((val>>20)&0xF);
    dst[3] = byte_to_char((val>>16)&0xF);
    dst[4] = byte_to_char((val>>12)&0xF);
    dst[5] = byte_to_char((val>>8)&0xF);
    dst[6] = byte_to_char((val>>4)&0xF);
    dst[7] = byte_to_char((val>>0)&0xF);
}

void to_hex_string(U8* src, char* dst, unsigned int len) __attribute__((__no_instrument_function__));
void to_hex_string(U8* src, char* dst, unsigned int len)
{
    static char hex_tbl[16] __attribute__((aligned(8)))= {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    int i, j;
    U8 val;
    for (i=0, j=0; i<len; ++i, j+=2)
    {
        val = src[i];
        dst[j] = hex_tbl[((val>>4) & 0x0F)];
        dst[j+1] = hex_tbl[(val & 0x0F)];
    }
}

unsigned int ns_uboot_cleanup(void)
{
    return MDrv_SEAL_Cleanup();
}

typedef void (*OPTEE_entry)(int _bootarg, int _ree_addr, int c, int d);

unsigned int ns_uboot_boot(unsigned int uboot_addr)
{
    mstar_boot_prameters_t* bootp = &boot;
    U8 emmc_CID[16];
    unsigned int AdditionalOpt = *((unsigned int*)((unsigned int)&_ld_TEE_LDR_run_base+ADDITIONOPT_OFFSET));

    // Avoid REE using eFuse key
    MDrv_AESDMA_DisableHwKey();
    
    LDR_PUTS("reeloader_addr=");LDR_PUTDW(uboot_addr);LDR_PUTS("\n");
    if((AdditionalOpt & MASK_DEVICELOCKMASK) == 1)
    {
        LDR_PUTS("MASK_DEVICELOCKMASK is ON\n");
    }

    _memset(emmc_CID, 0, sizeof(emmc_CID));
    _memset(EmbeddedTL_D_TI_KEY, 0, AES_KEY_LEN);
    _memset(EmbeddedTL_D_REE_KEY, 0, AES_KEY_LEN);
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    //Copy EMMC CID prior to SRAM cleanup since EMMC CID may be stored in SRAM
    if(_gIsTLreloaded == TLRELOADED)
    {
        eMMC_GetCID(emmc_CID);
    }
#endif

    if(SecureBootTeeOK)
    {
        //Copy default vector to 0x20000000
        _memcpy((void*)0x20000000,(void*)&_ld_TEE_LDR_vector_start,0x600);
        //Copy setvector_code to 0x20000600
        _memcpy((void*)0x20000600,setvector_code,sizeof(setvector_code));
        //SMC to set EL3 vector base address
        __asm__ __volatile__(
            ".arch_extension sec\n\t"
            "mov  r0,%0 \n\t"
            "smc #0  \n\t"
            :
            :"r"(&_ld_TEE_LDR_vector_start)
            :"r0"
            );
        //Clean old vector code at 0x20000000
        _memset((void*)0x20000000,0,0x600+sizeof(setvector_code));

        if ( !_gIsStr || !_gStrMode )//AC flow
        {
            bootp->BIN0_entry_point = BIN0_entry_point;
            bootp->BIN0_RW64 = MODE_RW_32;
            bootp->BIN1_entry_point = uboot_addr;
            bootp->BIN1_RW64 = MODE_RW_32;
#ifdef CONFIG_ARMv8_64BIT_KERNEL
            bootp->BIN2_RW64 = MODE_RW_64;
#else
	          bootp->BIN2_RW64 = MODE_RW_32;
#endif

            { // construct OPTEE boot args

            _memset(_gOpteeBootArgs, 0, sizeof(_gOpteeBootArgs));
            _gOpteeBootArgs[0] = 'C';
            _gOpteeBootArgs[1] = 'I';
            _gOpteeBootArgs[2] = 'D';
            _gOpteeBootArgs[3] = '=';
            to_hex_string(emmc_CID, _gOpteeBootArgs+4, 15);
            _gOpteeBootArgs[34] = ' ';
            _gOpteeBootArgs[35] = 'S';
            _gOpteeBootArgs[36] = 'T';
            _gOpteeBootArgs[37] = 'R';
            _gOpteeBootArgs[38] = 'M';
            _gOpteeBootArgs[39] = 'O';
            _gOpteeBootArgs[40] = 'D';
            _gOpteeBootArgs[41] = 'E';
            _gOpteeBootArgs[42] = '=';
            _gOpteeBootArgs[43] = '0';
            _gOpteeBootArgs[44] = 'x';
            u32_to_string(_gStrMode, _gOpteeBootArgs+45);
#if defined(CONFIG_ARMv8_ARM_TRUSTED_FIRMWARE)
            _gOpteeBootArgs[53] = ' ';
            _gOpteeBootArgs[54] = 'M';
            _gOpteeBootArgs[55] = 'C';
            _gOpteeBootArgs[56] = 'T';
            _gOpteeBootArgs[57] = '=';
            _gOpteeBootArgs[58] = '0';
            _gOpteeBootArgs[59] = 'x';
            u32_to_string(PTR2UINT(&_gMultiCoreTable), _gOpteeBootArgs+60);
            _gOpteeBootArgs[68] = ' ';
            _gOpteeBootArgs[69] = 'D';
            _gOpteeBootArgs[70] = 'E';
            _gOpteeBootArgs[71] = 'V';
            _gOpteeBootArgs[72] = 'I';
            _gOpteeBootArgs[73] = 'C';
            _gOpteeBootArgs[74] = 'E';
            _gOpteeBootArgs[75] = 'L';
            _gOpteeBootArgs[76] = 'O';
            _gOpteeBootArgs[77] = 'C';
            _gOpteeBootArgs[78] = 'K';
            _gOpteeBootArgs[79] = 'M';
            _gOpteeBootArgs[80] = 'A';
            _gOpteeBootArgs[81] = 'S';
            _gOpteeBootArgs[82] = 'K';
            _gOpteeBootArgs[83] = '=';
            _gOpteeBootArgs[84] = '0';
            _gOpteeBootArgs[85] = 'x';
            _gOpteeBootArgs[86] = (AdditionalOpt & MASK_DEVICELOCKMASK)==1 ? '1':'0';
            _gOpteeBootArgs[87] = '\0';
#else
            _gOpteeBootArgs[53] = '\0';
#endif

            bootp->BIN0_bootargs = (unsigned long long)_gOpteeBootArgs;
            }
#if defined(CONFIG_OPTEE_ANTIBRICK)
           unsigned short RTCBank = 0;
           //Enable watchdog and write status flag
           WREG16(0x1F006010,0x5400);
           WREG16(0x1F006014,0x2AEA);
           //OPTEECHECKIN
           _gOpteeConsisStatus = OPTEECHECKIN;
           RTCBank = RREG16(RTC_BANK);
           RTCBank = (RTCBank & 0x00FF) | _gOpteeConsisStatus << 8;
           WREG16(RTC_BANK,RTCBank);
           //
#endif
        }
        else//DC flow
        {
        }
    }
    else
    {
        bootp->BIN1_entry_point = uboot_addr;
        WREG16(DBG_BANK,0xEEFF);
        //Copy default vector to 0x20000000
        _memcpy((void*)0x20000000,(void*)&_ld_TEE_LDR_vector_start,0x600);
        //Copy setvector_code to 0x20000600
        _memcpy((void*)0x20000600,setvector_code_TeeFail,sizeof(setvector_code_TeeFail));
    }

    if (!ns_uboot_cleanup())
    {
        LDR_PUTS("NSClean up failed!\n");
        return FALSE;
    }
    ns_uboot_postboot();

    return TRUE;
}

#if defined(CONFIG_OPTEE_ANTIBRICK)
unsigned int ns_uboot_check_optee_consistency()
{
    _gOpteeConsisStatus = (unsigned char)(RREG16(RTC_BANK)>>8);
    //Check Optee consistency
    if(_gOpteeConsisStatus == OPTEECHECKIN)
    {
        return FALSE;
    }
    return TRUE;
}
#endif

unsigned int ns_uboot_get_optee_addr(void)
{
    return BIN0_entry_point;
}

unsigned int ns_uboot_nonsecure_handler(void)
{
    // set hosts of PM RIU bridge to non-secure
    // set hosts of NONPM RIU bridge to non-secure (except ARM)
    // set hosts of SEC RIU bridge to non-secure
    // set ARM to non-secure mode (must be the last step of this stage)
    SecureBootTeeOK = FALSE;

    if (_gIsStr)
    {
        ns_uboot_global_reset();
    }

    return 1;
}

#if defined(CONFIG_MSTAR_FTL_SD)
extern U32 FTLSd_Init(U16 u16_StartPBA, U16 u16_PBACnt, U16 IfLockCIS);
extern void FTLSd_DumpData(void);
#endif
unsigned int ns_uboot_init(void)
{
    _gIsStr = ns_uboot_is_str() ? 1 : 0;

    _memset(&RpmbConfig, 0, sizeof(RpmbConfig));

    //Reverse AESKeys
    array_reverse(EmbeddedTL_D_TI_KEY, AES_KEY_LEN);
    array_reverse(EmbeddedTL_D_REE_KEY, AES_KEY_LEN);
#if defined(CONFIG_TEE_LOADER_HWAES_TEST)
    AES_TestInput();
#endif

#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    if(eMMC_CheckIfReady())
    {
        return 0;
    }
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
    nand_set_partinfo(NULL);
#endif

#if defined(CONFIG_MSTAR_FTL_SD)
    bootargs_teeloader_t* argument = (void *)&_ld_TEE_LDR_arg_start;
    //nand_opts bit0:bNoLockCIS
    if (argument->nand_opts & 0x01)
    {
        //bNoLockCIS is enabled, skip locking CIS block
        FTLSd_Init(0,0,0);
    }
    else
    {
        //bNoLockCIS is disabled, locking CIS block
        FTLSd_Init(0,0,1);
    }
#endif

    //Reset 0x103382
    WREG16(DBG_BANK,0x0000);

    //return MDrv_SEAL_Init();
    return 1;
}

void ns_uboot_self_clean(void)
{
    //Clean teeloader
    unsigned int *start4Byte = (void *)&_ld_TEE_LDR_run_start;
    unsigned int *end4Byte =  (void *)&_ld_TEE_LDR_run_end;
    unsigned char *startByte = NULL;
    int wordnum = (int)((unsigned int)end4Byte-(unsigned int)start4Byte) >> 2;
    int slice = (int)((unsigned int)end4Byte-(unsigned int)start4Byte) & 0x03;
    if(((unsigned int)start4Byte & 0x00000003)==0)//Check if &_ld_TEE_LDR_run_start is aligned to 4byte
    {
        while (wordnum-- > 0)
        {
            *(start4Byte++) = 0x00000000;
        }
    }
    else//&_ld_TEE_LDR_run_start is not aligned to 4byte
    {
        slice = end4Byte-start4Byte;
    }
    startByte = (unsigned char*)start4Byte;
    while (slice-- > 0)
    {
        *(startByte++) = 0x00;
    }

    //Clean TLbss
    start4Byte = (void *)&_ld_TLbss_start;
    end4Byte = (void *)&_ld_TLbss_end;
    startByte = NULL;
    wordnum = (int)((unsigned int)end4Byte-(unsigned int)start4Byte) >> 2;
    slice = (int)((unsigned int)end4Byte-(unsigned int)start4Byte) & 0x03;
    if(((unsigned int)start4Byte & 0x00000003)==0)//Check if &_ld_TLbss_start is aligned to 4byte
    {
        while (wordnum-- > 0)
        {
            *(start4Byte++) = 0x00000000;
        }
    }
    else//&_ld_TLbss_start is not aligned to 4byte
    {
        slice = end4Byte-start4Byte;
    }
    startByte = (unsigned char*)start4Byte;
    while (slice-- > 0)
    {
        *(startByte++) = 0x00;
    }
}

void ns_uboot_DelayMS(unsigned int msDelay)
{
    unsigned int u32val1;
    unsigned int u32val2;
    unsigned int u32Interval;

    u32Interval = 12000 * msDelay; // clock is running @ 12MHz

    u32val1=((unsigned int)RREG16(0x1F006090)|((unsigned int)RREG16(0x1F006094)<<16));
    while (1)
    {
        u32val2=((unsigned int)RREG16(0x1F006090)|((unsigned int)RREG16(0x1F006094)<<16));
        if  ((u32val2 - u32val1) >= u32Interval)
            break;
    }
}

void ns_uboot_global_reset()
{
    do
    {
        ns_uboot_cleanup();
        ns_uboot_self_clean();
        MDrv_SEAL_DisableRIUBridges();
        ns_uboot_DelayMS(3000);
        *(volatile unsigned short*)(0x1F005CB8) = 0x79;
    } while(0);
}

void ns_uboot_postboot(void)
{
    mstar_boot_prameters_t* bootp = &boot;
    if(SecureBootTeeOK)
    {
        _gIsTLreloaded = TLRESUMED;
#if defined(CONFIG_ARMv8_ARM_TRUSTED_FIRMWARE)
        __asm__ __volatile__(
        ".arch_extension sec\n\t"
        "ldr  r1,[%0]  \n\t"
        "mov  r0,%1  \n\t"
        "smc #0  \n\t"
        :
        :"r"(&Fireware_entry),"r"(bootp)
        :"r0","r1"
        );
#else
        OPTEE_entry entry = (OPTEE_entry) BIN0_entry_point;
        entry(0, bootp->BIN0_bootargs, bootp->BIN1_entry_point, 0);
#endif
    }
    else
    {
        ns_uboot_self_clean();
        MDrv_SEAL_DisableRIUBridges();
        //SMC to set EL3 vector base address
        __asm__ __volatile__(
          ".arch_extension sec\n\t"
          "mov  r0,%0 \n\t"
          "mov  r1,%1 \n\t"
          "smc #0  \n\t"
          :
          :"r"(&_ld_TEE_LDR_vector_start),"r"(&bootp->BIN1_entry_point)
          :"r0","r1"
          );
        ///////////////////
    }

    /* CPU never goes here */
    ns_uboot_global_reset();
}

#if defined(CONFIG_TEE_LOADER_HWAES_TEST)
void AES_TestInput(void)
{
    LDR_PUTS("Dump EmbeddedAES_TestInput\n");
    LDR_DUMP(EmbeddedAES_TestInput,16);
    array_reverse(EmbeddedAES_TestInput,16);
    LDR_PUTS("Dump Reverse of EmbeddedAES_TestInput\n");
    LDR_DUMP(EmbeddedAES_TestInput,16);
    MDrv_AESDMA_Encrypt(__BA2PA(EmbeddedAES_TestInput), 16, NULL, AESDMA_ENGINE_CBC, E_AESDMA_UNIFORMKEY0);
    LDR_PUTS("Dump EmbeddedAES_TestInput_Encrypted\n");
    LDR_DUMP(EmbeddedAES_TestInput,16);
}
#endif

extern void teeloader_entry(void* argv)  __attribute__((section(".tee.entry")));
void teeloader_entry(void* argv)
{
	  // Setup stack pointer for TEE loader
	  __asm__ __volatile__(
        "ldr  r7, =_ld_TEE_LDR_stack_end  \n\t"
        "mov  sp,r7  \n\t" 
    );
    LDR_PUTS("optee teeloader entry\n");
    
    //Initial GIC and MPCore
    __asm__ __volatile__(
        "blx bootramTL_init \n\t"
    );

    bootargs_teeloader_t* argument = (void *)&_ld_TEE_LDR_arg_start;
    //Initialize TLbss
    if(_gIsTLreloaded == TLRELOADED)
    {
        _memset((void *)&_ld_TLbss_start, 0, ((unsigned int)&_ld_TLbss_end)-((unsigned int)&_ld_TLbss_start));
        if ( !ns_uboot_init() )
        {
            LDR_PUTS("NSInit failed\n");
            ns_uboot_nonsecure_handler();
        }

#if defined(CONFIG_OPTEE_ANTIBRICK)
        if ( !ns_uboot_check_optee_consistency() )
        {
            LDR_PUTS("OPTEE_ANTIBRICK\n");
            ns_uboot_nonsecure_handler();
        }
#endif
    }
    else if(_gIsTLreloaded == TLRESUMED)
    {
        _gIsStr = ns_uboot_is_str() ? 1 : 0;
        //Reset 0x103382
        WREG16(DBG_BANK,0x0000);
    }

    if(SecureBootTeeOK && !ns_uboot_load_armfw())
    {
        LDR_PUTS("Load armfw failed!\n");
        ns_uboot_nonsecure_handler();
    }
    
    if(SecureBootTeeOK && !ns_uboot_load_optee())
    {
        LDR_PUTS("Load optee failed!\n");
        ns_uboot_nonsecure_handler();
    }
    
    if(_gIsTLreloaded == TLRELOADED)
    {
        if(!ns_uboot_anti_rollback())
        {
            LDR_PUTS("Anti rollback check failed!\n");
            ns_uboot_nonsecure_handler();
        }
    }
    
    if(ns_uboot_support_faststr())
    {
        //Skip REELoader if FSTR is enabled
        if (!ns_uboot_boot(argument->ree_entry))
        {
            LDR_PUTS("NSBoot failed!\n");
            ns_uboot_nonsecure_handler();
        }
        /* CPU never goes here */
        ns_uboot_global_reset();
    }
    else if(_gIsTLreloaded == TLRESUMED)
    {
        LDR_PUTS("Resume failed!Please enabled FAST_BOOT.\n");
        ns_uboot_nonsecure_handler();
        ns_uboot_global_reset();
    }
    
    if(!ns_uboot_check_boundary(argument->ree_entry,argument->ree_size))
    {
        LDR_PUTS("reeloader ns_uboot_check_boundary failed!\n");
        ns_uboot_nonsecure_handler();
        ns_uboot_global_reset();
    }

    if(!ns_uboot_load_reeloader(argument->ree_entry,argument->ree_size,argument->ree_opt))
    {
        LDR_PUTS("ns_uboot_load_reeloader failed!\n");
        ns_uboot_nonsecure_handler();
        ns_uboot_global_reset();
    }

#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
    if((argument->ree_opt & 0x20) && !ns_uboot_check_boundary(argument->CKB_address,argument->CKB_size))
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
    if((argument->ree_opt & 0x20) && !ns_uboot_check_boundary(argument->CKB_address,argument->CKB_size+argument->CKB_Offset))
#endif
    {
        LDR_PUTS("CKB ns_uboot_check_boundary failed!\n");
        ns_uboot_nonsecure_handler();
        ns_uboot_global_reset();
    }

    if(!ns_uboot_load_CKB(argument->CKB_Offset,argument->CKB_address,argument->CKB_size,argument->ree_opt))
    {
        LDR_PUTS("ns_uboot_load_CKB failed!\n");
        ns_uboot_nonsecure_handler();
        ns_uboot_global_reset();
    }

    if (!ns_uboot_boot(argument->ree_entry))
    {
        LDR_PUTS("NSBoot failed!\n");
        ns_uboot_nonsecure_handler();
    }

    /* CPU never goes here */
    ns_uboot_global_reset();
}
