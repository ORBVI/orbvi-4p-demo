/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HBN_ISP_API_H__
#define __HBN_ISP_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hbn_api.h"
#include "./isp_feature/isp_lsc.h"
#include "./isp_feature/isp_awb.h"
#include "./isp_feature/isp_wdr.h"
#include "./isp_feature/isp_gamma.h"
#include "./isp_feature/isp_ccm.h"
#include "./isp_feature/isp_demosaic.h"
#include "./isp_feature/isp_ee.h"
#include "./isp_feature/isp_cproc.h"
#include "./isp_feature/isp_dpcc.h"
#include "./isp_feature/isp_bypass.h"
#include "./isp_feature/isp_pattern.h"
#include "./isp_feature/isp_calib.h"
#include "./isp_feature/isp_af.h"
#include "./isp_feature/isp_hist.h"
#include "./isp_feature/isp_rgbir.h"
#include "isp_common.h"

/* common define */

typedef struct isp_param_range {
	float min;
	float max;
} hbn_isp_param_range_t;

typedef struct isp_zone_weight_s {
	uint32_t x;
	uint32_t y;
	uint32_t h;
	uint32_t w;
	float weight;
} hbn_isp_zone_weight_t;

typedef struct hbn_isp_roi_s {
	hbn_windows_t window;	/**< ROI window */
	float weight;	/**< Weight */
} hbn_isp_roi_t;

/* exposure attribute */
typedef enum enum_isp_exposure_version {
	HBN_ISP_EXP_V0 = 0,
	HBN_ISP_EXP_V1,
	HBN_ISP_EXP_V2,
} hbn_isp_exposure_version_e;

typedef enum enum_isp_auto_exposure_mode {
	HBN_ISP_AUTO_EXP_MODE_ADAPTIVE = 0,
	HBN_ISP_AUTO_EXP_MODE_FIX,
} hbn_isp_auto_exposure_mode_e;

typedef struct isp_exposure_auto_attr_s {
	hbn_isp_param_range_t exp_time_range;	// 使用s单位
	hbn_isp_param_range_t again_range;
	hbn_isp_param_range_t dgain_range;
	hbn_isp_param_range_t isp_dgain_range;
	float speed_over;			// 亮到暗速度
	float speed_under; 			// 暗到亮速度
	float dampover_gain;			// 加速收敛控制参数
	float dampover_ratio;			// 加速收敛控制参数
	float dampunder_gain;			// 加速收敛控制参数
	float dampunder_ratio;			// 加速收敛控制参数
	float tolerance;			// 偏差容忍度
	float target;				// 目标亮度值
	uint32_t anti_flicker_status;		// 抗频闪状态
	float flicker_freq;			// flicker 频率
	hbn_isp_auto_exposure_mode_e mode;
} hbn_isp_exposure_auto_attr_t;

typedef struct isp_exposure_manual_attr_s {
	float exp_time;		// 使用s单位
	float again;
	float dgain;
	float ispgain;
	float ae_exp;
	uint32_t cur_lux;	// 环境照度
	uint32_t frame_id;
	uint64_t timestamps;
} hbn_isp_exposure_manual_attr_t;

typedef struct hbn_isp_exposure_attr_s {
	hbn_isp_exposure_version_e version; //版本号
	uint32_t lock_state; // 是否收敛
	hbn_isp_mode_e mode;
	hbn_isp_exposure_auto_attr_t auto_attr;
	hbn_isp_exposure_manual_attr_t manual_attr;
} hbn_isp_exposure_attr_t;

/* HDR exposure attribute */
typedef enum enum_isp_hdr_exposure_version {
	HBN_ISP_HDR_EXP_V0 = 0,
	HBN_ISP_HDR_EXP_V1,
	HBN_ISP_HDR_EXP_V2,
} hbn_isp_hdr_exposure_version_e;

typedef struct isp_hdr_exposure_auto_attr_s {
	float exp_ratio[4];
	hbn_isp_param_range_t exp_ratio_range;
} hbn_isp_hdr_exposure_auto_attr_t;

typedef struct isp_hdr_exposure_manual_attr_s {
	float exp_time; // 使用s单位
	float again;
	float dgain;
	float ispgain;
	uint32_t ae_exp;
} hbn_isp_hdr_exposure_manual_attr_t;

