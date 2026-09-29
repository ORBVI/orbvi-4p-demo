/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef __HB_VPF_INTERFACE_H__
#define __HB_VPF_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <hb_mem_mgr.h>
#include "hbn_error.h"
#include "gdc_bin_cfg.h"

#define HBN_VFLOW_PIPELINE_MAX 24u
#define HBN_VIO_BUFFER_MAX_PLANES 4u
#define HBN_META_PLANE 3u
#define HBN_METADATA_SIZE (4 * 1024) //4KB

#define HBN_LAYER_MAXIMUM 6u
#define HBN_PIPELINE_BIND_MAX 16u /**< vio pipeline max bind count  @NO{S09E05C02U}  */
#define HBN_GETFRAME_TIMEOUT 1000 //1000ms for dvb
#define HBN_MAX_VNODE_CONFIG 8

#define VIN_MODULE 0u
#define ISP_MODULE 1u
#define VSE_MODULE 2u
#define GDC_MODULE 3u
#define N2D_MODULE 4u
#define IDU_MODULE 5u
#define CODEC_MODULE 6u
#define MODULE_NUM 7u

#define AUTO_ALLOC_ID -1

typedef int64_t hbn_vnode_handle_t;
typedef int64_t hbn_vflow_handle_t;

typedef struct hbn_frame_info_s {
	uint32_t frame_id;
	uint64_t timestamps;
	uint64_t sys_timestamps;
	struct timeval tv;
	struct timeval trig_tv;
	uint32_t frame_done;
	int32_t bufferindex;
} hbn_frame_info_t;

typedef struct hbn_buf_alloc_attr_s {
	int64_t flags;
	uint32_t buffers_num;
	uint32_t is_contig;
} hbn_buf_alloc_attr_t;

typedef struct hbn_vnode_image_s {
	hbn_frame_info_t info;
	hb_mem_graphic_buf_t buffer;
	void *metadata;
} hbn_vnode_image_t;

//TODO use graphic group
typedef struct hbn_vnode_image_group_s {
	hbn_vnode_image_t image[HBN_LAYER_MAXIMUM];
	uint32_t bit_map;
	void *metadata;
} hbn_vnode_image_group_t;

typedef enum hb_vnode_type_e {
	HB_VIN,
	HB_ISP,
	HB_VSE,
	HB_GDC,
	HB_N2D,
	HB_CODEC,
	HB_OSD,
	HB_STITCH, // reserved
	HB_YNR, // reserved
	HB_PYM, // reserved
	HB_IDU, // reserved
	HB_VPU, // reserved
	HB_JPU, // reserved
	HB_VNODE_TYPE_MAX
} hb_vnode_type;

typedef enum hobot_vflow_event_type_e {
	HOBOT_vflow_EVENT_INFO_EOF,
	HOBOT_vflow_EVENT_INFO_PROCESSING_DONE,
	HOBOT_vflow_EVENT_INFO_FRAME_CAPTURE,
	HOBOT_vflow_EVENT_WARNING_CAPTURE_FRAME_DROP,
	HOBOT_vflow_EVENT_ERROR_INTERNAL_FAILURE,
	HOBOT_vflow_EVENT_ERROR_I2C_TRANSMISSION_FAILURE,
	HOBOT_vflow_EVENT_WARNING_CSI_FRAME_DISCONTINUITY,
	HOBOT_vflow_EVENT_ERROR_CSI_INPUT_STREAM_FAILURE
} hobot_vflow_event_type;

typedef enum hobot_vpf_chn_type_e {
	HOBOT_VNODE_CHN_AGGREGATE,
	HOBOT_VNODE_CHN_OTF,
	HOBOT_VNODE_CHN_IMAGE,
	HOBOT_VNODE_CHN_EMBDATA,
} hobot_vpf_chn_type;

typedef struct hbn_version_s {
	uint32_t major;
	uint32_t minor;
} hbn_version_t;

#define MAX_DS_NUM (6)
#define MAX_PRE_INT (8u)

