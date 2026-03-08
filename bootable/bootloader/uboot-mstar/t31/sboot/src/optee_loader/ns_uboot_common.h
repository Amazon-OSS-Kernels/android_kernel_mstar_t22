#include "./seal/src/drvSEAL.h"
#include "./aesdma/src/drvAESDMA.h"
#if defined(CONFIG_MSTAR_ROM_BOOT_WITH_EMMC_FLASH)
#include "./mmc/inc/common/eMMC.h"
#include "./part_emmc.h"
#include "partinfoM1.h"
#elif defined(CONFIG_MSTAR_ROM_BOOT_WITH_NAND_FLASH)
#include "./nand/inc/common/drvNAND.h"
extern U32 drvNAND_LoadBLOffsetDma(U8* pu8DMAAddr, U32 u32_Offset, U32 u32_Size);
#endif

#define IS_SECURE_CHIP()    ((*(volatile unsigned short*)(CONFIG_RIU_BASE_ADDRESS + (0x038E0<<1)))&0x01)

#define RREG16(addr) (*(volatile unsigned short *)(addr))
#define WREG16(addr, value) (*(volatile unsigned short *)(addr) = (value))

// UART
#ifndef REG_UART_BASE
#define REG_UART_BASE     (0x1F201300)
#endif
#ifndef UART_LSR_THRE
#define UART_LSR_THRE     (0x20)
#endif
#ifndef UART_LSR_TEMT
#define UART_LSR_TEMT     (0x40)
#endif
#ifndef REG_UART_RX
#define REG_UART_RX       (REG_UART_BASE + (0 * 8))
#endif
#ifndef REG_UART_TX
#define REG_UART_TX       (REG_UART_BASE + (0 * 8))
#endif
#ifndef REG_UART_LSR
#define REG_UART_LSR      (REG_UART_BASE + (5 * 8))
#endif

// MIU
#ifdef CONFIG_MSTAR_M7221
#define MIU0_PA_BASE               (0x00000000UL)
#define MIU1_PA_BASE               (0x80000000UL)
#else
#define MIU0_PA_BASE               (0x00000000UL)
#define MIU1_PA_BASE               (0x80000000UL)
#endif
#define MIU2_PA_BASE               (0xC0000000UL)

#if defined(CONFIG_MSTAR_M7221) || defined(CONFIG_MSTAR_M7322)
//For UMA structures, they have low performance of memory RW, hence we use BDMA to increase copying performance
#define MEMCPY_BDMA 1
#endif

#if defined(CONFIG_MCU_MIPS32)
#ifndef NULL
#define NULL 0
#endif
#endif

void LDR_PUTC(int c);
void LDR_PUTX(char val);
void LDR_PUTDW(unsigned int val);
void LDR_PUTS(const char *s);
void LDR_DUMP(const char *s,int count);
void *_memcpy(void *d, void *s, unsigned int n);
int _memcmp(const void * cs,const void * ct, unsigned int count);
void *_memset (void *s, int c, unsigned int n);
void array_reverse(unsigned char* ary, unsigned int len);
#if defined(__aarch64__)
unsigned long long __BA2PA(unsigned long long u64BusAddr);
unsigned long long __PA2BA(unsigned long long u64PhyAddr);
#else
unsigned int __BA2PA(unsigned int u64BusAddr);
unsigned int __PA2BA(unsigned int u64PhyAddr);
#endif

typedef struct bootargs_teeloader
{
	unsigned int ree_entry;
	unsigned int ree_size;
	unsigned int ree_opt;
	unsigned int CKB_Offset;
	unsigned int CKB_address;
	unsigned int CKB_size;
	unsigned int CustomizeID;
	unsigned int nand_opts;
	unsigned int ree_arg_addr;
	unsigned int reserve[55];
} bootargs_teeloader_t;//Maxim 256 bytes
//ree_opt
//bit0:bAuthReeloader
//bit1:bDecReeloader
//bit2:bBypassLoadReeloader
//bit3:bAuthCKB
//bit4:bDecCKB
//bit5:bEnableCKB
//nand_opts
//bit0:bNoLockCIS