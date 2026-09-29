/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

/**
 * @file hb_camera_data_config.h
 *
 * @NO{S10E02C01}
 * @ASIL{B}
 */

#ifndef __HB_CAMERA_DATA_CONFIG_H__
#define __HB_CAMERA_DATA_CONFIG_H__

#include <stdint.h>
#include <sys/time.h>

#include "hb_camera_data_vpf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def CAMERA_CONFIG_END_MAGIC
 * camera config end flag with magic and size
 * defing magic code as: CXYY  X-as type, YY-as size
 */
#define CAMERA_CONFIG_END_MAGIC(t, size)	((0x43 << 24) | ((0x30 + (t)) << 16) | ((size) & 0xFFFFU))
/**
 * @def CAMERA_VERSON_LEN_MAX
 * camera version buffer length size
 */
#define CAMERA_VERSON_LEN_MAX			(32)
/**
 * @def CAMERA_MODULE_NAME_LEN
 * camera module name string length
 */
#define CAMERA_MODULE_NAME_LEN			(108)
/**
 * @def CAMERA_DES_PORTDESP_LEN
 * camera deserial port_desp string length
 */
#define CAMERA_DES_PORTDESP_LEN			(64)

/**
 * @enum sensor_mode_e
 * sensor device param of bayer start enum
 * @NO{S10E02C05}
 */
enum sensor_mode_e {
        NORMAL_M = 1,
        DOL2_M = 2,
        DOL3_M = 3,
        DOL4_M = 4,
        PWL_M = 5,
        SLAVE_M = 6,
        MONO_M = 7,
        INVALID_MOD,
};

/**
 * @enum sensor_format_e
 * sensor device param of mipi format enum
 */
enum sensor_format_e {
	DATA_TYPE_YUV420_8BIT = 0x18,
	DATA_TYPE_YUV420_10BIT = 0x19,
	DATA_TYPE_YUV422_8BIT = 0x1E,	//UYVY
	DATA_TYPE_YUV422_10BIT = 0x1F,
	DATA_TYPE_RGB444 = 0x20,
	DATA_TYPE_RGB555 = 0x21,
	DATA_TYPE_RGB565 = 0x22,
	DATA_TYPE_RGB666 = 0x23,
	DATA_TYPE_RGB888 = 0x24,
	DATA_TYPE_RAW6 = 0x28,
	DATA_TYPE_RAW7 = 0x29,
	DATA_TYPE_RAW8 = 0x2A,
	DATA_TYPE_RAW10 = 0x2B,
	DATA_TYPE_RAW12 = 0x2C,
	DATA_TYPE_RAW14 = 0x2D,
	DATA_TYPE_RAW16 = 0x2E,
	DATA_TYPE_RAW20 = 0x2F,
};

/**
 * @enum camera_des_mfp_e
 * camera deserial mfp index enum
 */
typedef enum camera_des_mfp_e {
	CAMERA_DES_MFP0,
	CAMERA_DES_MFP1,
	CAMERA_DES_MFP2,
	CAMERA_DES_MFP3,
	CAMERA_DES_MFP4,
	CAMERA_DES_MFP5,
	CAMERA_DES_MFP6,
	CAMERA_DES_MFP7,
	CAMERA_DES_MFP8,
	CAMERA_DES_MFP9,
	CAMERA_DES_MFP10,
	CAMERA_DES_MFP11,
	CAMERA_DES_MFP12,
	CAMERA_DES_MFP13,
	CAMERA_DES_MFP14,
	CAMERA_DES_MFP15,
	CAMERA_DES_MFPMAX,
} camera_des_mfp_t;

/**
 * @enum camera_des_gpio_e
 * camera deserial mfp index enum
 */
typedef enum camera_des_gpio_e {
	CAMERA_DES_GPIO_TRIG0,
	CAMERA_DES_GPIO_TRIG1,
	CAMERA_DES_GPIO_TRIG2,
	CAMERA_DES_GPIO_TRIG3,
	CAMERA_DES_GPIO_CAMERR0,
	CAMERA_DES_GPIO_CAMERR1,
	CAMERA_DES_GPIO_CAMERR2,
	CAMERA_DES_GPIO_CAMERR3,
	CAMERA_DES_GPIO_ERRB,
	CAMERA_DES_GPIO_LOCK,
	CAMERA_DES_GPIO_NC10,
	CAMERA_DES_GPIO_NC11,
	CAMERA_DES_GPIO_NC12,
	CAMERA_DES_GPIO_NC13,
	CAMERA_DES_GPIO_NC14,
	CAMERA_DES_GPIO_NC15,
	CAMERA_DES_GPIO_MAX,
} camera_des_gpio_t;

/**
 * @enum camera_des_link_e
 * camera deserial link index enum
 */
