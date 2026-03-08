#include "halSEAL.h"
#include "../../drvSEAL.h"

static int _gSealLocalDelayVar;
static int flag_dummyread = -1;

/* I tried to reduce the loop count, but then SRAM won't be set correctly, so be cautious */
#define __delay() {int j; for (j=0; j<1000000; j+=2) {j=j-1; _gSealLocalDelayVar+=j;}}

//Non PM IP group
#define SEAL_NON_PM_GROUP0 \
    E_SEAL_DUMMY, \
    E_SEAL_DEMOD_PROT_NONPM, \
    E_SEAL_AESDMA_PROT_NONPM, \
    E_SEAL_AESDMA_PROTECTED_PROT_NONPM, \
    E_SEAL_ARM_D_NON_SECURE_PROT_NONPM, \
    E_SEAL_ARM_D_SECURE_PROT_NONPM, \
    E_SEAL_ARM_I_NON_SECURE_PROT_NONPM, \
    E_SEAL_ARM_I_SECURE_PROT_NONPM, \
    E_SEAL_ARM_MFDEC_NORMAL_PROT_NONPM, \
    E_SEAL_ARM_MFDEC_PROTECTED_PROT_NONPM, \
    E_SEAL_AU_R2_1_NONPM, \
    E_SEAL_AU_R2_PROT_NONPM, \
    E_SEAL_AUTODL_PROT_NONPM, \
    E_SEAL_BDMA_PROT_NONPM, \
    E_SEAL_COMB_PROT_NONPM, \
    E_SEAL_DIP_0_PROT_NONPM

#define SEAL_NON_PM_GROUP1 \
    E_SEAL_DIP_0_PROTECTED_PROT_NONPM, \
    E_SEAL_DIPR_0_PROT_NONPM, \
    E_SEAL_DIPR_0_PROTECTED_PROT_NONPM, \
    E_SEAL_DS_PROT_NONPM, \
    E_SEAL_DWIN_0_PROT_NONPM, \
    E_SEAL_DWIN_0_PROTECTED_PROT_NONPM, \
    E_SEAL_EMAC_PROT_NONPM, \
    E_SEAL_EVD_NONPM, \
    E_SEAL_EVD_PROTECTED_PROT_NONPM, \
    E_SEAL_EVD_R2_D_PROT_NONPM, \
    E_SEAL_EVD_R2_I_PROT_NONPM, \
    E_SEAL_FCIE_PROT_NONPM, \
    E_SEAL_FRC_R2_D_PROT_NONPM, \
    E_SEAL_FRC_R2_I_PROT_NONPM, \
    E_SEAL_GE0_PROT_NONPM, \
    E_SEAL_GE1_PROT_NONPM

#define SEAL_NON_PM_GROUP2 \
    E_SEAL_GECMQ_PROT_NONPM, \
    E_SEAL_GOP0G_0_PROT_NONPM, \
    E_SEAL_GOPG0_0_PROTECTED_PROT_NONPM, \
    E_SEAL_GOP1G_0_PROT_NONPM, \
    E_SEAL_GOPG1_0_PROTECTED_PROT_NONPM, \
    E_SEAL_GOP2G_0_PROT_NONPM, \
    E_SEAL_GOPG2_0_PROTECTED_PROT_NONPM, \
    E_SEAL_GPD_PROT_NONPM, \
    E_SEAL_MIU_CMD_PROT_NONPM, \
    E_SEAL_GPU_PROTECTED_PROT_NONPM, \
    E_SEAL_GPU_NORMAL_PROT_NONPM, \
    E_SEAL_HVD_PROT_NONPM, \
    E_SEAL_HVD_PROTECTED_PROT_NONPM, \
    E_SEAL_HVDM0_R_PROT_NONPM, \
    E_SEAL_HVDM0_W_PROT_NONPM, \
    E_SEAL_IOMMU_PAGE_WALK_PROT_NONPM

#define SEAL_NON_PM_GROUP3 \
    E_SEAL_IOMMU_TLB_INVALID_DUMMY_PROT_NONPM, \
    E_SEAL_IPM0_PROT_NONPM, \
    E_SEAL_JPD_PROT_NONPM, \
    E_SEAL_LD_PROT_NONPM, \
    E_SEAL_MEHR_W_PROT_NONPM, \
    E_SEAL_MEMV_R_PROT_NONPM, \
    E_SEAL_MEMV_W_PROT_NONPM, \
    E_SEAL_MIHR_R_PROT_NONPM, \
    E_SEAL_MIIC0_PROT_NONPM, \
    E_SEAL_MIIC1_PROT_NONPM, \
    E_SEAL_MIIC2_PROT_NONPM, \
    E_SEAL_MIMV_R_PROT_NONPM, \
    E_SEAL_MIU_BIST_PROT_NONPM, \
    E_SEAL_MVD_BBU_RD_PROT_NONPM, \
    E_SEAL_MVD_BBU_RD_PROTECTED_PROT_NONPM, \
    E_SEAL_MVD_BBU_WR_PROT_NONPM

