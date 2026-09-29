#ifndef __ISP_DEMOSAIC_H__
#define __ISP_DEMOSAIC_H__

#include "isp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HBN_DMSC_INTP_STRENGTH_LEVEL_NUM   4    /**< The number of DMSC interpolation strength level */
#define HBN_DMSC_INTERP_STR                2    /**< DMSC interpolation strength */
#define HBN_DMSC_SHARPEN_LAYER_NUM         3    /**< The number of DMSC sharpen layer */
#define HBN_DMSC_DEPURPLE_NUM              2    /**< The number of DMSC depurple */
#define HBN_DMSC_HIGHLIGHT_NUM             2    /**< The number of DMSC high light */



/*****************************************************************************/
/**
 * @brief   DMSC adaptive coefficient mode.
 *
 *****************************************************************************/
typedef enum hbn_isp_dmsc_adpt_coef_mode_e {
     HBN_DMSC_ADPT_COEF_MODE_DISABLE = 0,  /**< Adaptive mode off */
     HBN_DMSC_ADPT_COEF_R_B_CHANNEL,       /**< Enable coefficient adaptive calculation mode for the points in red and blue channel */
     HBN_DMSC_ADPT_COEF_GR_GB_CHANNEL,     /**< Enable coefficient adaptive calculation mode for the points in green and green blue channel */
     HBN_DMSC_ADPT_COEF_ALL_CHANNEL,       /**< Enable coefficient adaptive calculation mode for all points */
} hbn_isp_dmsc_adpt_coef_mode_t;

/*****************************************************************************/
/**
 * @brief   DMSC interpolation type.
 *
 *****************************************************************************/
typedef enum hbn_isp_dmsc_intp_type_e {
     HBN_DMSC_DIR0_INTERP_TYPE0 = 0,   /**< Interpolation type0 */
     HBN_DMSC_DIR0_INTERP_TYPE1,       /**< Interpolation type1 */
     HBN_DMSC_DIR0_INTERP_TYPE2,       /**< Interpolation type2 */
     HBN_DMSC_DIR0_INTERP_TYPE3,       /**< Interpolation type3 */
} hbn_isp_dmsc_intp_type_t;

/*****************************************************************************/
/**
 * @brief   DMSC deburst mode.
 *
 *****************************************************************************/
typedef enum hbn_isp_dmsc_deburst_mode_e {
     HBN_DMSC_DEBURST_MODE0 = 0,    /**< DMSC deburst mode0 */
     HBN_DMSC_DEBURST_MODE1,        /**< DMSC deburst mode1 */
     HBN_DMSC_DEBURST_MODE2,        /**< DMSC deburst mode2 */
     HBN_DMSC_DEBURST_MODE3,        /**< DMSC deburst mode3 */
     HBN_DMSC_DEBURST_MODE4,        /**< DMSC deburst mode4 */
     HBN_DMSC_DEBURST_MODE5,        /**< DMSC deburst mode5 */
     HBN_DMSC_DEBURST_MODE6,        /**< DMSC deburst mode6 */
     HBN_DMSC_DEBURST_MODE7,        /**< DMSC deburst mode7 */
} hbn_isp_dmsc_deburst_mode_t;

/*****************************************************************************/
/**
 * @brief  DMSC depurple color range mode.
 *
 *****************************************************************************/
typedef enum hbn_isp_dmsc_depurple_color_range_mode_e {
     HBN_DMSC_DEPURPLE_COLOR_RANGE_MODE0 = 0,  /**< DMSC depurple color range mode0 */
     HBN_DMSC_DEPURPLE_COLOR_RANGE_MODE1,      /**< DMSC depurple color range mode1 */
     HBN_DMSC_DEPURPLE_COLOR_RANGE_MODE2,      /**< DMSC depurple color range mode2 */
     HBN_DMSC_DEPURPLE_COLOR_RANGE_MODE3,      /**< DMSC depurple color range mode3 */
     HBN_DMSC_DEPURPLE_COLOR_RANGE_MODE4,      /**< DMSC depurple color range mode4 */
     HBN_DMSC_DEPURPLE_COLOR_RANGE_MODE5,      /**< DMSC depurple color range mode5 */
     HBN_DMSC_DEPURPLE_COLOR_RANGE_MODE6,      /**< DMSC depurple color range mode6 */
     HBN_DMSC_DEPURPLE_COLOR_RANGE_MODE7,      /**< DMSC depurple color range mode7 */
} hbn_isp_dmsc_depurple_color_range_mode_t;

/*****************************************************************************/
/**
 * @brief   DMSC depurple CbCr mode.
 *
 *****************************************************************************/
