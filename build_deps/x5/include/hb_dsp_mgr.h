/*************************************************************************
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2020-2023, Horizon Robotics Co., Ltd.
 *                    All rights reserved.
 *************************************************************************/
/**
 * @file hb_dsp_mgr.h
 *
 * @NO{S05E06C01I}
 * @ASIL{B}
 */

#ifndef HB_DSP_MGR_H
#define HB_DSP_MGR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <hb_mem_mgr.h>

/* error code */
#define HB_DSP_OK						(0)
#define HB_DSP_OPEN_DEV_ERR					(-1)
#define HB_DSP_START_IOCTL_ERR					(-2)
#define HB_DSP_STOP_IOCTL_ERR					(-4)
#define HB_DSP_GET_STATUS_IOCTL_ERR				(-5)
#define HB_DSP_RESET_IOCTL_ERR					(-6)
#define HB_DSP_PARAM_INVALID					(-7)
#define HB_DSP_SET_PATH_ERR					(-8)
#define HB_DSP_SET_NAME_ERR					(-9)
#define HB_DSP_CLOSE_DEV_ERR					(-10)
#define HB_DSP_ERR_IOC_GET_VERSION_INFO			(-11)
#define HB_DSP_ERR_IOC_MEM_ALLOC				(-12)
#define HB_DSP_ERR_IOC_MEM_FREE				(-13)
#define HB_DSP_ERR_IOC_SMMU_MAP				(-14)
#define HB_DSP_ERR_IOC_SMMU_UNMAP				(-15)
#define HB_DSP_ERR_INIT					(-16)
#define HB_DSP_ERR_DEINIT					(-17)
#define HB_DSP_ERR_NOT_INITED					(-18)
#define HB_DSP_ERR_ALLOC_COM_BUF				(-19)
#define HB_DSP_ERR_FREE_MEMBUF					(-20)
#define HB_DSP_ERR_GET_COM_BUF_WITH_VADDR			(-21)
#define HB_DSP_ERR_SMMU_MAP					(-22)
#define HB_DSP_ERR_SMMU_UNMAP					(-23)
#define HB_DSP_ERR_CHECK_VERSION				(-24)
#define HB_DSP_ERR_INSUFFICIENT_MEM				(-25)
#define HB_DSP_ERR_MISMATCH_INTERFACE				(-26)
#define HB_DSP_ERR_RBTREE_CREATE_NODE				(-27)
#define HB_DSP_ERR_RBTREE_INSERT_NODE				(-28)
#define HB_DSP_ERR_RBTREE_SEARCH_NODE				(-29)

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief get the dsp library version.
 *
 * @param[out] major: major version number
 * @param[out] minor: minor version number
 * @param[out] patch: patch version number
 *
 * @retval "0": succeed
 * @retval HB_DSP_PARAM_INVALID: invalid parameter
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_get_version(uint32_t *major, uint32_t *minor, uint32_t *patch);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief start dsp
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[in] timeout: timeout
 *			  =0:asynchronous start
 *			  <0:synchronous start, unlimited waiting
 *			  >0:synchronous start, wait time(ms)
 * @param[in] pathname: dsp image path(length<256) & name(length<256)
 *			NULL: use default path & name
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_START_IOCTL_ERR: ioctl error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_start(int32_t dsp_id, int32_t timeout, const char* pathname);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief stop dsp
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_STOP_IOCTL_ERR: stop ioctl error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_stop(int32_t dsp_id);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief get dsp status
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[out] status: dsp status
 *			    OFFLINE = 0,
 *				RUNNING = 2,
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_GET_STATUS_IOCTL_ERR: ioctl error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_get_status(int32_t dsp_id, int32_t *status);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief reset dsp
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_RESET_IOCTL_ERR: reset ioctl error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_reset(int32_t dsp_id);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief set dsp fw path
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[in] strpath: dsp image path(length<256) without name
 *			NULL: use default path
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_SET_PATH_ERR: set path ioctl error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_set_path(int32_t dsp_id, const char* strpath);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief set dsp name
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[in] name: dsp image name(length<256)
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_SET_NAME_ERR: set name ioctl error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_set_name(int32_t dsp_id, const char* name);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief dsp get fd
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[out] retfd: file descriptor
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_get_fd(int32_t dsp_id, int32_t *retfd);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief dsp close fd
 *
 * @param[in] dspfd: file descriptor
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 1.0.0
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_close_fd(int32_t dspfd);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief dsp init
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_INSUFFICIENT_MEM: dsp malloc error
 * @retval HB_DSP_OPEN_DEV_ERR: open fd failed
 * @retval HB_DSP_ERR_CHECK_VERSION: check version failed
 * @retval HB_DSP_ERR_INIT: dsp init error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_init(int32_t dsp_id);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief dsp deinit
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_ERR_DEINIT: dsp deinit error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_deinit(int32_t dsp_id);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief alloc hbmem and map
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[in] size: alloc size
 * @param[in] flags: buffer flags @mem_usage_t
 * @param[out] va: virtual address of allocated mem
 * @param[out] iova: mapped device address by the smmu
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_ERR_ALLOC_COM_BUF: alloc hbmem buf error
 * @retval HB_DSP_ERR_SMMU_MAP: smmu map error
 * @retval HB_DSP_ERR_RBTREE_CREATE_NODE: rbtree create node error
 * @retval HB_DSP_ERR_RBTREE_INSERT_NODE: rbtree insert node error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_mem_alloc(int32_t dsp_id, uint64_t size, int64_t flags, uint64_t *va, uint64_t *iova);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief free hbmem and unmap
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[in] va: virtual address of allocated mem
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_ERR_RBTREE_SEARCH_NODE: rbtree search node error
 * @retval HB_DSP_ERR_MISMATCH_INTERFACE: interface mismatch
 * @retval HB_DSP_ERR_SMMU_UNMAP: smmu unmap error
 * @retval HB_DSP_ERR_FREE_MEMBUF: free hbmem com buf error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_mem_free(int32_t dsp_id, uint64_t va);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief smmu map
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[in] va: virtual address of allocated mem
 * @param[in] size: map size
 * @param[out] dspiova: mapped device address by the smmu
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_ERR_GET_COM_BUF_WITH_VADDR: get hbmem com buf error
 * @retval HB_DSP_ERR_SMMU_MAP: smmu map error
 * @retval HB_DSP_ERR_RBTREE_CREATE_NODE: rbtree create node error
 * @retval HB_DSP_ERR_RBTREE_INSERT_NODE: rbtree insert node error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_mmu_map(int32_t dsp_id, uint64_t va, uint64_t size, uint64_t *dspiova);

/**
 * @NO{S05E06C01I}
 * @ASIL{B}
 * @brief smmu unmap
 *
 * @param[in] dsp_id: dsp id, range:[0,1]
 * @param[in] va: virtual address of allocated mem
 *
 * @retval HB_DSP_OK: successful
 * @retval HB_DSP_PARAM_INVALID: invalid params
 * @retval HB_DSP_ERR_NOT_INITED: get dsp mgr error
 * @retval HB_DSP_ERR_RBTREE_SEARCH_NODE: rbtree search node error
 * @retval HB_DSP_ERR_MISMATCH_INTERFACE: interface mismatch
 * @retval HB_DSP_ERR_SMMU_UNMAP: smmu map error
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.0.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_dsp_mmu_unmap(int32_t dsp_id, uint64_t va);

/**
 * @description: Get both the git commit hash and version of libdsp.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libdsp_get_version_info(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*HB_DSP_MGR_H*/
