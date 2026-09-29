#ifndef __ISP_EE_H__
#define __ISP_EE_H__

#include "isp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HBN_CURVE_SIZE         65   /**< The number of CA/DCI curve mode bin */
#define HBN_EE_HF_MERGE_NUM    4    /**< The number of EE HF merge */
#define HBN_EE_NOISE_NUM       2    /**< The number of EE noise point */
#define HBN_EE_ENHANCEMENT_NUM 2    /**< The number of EE enhancement */
#define HBN_EE_SKIN_NUM        6    /**< The number of EE skin */
#define HBN_DEPF_LIMIT_NUM     4   /**< The number of EE limitation */

/*****************************************************************************/
/**
 * @brief   EE CA mode configuration.
 *
 *****************************************************************************/
typedef enum hbn_isp_ee_ca_mode_s
{
    HBN_EE_CA_MODE0  = 0,    /**< Y mode */
    HBN_EE_CA_MODE1  = 1,    /**< Saturation mode */
    HBN_EE_CA_MODE2  = 2,    /**< Y & Saturation mode */
} hbn_isp_ee_ca_mode_t;

/******************************************************************************/
/**
 * @brief   EE edge and detail configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_edge_detail_attr_s
{
    uint8_t edge_nr_lvl;    /**< Edge NR level */
    uint8_t edge_scale;    /**< Edge scale */
    bool_t edge_use_auxi_dir;    /**< Edge use AUXIDIR */
    uint8_t detail_lvl;    /**< Detail level */
    uint8_t detail_scaler;    /**< Detail scaler */
    uint16_t detail_pre_enhance_str;    /**< Detail pre-enhance strength */
} hbn_isp_ee_edge_detail_attr_t;

/*****************************************************************************/
/**
 * @brief   EE skin configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_skin_attr_s
{
    uint8_t detect_str;    /**< Skin detection strength */
    uint16_t hth_y_min;    /**< Horizontal minimum threshold in Y channel */
    uint16_t hth_y_max;    /**< Horizontal maximum threshold in Y channel */
    int16_t hth_cr_min;    /**< Horizontal minimum threshold in Cr channel */
    int16_t hth_cr_max;    /**< Horizontal maximum threshold in Cr channel */
    int16_t hth_cb_min;    /**< Horizontal minimum threshold in Cb channel */
    int16_t hth_cb_max;    /**< Horizontal maximum threshold in Cb channel */
} hbn_isp_ee_skin_attr_t;

/*****************************************************************************/
/**
 * @brief   EE edge strength configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_edge_str_s{
    uint16_t t0;    /**< EE edge value of t0 */
    uint8_t t1_shift;    /**< EE edge value of t1 shift */
    uint16_t t2;    /**< EE edge value of t2 */
    uint8_t t3_shift;    /**< EE edge value of t3 shift */
} hbn_isp_ee_edge_str_t;

/******************************************************************************/
/**
 * @brief   EE merge curve configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_merge_curve_s
{
    hbn_isp_ee_edge_str_t hf_merge_curve;    /**< HF merge curve */
    hbn_isp_ee_edge_str_t hf_merge_curve_skin;    /**< HF merge curve skin */
} hbn_isp_ee_merge_curve_t;

/******************************************************************************/
/**
 * @brief   EE noise cut off configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_noise_cut_off_attr_s
{
    uint16_t grad_th_edge;    /**< Grad threshold of edge */
    uint16_t grad_th_detail;    /**< Grad detail threshold */
    uint16_t grad_th_edge_skin;    /**< Grad threshold of edge skin */
    uint16_t grad_th_detail_skin;    /**< Grad threshold of detail skin */
} hbn_isp_ee_noise_cut_off_attr_t;

/*****************************************************************************/
/**
 * @brief   enhancement coefficient configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_enhancement_coeff_s
{
    uint16_t up;    /**< Enhancement with up direction */
    uint16_t down;    /**< Enhancement with down direction */
} hbn_isp_ee_enhancement_coeff_t;