typedef struct {
	uint32_t start_top; //ROI coordinate Y
	uint32_t start_left; //ROI coordinate X
	uint32_t region_width; //ROI width
	uint32_t region_height; //ROI height
	uint32_t wstride_uv; //
	uint32_t wstride_y; //
	uint32_t vstride;
	uint32_t step_v;
	uint32_t step_h;
	uint32_t out_width;
	uint32_t out_height;
	uint32_t phase_y_v;
	uint32_t phase_y_h;
} vio_roi_box_t;

typedef struct {
	uint32_t pixel_num_before_sol;
	uint32_t src_in_width;
	uint32_t src_in_height;
	uint32_t src_in_stride_y;
	uint32_t src_in_stride_uv;
	uint32_t suffix_hb_val;
	uint32_t prefix_hb_val;
	uint32_t suffix_vb_val;
	uint32_t prefix_vb_val;
	uint8_t bl_max_layer_en;
	uint8_t ds_roi_en;
	uint8_t ds_roi_uv_bypass;
	uint8_t ds_roi_sel[MAX_DS_NUM];
	uint8_t ds_roi_layer[MAX_DS_NUM];
	vio_roi_box_t ds_roi_info[MAX_DS_NUM];
	uint32_t pre_int_set_y[MAX_PRE_INT];
	uint32_t pre_int_set_uv[MAX_PRE_INT];
} chn_ctrl_t;

typedef struct {
	uint8_t hw_id;
	uint8_t pym_mode;
	uint8_t slot_id;
	uint8_t axi_burst_len;
	uint8_t in_linebuff_watermark;
	uint8_t out_buf_noinvalid;
	uint8_t out_buf_noncached;
	uint8_t in_buf_noclean;
	uint8_t in_buf_noncached;
	uint8_t buf_consecutive;
	uint8_t pingpong_ring;
	uint32_t output_buf_num;
	uint32_t timeout;
	uint32_t threshold_time;
	int32_t layer_num_transfer_gdc;
	int32_t layer_num_share_ynr;
	chn_ctrl_t chn_ctrl;
	uint32_t reserved[8];
	uint32_t magicNumber;
} j6_pym_cfg_t;

#define GDC_MAX_INPUT_PLANE (3u)

typedef int32_t hobot_status;

hobot_status hbn_vnode_open(hb_vnode_type vnode_type, uint32_t hw_id, int32_t ctx_id, hbn_vnode_handle_t *vnode_fd);
void hbn_vnode_close(hbn_vnode_handle_t vnode_fd);
hobot_status hbn_vnode_set_attr(hbn_vnode_handle_t vnode_fd, void *attr);
hobot_status hbn_vnode_get_attr(hbn_vnode_handle_t vnode_fd, void *attr);
hobot_status hbn_vnode_set_attr_ex(hbn_vnode_handle_t vnode_fd, void *attr);
hobot_status hbn_vnode_get_attr_ex(hbn_vnode_handle_t vnode_fd, void *attr);
hobot_status hbn_vnode_set_ochn_attr(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, void *attr);
hobot_status hbn_vnode_set_ochn_attr_ex(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, void *attr);
hobot_status hbn_vnode_get_ochn_attr(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, void *attr);
hobot_status hbn_vnode_set_ichn_attr(hbn_vnode_handle_t vnode_fd, uint32_t ichn_id, void *attr);
hobot_status hbn_vnode_set_ichn_attr_ex(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, void *attr);
hobot_status hbn_vnode_get_ichn_attr(hbn_vnode_handle_t vnode_fd, uint32_t ichn_id, void *attr);
hobot_status hbn_vnode_enable_ichn(hbn_vnode_handle_t vnode_fd, uint32_t ichn_id);
hobot_status hbn_vnode_disable_ichn(hbn_vnode_handle_t vnode_fd, uint32_t ichn_id);
hobot_status hbn_vnode_enable_ochn(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id);
hobot_status hbn_vnode_disable_ochn(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id);
hobot_status hbn_vnode_set_ochn_buf_attr(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id,
					 hbn_buf_alloc_attr_t *alloc_attr);
