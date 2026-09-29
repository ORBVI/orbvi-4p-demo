/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

/**
 * @file hb_deserial_interface.h
 *
 * @NO{S10E02C02}
 * @ASIL{B}
 */

#ifndef __HB_DESERIAL_INTERFACE_H__
#define __HB_DESERIAL_INTERFACE_H__

#include "hb_camera_data_config.h"
#include "hb_camera_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @NO{S10E02C02I}
 * @ASIL{B}
 * @brief create deserial handle with deserial config
 * @detail the deserial config should adapt the hardware used.
 *	   use it if there is a deserial which camera connected to.
 *
 * @param[in] des_config: deserial config with deserial_config_t struct
 *         range: [!NULL, !NULL], default: NA
 * @param[out] des_fd: deserial handle fd created return
 *         range: [!NULL, !NULL], default: NA
 *
 * @return 0:Success, <0:Failure
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
extern int32_t hbn_deserial_create(deserial_config_t *des_config, deserial_handle_t *des_fd);

/**
 * @NO{S10E02C02I}
 * @ASIL{B}
 * @brief destroy deserial handle to exit
 * @detail all handle created should be destroyed at last.
 *	   use it if there is a deserial which camera connected to.
 *
 * @param[in] des_fd: deserial handle with create returned
 *         range: [!NULL, !NULL], default: NA
 *
 * @return 0:Success, <0:Failure
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
extern int32_t hbn_deserial_destroy(deserial_handle_t des_fd);

/**
 * @NO{S10E02C02I}
 * @ASIL{B}
 * @brief attach deserial handle to handle of vin node in vpf
 * @detail get connection info from vin and initializ the deserial(and camera) hardware.
 *	   use it if there is a deserial which camera connected to.
 *
 * @param[in] des_fd: deserial handle with create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] link: deserial link index to attach
 *         range: [0, 3], default: NA
 * @param[in] vin_fd: vpf handle of vin node which vpf create returned
 *         range: [!NULL, !NULL], default: NA
 *
 * @return 0:Success, <0:Failure
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
extern int32_t hbn_deserial_attach_to_vin(deserial_handle_t des_fd, camera_des_link_t link, vpf_handle_t vin_fd);

/**
 * @NO{S10E02C02I}
 * @ASIL{B}
 * @brief detach deserial handle from handle of vin node in vpf
 * @detail do deinitialization of deserial(and camera) hardware and detach from vin.
 *	   use it if there is a deserial which camera connected to.
 *
 * @param[in] des_fd: deserial handle with create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] link: deserial link index to detach
 *         range: [0, 3], default: NA
 *
 * @return 0:Success, <0:Failure
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
extern int32_t hbn_deserial_detach_from_vin(deserial_handle_t des_fd, camera_des_link_t link);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HB_DESERIAL_INTERFACE_H__ */