#define SEAL_NON_PM_GROUP4 \
    E_SEAL_MVD_BBU_WR_PROTECTED_PROT_NONPM, \
    E_SEAL_MVD_ENG_PROT_NONPM, \
    E_SEAL_MVD_ENG_PROTECTED_PROT_NONPM, \
    E_SEAL_MVOP_PROT_NONPM, \
    E_SEAL_MVOP_PROTECTED_PROT_NONPM, \
    E_SEAL_OD_PROT_NONPM, \
    E_SEAL_OPM0_PROT_NONPM, \
    E_SEAL_OPME0_PROT_NONPM, \
    E_SEAL_OPMI0_PROT_NONPM, \
    E_SEAL_PM51_PROT_NONPM, \
    E_SEAL_SC_ADC_DMA_PROT_NONPM, \
    E_SEAL_SC0_PROT_NONPM, \
    E_SEAL_SC0_PROTECTED_PROT_NONPM, \
    E_SEAL_HDR_DMA_PROT_NONPM, \
    E_SEAL_HDR_DMA_PROTECTED_PROT_NONPM, \
    E_SEAL_TSO_FILEIN_PROT_NONPM

#define SEAL_NON_PM_GROUP5 \
    E_SEAL_TSP_AEON_D_2MIU_PROT_NONPM, \
    E_SEAL_TSP_AEON_I_2MIU_PROT_NONPM, \
    E_SEAL_TSP_FILEIN0_PROT_NONPM, \
    E_SEAL_TSP_FIQ_PROT_NONPM, \
    E_SEAL_TSP_MMFI0_PROT_NONPM, \
    E_SEAL_TSP_MMFI1_PROT_NONPM, \
    E_SEAL_TSP_NON_SECURE_PROT_NONPM, \
    E_SEAL_TSP_OR_PROT_NONPM, \
    E_SEAL_TSP_PVR1_PROT_NONPM, \
    E_SEAL_TSP_PVR2_PROT_NONPM, \
    E_SEAL_TSP_SEC_PROT_NONPM, \
    E_SEAL_TSP_VQ_PROT_NONPM, \
    E_SEAL_TTX_PROT_NONPM, \
    E_SEAL_UH0_PROT_NONPM, \
    E_SEAL_UH1_PROT_NONPM, \
    E_SEAL_UH2_PROT_NONPM

#define SEAL_NON_PM_GROUP6 \
    E_SEAL_UH3_PROT_NONPM, \
    E_SEAL_URDMA_PROT_NONPM, \
    E_SEAL_VIVALDI0_PROT_NONPM, \
    E_SEAL_VIVALDI1_PROT_NONPM, \
    E_SEAL_VIVALDI6_PROT_NONPM, \
    E_SEAL_VIVALDI7_PROT_NONPM, \
    E_SEAL_VIVALDIa_2_PROT_NONPM, \
    E_SEAL_SDIO_PROT_NONPM, \
    E_SEAL_VIVALDI_AUBDMA_PROT_NONPM, \
    E_SEAL_LG_W_PROT_NONPM, \
    E_SEAL_LG_R_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY, \
    E_SEAL_DUMMY

//lock group
#define SEAL_LOCK_GROUP_DUMMY \
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
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_GROUP0 \
    E_SEAL_AUDIO_R2_2_WR_PROTN_0, \
    E_SEAL_AUDIO_R2_2_WR_PROTN_1, \
    E_SEAL_AUDIO_R2_2_WR_PROTN_2, \
    E_SEAL_AUDIO_R2_2_WR_PROTN_3, \
    E_SEAL_AUDIO_LCL_R2_WR_PROTN_0, \
    E_SEAL_AUDIO_LCL_R2_WR_PROTN_1, \
    E_SEAL_AUDIO_LCL_R2_WR_PROTN_2, \
    E_SEAL_AUDIO_LCL_R2_WR_PROTN_3, \
    E_SEAL_AUDIO_SCL_R2_WR_PROTN_0, \
    E_SEAL_AUDIO_SCL_R2_WR_PROTN_1, \
    E_SEAL_AUDIO_SCL_R2_WR_PROTN_2, \
    E_SEAL_AUDIO_SCL_R2_WR_PROTN_3, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_GROUP1 \
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
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_AUDIO_PAS_PROTN

