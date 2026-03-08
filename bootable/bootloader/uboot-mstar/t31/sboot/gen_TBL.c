#include "include/autoconf.h"
#include "drvRIU.h"
#include "drv_miu_init.h"
#include "drv_miu_init_str.h"
#include "drvPadConf.c"
                                                                                                        
const U8 padInitTbl_PreInit[] __attribute__((section(".padInitTbl_PreInit")));
const U8 padInitTbl[] __attribute__((section(".padInitTbl")));
const MS_REG_INIT MIU0_DDR_PreInit[] __attribute__((section(".MIU0_DDR_PreInit")));
const MS_REG_INIT MIU1_DDR_PreInit[] __attribute__((section(".MIU1_DDR_PreInit")));
const MS_REG_INIT MIU0_DDR_Init[] __attribute__((section(".MIU0_DDR_Init")));
const MS_REG_INIT MIU1_DDR_Init[] __attribute__((section(".MIU1_DDR_Init")));
const MS_REG_INIT MIU0_DDR_Init_Str[] __attribute__((section(".MIU0_DDR_Init_Str")));
const MS_REG_INIT MIU1_DDR_Init_Str[] __attribute__((section(".MIU1_DDR_Init_Str")));
const MS_REG_INIT MIU0_DDR_PreInit_Str[] __attribute__((section(".MIU0_DDR_PreInit_Str")));
const MS_REG_INIT MIU1_DDR_PreInit_Str[] __attribute__((section(".MIU1_DDR_PreInit_Str")));
void main( void )                                                                                       
{

}