typedef enum hbn_isp_dmsc_depurple_cbcr_mode_e {
     HBN_DMSC_DEPURPLE_CB_CR_MODE_DISABLE = 0,  /**< DMSC depurple mode off in CbCr channel */
     HBN_DMSC_DEPURPLE_CR_CHANNEL,              /**< Enable DMSC depurple mode in Cr channel */
     HBN_DMSC_DEPURPLE_CB_CHANNEL,              /**< Enable DMSC depurple mode in Cb channel */
     HBN_DMSC_DEPURPLE_CB_CR_CHANNEL,           /**< Enable DMSC depurple mode in CbCr channel */
} hbn_isp_dmsc_depurple_cbcr_mode_t;

/*****************************************************************************/
/**
 * @brief   DMSC CAC configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_cac_attr_s
{
    float32_t a_blue;            /**< Parameter aBlue for radial blue shift calculation. */
    float32_t a_red;             /**< Parameter aRed for radial blue shift calculation. */
    float32_t b_blue;            /**< Parameter bBlue for radial blue shift calculation. */
    float32_t b_red;             /**< Parameter bRed for radial blue shift calculation. */
    float32_t c_blue;            /**< Parameter cBlue for radial blue shift calculation. */
    float32_t c_red;             /**< Parameter cRed for radial blue shift calculation. */
    int16_t center_h_offs;       /**< The red/blue pixel shift in horizontal direction. */
    int16_t center_v_offs;       /**< The red/blue pixel shift in vertical direction. */
} hbn_isp_dmsc_cac_attr_t;

/******************************************************************************/
/**
 * @brief   DMSC demoire configuration.
 *
 ****************************************************************************/
typedef struct hbn_isp_dmsc_demoire_attr_s
{
    uint16_t refine_g_th;  /**< Refine the threshold, 12-bit, range:[0, 4095] */
} hbn_isp_dmsc_demoire_attr_t;

/******************************************************************************/
/**
 * @brief   DMSC sharpen line configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_sharpen_line_attr_s
{
    uint16_t strength;  /**< Sharpen line strength, 12-bit, range:[0, 4095] */
    uint16_t t1;  /**< Sharpen line t1 value, 11bit, range:[0, 2047] */
    uint8_t shift1;    /**< Sharpen line shift1, 4bit, range:[0, 11] */
    uint16_t r1;    /**< Sharpen line r1, 9bit, range:[0, 256] */
    uint16_t r2;  /**< Sharpen line r2, 9bit, range:[0, 256] */
} hbn_isp_dmsc_sharpen_line_attr_t;

/******************************************************************************/
/**
 * @brief   DMSC sharpen layer configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_sharpen_layer_e{
    uint16_t white;    /**< Sharpen layer white factor */
    uint16_t black;    /**< Sharpen layer black factor */
} hbn_isp_dmsc_sharpen_layer_t;

/*****************************************************************************/
/**
 * @brief   DMSC sharpen curve structure.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_sharpen_curve_s{
    uint16_t layer_t1;    /**< Sharpen curve layer T1 */
    uint8_t layer_t2_shift;    /**< Sharpen curve layer T2 shift */
    uint16_t layer_t3;    /**< Sharpen curve layer T3 */
    uint8_t layer_t4_shift;    /**< Sharpen curve layer T4 shift */
    uint16_t layer_r1;    /**< Sharpen curve layer R1 */
    uint16_t layer_r2;    /**< Sharpen curve layer R2 */
    uint16_t layer_r3;    /**< Sharpen curve layer R3 */
} hbn_isp_dmsc_sharpen_curve_t;

/*****************************************************************************/
/**
 * @brief   DMSC sharpen structure.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_sharpen_attr_s
{
    uint8_t denoise_strength;    /**< Sharpen denoise strength, 6bit, range:[0, 32] */
    hbn_isp_dmsc_sharpen_layer_t factor[HBN_DMSC_SHARPEN_LAYER_NUM];  /**< Sharpen layer factor */
    hbn_isp_dmsc_sharpen_layer_t clip[HBN_DMSC_SHARPEN_LAYER_NUM];  /**< Sharpen layer clip */
    hbn_isp_dmsc_sharpen_curve_t curve[HBN_DMSC_SHARPEN_LAYER_NUM];  /**< Sharpen layer curve, 11bit, range:[0, 2047] */
} hbn_isp_dmsc_sharpen_attr_t;

