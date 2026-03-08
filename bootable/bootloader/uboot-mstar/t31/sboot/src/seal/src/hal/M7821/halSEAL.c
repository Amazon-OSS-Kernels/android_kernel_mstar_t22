#include "halSEAL.h"
#include "../../drvSEAL.h"

static int _gSealLocalDelayVar;

/* I tried to reduce the loop count, but then SRAM won't be set correctly, so be cautious */
#define __delay() {int j; for (j=0; j<1000000; j+=2) {j=j-1; _gSealLocalDelayVar+=j;}}

//Non PM IP group
#define SEAL_NON_PM_GROUP0 \
    E_SEAL_RIU_DBG_PROT_NONPM, \
    E_SEAL_CLKGEN_AUR2_0_PROT_NONPM, \
    E_SEAL_CLKGEN_AUR2_1_PROT_NONPM, \
    E_SEAL_POR_STATUS_PROT_NONPM, \
    E_SEAL_INTR_CPUINT_PROT_NONPM, \
    E_SEAL_MAILBOX_PQ_PROT_NONPM, \
    E_SEAL_MIU2_PROT_NONPM, \
    E_SEAL_USB0_PROT_NONPM, \
    E_SEAL_USB1_PROT_NONPM, \
    E_SEAL_UPLL0_PROT_NONPM, \
    E_SEAL_UPLL1_PROT_NONPM, \
    E_SEAL_BDMA_CH0_PROT_NONPM, \
    E_SEAL_BDMA_CH1_PROT_NONPM, \
    E_SEAL_UART3_PROT_NONPM, \
    E_SEAL_UART0_PROT_NONPM, \
    E_SEAL_CLKGEN2_PROT_NONPM

#define SEAL_NON_PM_GROUP1 \
    E_SEAL_CLKGEN0_PROT_NONPM, \
    E_SEAL_DSCRMB_PROT_NONPM, \
    E_SEAL_UHC1_PROT_NONPM, \
    E_SEAL_ACP_PROT_NONPM, \
    E_SEAL_SPI_PRE_ARB_PROT_NONPM, \
    E_SEAL_INTR_CTRL1_PROT_NONPM, \
    E_SEAL_MVD_PROT_NONPM, \
    E_SEAL_MIU_PROT_NONPM, \
    E_SEAL_MVOPSUB_PROT_NONPM, \
    E_SEAL_MVOP_PROT_NONPM, \
    E_SEAL_TSP0_PROT_NONPM, \
    E_SEAL_TSP1_PROT_NONPM, \
    E_SEAL_TSP_SPARE_PROT_NONPM, \
    E_SEAL_SEMAPH_PROT_NONPM, \
    E_SEAL_L3_AXI_PROT_NONPM, \
    E_SEAL_INTR_CTRL_PROT_NONPM

#define SEAL_NON_PM_GROUP2 \
    E_SEAL_BYTE2WORD_PROT_PM, \
    E_SEAL_HVD_PROT_NONPM, \
    E_SEAL_TSP2_PROT_NONPM, \
    E_SEAL_MCU_ARM_PROT_NONPM, \
    E_SEAL_CHIP_PROT_NONPM, \
    E_SEAL_GOP_PROT_NONPM, \
    E_SEAL_EMAC0_PROT_NONPM, \
    E_SEAL_EMAC1_PROT_NONPM, \
    E_SEAL_EMAC2_PROT_NONPM, \
    E_SEAL_EMAC3_PROT_NONPM, \
    E_SEAL_UHC0_PROT_NONPM, \
    E_SEAL_ADC_ATOP_PROT_NONPM, \
    E_SEAL_ADC_DTOP_PROT_NONPM, \
    E_SEAL_CODEC_SPARE_PROT_NONPM, \
    E_SEAL_GE0_PROT_NONPM, \
    E_SEAL_SMART_PROT_NONPM

#define SEAL_NON_PM_GROUP3 \
    E_SEAL_MHEG5_SPARE_PROT_NONPM, \
    E_SEAL_CHIPGPIO_PROT_NONPM, \
    E_SEAL_LDM_DMA0_PROT_NONPM, \
    E_SEAL_LDM_DMA1_PROT_NONPM, \
    E_SEAL_SC0_PROT_NONPM, \
    E_SEAL_SC1_PROT_NONPM, \
    E_SEAL_SC2_PROT_NONPM, \
    E_SEAL_SC3_PROT_NONPM, \
    E_SEAL_SC4_PROT_NONPM, \
    E_SEAL_CLKGEN1_PROT_NONPM, \
    E_SEAL_MAILBOX_PROT_NONPM, \
    E_SEAL_PCM_PROT_NONPM, \
    E_SEAL_VDMCU51_IF_PROT_NONPM, \
    E_SEAL_DMDMCU51_IF_PROT_NONPM, \
    E_SEAL_URDMA_PROT_NONPM, \
    E_SEAL_AFEC_PROT_NONPM

