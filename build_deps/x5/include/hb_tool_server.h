#ifndef _HB_TOOL_SERVER_H
#define _HB_TOOL_SERVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define TOOL_VERSION                                      (2) // 0-XJ3 1-J5 2-J6
#define COMMAND_SET                                       0x00000000
#define COMMAND_GET                                       0x00000001

#define TOOL_HEADER_START_N                              (0xccddeeff)
#define TOOL_HEADER_CHECK1_N                             (0x6789abcd)
#define TOOL_HEADER_CHECK2_N                             (0x00)
#define TOOL_HEADER_END_N                                (0xffeeddcc)


enum RAW_BIT {
	RAW_8 = 0,
	RAW_10 = 1,
    RAW_12 = 2,
	RAW_14 = 3,
    RAW_16 = 4,
    RAW_COMP_10 = 5,
    RAW_COMP_12 = 6,
    RAW_COMP_14 = 7,
};

enum YUV_TYEP {
	YUVNV12 = 0,
    YUV420 = 1,
	YUV422 = 2,
	YUV444 = 3,
	YUVI420 = 4,
	RGB888 = 5,
};

enum VIDEO_TYPE {
	H264 = 0,
    H265 = 1,
	PPS = 2,
};

enum DateType {
    RAW_DATA = 0,
    YUV_DATA = 1,
    JEPG_DATA = 2,
    VIDEO_DATA = 3,
    VIDEO_CFG = 4,
    STATS_AWB_DATA = 5,
    STATS_AEfull_DATA = 6,
    STATS_AE5bin_DATA = 7,
    STATS_LUMVAR_DATA = 8,
    STATS_AF_DATA = 9,
    RGB888_DATA = 10,
    NEW_CMD_HEADER = 11,
    ISP_INFO_DATA = 12,
    NET_SEND_CFG = 13,
    ACT_CTL_DATA = 14,
    ACT_CTL_ACK = 15,
    MESSAGE_CTL_DEFINE_BY_USE = 16,
    MESSAGE_CTL_ARC = 17,
    RECEIVE_UNSERVED,
};

enum sensor_mode {
	NORMAL = 1,
	DOL2 = 2,
	DOL3 = 3,
	DOL4 = 4,
};

// format
enum Act_ctrl_format {
    ACT_HARDWARE_REG = 0,
    ACT_SOFTWARE_API = 1,
    ACT_SOFTWARE_CALIB = 2,
};

typedef struct pic_info_s {
    uint32_t pipe_id;
	uint32_t chn_id; // yuv channel or raw frame_plane
	uint32_t frame_id;
    uint32_t type; //raw or yuv or jpg or video
    uint32_t format; //raw: raw8 raw10 raw12 raw14 raw16 yuv:yuv420 yuv444
    uint32_t width;
	uint32_t height;
	uint32_t stride;
    uint32_t code_type;//compression mode h264,h265
} pic_info_t;

typedef struct cmd_header_new_s {
	uint32_t header_start;
    uint32_t header_check1;
    uint32_t header_check2;
    uint32_t header_end;
    uint32_t header_crc;
	uint32_t len;
	uint32_t type; // DateType
    uint32_t format; // RAW_BIT or YUV_TYEP
	union {
		struct {
			uint32_t width;
			uint32_t height;
			uint32_t stride;
			uint32_t frame_plane; // sensor_mode
			uint32_t code_type; //  VIDEO_TYPE
			uint32_t pipe_info;
		} pic_i;
		struct {
			uint32_t cmd_id;
		    uint32_t command_id;
			uint32_t value;
		} api_rw;
		struct {
		    uint32_t id;
			uint32_t type;
			uint32_t size;
			uint32_t width;
			uint32_t rows;
			uint32_t cols;
		} calib_rw;
		struct {
			uint32_t offset;
			uint32_t size;
			uint32_t type; // mask or not
		} reg_rw;
	} metadata;
	union {
		struct {
    		uint32_t pipe_id;
			uint32_t chn_id;
			uint32_t frame_id;
		} r_i;
		struct {
			uint32_t pipe_id;
    		uint32_t direction;
			uint32_t count_id;
		} s_i;
	} packinfo;
    uint32_t chip_version;
    uint32_t plugin_id;
    uint32_t reserved2;
} cmd_header_new_t;

