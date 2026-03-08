#include "halSEAL.h"
#include "../../drvSEAL.h"

#define __delay() ()

//[SEAL][HAL][009] Non-PM Domain IP List [START]
//Non PM IP group
#define SEAL_NON_PM_GROUP0 \
    E_SEAL_RIU_DBG_PROT_NONPM, \
    E_SEAL_MSPI0_PROT_NONPM, \
    E_SEAL_VD_MHEG5_PROT_NONPM, \
    E_SEAL_MAU1_PROT_NONPM, \
    E_SEAL_HIREG_PROT_NONPM, \
    E_SEAL_POR_STATUS_PROT_NONPM, \
    E_SEAL_INTR_CPUINT_PROT_NONPM, \
    E_SEAL_NORPF_PROT_NONPM, \
    E_SEAL_MIU2_PROT_NONPM, \
    E_SEAL_USB0_PROT_NONPM, \
    E_SEAL_DUMMY,  \
    E_SEAL_UPLL0_PROT_NONPM, \
    E_SEAL_UPLL1_PROT_NONPM, \
    E_SEAL_BDMA_CH0_PROT_NONPM, \
    E_SEAL_BDMA_CH1_PROT_NONPM, \
    E_SEAL_UART3_PROT_NONPM

#define SEAL_NON_PM_GROUP1 \
    E_SEAL_UART0_PROT_NONPM, \
    E_SEAL_UART4_PROT_NONPM, \
    E_SEAL_MSPI1_PROT_NONPM, \
    E_SEAL_CLKGEN0_PROT_NONPM, \
    E_SEAL_DSCRMB_PROT_NONPM, \
    E_SEAL_DUMMY,  \
    E_SEAL_MHEG5_PROT_NONPM, \
    E_SEAL_DUMMY,  \
    E_SEAL_MVD_PROT_NONPM, \
    E_SEAL_MIU_PROT_NONPM, \
    E_SEAL_MVOPSUB_PROT_NONPM, \
    E_SEAL_MVOP_PROT_NONPM, \
    E_SEAL_TSP0_PROT_NONPM, \
    E_SEAL_TSP1_PROT_NONPM, \
    E_SEAL_JPD_PROT_NONPM, \
    E_SEAL_SEMAPH_PROT_NONPM

#define SEAL_NON_PM_GROUP2 \
    E_SEAL_DUMMY,  \
    E_SEAL_DUMMY,  \
    E_SEAL_ECBRIDGE_PROT_NONPM, \
    E_SEAL_INTR_CTRL_PROT_NONPM, \
    E_SEAL_HDMI2_PROT_NONPM, \
    E_SEAL_HVD_PROT_NONPM, \
    E_SEAL_TSP2_PROT_NONPM, \
    E_SEAL_MIPS_PROT_NONPM, \
    E_SEAL_CHIP_PROT_NONPM, \
    E_SEAL_GOP_PROT_NONPM, \
    E_SEAL_EMAC0_PROT_NONPM, \
    E_SEAL_EMAC1_PROT_NONPM, \
    E_SEAL_EMAC2_PROT_NONPM, \
    E_SEAL_EMAC3_PROT_NONPM, \
    E_SEAL_UHC0_PROT_NONPM,  \
    E_SEAL_ADC_ATOP_PROT_NONPM

#define SEAL_NON_PM_GROUP3 \
    E_SEAL_ADC_DTOP_PROT_NONPM, \
    E_SEAL_HDMI_PROT_NONPM,\
    E_SEAL_GE0_PROT_NONPM, \
    E_SEAL_SMART_PROT_NONPM, \
    E_SEAL_CI_PROT_NONPM, \
    E_SEAL_MIIC0_PROT_NONPM, \
    E_SEAL_MIIC1_PROT_NONPM, \
    E_SEAL_MIIC2_PROT_NONPM, \
    E_SEAL_MIIC3_PROT_NONPM, \
    E_SEAL_LDM_DMA0_PROT_NONPM, \
    E_SEAL_SC0_PROT_NONPM, \
    E_SEAL_SC1_PROT_NONPM, \
    E_SEAL_SC2_PROT_NONPM, \
    E_SEAL_SC3_PROT_NONPM, \
    E_SEAL_SC4_PROT_NONPM, \
    E_SEAL_CLKGEN1_PROT_NONPM