#define SEAL_NON_PM_GROUP4 \
    E_SEAL_COMB_PROT_NONPM, \
    E_SEAL_VBI_PROT_NONPM, \
    E_SEAL_SCM_PROT_NONPM, \
    E_SEAL_UTMI2_PROT_NONPM, \
    E_SEAL_ARM_I_SECURE_PROT_NONPM, \
    E_SEAL_UTMI1_PROT_NONPM, \
    E_SEAL_UTMI_PROT_NONPM, \
    E_SEAL_VE_0_PROT_NONPM, \
    E_SEAL_REG_PIU_NONPM_PROT_NONPM, \
    E_SEAL_ADC_ATOPB_PROT_NONPM, \
    E_SEAL_VE_1_PROT_NONPM, \
    E_SEAL_VE_2_PROT_NONPM, \
    E_SEAL_SC_GP1_NONPM, \
    E_SEAL_CHIPGPIO1_NONPM, \
    E_SEAL_UART1_PROT_NONPM, \
    E_SEAL_UART2_PROT_NONPM

#define SEAL_NON_PM_GROUP5 \
    E_SEAL_FUART_PROT_NONPM, \
    E_SEAL_UART4_PROT_NONPM, \
    E_SEAL_GE1_PROT_NONPM, \
    E_SEAL_GPU_NONPM, \
    E_SEAL_TS_SAMPLE_NONPM, \
    E_SEAL_AU_R2_PROT_NONPM, \
    E_SEAL_SCPLL_PROT_NONPM, \
    E_SEAL_MPLL_PROT_NONPM, \
    E_SEAL_MIPSPLL_PROT_NONPM, \
    E_SEAL_LPF_GRADE_PROT_NONPM, \
    E_SEAL_TSP_AEON_D_2MIU_NS_PROT_NONPM, \
    E_SEAL_ARM_I_NON_SECURE_PROT_NONPM, \
    E_SEAL_LPF_PROT_NONPM, \
    E_SEAL_DDR_SCAL_PROT_NONPM, \
    E_SEAL_ANA_MISC_GMAC_PROT_NONPM, \
    E_SEAL_MIU_ATOP_PROT_NONPM

#define SEAL_NON_PM_GROUP6 \
    E_SEAL_MFDEC0_PROT_NONPM, \
    E_SEAL_MFDEC1_PROT_NONPM, \
    E_SEAL_MFE0_PROT_NONPM, \
    E_SEAL_MFE1_PROT_NONPM, \
    E_SEAL_ADC_DTOPB_PROT_NONPM, \
    E_SEAL_NFIE0_PROT_NONPM, \
    E_SEAL_NFIE1_PROT_NONPM, \
    E_SEAL_NFIE2_PROT_NONPM, \
    E_SEAL_NFIE4_PROT_NONPM, \
    E_SEAL_DMD_MCU2_PROT_NONPM, \
    E_SEAL_MIIC0_PROT_NONPM, \
    E_SEAL_MIIC1_PROT_NONPM, \
    E_SEAL_MIIC2_PROT_NONPM, \
    E_SEAL_MIIC3_PROT_NONPM, \
    E_SEAL_MOD2_PROT_NONPM, \
    E_SEAL_CLKGEN_DMD_PROT_NONPM

#define SEAL_NON_PM_GROUP7 \
    E_SEAL_DEMOD_0_PROT_NONPM, \
    E_SEAL_DEMOD_1_PROT_NONPM, \
    E_SEAL_DEMOD_2_PROT_NONPM, \
    E_SEAL_DEMOD_3_PROT_NONPM, \
    E_SEAL_DEMOD_4_PROT_NONPM, \
    E_SEAL_DEMOD_5_PROT_NONPM, \
    E_SEAL_DEMOD_6_PROT_NONPM, \
    E_SEAL_DEMOD_7_PROT_NONPM, \
    E_SEAL_AUR20_PROT_NONPM, \
    E_SEAL_DMD_ANA_MISC_PROT_NONPM, \
    E_SEAL_VIVALDI0_PROT_NONPM, \
    E_SEAL_VIVALDI1_PROT_NONPM, \
    E_SEAL_VIVALDI2_PROT_NONPM, \
    E_SEAL_VIVALDI3_PROT_NONPM, \
    E_SEAL_VIVALDI4_PROT_NONPM, \
    E_SEAL_VIVALDI5_PROT_NONPM

