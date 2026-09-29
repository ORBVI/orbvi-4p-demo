/**
 * @file hobot_ota_hl.h
 *
 * @NO{S21E03C02}
 *
 * @ASIL{B}
 *
 * COPYRIGHT NOTICE
 * Copyright 2023 Horizon Robotics, Inc.
 * All rights reserved.
 */

/*
 Version History

 Version 1.0.1 June 13, 2023
 - Initial release
*/

#ifndef __HOBOT_OTA_HL_H__
#define __HOBOT_OTA_HL_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_2 2
#define ARRAY_4 4
#define ARRAY_32 32
#define ARRAY_64 64
#define ARRAY_128 128
#define ARRAY_256 256
#define ARRAY_512 512

/**
 * @enum ota_err_e
 * @brief ota lib error type
 * @NO{S21E03C02}
 */
enum ota_err_e {
	OTA_SUCCESS = 0,
	OTAERR_IO,
	OTAERR_PLAT_UNSUPPORT,
	OTAERR_REPEAT,
	OTAERR_MUTEX_INIT_LOCK_ERR,
	OTAERR_NOTINIT,
	OTAERR_NULLPOINTER,
	OTAERR_SHORTBUF,
	OTAERR_THREAD_CREATE,
	OTAERR_RANGE,
	OTAERR_STAGE,
	OTAERR_IMAGE_WRITE,
	OTAERR_BOOT_FAILED,

	OTAERR_VEEPROM,
	OTAERR_FILE_TYPE,
	OTAERR_UNZIP,
	OTAERR_NO_EXISTS,
	OTAERR_MALLOC,
	OTAERR_VERIFY,
	OTAERR_IMG_SIZE,
	OTAERR_UPDATE_STATUS,
};

/**
 * @enum otahl_update_result
 * @brief ota upgrade result
 * @NO{S21E03C02}
 */
typedef enum otahl_update_result {
	OTA_UPGRADE_NOT_START = 0, /**< OTA upgrade not start */
	OTA_UPGRADE_IN_PROGRESS, /**< OTA upgrading */
	OTA_UPGRADE_SUCCESS, /**< OTA upgrade success*/
	OTA_UPGRADE_FAILED, /**< OTA upgrade failed*/
} otahl_update_result_e;

/**
 * @enum ota_update_owner_e
 * @brief
 * @NO{S21E03C02}
 */
typedef enum ota_update_owner {
	NORMAL_BOOT = 0, /**< normal boot */
	OTA_TOOL, /**< OTA tool */
	OTA_SERVICE, /**< OTA service */
	/* Add owner here */
	OTA_N_OWNER
} ota_update_owner_e;

/**
 * @enum ota_sec_update_type_e
 * @brief secure image type
 * @NO{S21E03C02}
 */
typedef enum sec_update_type {
	SEC_FW = 0xD2, /**< SEC firmware */
	SEC_RCA, /**< SEC RCA */
} ota_sec_update_type_e;

/**
 * @struct ota_sec_img_t
 * @brief secure image struct
 * @NO{S21E03C02}
 */
typedef struct ota_sec_img_s {
	ota_sec_update_type_e sec_type; /**< secure image type */
	char	cert_path[ARRAY_128]; /**< OTA cert path, reserved  */
	char	img_path[ARRAY_128]; /**< secure image path, filled by lib */
	uint8_t nouce[ARRAY_32]; /**< nouce, filled by lib */
	uint8_t signature[ARRAY_64]; /**< signature, filled by service */
} ota_sec_img_t;

/**
 * @struct ota_sec_info_t
 * @brief secure image information
 * @NO{S21E03C02}
 */