typedef enum camera_des_link_e {
	CAMERA_DES_LINKA,
	CAMERA_DES_LINKB,
	CAMERA_DES_LINKC,
	CAMERA_DES_LINKD,
	CAMERA_DES_LINKMAX,
} camera_des_link_t;

/**
 * @enum camera_txs_csi_e
 * camera txser csi index enum
 */
typedef enum camera_txs_csi_e {
	CAMERA_TXS_CSIA,
	CAMERA_TXS_CSIB,
	CAMERA_TXS_CSIMAX,
} camera_txs_csi_t;

/**
 * @struct camera_config_s
 * camera config infor struct for handle create
 * @NO{S10E02C01}
 */
typedef struct camera_config_s {
	char name[CAMERA_MODULE_NAME_LEN];	// sensor名称: ar0820/...
	uint32_t addr;		// sensor设备地址
	uint32_t isp_addr;	// isp设备地址(如有)
	uint32_t eeprom_addr;	// eeprom设备地址(内参)
	uint32_t serial_addr;	// seriala设备地址(如有)
	uint32_t sensor_mode;	// 模式选址: Linear/PWL
	uint32_t sensor_clk;	// sensor的clk时钟配置
	uint32_t gpio_enable_bit;	// GPIO操作使能，索引自VCON
	uint32_t gpio_level_bit;	// GPIO操作状态，工作状态
	uint32_t bus_select;	// I2C的选择，索引自VCON
	uint32_t bus_timeout;	// I2C的timeout时间配置
	uint32_t fps;		// 帧率
	uint32_t width;		// 宽
	uint32_t height;	// 高
	uint32_t format;	// 数据格式
	uint32_t flags;		// 可选功能:诊断,恢复,debug等
	uint32_t extra_mode;	// 模组索引配置
	uint32_t config_index;	// 功能配置
	int32_t ts_compensate;	// 时间戳补偿配置
	mipi_config_t *mipi_cfg; // MIPI配置,NULL自动获取
	char calib_lname[CAMERA_MODULE_NAME_LEN];// sensor效果库路径
	char *sensor_param;	// sensor自定义数据
	uint32_t iparam_mode;
	uint32_t end_flag;	// end flag of config struct.
} camera_config_t;

/**
 * @struct poc_config_s
 * poc config infor struct for handle create
 * @NO{S10E02C01}
 */
typedef struct poc_config_s {
	char name[CAMERA_MODULE_NAME_LEN];// poc名称: max20087/...
	uint32_t addr;		// poc设备地址
	uint32_t gpio_enable_bit;	// GPIO操作使能，索引自VCON
	uint32_t gpio_level_bit;	// GPIO操作状态，工作状态
	uint32_t poc_map;	// poc与link的map关系
	uint32_t power_delay;	// poc开关操作的delay时间
	uint32_t end_flag;	// end flag of config struct.
} poc_config_t;

/**
 * @struct deserial_config_s
 * deserial config infor struct for handle create
 * @NO{S10E02C01}
 */
typedef struct deserial_config_s {
	char name[CAMERA_MODULE_NAME_LEN];// deserial名称: max9296/...
	uint32_t addr;		// deserial设备地址
	uint32_t gpio_enable_bit;	// GPIO操作使能，索引自VCON
	uint32_t gpio_level_bit;	// GPIO操作状态，工作状态
	uint8_t gpio_mfp[CAMERA_DES_GPIO_MAX];	// MFP的GPIO功能选择
	uint32_t bus_select;	// I2C的选择，索引自VCON
	uint32_t bus_timeout;	// I2C的timeout时间配置
	uint32_t lane_mode;	// PHY配置: lane模式选择
	uint32_t lane_speed;	// PHY配置: lane速率配置
	uint32_t link_map;	// Link与CSI/VC的map关系配置
	char link_desp[CAMERA_DES_LINKMAX][CAMERA_DES_PORTDESP_LEN]; // 各Link连接模组的配置描述(多进程使用)
	uint32_t reset_delay;	// reset操作的delay时间
	uint32_t flags;		// 可选功能:诊断,debug等
	poc_config_t *poc_cfg; // POC配置,NULL无POC
	mipi_config_t *mipi_cfg; // MIPI配置,NULL自动获取
	char *deserial_param;	// deserial自定义数据;
	uint32_t end_flag;	// end flag of config struct.
} deserial_config_t;

/**
 * @struct txser_config_s
 * txser config infor struct for handle create
 * @NO{S10E02C01}
 */