/******************************************************************************/
/**
 * @brief   EE enhancement configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_enhancement_attr_s
{
    uint8_t strength;    /**< Strength value */
    uint8_t sharp_curve_lvl;    /**< Sharp curve level */
    uint8_t sharp_skin_curve_lvl;    /**< Sharp skin curve level */
    hbn_isp_ee_enhancement_coeff_t sharp_gain;    /**< Sharp gain */
    hbn_isp_ee_enhancement_coeff_t sharp_gain_skin;    /**< Sharp skin gain */
    uint8_t sharp_gain_uv;    /**< Sharp UV channel gain */
    uint8_t sharp_limit_type;    /**< Sharpen limit type */
    hbn_isp_ee_enhancement_coeff_t sharp_limit;    /**< Sharp limit */
    hbn_isp_ee_enhancement_coeff_t sharp_limit_skin;    /**< Sharp skin limit */
} hbn_isp_ee_enhancement_attr_t;

/******************************************************************************/
/**
 * @brief   EE depurple configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_depurple_attr_s
{
    uint8_t depf_detect_range;    /**< Detection range */
    uint16_t depf_u_up_limit;    /**< Up limit of U channel */
    uint16_t depf_u_down_limit;    /**< Down limit of Y channel  */
    uint16_t depf_v_up_limit;    /**< Up limit of V channel */
    uint16_t depf_v_down_limit;    /**< Down limit of V channel */
    uint16_t depf_detect_luma_th;    /**< Luma threshold detection */
    uint16_t depf_detect_luma_diff;    /**< Luma difference detection */
    uint16_t depf_comp_luma_diff;    /**< Luma difference comparasion */
    uint16_t depf_sat_str;    /**< Sauration strength */
    uint16_t depf_fix_str;    /**< Fix strength */
} hbn_isp_ee_depurple_attr_t;

/******************************************************************************/
/**
 * @brief   EE CA configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_ca_attr_s
{
    uint16_t lut_chroma_tbl[HBN_CURVE_SIZE];    /**< EE CA curve */
    hbn_isp_ee_ca_mode_t ca_mode;    /**< EE CA mode */
} hbn_isp_ee_ca_attr_t;

/******************************************************************************/
/**
 * @brief   EE DCI configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_dci_attr_s
{
    float32_t degamma;    /**< EE DCI degamma */
    uint16_t lut_luma_tbl[HBN_CURVE_SIZE]; /**< EE DCI luma table */
} hbn_isp_ee_dci_attr_t;

/******************************************************************************/
/**
 * @brief   EE current configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_manual_attr_s
{
    uint16_t src_strength;         /**< Y channel input image weight */
    uint16_t src_strength_skin;    /**< Weight to merge EE result to input image */
    hbn_isp_ee_edge_detail_attr_t edge_detail_attr;    /**< EE edge detail configuration */
    hbn_isp_ee_skin_attr_t skin_attr;    /**< EE skin configuration */
    hbn_isp_ee_merge_curve_t merge_curve_attr;    /**< EE merge curve configuration */
    hbn_isp_ee_noise_cut_off_attr_t noise_cut_off_attr;    /**< EE noise cut off configuration */
    hbn_isp_ee_enhancement_attr_t enhancement_attr;    /**< EE enhancement configuration */
    hbn_isp_ee_depurple_attr_t depurple_attr;    /**< EE depurple configuration */
    hbn_isp_ee_ca_attr_t ca_attr;    /**< EE CA configuration */
    hbn_isp_ee_dci_attr_t dci_attr;    /**< EE DCI configuration */
} hbn_isp_ee_manual_attr_t;

