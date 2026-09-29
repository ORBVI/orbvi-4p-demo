/***
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2019 -2023, Horizon Robotics Co., Ltd.
 *                   All rights reserved.
 ***/

/**
 * @file ipcf_hal_data.h
 *
 * @NO{S17E09C01I}
 * @ASIL{B}
 */

#ifndef IPCF_HAL_DATA_H
#define IPCF_HAL_DATA_H
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include <stdint.h>

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define IPCF_IOC_MAGIC                'i'/**< ioctl magic number*/
#define IPCF_IO_MASK_CHANNEL_CONFIG    _IO(IPCF_IOC_MAGIC, 8)/**< ioctl config channel command*/
#define IPCF_IO_MASK_CHANNEL_DISABLE   _IO(IPCF_IOC_MAGIC, 9)/**< ioctl disable channel command*/
#define IPCF_IO_CHECK_VERSION          _IOWR(IPCF_IOC_MAGIC, 10, struct ipcf_ver_check_info)/**< ioctl check version*/

#define IPCFHAL_CHANNEL_CONFIG_MIN (0)/**< min config channel id*/
#define CONFIG_FILE_PATH_LEN (128u)/**< max length of file path*/
/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/
/* open device file by block / noblock */

/**
 * @enum device_open_mode_e
 * Define the descriptor of device open mode.
 * @NO{S17E09C01}
 */
enum device_open_mode_e {
	DEVICE_OPEN_NONBLOCK = 0,/**< open device in noblock mode*/
	DEVICE_OPEN_BLOCK = 1,/**< open device in block mode*/
};

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/**
 * @struct ipcf_ver_check_info
 * Define the descriptor of IPCF version check
 * @NO{S17E09C01}
 */
struct ipcf_ver_check_info {
	uint32_t major;/**< the major version number*/
	uint32_t minor;/**< the minor version number*/
};

/**
 * @struct ipcf_txinfo_t
 * @brief Define the descriptor of tx information.
 * @NO{S17E09C01}
 */
struct ipcf_txinfo_t {
	uint32_t channel_id;/**< channel id*/
	uint64_t buffer;/**< data*/
	uint32_t len;/**< data size*/
};

/**
 * @struct ipcf_rxinfo_t
 * @brief Define the descriptor of rx information.
 * @NO{S17E09C01}
 */
struct ipcf_rxinfo_t {
	uint32_t channel_id;/**< channel id*/
	uint64_t buffer;/**< data*/
	uint32_t len;/**< data size*/
	int32_t timeout;/**< timeout*/
	uint32_t pkg_num;/**< package count in data*/
};

/**
 * @struct config_info_t
 * @brief Define the descriptor of config information.
 * @NO{S17E09C01}
 */
struct config_info_t{
	uint32_t channel_id;/**< channel id*/
	uint32_t fifo_size;/**< channel fifo size*/
	uint32_t fifo_type;/**< fifo type*/
};

/**
 * @description: Get both the git commit hash and version of libhbipcfhal.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libhbipcfhal_get_version_info(void);

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif/*IPCF_HAL_DATA_H*/
