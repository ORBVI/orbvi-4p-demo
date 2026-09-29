/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

/**
 * @file hb_camera_interface.h
 *
 * @NO{S10E02C01}
 * @ASIL{B}
 */

#ifndef __HB_CAMERA_INTERFACE_H__
#define __HB_CAMERA_INTERFACE_H__

#include "hb_camera_data_config.h"
#include "hb_camera_data_info.h"
#include "hb_camera_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief create camera handle with camera config
 * @detail the camera config should adapt the sensor to be used.
 *         the handle returned should be used in APIs of this lib only.
 *
 * @param[in] cam_config: camera config with camera_config_t struct
 *         range: [!NULL, !NULL], default: NA
 * @param[out] cam_fd: camera handle fd created return
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
extern int32_t hbn_camera_create(camera_config_t *cam_config, camera_handle_t *cam_fd);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief destroy camera handle to exit
 * @detail all handle created should be destroyed at last.
 *
 * @param[in] cam_fd: camera handle which create returned
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
extern int32_t hbn_camera_destroy(camera_handle_t cam_fd);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief attach camera handle to handle of vin node in vpf
 * @detail get connection info from vin and initializ the camera hardware.
 *	   use it only if there is no deserial here.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] vin: vpf handle of vin node which vpf create returned
 *         range_fd: [!NULL, !NULL], default: NA
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
extern int32_t hbn_camera_attach_to_vin(camera_handle_t cam_fd, vpf_handle_t vin_fd);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief detach camera handle from handle of vin node in vpf
 * @detail do deinitialization of camera hardware and detach from vin.
 *	   use it only if there is no deserial here.
 *
 * @param[in] cam_fd: camera handle which create returned
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
extern int32_t hbn_camera_detach_from_vin(camera_handle_t cam_fd);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief attach camera handle to handle of deserial link
 * @detail camera connection info inherit from deserial and initializ hardware.
 *	   use it if there is a deserial which camera connected to.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] des_fd: deserial handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] link: the link index of deerial to attach
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
extern int32_t hbn_camera_attach_to_deserial(camera_handle_t cam_fd, deserial_handle_t des_fd, camera_des_link_t link);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief detach camera handle from deserial handle
 * @detail do deinitialization of camera hardware and detach from deserial
 *
 * @param[in] cam_fd: camera handle which create returned
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
extern int32_t hbn_camera_detach_from_deserial(camera_handle_t cam_fd);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief camera start stream to enable sensor output
 * @detail only operate camera sensor hardware, and not affect internal hardware.
 *
 * @param[in] cam_fd: camera handle which create returned
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
extern int32_t hbn_camera_start(camera_handle_t cam_fd);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief camera stop stream to disable sensor output
 * @detail only operate camera sensor hardware, and not affect internal hardware.
 *
 * @param[in] cam_fd: camera handle which create returned
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
extern int32_t hbn_camera_stop(camera_handle_t cam_fd);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief camera reset operation
 * @detail do stop deinit and init start again to reset the camera sensor.
 *
 * @param[in] cam_fd: camera handle which create returned
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
extern int32_t hbn_camera_reset(camera_handle_t cam_fd);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief change frame frequency of camera sensor output
 * @detail only valid if sensor lib support corresponding function.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] fps: frame frequency per second
 *         range: [1, 120], default: NA
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
extern int32_t hbn_camera_change_fps(camera_handle_t cam_fd, int32_t fps);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief read register value from camera hardware
 * @detail the hardware info should configed in camera_config when created.
 *         the type should adapt the camera sensor used.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] type: device type of camera sensor
 *         range: [CAMERA_SENSOR_REG], CAMERA_EEPROM_REG], default: NA
 * @param[in] reg_addr: address of register to read
 *         range: [0x0, 0xFFFF], default: NA
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
extern int32_t hbn_camera_read_register(camera_handle_t cam_fd, camera_reg_type_t type, uint32_t reg_addr);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief write register value to camera hardware
 * @detail the hardware info should configed in camera_config when created.
 *         the type should adapt the camera sensor used.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] type: device type of camera sensor
 *         range: [CAMERA_SENSOR_REG], CAMERA_EEPROM_REG], default: NA
 * @param[in] reg_addr: address of register to write
 *         range: [0x0, 0xFFFF], default: NA
 * @param[in] value: value of register to write
 *         range: [0x0, 0xFFFF], default: NA
 *
 * @return 0:Success, <0:Failure
 *
 * @data_write None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
