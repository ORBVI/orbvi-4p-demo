/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

/**
 * @file hb_camera_data_info.h
 *
 * @NO{S10E02C01}
 * @ASIL{B}
 */

#ifndef __HB_CAMERA_DATA_INFO_H__
#define __HB_CAMERA_DATA_INFO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def CAMERA_FRAMEWORK_HBN
 * macro flag for the hbn_ APIs framework
 */
#define CAMERA_FRAMEWORK_HBN

/**
 * @enum camera_reg_type_e
 * camera register operation type
 */
typedef enum camera_reg_type_e {
	CAMERA_SERIAL_REG,
	CAMERA_EEPROM_REG,
	CAMERA_SENSOR_REG,
} camera_reg_type_t;

/**
 * @enum camera_param_type_e
 * camera param operation type
 */
typedef enum camera_param_type_e {
	CAMERA_SENSOR_PARAM,
	CAMERA_EEPROM_INTRI_PARAM,
	CAMERA_EEPROM_FULL_PARAM = 3,
} camera_param_type_t;

typedef struct camera_version_s {
    uint8_t major;
    uint8_t minor;
} camera_version_t;

/**
 * @enum camera_version_type_e
 * camera version get type
 */
typedef enum camera_version_type_e {
	CAMERA_CAM_VERSION,
	CAMERA_SENSOR_VERSION,
	CAMERA_CALIB_VERSION,
	CAMERA_DESERIAL_VERSION,
	CAMERA_POC_VERSION,
	CAMERA_TXSER_VERSION,
} camera_version_type_t;

#define LEN_VENDOR_STR			20
#define LEN_SENSOR_NAME_STR		20
#define LEN_FOV_STR			10
#define LEN_PATTERN_STR			10
#define LEN_CALB_NAME_STR		100
#define LEN_HW_VER_STR			20

/**
 * @struct sensor_parameter_s
 * sensor parameter struct info
 */
typedef struct sensor_parameter_s {
	uint32_t frame_length;
	uint32_t line_length;
	uint32_t width;
	uint32_t height;
	float    fps;
	uint32_t pclk;
	uint32_t exp_num;
	uint32_t lines_per_second;
	char     version[10];
	char     vendor[LEN_VENDOR_STR];
	char     sensor_name[LEN_SENSOR_NAME_STR];
	char     fov[LEN_FOV_STR];
	char     bayer_pattern[LEN_PATTERN_STR];
	char     calb_name[LEN_CALB_NAME_STR];
	char     sensor_hw_version[LEN_HW_VER_STR];  // reserved
	char     reserved[64];
} sensor_parameter_t;

/**
 * @struct sensor_intrinsic_parameter_s
 * sensor intrinsic parameter struct info
 */
typedef struct sensor_intrinsic_parameter_s {
	uint8_t major_version;
	uint8_t minor_version;
	uint16_t vendor_id;
	uint16_t module_id;
	uint32_t module_serial;
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t cam_type;
	uint8_t module_falg;
	uint8_t efl_flag;
	uint8_t cod_flag;
	uint8_t pp_flag;
	uint8_t distortion_flag;
	uint16_t image_height;
	uint16_t image_width;
	uint32_t crc32_1;
	uint32_t crc_group1;
	uint8_t distort_params;
	uint8_t distort_model_type;
	uint8_t serial_num[40];
	double pp_x;
	double pp_y;
	double cam_skew;
	double focal_u;
	double focal_v;
	double center_u;
	double center_v;
	double hfov;
	double k1;
	double k2;
	double p1;
	double p2;
	double k3;
	double k4;
	double k5;
	double k6;
	double k7;
	double k8;
	double k9;
	double k10;
	double k11;
	double k12;
	double k13;
	double k14;
	double focal_u_2;
	double focal_v_2;
	double center_u_2;
	double center_v_2;
	double k1_2;
	double k2_2;
	double k3_2;
	double k4_2;
} sensor_intrinsic_parameter_t;

/**
 * @struct cam_parameter_s
 * camera parameter struct info
 */
typedef struct cam_parameter_s {
	sensor_parameter_t sns_param;
	sensor_intrinsic_parameter_t sns_intrinsic_param;
} cam_parameter_t;

#define MAX_COMBINE_FRAME		(4)
#define MAX_TEMPERATURE_NUM		(2)

/**
 * @struct embed_data_cust_info_s
 * custom embedded data info struct
 */
typedef struct embed_data_cust_info_s {
	const char *name;
	uint32_t size;
	void *info;
} embed_data_cust_info_t;

/**
 * @struct cam_timeval_s
 * timeval struct in camera
 */
typedef struct cam_timeval_s {
	uint64_t tv_sec;
	uint64_t tv_usec;
} cam_timeval_t;

/**
 * @struct embed_data_info_s
 * embedded data info struct
 */