#define SEAL_NON_PM_GROUP4 \
    E_SEAL_MAILBOX_PROT_NONPM, \
    E_SEAL_MIIC_PROT_NONPM, \
    E_SEAL_PCM_PROT_NONPM, \
    E_SEAL_VDMCU51_IF_PROT_NONPM, \
    E_SEAL_DMDMCU51_IF_PROT_NONPM, \
    E_SEAL_PM_PROT_NONPM, \
    E_SEAL_URDMA_PROT_NONPM, \
    E_SEAL_AFEC_PROT_NONPM, \
    E_SEAL_COMB_PROT_NONPM, \
    E_SEAL_VBI_PROT_NONPM, \
    E_SEAL_SCM_PROT_NONPM, \
    E_SEAL_PATGEN_PROT_NONPM, \
    E_SEAL_UTMI1_PROT_NONPM, \
    E_SEAL_UTMI_PROT_NONPM, \
    E_SEAL_VE_0_PROT_NONPM,  \
    E_SEAL_REG_PIU_NONPM_PROT_NONPM

#define SEAL_NON_PM_GROUP5 \
    E_SEAL_ADC_ATOPB_PROT_NONPM, \
    E_SEAL_VE_1_PROT_NONPM, \
    E_SEAL_VE_2_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_MPIF_PROT_NONPM, \
    E_SEAL_MIIC4_PROT_NONPM, \
    E_SEAL_MIIC5_PROT_NONPM, \
    E_SEAL_UART1_PROT_NONPM, \
    E_SEAL_UART2_PROT_NONPM, \
    E_SEAL_FUART_PROT_NONPM, \
    E_SEAL_UART5_PROT_NONPM, \
    E_SEAL_GE1_PROT_NONPM, \
    E_SEAL_G3D_PROT_NONPM, \
    E_SEAL_DVI_ATOP_PROT_NONPM, \
    E_SEAL_DVI_DTOP_PROT_NONPM

#define SEAL_NON_PM_GROUP6 \
    E_SEAL_DVIEQ_PROT_NONPM, \
    E_SEAL_HDCP_PROT_NONPM, \
    E_SEAL_TS_SAMPLE_NONPM, \
    E_SEAL_ANA_MISC_PROT_NONPM, \
    E_SEAL_MIU_ATOP_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_UTMI2_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_MFE0_PROT_NONPM, \
    E_SEAL_MFE1_PROT_NONPM, \
    E_SEAL_ADC_DTOPB_PROT_NONPM, \
    E_SEAL_NFIE0_PROT_NONPM, \
    E_SEAL_NFIE1_PROT_NONPM, \
    E_SEAL_NFIE2_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_LDM_DMA1_PROT_NONPM

#define SEAL_NON_PM_GROUP7 \
    E_SEAL_DMD_MCU2_PROT_NONPM, \
    E_SEAL_CLKGEN_DMD_PROT_NONPM, \
    E_SEAL_DEMOD_0_PROT_NONPM, \
    E_SEAL_DEMOD_1_PROT_NONPM, \
    E_SEAL_DEMOD_2_PROT_NONPM, \
    E_SEAL_DEMOD_3_PROT_NONPM, \
    E_SEAL_DEMOD_4_PROT_NONPM, \
    E_SEAL_DEMOD_5_PROT_NONPM, \
    E_SEAL_DEMOD_6_PROT_NONPM, \
    E_SEAL_DEMOD_7_PROT_NONPM, \
    E_SEAL_DMD_ANA_MISC_PROT_NONPM, \
    E_SEAL_AUR20_PROT_NONPM, \
    E_SEAL_VIVALDI0_PROT_NONPM, \
    E_SEAL_VIVALDI1_PROT_NONPM, \
    E_SEAL_VIVALDI2_PROT_NONPM, \
    E_SEAL_VIVALDI3_PROT_NONPM

#define SEAL_NON_PM_GROUP8 \
    E_SEAL_VIVALDI4_PROT_NONPM, \
    E_SEAL_VIVALDI5_PROT_NONPM, \
    E_SEAL_AUR21_PROT_NONPM, \
    E_SEAL_AUR22_PROT_NONPM, \
    E_SEAL_DVI_ATOP_1_PROT_NONPM, \
    E_SEAL_DVI_DTOP_1_PROT_NONPM, \
    E_SEAL_DVIEQ_1_PROT_NONPM, \
    E_SEAL_HDCP_1_PROT_NONPM, \
    E_SEAL_DVI_ATOP_2_PROT_NONPM, \
    E_SEAL_DVI_DTOP_2_PROT_NONPM, \
    E_SEAL_DVIEQ_2_PROT_NONPM, \
    E_SEAL_HDCP_2_PROT_NONPM, \
    E_SEAL_DVI_PS_PROT_NONPM, \
    E_SEAL_DVI_DTOP_3_PROT_NONPM, \
    E_SEAL_DVIEQ_3_PROT_NONPM, \
    E_SEAL_HDCP_3_PROT_NONPM