typedef struct isp_hdr_exposure_attr_s {
	hbn_isp_hdr_exposure_version_e version;
	hbn_isp_mode_e mode;
	hbn_isp_hdr_exposure_auto_attr_t auto_attr;
	hbn_isp_hdr_exposure_manual_attr_t manual_attr;
} hbn_isp_hdr_exposure_attr_t;

/* AWB attribute */
typedef enum enum_isp_awb_version {
	HBN_ISP_WB_V0 = 0,
	HBN_ISP_WB_V1,
} hbn_isp_awb_version_e;

/* AWB parameter */
typedef struct hbn_isp_awb_auto_attr_s {
	uint32_t use_damping;	// 启用收敛
	uint32_t use_manual_damp_coff;	// 使用固定的收敛系数
	float manual_damp_coff;	// 收敛系数
	float lock_tolerance;	// 进入lock状态的tolerance
	float unlock_tolerance;	// 进入unlock状态的tolerance
	uint32_t rg_strength;  // r通道强度（设置白平衡偏好r通道）
	uint32_t bg_strength;  // b通道强度（设置白平衡偏好b通道）
	hbn_isp_awb_gain_t gain;
	uint32_t temper;
} hbn_isp_awb_auto_attr_t;

typedef struct hbn_isp_awb_manual_attr_s {
	hbn_isp_awb_gain_t gain;
	uint32_t temper;	// 预留
} hbn_isp_awb_manual_attr_t;

typedef struct hbn_isp_awb_attr_s {
	hbn_isp_awb_version_e version;
	uint32_t lock_state; // 是否收敛
	hbn_isp_mode_e mode;
	hbn_isp_awb_auto_attr_t auto_attr;
	hbn_isp_awb_manual_attr_t manual_attr;
} hbn_isp_awb_attr_t;

/* color process attribute */
typedef enum enum_isp_color_process_version {
	HBN_ISP_COLOR_PROCESS_V0 = 0,
	HBN_ISP_COLOR_PROCESS_V1,
} hbn_isp_color_process_version_e;

typedef struct hbn_isp_color_process_auto_attr_s {
	uint32_t total_size;
	float gain[20];
	float bright[20];
	float contrast[20];
	float saturation[20];
	float hue[20];
} hbn_isp_color_process_auto_attr_t;

typedef struct hbn_isp_color_process_manual_attr_s {
	uint32_t total_size;
	float bright;
	float contrast;
	float saturation;
	float hue;
} hbn_isp_color_process_manual_attr_t;

typedef struct hbn_isp_color_process_attr_s {
	hbn_isp_color_process_version_e version;
	hbn_isp_mode_e mode;
	hbn_isp_color_process_auto_attr_t auto_attr;
	hbn_isp_color_process_manual_attr_t manual_attr;
} hbn_isp_color_process_attr_t;

/* AE 1024-zone weight */
#define HBN_ISP_AE_ZONE_WEIGHT_MIN 0.0f
#define HBN_ISP_AE_ZONE_WEIGHT_MAX 255.0f
#define HBN_ISP_AE_ZONE_GRID_NUM 32
#define HBN_ISP_AE_ZONE_GRID_ITEMS (HBN_ISP_AE_ZONE_GRID_NUM * HBN_ISP_AE_ZONE_GRID_NUM)  /**< number of grid items */
#define HBN_ISP_GRID_ITEMS HBN_ISP_AE_ZONE_GRID_ITEMS

typedef enum enum_isp_ae_zone_weight_version {
	HBN_ISP_AE_ZONE_WEIGHT_A = 0,
	HBN_ISP_AE_ZONE_WEIGHT_B,
} hbn_isp_ae_zone_weight_version_e;

typedef struct hbn_isp_ae_zone_weight_attr_s {
	hbn_isp_ae_zone_weight_version_e version;
	hbn_isp_zone_weight_t weight[HBN_ISP_AE_ZONE_GRID_ITEMS];
} hbn_isp_ae_zone_weight_attr_t;