#define SEAL_LOCK_GROUP2 \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_PCIERC_IBWP_PROTN, \
    E_SEAL_PCIERC_OBWP_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
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
    E_SEAL_HVD_ES3_BUF, \
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
    E_SEAL_ALL_PAD_IN_WR_PROTN, \
    E_SEAL_NAND_SEC_RANGE_CLK_PROTN, \
    E_SEAL_NAND_SEC_RANGE_PAD_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_IDAC_WP_N, \
    E_SEAL_IDAC_RP_N, \
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

#define SEAL_LOCK_GROUP5 \
    E_SEAL_HVD_ES4_BUF, \
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
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
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

#define SEAL_LOCK_GROUP7 \
    E_SEAL_BIST_PROTN, \
    E_SEAL_MTCMOS_WR_PROTN, \
    E_SEAL_OTP_LDO_WR_PROTN, \
    E_SEAL_PAD_CTRL_WR_PROTN, \
    E_SEAL_PAD_MUX_WR_PROTN, \
    E_SEAL_PLL_GATER_WR_PROTN, \
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

#define SEAL_LOCK_GROUP8 \
    E_SEAL_USB_MIUPROT_WP_N, \
    E_SEAL_EMAC_MIUPROT_WP_N, \
    E_SEAL_PCIE_MIUPROT_WP_N, \
    E_SEAL_AU_HDMI_DMA_WP_N, \
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

#define SEAL_LOCK_GROUP9 \
    E_SEAL_EVD_LITE_0_WR_PROTN, \
    E_SEAL_EVD_LITE_1_WR_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_VIDEO_R2_AL_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_GPU_DRAM_MASK_PROTN, \
    E_SEAL_GPU_FBDEC_WP

#define SEAL_LOCK_GROUP10 \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_HDCP_RIU_R_PROTN, \
    E_SEAL_HDCP_XIU_R_PROTN, \
    E_SEAL_HDCP_RIU_W_PROTN, \
    E_SEAL_HDCP_XIU_W_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_GROUP20 \
    E_SEAL_UTMI_P0_PROTN_2, \
    E_SEAL_UTMI_P1_PROTN_2, \
    E_SEAL_UTMI_P2_PROTN_2, \
    E_SEAL_UTMI_P3_PROTN_2, \
    E_SEAL_UTMI_PSS0_PROTN_2, \
    E_SEAL_UTMI_PSS1_PROTN_2, \
    E_SEAL_UTMI_PSS2_PROTN_2, \
    E_SEAL_UPLL_PROTN_2, \
    E_SEAL_UPLL_1_PROTN_2, \
    E_SEAL_UTMI_P0_PROTN_3, \
    E_SEAL_UTMI_P1_PROTN_3, \
    E_SEAL_UTMI_P2_PROTN_3, \
    E_SEAL_UTMI_P3_PROTN_3, \
    E_SEAL_UTMI_PSS0_PROTN_3, \
    E_SEAL_UTMI_PSS1_PROTN_3, \
    E_SEAL_UTMI_PSS2_PROTN_3

#define SEAL_LOCK_GROUP21 \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_UTMI_P0_PROTN_1, \
    E_SEAL_UTMI_P1_PROTN_1, \
    E_SEAL_UTMI_P2_PROTN_1, \
    E_SEAL_UTMI_P3_PROTN_1, \
    E_SEAL_UTMI_PSS0_PROTN_1, \
    E_SEAL_UTMI_PSS1_PROTN_1, \
    E_SEAL_UTMI_PSS2_PROTN_1, \
    E_SEAL_UPLL_PROTN_1, \
    E_SEAL_UPLL_1_PROTN_1, \
    E_SEAL_LOCK_DUMMY

//[SEAL][HAL][000] Lock IP of TSP List [START]
#define SEAL_LOCK_TSP_GROUP0 \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_TSP_WP_TOP, \
    E_SEAL_TSP_RP_TOP, \
    E_SEAL_TSP_WP_CA_DEST, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_TSP_GROUP5 \
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
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_SECURE_R2_WR_PROTN_0, \
    E_SEAL_SECURE_R2_WR_PROTN_1, \
    E_SEAL_SECURE_R2_WR_PROTN_2, \
    E_SEAL_SECURE_R2_WR_PROTN_3

#define SEAL_LOCK_TSP_GROUP6 \
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
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_TSP_GROUP7 \
    E_SEAL_DSCRMB_RP_RDATA, \
    E_SEAL_CRI_KL_WP_N, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_RNG_PROTECT, \
    E_SEAL_RSA_PROTECT, \
    E_SEAL_SHA_PROTECT, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_TSP_GROUP8 \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_MFE_WR_PROTN, \
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