typedef struct ota_sec_info_s {
	uint8_t	      num; /**< secure number */
	ota_sec_img_t img_info[ARRAY_2]; /**< secure image struct */
} ota_sec_info_t;

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief OTA upgrade library initialization
 *
 * @retval 0: OTA upgrade library init successfully
 * @retval -OTAERR_REPEAT: repeat init
 *
 * @data_read None
 * @data_updated g_upgrade_info.result: Set OTA upgrade result to Not Started
 * @data_updated g_is_ota_init: Set true
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaInitLib(void);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief OTA upgrade library Deinitialization
 *
 * @retval 0: OTA upgrade library deinit successfully
 * @retval -OTAERR_NOTINIT: The library has not been initialized
 *
 * @data_read None
 * @data_updated g_upgrade_info.result: Set OTA upgrade result to Not Started
 * @data_updated g_is_ota_init: Set false
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaDeinitLib(void);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief get ota upgrade library version
 *
 * @param[out] version: version string pointer
 * @param[in] len: length of version
 *
 * @retval 0: success
 * @retval -OTAERR_NULLPOINTER: null pointer
 * @retval -OTAERR_SHORTBUF: buffer too short
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaGetLibVersion(char *version, int32_t len);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief get system version
 *
 * @param[out] version: version string pointer
 * @param[in] len: length of version
 *
 * @retval 0: success
 * @retval -OTAERR_NULLPOINTER: null pointer
 * @retval -OTAERR_IO: IO error
 * @retval -OTAERR_SHORTBUF: buffer too short
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaGetSysVersion(char *version, int32_t len);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief get ota upgrade result
 *
 * @retval OTA_UPGRADE_NOT_START: upgrade success
 * @retval OTA_UPGRADE_IN_PROGRESS: upgrade in progress
 * @retval OTA_UPGRADE_SUCCESS: upgrade not start
 * @retval OTA_UPGRADE_FAILED: upgrade failed
 * @retval -OTAERR_NOTINIT: not initialized
 *
 * @data_read g_upgrade_info.result: upgrade result
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaGetResult(void);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief get ota upgrade progress
 *
 * @retval "0-100": upgrade progress percentage
 * @retval -OTAERR_NOTINIT: not initialized
 *
 * @data_read g_upgrade_info.progress: current upgrade progress
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaGetProgress(void);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief get partition name in ota upgrade process
 *
 * @param[out] image_name: partition name string pointer
 * @param[in] len: the length of partition name
 *
 * @retval 0: get partition name success
 * @retval -OTAERR_NOTINIT: not initialized
 * @retval -OTAERR_NULLPOINTER: null pointer
 * @retval -OTAERR_SHORTBUF: buffer too short
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaGetUpdatingImageName(char *image_name, int32_t len);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief get the partition currently in AB
 *
 * @param[out] partition: buffer to store result
 *                        0: A slot, 1: B slot
 *
 * @retval 0: success
 * @retval -OTAERR_NULLPOINTER: null pointer
 * @retval -OTAERR_IO: IO error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaGetPartition(uint8_t *partition);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief Set AB slot of next boot
 *
 * @param[in] partition: next boot slot
 *                       0: A partition, 1: B partition
 *
 * @retval 0: success
 * @retval -OTAERR_IO: IO error
 * @retval -OTAERR_RANGE: the value of partition is out of range
 * @retval -OTAERR_STAGE: This operation is not supported in current stage(failed or in_progress)
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaSetPartition(uint8_t partition);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief start ota upgrade process
 *
 * @param[in] zip_path: Path of ota packages which is divided by semicolons
 * @param[in] owner: who launched this upgrade
 *
 * @retval 0: start ota upgrade success
 * @retval -OTAERR_NOTINIT: not initialized
 * @retval -OTAERR_NULLPOINTER: null pointer
 * @retval -OTAERR_REPEAT: repeat start
 * @retval -OTAERR_IO: IO error
 * @retval -OTAERR_THREAD_CREATE: thread or process create failed
 *
 * @data_read g_is_ota_init: check whether OTA lib is initialized
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaRequestStart(const char *zip_path, ota_update_owner_e owner);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief Get owner of this upgrade
 *
 * @param[out] owner: Result of the owner
 *
 * @retval 0: success
 * @retval -OTAERR_IO: IO error
 * @retval -OTAERR_NOTINIT: not initialized
 * @retval -OTAERR_NULLPOINTER: null pointer
 *
 * @data_read g_is_ota_init: check whether OTA lib is initialized
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaGetOwnerFlag(ota_update_owner_e *owner);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief Check result of the upgrade
 *
 * @retval 0: success
 * @retval -OTAERR_IO: IO error
 * @retval -OTAERR_NOTINIT: not initialized
 * @retval -OTAERR_STAGE: OTA has not been launched
 * @retval -OTAERR_IMAGE_WRITE: image write failed
 * @retval -OTAERR_BOOT_FAILED: new version boot failed
 *
 * @data_read g_is_ota_init: check whether OTA lib is initialized
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaCheckUpdate(void);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief Synchronize the current partition to the opposite partition
 *        and clear veeprom flags
 *
 * @retval 0: success
 * @retval -OTAERR_IO: IO error
 * @retval -OTAERR_NOTINIT: not initialized
 *
 * @data_read g_is_ota_init: check whether OTA lib is initialized
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaPartitionSync(void);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief mark system boot success to misc partition
 *
 * @retval 0: success
 * @retval -OTAERR_NOTINIT: not initialized
 * @retval -OTAERR_IO: IO error
 *
 * @data_read g_is_ota_init: check whether OTA lib is initialized
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaMarkOTASuccessful(void);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief Initialize the secure img and generate a random number from hsm
 *
 * @retval 0: success
 * @retval -OTAERR_NOTINIT: not initialized
 * @retval -OTAERR_IO: IO error
 * @retval -OTAERR_NULLPOINTER: null pointer
 *
 * @data_read g_is_ota_init: check whether OTA lib is initialized
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaInitSecImg(const char *zip_path, ota_sec_info_t *sec_info);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief Authenticate the hsm upgrade
 *
 * @retval 0: success
 * @retval -OTAERR_NOTINIT: not initialized
 * @retval -OTAERR_IO: IO error
 * @retval -OTAERR_NULLPOINTER: null pointer
 *
 * @data_read g_is_ota_init: check whether OTA lib is initialized
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t otaVerifySecImg(ota_sec_info_t *sec_info);

/**
 * @NO{S21E03C02I}
 * @ASIL{B}
 * @brief Abort current upgrading and clear flags in veeprom
 *
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
void otaClearFlags(void);

/**
 * @description: Get both the git commit hash and version of libupdate.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libupdate_get_version_info(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HOBOT_OTA_HL_H__ */