typedef struct txser_config_s {
	char name[CAMERA_MODULE_NAME_LEN]; // txser名称: max9296/...
	uint32_t addr;		// txser设备地址
	uint32_t gpio_enable_bit;	// GPIO操作使能，索引自VCON
	uint32_t gpio_level_bit;	// GPIO操作状态，工作状态
	uint8_t gpio_mfp[CAMERA_DES_GPIO_MAX];	// MFP的GPIO功能选择
	uint32_t bus_select;	// I2C的选择，索引自VCON
	uint32_t bus_timeout;	// I2C的timeout时间配置
	uint32_t lane_mode;	// PHY配置: lane模式选择
	uint32_t link_map;	// Link与CSI/VC的map关系配置
	uint32_t reset_delay;	// reset操作的delay时间
	uint32_t flags;		// 可选功能:诊断,debug等
	mipi_config_t *mipi_cfg; // MIPI配置,NULL自动获取
	char *txser_param;	// txser自定义数据;
	uint32_t end_flag;	// end flag of config struct.
} txser_config_t;

/**
 * @struct camera_global_config_s
 * camera global config info struct
 * @NO{S10E02C01}
 */
typedef struct camera_global_config_s {
	uint32_t init_in_seq;	// 串行初始化，仅对json方式生效
	uint32_t diag_disable;	// 诊断功能禁止
	uint32_t partial_enable;// 部分正常功能使能
	uint32_t recov_enable;	// 自恢复功能使能
	int32_t recov_timeout;	// 自恢复超时：ms
	int32_t recov_retry;	// 自恢复重试次数
	int32_t reserved[16];	// 备用
	uint32_t end_flag;	// end flag of config struct.
} camera_global_config_t;

/**
 * @def CAMERA_CONFIG_END_FLAG
 * camera config end_flag value for verify check
 */
#define CAMERA_CONFIG_END_FLAG		CAMERA_CONFIG_END_MAGIC(1, sizeof(camera_config_t))
/**
 * @def DESERIAL_CONFIG_END_FLAG
 * desrial config end_flag value for verify check
 */
#define DESERIAL_CONFIG_END_FLAG	CAMERA_CONFIG_END_MAGIC(2, sizeof(deserial_config_t))
/**
 * @def POC_CONFIG_END_FLAG
 * poc config end_flag value for verify check
 */
#define POC_CONFIG_END_FLAG		CAMERA_CONFIG_END_MAGIC(3, sizeof(poc_config_t))
/**
 * @def TXSER_CONFIG_END_FLAG
 * txser config end_flag value for verify check
 */
#define TXSER_CONFIG_END_FLAG		CAMERA_CONFIG_END_MAGIC(4, sizeof(txser_config_t))
/**
 * @def MIPI_BYPASS_END_FLAG
 * mipi bypass config end_flag value for verify check
 */
#define MIPI_BYPASS_END_FLAG		CAMERA_CONFIG_END_MAGIC(5, sizeof(mipi_bypass_t))
/**
 * @def MIPI_CONFIG_END_FLAG
 * mipi config end_flag value for verify check
 */
#define MIPI_CONFIG_END_FLAG		CAMERA_CONFIG_END_MAGIC(6, sizeof(mipi_config_t))
/**
 * @def MIPI_CONFIG_END_FLAG
 * mipi config end_flag value for verify check
 */
#define GLOBAL_CONFIG_END_FLAG		CAMERA_CONFIG_END_MAGIC(7, sizeof(camera_global_config_t))

/**
 * @def CAMERA_CONFIGS_INIT
 * camera config init with name and fill end_flag with coinfig struct type
 */
#define CAMERA_CONFIGS_INIT(name, n, t) { \
	.name = name; \
	.end_flag = CAMERA_CONFIG_END_MAGIC(sizeof(t)); \
}
/**
 * @def CAMERA_CONFIG_INIT
 * camera config init with name and auto fill end_flag with camera_config_t
 */
#define CAMERA_CONFIG_INIT(name)	CAMERA_CONFIGS_INIT(name, 1, camera_config_t)
/**
 * @def DESERIAL_CONFIG_INIT
 * deserial config init with name and auto fill end_flag with deserial_config_t
 */
#define DESERIAL_CONFIG_INIT(name)	DESERIAL_CONFIGS_INIT(name, 2, deserial_config_t)
/**
 * @def POC_CONFIG_INIT
 * poc config init with name and auto fill end_flag with poc_config_t
 */
#define POC_CONFIG_INIT(name)		POC_CONFIGS_INIT(name, 3, poc_config_t)
/**
 * @def TXSER_CONFIG_INIT
 * txser config init with name and auto fill end_flag with txser_config_t
 */
#define TXSER_CONFIG_INIT(name)		TXSER_CONFIGS_INIT(name, 4, txser_config_t)

/**
 * @typedef camera_handle_t
 * camera handle fd return by create
 */
typedef int64_t camera_handle_t;
/**
 * @typedef deserial_handle_t
 * desrial handle fd return by create
 */
typedef int64_t deserial_handle_t;
/**
 * @typedef txser_handle_t
 * txser handle fd return by create
 */
typedef int64_t txser_handle_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HB_CAMERA_DATA_INFO_H__ */