#define SEAL_LOCK_TSP_GROUP9 \
    E_SEAL_TSO_WP_TSOFI, \
    E_SEAL_TSO_RP_TSOFI, \
    E_SEAL_TSP_WP_MMFI0, \
    E_SEAL_TSP_RP_MMFI0, \
    E_SEAL_TSP_WP_MMFI1, \
    E_SEAL_TSP_RP_MMFI1, \
    E_SEAL_TSP_WP_PVR, \
    E_SEAL_TSP_RP_PVR, \
    E_SEAL_TSP_WP_PVR1, \
    E_SEAL_TSP_RP_PVR1, \
    E_SEAL_TSP_WP_FILEIN, \
    E_SEAL_TSP_RP_FILEIN, \
    E_SEAL_TSP_WP_QMEM, \
    E_SEAL_TSP_RP_QMEM, \
    E_SEAL_TSP_WP_FW, \
    E_SEAL_TSP_RP_FW

#define SEAL_LOCK_TSP_GROUP10 \
    E_SEAL_TSO1_WP_TSOFI, \
    E_SEAL_TSO1_RP_TSOFI, \
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
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_TSP_GROUP11 \
    E_SEAL_TSP_WP_PVR4, \
    E_SEAL_TSP_RP_PVR4, \
    E_SEAL_TSP_WP_PVR5, \
    E_SEAL_TSP_RP_PVR5, \
    E_SEAL_TSP_WP_PVR6, \
    E_SEAL_TSP_RP_PVR6, \
    E_SEAL_TSP_WP_PVR7, \
    E_SEAL_TSP_RP_PVR7, \
    E_SEAL_TSP_WP_PVR_MULTI, \
    E_SEAL_TSP_RP_PVR_MULTI, \
    E_SEAL_TSP_WP_PVR8, \
    E_SEAL_TSP_RP_PVR8, \
    E_SEAL_TSP_WP_PVR9, \
    E_SEAL_TSP_RP_PVR9, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_TSP_GROUP12 \
    E_SEAL_TSP_WP_FILEIN1, \
    E_SEAL_TSP_RP_FILEIN1, \
    E_SEAL_TSP_WP_PVR2, \
    E_SEAL_TSP_RP_PVR2, \
    E_SEAL_TSP_WP_FIQ, \
    E_SEAL_TSP_RP_FIQ, \
    E_SEAL_TSP_WP_PVR3, \
    E_SEAL_TSP_RP_PVR3, \
    E_SEAL_TSP_WP_FILEIN2, \
    E_SEAL_TSP_RP_FILEIN2, \
    E_SEAL_TSP_WP_FILEIN3, \
    E_SEAL_TSP_RP_FILEIN3, \
    E_SEAL_TSP_WP_FILEIN4, \
    E_SEAL_TSP_RP_FILEIN4, \
    E_SEAL_TSP_WP_FILEIN5, \
    E_SEAL_TSP_RP_FILEIN5

#define SEAL_LOCK_TSP_GROUP13 \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_TSP_WP_FIQ1, \
    E_SEAL_TSP_RP_FIQ1, \
    E_SEAL_TSP_WP_ORZ, \
    E_SEAL_TSP_RP_ORZ, \
    E_SEAL_TSP_WP_VQ, \
    E_SEAL_TSP_RP_VQ, \
    E_SEAL_TSP_WP_SEC, \
    E_SEAL_TSP_RP_SEC, \
    E_SEAL_TSP_WP_FIQ2, \
    E_SEAL_TSP_RP_FIQ2, \
    E_SEAL_TSP_WP_FIQ3, \
    E_SEAL_TSP_RP_FIQ3

#define SEAL_LOCK_TSP_GROUP14 \
    E_SEAL_TSP_WP_FILEIN6, \
    E_SEAL_TSP_RP_FILEIN6, \
    E_SEAL_TSIO_SGDMA_OUT_WP, \
    E_SEAL_TSIO_SGDMA_OUT_RP, \
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

#define SEAL_LOCK_TSP_GROUP16 \
    E_SEAL_TSP_WP_SEC_PRIVILEGE, \
    E_SEAL_TSP_RP_SEC_PRIVILEGE, \
    E_SEAL_CERT_KL_WP_N, \
    E_SEAL_PKA_PROTN, \
    E_SEAL_ALLOW_CLEAN_PKT_TO_RASP_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_BISS_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_HKDF_WP_N, \
    E_SEAL_HKDF_RP_N, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY
//[SEAL][HAL][000] Lock IP of TSP List [END]