#define SEAL_NON_PM_GROUP8 \
    E_SEAL_AU_MAU_NONPM, \
    E_SEAL_VIVALDI00_PROT_NONPM, \
    E_SEAL_CLKGEN_SC_FE, \
    E_SEAL_CLKGEN_SC_BE, \
    E_SEAL_CLKGEN_SC_GP2, \
    E_SEAL_SC_GPLUS, \
    E_SEAL_SC_GPLUS_TOP_PROT_NONPM, \
    E_SEAL_USB2_PROT_NONPM, \
    E_SEAL_USB3_PROT_NONPM, \
    E_SEAL_UHC2_PROT_NONPM, \
    E_SEAL_TSO_PROT_NONPM, \
    E_SEAL_DRM_SECURE_PROT_NONPM, \
    E_SEAL_DSCRMB2_PROT_NONPM, \
    E_SEAL_DSCRMB3_PROT_NONPM, \
    E_SEAL_GPD0_PROT_NONPM, \
    E_SEAL_GPD1_PROT_NONPM

#define SEAL_NON_PM_GROUP9 \
    E_SEAL_GOP0G_0_PROT_NONPM, \
    E_SEAL_GOP0G_1_PROT_NONPM, \
    E_SEAL_GOP0G_ST_PROT_NONPM, \
    E_SEAL_GOP1G_0_PROT_NONPM, \
    E_SEAL_GOP1G_1_PROT_NONPM, \
    E_SEAL_GOP1G_ST_PROT_NONPM, \
    E_SEAL_GOP2G_0_PROT_NONPM, \
    E_SEAL_GOP2G_1_PROT_NONPM, \
    E_SEAL_GOP2G_ST_PROT_NONPM, \
    E_SEAL_GOP3G_0_PROT_NONPM, \
    E_SEAL_GOP3G_1_PROT_NONPM, \
    E_SEAL_GOP3G_ST_PROT_NONPM, \
    E_SEAL_SDIO0_PROT_NONPM, \
    E_SEAL_SDIO_PLL_PROT_NONPM, \
    E_SEAL_INTR_CPUINT_SECR2_PROT_NONPM, \
    E_SEAL_INTR_CPUINT_DIAMOND_PROT_NONPM

#define SEAL_NON_PM_GROUP10 \
    E_SEAL_INTR_CPUINT_FOSSIL_PROT_NONPM, \
    E_SEAL_TZPC_NONPM2_NONPM, \
    E_SEAL_GOP4G_0_PROT_NONPM, \
    E_SEAL_MIIC4_PROT_NONPM, \
    E_SEAL_MIIC5_PROT_NONPM, \
    E_SEAL_GOP4G_1_PROT_NONPM, \
    E_SEAL_GOP4G_ST_PROT_NONPM, \
    E_SEAL_UTMI3_PROT_NONPM, \
    E_SEAL_UHC3_PROT_NONPM, \
    E_SEAL_SDIO1_PROT_NONPM, \
    E_SEAL_SDIO2_PROT_NONPM, \
    E_SEAL_SEC_R2_PROT_NONPM, \
    E_SEAL_SEC_MAU0_PROT_NONPM, \
    E_SEAL_DSCRMB4_PROT_NONPM, \
    E_SEAL_IMI_TOP_PROT_NONPM, \
    E_SEAL_MOBF_PROT_NONPM

#define SEAL_NON_PM_GROUP11 \
    E_SEAL_DC_SCL_PROT_NONPM, \
    E_SEAL_JPD_PROT_NONPM, \
    E_SEAL_MFDEC_PROT_NONPM, \
    E_SEAL_CMDQ_PROT_NONPM, \
    E_SEAL_USBBC0_PROT_NONPM, \
    E_SEAL_USBBC1_PROT_NONPM, \
    E_SEAL_USBBC2_PROT_NONPM, \
    E_SEAL_USBBC3_PROT_NONPM, \
    E_SEAL_PM51_I_PROT_NONPM, \
    E_SEAL_PM51_D_PROT_NONPM, \
    E_SEAL_SECU_R2_I_PROT_NONPM, \
    E_SEAL_SECU_R2_D_PROT_NONPM, \
    E_SEAL_NFIE3_PROT_NONPM, \
    E_SEAL_DSCRMB5_PROT_NONPM, \
    E_SEAL_TSO1_NONPM, \
    E_SEAL_ZDEC_PROT_NONPM