#define SEAL_NON_PM_GROUP9 \
    E_SEAL_USB2_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_UHC2_PROT_NONPM, \
    E_SEAL_TSO_PROT_NONPM, \
    E_SEAL_DRM_SECURE_PROT_NONPM, \
    E_SEAL_DSCRMB2_PROT_NONPM, \
    E_SEAL_DSCRMB3_PROT_NONPM, \
    E_SEAL_GPD0_PROT_NONPM, \
    E_SEAL_GPD1_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_GOP4G_0_PROT_NONPM, \
    E_SEAL_GOP4G_1_PROT_NONPM, \
    E_SEAL_GOP4G_ST_PROT_NONPM, \
    E_SEAL_GOP2G_0_PROT_NONPM, \
    E_SEAL_GOP2G_1_PROT_NONPM

#define SEAL_NON_PM_GROUP10 \
    E_SEAL_GOP2G_ST_PROT_NONPM, \
    E_SEAL_GOP1G_0_PROT_NONPM, \
    E_SEAL_GOP1G_1_PROT_NONPM, \
    E_SEAL_GOP1G_ST_PROT_NONPM, \
    E_SEAL_GOP1GX_0_PROT_NONPM, \
    E_SEAL_GOP1GX_1_PROT_NONPM, \
    E_SEAL_GOP1GX_ST_PROT_NONPM, \
    E_SEAL_GOPD_PROT_NONPM, \
    E_SEAL_SPARE0_PROT_NONPM, \
    E_SEAL_SPARE1_PROT_NONPM, \
    E_SEAL_SRAM_LDO, \
    E_SEAL_SPARE2_PROT_NONPM, \
    E_SEAL_SPARE3_PROT_NONPM, \
    E_SEAL_SWCD_PROT_NONPM, \
    E_SEAL_SPARE4_PROT_NONPM, \
    E_SEAL_SPARE5_PROT_NONPM

#define SEAL_NON_PM_GROUP11 \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_TZPC_NONPM2_NONPM, \
    E_SEAL_SPARE6_PROT_NONPM, \
    E_SEAL_SPARE7_PROT_NONPM, \
    E_SEAL_MHL_TMDS_PROT_NONPM, \
    E_SEAL_SEC_R2_PROT_NONPM, \
    E_SEAL_SEC_MAU0_PROT_NONPM, \
    E_SEAL_DSCRMB4_PROT_NONPM, \
    E_SEAL_MAU0_PROT_NONPM, \
    E_SEAL_USBC0_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_USBC2_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_SECURERANGE0_PROT_NONPM

#define SEAL_NON_PM_GROUP12 \
    E_SEAL_SECURERANGE1_PROT_NONPM, \
    E_SEAL_TZPC_NONPM_PROT_NONPM, \
    E_SEAL_HDCPKEY_PROT_NONPM, \
    E_SEAL_NFIE3_PROT_NONPM, \
    E_SEAL_DSCRMB5_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_EMMC_PLL_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_MSC_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_VIVALDI6_PROT_NONPM, \
    E_SEAL_VIVALDI7_PROT_NONPM, \
    E_SEAL_PCM2_PROT_NONPM, \
    E_SEAL_TSP3_PROT_NONPM, \
    E_SEAL_EVD_NONPM, \
    E_SEAL_GPU_PLL_PROT_NONPN

#define SEAL_NON_PM_GROUP13 \
    E_SEAL_MIU_ARB_NONPM, \
    E_SEAL_MIU_ATOP2_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_MIU_ARB2_NONPM, \
    E_SEAL_GE2_NONPM, \
    E_SEAL_GE3_NONPM, \
    E_SEAL_HDCP22_P0_NONPM, \
    E_SEAL_HDCP22_P1_NONPM, \
    E_SEAL_HDCP22_P2_NONPM, \
    E_SEAL_HDCP22_P3_NONPM, \
    E_SEAL_DVI_ATOP3_NONPM, \
    E_SEAL_SPI2FCIE_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_VIVALDI8_PROT_NONPM, \
    E_SEAL_VIVALDI9_PROT_NONPM, \
    E_SEAL_VIVALDIa_0_PROT_NONPM

#define SEAL_NON_PM_GROUP14 \
    E_SEAL_VIVALDIa_1_PROT_NONPM, \
    E_SEAL_VIVALDIa_2_PROT_NONPM, \
    E_SEAL_VIVALDIa_3_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_VIVALDIb_PROT_NONPM, \
    E_SEAL_X32_GPUAPB0_PROT_NONPM, \
    E_SEAL_X32_GPUAPB1_PROT_NONPM, \
    E_SEAL_X32_GPUAPB2_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY
//[SEAL][HAL][009] Non-PM Domain IP List [END]

