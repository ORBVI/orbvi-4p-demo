/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef J6_VIO_GDC_H_
#define J6_VIO_GDC_H_

#include "hbn_api.h"

typedef struct gdc_attr_s {
	uint64_t config_addr;   /* gdc bin address */
	uint32_t config_size;
	uint8_t div_width;      /* use in dividing UV dimensions parameters */
	uint8_t div_height;     /* use in dividing UV dimensions parameters */
	uint32_t total_planes;
	int32_t binary_ion_id;  /* share id for config binary physical addr */
	uint64_t binary_offset; /* config binary physical addr offset */
} gdc_attr_t;

typedef struct gdc_ichn_attr_s {
	uint32_t input_width;
	uint32_t input_height;
	uint32_t input_stride;
	uint32_t n_in_one;
} gdc_ichn_attr_t;

typedef struct gdc_ochn_attr_s {
	uint32_t output_width;
	uint32_t output_height;
	uint32_t output_stride;
} gdc_ochn_attr_t;

typedef struct gdc_cfg_s {
	/**
         * @var gdc_cfg_s::input_width
         * gdc input width resolution parameters.
         * range:N/A; default: N/A
         */
	uint32_t input_width;
	/**
         * @var gdc_cfg_s::input_height
         * gdc input height resolution parameters.
         * range:N/A; default: N/A
         */
	uint32_t input_height;
	/**
         * @var gdc_cfg_s::output_width
         * gdc output width resolution parameters.
         * range:N/A; default: N/A
         */
	uint32_t output_width;
	/**
         * @var gdc_cfg_s::output_height
         * gdc output height resolution parameters.
         * range:N/A; default: N/A
         */
	uint32_t output_height;
	/**
         * @var gdc_cfg_s::buf_num
         * gdc out buffer number parameters.
         * range:N/A; default: N/A
         */
	uint32_t buf_num;
	/**
         * @var gdc_cfg_s::in_buf_noclean
         * gdc input buffer not clean parameters.
         * range:N/A; default: N/A
         */
	uint32_t in_buf_noclean;
	/**
         * @var gdc_cfg_s::in_buf_noncached
         * gdc input buffer not cached parameters.
         * range:N/A; default: N/A
         */
	uint32_t in_buf_noncached;
	/**
         * @var gdc_cfg_s::out_buf_noinvalid
         * gdc output buffer not invalid parameters.
         * range:N/A; default: N/A
         */
	uint32_t out_buf_noinvalid;
	/**
         * @var gdc_cfg_s::out_buf_noncached
         * gdc output buffer not cached parameters.
         * range:N/A; default: N/A
         */
	uint32_t out_buf_noncached;
	/**
         * @var gdc_cfg_s::gdc_pipeline
         * gdc pipline parameters.
         * range:N/A; default: N/A
         */
	uint32_t gdc_pipeline;
	/**
         * @var gdc_cfg_s::n_in_one
         * gdc split n in one parameters.
         * range:N/A; default: N/A
         */
	uint32_t n_in_one;
} gdc_cfg_t;

int32_t gdc_node_parser_config(const void *root, gdc_cfg_t *cfg);

#endif
