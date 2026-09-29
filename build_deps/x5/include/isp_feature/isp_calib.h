/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HBN_ISP_CALIB_H__
#define __HBN_ISP_CALIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/* awb calib */
#define HBN_ISP_AWB_CCM_NUM                  9U
#define HBN_ISP_AWB_CCM_OFFSET_NUM           3U
#define HBN_ISP_AWB_FADE_NUM                 16U
#define HBN_ISP_AWB_GAINDIST_NUM             16U
#define HBN_ISP_AWB_COMPONENTGAIN_NUM        4U
#define HBN_ISP_AWB_PROFILE_NAME             20U
#define HBN_ISP_AWB_ILLUPROFILE_NUM          10U

typedef struct hbn_isp_awb_calib_centerline_s {
	float rg;
	float bg;
	float d;
} hbn_isp_awb_calib_centerline_t;

typedef struct hbn_isp_awb_calib_ct_curve_s {
	float ct_a;
	float ct_b;
	float ct_c;
} hbn_isp_awb_calib_ct_curve_t;

typedef struct hbn_isp_awb_calib_fade_s {
	float fade[HBN_ISP_AWB_FADE_NUM];
	float gain_dist[HBN_ISP_AWB_GAINDIST_NUM];
	int32_t fade_point;	// Reserved
} hbn_isp_awb_calib_fade_t;

typedef struct hbn_isp_awb_calib_illu_profile_s {
	char name[HBN_ISP_AWB_PROFILE_NAME];
	uint8_t id;		// Reserved
	int32_t door_type;	// Reserved
	int32_t awb_type;	// Reserved
	float ccm_coeff[HBN_ISP_AWB_CCM_NUM];			// Reserved
	float ccm_offset[HBN_ISP_AWB_CCM_OFFSET_NUM];		// Reserved
	float manual_wb[HBN_ISP_AWB_COMPONENTGAIN_NUM];
} hbn_isp_awb_calib_illu_profile_t;

typedef struct hbn_isp_awb_calib_iir_s {
	float iir_add;
	float iir_sub;
	float iir_threshold;
	float iir_min;
	float iir_max;
	float iir_coef_init;
	int32_t filter_item_number;
} hbn_isp_awb_calib_iir_t;

typedef struct hbn_isp_awb_calib_s {
	float rg_proj_indoor_min;
	float rg_proj_max;
	float k_factor;
	hbn_isp_awb_calib_centerline_t center_line;	// The perpendicular line of fitting curve
	hbn_isp_awb_calib_ct_curve_t ctcurve;		// Reserved
	hbn_isp_awb_calib_fade_t fade1, fade2;		// The distance of white point to fitting curve
	hbn_isp_awb_calib_illu_profile_t illu_profiles[HBN_ISP_AWB_ILLUPROFILE_NUM];
	hbn_isp_awb_calib_iir_t iir;			// The coefficient of iir for awb damping
	int32_t illu_profile_number;
} hbn_isp_awb_calib_t;

/* lsc calib */
#define HBN_ISP_LSC_MATRIX_POINT_NUM 33
#define HBN_ISP_LSC_RAW_CHANNEL_NUM 4
#define HBN_ISP_CALIB_PROFILES_NUM 10

typedef char hbn_isp_lsc_profile_name_t[20U];
typedef char hbn_isp_resolution_name_t[50U];
typedef char hbn_isp_illumination_name_t[20U];

typedef struct hbn_isp_lsc_point_matrix_s
{
	uint16_t coeff[(HBN_ISP_LSC_MATRIX_POINT_NUM) * (HBN_ISP_LSC_MATRIX_POINT_NUM)];
} hbn_isp_lsc_point_matrix_t;

typedef struct hbn_isp_lsc_calib_profiles_s
{
	hbn_isp_lsc_profile_name_t name;
	hbn_isp_resolution_name_t resolution;
	hbn_isp_illumination_name_t illumination;
	hbn_isp_lsc_point_matrix_t matrixs[HBN_ISP_LSC_RAW_CHANNEL_NUM];
} hbn_isp_lsc_calib_profiles_t;

typedef struct hbn_isp_lsc_calib_inter_s
{
	hbn_isp_lsc_profile_name_t name;
	hbn_isp_resolution_name_t resolution;
	hbn_isp_illumination_name_t illumination;
	hbn_isp_lsc_point_matrix_t matrixs[HBN_ISP_LSC_RAW_CHANNEL_NUM];
	uint32_t id;
	int32_t valid_num;
} hbn_isp_lsc_calib_inter_t;

typedef struct hbn_isp_lsc_calib_s
{
	hbn_isp_lsc_calib_profiles_t calib_profiles[HBN_ISP_CALIB_PROFILES_NUM];
	int32_t valid_num;	// Get only
} hbn_isp_lsc_calib_t;


#ifdef __cplusplus
}
#endif

#endif	//__HBN_ISP_CALIB_H__
 