//[SEAL][HAL][010] PM Domain IP List [START]
//PM IP group
#define SEAL_PM_GROUP0 \
    E_SEAL_RIU_DBG_PROT_PM, \
    E_SEAL_MENULOAD_PROT_PM, \
    E_SEAL_GDMA_PROT_PM, \
    E_SEAL_DDC_PROT_PM, \
    E_SEAL_PM_POR_PROT_PM, \
    E_SEAL_ISP_PROT_PM, \
    E_SEAL_FSP_PROT_PM, \
    E_SEAL_QSPI_PROT_PM, \
    E_SEAL_PM_SLEEP_PROT_PM, \
    E_SEAL_PM_GPIO_PROT_PM, \
    E_SEAL_MCU_PROT_PM, \
    E_SEAL_PM_CEC_PROT_PM, \
    E_SEAL_PM_RTC0_PROT_PM, \
    E_SEAL_PM_RTC1_PROT_PM, \
    E_SEAL_PM_SAR_PROT_PM, \
    E_SEAL_PM_AV_LINK_PROT_PM

#define SEAL_PM_GROUP1 \
    E_SEAL_PM_TOP_PROT_PM, \
    E_SEAL_MHL_CBUS_PROT_PM, \
    E_SEAL_EFUSE_PROT_PM, \
    E_SEAL_IRQ_PROT_PM, \
    E_SEAL_CACHE_PROT_PM, \
    E_SEAL_XDMIU_PROT_PM, \
    E_SEAL_PM_MISC_PROT_PM, \
    E_SEAL_PM_MHL_CBUS_PROT_PM, \
    E_SEAL_WDT_PROT_PM, \
    E_SEAL_TIMER0_PROT_PM, \
    E_SEAL_TIMER1_PROT_PM, \
    E_SEAL_TIMER2_PROT_PM, \
    E_SEAL_DUMMY, \
    E_SEAL_ALBANY0_PROT_PM, \
    E_SEAL_ALBANY1_PROT_PM, \
    E_SEAL_ALBANY2_PROT_PM

#define SEAL_PM_GROUP2 \
    E_SEAL_DID_KEY_PROT_PM, \
    E_SEAL_TZPC_PROT_PM, \
    E_SEAL_REG_PIU_MISC_0_PROT_PM, \
    E_SEAL_IR_PROT_PM, \
    E_SEAL_PM_SPARE0_PROT_PM, \
    E_SEAL_PM_SPARE1_PROT_PM, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY

//[SEAL][HAL][010] PM Domain IP List [END]
//[SEAL][HAL][011] Processor List [START]
//processor group
#define SEAL_PROCESSOR_GROUP0 \
    E_SEAL_DBBUS, \
    E_SEAL_MCU51, \
    E_SEAL_CPU2, \
    E_SEAL_VD_R2, \
    E_SEAL_SECURE_R2, \
    E_SEAL_SC, \
    E_SEAL_CMDQ, \
    E_SEAL_HEMCU ,\
    E_SEAL_PROCESSOR_NUM
//[SEAL][HAL][011] Processor List [END]
//[SEAL][HAL][012] Lock IP List [START]
//lock group

#define SEAL_LOCK_GROUP0 \
    E_SEAL_SC_WP_DIPW, \
    E_SEAL_SC_RP_DIPW, \
    E_SEAL_SC_WP_DIPR, \
    E_SEAL_SC_RP_DIPR, \
    E_SEAL_AUDIO_LCL_R2_WR_PROTN_0, \
    E_SEAL_AUDIO_LCL_R2_WR_PROTN_1, \
    E_SEAL_AUDIO_LCL_R2_WR_PROTN_2, \
    E_SEAL_AUDIO_LCL_R2_WR_PROTN_3, \
    E_SEAL_AUDIO_SCL_R2_WR_PROTN_0, \
    E_SEAL_AUDIO_SCL_R2_WR_PROTN_1, \
    E_SEAL_AUDIO_SCL_R2_WR_PROTN_2, \
    E_SEAL_AUDIO_SCL_R2_WR_PROTN_3, \
    E_SEAL_SECURE_R2_WR_PROTN_0, \
    E_SEAL_SECURE_R2_WR_PROTN_1, \
    E_SEAL_SECURE_R2_WR_PROTN_2, \
    E_SEAL_SECURE_R2_WR_PROTN_3

#define SEAL_LOCK_GROUP1 \
    E_SEAL_CIPHERENG_WP_SYSKEY, \
    E_SEAL_CIPHERENG_RP_SYSKEY, \
    E_SEAL_DSCRMB_WP_RIV0, \
    E_SEAL_DSCRMB_RP_RIV0, \
    E_SEAL_DSCRMB_WP_RIV1, \
    E_SEAL_DSCRMB_RP_RIV1, \
    E_SEAL_DSCRMB_WP_WDATA, \
    E_SEAL_DSCRMB_RP_WDATA, \
    E_SEAL_DSCRMB_WP_RIV2, \
    E_SEAL_DSCRMB_RP_RIV2, \
    E_SEAL_DSCRMB_WP_RIV3, \
    E_SEAL_DSCRMB_RP_RIV3, \
    E_SEAL_SC2_WP_SCM_M, \
    E_SEAL_SC2_RP_SCM_M, \
    E_SEAL_MVOP_WP_TLB, \
    E_SEAL_AUDIO_PAS_PROTN