//[SEAL][HAL][000] Lock IP of SC List [STRAT]
#define SEAL_LOCK_SC_GROUP0 \
    E_SEAL_SC_WP_DIPW, \
    E_SEAL_SC_RP_DIPW, \
    E_SEAL_SC_WP_DIPR, \
    E_SEAL_SC_RP_DIPR, \
    E_SEAL_SC_WP_IPMUX0, \
    E_SEAL_SC_RP_IPMUX0, \
    E_SEAL_SC_WP_IPMUX1, \
    E_SEAL_SC_RP_IPMUX1, \
    E_SEAL_SC_WP_IPMUX2, \
    E_SEAL_SC_RP_IPMUX2, \
    E_SEAL_SC_WP_IPMUX3, \
    E_SEAL_SC_RP_IPMUX3, \
    E_SEAL_SC_WP_IPMUX4, \
    E_SEAL_SC_RP_IPMUX4, \
    E_SEAL_SC_WP_IPMUX2_SC2, \
    E_SEAL_SC_RP_IPMUX2_SC2

#define SEAL_LOCK_SC_GROUP1 \
    E_SEAL_MDWIN_WP_N, \
    E_SEAL_MDWIN_RP_N, \
    E_SEAL_IPMUX_SECURE_TAB, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_MVOP_SUB_WP, \
    E_SEAL_SC_WP_RESOLUTION_ADJ_CTRL, \
    E_SEAL_SC_WP_SEL_DIU, \
    E_SEAL_SC2_WP_SCM_M, \
    E_SEAL_SC2_RP_SCM_M, \
    E_SEAL_MVOP_WP_TLB, \
    E_SEAL_MVOP_WP

#define SEAL_LOCK_SC_GROUP5 \
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
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_SC_GROUP6 \
    E_SEAL_SC_WP_HDR_DMA_M, \
    E_SEAL_SC_RP_HDR_DMA_M, \
    E_SEAL_SCR_OFF_SC0, \
    E_SEAL_SCR_OFF_SC1, \
    E_SEAL_SC_DS, \
    E_SEAL_SC_ADL, \
    E_SEAL_SC_ADC_DMA, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_MFDEC_PROTN, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_SC_AUTODL_WP_N, \
    E_SEAL_SC_AUTODL_RP_N, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY

#define SEAL_LOCK_SC_GROUP9 \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_GOPG_WP_N, \
    E_SEAL_DIP_PROTN, \
    E_SEAL_HDGEN_WP_N, \
    E_SEAL_HDGEN_RP_N, \
    E_SEAL_HDGEN_DCS_MV_WP_N, \
    E_SEAL_HDGEN_DCS_MV_RP_N, \
    E_SEAL_VE_DSC_MV_WP_N, \
    E_SEAL_VE_DSC_MV_RP_N, \
    E_SEAL_VMARK_PROTN, \
    E_SEAL_VTRACK_PROTN

#define SEAL_LOCK_SC_GROUP11 \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_SC_CROP_MAX_DIPW, \
    E_SEAL_SC_CROP_MAX_PDW0, \
    E_SEAL_SC_CROP_MAX_PDW1, \
    E_SEAL_LOCK_DUMMY, \
    E_SEAL_SC_MIU_PROTN_M, \
    E_SEAL_SC_MIU_PROTN_S, \
    E_SEAL_SC_ARB_PROTN, \
    E_SEAL_SC_T3D_PROTN, \
    E_SEAL_SC_LD_PROTN, \
    E_SEAL_SC_MCBUF_PROTN, \
    E_SEAL_SC_CMDQ_PROTN, \
    E_SEAL_LOCK_DUMMY
//[SEAL][HAL][000] Lock IP of SC List [END]

#define MIU0_PA_BASE               (0x00000000UL)
#define MIU1_PA_BASE               (0x40000000UL) // 1512MB
#define MIU2_PA_BASE               (0xC0000000UL)

const eSeal_IP NonPmIpTbl[SEAL_NONPM_TBL_IP_NUM] =
{
    SEAL_NON_PM_GROUP0, SEAL_NON_PM_GROUP1, SEAL_NON_PM_GROUP2,
    SEAL_NON_PM_GROUP3, SEAL_NON_PM_GROUP4, SEAL_NON_PM_GROUP5,
    SEAL_NON_PM_GROUP6
};

