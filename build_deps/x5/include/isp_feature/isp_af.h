/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HBN_ISP_AF_H__
#define __HBN_ISP_AF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_common.h"

#define HBN_ISP_AFM_BLOCK_POINT_NUM 4		/**< The number of AFM block point numbers */
#define HBN_ISP_AFM_LDG_Y_NUM 3			/**< The number of AFM Level Depend Gain in Y axis */
#define HBN_ISP_AFM_LDG_X_NUM 4			/**< The number of AFM Level Depend Gain in X axis */
#define HBN_ISP_AFM_CORING_COEFF_NUM 3		/**< The number of AFM coring coefficient */
#define HBN_ISP_AFM_IIR_FILTER_NUM 3		/**< The number of AFM IIR filter */
#define HBN_ISP_AFM_IIR_FILTER_COEFF_NUM 4	/**< The number of AFM IIR filter coefficient */
#define HBN_ISP_AFM_FIR_HIGHPASS_COEFF_NUM 2	/**< The number of AFM fir highpass filter coefficient*/
#define HBN_ISP_AFM_FIR_LOWPASS_COEFF_NUM 4	/**< The number of AFM fir lowpass filter coefficient */
#define HBN_ISP_AFMV1_WINDOW_NUM 3

typedef struct hbn_isp_afm_ldg_s {
	float iir_highpass_y[HBN_ISP_AFM_LDG_Y_NUM];	/**< IIR high pass on Y axis */
	uint8_t iir_highpass_x[HBN_ISP_AFM_LDG_X_NUM];	/**< IIR high pass on X axis */
	float iir_lowpass_y[HBN_ISP_AFM_LDG_Y_NUM];	/**< IIR low pass on Y axis */
	uint8_t iir_lowpass_x[HBN_ISP_AFM_LDG_X_NUM];	/**< IIR low pass on X axis */
	float fir_highpass_y[HBN_ISP_AFM_LDG_Y_NUM];	/**< FIR high pass on Y axis */
	uint8_t fir_highpass_x[HBN_ISP_AFM_LDG_X_NUM];	/**< FIR high pass on X axis */
	float fir_lowpass_y[HBN_ISP_AFM_LDG_Y_NUM];	/**< FIR low pass on Y axis */
	uint8_t fir_lowpass_x[HBN_ISP_AFM_LDG_X_NUM];	/**< FIR low pass on X axis */
} hbn_isp_afm_ldg_t;

typedef struct hbn_isp_afm_coring_s {
	uint8_t iir_highpass[HBN_ISP_AFM_CORING_COEFF_NUM];	/**< IIR high pass */
	uint8_t iir_lowpass[HBN_ISP_AFM_CORING_COEFF_NUM];	/**< IIR low pass */
	uint8_t fir_highpass[HBN_ISP_AFM_CORING_COEFF_NUM];	/**< FIR high pass */
	uint8_t fir_lowpass[HBN_ISP_AFM_CORING_COEFF_NUM];	/**< FIR low pass */
} hbn_isp_afm_coring_t;

typedef struct hbn_isp_afm_iir_s {
	uint8_t data_shift;	/**< Data shift */
	uint8_t data_shift_v;	/**< Data shift Vetical*/
	float highpass_weight;	/**< High pass weight */
	float lowpass_weight;	/**< Low pass weight */
	float highpass_coeff[HBN_ISP_AFM_IIR_FILTER_NUM][HBN_ISP_AFM_IIR_FILTER_COEFF_NUM];  /**< High pass coefficient */
	float lowpass_coeff[HBN_ISP_AFM_IIR_FILTER_NUM][HBN_ISP_AFM_IIR_FILTER_COEFF_NUM];   /**< Low pass coefficient */
} hbn_isp_afm_iir_t;