/* AF 225-zone weight */
typedef enum enum_isp_af_zone_weight_version_e {
	HBN_ISP_AF_ZONE_WEIGHT_A = 0,
	HBN_ISP_AF_ZONE_WEIGHT_B,
} hbn_isp_af_zone_weight_version_e;

typedef struct isp_af_zone_weight_func_a_attr_s {
	uint32_t total_size;
	hbn_isp_zone_weight_t weight[15 * 15];
} isp_af_zone_weight_func_a_attr_t;

typedef struct hbn_isp_af_zone_weight_attr_s {
	hbn_isp_af_zone_weight_version_e version;
	union {
		isp_af_zone_weight_func_a_attr_t func_a_attr;
	} attr;
} hbn_isp_af_zone_weight_attr_t;

/* statistics */
#define HBN_ISP_PIXEL_CHANNEL 4   /**< number of pixel channel */
#define HBN_ISP_AFM_BLOCK_NUM 225

typedef enum enum_isp_exp_datatype_e {
	ISP_EXP_8BIT_DATA = 0,	/**< EXP 8 bit data */
	ISP_EXP_16BIT_DATA = 1,	/**< EXP 16 bit data */
	ISP_EXP_24BIT_DATA = 2,	/**< EXP 24 bit data */
	ISP_BIT_DATA_MAX
} hbn_isp_exp_datatype_t;

typedef struct hbn_isp_ae_statistics_s {
	uint32_t expStat[HBN_ISP_AE_ZONE_GRID_ITEMS * HBN_ISP_PIXEL_CHANNEL];
	uint32_t datatype;
	uint32_t frame_id;
	uint64_t timestamps;
} hbn_isp_ae_statistics_t;

typedef struct hbn_isp_awb_statistics_s {
	uint32_t awbStat[HBN_ISP_AE_ZONE_GRID_ITEMS * HBN_ISP_PIXEL_CHANNEL];
	uint32_t datatype;
	uint32_t frame_id;
	uint64_t timestamps;
} hbn_isp_awb_statistics_t;

typedef struct hbn_isp_af_statistics_s {
	uint32_t sharpnessLowPass[HBN_ISP_AFM_BLOCK_NUM];
	uint32_t sharpnessHighPass[HBN_ISP_AFM_BLOCK_NUM];
	uint32_t histLowData[HBN_ISP_AFM_BLOCK_NUM];
	uint32_t histHighData[HBN_ISP_AFM_BLOCK_NUM];
	uint32_t frame_id;	// 当前对应frame id(备用)
} hbn_isp_af_statistics_t;

/* exposure table */
#define CAMDEV_AE_EXP_TABLE_NUM	8
#define HBN_ISP_EXP_TABLE_NUM	8

typedef struct hbn_isp_table_s {
	float exposure_time;	/**< AE exposure time */
	float again;		/**< AE simulated again */
	float dgain;		/**< AE digital gain */
	float isp_gain;		/**< AE isp gain */
} hbn_isp_table_t;

typedef struct hbn_isp_exposure_table_s {
	hbn_isp_table_t	exp_table[HBN_ISP_EXP_TABLE_NUM];	/**< Exposure table */
	uint8_t valid_num;	/**< The valid number of exposure table */
} hbn_isp_exposure_table_t;

/* exposure roi */
#define HBN_ISP_ROI_WINDOWS_MAX 25
#define HBN_ISP_AE_ROI_WIN_WEIGHT_MIN 0.0f
#define HBN_ISP_AE_ROI_WIN_WEIGHT_MAX 255.0f

typedef struct hbn_isp_exposure_roi_s {
	uint32_t roi_num;			/**< Number of ROI window */
	float roi_weight;			/**< The weight of ROI; */
	hbn_isp_roi_t roi_window[HBN_ISP_ROI_WINDOWS_MAX];	/**< ROI windows */
} hbn_isp_exposure_roi_t;

/* 2dnr attr */
#define HBN_ISP_2DNR_CURVE_SIZE 12
#define HBN_ISP_2DNR_MOTION_SIZE 2
#define HBN_ISP_2DNR_STATIC_X_NUM 2
#define HBN_ISP_2DNR_STATIC_Y_NUM 3
#define HBN_ISP_2DNR_MOVING_X_NUM 2
#define HBN_ISP_2DNR_MOVING_Y_NUM 3
#define HBN_ISP_2DNR_SIGMA_NUM 3

