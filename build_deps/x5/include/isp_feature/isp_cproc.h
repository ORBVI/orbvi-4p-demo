#ifndef __ISP_CPROC_H__
#define __ISP_CPROC_H__

#include "isp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HBN_CPROC_RGBTOYUV_MATRIX_NUM 9

/******************************************************************************/
/**
 * @brief   CPROC pixel clipping range type enumeration for chroma and luma.
 *
 *****************************************************************************/
typedef enum hbn_isp_cproc_yuv_range_type_e {
    HBN_CPROC_YUV_RANGE_INVALID       = 0,    /**< Invalid range tpye */
    HBN_CPROC_YUV_RANGE_LIMIT_RANGE   = 1,    /**< Limit Y/U/V clipping range 16..240 */
    HBN_CPROC_YUV_RANGE_FULL_RANGE    = 2,    /**< Full Y/U/V clipping range 0..255 */
    HBN_CPROC_YUV_RANGE_MAX                   /**< Range type max */
} hbn_isp_cproc_yuv_range_type_t;
/*******************************************/

/******************************************************************************/
/**
 * @brief   CPROC color gamut type enumeration.
 *
 *****************************************************************************/
typedef enum hbn_isp_cproc_color_gamut_type_e {
    HBN_CPROC_COLOR_GAMUT_BT601       = 0,    /**< ITU-R BT.601 standard */
    HBN_CPROC_COLOR_GAMUT_BT709       = 1,    /**< ITU-R BT.709 standard */
    HBN_CPROC_COLOR_GAMUT_BT2020      = 2,    /**< ITU-R BT.2020 standard */
    HBN_CPROC_COLOR_GAMUT_MAX                 /**< gamut type max */
} hbn_isp_cproc_color_gamut_type_t;
/*******************************************/

/******************************************************************************/
/**
 * @brief   CPROC YUV range configuration structure.
 *
 *****************************************************************************/
typedef struct hbn_isp_cproc_yuv_range_s {
    bool_t use_out_conv_matrix;       /**< The flag for use outConvMatrix or not*/
    float32_t rgb_to_yuv_matrix[HBN_CPROC_RGBTOYUV_MATRIX_NUM];  /**< The rgb to yuvMatrix*/
    hbn_isp_cproc_yuv_range_type_t yuv_range;        /**< The clipping range for chrominance pixel output*/
    hbn_isp_cproc_color_gamut_type_t gamut;       /** < The color gamut select*/
} hbn_isp_cproc_yuv_range_t;

/******************************************************************************/
/**
 * @brief   CPROC auto configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_cproc_auto_attr_s {
    uint8_t auto_level;                          /**< The auto configuration level */

    float32_t gains[HBN_ISO_STRENGTH_NUM];       /**< CPROC gains */
    float32_t contrast[HBN_ISO_STRENGTH_NUM];    /**< CPROC contrast adjust value */
    float32_t bright[HBN_ISO_STRENGTH_NUM];      /**< CPROC brightness adjust value */
    float32_t saturation[HBN_ISO_STRENGTH_NUM];  /**< CPROC saturation adjust value */
    float32_t hue[HBN_ISO_STRENGTH_NUM];         /**< CPROC rotation in HSV domain */
} hbn_isp_cproc_auto_attr_t;


/******************************************************************************/
/**
 * @brief   CPROC current configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_cproc_manual_attr_s {
    float32_t contrast;     /**< CPROC contrast adjust value [0, 1.999]*/
    float32_t bright;		/**< CPROC brightness adjust value [-127, 127]*/
    float32_t saturation;	/**< CPROC saturation adjust value [0, 1.999]*/
    float32_t hue;			/**< CPROC rotation in HSV domain [-90, 90]*/
} hbn_isp_cproc_manual_attr_t;

/******************************************************************************/
/**
 * @brief   CPROC configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_cproc_attr_s {
    hbn_isp_mode_e mode;
    hbn_isp_cproc_auto_attr_t auto_attr;
    hbn_isp_cproc_manual_attr_t manual_attr;
} hbn_isp_cproc_attr_t;


#ifdef __cplusplus
}
#endif

#endif // __ISP_CPROC_H__