#define SEAL_NON_PM_GROUP12 \
    E_SEAL_EMMC_PLL_PROT_NONPM, \
    E_SEAL_MSC_PROT_NONPM, \
    E_SEAL_ARM_D_NON_SECURE_PROT_NONPM, \
    E_SEAL_ARM_D_SECURE_PROT_NONPM, \
    E_SEAL_DSP_I_PROT_NONPM, \
    E_SEAL_PATGEN_GPU_PROT_NONPM, \
    E_SEAL_PATGEN_DEMOD_PROT_NONPM, \
    E_SEAL_DDI_0_PROT_NONPM, \
    E_SEAL_PATGEN_DEMOD1_PROT_NONPM, \
    E_SEAL_DDI_1_PROT_NONPM, \
    E_SEAL_DUMMY, \
    E_SEAL_PATGEN_EVD_PROT_NONPM, \
    E_SEAL_DDI_2_PROT_NONPM, \
    E_SEAL_GOP5G_0_PROT_NONPM, \
    E_SEAL_BAT_PROT_NONPM, \
    E_SEAL_PATGEN_GPU_DIE_PROT_NONPM

#define SEAL_NON_PM_GROUP13 \
    E_SEAL_PATGEN_AUR2_0_PROT_NONPM, \
    E_SEAL_MVOP_PROTECTED_PROT_NONPM, \
    E_SEAL_MIU_BWFLAG_PROT_NONPM, \
    E_SEAL_MIU_BWFLAG2_PROT_NONPM, \
    E_SEAL_MIU_ATOP_E_PROT_NONPM, \
    E_SEAL_MIU_ATOP_E2_PROT_NONPM, \
    E_SEAL_TSP7_PROT_NONPM, \
    E_SEAL_PAD_MUX_NONPM, \
    E_SEAL_ROM_CRC_NONPM, \
    E_SEAL_MIU_ARBB_NONPM, \
    E_SEAL_MIU_ARBB2_NONPM, \
    E_SEAL_COMB1_PROT_NONPM, \
    E_SEAL_COMB2_PROT_NONPM, \
    E_SEAL_COMB3_PROT_NONPM, \
    E_SEAL_COMB4_PROT_NONPM, \
    E_SEAL_COMB5_PROT_NONPM

#define SEAL_NON_PM_GROUP14 \
    E_SEAL_DEMOD_8_PROT_NONPM, \
    E_SEAL_DEMOD_9_PROT_NONPM, \
    E_SEAL_MIU_DIG_E_PROT_NONPM, \
    E_SEAL_MIU_DIG_E2_PROT_NONPM, \
    E_SEAL_MIU_DIG_E4_PROT_NONPM, \
    E_SEAL_DMD_TOP_1_PROT_NONPM, \
    E_SEAL_USB0_MIUPROT_PROT_NONPM, \
    E_SEAL_USB1_MIUPROT_PROT_NONPM, \
    E_SEAL_USB2_MIUPROT_PROT_NONPM, \
    E_SEAL_USB3_MIUPROT_PROT_NONPM, \
    E_SEAL_TSO2_PROT_NONPM, \
    E_SEAL_MSPI0_PROT_NONPM, \
    E_SEAL_MSPI1_PROT_NONPM, \
    E_SEAL_MSPI_MCARD_PROT_NONPM, \
    E_SEAL_VIVALDId_PROT_NONPM, \
    E_SEAL_VIVALDIe_PROT_NONPM

#define SEAL_NON_PM_GROUP15 \
    E_SEAL_VIVALDIf_PROT_NONPM, \
    E_SEAL_VIVALDI6_PROT_NONPM, \
    E_SEAL_VIVALDI7_PROT_NONPM, \
    E_SEAL_GMAC0_PROT_NONPM, \
    E_SEAL_GMAC1_PROT_NONPM, \
    E_SEAL_GMAC2_PROT_NONPM, \
    E_SEAL_GMAC3_PROT_NONPM, \
    E_SEAL_GMAC4_PROT_NONPM, \
    E_SEAL_PCM2_PROT_NONPM, \
    E_SEAL_TSP3_PROT_NONPM, \
    E_SEAL_EVD_NONPM, \
    E_SEAL_EMACMIUPROT_PROT_NONPM, \
    E_SEAL_GPU2MIU_MASK_PROT_NONPM, \
    E_SEAL_GPU_PLL_PROT_NONPM, \
    E_SEAL_MIU_ARB_SC_NONPM, \
    E_SEAL_MIU_ARB2_SC_NONPM