typedef struct hbn_isp_2dnr_curve_s {
	uint16_t ary_x[HBN_ISP_2DNR_CURVE_SIZE];	/**< Luma curve in X axis */
	uint16_t ary_y[HBN_ISP_2DNR_CURVE_SIZE];	/**< Luma curve in Y axis */
	uint16_t ary_px[HBN_ISP_2DNR_CURVE_SIZE];	/**< Luma curve of delta X */
	uint32_t interp_mode;				/**< 2DNR internal mode */
} hbn_isp_2dnr_curve_t;

typedef struct hbn_isp_2dnr_motion_config_s {
	uint16_t motion_anchor_x[HBN_ISP_2DNR_MOTION_SIZE];	/**< Motion anchor in X axis */
	hbn_isp_2dnr_curve_t curve_cfg;				/**< 2DNR curve configuration parameters */
} hbn_isp_2dnr_motion_config_t;

typedef struct hbn_isp_2dnr_manual_attr_s {
	float blend_static;		/**< Weight of spatial NR result in final output for static pixels. */
	float blend_motion;		/**< Weight of spatial NR result in final output for 100% moving pixels. */
	float blend_slope;		/**< Merge slope. Larger values mean more NLM image weight. */
	float vst_factor;		/**< VST factor */
	float sigma_scale[HBN_ISP_2DNR_SIGMA_NUM];	/**< The scale of sigma */
	float sigma_factor_mul[HBN_ISP_2DNR_SIGMA_NUM];	/**< Sigma factor multiplication */
	uint16_t sigma_factor_motion_max;		/**< Maximum sigma factor motion */
	uint16_t sigma_factor_motion_min;		/**< Minimum sigma factor motion */
	uint16_t sigma_offset;				/**< Sigma square */
	uint16_t static_detail_thresh[HBN_ISP_2DNR_STATIC_X_NUM][HBN_ISP_2DNR_STATIC_Y_NUM];		/**< Static detail threshold */
	uint16_t static_detail_boost_thresh[HBN_ISP_2DNR_STATIC_X_NUM][HBN_ISP_2DNR_STATIC_Y_NUM];	/**< Static detail boost threshold */
	float static_detail_boost[HBN_ISP_2DNR_STATIC_X_NUM][HBN_ISP_2DNR_STATIC_Y_NUM];		/**< Static detail boost */
	uint16_t static_detail_clip_thresh[HBN_ISP_2DNR_STATIC_X_NUM][HBN_ISP_2DNR_STATIC_Y_NUM];	/**< Static detail clip threshold */
	uint16_t moving_detail_thresh[HBN_ISP_2DNR_MOVING_X_NUM][HBN_ISP_2DNR_MOVING_Y_NUM];		/**< Moving detail threshold */
	uint16_t moving_detail_boost_thresh[HBN_ISP_2DNR_MOVING_X_NUM][HBN_ISP_2DNR_MOVING_Y_NUM];	/**< Moving detail boost threshold */
	float moving_detail_boost[HBN_ISP_2DNR_MOVING_X_NUM][HBN_ISP_2DNR_MOVING_Y_NUM];		/**< Moving detail boost */
	uint16_t moving_detail_clip_thresh[HBN_ISP_2DNR_MOVING_X_NUM][HBN_ISP_2DNR_MOVING_Y_NUM];	/**< Moving detail clip threshold */
	float static_factor[HBN_ISP_2DNR_SIGMA_NUM];	/**< Static factor */
	hbn_isp_2dnr_curve_t luma_curve_cfg;		/**< Luma curve configuration*/
	hbn_isp_2dnr_curve_t lsc_comp_curve_cfg;	/**< LSC comparison curve configuration*/
	hbn_isp_2dnr_motion_config_t motion_cfg;	/**< Motion configuration*/
} hbn_isp_2dnr_manual_attr_t;

