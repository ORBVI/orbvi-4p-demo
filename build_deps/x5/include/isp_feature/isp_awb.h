/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HBN_ISP_AWB_H__
#define __HBN_ISP_AWB_H__

#ifdef __cplusplus
extern "C" {
#endif

/* AWB Gray Perference */
#define HBN_ISP_AWB_LIGHT_LEVEL 18	/**< AWB light level */
#define HBN_ISP_ILLUPROFILE_NUM 10	/**< The number of illumination profiles */
#define HBN_ISP_AWB_PREFER_GAIN_MIN 0u
#define HBN_ISP_AWB_PREFER_GAIN_MAX 512u

typedef struct hbn_isp_awb_gray_preference_attr_s {
	uint32_t enable;					/**< Grayscale preference enable */
	float brightness_level[HBN_ISP_AWB_LIGHT_LEVEL];	/**< Brightness level */
	uint16_t gray_rgain[HBN_ISP_AWB_LIGHT_LEVEL];		/**< The grayscale of red channel */
	uint16_t gray_bgain[HBN_ISP_AWB_LIGHT_LEVEL];		/**< The grayscale of green channel */
} hbn_isp_awb_gray_preference_attr_t;

typedef struct hbn_isp_awb_preference_attr_s {
	hbn_isp_awb_gray_preference_attr_t gray_preference[HBN_ISP_ILLUPROFILE_NUM];	/**< Gray scale preference */
} hbn_isp_awb_preference_attr_t;

typedef struct hbn_isp_awb_gain_s {
	float	rgain;
	float	grgain;
	float	gbgain;
	float	bgain;
} hbn_isp_awb_gain_t;

typedef struct hbn_isp_awb_ctgain_s {
	uint16_t		ct;
	int16_t			shift;
	hbn_isp_awb_gain_t	gain;
} hbn_isp_awb_ctgain_t;

#ifdef __cplusplus
}
#endif

#endif	//__HBN_ISP_AWB_H__