typedef struct embed_data_info_s {
	uint32_t port;
	uint32_t dev_port;
	uint32_t frame_count;
	uint32_t line[MAX_COMBINE_FRAME];
#if 0
	uint32_t again[MAX_COMBINE_FRAME];
	uint32_t dgain[MAX_COMBINE_FRAME];
	uint32_t rgain[MAX_COMBINE_FRAME];
	uint32_t bgain[MAX_COMBINE_FRAME];
	uint32_t grgain[MAX_COMBINE_FRAME];
	uint32_t gbgain[MAX_COMBINE_FRAME];
	uint32_t vts;
	uint32_t hts;
#endif
	uint32_t height;
	uint32_t width;
	uint32_t exposure;
	int32_t  temperature;
	uint32_t check_state;

	uint32_t num_exp;
	uint32_t exposures[MAX_COMBINE_FRAME];
	uint32_t num_temp;
	int32_t  temperatures[MAX_TEMPERATURE_NUM];

	uint32_t frame_id;
	uint64_t time_stamp;//HW time stamp
	struct cam_timeval_s tv;//system time of hal get buf

	int32_t  reserved[16];
	struct embed_data_cust_info_s cust;
} embed_data_info_t;

/**
 * @struct cam_statinfo
 * camera status info struct
 */
typedef struct cam_statinfo {
	uint32_t inited;
	uint32_t started;
	uint32_t diag_enable;
	uint32_t recov_enable;
	uint32_t link_status;
	uint32_t recoving;
} camera_staus_t;

/**
 * @struct camera_ae_info_s
 * camera ae info struct
 */
typedef struct camera_ae_info_s {
	uint32_t line;
	uint32_t again;
	uint32_t dgain;
} camera_ae_info_t;

/**
 * @enum CAM_EVENT_TYPE_S
 * camera event type enum
 */
typedef enum CAM_EVENT_TYPE_S {
	HB_CAM_EVENT_DIAG = 0,
} CAM_EVENT_TYPE_E;

/**
 * @struct hb_cam_event_s
 * camera event info struct
 */
typedef struct hb_cam_event_s {
    uint32_t port;
    uint32_t status;
    uint32_t event_id;
    uint32_t module_id;
    uint32_t event_type;
} cam_event_t;

/* sensor otp config */
#define SENSOR_OTP_LSC_CT_NUM 1
#define SENSOR_OTP_AWB_CT_NUM 3
#define SENSOR_OTP_LSC_EEPROM_SIZE 17
#define SENSOR_OTP_LSC_V_GRID_NUM 33
#define SENSOR_OTP_LSC_H_GRID_NUM 33
#define SENSOR_OTP_PDAF_FOCAL_SIZE 48

typedef enum color_temperature_e {
	COLOR_TEMPERATURE_3100K = 0,
	COLOR_TEMPERATURE_4000K = 1,
	COLOR_TEMPERATURE_5800K = 2,
	COLOR_TEMPERATURE_MAX = 3
} color_temperature_t;

typedef struct sensor_otp_lsc_s {
	uint16_t r[SENSOR_OTP_LSC_EEPROM_SIZE][SENSOR_OTP_LSC_EEPROM_SIZE];
	uint16_t gr[SENSOR_OTP_LSC_EEPROM_SIZE][SENSOR_OTP_LSC_EEPROM_SIZE];
	uint16_t gb[SENSOR_OTP_LSC_EEPROM_SIZE][SENSOR_OTP_LSC_EEPROM_SIZE];
	uint16_t b[SENSOR_OTP_LSC_EEPROM_SIZE][SENSOR_OTP_LSC_EEPROM_SIZE];
	uint16_t golden_r[SENSOR_OTP_LSC_V_GRID_NUM][SENSOR_OTP_LSC_H_GRID_NUM];
	uint16_t golden_gr[SENSOR_OTP_LSC_V_GRID_NUM][SENSOR_OTP_LSC_H_GRID_NUM];
	uint16_t golden_gb[SENSOR_OTP_LSC_V_GRID_NUM][SENSOR_OTP_LSC_H_GRID_NUM];
	uint16_t golden_b[SENSOR_OTP_LSC_V_GRID_NUM][SENSOR_OTP_LSC_H_GRID_NUM];
} sensor_otp_lsc_t;

typedef struct sensor_otp_awb_s {
	color_temperature_t color_temperature;
	uint16_t r;
	uint16_t gr;
	uint16_t gb;
	uint16_t b;
	uint16_t rg_ratio;
	uint16_t bg_ratio;
} sensor_otp_awb_t;

typedef struct sensor_otp_af_s {
	uint8_t focus_enable;
	uint16_t cd_min_focal;
	uint16_t cd_max_focal;
	int32_t pd_focal[SENSOR_OTP_PDAF_FOCAL_SIZE];
} sensor_otp_af_t;

typedef struct sensor_otp_s {
	uint8_t otp_lsc_enable;
	uint8_t otp_awb_enable;
	uint8_t otp_af_enable;
	uint8_t lsc_ct_num;
	uint8_t awb_ct_num;
	uint8_t awb_golden_ct_num;
	sensor_otp_lsc_t lsc_data[SENSOR_OTP_LSC_CT_NUM];
	sensor_otp_awb_t awb_data[SENSOR_OTP_AWB_CT_NUM];
	sensor_otp_awb_t awb_golden_data[SENSOR_OTP_AWB_CT_NUM];
	sensor_otp_af_t af_data;
} sensor_otp_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HB_CAMERA_DATA_INFO_H__ */