typedef struct hbn_isp_2dnr_auto_attr_s {
	uint8_t auto_level;			/**< The auto level */
	float gain[HBN_ISP_AUTO_LEVEL_MAX];	/**< 2DNR gain */
	float vst_factor[HBN_ISP_AUTO_LEVEL_MAX];	/**< VST factor */
	float blend_static[HBN_ISP_AUTO_LEVEL_MAX];	/**< Weight of spatial NR result in final output for static pixels. */
	float blend_motion[HBN_ISP_AUTO_LEVEL_MAX];	/**< Weight of spatial NR result in final output for 100% moving pixels. */
	float blend_slope[HBN_ISP_AUTO_LEVEL_MAX];	/**< Merge slope. Larger values mean more NLM image weight. */
	uint16_t sigma_offset[HBN_ISP_AUTO_LEVEL_MAX];	/**< Sigma offset */
	uint16_t luma_curve_y[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_CURVE_SIZE];		/**< Luma curve in Y axis */
	uint16_t lsc_comp_curve_y[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_CURVE_SIZE];	/**< LSC comparison curve in Y axis */
	uint16_t motion_fac_curve_y[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_CURVE_SIZE];	/**< Motion factor curve in Y axis */
	uint16_t motion_anchor_x[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_MOTION_SIZE];	/**< Motion anchor in X axis */
	uint16_t static_detail_thresh[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_STATIC_X_NUM][HBN_ISP_2DNR_STATIC_Y_NUM];		/**< Static detail threshold */
	uint16_t static_detail_boost_thresh[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_STATIC_X_NUM][HBN_ISP_2DNR_STATIC_Y_NUM];	/**< Static detail boost threshold */
	float static_detail_boost[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_STATIC_X_NUM][HBN_ISP_2DNR_STATIC_Y_NUM];		/**< Static detail boost */
	uint16_t static_detail_clip_thresh[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_STATIC_X_NUM][HBN_ISP_2DNR_STATIC_Y_NUM];	/**< Static detail clip threshold */
	uint16_t moving_detail_thresh[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_MOVING_X_NUM][HBN_ISP_2DNR_MOVING_Y_NUM];		/**< Moving detail threshold */
	uint16_t moving_detail_boost_thresh[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_MOVING_X_NUM][HBN_ISP_2DNR_MOVING_Y_NUM];	/**< Moving detail boost threshold */
	float moving_detail_boost[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_MOVING_X_NUM][HBN_ISP_2DNR_MOVING_Y_NUM];		/**< Moving detail boost */
	uint16_t moving_detail_clip_thresh[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_MOVING_X_NUM][HBN_ISP_2DNR_MOVING_Y_NUM];	/**< Moving detail clip threshold */
	float sigma_scale[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_SIGMA_NUM];		/**< The scale of sigma */
	float static_factor[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_SIGMA_NUM];		/**< Static factor */
	float sigma_factor_mul[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_2DNR_SIGMA_NUM];		/**< Sigma factor multiplication */
	uint16_t sigma_factor_motion_max[HBN_ISP_AUTO_LEVEL_MAX];			/**< Maximum sigma factor motion */
} hbn_isp_2dnr_auto_attr_t;

typedef struct hbn_isp_2dnr_attr_s {
	hbn_isp_mode_e mode;			/**< The run mode */
	hbn_isp_2dnr_manual_attr_t manual_attr;	/**< 2DNR manual configuration*/
	hbn_isp_2dnr_auto_attr_t auto_attr;	/**< 2DNR auto configuration*/
} hbn_isp_2dnr_attr_t;

/* 3dnr attr */
#define HBN_ISP_3DNR_BLS_EXP_NUM 4		/**< The number of BLS explosure */
#define HBN_ISP_3DNR_THR_LUMA_CURVE_NUM 12	/**< The number of luma curve threshold */

typedef enum hbn_isp_3dnr_range_dilate_e {
	HBN_ISP_3DNR_RANGE_3 = 3,    /**< Dilate range:3 */
	HBN_ISP_3DNR_RANGE_4 = 6,    /**< Dilate range:6 */
} hbn_isp_3dnr_range_dilate_t;