#define SEAL_LOCK_GROUP2 \
    E_SEAL_DSCRMB_RP_RDATA, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_RNG_PROTECT, \
    E_SEAL_RSA_PROTECT, \
    E_SEAL_SHA_PROTECT, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_EVD_R2_WR_PROTN_0, \
    E_SEAL_EVD_R2_WR_PROTN_1, \
    E_SEAL_EVD_R2_WR_PROTN_2, \
    E_SEAL_EVD_R2_WR_PROTN_3, \
    E_SEAL_HVD_R2_WR_PROTN_0, \
    E_SEAL_HVD_R2_WR_PROTN_1, \
    E_SEAL_HVD_R2_WR_PROTN_2, \
    E_SEAL_HVD_R2_WR_PROTN_3

#define SEAL_LOCK_GROUP3 \
    E_SEAL_HVD_ES0_BUF, \
    E_SEAL_HVD_ES1_BUF, \
    E_SEAL_HVD_ES2_BUF, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_MFE0_ES_BUF, \
    E_SEAL_MVD_WR_PROTN_0, \
    E_SEAL_MVD_WR_PROTN_1, \
    E_SEAL_MVD_WR_PROTN_2, \
    E_SEAL_EVD_0_WR_PROTN_0, \
    E_SEAL_EVD_1_WR_PROTN_0, \
    E_SEAL_MHEG5_WR_PROTN_0, \
    E_SEAL_AUDIO_DSP_ES_PROTN, \
    E_SEAL_AUDIO_DSP_CACHE_PROTN, \
    E_SEAL_EVD_0_WR_PROTN_1, \
    E_SEAL_EVD_1_WR_PROTN_1, \
    E_SEAL_VP9_TOP

#define SEAL_LOCK_GROUP4 \
    E_SEAL_TSO_WP_TSOFI, \
    E_SEAL_TSO_RP_TSOFI, \
    E_SEAL_MMFI_WP_MMFI0, \
    E_SEAL_MMFI_RP_MMFI0, \
    E_SEAL_MMFI_WP_MMFI1, \
    E_SEAL_MMFI_RP_MMFI1, \
    E_SEAL_TSP0_WP_PVR, \
    E_SEAL_TSP0_RP_PVR, \
    E_SEAL_TSP0_WP_PVR1, \
    E_SEAL_TSP0_RP_PVR1, \
    E_SEAL_TSP0_WP_FILEIN, \
    E_SEAL_TSP0_RP_FILEIN, \
    E_SEAL_TSP0_WP_QMEM, \
    E_SEAL_TSP0_RP_QMEM, \
    E_SEAL_TSP0_WP_FW, \
    E_SEAL_TSP0_RP_FW

#define SEAL_LOCK_GROUP5 \
    E_SEAL_VE_WP, \
    E_SEAL_VE_RP, \
    E_SEAL_SC_WP_OD, \
    E_SEAL_SC_RP_OD, \
    E_SEAL_SC_WP_SCM_M, \
    E_SEAL_SC_RP_SCM_M, \
    E_SEAL_SC_WP_SCM_S, \
    E_SEAL_SC_RP_SCM_S, \
    E_SEAL_SC_WP_PDW0, \
    E_SEAL_SC_RP_PDW0, \
    E_SEAL_SC_WP_PDW1, \
    E_SEAL_SC_RP_PDW1, \
    E_SEAL_SC_WP_OPW, \
    E_SEAL_SC_RP_OPW, \
    E_SEAL_GOPD_PROTN, \
    E_SEAL_AUDIO_AL_PROTN

#define SEAL_LOCK_GROUP6 \
    E_SEAL_GE0_SB_PROTN, \
    E_SEAL_GE0_DB_PROTN, \
    E_SEAL_GE1_SB_PROTN, \
    E_SEAL_GE1_DB_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY
//[SEAL][HAL][012] Lock IP List [END]

#define MIU0_PA_BASE               0x00000000UL
#define MIU1_PA_BASE               0x80000000UL
#define MIU2_PA_BASE               0xC0000000UL

#if 0
const eSeal_IP NonPmIpTbl[SEAL_NONPM_TBL_IP_NUM] =
{
    SEAL_NON_PM_GROUP0, SEAL_NON_PM_GROUP1, SEAL_NON_PM_GROUP2,
    SEAL_NON_PM_GROUP3, SEAL_NON_PM_GROUP4, SEAL_NON_PM_GROUP5,
    SEAL_NON_PM_GROUP6, SEAL_NON_PM_GROUP7, SEAL_NON_PM_GROUP8,
    SEAL_NON_PM_GROUP9, SEAL_NON_PM_GROUP10, SEAL_NON_PM_GROUP11,
    SEAL_NON_PM_GROUP12, SEAL_NON_PM_GROUP13, SEAL_NON_PM_GROUP14,
};
#endif