/*****************************************************************************/
/**
 * @brief   DMSC defalse configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_defalse_attr_e
{
    uint8_t color_str;  /**< Color strength, 8-bit, range:[0, 128] */
    uint8_t cbcr_class_th;  /**< CbCr class threshold, 3bit, range:[0, 7] */
    uint8_t gray_protect_str;  /**< Gray protection strength, 8-bit, range:[0, 128] */
} hbn_isp_dmsc_defalse_attr_t;

/******************************************************************************/
/**
 * @brief   DMSC depurple configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_depurple_attr_s
{
    uint8_t grad_control; /**< Grad control, 1bit, range:[0, 1] */
    uint8_t protect_th_low;   /**< Low threshold protection, 8-bit, range:[0,255] */
    uint8_t protect_shift;  /**< Shift protection, 4bit, range:[0, 8] */
    uint8_t thr;  /**< Threshold, 8-bit, range:[0, 255] */
    hbn_isp_dmsc_depurple_cbcr_mode_t cbcr_mode;  /**< Depurple CbCr mode, 2bit, range:[0, 3] */
    uint8_t sat_shrink;    /**< Saturation shrink, 4bit, range:[0, 8] */
    hbn_isp_dmsc_depurple_color_range_mode_t color_range_mode;  /**< Color range mode, 2bit, range:[0, 3] */
    uint8_t cb_lowth[HBN_DMSC_DEPURPLE_NUM];    /**< Low threshold of Cb channel */
    uint8_t cb_highth[HBN_DMSC_DEPURPLE_NUM];    /**< High threshold of Cb channel */
    uint8_t cr_lowth[HBN_DMSC_DEPURPLE_NUM];    /**< Low threshold of Cr channel */
    uint8_t cr_highth[HBN_DMSC_DEPURPLE_NUM];    /**< High threshold of Cr channel */
} hbn_isp_dmsc_depurple_attr_t;

/******************************************************************************/
/**
 * @brief   DMSC interpolation corner configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_interp_corner_attr_s
{
    uint8_t inter_dir_strength[HBN_DMSC_INTP_STRENGTH_LEVEL_NUM];   /**< Interpolation direction strength, 8-bit, range:[0, 255] */

    uint8_t interp_large_str[HBN_DMSC_INTERP_STR];    /**< Large strength of interpolation, 5bit, range:[0, 16] */
    hbn_isp_dmsc_intp_type_t dir0_interp_type;     /**< Interpolation type in direction0, 2bit, range:[0, 3] */
} hbn_isp_dmsc_interp_corner_attr_t;

/******************************************************************************/
/**
 * @brief   DMSC decline configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_decline_attr_s
{
    uint16_t corner_low_th;    /**< Low threshold of corner, 12-bit, range:[0, 4095] */
    uint8_t corner_shift;     /**< Corner shift, 4bit, range:[0, 12] */
} hbn_isp_dmsc_decline_attr_t;

/*****************************************************************************/
/**
 * @brief   DMSC current structure.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_manual_attr_s {
    uint16_t grad_th_min;    /**< Minimum grad threshold, 12-bit, range:[0, 4095] */
    uint16_t grad_th_max;    /**< Maximum grad threshold, 12-bit, range:[0, 4095] */
    uint8_t diff_color_coef;  /**< Color coefficient difference, 8-bit, range:[0, 128] */
    hbn_isp_dmsc_adpt_coef_mode_t adpt_coef_mode;  /**< Adaptive coefficient mode, 2bit, range: [0, 3] */
    uint8_t adpt_coef_th_low;  /**< Low threshold of adaptive coefficient, 8-bit, range:[0, 255] */
    uint8_t adpt_coef_th_shift;  /**< Threshold shift of adaptive coefficient, 4bit, range:[0, 8] */
    uint8_t adpt_coef_min;  /**< Minimum adaptive coefficient, 8-bit, range:[0, 128] */
    uint8_t adpt_coef_max;    /**< Maximum adaptive coefficient, 8-bit, range:[0, 128] */

    uint16_t highlight_control_th;    /**< Highlight control threshold, 12-bit, range:[0, 4095] */
    uint8_t highlight_control_str;  /**< Highlight control strength, 1bit, range:[0, 1] */

    hbn_isp_dmsc_deburst_mode_t deburst_mode;  /**< Use 2 bit, 0 for green, bit 1 for red blue*/
    uint8_t pure_g_weight;  /**< Pure green weight, 5bit, range:[0, 16] */
    uint8_t pure_rb_weight;  /**< Pure red blue weight, 5bit, range:[0, 16] */
    uint8_t pure_g_weight_iso;  /**< Pure green weight ISO, 5bit, range:[0, 16] */
    uint8_t pure_rb_weight_iso;  /**< Pure red blue weight ISO, 5bit, range:[0, 16] */
    hbn_isp_dmsc_cac_attr_t cac_attr;    /**< DMSC CAC configuration*/
    hbn_isp_dmsc_demoire_attr_t demoire_attr;    /**< DMSC demoire configuration*/
    hbn_isp_dmsc_sharpen_line_attr_t sharpen_line_attr;    /**< DMSC sharpen line configuration*/
    hbn_isp_dmsc_sharpen_attr_t sharpen_attr;    /**< DMSC sharpen configuration*/
    hbn_isp_dmsc_defalse_attr_t defalse_attr;   /**< DMSC defalse configuration*/
    hbn_isp_dmsc_depurple_attr_t depurple_attr;   /**< DMSC depurple configuration*/
    hbn_isp_dmsc_interp_corner_attr_t interp_corner_attr;   /**< DMSC inter corner configuration*/
    hbn_isp_dmsc_decline_attr_t decline_attr;    /**< DMSC decline configuration*/
} hbn_isp_dmsc_manual_attr_t;