typedef struct hbn_isp_3dnr_noise_model_s {
	uint8_t input_bits;		/**< Input bits */
	uint16_t fix_curve_start;	/**< Start to fix curve */
	double noisemodel_a;		/**< Noise model A */
	double noisemodel_b;		/**< Noise model B */
	uint32_t bls_exp[HBN_ISP_3DNR_BLS_EXP_NUM];	/**< BLS exposure */
} hbn_isp_3dnr_noise_model_t;

typedef struct hbn_isp_3dnr_manual_attr_s {
	double vst_factor;	/**< VST factor */
	uint8_t tnr_strength;	/**< The threshold of strength */
	uint8_t tnr_strength2;	/**< The threshold of strength2 */
	uint8_t filter_len;	/**< IIR filter length of reference frame */
	uint8_t filter_len2;	/**< IIR filter length of motion frame */
	double motion_smooth_factor;	/**< Motion smooth factor */
	hbn_isp_3dnr_range_dilate_t range_h;	/**< Set motion detection window size in horizontal direction. */
	uint8_t sad_weight;		/**< Set weight of motion difference(SAD+mean). */
	uint32_t diff_type;		/**< The type of difference */
	uint8_t sqr_diff_factor;	/**< Square difference factor */
	uint8_t motion_smooth_lvl;	/**< Motion smooth level */
	hbn_isp_3dnr_range_dilate_t dilate_h;	/**< Set motion dilation window size in horizontal direction. */
	uint16_t noise_level;		/**< Noise calibration data */
	uint16_t thr_motion_slope;	/**< The threshold of gap between static and 100% moving. */
	uint16_t tnr_luma_curve_x[HBN_ISP_3DNR_THR_LUMA_CURVE_NUM];	/**< The threshold of luma curve in X axis */
	uint16_t tnr_luma_curve_y[HBN_ISP_3DNR_THR_LUMA_CURVE_NUM];	/**< The threshold of luma curve in Y axis */
	uint16_t tnr_motion_slop_y[HBN_ISP_3DNR_THR_LUMA_CURVE_NUM];	/**< The threshold of motion slop in Y axis */
	hbn_isp_3dnr_noise_model_t noise_cfg;				/**< 3DNR noise model configuration */
} hbn_isp_3dnr_manual_attr_t;

typedef struct hbn_isp_3dnr_auto_attr_s {
	uint8_t auto_level;	/**< 3DNR auto level */
	float nm_k;		/**< Noise model K */
	float nm_p;		/**< Noise model P */
	float gains[HBN_ISP_AUTO_LEVEL_MAX];			/**<  3DNR gains */
	uint16_t fix_curve_start[HBN_ISP_AUTO_LEVEL_MAX];	/**< Fix curve start */
	double noisemodel_a[HBN_ISP_AUTO_LEVEL_MAX];		/**<Noise model A */
	double noisemodel_b[HBN_ISP_AUTO_LEVEL_MAX];		/**<Noise model B */
	uint32_t bls_exp[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_3DNR_BLS_EXP_NUM];	/**< BLS exposure */
	uint8_t tnr_strength[HBN_ISP_AUTO_LEVEL_MAX];		/**< TNR strength */
	uint8_t tnr_strength2[HBN_ISP_AUTO_LEVEL_MAX];		/**< TNR strength2  */
	uint8_t filter_len[HBN_ISP_AUTO_LEVEL_MAX];
	uint8_t filter_len2[HBN_ISP_AUTO_LEVEL_MAX];
	double motion_smooth_factor[HBN_ISP_AUTO_LEVEL_MAX];	/**< Motion smooth factor */
	hbn_isp_3dnr_range_dilate_t range_h[HBN_ISP_AUTO_LEVEL_MAX];	/**< Set motion detection window size in horizontal direction. */
	uint8_t sad_weight[HBN_ISP_AUTO_LEVEL_MAX];		/**< Set weight of motion difference(SAD+mean). */
	uint8_t sqr_diff_factor[HBN_ISP_AUTO_LEVEL_MAX];	/**< Square difference factor */
	uint8_t motion_smooth_lvl[HBN_ISP_AUTO_LEVEL_MAX];	/**< Motion smooth level */
	uint32_t motion_dilate_en[HBN_ISP_AUTO_LEVEL_MAX];	/**< Motion dilate enable */
	hbn_isp_3dnr_range_dilate_t dilate_h[HBN_ISP_AUTO_LEVEL_MAX];		/**< Set motion dilation window size in horizontal direction. */
	uint16_t tnr_luma_curve_y[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_3DNR_THR_LUMA_CURVE_NUM];	/**< Threshold of luma curve on Y axis */
	uint16_t tnr_motion_slop_y[HBN_ISP_AUTO_LEVEL_MAX][HBN_ISP_3DNR_THR_LUMA_CURVE_NUM];	/**< Threshold of motion slop on Y axis */
} hbn_isp_3dnr_auto_attr_t;

