/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HBN_ISP_HIST_H__
#define __HBN_ISP_HIST_H__

#include "isp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HBN_ISP_HIST_BIN_NUM 256

typedef enum hbn_isp_hist_mode_e {
	HBN_ISP_HIST_MODE_RGB_COMBINED = 1,	/**< RGB combined histogram(Not supported in X5) */
	HBN_ISP_HIST_MODE_R            = 2,	/**< R histogram */
	HBN_ISP_HIST_MODE_G            = 3,	/**< G histogram */
	HBN_ISP_HIST_MODE_B            = 4,	/**< B histogram */
	HBN_ISP_HIST_MODE_Y            = 5,	/**< Luminance histogram */
	HBN_ISP_HIST_MODE_MAX,
} hbn_isp_hist_mode_t;

typedef struct hbn_isp_hist_s {
	hbn_isp_hist_mode_t mode;	/**< HIST mode */
	hbn_windows_t window;		/**< HIST window */
} hbn_isp_hist_t;

typedef struct hbn_isp_hist_sta_s {
	uint32_t bins[HBN_ISP_HIST_BIN_NUM];
} hbn_isp_hist_sta_t;


#ifdef __cplusplus
}
#endif

#endif	//__HBN_ISP_HIST_H__
