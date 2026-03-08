#ifndef __MSTAR_MBX_HEAD_H__
#define __MSTAR_MBX_HEAD_H__

#define DEBUG 0

#if DEBUG //debug
#include "mbx_debug.h"
#elif defined(CONFIG_MSTAR_CHIP) //Kernel
#include "mbx_linux.h"
#elif defined(CFG_MSTAR_BOARD) //optee
#include "mbx_optee.h"
#else //Mboot
#include "mbx_mboot.h"
#endif

typedef enum {
	CMD_R2T_GENERAL = 0x6006,
	CMD_R2T_RPC_DONE,
	CMD_T2R_GENERAL_DONE,
	CMD_T2R_RPC,
	CMD_NULL = 0x8898,
} OPTEE_MBX_CMD_ID;

typedef struct {		// Compatible struct smc_param
	uint64_t version;	// version check
	uint64_t cmd_id;	// OPTEE_MBX_CMD_ID
	uint64_t a0;		// funcid
	uint64_t a1;
	uint64_t a2;
	uint64_t a3;
	uint64_t a4;
	uint64_t a5;
	uint64_t a6;
	uint64_t a7;
	uint64_t reserve[8];
} mbx_smc_args;

TEE_Result register_mbx_base(mbx_smc_args *base);
TEE_Result send_mbx(thread_smc_args *args,OPTEE_MBX_CMD_ID id, uint32_t mbx_slot);

extern mbx_smc_args *Sec_ARM_MBX_Base;
/* Example 1 Mboot
 * struct thread_smc_args args;
 * args.a0 = fase_call_id;
 * args.a1 = parameter; //value or point depend on case
 * args.a2 = parameter; //value or point depend on case
 * args.a3 = parameter; //value or point depend on case Max parameter size
 * register_mbx_base(MBX_BASE); //ask optee owner Sec_ARM Mbx Address depend on chip
 * if(send_mbx(&args, CMD_R2T_GENERAL, 0) != 0) //mbx_slot always = 0
 * {
 * 		printf("Error\n");
 * }
 * //Get result from args.a0
 * //Get return parameter from args.a1
 * //Get return parameter from args.a2
 * //Get return parameter from args.a3
 */
#endif