/*****************************************************************************/
/**
 * @brief   DMSC auto configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_auto_attr_s
{
    uint8_t auto_level;                 /**< The auto config level */
    float32_t gains[HBN_ISO_STRENGTH_NUM];              /**< DMSC gains */
    uint8_t interp_dir_strength[HBN_ISO_STRENGTH_NUM][HBN_DMSC_INTP_STRENGTH_LEVEL_NUM];  /**< Interpolation direction strength, 8-bit, range:[0, 255] */
    uint8_t interp_large_str[HBN_ISO_STRENGTH_NUM][HBN_DMSC_INTERP_STR];    /**< Large strength of interpolation, 5bit, range:[0, 16] */
    bool_t decline_en[HBN_ISO_STRENGTH_NUM];   /**< Enable decline, 1bit, range:[0, 1] */
    uint16_t highlight_th[HBN_ISO_STRENGTH_NUM][HBN_DMSC_HIGHLIGHT_NUM];    /**< High light threshold */

    bool_t dmsc_depurple_enable[HBN_ISO_STRENGTH_NUM];    /**< Enable DMSC depurple */
    uint8_t dmsc_depurple_thr[HBN_ISO_STRENGTH_NUM];    /**< DMSC depurple threshold */
    uint8_t dmsc_depurple_sat_shrink[HBN_ISO_STRENGTH_NUM];    /**< DMSC depurple saturation shrink */
    uint8_t dmsc_depurple_cbcr_range[HBN_ISO_STRENGTH_NUM][HBN_DMSC_DEPURPLE_NUM * 4];    /**< DMSC depurple CbcCr range */

    uint8_t denoise_strength[HBN_ISO_STRENGTH_NUM];    /**< Sharpen denoise strength, 6bit, range:[0, 32] */

    bool_t dmsc_sharpen_enable[HBN_ISO_STRENGTH_NUM];    /**< Enable DMSC sharpen effect */
    uint16_t dmsc_sharpen_factor[HBN_ISO_STRENGTH_NUM][HBN_DMSC_SHARPEN_LAYER_NUM * 2];    /**< Enable DMSC sharpen factor effect */
    uint16_t dmsc_sharpen_clip[HBN_ISO_STRENGTH_NUM][HBN_DMSC_SHARPEN_LAYER_NUM * 2];    /**< Enable DMSC sharpen clip effect */
    hbn_isp_dmsc_sharpen_curve_t dmsc_sharpen_curve0[HBN_ISO_STRENGTH_NUM];    /**< DMSC sharpen curve0 */
    hbn_isp_dmsc_sharpen_curve_t dmsc_sharpen_curve1[HBN_ISO_STRENGTH_NUM];    /**< DMSC sharpen curve1 */
    hbn_isp_dmsc_sharpen_curve_t dmsc_sharpen_curve2[HBN_ISO_STRENGTH_NUM];    /**< DMSC sharpen curve2 */
} hbn_isp_dmsc_auto_attr_t;

/*****************************************************************************/
/**
 * @brief   DMSC configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_dmsc_attr_s {
    hbn_isp_mode_e mode;
    hbn_isp_dmsc_manual_attr_t manual_attr;  /**< DMSC manual configuration*/
    hbn_isp_dmsc_auto_attr_t auto_attr;    /**< DMSC auto configuration*/
} hbn_isp_dmsc_attr_t;

#ifdef __cplusplus
}
#endif

#endif // __ISP_DEMOSAIC_H__