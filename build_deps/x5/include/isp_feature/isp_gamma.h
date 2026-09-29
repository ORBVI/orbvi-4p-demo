#ifndef __ISP_GAMMA_H__
#define __ISP_GAMMA_H__

#include "isp_common.h"

#define HBN_GC_CURVE_SIZE 64   /**< Curve size */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/**
 * @brief   GC auto configuration.
 *
 *****************************************************************************/
typedef struct hbn_gc_auto_attr_s {
    uint8_t auto_level;                                               /**< GC auto configuration level */
    float32_t gain[HBN_ISO_STRENGTH_NUM];                             /**< GC gain */
    uint16_t curve[HBN_ISO_STRENGTH_NUM][HBN_GC_CURVE_SIZE];          /**< Gamma curve */
} hbn_isp_gc_auto_attr_t;

/******************************************************************************/
/**
 * @brief   GC current configuration.
 *
 *****************************************************************************/
typedef struct hbn_gc_manual_attr_s {
    bool_t standard;                        /**< True/false: true for standard_val, false for curve */
    float32_t standard_val;                 /**< Generate uniform curve with gamma formula */
    uint16_t curve[HBN_GC_CURVE_SIZE];   /**< Gamma curve */
    bool_t user_curve_x;                      /**< User curve in X axis */
    uint32_t curve_px[HBN_GC_CURVE_SIZE];   /**< Gamma curve Px */
} hbn_isp_gc_manual_attr_t;

/******************************************************************************/
/**
 * @brief   GC configuration.
 *
 *****************************************************************************/
typedef struct hbn_gc_attr_s {
    hbn_isp_mode_e mode;                /**< GC mode configuration */
    hbn_isp_gc_auto_attr_t auto_attr;      /**< GC auto configuration */
    hbn_isp_gc_manual_attr_t manual_attr;  /**< GC manual configuration */
} hbn_isp_gc_attr_t;

#ifdef __cplusplus
}
#endif

#endif // __ISP_GAMMA_H__
