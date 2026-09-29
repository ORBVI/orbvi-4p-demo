/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HBN_ISP_DPCC_H__
#define __HBN_ISP_DPCC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_common.h"

#define HBN_ISP_DPCC_DP_NUM 2048	/**< Maximum number of defected pixels */
#define HBN_ISP_DPCC_CHANNEL_NUM 2	/**< Channel number of defected pixels */
#define HBN_ISP_DPCC_MP_TYPE_NUM 3	/**< Method parameter type number of defected pixels */

typedef struct hbn_isp_dpcc_manual_attr_s {
	uint32_t bpt_enable;		/**< Bad pixel table enable */
	uint16_t bpt_num;		/**< Bad pixel table number */
	uint8_t bpt_out_mode;		/**< Bad pixel table output mode */
	uint16_t bpt_pos_x[HBN_ISP_DPCC_DP_NUM];	/**< Bad pixel table X position */
	uint16_t bpt_pos_y[HBN_ISP_DPCC_DP_NUM];	/**< Bad pixel table Y position */
	uint8_t line_mad_fac[HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< [0][0~2]Represents the three thresholds of Line_mad method used by the RB channel */
	/**< [1][0~2]Represents the three thresholds of Line_mad method used by the G channel */
	uint8_t line_thresh[HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< [0][0~2]Represents the three thresholds of Line_thresh method used by the RB channel */
	/**< [1][0~2]Represents the three thresholds of Line_thresh method used by the G channel */
	uint16_t methods_set[HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< [0] Represents the switch whether the first set of thresholds of the five bad point determination methods are enabled */
	/**< [1] Represents the switch whether the second set of thresholds of the five bad point determination methods are enabled */
	/**< [2] Represents the switch whether the third set of thresholds of the five bad point determination methods are enabled */
	uint8_t out_mode;  /**<Interpolation mode for correction unit */
	uint8_t pg_fac[HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< [0][0~2]Represents the three thresholds of peak Gradient check method used by the RB channel */
	/**< [1][0~2]Represents the three thresholds of peak Gradient check method used by the G channel */
	uint8_t rg_fac[HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< [0][0~2]Represents the three thresholds of Rank neighbor difference method used by the RB channel */
	/**< [1][0~2]Represents the three thresholds of Rank neighbor difference method used by the G channel */
	uint8_t rnd_offs[HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< [0][0~2]Represents the three offsets of Rand Neighbor Difference method used by the RB channel */
	/**< [1][0~2]Represents the three offsets of Rand Neighbor Difference method used by the G channel */
	uint8_t rnd_thresh[HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< [0][0~2]Represents the three thresholds of Rand Neighbor Difference method used by the RB channel */
	/**< [1][0~2]Represents the three thresholds of Rand Neighbor Difference method used by the G channel */
	uint8_t ro_limits[HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< [0][0~2]Represents the three thresholds of Rand Order method used by the RB channel */
	/**< [1][0~2]Represents the three thresholds of Rand Order method used by the G channel */
	uint8_t set_use;	/**< DPCC methods set usage for detection */
} hbn_isp_dpcc_manual_attr_t;

typedef struct hbn_isp_dpcc_auto_attr_s {
	uint8_t auto_level;			/**< The auto config level */
	float gains[HBN_ISP_AUTO_LEVEL_MAX];	/**< DPCC gains */
	uint8_t line_mad_fac[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< Line check for mean absolute difference factor */
	uint8_t line_thresh[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];
	/**< Line threshold for green or red/blue channel */
	uint16_t methods_set[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_DPCC_MP_TYPE_NUM];	/**< DPCC methods enable bits */
	uint8_t out_mode[HBN_ISP_AUTO_LEVEL_MAX];	/**< Interpolation mode for correction unit */
	uint8_t pg_fac[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];	/**< Peak of gradient factor */
	uint8_t rg_fac[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];	/**< Rank of gradient factor */
	uint8_t rnd_offs[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];	/**< Differential rank offsets for neighbor difference */
	uint8_t rnd_thresh[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];	/**< Rank neighbor difference threshold */
	uint8_t ro_limits[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_DPCC_CHANNEL_NUM][HBN_ISP_DPCC_MP_TYPE_NUM];	/**< Rank order limits */
	uint8_t set_use[HBN_ISP_AUTO_LEVEL_MAX];		/**< Enable DPCC detection */
} hbn_isp_dpcc_auto_attr_t;

typedef struct hbn_isp_dpcc_attr_s {
	hbn_isp_mode_e mode;			/**< The run mode */
	hbn_isp_dpcc_manual_attr_t manual_attr;	/**< DPCC manual configuration*/
	hbn_isp_dpcc_auto_attr_t auto_attr;	/**< DPCC auto configuration*/
} hbn_isp_dpcc_attr_t;


#ifdef __cplusplus
}
#endif

#endif	//__HBN_ISP_DPCC_H__
