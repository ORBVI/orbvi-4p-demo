/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HBN_ISP_PATTERN_H__
#define __HBN_ISP_PATTERN_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum hbn_isp_pattern_e {
	HBN_ISP_RAW_RGGB = 0,	/**< RGGB */
	HBN_ISP_RAW_GRBG,	/**< GRBG */
	HBN_ISP_RAW_GBRG,	/**< GBRG */
	HBN_ISP_RAW_BGGR,	/**< BGGR */
} hbn_isp_pattern_t;


#ifdef __cplusplus
}
#endif

#endif	//__HBN_ISP_PATTERN_H__
