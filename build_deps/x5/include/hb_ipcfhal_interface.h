/***
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2019 -2023, Horizon Robotics Co., Ltd.
 *                   All rights reserved.
 ***/

/**
 * @file hb_ipcfhal_interface.h
 *
 * @NO{S17E09C01I}
 * @ASIL{B}
 */

#ifndef HB_IPCFHAL_INTERFACE_H
#define HB_IPCFHAL_INTERFACE_H
#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define IPCFHAL_CHANNEL_PACKAGE_SIZE_MIN (0)/**< min package size*/
#define IPCFHAL_CHANNEL_PACKAGE_SIZE_MAX (0x4000u)/**< max package size*/

#define IPCFHAL_CHANNEL_ID_UNUSED (1u)/**< unused channel id*/
/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/**
 * @struct dev_info_t
 * @brief Define the descriptor of device information.
 * @NO{S17E09C01}
 */
struct dev_info_t {
	int32_t fd;/**< fd*/
	unsigned char users = 0;/**< dev users count*/
	char dev_path[128] = {0};/**< device path*/
	char dev_name[128] = {0};/**< device name*/
	bool vaild_state = false;/**< device valid state*/
};

/**
 * @struct dev_info_t
 * @brief Define the descriptor of ipcf hal api information.
 * @NO{S17E09C01}
 */
struct hal_info_t {
	bool init_state = false;/**< hal api init state*/
	bool vaild_state = false;/**< hal api config state*/
	uint32_t pkg_size_max = IPCFHAL_CHANNEL_PACKAGE_SIZE_MAX;/**< per package max size*/
};

/**
 * @struct IPCFHAL_Channel
 * @brief Define the descriptor of IPCF HAL channel struct information.
 * @NO{S17E09C01}
 */
struct IPCFHAL_Channel {
	/* channel info */
	int32_t id = -IPCFHAL_CHANNEL_ID_UNUSED;/**< channel id */
	char name[64] = {0};/**< channel name */
	int32_t fifo_size = 0;/**< channel fifo size */
	int32_t fifo_type = 0;/**< channel fifo type */

	/* ipcf dev info */
	struct dev_info_t ipcf_dev;/**< ipcf device information*/

	/* ipcf hal api info */
	struct hal_info_t ipcf_hal;/**< ipcf hal information*/
};

typedef struct IPCFHAL_Channel ipcfhal_chan_t;

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief ipcfhal init.
 *
 * @param[in] channel: channel
 *
 * @retval "0": success
 * @retval "!0": failure
 *
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_ipcfhal_init(ipcfhal_chan_t *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief ipcfhal init
 *
 * @param[in] name: IPCFHAL channel get config information
 * @param[in] channel: channel information.
 * @param[in] json_file: config information.
 *
 * @retval ">=0": success
 * @retval "<0": failure
 *
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_ipcfhal_getchan_byjson(const char *name,
				ipcfhal_chan_t *channel,
				const char *json_file);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief ipcfhal channel config
 *
 * @param[in] channel: channel information.
 *
 * @retval ">=0": success
 * @retval "<0": failure
 *
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_ipcfhal_config(ipcfhal_chan_t *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief ipcfhal send message
 *
 * @param[in] data: send data.
 * @param[in] length: length of data.
 *                      range: (0, IPCFHAL_CHANNEL_PACKAGE_SIZE_MAX)
 * @param[in] channel: channel information.
 *
 * @retval ">=0": success
 * @retval "<0": failure
 *
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_ipcfhal_send(const uint8_t *data, int16_t length,
			ipcfhal_chan_t *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief ipcfhal recv message
 *
 * @param[in] data: data destination address.
 * @param[in] length: max length of data.
 * @param[in] timeout: 0 nonblock, >0 block timout in ms, -1 block infinite.
 * @param[in] channel: channel information
 * @param[out] data: received once data.
 *
 * @retval ">=0": success
 * @retval "<0": failure
 *
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_ipcfhal_recv(uint8_t *data, int16_t length, int32_t timeout,
			ipcfhal_chan_t *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief ipcfhal deinit
 *
 * @param[in] data: data destination address.
 * @param[in] channel: channel.
 *
 * @retval "0": success
 * @retval "!0": failure
 *
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_ipcfhal_deinit(ipcfhal_chan_t *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief transform ipcfhal library error code to string description
 *
 * @param[in] err_code: error code
 *			range: ( , 0)
 *
 * @param[out] err_str: pointer to error string
 *
 * @retval "0": success
 * @retval "!0": failure
 *
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_ipcfhal_trans_err(int32_t err_code, char **err_str);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief get the version of ipcfhal library.
 *
 * @param[in] major: major version number
 * @param[in] minor: minor version number
 * @param[in] patch: patch version number
 *
 * @retval "0": success
 * @retval "!0": failure
 *
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_ipcfhal_get_version(uint32_t *major, uint32_t *minor, uint32_t *patch);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief IPCFHAL init.
 *
 * @param[in] channel: channel
 *
 * @retval "0": success
 * @retval "!0": failure
 *
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t HorizonHal_IPCF_Init(IPCFHAL_Channel *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief IPCFHAL init
 *
 * @param[in] name: IPCFHAL channel get config information
 * @param[in] channel: channel information.
 * @param[in] json_file: config information.
 *
 * @retval ">=0": success
 * @retval "<0": failure
 *
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t HorizonHal_IPCF_GetChannelbyJson(const char *name,
					IPCFHAL_Channel *channel,
					const char *json_file);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief IPCFHAL channel config
 *
 * @param[in] channel: channel information.
 *
 * @retval ">=0": success
 * @retval "<0": failure
 *
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t HorizonHal_IPCF_Config(IPCFHAL_Channel *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief IPCFHAL send message
 *
 * @param[in] data: send data.
 * @param[in] length: length of data.
 *                      range: (0, IPCFHAL_CHANNEL_PACKAGE_SIZE_MAX)
 * @param[in] channel: channel information.
 *
 * @retval ">=0": success
 * @retval "<0": failure
 *
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t HorizonHal_IPCF_Send(const uint8_t *data, int16_t length,
				IPCFHAL_Channel *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief IPCFHAL recv message
 *
 * @param[in] data: data destination address.
 * @param[in] length: max length of data.
 * @param[in] timeout: 0 nonblock, >0 block timout in ms, -1 block infinite.
 * @param[in] channel: channel information
 * @param[out] data: received once data.
 *
 * @retval ">=0": success
 * @retval "<0": failure
 *
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t HorizonHal_IPCF_Recv(uint8_t *data, int16_t length, int32_t timeout,
				IPCFHAL_Channel *channel);

/**
 * @NO{S17E09C01I}
 * @ASIL{B}
 * @brief IPCFHAL deinit
 *
 * @param[in] data: data destination address.
 * @param[in] channel: channel.
 *
 * @retval "0": success
 * @retval "!0": failure
 *
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t HorizonHal_IPCF_Deinit(IPCFHAL_Channel *channel);

#ifdef __cplusplus
}
#endif

#endif/*HB_IPCFHAL_INTERFACE_H*/
