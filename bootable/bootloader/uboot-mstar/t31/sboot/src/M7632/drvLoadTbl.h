//Below area is auto-updated by gen_Tbl.sh, try not to modify it!
#define OUTERSIZE                           0x24000

#define PAD_PRE_SECT_SIZE           0x400
#define PAD_SECT_SIZE                   0x1000
#define MIU0_PRE_SECT_SIZE         0x1000
#define MIU0_SECT_SIZE                 0x1000
#define MIU1_PRE_SECT_SIZE         0x1000
#define MIU1_SECT_SIZE                 0x1000
#define MIU0_STR_PRE_SECT_SIZE 0x1000
#define MIU1_STR_PRE_SECT_SIZE 0x1000
#define MIU0_STR_SECT_SIZE         0x1000
#define MIU1_STR_SECT_SIZE         0x1000
// End of auto-update section

#define PADPRE_TBL_SIZE         0x200
#define PAD_TBL_SIZE            0x800
#define MIU_PRE_TBL_SIZE        0x800
#define MIU_TBL_SIZE            0x800
#define MIU_STR_TBL_SIZE        0x800
#define MIU_PRE_STR_TBL_SIZE    0x800

#define ONEBIN_TBL_SIZE_512   0x200
#define ONEBIN_TBL_SIZE_1024  0x400
#define ONEBIN_TBL_SIZE_2048  0x800
#define ONEBIN_SIG_OFF_512    ONEBIN_TBL_SIZE_512 - 0x100
#define ONEBIN_SIG_OFF_1024    ONEBIN_TBL_SIZE_2048 - 0x100
#define ONEBIN_SIG_OFF_2048    ONEBIN_TBL_SIZE_2048 - 0x100

//TBD move to drvLoadTbl.S
#define TYPE_OFF r10
#define TBL_OFF  r11
#define TBL_SIZE r9

#define PADPRE_SEC_OFF 0x0
#define PAD_SECT_OFF   PAD_PRE_SECT_SIZE
#define MIU0_PRE_SECT_OFF  PAD_SECT_OFF+PAD_SECT_SIZE
#define MIU0_SECT_OFF  MIU0_PRE_SECT_OFF+MIU0_PRE_SECT_SIZE
#define MIU1_PRE_SECT_OFF MIU0_SECT_OFF+MIU0_SECT_SIZE
#define MIU1_SECT_OFF MIU1_PRE_SECT_OFF+MIU1_PRE_SECT_SIZE
#define MIU0_STR_PRE_SECT_OFF MIU1_SECT_OFF+MIU1_SECT_SIZE
#define MIU0_STR_SECT_OFF MIU0_STR_PRE_SECT_OFF+MIU0_STR_PRE_SECT_SIZE
#define MIU1_STR_PRE_SECT_OFF MIU0_STR_SECT_OFF+MIU0_STR_SECT_SIZE
#define MIU1_STR_SECT_OFF MIU1_STR_PRE_SECT_OFF+MIU1_STR_PRE_SECT_SIZE
//TBD move to drvLoadTbl.S

#define PADPRE       0x1
#define PAD          0x2
#define MIU0_Pre     0x3
#define MIU0         0x4
#define MIU1_Pre     0x5
#define MIU1         0x6
#define MIU0_STR_Pre 0x7
#define MIU0_STR     0x8
#define MIU1_STR_Pre 0x9
#define MIU1_STR     0xa