#define SEAL_NON_PM_GROUP16 \
    E_SEAL_MIU_ARB3_SC_NONPM, \
    E_SEAL_CODEC_MRQ_PROT_NONPM, \
    E_SEAL_MIU_ARB_NONPM, \
    E_SEAL_MIU_ATOP2_PROT_NONPM, \
    E_SEAL_TSP4_NONPM, \
    E_SEAL_DMD51_I_PROT_NONPM, \
    E_SEAL_TZPC_SC_PROT_NONPM, \
    E_SEAL_VD_R2_I_PROT_NONPM, \
    E_SEAL_TSP6_PROT_NONPM, \
    E_SEAL_MIU_ARB2_NONPM, \
    E_SEAL_MIU4_NONPM, \
    E_SEAL_MIU_ATOP4_NONPM, \
    E_SEAL_MIU_ARB4_NONPM, \
    E_SEAL_VIVALDIc_PROT_NONPM, \
    E_SEAL_GE2_NONPM, \
    E_SEAL_GE3_NONPM

#define SEAL_NON_PM_GROUP17 \
    E_SEAL_MIU_ARB_FRC_NONPM, \
    E_SEAL_MIU_ARB2_FRC_NONPM, \
    E_SEAL_MIU_ARB3_FRC_NONPM, \
    E_SEAL_AU_R2_1_NONPM, \
    E_SEAL_AU_MAU_1_NONPM, \
    E_SEAL_VD_EVD_R2_NONPM, \
    E_SEAL_MAU_EVD_NONPM, \
    E_SEAL_HIREG_EVD_PROT_NONPM, \
    E_SEAL_VD_R2_SUBSYS_PROT_NONPM, \
    E_SEAL_MAU_EVD_LV2_0_NONPM, \
    E_SEAL_MAU_EVD_LV2_1_NONPM, \
    E_SEAL_SEC_MAU_LV2_0_NONPM, \
    E_SEAL_SEC_MAU_LV2_1_NONPM, \
    E_SEAL_TSP5_NONPM, \
    E_SEAL_VIVALDI8_PROT_NONPM, \
    E_SEAL_VIVALDI9_PROT_NONPM

#define SEAL_NON_PM_GROUP18 \
    E_SEAL_VIVALDIa_0_PROT_NONPM, \
    E_SEAL_VIVALDIa_1_PROT_NONPM, \
    E_SEAL_VIVALDIa_2_PROT_NONPM, \
    E_SEAL_VIVALDIa_3_PROT_NONPM, \
    E_SEAL_VIVALDIb_0_PROT_NONPM, \
    E_SEAL_VIVALDIb_1_PROT_NONPM, \
    E_SEAL_VIVALDIb_2_PROT_NONPM, \
    E_SEAL_VIVALDIb_3_PROT_NONPM, \
    E_SEAL_COMBO_PHY0_P0_PROT_NONPM, \
    E_SEAL_COMBO_PHY1_P0_PROT_NONPM, \
    E_SEAL_COMBO_PHY0_P1_PROT_NONPM, \
    E_SEAL_COMBO_PHY1_P1_PROT_NONPM, \
    E_SEAL_COMBO_PHY0_P2_PROT_NONPM, \
    E_SEAL_COMBO_PHY1_P2_PROT_NONPM, \
    E_SEAL_COMBO_PHY0_P3_PROT_NONPM, \
    E_SEAL_COMBO_PHY1_P3_PROT_NONPM

#define SEAL_NON_PM_GROUP19 \
    E_SEAL_DVI_DTOP_DUAL_P0_PROT_NONPM, \
    E_SEAL_DVI_RSV_DUAL_P0_PROT_NONPM, \
    E_SEAL_HDCP_DUAL_P0_PROT_NONPM, \
    E_SEAL_DVI_DTOP_DUAL_P1_PROT_NONPM, \
    E_SEAL_DVI_RSV_DUAL_P1_PROT_NONPM, \
    E_SEAL_HDCP_DUAL_P1_PROT_NONPM, \
    E_SEAL_DVI_DTOP_DUAL_P2_PROT_NONPM, \
    E_SEAL_DVI_RSV_DUAL_P2_PROT_NONPM, \
    E_SEAL_HDCP_DUAL_P2_PROT_NONPM, \
    E_SEAL_DVI_DTOP_DUAL_P3_PROT_NONPM, \
    E_SEAL_DVI_RSV_DUAL_P3_PROT_NONPM, \
    E_SEAL_HDCP_DUAL_P3_PROT_NONPM, \
    E_SEAL_COMBO_PHY2_P0_PROT_NONPM, \
    E_SEAL_COMBO_PHY2_P1_PROT_NONPM, \
    E_SEAL_COMBO_PHY2_P2_PROT_NONPM, \
    E_SEAL_COMBO_PHY2_P3_PROT_NONPM