const int LockTbl[SEAL_TBL_LOCK_NUM] =
{
    SEAL_LOCK_GROUP0, /*0x74*/
    SEAL_LOCK_GROUP1,
    SEAL_LOCK_GROUP2,
    SEAL_LOCK_GROUP3,
    SEAL_LOCK_GROUP4,
    SEAL_LOCK_GROUP5,
    SEAL_LOCK_GROUP6,
};

static unsigned long long __PA2BA(unsigned long long u64PhyAddr)
{
    unsigned long long u64BusAddr = 0x0;

    if( (u64PhyAddr >= MIU0_PA_BASE) && (u64PhyAddr < MIU1_PA_BASE) ) // MIU0
        u64BusAddr = u64PhyAddr - MIU0_PA_BASE + CONFIG_MIU0_BUSADDR;
    else if ( (u64PhyAddr >= MIU1_PA_BASE) ) // MIU1
        u64BusAddr = u64PhyAddr - MIU1_PA_BASE + CONFIG_MIU1_BUSADDR;

    return u64BusAddr;
}

#if 0
static int HAL_SEAL_GetNonPmIpIdx(int eNonPmIP)
{
    int s32HwIdx;
    for (s32HwIdx = 0; s32HwIdx < SEAL_NONPM_TBL_IP_NUM; s32HwIdx++)
    {
        if(eNonPmIP == NonPmIpTbl[s32HwIdx])
        {
            return s32HwIdx;
        }
    }
    return (-1);
}
#endif

static int HAL_SEAL_GetLockIdx(int eLockId)
{
    int s32HwIdx;
    for (s32HwIdx = 0; s32HwIdx < SEAL_TBL_LOCK_NUM; s32HwIdx++)
    {
        if(eLockId == LockTbl[s32HwIdx])
        {
            return s32HwIdx;
        }
    }
    return (-1);
}


int HAL_SEAL_SetSecureRange(unsigned long long startAddr, unsigned long long endAddr, int attr)
{
    unsigned int bankBase;
    unsigned int startRegOffset;
    unsigned int endRegOffset;
    unsigned int attrRegOffset;
    unsigned int miuSel;
    unsigned int miuSelEnd;
    unsigned long long startOffset;
    unsigned long long endOffset;
    unsigned int offset;
    int i, free;

    _phy_to_miu_offset(miuSel, startOffset, startAddr);
    _phy_to_miu_offset(miuSelEnd, endOffset, endAddr);

    if (miuSel != miuSelEnd)
    {
        return 0;
    }

    if (miuSel==0)
    {
        bankBase = SEAL_SECURE0_RANGE0;
    }
    else if (miuSel==1)
    {
        bankBase = SEAL_SECURE1_RANGE0;
    }
    else
    {
        return 0;
    }

    // check same range
    for (i=0; i<REG_SECURE_RANGE_NUM; i++)
    {
        unsigned long long start = 0;
        unsigned long long end = 0;
        offset = i*REG_SECURE_RANGE_OFFSET;
        startRegOffset = REG_SECURE_RANGE0_START_ADDR + offset;
        endRegOffset = REG_SECURE_RANGE0_END_ADDR + offset;
        start |= RIU[bankBase + startRegOffset + 4];
        start <<= 16;
        start |= RIU[bankBase + startRegOffset + 2];
        start <<= 16;
        start |= RIU[bankBase + startRegOffset];
        end |= RIU[bankBase + endRegOffset + 4];
        end <<= 16;
        end |= RIU[bankBase + endRegOffset + 2];
        end <<= 16;
        end |= RIU[bankBase + endRegOffset];

        // same range, only set attribute
        if ( (start == startOffset)
            && (end == endOffset) )
        {
            RIU[bankBase + attrRegOffset] = attr;
            return 1;
        }
    }

    // find an unused range
    for (free=0; free<REG_SECURE_RANGE_NUM; free++)
    {
        if ( ((RIU[bankBase + 0x0E + (free<<4)]) == 0x000F) )
        {
            break;
        }
    }

    if (free >= REG_SECURE_RANGE_NUM) // no free range
    {
        return 0;
    }

    offset = free*REG_SECURE_RANGE_OFFSET;
    startRegOffset = REG_SECURE_RANGE0_START_ADDR + offset;
    endRegOffset = REG_SECURE_RANGE0_END_ADDR + offset;
    attrRegOffset = REG_SECURE_RANGE0_ATTRIBUTE + offset;

    RIU[bankBase + startRegOffset] = (startOffset & 0xFFFFULL);
    RIU[bankBase + startRegOffset + 2] = ((startOffset>>16) & 0xFFFFULL);
    RIU[bankBase + startRegOffset + 4] = ((startOffset>>32) & 0xFFFFULL);

    RIU[bankBase + endRegOffset] = (endOffset & 0xFFFFULL);
    RIU[bankBase + endRegOffset + 2] = ((endOffset>>16) & 0xFFFFULL);
    RIU[bankBase + endRegOffset + 4] = ((endOffset>>32) & 0xFFFFULL);

    RIU[bankBase + attrRegOffset] = attr;

    return 1;
}

