/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HBN_ISP_BYPASS_H__
#define __HBN_ISP_BYPASS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum enum_isp_module_version {
	HBN_ISP_MODULE_V0 = 0,
	HBN_ISP_MODULE_V1,
	HBN_ISP_MODULE_BUTT
} hbn_isp_module_version_e;

typedef union tag_isp_module_ctrl_u {
	uint32_t u32Key;
	struct {
		uint32_t bit_compand: 1;	/* RW;[0] */
		uint32_t bit_lsc: 1;		/* RW;[1] */
		uint32_t bit_dg: 1;		/* RW;[2] */
		uint32_t bit_wdr: 1;		/* RW;[3] */
		uint32_t bit_ge: 1;		/* RW;[4] */
		uint32_t bit_dpcc: 1;		/* RW;[5] */
		uint32_t bit_2dnr: 1;		/* RW;[6] */
		uint32_t bit_3dnr: 1;		/* RW;[7] */
		uint32_t bit_demosaic: 1;	/* RW;[8] */
		uint32_t bit_ccm: 1;		/* RW;[9] */
		uint32_t bit_gamma: 1;		/* RW;[10] */
		uint32_t bit_ee: 1;		/* RW;[11] */
		uint32_t bit_cproc: 1;		/* RW;[12] */
		uint32_t bit_cnr: 1;		/* RW;[13] */
	};
} isp_module_ctrl_u;

typedef struct isp_module_ctrl_s {
	hbn_isp_module_version_e version;
	isp_module_ctrl_u module;
} hbn_isp_module_ctrl_t;


#ifdef __cplusplus
}
#endif

#endif	//__HBN_ISP_BYPASS_H__