const int LockTbl[SEAL_TBL_LOCK_NUM] =
{
    SEAL_LOCK_GROUP10, /*0x64*/
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP20,     SEAL_LOCK_GROUP21,     SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP0, /*0x74*/
    SEAL_LOCK_GROUP1, SEAL_LOCK_GROUP2, SEAL_LOCK_GROUP3,
    SEAL_LOCK_GROUP4, SEAL_LOCK_GROUP5, SEAL_LOCK_GROUP6,
    SEAL_LOCK_GROUP7, SEAL_LOCK_GROUP8, SEAL_LOCK_GROUP9
};

const eSeal_Lock TSPLockTbl[SEAL_TBL_LOCK_TSP_NUM] =
{
    SEAL_LOCK_TSP_GROUP0, /*0x6f*/
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_TSP_GROUP5, /*0x74*/
    SEAL_LOCK_TSP_GROUP6,
    SEAL_LOCK_TSP_GROUP7,
    SEAL_LOCK_TSP_GROUP8,
    SEAL_LOCK_TSP_GROUP9,
    SEAL_LOCK_TSP_GROUP10,
    SEAL_LOCK_TSP_GROUP11,
    SEAL_LOCK_TSP_GROUP12,
    SEAL_LOCK_TSP_GROUP13,
    SEAL_LOCK_TSP_GROUP14, /*0x7d*/
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_TSP_GROUP16 /*0x7f*/
};

const eSeal_Lock SCLockTbl[SEAL_TBL_LOCK_SC_NUM] =
{
    SEAL_LOCK_SC_GROUP0, /*0x74*/
    SEAL_LOCK_SC_GROUP1,
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_SC_GROUP5, /*0x79*/
    SEAL_LOCK_SC_GROUP6,
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_SC_GROUP9, /*0x7d*/
    SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_SC_GROUP11 /*0x7f*/
};

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

static int HAL_SEAL_GetLockIdx(int eLockId, int* RegAddr)
{
    int s32HwIdx;

    for (s32HwIdx = 0; s32HwIdx < SEAL_TBL_LOCK_NUM; s32HwIdx++)
    {
        if(eLockId == LockTbl[s32HwIdx])
        {
            *RegAddr = (unsigned int)(REG_TZPC_NONPM_SECURE_REG);
            return s32HwIdx;
        }
    }
    for (s32HwIdx = 0; s32HwIdx < SEAL_TBL_LOCK_TSP_NUM; s32HwIdx++)
    {
        if(eLockId == TSPLockTbl[s32HwIdx])
        {
            *RegAddr = (unsigned int)(REG_TZPC_BUFFER_LOCK_TSP);
            return s32HwIdx;
        }
    }
    for (s32HwIdx = 0; s32HwIdx < SEAL_TBL_LOCK_SC_NUM; s32HwIdx++)
    {
        if(eLockId == SCLockTbl[s32HwIdx])
        {
            *RegAddr = (unsigned int)(REG_TZPC_BUFFER_LOCK_SC);
            return s32HwIdx;
        }
    }

    return (-1);
}

static unsigned long long DefaultNonSecClient[2] =
{
    0x0000000000000301ULL, 0x0000000000000000ULL
};

static unsigned long long DefaultSecClient[2] =
{
    0x0000FFFFFFFF0300ULL, 0x0000000000000000ULL
};


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

    if (startAddr >= endAddr)
    {
        return 0;
    }

    _phy_to_miu_offset(miuSel, startOffset, startAddr);
    _phy_to_miu_offset(miuSelEnd, endOffset, endAddr);

    if (miuSel != miuSelEnd)
    {
        return 0;
    }

    // K6 addr is 4KB shifted
    startOffset = startOffset >> 12;
    endOffset = endOffset >> 12;

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
        offset = i*REG_SECURE_RANGE_OFFSET;
        startRegOffset = REG_SECURE_RANGE0_START_ADDR + offset;
        endRegOffset = REG_SECURE_RANGE0_END_ADDR + offset;
        unsigned long long start = 0;
        unsigned long long end = 0;
        start |= (RIU[bankBase + startRegOffset + 2] & 0x000F);
        start <<= 16;
        start |= RIU[bankBase + startRegOffset];
        end |= (RIU[bankBase + endRegOffset + 2] & 0x000F);
        end <<= 16;
        end |= RIU[bankBase + endRegOffset];

        // same range, only set attribute
        if ( ((RIU[bankBase + REG_SECURE0_DETECT_ENABLE]) & (1<<i))
            && (start == startOffset)
            && (end == endOffset) )
        {
            if ( attr == 0xF )
            {
                RIU[bankBase + REG_SECURE0_DETECT_ENABLE] &= (~(1<<i));
                RIU[bankBase + startRegOffset] = 0x0;
                RIU[bankBase + startRegOffset + 2] = 0x0;
                RIU[bankBase + endRegOffset] = 0x0;
                RIU[bankBase + endRegOffset + 2] = 0x8000;
            }
            return 1;
        }
    }

    // find an unused range
    for (free=0; free<REG_SECURE_RANGE_NUM; free++)
    {
        if ( ! ((RIU[bankBase + REG_SECURE0_DETECT_ENABLE]) & (1<<free)) )
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
    RIU[bankBase + startRegOffset + 2] = ((startOffset>>16) & 0x000FULL);

    RIU[bankBase + endRegOffset] = (endOffset & 0xFFFFULL);
    RIU[bankBase + endRegOffset + 2] = ((endOffset>>16) & 0x000FULL);

    (RIU[bankBase + attrRegOffset]) |= FLAG_SECURE_RANGE_ADDR_DYN_CH;
    (RIU[bankBase + REG_SECURE0_DETECT_ENABLE]) |= (1<<free);

    return 1;
}

