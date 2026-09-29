/***
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2019 -2023, Horizon Robotics Co., Ltd.
 *                   All rights reserved.
 ***/
/**
 * @file hbmem.h
 *
 * @NO{S21E04C02I}
 * @ASIL{B}
 */
#ifndef HBMEM_H
#define HBMEM_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#define HBMEM_VERSION_MAJOR 0	/**< major number of hbmem lib version*/
#define HBMEM_VERSION_MINOR 1	/**< minor number of hbmem lib version*/
#define HBMEM_VERSION_PATCH 1	/**< patch number of hbmem lib version*/

#define hbmem_addr_t uint64_t	/**< define the hbmem paddr.*/

/**
 * @enum hbmem_backends
 * Describe hbmem backends flags
 */
enum hbmem_backends {
	BACKEND_ION_CMA,		/**< ion cma heap.*/
	BACKEND_ION_CARVEOUT,	/**< ion carveout heap.*/
	BACKEND_ION_SRAM,		/**< ion sram heap.*/

	BACKEND_MAX				/**< backend max number.*/
};

/* hbmem alloc flag type */

/**
 * @def BACKEND_FLAG
 * @brief flag direct to backend, _flag: flag
 */
#define BACKEND_FLAG(_flag)	((uint64_t)_flag << 32)

/**
 * @def BACKEND_TYPE
 * @brief backend type which value from enum hbmem_backends, backend: backend enum @hbmem_backends
 */
#define BACKEND_TYPE(backend)	((uint64_t)backend << 24)

#define MEM_CACHEABLE ((uint64_t)0x1U << 0)	/**< memory cacheable flag*/

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get the hbmem version
 *
 * @param[out] major: Major version number
 * @param[out] minor: Minor version number
 * @param[out] patch: Patch version number
 *
 * @retval "0": succeed
 * @retval "-1": failed
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hbmem_version(uint32_t *major, uint32_t *minor, uint32_t *patch);

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
 * @retval "=0": failed
 * @retval "!=0": succeed
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
hbmem_addr_t hbmem_alloc(uint32_t size, uint64_t flag, const char* label);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief free hobot memory
 *
 * @param[in] addr: the return value of hbmem_alloc
					range: (0, ); default: 0
 *
 * @return None
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
void hbmem_free(hbmem_addr_t addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Map the memory space of the known physical address with hbmem
 *
 * @param[in] phyaddr: Start address (page alignment)
					   range: (0, ); default: 0
 * @param[in] size: Size of mmap
					range: (0, ); default: 0
 * @param[in] flag: Request identifier @hbmem_backends
 *
 * @retval "=0": failed
 * @retval "!=0": succeed
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
hbmem_addr_t hbmem_mmap(uint64_t phyaddr, uint32_t size, uint64_t flag);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Use share_id maps the memory space of the known physical address to hbmem
 *
 * @param[in] phyaddr: Start address (page alignment)
					   range: (0, ); default: 0
 * @param[in] size: Map Size
					range: (0, ); default: 0
 * @param[in] flag: Request identifier @hbmem_backends
 * @param[in] share_id: the share id
						range: [0, ); default: 0
 *
 * @retval "=0": failed
 * @retval "!=0": succeed
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
hbmem_addr_t hbmem_mmap_with_share_id(uint64_t phyaddr, uint32_t size,
				uint64_t flag, int32_t share_id);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief release the hbmem_mmap
 *
 * @param[in] addr: the return value of hbmem_alloc
					range: (0, ); default: 0
 *
 * @return None
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
void hbmem_munmap(hbmem_addr_t addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Use the system's dma to complete the copy operation of data in two hbmem memory spaces
 *
 * @param[in] dst: Start address of target hbmem space
				   range: (0, ); default: 0
 * @param[in] src: Start address of source hbmem space
				   range: (0, ); default: 0
 * @param[in] size: Memory space size to be copied
					range: (0, ); default: 0
 *
 * @retval "=0": succeed
 * @retval "<0": failed
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hbmem_dmacpy(hbmem_addr_t dst, hbmem_addr_t src, uint32_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the hidden object type of hbmem space corresponding to hbmem_addr_t
 *
 * @param[in] addr: the return value of hbmem_alloc
					range: (0, ); default: 0
 *
 * @retval "=0": uncacheable
 * @retval ">0": cacheable
 * @retval "<0": invalid hbmem_addr_t
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hbmem_is_cacheable(hbmem_addr_t addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Invalid operation on hidden objects in hbmem memory space
 *
 * @param[in] addr: the return value of hbmem_alloc
					range: (0, ); default: 0
 * @param[in] size: the invalid size
					range: (0, ); default: 0
 *
 * @return None
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
void hbmem_cache_invalid(hbmem_addr_t addr, uint32_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Clean operation on hidden objects in hbmem memory space
 *
 * @param[in] addr: the return value of hbmem_alloc
					range: (0, ); default: 0
 * @param[in] size: the clean size
					range: (0, ); default: 0
 *
 * @return None
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
void hbmem_cache_clean(hbmem_addr_t addr, uint32_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the actual ddr physical address corresponding to the hbmem memory space address
 *
 * @param[in] addr: valid hbmem_addr_t
					range: (0, ); default: 0
 *
 * @retval "=0": failed
 * @retval "!=0": succeed,physical address
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
uint64_t hbmem_phyaddr(hbmem_addr_t addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the real virtual address corresponding to the hbmem memory space address
 *
 * @param[in] addr: the return value of hbmem_alloc
					range: (0, ); default: 0
 *
 * @retval "=0": Invalid virtual address
 * @retval "!=0": virtual address
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
uint64_t hbmem_virtaddr(hbmem_addr_t addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get incoming hbmem_addr_t address value information
 *
 * @param[in] addr: the return value of hbmem_alloc
					range: (0, ); default: 0
 * @param[out] start: Corresponding starting virtual address
 * @param[out] size: the hbmem alloc size
 *
 * @retval "=0": succeed
 * @retval "<0": the hbmem_addr_t is not in hbmem space
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hbmem_info(hbmem_addr_t addr, hbmem_addr_t *start, uint32_t *size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the share_id of virtual address
 *
 * @param[in] addr: the return value of hbmem_alloc
					range: (0, ); default: 0
 * @param[out] share_id: the share_id
 *
 * @retval "=0": succeed
 * @retval "<0": Virtual address not found
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hbmem_get_share_id(hbmem_addr_t addr, int32_t *share_id);

#ifdef __cplusplus
}
#endif
#endif