extern int32_t hbn_camera_write_register(camera_handle_t cam_fd, camera_reg_type_t type, uint32_t reg_addr, uint32_t value);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief parse the embedded raw data to embed_info struct
 * @detail only valid if sensor lib support corresponding function.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] embed_raw: raw data buffer to parse
 *         range: [!NULL, !NULL], default: NA
 * @param[out] embed_info: embedded info struct to store
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
extern int32_t hbn_camera_parse_emb(camera_handle_t cam_fd, char* embed_raw, struct embed_data_info_s *embed_info);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief update ae info to camera sensor driver
 * @detail only valid if sensor lib support corresponding function.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] ae_info: ae info struct to update
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
extern int32_t hbn_camera_update_ae_info(camera_handle_t cam_fd, camera_ae_info_t *ae_info);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief enable otp to ISP
 * @detail Need to be called before attach to vin
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] otp_data: otp info struct to update
 *         range: [!NULL, !NULL], default: NA
 *
 * @return 0:Success, <0:Failure
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: X5
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
 extern int32_t hbn_camera_enable_otp(camera_handle_t cam_fd, sensor_otp_t *otp_data);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief get parameter info of camera sensor
 * @detail the base param from config and the internal param from eeprom hardware.
 *         only valid if sensor lib support corresponding function.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] type: the param type as camera_param_type_t
 *         range: [CAMERA_SENSOR_PARAM, CAMERA_EEPROM_FULL_PARAM], default: NA
 * @param[out] sp: camera param struct to store
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
extern int32_t hbn_camera_get_sns_info(camera_handle_t cam_fd, camera_param_type_t type, cam_parameter_t *sp);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief set camera event callback func
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] event_callback: camera event callback func
 *         range: [!NULL, !NULL], default: NA
 *
 * @return 0:Success, <0:Failure
 *
 * @data_read None
 * @data_updated None
 * @compatibility None
 *
 * @callgraph
 * @callergraph
 * @design
 */
extern int32_t hbn_camera_set_event_callback(camera_handle_t cam_fd, void (*event_callback)(cam_event_t* fault_info));

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief get camera runtime status
 * @detail get the status as init status, start status, link status, recovery status, etc.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[out] status: camera runtime status struct to store
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
extern int32_t hbn_camera_get_status(camera_handle_t cam_fd, camera_staus_t *status);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief get version info of camera library
 * @detail version info store as sting buff with size mast larger then CAMERA_VERSON_LEN_MAX.
 *
 * @param[in] cam_fd: camera handle which create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] type: library type as camera_version_type_t
 *         range: [CAMERA_CAM_VERSION, CAMERA_TXSER_VERSION], default: NA
 * @param[out] name: library name string buffer to store, NULL ignore
 *         range: [NULL, !NULL], default: NA
 * @param[out] version: library version string buffer to store
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
extern int32_t hbn_camera_get_version(camera_handle_t cam_fd, camera_version_type_t type, char *name, char *version);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief get camera handle by vin handle attached or camera index
 * @detail get by vin handle if valid, else get by camera index.
 *
 * @param[in] vin_fd: vin handle if attached to get
 *         range: [NULL, !NULL], default: NA
 * @param[in] camera_index: camera port index
 *         range: [0, NA], default: NA
 *
 * @return !NULL-camera handle point matched, NULL-not found
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
extern camera_handle_t hbn_camera_get_handle(vpf_handle_t vin_fd, int32_t camera_index);

/**
 * @NO{S10E02C01I}
 * @ASIL{B}
 * @brief camera init with config json file
 * @detail all camera and deserial config should included in json file.
 *         it will auto create camera handle and attach them.
 *
 * @param[in] cfg_file: camera config json file path
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
extern int32_t hbn_camera_init_cfg(const char *cfg_file);


/**
 * @description: Get both the git commit hash and version of camsys libcam.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libcam_get_version_info(void);

extern int32_t hbn_camera_switch_scence(int64_t vflow_fd, int64_t isp_vnode_fd, const char *cname);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HB_CAMERA_INTERFACE_H__ */