int HAL_SEAL_DisableRIUBridges(void)
{
    // set hosts of PM RIU bridge to non-secure
    RIU[SEAL_TZPC_PM_BASE + REG_PM_RIU_BRIDGE] = (0x5555);

    // set hosts of NONPM RIU bridge to non-secure (except ARM)
    RIU[SEAL_TZPC_NONPM_BASE + REG_NONPM_RIU_BRIDGE] = (0x5555);

    // Set ARM to non-secure
    (RIU[REG_TZPC_LEGACY_SECUREPROCESSOR]) |= (FLAG_TZPC_NONSECURE_ARM);

    return 1;
}

int HAL_SEAL_Init(void)
{
    int i, offset;

    //aviod compiler doing optimization
    flag_dummyread = 0x0;

    // mask decode error when non-secure CPU access miu secure range
    // mask decode error when non-secure CPU access secure riu register bank
    // mask decode error when CPU access undefined address area
    (RIU[SEAL_TZPC_NONPM_BASE + (0x7e<<1)]) |= (0x000e);

    // BDMA RIU in TEE
    HAL_SEAL_SetRIUBank(E_SEAL_BDMA_CH0_PROT_NONPM, 1);

    // Set up the configuration of miu ns table
    // Only Seucre ARM can modify it - 9'd5
    (RIU[REG_TZPC_MIU0_CTL]) = REG_TZPC_MIU_TOP_EN;
    RIU[REG_TZPC_MIU0_ID0] = 0x1005; // 5 | bit12
    while (RIU[REG_TZPC_MIU0_ID0] != 0x1005);

    // Init the access id permission table
    for (i=0, offset=0; i<SEAL_NONPM_TBL_IP_NUM; ++i, offset+=0x20)
    {
        if (i == 3 //tzpc_aesdma_protected_miu_ns
            || i == 2 //tzpc_aesdma_miu_ns
            || i == 27 //tzpc_fcie_miu_ns
            || i == 7 //tzpc_arm_I_2miu_secure_ns
            || i == 5 //tzpc_arm_D_2miu_secure_ns
#if !defined(CONFIG_SEAL_SELF_TEST)
            || i == 13 //bdma
#endif
            )
        {
            *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset) = 0x0000FFFFFFFF0300ULL;
            *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset + 8) = 0x0000000000000000ULL;
        }
        else
        {
            *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset) = 0x0000000000000301ULL;
            *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset + 8) = 0x0000000000000000ULL;

        }
    }
    //M7221 cannot read permission table, therefore we use delay to make sure that data is written.
    while(*(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR) < flag_dummyread);

    RIU[REG_TZPC_MIU0_ID0] = 0x0005;
	while (RIU[REG_TZPC_MIU0_ID0] != 0x0005);

#ifndef CONFIG_MIU1_DRAM_NONE
    (RIU[REG_TZPC_MIU1_CTL]) = REG_TZPC_MIU_TOP_EN;
    RIU[REG_TZPC_MIU1_ID0] = 0x1005;
    while (RIU[REG_TZPC_MIU1_ID0] != 0x1005);

    for (i=0, offset=0; i<SEAL_NONPM_TBL_IP_NUM; ++i, offset+=0x20)
    {
        if (i == 3 //tzpc_aesdma_protected_miu_ns
            || i == 2 //tzpc_aesdma_miu_ns
            || i == 27 //tzpc_fcie_miu_ns
            || i == 7 //tzpc_arm_I_2miu_secure_ns
            || i == 5 //tzpc_arm_D_2miu_secure_ns
#if !defined(CONFIG_SEAL_SELF_TEST)
            || i == 13
#endif
            ) //bdma
        {
            *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset) = 0x0000FFFFFFFF0300ULL;
            *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset + 8) = 0x0000000000000000ULL;
        }
        else
        {
            *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset) = 0x0000000000000301ULL;
            *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset + 8) = 0x0000000000000000ULL;
        }
    }
    //M7221 cannot read permission table, therefore we use delay to make sure that data is written.
    while(*(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR) < flag_dummyread);

    RIU[REG_TZPC_MIU1_ID0] = 0x0005;
    while (RIU[REG_TZPC_MIU1_ID0] != 0x0005);