hobot_status hbn_vnode_start(hbn_vnode_handle_t vnode_fd);
hobot_status hbn_vnode_stop(hbn_vnode_handle_t vnode_fd);
hobot_status hbn_vnode_get_fd(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, int32_t *fd);

hobot_status hbn_vnode_getframe(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, uint32_t millisecondTimeout,
				hbn_vnode_image_t *out_img); // block function;
hobot_status hbn_vnode_getframe_cond(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, uint32_t millisecondTimeout,
				     int32_t cond_time, hbn_vnode_image_t *out_img); // block function;
hobot_status hbn_vnode_getframe_group(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, uint32_t millisecondTimeout,
				      hbn_vnode_image_group_t *out_img); // block function;
hobot_status hbn_vnode_getframe_group_cond(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, uint32_t millisecondTimeout,
					   int32_t cond_time, hbn_vnode_image_group_t *out_img); // block function;
hobot_status hbn_vnode_sendframe(hbn_vnode_handle_t vnode_fd, uint32_t ichn_id,
				 hbn_vnode_image_t *img); // 33ms block function
hobot_status hbn_vnode_sendframe_async(hbn_vnode_handle_t vnode_fd, uint32_t ichn_id,
				       hbn_vnode_image_t *img); // no block function
hobot_status hbn_vnode_releaseframe(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, hbn_vnode_image_t *img);
hobot_status hbn_vnode_releaseframe_group(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id,
					  hbn_vnode_image_group_t *img_group);
hobot_status hbn_vnode_set_output_frame(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, hbn_vnode_image_t *img);
hobot_status hbn_vnode_get_output_frame(hbn_vnode_handle_t vnode_fd, uint32_t ochn_id, hbn_vnode_image_t *img);

hobot_status hbn_vflow_create(hbn_vflow_handle_t *vflow_fd);
void hbn_vflow_destroy(hbn_vflow_handle_t vflow_fd);
hobot_status hbn_vflow_create_cfg(const char *cfg_file, hbn_vflow_handle_t *vflow_fd);
hobot_status hbn_vflow_add_vnode(hbn_vflow_handle_t vflow_fd, hbn_vnode_handle_t vnode_fd);
hobot_status hbn_vflow_bind_vnode(hbn_vflow_handle_t vflow_fd, hbn_vnode_handle_t src_vnode_fd, uint32_t out_chn,
				  hbn_vnode_handle_t dst_vnode_fd, uint32_t in_chn);
hobot_status hbn_vflow_unbind_vnode(hbn_vflow_handle_t vflow_fd, hbn_vnode_handle_t src_vnode_fd, uint32_t out_chn,
				    hbn_vnode_handle_t dst_vnode_fd, uint32_t in_chn);
hobot_status hbn_vflow_start(hbn_vflow_handle_t vflow_fd);
hobot_status hbn_vflow_stop(hbn_vflow_handle_t vflow_fd);
hobot_status hbn_vflow_pause(hbn_vflow_handle_t vflow_fd);
hobot_status hbn_vflow_resume(hbn_vflow_handle_t vflow_fd);
hobot_status hbn_vflow_get_version(hbn_version_t *version);
hbn_vnode_handle_t hbn_vflow_get_vnode_handle(hbn_vflow_handle_t vflow_fd, hb_vnode_type vnode_type, uint32_t index);

int32_t hbn_gen_gdc_bin_json(const char *layout_file, char *config_file, uint32_t **cfg_buf, uint64_t *config_size);
int32_t hbn_gen_gdc_bin(const param_t *gdc_param, const window_t *windows, uint32_t wnd_num, uint32_t **cfg_buf, uint64_t *cfg_size);
void hbn_free_gdc_bin(uint32_t *cfg_buf);

hobot_status hbn_get_codec_channel_idx(hbn_vnode_handle_t vnode_fd, int32_t encoder, int32_t *channel_idx);
#ifdef __cplusplus
}
#endif

#endif
