/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef X5_ISP_CFG_H_
#define X5_ISP_CFG_H_
#include "hbn_api.h"
#include "cam_def.h"

enum isp_chn_type {
	ISP_FRAME_DATA,
	ISP_PDAF_DATA,
	ISP_TYPE_INVALID,
};

enum isp_ochn_channel_type_e {
	ISP_MAIN_FRAME = 0,
	ISP_MAIN_RAW_FRAME = 2,
	ISP_CHN_MAX,
};

typedef enum isp_sensor_mode_e {
	ISP_NORMAL_M = 0,
	ISP_DOL2_M = 1,
	ISP_PWL_M = 2,
	ISP_INVALID_MOD,
} isp_sensor_mode_t;

typedef enum enum_mcm_sched_mode_e {
	ROUND_ROBIN = 0,
	FIXED_SEQUENCE,
	FIFO,
} sched_mode_e;

typedef enum enum_input_mode_e {
	PASSTHROUGH_MODE = 0,
	MCM_MODE,
	DDR_MODE,
} input_mode_e;

typedef struct isp_attr_s {
	uint32_t input_mode;
	uint32_t sched_mode;
	uint32_t tile_mode;
	uint32_t af_mode;
	uint32_t pd_format;
	uint32_t pd_width;
	uint32_t pd_height;
	isp_sensor_mode_t sensor_mode;
	common_rect_t crop;
} isp_attr_t;

typedef struct isp_ichn_attr_s {
	cam_bool_e tpg_en;
	uint32_t width;
	uint32_t height;
	frame_format_e fmt;
	uint32_t bit_width;
} isp_ichn_attr_t;

typedef struct isp_ochn_attr_s {
	cam_bool_e ddr_en;
	common_rect_t out;
	frame_format_e fmt;
	uint32_t bit_width;
} isp_ochn_attr_t;

typedef struct isp_cfg_s {
	isp_attr_t isp_attr;
	isp_ichn_attr_t ichn_attr;
	isp_ochn_attr_t ochn_attr;
	isp_ochn_attr_t ochn_raw_attr;
} isp_cfg_t;

int32_t isp_node_parser_config(const void *root, isp_cfg_t *cfg);

#endif // X5_ISP_CFG_H_