#endif

    // Set hosts of RIU bridge to non-secure (except ARM and DB_BUS)
    RIU[SEAL_TZPC_PM_BASE + REG_PM_RIU_BRIDGE] = (0x5554);
    RIU[SEAL_TZPC_NONPM_BASE + REG_NONPM_RIU_BRIDGE] = (0x5554);

    return 1;
}

int HAL_SEAL_Cleanup(void)
{
    int i, offset;

    //aviod compiler doing optimization
    flag_dummyread = 0x0;

    RIU[REG_TZPC_MIU0_ID0] = 0x1005; // 5 | bit12
    while (RIU[REG_TZPC_MIU0_ID0] != 0x1005);


    // Init the access id permission table
    for (i=0, offset=0; i<SEAL_NONPM_TBL_IP_NUM; ++i, offset+=0x20)
    {
        if (i == 27 ////tzpc_fcie_miu_ns
            || i == 13) //bdma
        {
            *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset) = 0x0000000000000301ULL;
            *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset + 8) = 0x0000000000000000ULL;

        }
    }
    //M7221 cannot read permission table, therefore we use delay to make sure that data is written.
    while(*(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR) < flag_dummyread);

    RIU[REG_TZPC_MIU0_ID0] = 0x0005;
    while (RIU[REG_TZPC_MIU0_ID0] != 0x0005);

#ifndef CONFIG_MIU1_DRAM_NONE
    RIU[REG_TZPC_MIU1_ID0] = 0x1005;
    while (RIU[REG_TZPC_MIU1_ID0] != 0x1005);

    for (i=0, offset=0; i<SEAL_NONPM_TBL_IP_NUM; ++i, offset+=0x20)
    {
        if (i == 27 ////tzpc_fcie_miu_ns
            || i == 13) //bdma
        {
            *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset) = 0x0000000000000301ULL;
            *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset + 8) = 0x0000000000000000ULL;
        }
    }

    //M7221 cannot read permission table, therefore we use delay to make sure that data is written.
    while(*(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR) < flag_dummyread);

    RIU[REG_TZPC_MIU1_ID0] = 0x0005;
    while (RIU[REG_TZPC_MIU1_ID0] != 0x0005);
#endif

    // release BDMA RIU in TEE
    HAL_SEAL_SetRIUBank(E_SEAL_BDMA_CH0_PROT_NONPM, 0);

    //reg_rom_security_oneway_prot
	  (RIU[0x101800+(0x59<<1)]) |= 1;

    return 1;
}

int HAL_SEAL_SetMIUHost(int swIdx, int bSecure)
{
    int hwIdx = HAL_SEAL_GetNonPmIpIdx(swIdx);
    unsigned int offset;
    unsigned long long* val = (bSecure ? DefaultSecClient : DefaultNonSecClient);

    if (hwIdx < 0)
    {
        return 0;
    }

    offset = 0x20*hwIdx;

    (RIU[REG_TZPC_MIU0_ID0]) |= REG_TZPC_MIU_ID_ENABLE;
    __delay();
    *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset) = val[0];
    *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset + 8) = val[1];
    __delay();
    (RIU[REG_TZPC_MIU0_ID0]) &= (~REG_TZPC_MIU_ID_ENABLE);
    __delay();

#ifndef CONFIG_MIU1_DRAM_NONE
    (RIU[REG_TZPC_MIU1_ID0]) |= REG_TZPC_MIU_ID_ENABLE;
    __delay();
    *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset) = val[0];
    *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset + 8) = val[1];
    __delay();
    (RIU[REG_TZPC_MIU1_ID0]) &= (~REG_TZPC_MIU_ID_ENABLE);
    __delay();
#endif
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


int HAL_SEAL_SetRIURegister(int swIdx, int bSecure)
{
    int RegBase;
    int hwIdx = HAL_SEAL_GetLockIdx(swIdx, &RegBase);
    if (hwIdx < 0)
    {
        return 0;
    }

    if (bSecure)
        (RIU[RegBase + ((hwIdx>>4)<<1)]) &= ~(1<<(hwIdx&15));
    else
        (RIU[RegBase + ((hwIdx>>4)<<1)]) |= (1<<(hwIdx&15));
    return 1;
}
