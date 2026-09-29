/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef _VIO_CODEC_NODE_CFG_H_
#define _VIO_CODEC_NODE_CFG_H_

/**
 * @struct codec_cfg_s
 * @brief codec cfg parameters.
 * @NO{S09E03C02}
 */
typedef struct codec_cfg_s {
	uint32_t input_width;
	uint32_t input_stride;
	uint32_t input_height;
	uint32_t output_width;
	uint32_t output_stride;
	uint32_t output_height;
	uint32_t buf_num;
	uint32_t fb_buf_num;
	uint32_t channel_idx;
} codec_cfg_t;

typedef struct codec_ochn_attr_s {
	uint32_t  ddr_en;
} codec_ochn_attr_t;

typedef struct codec_ichn_attr_s {
	uint32_t  format;
	uint32_t  width;
	uint32_t  height;
} codec_ichn_attr_t;

/**
 * @description: Get both the git commit hash and version of libvpf.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libvpf_get_version_info(void);

#endif // _VIO_CODEC_NODE_CFG_H_
