#ifndef __ISP_CCM_H__
#define __ISP_CCM_H__

#include "isp_common.h"

#define HBN_CC_MATRIX_SIZE       9      /**< Color correction matrix size 3x3*/
#define HBN_CC_COLOR_CHANNEL_NUM 3      /**< Color channel number 3*/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/**
 * @brief   CCM manual configuration.
 *
 *****************************************************************************/
typedef struct hbn_ccm_manual_attr_s {
    float32_t cc_matrix[HBN_CC_MATRIX_SIZE];          /**< Color correction matrix coefficient*/
    float32_t cc_offset[HBN_CC_COLOR_CHANNEL_NUM];    /**< Color offset coefficient*/
} hbn_isp_ccm_manual_attr_t;

/******************************************************************************/
/**
 * @brief   CCM configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_ccm_attr_s {
    hbn_isp_ccm_manual_attr_t manual_attr;   /**< CCM manual configuration*/
    uint32_t configMode;
} hbn_isp_ccm_attr_t;


#ifdef __cplusplus
}
#endif

#endif // __ISP_CCM_H__