typedef struct hbn_isp_3dnr_attr_s {
	hbn_isp_mode_e mode;			/**< The run mode */
	hbn_isp_3dnr_manual_attr_t manual_attr;	/**< 3DNR current configuration */
	hbn_isp_3dnr_auto_attr_t auto_attr;	/**< 3DNR SNR configuration */
} hbn_isp_3dnr_attr_t;

typedef struct hbn_isp_sensor_param_s {
	uint32_t lines_per_second;
	uint32_t again_max;
	uint32_t dgain_max;
	uint32_t exp_time_max;
	uint32_t exp_time_min;
} hbn_isp_sensor_param_t;

extern int32_t hbn_isp_set_module_control(hbn_vnode_handle_t vnode_fd, hbn_isp_module_ctrl_t *p_ctrl);
extern int32_t hbn_isp_get_module_control(hbn_vnode_handle_t vnode_fd, hbn_isp_module_ctrl_t *p_ctrl);
extern int32_t hbn_isp_set_exposure_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_exposure_attr_t *p_attr);
extern int32_t hbn_isp_get_exposure_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_exposure_attr_t *p_attr);
extern int32_t hbn_isp_set_hdr_exposure_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_hdr_exposure_attr_t *p_attr);
extern int32_t hbn_isp_get_hdr_exposure_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_hdr_exposure_attr_t *p_attr);
extern int32_t hbn_isp_set_awb_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_awb_attr_t *p_attr);
extern int32_t hbn_isp_get_awb_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_awb_attr_t *p_attr);
extern int32_t hbn_isp_set_color_process_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_color_process_attr_t *p_attr);
extern int32_t hbn_isp_get_color_process_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_color_process_attr_t *p_attr);
extern int32_t hbn_isp_set_ae_zone_weight_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_ae_zone_weight_attr_t *p_attr);
extern int32_t hbn_isp_get_ae_zone_weight_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_ae_zone_weight_attr_t *p_attr);
extern int32_t hbn_isp_set_af_zone_weight_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_af_zone_weight_attr_t *p_attr);
extern int32_t hbn_isp_get_af_zone_weight_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_af_zone_weight_attr_t *p_attr);
extern int32_t hbn_isp_get_ae_statistics(hbn_vnode_handle_t vnode_fd, hbn_isp_ae_statistics_t *p_data);
extern int32_t hbn_isp_get_awb_statistics(hbn_vnode_handle_t vnode_fd, hbn_isp_awb_statistics_t *p_data);
extern int32_t hbn_isp_get_af_statistics(hbn_vnode_handle_t vnode_fd, hbn_isp_af_statistics_t *p_data);
extern int32_t hbn_isp_set_exposure_table(hbn_vnode_handle_t vnode_fd, hbn_isp_exposure_table_t *p_attr);
extern int32_t hbn_isp_get_exposure_table(hbn_vnode_handle_t vnode_fd, hbn_isp_exposure_table_t *p_attr);
extern int32_t hbn_isp_set_exposure_roi(hbn_vnode_handle_t vnode_fd, hbn_isp_exposure_roi_t *p_attr);
extern int32_t hbn_isp_get_exposure_roi(hbn_vnode_handle_t vnode_fd, hbn_isp_exposure_roi_t *p_attr);
extern int32_t hbn_isp_get_lines_persecond(hbn_vnode_handle_t vnode_fd, uint32_t *lines);
extern int32_t hbn_isp_set_2dnr_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_2dnr_attr_t *p_attr);
extern int32_t hbn_isp_get_2dnr_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_2dnr_attr_t *p_attr);
extern int32_t hbn_isp_set_3dnr_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_3dnr_attr_t *p_attr);
extern int32_t hbn_isp_get_3dnr_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_3dnr_attr_t *p_attr);
extern int32_t hbn_isp_get_lsc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_lsc_attr_t *p_attr);
extern int32_t hbn_isp_set_lsc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_lsc_attr_t *p_attr);
extern int32_t hbn_isp_set_awb_preference_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_awb_preference_attr_t *p_attr);
extern int32_t hbn_isp_get_awb_preference_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_awb_preference_attr_t *p_attr);
extern int32_t hbn_isp_get_wdr_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_wdr_attr_t *p_attr);
extern int32_t hbn_isp_set_wdr_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_wdr_attr_t *p_attr);
extern int32_t hbn_isp_set_ccm_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_ccm_attr_t *p_attr);
extern int32_t hbn_isp_get_ccm_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_ccm_attr_t *p_attr);
extern int32_t hbn_isp_set_gc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_gc_attr_t *p_attr);
extern int32_t hbn_isp_get_gc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_gc_attr_t *p_attr);
extern int32_t hbn_isp_set_dmsc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_dmsc_attr_t *p_attr);
extern int32_t hbn_isp_get_dmsc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_dmsc_attr_t *p_attr);
extern int32_t hbn_isp_set_ee_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_ee_attr_t *p_attr);
extern int32_t hbn_isp_get_ee_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_ee_attr_t *p_attr);
extern int32_t hbn_isp_set_cproc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_cproc_attr_t *p_attr);
extern int32_t hbn_isp_get_cproc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_cproc_attr_t *p_attr);
extern int32_t hbn_isp_get_dpcc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_dpcc_attr_t *p_attr);
extern int32_t hbn_isp_set_dpcc_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_dpcc_attr_t *p_attr);
extern int32_t hbn_isp_set_pattern_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_pattern_t *p_attr);
extern int32_t hbn_isp_get_awb_calib_data(hbn_vnode_handle_t vnode_fd, hbn_isp_awb_calib_t *p_attr);
extern int32_t hbn_isp_set_awb_calib_data(hbn_vnode_handle_t vnode_fd, hbn_isp_awb_calib_t *p_attr);
extern int32_t hbn_isp_get_lsc_calib_data(hbn_vnode_handle_t vnode_fd, hbn_isp_lsc_calib_t *p_attr);
extern int32_t hbn_isp_set_lsc_calib_data(hbn_vnode_handle_t vnode_fd, hbn_isp_lsc_calib_t *p_attr);
extern int32_t hbn_isp_get_afm_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_afm_attr_t *p_attr);
extern int32_t hbn_isp_set_afm_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_afm_attr_t *p_attr);
extern int32_t hbn_isp_cal_gain_by_temp(hbn_vnode_handle_t vnode_fd, uint32_t color_temp,
				int32_t shift, hbn_isp_awb_gain_t *p_attr);
extern int32_t hbn_isp_get_sensor_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_sensor_param_t *p_sensor_param);
extern int32_t hbn_isp_set_hist_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_hist_t *p_attr);
extern int32_t hbn_isp_get_hist_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_hist_t *p_attr);
extern int32_t hbn_isp_get_hist_sta(hbn_vnode_handle_t vnode_fd, hbn_isp_hist_sta_t *p_attr);
extern int32_t hbn_isp_get_af_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_af_attr_t *p_attr);
extern int32_t hbn_isp_set_af_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_af_attr_t *p_attr);
extern int32_t hbn_isp_set_rgbir_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_rgbir_attr_t *p_attr);
extern int32_t hbn_isp_get_rgbir_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_rgbir_attr_t *p_attr);
extern int32_t hbn_isp_get_afmv1_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_afmv1_attr_t *p_attr);
extern int32_t hbn_isp_set_afmv1_attr(hbn_vnode_handle_t vnode_fd, hbn_isp_afmv1_attr_t *p_attr);
extern int32_t hbn_isp_get_afmv1_statistics(hbn_vnode_handle_t vnode_fd, hbn_isp_afmv1_statistics_t *p_attr);



#ifdef __cplusplus
}
#endif

#endif	//__HBN_ISP_API_H__