int HAL_SEAL_DisableRIUBridges(void)
{
    // set hosts of PM RIU bridge to non-secure
    RIU[SEAL_TZPC_PM_BASE + REG_PM_RIU_BRIDGE] = (0x5555);

    // set hosts of NONPM RIU bridge to non-secure (except ARM)
    RIU[SEAL_TZPC_NONPM_BASE + REG_NONPM_RIU_BRIDGE] = (0x5555);

    // Set ARM to non-secure
	// Set ARM to non-seucre in TEE Loader via eret instead
    //(RIU[REG_TZPC_LEGACY_SECUREPROCESSOR]) |= (FLAG_TZPC_NONSECURE_ARM);

    return 1;
}

int HAL_SEAL_Init(void)
{
    // Disable ejtag to TEE debugging capability
    (RIU[SEAL_TZPC_NONPM_BASE + (0x02<<1)]) &= (0x00ff);

    // mask decode error when non-secure CPU access miu secure range
    // mask decode error when non-secure CPU access secure riu register bank
    // mask decode error when CPU access undefined address area
    (RIU[SEAL_TZPC_NONPM_BASE + (0x7e<<1)]) |= (0x000e);

#if 0
    // SEC R2
    if(!HAL_SEAL_SetRIUBank(E_SEAL_SEC_R2_PROT_NONPM, 1)
       || !HAL_SEAL_SetRIUBank(E_SEAL_SEC_MAU0_PROT_NONPM, 1))
    {
        return 0;
    }
#endif
    (RIU[REG_TZPC_NONPM_SECURE_BANK + ((183>>4)<<1)]) &= ~(1<<(183&15));
    (RIU[REG_TZPC_NONPM_SECURE_BANK + ((184>>4)<<1)]) &= ~(1<<(184&15));

//    HAL_SEAL_SetMIUHost(E_SEAL_SEC_R2_PROT_NONPM, 1);
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((183>>4)<<1)]) &= ~(1<<(183&15));

    // add for SEAL.ini
    HAL_SEAL_SetRIURegister(E_SEAL_HVD_ES0_BUF, 1);
    HAL_SEAL_SetRIURegister(E_SEAL_HVD_ES1_BUF, 1);
    HAL_SEAL_SetRIURegister(E_SEAL_HVD_ES2_BUF, 1);
    HAL_SEAL_SetRIURegister(E_SEAL_MFE0_ES_BUF, 1);
    HAL_SEAL_SetRIURegister(E_SEAL_DSCRMB_RP_RIV0, 1);
    HAL_SEAL_SetRIURegister(E_SEAL_DSCRMB_RP_RIV1, 1);
    HAL_SEAL_SetRIURegister(E_SEAL_DSCRMB_RP_WDATA, 1);
    HAL_SEAL_SetRIURegister(E_SEAL_DSCRMB_RP_RDATA, 1);

#if 0
    HAL_SEAL_SetMIUHost(E_SEAL_SC1_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_MSC_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_MVOPSUB_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_MVOP_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP4G_0_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP4G_1_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP4G_ST_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP2G_0_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP2G_1_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP2G_ST_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP1G_0_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP1G_1_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP1G_ST_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP1GX_0_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP1GX_1_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOP1GX_ST_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_GOPD_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_AUR20_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_VIVALDI0_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_VIVALDI1_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_VIVALDI2_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_VIVALDI3_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_VIVALDI4_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_VIVALDI5_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_AUR21_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_AUR22_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_VD_MHEG5_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_MVD_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_HVD_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_MFE0_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_MFE1_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_EVD_NONPM, 1);
