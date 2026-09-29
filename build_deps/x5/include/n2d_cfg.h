/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef X5_VIO_N2D_H_
#define X5_VIO_N2D_H_

#include "hbn_api.h"

#define N2D_CH_MAX 5
#define N2D_IN_MAX 4
#define TS_MAX_GAP 2000

typedef enum n2d_command {
	/* 0 */ scale,
	/* 1 */ overlay,
	/* 2 */ stitch,
	/* 3 */ csc,
	/* 4 */ rotate
} n2d_command_t;

typedef struct n2d_config {
	uint64_t config_addr;              //n2d config address
	uint32_t config_size;              //n2d config size in 32bit
	uint32_t ninputs;                  //n2d number of inputs
	uint32_t input_width[N2D_IN_MAX];  //n2d input width resolution
	uint32_t input_height[N2D_IN_MAX]; //n2d input height resolution
	uint32_t input_stride[N2D_IN_MAX]; //n2d input stride (pixel)
	uint32_t input_format;             //n2d input format
	uint32_t output_width;             //n2d output width resolution
	uint32_t output_height;            //n2d output height resolution
	uint32_t output_stride;            //n2d output stride (pixel)
	uint32_t output_format;            //n2d output format
	uint8_t div_width;                 //use in dividing UV dimensions; actually a shift right
	uint8_t div_height;                //use in dividing UV dimensions; actually a shift right
	uint32_t total_planes;
	uint8_t sequential_mode; //sequential processing
	uint64_t in_buffer_addr[N2D_IN_MAX][HBN_VIO_BUFFER_MAX_PLANES];
	uint64_t out_buffer_addr[HBN_VIO_BUFFER_MAX_PLANES];
	uint32_t overlay_x;
	uint32_t overlay_y;
	n2d_command_t command;
	uint32_t rotation;
	uint32_t crop_x;
	uint32_t crop_y;
	uint32_t crop_width;
	uint32_t crop_height;
} n2d_config_t;

int32_t n2d_node_parser_config(const void *root, n2d_config_t *cfg);

#endif