#define SEAL_NON_PM_GROUP20 \
    E_SEAL_HDMI_DUAL_0_PROT_NONPM, \
    E_SEAL_HDMI2_DUAL_0_PROT_NONPM, \
    E_SEAL_HDMI_DUAL_1_PROT_NONPM, \
    E_SEAL_HDMI2_DUAL_1_PROT_NONPM, \
    E_SEAL_HDMI3_DUAL_0_PROT_NONPM, \
    E_SEAL_HDMI3_DUAL_1_PROT_NONPM, \
    E_SEAL_HDCPKEY_PROT_NONPM, \
    E_SEAL_COMBO_GP_TOP_PROT_NONPM, \
    E_SEAL_SECURE_TZPC_PROT_NONPM, \
    E_SEAL_DSCRMB6_PROT_NONPM, \
    E_SEAL_DSCRMB7_PROT_NONPM, \
    E_SEAL_X32_GPUAPB0_NONPM, \
    E_SEAL_X32_GPUAPB1_NONPM, \
    E_SEAL_X32_GPUAPB2_NONPM, \
    E_SEAL_X32_CRI_APB_PROT_NONPM, \
    E_SEAL_X32_CERT_PROT_NONPM

#define SEAL_NON_PM_GROUP21 \
    E_SEAL_X32_EMAC0_NONPM, \
    E_SEAL_X32_EMAC1_NONPM, \
    E_SEAL_X32_EMAC2_NONPM, \
    E_SEAL_X32_PKA_APB_PROT_NONPM, \
    E_SEAL_VMX_VMARK_SC0_PROT_NONPM, \
    E_SEAL_VMX_VMARK_SC2_PROT_NONPM, \
    E_SEAL_SC_EXT_PROT_NONPM, \
    E_SEAL_MIU_ARB_E_PROT_NONPM, \
    E_SEAL_MIU_ARB_E2_PROT_NONPM, \
    E_SEAL_MIU_ARB_F_PROT_NONPM, \
    E_SEAL_MIU_ARB_F2_PROT_NONPM, \
    E_SEAL_VD_R2_D_PROT_NONPM, \
    E_SEAL_AU_R2_0_I_PROT_NONPM, \
    E_SEAL_AU_R2_1_I_PROT_NONPM, \
    E_SEAL_MSC_EXT_PROT_NONPM, \
    E_SEAL_X32_GPU_FBDEC_NONPM

#define SEAL_NON_PM_GROUP22 \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_RESERVED_PROT_NONPM, \
    E_SEAL_GPU_PROTECTED_MODE_PROT_NONPM, \
    E_SEAL_MI2MCMI_PROT_NONPM, \
    E_SEAL_MI2MEHALO_R_PROT_NONPM, \
    E_SEAL_MI2MEHALO_W_PROT_NONPM

#define SEAL_NON_PM_GROUP23 \
    E_SEAL_MI2MCME_R_PROT_NONPM, \
    E_SEAL_MI2MCME_W_PROT_NONPM, \
    E_SEAL_MI2OPMI1_PROT_NONPM, \
    E_SEAL_MI2OPMI0_PROT_NONPM, \
    E_SEAL_MI2OPME1_PROT_NONPM, \
    E_SEAL_MI2OPME0_PROT_NONPM, \
    E_SEAL_MI2OPM1_PROT_NONPM, \
    E_SEAL_MI2OPM0_PROT_NONPM, \
    E_SEAL_MI2IPM1_PROT_NONPM, \
    E_SEAL_MI2IPM0_PROT_NONPM, \
    E_SEAL_MI2FSCM3_PROT_NONPM, \
    E_SEAL_MI2FSCM2_PROT_NONPM, \
    E_SEAL_DIP_2_PROT_NONPM, \
    E_SEAL_GOPMIX_SD_PROT_NONPM, \
    E_SEAL_DIP_0_PROT_NONPM, \
    E_SEAL_MIU_BIST_PROT_NONPM

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
    E_SEAL_LOCK_DUMMY, \
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

