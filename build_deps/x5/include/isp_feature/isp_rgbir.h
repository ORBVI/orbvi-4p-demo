#ifndef __ISP_RGBIR_H__
#define __ISP_RGBIR_H__

#include "isp_common.h"

#define CAMDEV_RGBIR_CC_MATRIX_SIZE 12
#define CAMDEV_RGBIR_CHANNEL_NUM 4

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************/
/**
 * @briefrgbir configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_rgbir_attr_s {
    float32_t cc_matrix[CAMDEV_RGBIR_CC_MATRIX_SIZE];
    uint16_t dpcc_mid_th[CAMDEV_RGBIR_CHANNEL_NUM];
    uint16_t dpcc_th[CAMDEV_RGBIR_CHANNEL_NUM];
    uint32_t ir_threshold;
    uint32_t l_threshold;
} hbn_isp_rgbir_attr_t;

#ifdef __cplusplus
}
#endif

#endif // __ISP_RGBIR_H__
