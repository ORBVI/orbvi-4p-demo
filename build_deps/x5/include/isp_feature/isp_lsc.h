#ifndef __ISP_LSC_H__
#define __ISP_LSC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_common.h"

// ========== LSC ==========
// --- user define ---
#define HBN_LSC_H_BLOCK_NUM 32  /**< Maximum horizontal block size of LSC map */
#define HBN_LSC_V_BLOCK_NUM 32  /**< Maximum vertical block size of LSC map */
#define HBN_LSC_H_POINT_NUM (HBN_LSC_H_BLOCK_NUM + 1)  /**< Maximum horizontal size of LSC map */
#define HBN_LSC_V_POINT_NUM (HBN_LSC_H_BLOCK_NUM + 1)  /**< Maximum vertical size of LSC map */

#define HBN_ISP_LSC_M_MIN (1024)
#define HBN_ISP_LSC_M_MAX (16383)

#define HBN_ISP_LSC_X_MIN (10)
#define HBN_ISP_LSC_Y_MIN (8)

/*****************************************************************************/
/**
 * @brief   RAW image color channel.
 */
/*****************************************************************************/
typedef enum hbn_raw_color_ch_e {
    HBN_RAW_CHANNEL_RED     = 0,  /**< Red channel */
    HBN_RAW_CHANNEL_GREENR  = 1,  /**< GreenR channel */
    HBN_RAW_CHANNEL_GREENB  = 2,  /**< GreenB channel */
    HBN_RAW_CHANNEL_BLUE    = 3,  /**< Blue channel */
    HBN_RAW_CHANNEL_NUM
} hbn_raw_color_ch_t;


/******************************************************************************/
/**
 * @brief   LSC internal mode.
 *
 *****************************************************************************/
typedef enum hbn_lsc_inter_mode_e
{
	HBN_LSC_GAIN_INTER                    = 0,    /**< LSC internal gain */
	HBN_LSC_COLOR_TEMP_WEIGHT_INTER       = 1,    /**< LSC internal temperature weight */
	HBN_LSC_GAIN_COLOR_TEMP_WEIGHT_INTER  = 2,    /**< LSC internal color temperature weight gain */
	HBN_LSC_INTER_MAX
} hbn_lsc_inter_mode_t;

/******************************************************************************/
/**
 * @brief   LSC current configuration.
 *
 *****************************************************************************/
typedef struct hbn_lsc_manual_attr_s {
	uint16_t matrix[HBN_RAW_CHANNEL_NUM][HBN_LSC_V_POINT_NUM][HBN_LSC_H_POINT_NUM];  /**< 4 channel(r, gr, gb, b) 33*33 grids */
	uint16_t x_size[HBN_LSC_H_BLOCK_NUM];                                             /**< LSC int array for x axis size */
	uint16_t y_size[HBN_LSC_V_BLOCK_NUM/2];                                           /**< LSC int array for y axis size*/
} hbn_lsc_manual_attr_t;

/******************************************************************************/
/**
 * @brief   LSC auto configuration.
 *
 *****************************************************************************/
typedef struct hb_lsc_auto_attr_s
{
	uint8_t auto_level;    /**< Auto configuration level */
	float32_t gains[HBN_ISO_STRENGTH_NUM];    /**< LSC gains */
	float32_t damping;    /**< LSC damping value */
	float32_t strength[HBN_ISO_STRENGTH_NUM];    /**< LSC strength */
	hbn_lsc_inter_mode_t inter_mode;    /**< LSC internal mode */
} hbn_lsc_auto_attr_t;

/******************************************************************************/
/**
 * @brief   LSC configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_lsc_attr_s {
	hbn_isp_mode_e mode;
	hbn_lsc_manual_attr_t manual_attr;    /**< LSC manual configuration */
	hbn_lsc_auto_attr_t   auto_attr;      /**< LSC auto configuration */
} hbn_isp_lsc_attr_t;


#ifdef __cplusplus
}
#endif

#endif // __ISP_LSC_H__