#define SEAL_LOCK_GROUP21 \
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

#define SEAL_LOCK_GROUP22 \
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

#define MIU0_PA_BASE               0x00000000UL
#define MIU1_PA_BASE               0x80000000UL
#define MIU2_PA_BASE               0xC0000000UL


const int NonPmIpTbl[SEAL_NONPM_TBL_IP_NUM] =
{
    SEAL_NON_PM_GROUP0, SEAL_NON_PM_GROUP1, SEAL_NON_PM_GROUP2,
    SEAL_NON_PM_GROUP3, SEAL_NON_PM_GROUP4, SEAL_NON_PM_GROUP5,
    SEAL_NON_PM_GROUP6, SEAL_NON_PM_GROUP7, SEAL_NON_PM_GROUP8,
    SEAL_NON_PM_GROUP9, SEAL_NON_PM_GROUP10, SEAL_NON_PM_GROUP11,
    SEAL_NON_PM_GROUP12, SEAL_NON_PM_GROUP13, SEAL_NON_PM_GROUP14,
    SEAL_NON_PM_GROUP15, SEAL_NON_PM_GROUP16, SEAL_NON_PM_GROUP17,
    SEAL_NON_PM_GROUP18, SEAL_NON_PM_GROUP19, SEAL_NON_PM_GROUP20,
    SEAL_NON_PM_GROUP21, SEAL_NON_PM_GROUP22, SEAL_NON_PM_GROUP23
};