typedef int32_t (* hb_reg_ctrl_func)(uint32_t pipeline, uint32_t plugin_id, uint32_t offset,
    uint32_t *data, uint32_t size, uint32_t direction, uint32_t type, void *arg);
typedef int32_t (* hb_api_ctrl_func)(uint32_t pipeline, uint32_t plugin_id, uint32_t direction,
    uint32_t cmd, uint32_t id, uint32_t *value, void *arg);
typedef int32_t (* hb_calib_ctrl_func)(uint32_t pipeline, uint32_t plugin_id, uint32_t direction,
    uint32_t type, uint32_t id, uint32_t width, uint32_t rows, uint32_t cols,
	void *data, uint32_t size, void *arg);
typedef int32_t (* hb_common_ctrl_func)(cmd_header_new_t *message,
		void *ptr, uint32_t size, void *arg);

typedef struct _tool_event_s {
	void *ev_base;
	hb_reg_ctrl_func regcb;
	hb_api_ctrl_func apicb;
	hb_calib_ctrl_func calibcb;
    hb_common_ctrl_func commcb;
	void *cbarg;
	uint32_t port;
} tool_event_t;

tool_event_t * hb_tool_start_transfer(uint32_t port);
int32_t hb_tool_stop_transfer(tool_event_t * ev);
int32_t hb_tool_send_yuv_pic(tool_event_t * ev, pic_info_t *info,
	void *y, uint32_t y_size, void *uv, uint32_t uv_size, void *ext, uint32_t ext_size);
int32_t hb_tool_send_raw_pic(tool_event_t * ev, pic_info_t *info,
	void *ptr, uint32_t size, void *ext, uint32_t ext_size);
int32_t hb_tool_send_video_pic(tool_event_t * ev, pic_info_t *info, void *ptr, uint32_t size);
int32_t hb_tool_used_define_pic(tool_event_t * ev, cmd_header_new_t *info, void *ptr, uint32_t size);

#define EV_REG_FLAG	         (1 << 0)
#define EV_API_FLAG		     (1 << 1)
#define EV_CALIB_FLAG   	 (1 << 2)
#define EV_COMMON_FLAG  	 (1 << 3)

#define DEFAULT_PORT         (10086)

int32_t hb_tool_event_enable(tool_event_t * ev, uint32_t event);
int32_t hb_tool_event_disable(tool_event_t * ev, uint32_t event);
void hb_tool_event_setcb(tool_event_t * ev, hb_reg_ctrl_func regcb, hb_api_ctrl_func apicb,
    hb_calib_ctrl_func calibcb, hb_common_ctrl_func commcb, void *cbarg);

// xj3 & j5 & j6 stats info
typedef struct hb_tool_statics_awb_zone_attr_s {
	uint16_t rg;
	uint16_t bg;
	uint32_t sum;
} hb_tool_statics_awb_zone_attr_t;

typedef struct hb_tool_awb_pos_status_s {
	uint16_t awb_rg_pos[15];
	uint16_t awb_bg_pos[15];
} hb_tool_awb_pos_status_t;

typedef struct hb_tool_mesh_rgbg_weight_s {
	uint16_t mesh_rgbg_weight[15][15];
} hb_tool_mesh_rgbg_weight_t;

typedef struct hb_tool_zone_attr_s {
	uint8_t horiz;
	uint8_t vert;
} hb_tool_zone_attr_t;

typedef struct _awb_stats_info_s {
	hb_tool_statics_awb_zone_attr_t stats[33*33];
	hb_tool_awb_pos_status_t pos;
	hb_tool_mesh_rgbg_weight_t weight;
	hb_tool_zone_attr_t zone;
	uint16_t wb_info[4];
	uint32_t rgain;
	uint32_t bgain;
} awb_stats_info;

typedef struct _ae_1024bin_stats_info_s {
	uint32_t stats[1024];
} ae_1024bin_stats_info;

typedef struct _af_stats_info_s {
    uint32_t stats[33*33*2];
    hb_tool_zone_attr_t zone;
} _af_stats_info_t;

/**
 * @description: Get both the git commit hash and version of libhbplayer.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libhbplayer_get_version_info(void);

#ifdef __cplusplus
    }
#endif

#endif // CLIENTCOMMAND_H