/******************************************************************************/
/**
 * @brief   EE auto configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_auto_attr_ {
    uint8_t auto_level;                       /**< EE auto configuration level */
    float32_t gains[HBN_ISO_STRENGTH_NUM];            /**< EE gain */
    uint8_t ee_strength[HBN_ISO_STRENGTH_NUM];    /**< EE strength */
    uint16_t ee_src_strength[HBN_ISO_STRENGTH_NUM];    /**< Y channel input image weight */
    uint16_t ee_src_strength_skin[HBN_ISO_STRENGTH_NUM];    /**< Weight to merge EE result to input image */

    uint8_t edge_nr_lvl[HBN_ISO_STRENGTH_NUM];    /**< Edge NR level */
    uint8_t detail_lvl[HBN_ISO_STRENGTH_NUM];    /**< Detail level */
    uint16_t detail_pre_enhance_str[HBN_ISO_STRENGTH_NUM];    /**< Detail pre-enhance strength */

    uint8_t skin_proc_en[HBN_ISO_STRENGTH_NUM];    /**< Skin process enable */
    uint8_t skin_detect_str[HBN_ISO_STRENGTH_NUM];    /**< Skin detection strength */
    int16_t skin_threshold[HBN_ISO_STRENGTH_NUM][HBN_EE_SKIN_NUM];    /**< Skin threshold */

    uint16_t hf_merge_curve[HBN_ISO_STRENGTH_NUM][HBN_EE_HF_MERGE_NUM];    /**< HF merge curve */
    uint16_t hf_merge_curve_skin[HBN_ISO_STRENGTH_NUM][HBN_EE_HF_MERGE_NUM];    /**< HF merge curve skin */

    uint16_t grad_th[HBN_ISO_STRENGTH_NUM][HBN_EE_NOISE_NUM];    /**< Grad threshold of edge */
    uint16_t grad_th_skin[HBN_ISO_STRENGTH_NUM][HBN_EE_NOISE_NUM];    /**< Grad threshold of edge skin */

    uint8_t sharp_curve_lvl[HBN_ISO_STRENGTH_NUM];    /**< Sharp curve level */
    uint8_t sharp_skin_curve_lvl[HBN_ISO_STRENGTH_NUM];    /**< Sharp skin curve level */
    uint16_t sharp_gain[HBN_ISO_STRENGTH_NUM][HBN_EE_ENHANCEMENT_NUM];    /**< Sharp gain */
    uint16_t sharp_gain_skin[HBN_ISO_STRENGTH_NUM][HBN_EE_ENHANCEMENT_NUM];    /**< Sharp skin gain */
    uint8_t sharp_gain_uv[HBN_ISO_STRENGTH_NUM];    /**< Sharp UV channel gain */
    uint16_t sharp_limit[HBN_ISO_STRENGTH_NUM][HBN_EE_ENHANCEMENT_NUM];    /**< Sharp limit */
    uint16_t sharp_limit_skin[HBN_ISO_STRENGTH_NUM][HBN_EE_ENHANCEMENT_NUM];    /**< Sharp skin limit */

    bool_t depurple_en[HBN_ISO_STRENGTH_NUM];    /**< Enable depurple */
    uint16_t depf_limit[HBN_ISO_STRENGTH_NUM][HBN_DEPF_LIMIT_NUM];    /**< DEPF limit  */
    uint16_t depf_detect_luma_th[HBN_ISO_STRENGTH_NUM];    /**< Luma threshold detection */
    uint16_t depf_detect_luma_diff[HBN_ISO_STRENGTH_NUM];    /**< Luma difference detection */
    uint16_t depf_comp_luma_diff[HBN_ISO_STRENGTH_NUM];    /**< Luma difference comparasion */
    uint16_t depf_sat_str[HBN_ISO_STRENGTH_NUM];    /**< Sauration strength */
    uint16_t depf_fix_str[HBN_ISO_STRENGTH_NUM];    /**< Fix strength */

    bool_t ca_enable[HBN_ISO_STRENGTH_NUM];   /**< EE CA enable value */
    uint16_t lut_chroma_tbl[HBN_ISO_STRENGTH_NUM][HBN_CURVE_SIZE]; /**< EE color adjust curve  */
    bool_t dci_enable[HBN_ISO_STRENGTH_NUM]; /**< EE DCI enable value */
    uint16_t dci_curve[HBN_ISO_STRENGTH_NUM][HBN_CURVE_SIZE]; /**< EE dynamic contrast improve curve */
} hbn_isp_ee_auto_attr_t;

/******************************************************************************/
/**
 * @brief   EE configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ee_attr_s
{
    hbn_isp_mode_e mode;
    hbn_isp_ee_manual_attr_t manual_attr;
    hbn_isp_ee_auto_attr_t auto_attr;
} hbn_isp_ee_attr_t;

#ifdef __cplusplus
}
#endif

#endif // __ISP_EE_H__