typedef struct hbn_isp_afm_fir_s {
	uint8_t data_shift;		/**< Data shift */
	uint8_t data_shift_v;		/**< Data shift Vetical*/
	float highpass_weight;		/**< High pass weight */
	float lowpass_weight;		/**< Low pass weight */
	uint8_t highdata_shift;
	uint8_t highdata_shift_v;
	uint8_t lowdata_shift;
	uint8_t lowdata_shift_v;
	uint8_t highpass_coeff[HBN_ISP_AFM_FIR_HIGHPASS_COEFF_NUM];	/**< High pass coefficient */
	uint8_t lowpass_coeff[HBN_ISP_AFM_FIR_LOWPASS_COEFF_NUM];	/**< Low pass coefficient */
} hbn_isp_afm_fir_t;

typedef struct hbn_isp_afm_attr_s {
	uint16_t bls;					/**< BLS components */
	uint8_t hist_threshold;				/**< Histogram threshold */
	float gamma;					/**< Gamma value */
	uint16_t hw_roi[HBN_ISP_AFM_BLOCK_POINT_NUM];	/**< Hardware ROI */
	hbn_isp_afm_ldg_t ldg;				/**< Level Dependent Gain configuration */
	hbn_isp_afm_coring_t coring;			/**< Coring configuration */
	hbn_isp_afm_iir_t iir;				/**< IIR configuration */
	hbn_isp_afm_fir_t fir;				/**< FIR configuration */
} hbn_isp_afm_attr_t;

typedef struct hbn_isp_af_auto_attr_s {
	uint8_t af_mode;

	// cdaf params
	float stable_tolerance;		/**< Range: [0, 1] */
	uint8_t points_of_curve;
	uint16_t min_focal;		/**< Minimum focal point*/
	uint16_t max_focal;		/**< Maximum focal point */
	float motion_threshold;		/**< Motion threshold */
	uint8_t uphill_allowance;	/**< The range of top hill when CDAF in searching  */
	uint8_t downhill_allowance;	/**< The range of bottom hill when CDAF in searching */

	// pdaf params
	float pdconf_threshold;		/**< PDAF configuration threshold */
	float pdshift_threshold;	/**< Phase Detection shift stable threshold */
	uint8_t pdstable_count_max;	/**< Maximum focusing to which PDAF locks */
	float pdaf_unlock_threshold;	/**< PDAF unlock threshold */

	// PDAF and CDAF hybrid params
	uint8_t defocus_framenum;	/**< The number of defocus frames */
	uint8_t lossconfidence_framenum;	/**< The number of loss confidence frames */
	uint8_t accurate_focus_step;	/**< Accurate focus step */
	uint8_t accurate_focus_enable;	/**< Accurate focus enable */
} hbn_isp_af_auto_attr_t;

typedef struct hbn_isp_af_attr_s {
	hbn_isp_mode_e mode;			/**< Support manual mode only */
	uint32_t position;
	hbn_isp_af_auto_attr_t auto_attr;
} hbn_isp_af_attr_t;

typedef struct hbn_isp_afmv1_attr_s {
	uint32_t threshold;
	hbn_windows_t afm_windows[HBN_ISP_AFMV1_WINDOW_NUM];
} hbn_isp_afmv1_attr_t;

typedef struct hbn_isp_afmv1_statistics_s {
	uint32_t sharpness_a;		/**< Sharpness of window A */
	uint32_t sharpness_b;		/**< Sharpness of window B */
	uint32_t sharpness_c;		/**< Sharpness of window C */
	uint32_t luminance_a;		/**< Luminance of window A */
	uint32_t luminance_b;		/**< Luminance of window B */
	uint32_t luminance_c;		/**< Luminance of window C */
	uint32_t pixelcnt_a;		/**< Pixel counts of window A */
	uint32_t pixelcnt_b;		/**< Pixel counts of window B */
	uint32_t pixelcnt_c;		/**< Pixel counts of window C */
} hbn_isp_afmv1_statistics_t;

#ifdef __cplusplus
}
#endif

#endif	//__HBN_ISP_AF_H__