#endif
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((59>>4)<<1)]) &= ~(1<<(59&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((200>>4)<<1)]) &= ~(1<<(200&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((26>>4)<<1)]) &= ~(1<<(26&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((27>>4)<<1)]) &= ~(1<<(27&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((41>>4)<<1)]) &= ~(1<<(41&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((155>>4)<<1)]) &= ~(1<<(155&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((156>>4)<<1)]) &= ~(1<<(156&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((157>>4)<<1)]) &= ~(1<<(157&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((158>>4)<<1)]) &= ~(1<<(158&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((159>>4)<<1)]) &= ~(1<<(159&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((160>>4)<<1)]) &= ~(1<<(160&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((161>>4)<<1)]) &= ~(1<<(161&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((162>>4)<<1)]) &= ~(1<<(162&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((163>>4)<<1)]) &= ~(1<<(163&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((164>>4)<<1)]) &= ~(1<<(164&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((165>>4)<<1)]) &= ~(1<<(165&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((166>>4)<<1)]) &= ~(1<<(166&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((167>>4)<<1)]) &= ~(1<<(167&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((123>>4)<<1)]) &= ~(1<<(123&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((124>>4)<<1)]) &= ~(1<<(124&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((125>>4)<<1)]) &= ~(1<<(125&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((126>>4)<<1)]) &= ~(1<<(126&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((127>>4)<<1)]) &= ~(1<<(127&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((128>>4)<<1)]) &= ~(1<<(128&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((129>>4)<<1)]) &= ~(1<<(129&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((130>>4)<<1)]) &= ~(1<<(130&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((131>>4)<<1)]) &= ~(1<<(131&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((2>>4)<<1)]) &= ~(1<<(2&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((24>>4)<<1)]) &= ~(1<<(24&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((37>>4)<<1)]) &= ~(1<<(37&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((104>>4)<<1)]) &= ~(1<<(104&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((105>>4)<<1)]) &= ~(1<<(105&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((206>>4)<<1)]) &= ~(1<<(206&15));

#if 0
    // secure AESDMA(TRNG)
    if(!HAL_SEAL_SetRIUBank(E_SEAL_DSCRMB3_PROT_NONPM, 1))
        return 0;
#endif
    (RIU[REG_TZPC_NONPM_SECURE_BANK + ((150>>4)<<1)]) &= ~(1<<(150&15));

//    HAL_SEAL_SetMIUHost(E_SEAL_DSCRMB3_PROT_NONPM, 1);
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((150>>4)<<1)]) &= ~(1<<(150&15));


    // secure NFIE/RSA/SHA
#if 0
    HAL_SEAL_SetMIUHost(E_SEAL_DSCRMB5_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_NFIE0_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_NFIE1_PROT_NONPM, 1);
    HAL_SEAL_SetMIUHost(E_SEAL_NFIE2_PROT_NONPM, 1);
#endif
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((196>>4)<<1)]) &= ~(1<<(196&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((107>>4)<<1)]) &= ~(1<<(107&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((108>>4)<<1)]) &= ~(1<<(108&15));
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((109>>4)<<1)]) &= ~(1<<(109&15));

    RIU[SEAL_TZPC_PM_BASE + REG_PM_RIU_BRIDGE] = (0x5454);
    RIU[SEAL_TZPC_NONPM_BASE + REG_NONPM_RIU_BRIDGE] = (0x5454);

    return 1;
}

#if 0
int HAL_SEAL_SetMIUHost(int swIdx, int bSecure)
{
    int hwIdx = HAL_SEAL_GetNonPmIpIdx(swIdx);

    if (hwIdx < 0)
    {
        return 0;
    }

    if (bSecure)
        (RIU[REG_TZPC_NONPM_MIU_HOST + ((hwIdx>>4)<<1)]) &= ~(1<<(hwIdx&15));
    else
        (RIU[REG_TZPC_NONPM_MIU_HOST + ((hwIdx>>4)<<1)]) |= (1<<(hwIdx&15));

    return 1;
}

int HAL_SEAL_SetRIUBank(int swIdx, int bSecure)
{
    int hwIdx = HAL_SEAL_GetNonPmIpIdx(swIdx);
    if (hwIdx < 0)
    {
        return 0;
    }

    if (bSecure)
        (RIU[REG_TZPC_NONPM_SECURE_BANK + ((hwIdx>>4)<<1)]) &= ~(1<<(hwIdx&15));
    else
        (RIU[REG_TZPC_NONPM_SECURE_BANK + ((hwIdx>>4)<<1)]) |= (1<<(hwIdx&15));

    return 1;
}
#endif


int HAL_SEAL_SetRIURegister(int swIdx, int bSecure)
{
    int hwIdx = HAL_SEAL_GetLockIdx(swIdx);
    if (hwIdx < 0)
    {
        return 0;
    }

    if (bSecure)
        (RIU[REG_TZPC_NONPM_SECURE_REG + ((hwIdx>>4)<<1)]) &= ~(1<<(hwIdx&15));
    else
        (RIU[REG_TZPC_NONPM_SECURE_REG + ((hwIdx>>4)<<1)]) |= (1<<(hwIdx&15));
    return 1;
}


int HAL_SEAL_Cleanup(void)
{
    //reg_rom_security_oneway_prot
	  (RIU[0x101800+(0x59<<1)]) |= 1;
    // non secure RSA/SHA
    //HAL_SEAL_SetMIUHost(E_SEAL_DSCRMB5_PROT_NONPM, 0);
    (RIU[REG_TZPC_NONPM_MIU_HOST + ((196>>4)<<1)]) |= (1<<(196&15));
    return 1;
}
