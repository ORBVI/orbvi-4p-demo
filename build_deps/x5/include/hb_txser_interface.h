/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

/**
 * @file hb_txser_interface.h
 *
 * @NO{S10E02C03}
 * @ASIL{B}
 */

#ifndef __HB_TXSER_INTERFACE_H__
#define __HB_TXSER_INTERFACE_H__

#include "hb_camera_data_config.h"
#include "hb_camera_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @NO{S10E02C03I}
 * @ASIL{B}
 * @brief creat txser handle with txser config
 * @detail the txser config should adapt the serial hardware used.
 *	   use it only if there is a serial which connected to tx.
 *
 * @param[in] txs_config: txser config with txser_config_t stuct
 *         range: [!NULL, !NULL], default: NA
 * @param[out] txs_fd: txser handle fd created return
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
extern int32_t hbn_txser_create(txser_config_t *txs_config, txser_handle_t *txs_fd);

/**
 * @NO{S10E02C03I}
 * @ASIL{B}
 * @brief destroy txser handle to exit
 * @detail all handle created should be destroyed at last.
 *	   use it only if there is a serial which connected to tx.
 *
 * @param[in] txs_fd: txser handle with create returned
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
extern int32_t hbn_txser_destroy(txser_handle_t txs_fd);

/**
 * @NO{S10E02C03I}
 * @ASIL{B}
 * @brief attach txser handle to handle of vin node in vpf
 * @detail get connection info from vin and initializ th serial of tx hardware.
 *	   use it only if there is a serial which connected to tx.
 *
 * @param[in] txs_fd: txser handle with create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] csi: txser csi index to attach
 *         range: [0, 1], default: NA
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
extern int32_t hbn_txser_attach_to_vin(txser_handle_t txs_fd, camera_txs_csi_t csi, vpf_handle_t vin_fd);

/**
 * @NO{S10E02C03I}
 * @ASIL{B}
 * @brief detach txser handle from handle of vin node in vpf
 * @detail do deinitialization of txser(and tx) hardware and detach from vin.
 *	   use it only if there is a serial which connected to tx.
 *
 * @param[in] txs_fd: txser handle with create returned
 *         range: [!NULL, !NULL], default: NA
 * @param[in] csi: txser csi index to detach
 *         range: [0, 1], default: NA
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
extern int32_t hbn_txser_detach_from_vin(txser_handle_t txs_fd, camera_txs_csi_t csi);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _HB_DESERIAL_INTERFACE_H__ */
