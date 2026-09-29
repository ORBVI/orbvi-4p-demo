/*************************************************************************
 *                     COPYRIGHT NOTICE
 *            Copyright 2021 Horizon Robotics, Inc.
 *                   All rights reserved.
 *************************************************************************/
#ifndef INC_HB_MEM_PC_ALLOC_H_
#define INC_HB_MEM_PC_ALLOC_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

/**
 * @struct hb_mem_pc_handle
 * @brief hbmem pc buffer handle
 * @NO{S21E04C02I}
 */
typedef struct hb_mem_pc_handle {
	int32_t fd;		/**< the file descriptor*/
	uint64_t size;	/**< the buffer size*/
	uint64_t phys;	/**< the physcial address*/
	void *vaddr;	/**< the virtual address*/
} *hb_mem_pc_handle_t;

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief alloc hobot memory
 *
 * @param[in] size: Size of memory space to be allocated
					range: (0, ); default: 0
 * @param[in] flag: Request identifier @hbmem_backends
 * @param[in] lable: String to identify the memory space
 *
 * @retval "NULL": failed
 * @retval "handle": hbmem pc buffer handle
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
hb_mem_pc_handle_t hb_mem_pc_alloc(uint32_t size, uint64_t flag, const char* label);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief free hobot memory
 *
 * @param[in] handle: hbmem pc buffer handle
 *
 * @return None
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
void hb_mem_pc_free(hb_mem_pc_handle_t handle);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Invalid operation on hidden objects in hbmem memory space
 *
 * @param[in] handle: the hbmem pc buffer handle
 * @param[in] size: the invalid size
					range: (0, ); default: 0
 *
 * @return None
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
void hb_mem_pc_cache_invalid(hb_mem_pc_handle_t handle, uint32_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Clean operation on hidden objects in hbmem memory space
 *
 * @param[in] handle: the hbmem pc buffer handle
 * @param[in] size: the clean size
					range: (0, ); default: 0
 *
 * @return None
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
void hb_mem_pc_cache_clean(hb_mem_pc_handle_t handle, uint32_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the actual ddr physical address corresponding to the hbmem memory space address
 *
 * @param[in] handle: the hbmem pc buffer handle
 *
 * @retval "=0": failed
 * @retval "!=0": succeed,physical address
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
uint64_t hb_mem_pc_phyaddr(hb_mem_pc_handle_t handle);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the real virtual address corresponding to the hbmem memory space address
 *
 * @param[in] handle: the hbmem pc buffer handle
 *
 * @retval "=0": Invalid virtual address
 * @retval "!=0": virtual address
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
uint64_t hb_mem_pc_virtaddr(hb_mem_pc_handle_t handle);

/**
 * @description: Get both the git commit hash and version of libhbmem.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libhbmem_get_version_info(void);

#ifdef __cplusplus
}
#endif
#endif /* INC_HB_MEM_PC_ALLOC_H_ */