const int LockTbl[SEAL_TBL_LOCK_NUM] =
{
    SEAL_LOCK_GROUP10, /*0x64*/
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP21,     SEAL_LOCK_GROUP22,
    SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY, SEAL_LOCK_GROUP_DUMMY,
    SEAL_LOCK_GROUP0, /*0x74*/
    SEAL_LOCK_GROUP1, SEAL_LOCK_GROUP2, SEAL_LOCK_GROUP3,
    SEAL_LOCK_GROUP4, SEAL_LOCK_GROUP5, SEAL_LOCK_GROUP6,
    SEAL_LOCK_GROUP7, SEAL_LOCK_GROUP8, SEAL_LOCK_GROUP9
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

int HAL_SEAL_GetNonPmIpIdx(int eNonPmIP)
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

int HAL_SEAL_GetLockIdx(int eLockId)
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
        unsigned int offset = i*REG_SECURE_RANGE_OFFSET;
        unsigned int startRegOffset = REG_SECURE_RANGE0_START_ADDR + offset;
        unsigned int endRegOffset = REG_SECURE_RANGE0_END_ADDR + offset;
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
    // mask decode error when non-secure CPU access miu secure range
    // mask decode error when non-secure CPU access secure riu register bank
    // mask decode error when CPU access undefined address area
    (RIU[SEAL_TZPC_NONPM_BASE + (0x7e<<1)]) |= (0x000e);

    // Set up the configuration of miu ns table
    // Only Seucre ARM can modify it - 9'd377
    (RIU[REG_TZPC_MIU0_CTL]) = REG_TZPC_MIU_TOP_EN;
    (RIU[REG_TZPC_MIU1_CTL]) = REG_TZPC_MIU_TOP_EN;

    RIU[REG_TZPC_MIU0_ID0] = 0x10C3; // 195 | bit12
    RIU[REG_TZPC_MIU1_ID0] = 0x10C3;
    while (RIU[REG_TZPC_MIU0_ID0] != 0x10C3);
    while (RIU[REG_TZPC_MIU1_ID0] != 0x10C3);

    // Init the access id permission table
    for (i=0, offset=0; i<SEAL_NONPM_TBL_IP_NUM; ++i, offset+=0x20)
    {
        if (i == 141 //dscrmb3, dscrmb_file_w_ree
            || i == 189 //dscrmb5, dscrmb_sha_r
            || i == 140 //dscrmb2, dscrmb_file_r_tee
            || i == 173 //dscrmb4, dscrmb_file_w_tee
            || i == 101 //nfie0
            || i == 102 //nfie1
            || i == 103 //nfie2
            || i == 104 //nfie4
            || i == 188 //nfie3
            || i == 17 //dscrmb, dscrmb_file_r_ree
            || i == 68 //tzpc_arm_I_2miu_secure_ns
            || i == 195) //tzpc_arm_D_2miu_secure_ns
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
    while (*(volatile unsigned char*)(CONFIG_MIU0_BUSADDR + 0x20*383) != 0x01);

    for (i=0, offset=0; i<SEAL_NONPM_TBL_IP_NUM; ++i, offset+=0x20)
    {
        if (i == 141 //dscrmb3, dscrmb_file_w_ree
            || i == 189 //dscrmb5, dscrmb_sha_r
            || i == 140 //dscrmb2, dscrmb_file_r_tee
            || i == 173 //dscrmb4, dscrmb_file_w_tee
            || i == 101 //nfie0
            || i == 102 //nfie1
            || i == 103 //nfie2
            || i == 104 //nfie4
            || i == 188 //nfie3
            || i == 17 //dscrmb, dscrmb_file_r_ree
            || i == 68 //tzpc_arm_I_2miu_secure_ns
            || i == 195) //tzpc_arm_D_2miu_secure_ns
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
    while (*(volatile unsigned char*)(CONFIG_MIU1_BUSADDR + 0x20*383) != 0x01);

    RIU[REG_TZPC_MIU0_ID0] = 0x00C3;
    RIU[REG_TZPC_MIU1_ID0] = 0x00C3;
    while (RIU[REG_TZPC_MIU0_ID0] != 0x00C3);
    while (RIU[REG_TZPC_MIU1_ID0] != 0x00C3);

    // Set hosts of RIU bridge to non-secure (except ARM and DB_BUS)
    RIU[SEAL_TZPC_PM_BASE + REG_PM_RIU_BRIDGE] = (0x5554);
    RIU[SEAL_TZPC_NONPM_BASE + REG_NONPM_RIU_BRIDGE] = (0x5554);

    return 1;
}

int HAL_SEAL_Cleanup(void)
{
    int i, offset;

    RIU[REG_TZPC_MIU0_ID0] = 0x10C3; // 195 | bit12
    RIU[REG_TZPC_MIU1_ID0] = 0x10C3;
    while (RIU[REG_TZPC_MIU0_ID0] != 0x10C3);
    while (RIU[REG_TZPC_MIU1_ID0] != 0x10C3);

    // Init the access id permission table
    for (i=0, offset=0; i<SEAL_NONPM_TBL_IP_NUM; ++i, offset+=0x20)
    {
        if (i == 101 //nfie0
            || i == 102 //nfie1
            || i == 103 //nfie2
            || i == 104 //nfie4
            || i == 188 ) //nfie3
        {
            *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset) = 0x0000000000000301ULL;
            *(volatile unsigned long long*)(CONFIG_MIU0_BUSADDR + offset + 8) = 0x0000000000000000ULL;

        }
    }
    while (*(volatile unsigned char*)(CONFIG_MIU0_BUSADDR + 0x20*383) != 0x01);

    for (i=0, offset=0; i<SEAL_NONPM_TBL_IP_NUM; ++i, offset+=0x20)
    {
        if (i == 101 //nfie0
            || i == 102 //nfie1
            || i == 103 //nfie2
            || i == 104 //nfie4
            || i == 188 ) //nfie3
        {
            *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset) = 0x0000000000000301ULL;
            *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset + 8) = 0x0000000000000000ULL;
        }
    }
    while (*(volatile unsigned char*)(CONFIG_MIU1_BUSADDR + 0x20*383) != 0x01);

    RIU[REG_TZPC_MIU0_ID0] = 0x00C3;
    RIU[REG_TZPC_MIU1_ID0] = 0x00C3;
    while (RIU[REG_TZPC_MIU0_ID0] != 0x00C3);
    while (RIU[REG_TZPC_MIU1_ID0] != 0x00C3);

    return 1;
}

int HAL_SEAL_SetMIUHost(int swIdx, int bSecure)
{
    int hwIdx = HAL_SEAL_GetNonPmIpIdx(swIdx);
    unsigned int offset;
    unsigned long long* val = (bSecure ? DefaultSecClient : DefaultNonSecClient);
    int j;

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

    (RIU[REG_TZPC_MIU1_ID0]) |= REG_TZPC_MIU_ID_ENABLE;
    __delay();
    *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset) = val[0];
    *(volatile unsigned long long*)(CONFIG_MIU1_BUSADDR + offset + 8) = val[1];
    __delay();
    (RIU[REG_TZPC_MIU1_ID0]) &= (~REG_TZPC_MIU_ID_ENABLE);
    __delay();
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
