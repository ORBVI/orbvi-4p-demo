/***
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2019 -2023, Horizon Robotics Co., Ltd.
 *                   All rights reserved.
 ***/
/**
 * @file hb_mem_mgr.h
 *
 * @NO{S21E04C02I}
 * @ASIL{B}
 */
#ifndef HB_MEM_MGR_H
#define HB_MEM_MGR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_GRAPHIC_BUF_COMP 3		/**< max graphic buffer number.*/

/**
 * @enum mem_pixel_format_t
 * @brief Define the buffer format of graphic buffer.
 * @NO{S21E04C02I}
 */
typedef enum mem_pixel_format_t {
	MEM_PIX_FMT_NONE = -1,	/**< none number.*/

	MEM_PIX_FMT_RGB565,		/**< packed RGB 5:6:5, 16bpp,*/
	MEM_PIX_FMT_RGB24,		/**< packed RGB 8:8:8, 24bpp, RGBRGB...*/
	MEM_PIX_FMT_BGR24,		/**< packed RGB 8:8:8, 24bpp, BGRBGR...*/
	MEM_PIX_FMT_ARGB,		/**< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...*/
	MEM_PIX_FMT_RGBA,		/**< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...*/
	MEM_PIX_FMT_ABGR,		/**< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...*/
	MEM_PIX_FMT_BGRA,		/**< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...*/
	MEM_PIX_FMT_YUV420P,	/**< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples) */
	MEM_PIX_FMT_NV12,		/**< planar YUV 4:2:0, 12bpp, 1 plane for Y and
							 * 1 plane for the UV components, which are
							 * interleaved (first byte U and the following
							 * byte V).*/
	MEM_PIX_FMT_NV21,		/**< as above, but U and V bytes are swapped */
	MEM_PIX_FMT_YUV422P,	/**< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples) */
	MEM_PIX_FMT_NV16,		/**< interleaved chroma (first byte U and the following byte V)
							 * YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
							 */
	MEM_PIX_FMT_NV61,		/**< * interleaved chroma (first byte V and the following byte U)
							 * YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
							 */
	MEM_PIX_FMT_YUYV422,	/**< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr */
	MEM_PIX_FMT_YVYU422,	/**< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb */
	MEM_PIX_FMT_UYVY422,	/**< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1 */
	MEM_PIX_FMT_VYUY422,	/**< packed YUV 4:2:2, 16bpp, Cr Y0 Cb Y1 */
	MEM_PIX_FMT_YUV444,		/**< packed YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
	MEM_PIX_FMT_YUV444P,	/**< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
	MEM_PIX_FMT_NV24,		/**< interleaved chroma (first byte U and the following byte V)
							 * YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)*/
	MEM_PIX_FMT_NV42,		/**< interleaved chroma (first byte V and the following byte U)
							 * YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)*/
	MEM_PIX_FMT_YUV440P,	/**< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples) */
	MEM_PIX_FMT_YUV400,		/**< Gray Y, YUV 4:0:0 */
	MEM_PIX_FMT_RAW8,		/**< raw8 format*/
	MEM_PIX_FMT_RAW10,		/**< raw10 format*/
	MEM_PIX_FMT_RAW12,		/**< raw12 format*/
	MEM_PIX_FMT_RAW14,		/**< raw14 format*/
	MEM_PIX_FMT_RAW16,		/**< raw16 format*/
	MEM_PIX_FMT_RAW20,		/**< raw20 format*/
	MEM_PIX_FMT_RAW24,		/**< raw24 format*/
	MEM_PIX_FMT_TOTAL,		/**< the number of pix format*/
} mem_pixel_format_t;

/**
 * @enum mem_usage_t
 * @brief Define the usage of buffer. You can combine different usages as a final flags. Undefined usage will be ignored.
 * @NO{S21E04C02I}
 */
typedef enum mem_usage_t {
	HB_MEM_USAGE_CPU_READ_NEVER          = 0x00000000LL,	/**< buffer is never read by cpu */
	HB_MEM_USAGE_CPU_READ_OFTEN          = 0x00000001LL,	/**< buffer is often read by cpu */
	HB_MEM_USAGE_CPU_READ_MASK           = 0x0000000FLL,	/**< mask for the software read and write flag */

	HB_MEM_USAGE_CPU_WRITE_NEVER         = 0x00000000LL,	/**< buffer is never write by cpu */
	HB_MEM_USAGE_CPU_WRITE_OFTEN         = 0x00000010LL,	/**< buffer is often write by cpu, on arm platform *WRITE implies *READ*/
	HB_MEM_USAGE_CPU_WRITE_MASK          = 0x000000F0LL,	/**< mask for the software read and write flag */

	HB_MEM_USAGE_HW_CIM                  = 0x00000100LL,	/**< buffer will be used as camera interface module buffer */
	HB_MEM_USAGE_HW_PYRAMID              = 0x00000200ULL,	/**< buffer will be used as pyramid buffer */
	HB_MEM_USAGE_HW_GDC                  = 0x00000400LL,	/**< buffer will be used as geometric distortion correction buffer */
	HB_MEM_USAGE_HW_STITCH               = 0x00000800LL,	/**< buffer will be used as stitch buffer */
	HB_MEM_USAGE_HW_OPTICAL_FLOW         = 0x00001000ULL,	/**< buffer will be used as optical flow buffer */
	HB_MEM_USAGE_HW_BPU                  = 0x00002000LL,	/**< buffer will be used as bpu buffer */
	HB_MEM_USAGE_HW_ISP                  = 0x00004000LL,	/**< buffer will be used as isp buffer */
	HB_MEM_USAGE_HW_DISPLAY              = 0x00008000LL,	/**< buffer will be used as display buffer */
	HB_MEM_USAGE_HW_VIDEO_CODEC          = 0x00010000LL,	/**< buffer will be used as video codec buffer */
	HB_MEM_USAGE_HW_JPEG_CODEC           = 0x00020000LL,	/**< buffer will be used as jpeg codec buffer */
	HB_MEM_USAGE_HW_VDSP                 = 0x00040000LL,	/** buffer will be used as vdsp buffer */
	HB_MEM_USAGE_HW_GDC_OUT              = 0x00080000LL,	/**< buffer will be used as geometric distortion correction out buffer */
	HB_MEM_USAGE_HW_IPC                  = 0x00100000LL,	/**< buffer will be used as ipc buffer.*/
	HB_MEM_USAGE_HW_PCIE                 = 0x00200000LL,	/**< buffer will be used as pcie buffer.*/
	HB_MEM_USAGE_HW_YNR                  = 0x00400000LL,	/**< buffer will be used as ynr buffer.*/
	HB_MEM_USAGE_HW_MASK                 = 0x00FFFF00LL,	/**< mask for the hw flag */

	HB_MEM_USAGE_MAP_INITIALIZED         = 0x01000000LL,	/**< buffer will be initialized */
	HB_MEM_USAGE_MAP_UNINITIALIZED       = 0x02000000LL,	/**< buffer will not be initialized */
	HB_MEM_USAGE_CACHED                  = 0x04000000LL,	/**< buffer will not be initialized, unchangable */
	HB_MEM_USAGE_GRAPHIC_CONTIGUOUS_BUF  = 0x08000000LL,	/**< graphic buffer will be contiguous */
	HB_MEM_USAGE_MEM_POOL				 = 0x10000000LL,	/**< It only indicates the buffer is a memory pool. Don't use this flag */
	HB_MEM_USAGE_MEM_SHARE_POOL			 = 0x20000000LL,	/* It only indicates the buffer is a share pool. Don't use this flag
															 * to allocate buffer. It's useless.*/
	HB_MEM_USAGE_TRIVIAL_MASK            = 0xFF000000LL,	/**< mask for the trivial flag.*/

	HB_MEM_USAGE_PRIV_HEAP_DMA           = 0x000000000LL,	/**< buffer will be allocated from CMA heap in linux system */
	HB_MEM_USAGE_PRIV_HEAP_RESERVERD     = 0x100000000LL,	/**< buffer will be allocated from Carveout heap in linux system */
	HB_MEM_USAGE_PRIV_HEAP_RESERVED      = 0x100000000LL,	/**< buffer will be allocated from Carveout heap in linux system */
	HB_MEM_USAGE_PRIV_HEAP_SRAM          = 0x200000000LL,	/**< buffer will be allocated from SRAM heap in linux system */
	HB_MEM_USAGE_PRIV_HEAP_2_RESERVERD   = 0x400000000LL,	/**< buffer will be allocated from another Carveout heap in linux syste.*/
	HB_MEM_USAGE_PRIV_HEAP_2_RESERVED    = 0x400000000LL,	/**< buffer will be allocated from another Carveout heap in linux syste.*/
	HB_MEM_USAGE_PRIV_HEAP_SRAM_LIMIT    = 0x800000000LL,	/**< buffer will be allocated from sram limit heap.*/
	HB_MEM_USAGE_PRIV_HEAP_INLINE_ECC    = 0x1000000000LL,	/**< buffer will be allocated from inline ecc heap.*/
	HB_MEM_USAGE_PRIV_MASK               = 0xFF00000000LL,	/**< mask for the private flag */
} mem_usage_t;

/**
 * @enum hb_mem_buffer_type_t
 * @brief Define the buffer type
 * @NO{S21E04C02I}
 */
typedef enum hb_mem_buffer_type_t {
	HB_MEM_BUFFER_TYPE_COMMON,	/**< buffer type for common buffer */
	HB_MEM_BUFFER_TYPE_GRAPHIC,	/**< buffer type for graphic buffer */
	HB_MEM_BUFFER_TYPE_GRAPHIC_GROUP,	/**< buffer type for graphic buffer group*/
	HB_MEM_BUFFER_TYPE_MAX,	/**< the buffer type max index*/
} hb_mem_buffer_type_t;

/**
 * @struct hb_mem_common_buf_t
 * Define the descriptor of common buffer.
 * @NO{S21E04C02I}
 */
typedef struct hb_mem_common_buf_t {
	int32_t fd;			/**< File descriptors of the buffer.*/
	int32_t share_id;	/**< Share id of the buffer.*/
/**
 * - Default: 0
 */
	int64_t flags;		/**< Buffer flags for allocation */
	uint64_t size;		/**< Total Buffer size specified by user during allocation.*/
	uint8_t *virt_addr;	/**< Buffer starting virtual address.*/
/**
 * - Note: It's not recommended to use or transfer the physical address directly.
 *         There is no memory object to keep track of the physical address.
 */
	uint64_t phys_addr;	/**< Buffer starting physical address.*/
/**
 * - Default: 0
 */
	uint64_t offset;	/**< Buffer offset.*/
} hb_mem_common_buf_t;

/**
 * @struct hb_mem_graphic_buf_t
 * Define the descriptor of graphic buffer.
 * @NO{S21E04C02I}
 */
typedef struct hb_mem_graphic_buf_t {
	int32_t fd[MAX_GRAPHIC_BUF_COMP];	/**< File descriptors of the buffer for each component.*/
/**
 * Values [1, MAX_GRAPHIC_BUF_COMP]
 */
	int32_t plane_cnt;					/**< Plane count of this graphic buffer*/
	int32_t format;						/**< Buffer format for allocation.@mem_pixel_format_t*/
	int32_t width;						/**< Buffer width.*/
	int32_t height;						/**< Buffer height.*/
	int32_t stride;						/**< Buffer horizontal stride.*/
	int32_t vstride;					/**< Buffer vertical stride.*/
/**
 * - Default: 0
 */
	int32_t is_contig;					/**< Buffer physical memory is contiguous.@mem_usage_t*/
	int32_t share_id[MAX_GRAPHIC_BUF_COMP];	/**< Share id of the buffer.*/
/**
 * - Default: 0
 */
	int64_t flags;						/**< Buffer flags for allocation.@mem_usage_t*/
	uint64_t size[MAX_GRAPHIC_BUF_COMP];	/**< Total Buffer size for each component.*/
	uint8_t *virt_addr[MAX_GRAPHIC_BUF_COMP];	/**< Buffer virtual address for each component.*/
/**
 * - Note: It's not recommended to use or transfer the physical address directly.
 *         There is no memory object to keep track of the physical address.
 */
	uint64_t phys_addr[MAX_GRAPHIC_BUF_COMP];	/**< Buffer physical address for each component.*/
/**
 * - Default: 0
 */
	uint64_t offset[MAX_GRAPHIC_BUF_COMP];	/**< Buffer offset.*/
} hb_mem_graphic_buf_t;

#define HB_MEM_MAXIMUM_GRAPH_BUF 8		/**< max graphic buffer number in graphic buffer group*/

/**
 * @struct hb_mem_graphic_buf_group_t
 * Define the descriptor of graphic buffer group
 * @NO{S21E04C02I}
 */
typedef struct hb_mem_graphic_buf_group_t {
        hb_mem_graphic_buf_t graph_group[HB_MEM_MAXIMUM_GRAPH_BUF];	/**graphic buffer array*/
        int32_t group_id;		/**< graphic buffer group id which alloc from ION driver*/
        uint32_t bit_map;		/**< graphic buffer group bitmap*/
} hb_mem_graphic_buf_group_t;

/**
 * @struct hb_mem_buf_queue_t
 * Define the descriptor of buffer queue.
 * @NO{S21E04C02I}
 */
typedef struct hb_mem_buf_queue_t {
	uint64_t unique_id;	/**< Uinque id specified by memory manager. Should no be modified.*/
	uint32_t count;		/**< Total items of the buffer queue.*/
	uint32_t item_size;	/**< Size of each item.*/
} hb_mem_buf_queue_t;

/**
 * @struct hb_mem_pool_t
 * Define the descriptor of memory pool.
 * @NO{S21E04C02I}
 */
typedef struct hb_mem_pool_t {
/**
 * - Default: 0
 */
	int64_t flags;		/**< Buffer flags for allocation.@mem_usage_t*/
	uint64_t size;		/**< Total Buffer size specified by user during allocation.*/
	int32_t fd;			/**< File descriptors of the pool.*/
	int32_t page_size;	/**< Page size in byte.*/
	int32_t total_page_cnt;	/**< Total page count.*/
	int32_t avail_page_cnt;	/**< Available page count.*/
	int32_t cur_client_cnt;	/**< Current pool client count.*/

	int32_t reserved;		/**< reserved*/
} hb_mem_pool_t;

/**
 * @struct hb_mem_share_pool_t
 * Define the descriptor of share memory pool.
 * @NO{S21E04C02I}
 */
typedef struct hb_mem_share_pool_t {
/**
 * - Default: 0
 */
	int64_t flags;		/**< Buffer flags for allocation.@mem_usage_t*/
	uint32_t size;		/**< Buffer size specified by user during allocation.*/
	int32_t fd;			/**< File descriptors of the pool.*/
	int32_t total_buf_cnt;	/**< Total buf count.*/
	int32_t avail_buf_cnt;	/**< Available buffer count.*/

	int32_t reserved;		/**< reserved*/
} hb_mem_share_pool_t;

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the memory module version.
 *
 * @param[out] major: major version number
 * @param[out] minor: minor version number
 * @param[out] patch: patch version number
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
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
int32_t hb_mem_get_version(uint32_t *major, uint32_t *minor, uint32_t *patch);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Open the memory module.
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_OPEN_FAIL": Unable to open memory module
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
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
int32_t hb_mem_module_open(void);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Close memory module.
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
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
int32_t hb_mem_module_close(void);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief alloc common buffer
 *
 * @param[in] size: buffer size
					range: (0, ); default: 0
 * @param[in] flags: buffer flags @mem_usage_t
 * @param[out] buf: common buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
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
int32_t hb_mem_alloc_com_buf(uint64_t size, int64_t flags,
			hb_mem_common_buf_t * buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get common buffer info by fd
 *
 * @param[in] fd: file descriptor
				  range: [0, ); default: 0
 * @param[out] buf: common buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
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
int32_t hb_mem_get_com_buf(int32_t fd, hb_mem_common_buf_t *buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief alloc graphic buffer
 *
 * @param[in] w: Image width
				 range: (0, ); default: 0
 * @param[in] h: Image height
				 range: (0, ); default: 0
 * @param[in] format: Image format @mem_pixel_format_t
 * @param[in] flags: buffer flag @mem_usage_t
 * @param[in] stride: Image horizontal span
					  range: (0, ); default: 0
 * @param[in] vstride: Image vertical span
					   range: (0, ); default: 0
 * @param[out] buf: graphic buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
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
int32_t hb_mem_alloc_graph_buf(int32_t w, int32_t h, int32_t format, int64_t flags,
			int32_t stride, int32_t vstride, hb_mem_graphic_buf_t * buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get graphic buffer info by fd
 *
 * @param[in] fd: file descriptor
				  range: [0, ); default: 0
 * @param[out] buf: graphic buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
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
int32_t hb_mem_get_graph_buf(int32_t fd, hb_mem_graphic_buf_t *buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief free buffer by fd
 *
 * @param[in] fd: file descriptor
				  range: [0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
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
int32_t hb_mem_free_buf(int32_t fd);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief invalidate buffer by fd
 *
 * @param[in] fd: file descriptor
				  range: [0, ); default: 0
 * @param[in] offset: address offset in buffer
					  range: [0, ); default: 0
 * @param[in] size: invlidate size
					range: (0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
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
int32_t hb_mem_invalidate_buf(int32_t fd, uint64_t offset, uint64_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief flush buffer by fd
 *
 * @param[in] fd: file descriptor
				  range: [0, ); default: 0
 * @param[in] offset: address offset in buffer
					  range: [0, ); default: 0
 * @param[in] size: flush size
					range: (0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
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
int32_t hb_mem_flush_buf(int32_t fd, uint64_t offset, uint64_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief flush buffer by fd
 *
 * @param[in] virt_addr: Virtual address
						 range: (0, ); default: 0
 * @param[in] size: buffer size
					range: (0, ); default: 0
 * @param[out] valid: Whether the virtual address is valid, 0: invalid, 1: valid
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
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
int32_t hb_mem_is_valid_buf(uint64_t virt_addr, uint64_t size, int32_t *valid);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the physical address information corresponding to the input virtual address
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 * @param[out] phys_addr: Corresponding physical address
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
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
int32_t hb_mem_get_phys_addr(uint64_t virt_addr, uint64_t * phys_addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the starting virtual address and buffer size information corresponding to the input virtual address
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 * @param[out] start: Corresponding starting virtual address
 * @param[out] size: buffer size
 * @param[out] flags: buffer flag
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_get_buf_info(uint64_t virt_addr, uint64_t *start, uint64_t *size, int64_t *flags);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief invalidate buffer with virtual address
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 * @param[in] size: invalidate buffer size
					range: (0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_invalidate_buf_with_vaddr(uint64_t virt_addr, uint64_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief flush buffer with virtual address
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 * @param[in] size: flush buffer size
					range: (0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_flush_buf_with_vaddr(uint64_t virt_addr, uint64_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get common buffer with virtual address
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 * @param[out] buf: common buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_get_com_buf_with_vaddr(uint64_t virt_addr,  hb_mem_common_buf_t *buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get graphic buffer with virtual address
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 * @param[out] buf: graphic buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_get_graph_buf_with_vaddr(uint64_t virt_addr,  hb_mem_graphic_buf_t *buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief free buffer with virtual address
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_free_buf_with_vaddr(uint64_t virt_addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Import the shared memory of the common buffer and get the new common buffer information.
 *
 * @param[in] buf: input common buffer
 * @param[out] out_buf: output common buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
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
int32_t hb_mem_import_com_buf(hb_mem_common_buf_t * buf, hb_mem_common_buf_t * out_buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Import the shared memory of the graphic buffer and get the new graphic buffer information.
 *
 * @param[in] buf: input graphic buffer
 * @param[out] out_buf: output graphic buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
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
int32_t hb_mem_import_graph_buf(hb_mem_graphic_buf_t * buf, hb_mem_graphic_buf_t * out_buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the number of shared clients corresponding to the buffer through Fd
 *
 * @param[in] fd: file descriptor
				  range: [0, ); default: 0
 * @param[out] share_client_cnt: The number of shared clients corresponding to the buffer of fd
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
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
int32_t hb_mem_get_share_info(int32_t fd, int32_t * share_client_cnt);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the number of shared clients corresponding to the buffer through virtual address
 *
 * @param[in] virt_addr: Virtual address, which can be the offset virtual address
						 range: (0, )
 * @param[out] share_client_cnt: The number of shared clients corresponding to the buffer of fd
 *
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_get_share_info_with_vaddr(uint64_t virt_addr, int32_t * share_client_cnt);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief The number of shared clients waiting for the corresponding buffer of fd is less than or equal to the target value
 *
 * @param[in] fd: file descriptor
				  range: [0, ); default: 0
 * @param[in] share_client_cnt: The number of shared clients corresponding to the buffer of fd
								range: [0, ); default: 0
 * @param[in] timeout: Timeout (ms)
					   range: [0, ); default: 0
 *
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
 * @retval "HB_MEM_ERR_TIMEOUT": Wait timeout
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
int32_t hb_mem_wait_share_status(int32_t fd, int32_t share_client_cnt, int64_t timeout);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief The number of shared clients waiting for the corresponding buffer of virtual address is less than or equal to the target value
 *
 * @param[in] virt_addr: Virtual address, which can be the offset virtual address
						 range: [0, ); default: 0
 * @param[in] share_client_cnt: The number of shared clients corresponding to the buffer of fd
								range: [0, ); default: 0
 * @param[in] timeout: Timeout (ms)
					   range: [0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
 * @retval "HB_MEM_ERR_TIMEOUT": Wait timeout
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
int32_t hb_mem_wait_share_status_with_vaddr(uint64_t virt_addr, int32_t share_client_cnt, int64_t timeout);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Create a memory queue.
 *
 * @param[in] queue: hbmem buffer queue structure
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
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
int32_t hb_mem_create_buf_queue(hb_mem_buf_queue_t *queue);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief destory a memory queue.
 *
 * @param[in] queue: memory queue
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_QUEUE_NOT_FOUND": Unable to find the specified queue
 * @retval "HB_MEM_ERR_POOL_DESTROYED": Memory pool has been destroyed
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
int32_t hb_mem_destroy_buf_queue(hb_mem_buf_queue_t *queue);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Producers obtain available slot information
 *
 * @param[in] queue: memory queue
 * @param[out] slot: Memory queue element index
 * @param[out] buf: Element information
 * @param[in] timeout: Timeout (ms)
					   range: [0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_QUEUE_NOT_FOUND": Unable to find the specified queue
 * @retval "HB_MEM_ERR_QUEUE_DESTROYED": The queue has been destroyed
 * @retval "HB_MEM_ERR_QUEUE_NO_AVAILABLE_SLOT": No slots available
 * @retval "HB_MEM_ERR_TIMEOUT": Wait timeout
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
int32_t hb_mem_dequeue_buf(hb_mem_buf_queue_t * queue, int32_t *slot,
			void * buf, int64_t timeout);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief The producer fills in the element information and queues it into the slot
 *
 * @param[in] queue: memory queue
 * @param[in] slot: Memory queue element index
					range: [0, ); default: 0
 * @param[in] buf: Element information
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_QUEUE_NOT_FOUND": Unable to find the specified queue
 * @retval "HB_MEM_ERR_QUEUE_DESTROYED": The queue has been destroyed
 * @retval "HB_MEM_ERR_QUEUE_WRONG_SLOT":slot error
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
int32_t hb_mem_queue_buf(hb_mem_buf_queue_t * queue, int32_t slot, const void * buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief The consumer obtains the element information of the producer from the queue
 *
 * @param[in] queue: memory queue
 * @param[out] slot: Memory queue element index
 * @param[out] buf: Element information
 * @param[in] timeout: Timeout (ms)
					   range: [0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_QUEUE_NOT_FOUND": Unable to find the specified queue
 * @retval "HB_MEM_ERR_QUEUE_DESTROYED": The queue has been destroyed
 * @retval "HB_MEM_ERR_QUEUE_NO_AVAILABLE_SLOT": No slots available
 * @retval "HB_MEM_ERR_TIMEOUT": Wait timeout
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
int32_t hb_mem_request_buf(hb_mem_buf_queue_t * queue, int32_t *slot,
			void * buf, int64_t timeout);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Consumer releases the used element index.
 *
 * @param[in] queue: memory queue
 * @param[in] slot: Memory queue element index
					range: [0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_QUEUE_NOT_FOUND": Unable to find the specified queue
 * @retval "HB_MEM_ERR_QUEUE_DESTROYED": The queue has been destroyed
 * @retval "HB_MEM_ERR_QUEUE_WRONG_SLOT":slot error
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
int32_t hb_mem_release_buf(hb_mem_buf_queue_t * queue, int32_t slot);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief The producer cancels the slot obtained by exiting the queue or the consumer cancels the requested slot information.
 *
 * @param[in] queue: memory queue
 * @param[in] slot: Memory queue element index
					range: [0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_QUEUE_NOT_FOUND": Unable to find the specified queue
 * @retval "HB_MEM_ERR_QUEUE_DESTROYED": The queue has been destroyed
 * @retval "HB_MEM_ERR_QUEUE_WRONG_SLOT":slot error
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
int32_t hb_mem_cancel_buf(hb_mem_buf_queue_t * queue, int32_t slot);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Create a memory pool.
 *
 * @param[in] size: memory pool size
					range: (0, ); default: 0
 * @param[in] flags: Memory pool buffer flags @mem_usage_t
 * @param[out] pool: Memory pool
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
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
int32_t hb_mem_pool_create(uint64_t size, int64_t flags, hb_mem_pool_t * pool);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief destroy a memory pool.
 *
 * @param[in] fd: the fd of memory pool
				  range: [0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
 * @retval "HB_MEM_ERR_POOL_DESTROYED": Memory pool has been destroyed
 * @retval "HB_MEM_ERR_POOL_BUSY": The memory pool cannot be destroyed. There are unreleased memory chips in the memory pool
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
int32_t hb_mem_pool_destroy(int32_t fd);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief alloc a memory buffer from memory pool.
 *
 * @param[in] fd: the fd of memory pool
				  range: [0, ); default: 0
 * @param[in] size: the buffer size
					range: (0, ); default: 0
 * @param[out] buf: common buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_POOL_DESTROYED": Memory pool has been destroyed
 * @retval "HB_MEM_ERR_POOL_NOT_FOUND": Can not find the memory pool
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
int32_t hb_mem_pool_alloc_buf(int32_t fd, uint64_t size, hb_mem_common_buf_t * buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief free the memory buffer to memory pool.
 *
 * @param[in] virt_addr: Virtual address
						range: (0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
 * @retval "HB_MEM_ERR_POOL_DESTROYED": Memory pool has been destroyed
 * @retval "HB_MEM_ERR_POOL_NOT_FOUND": Can not find the memory pool
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
int32_t hb_mem_pool_free_buf(uint64_t virt_addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get real-time information of memory pool
 *
 * @param[in] fd: the fd of memory pool
				  range: [0, ); default: 0
 * @param[out] pool: memory pool
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_POOL_DESTROYED": Memory pool has been destroyed
 * @retval "HB_MEM_ERR_POOL_NOT_FOUND": Can not find the memory pool
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
int32_t hb_mem_pool_get_info(int32_t fd, hb_mem_pool_t * pool);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief create share memory pool.
 *
 * @param[in] num: the buffer number of the share memory pool.
				   range: (0, ); default: 0
 * @param[in] size: the buffer size.
				   range: (0, ); default: 0
 * @param[in] flags: the alloc flags @mem_usage_t
 * @param[out] pool: the share memory pool structure.@hb_mem_share_pool_t
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
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
int32_t hb_mem_share_pool_create(uint32_t num, uint64_t size, int64_t flags, hb_mem_share_pool_t * pool);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief destroy the share memory pool.
 *
 * @param[in] fd: one of the fd of share memory pool buffer
				  range: [0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
 * @retval "HB_MEM_ERR_POOL_DESTROYED": Memory pool has been destroyed
 * @retval "HB_MEM_ERR_POOL_BUSY": The memory pool cannot be destroyed. There are unreleased memory chips in the memory pool
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
int32_t hb_mem_share_pool_destroy(int32_t fd);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief alloc a memory buffer from share memory pool.
 *
 * @param[in] fd: the fd of memory pool
				  range: [0, ); default: 0
 * @param[out] buf: common buffer @hb_mem_common_buf_t
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_POOL_DESTROYED": Memory pool has been destroyed
 * @retval "HB_MEM_ERR_POOL_NOT_FOUND": Can not find the memory pool
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
int32_t hb_mem_share_pool_alloc_buf(int32_t fd, hb_mem_common_buf_t * buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief free the memory buffer from share memory pool by vaddr.
 *
 * @param[in] virt_addr: Virtual address
						 range: (0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
 * @retval "HB_MEM_ERR_POOL_NOT_FOUN": Can not find the share memory pool
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
int32_t hb_mem_share_pool_free_buf(uint64_t virt_addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief try get the share memory pool information.
 *
 * @param[in] fd: the file discriptor.
				  range: [0, ); default: 0
 * @param[out] pool: the share memory pool structure.@hb_mem_share_pool_t
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_POOL_DESTROYED": Memory pool has been destroyed
 * @retval "HB_MEM_ERR_POOL_NOT_FOUN": Can not find the memory pool
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
int32_t hb_mem_share_pool_get_info(int32_t fd, hb_mem_share_pool_t * pool);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get the buffer type with virtual address
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_mem_get_buf_type_with_vaddr(uint64_t virt_addr, hb_mem_buffer_type_t *type);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get the buffer type and buffer with virtual address, also can change buffer type. type should not be null
 * 	There are some case when use this interface
 * 	case1: com_buf = NULL, graph_buf = NULL, return error.
 * 	case2: com_buf = NULL, graph_buf != NULL, virt_addr belong to common buffer, return the graphic buffer change
 * 	       from common buffer.
 * 	case3: com_buf != NULL, graph_buf = NULL, virt_addr belong to common buffer, only return the common buffer.
 * 	case4: com_buf != NULL, graph_buf != NULL, virt_addr belong to common buffer, return the common buffer and
 * 	       graphic buffer change from common buffer.
 * 	case5: com_buf = NULL, graph_buf != NULL, virt_addr belong to graphic buffer, only return the graphic buffer.
 * 	case6: com_buf != NULL, graph_buf = NULL, virt_addr belong to graphic buffer, return the common buffer change
 * 	       from graphic buffer. BUT when source graphic buffer is uncontiguous return error.
 * 	case7: com_buf != NULL, graph_buf != NULL, virt_addr belong to graphic buffer, return the graphic buffer and
 * 	       common buffer change from graphic buffer, BUT when source graphic buffer is uncontiguous return error.
 *
 * @param[in] virt_addr: Virtual address, can be the offset virtual address
						 range: (0, ); default: 0
 * @param[out] type: the buffer type
 * @param[out] com_buf: the return common buffer structure
 * @param[out] graph_buf: the return graphic buffer structure
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_mem_get_buf_type_and_buf_with_vaddr(uint64_t virt_addr, hb_mem_buffer_type_t *type,
			hb_mem_common_buf_t *com_buf, hb_mem_graphic_buf_t *graph_buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get all the process pid of the buffer
 *
 * @param[in] virt_addr: virtal address, @hb_mem_common_buf_t
 * @param[out] pid: process pid which hold the buffer
 * @param[in] num: target num of the process which hold the buffer
 * @param[out] ret_num: total num of the process which hold the buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_get_buffer_process_info(uint64_t virt_addr, int32_t *pid, int32_t num, int32_t *ret_num);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get all the process pid of the buffer with share id
 *
 * @param[in] share_id: buffer share id  @hb_mem_common_buf_t
 * @param[out] pid: process pid which hold the buffer
 * @param[in] num: target num of the process which hold the buffer
 * @param[out] ret_num: total num of the process which hold the buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_get_buffer_process_info_with_share_id(int32_t share_id, int32_t *pid, int32_t num, int32_t *ret_num);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get the consume count of the buffer with fd
 *
 * @param[in] fd: the fd descriptor
 * @param[in] share_consume_cnt: the target consume count for the buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": The fd is abnormal.
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
int32_t hb_mem_get_consume_info(int32_t fd, int32_t * share_consume_cnt);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief Get the consume count of the buffer with vaddr
 *
 * @param[in] virt_addr: the virtual address
 * @param[out] share_consume_cnt: the consume count of the buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_get_consume_info_with_vaddr(uint64_t virt_addr, int32_t * share_consume_cnt);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief wait for the consume count of the buffer with fd
 *
 * @param[in] fd: the fd descriptor
 * @param[in] share_consume_cnt: the target consume count for the buffer
 * @param[in] timeout: the wait time
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": The fd is abnormal.
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
int32_t hb_mem_wait_consume_status(int32_t fd, int32_t share_consume_cnt, int64_t timeout);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief wait for the consume count of the buffer with vaddr
 *
 * @param[in] virt_addr: buffer virtual address
 * @param[in] share_consume_cnt: the target consume count for the buffer
 * @param[in] timeout: the wait time
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_wait_consume_status_with_vaddr(uint64_t virt_addr, int32_t share_consume_cnt, int64_t timeout);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief increase the consume count of the common buffer
 *
 * @param[in] buf: the common buffer structure
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
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
int32_t hb_mem_inc_com_buf_consume_cnt(hb_mem_common_buf_t * buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief increase the consume count of the graphic buffer
 *
 * @param[in] buf: the graphic buffer structure
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_inc_graph_buf_consume_cnt(hb_mem_graphic_buf_t * buf);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief decrease the consume count of the buffer with fd
 *
 * @param[in] fd: the buffer fd
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": The fd is abnormal. There is an abnormal fd
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
int32_t hb_mem_dec_consume_cnt(int32_t fd);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief decrease the consume count of the buffer with vaddr
 *
 * @param[in] virt_addr: the buffer virt_addr
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
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
int32_t hb_mem_dec_consume_cnt_with_vaddr(uint64_t virt_addr);

/**
 * @NO{S21E04C02U}
 * @ASIL{B}
 * @brief Import the shared memory of the common buffer and get the new common buffer information with paddr.
 * 	Attention：This interface is only used for memory which does not alloc from hbmem. When using this interface for
 * 	share memory accessing，please ensure the lifecycle of memory usage.
 *
 * @param[in] phys_addr: physical address
 * @param[in] size: buffer size
 * @param[in] flags: memory flags
 * @param[out] buf: output common buffer
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_mem_import_com_buf_with_paddr(uint64_t phys_addr,
			uint64_t size, int64_t flags, hb_mem_common_buf_t *buf);

/**
 * @NO{S21E04C02U}
 * @ASIL{B}
 * @brief Use the system's dma to complete the copy operation of data in two hbmem memory spaces
 *
 * @param[in] dst_vaddr: Start address of target hbmem space
						 range: (0, ); default: 0
 * @param[in] src_vaddr: Start address of source hbmem space
						 range: (0, ); default: 0
 * @param[in] size: Memory space size to be copied
					range: (0, ); default: 0
 *
 * @retval "=0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
 */
int32_t hb_mem_dma_copy(uint64_t dst_vaddr, uint64_t src_vaddr, uint64_t size);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief alloc graphic buffer group
 *
 * @param[in] w: the Image width array
 * @param[in] h: the Image height array
 * @param[in] format: Image format array @mem_pixel_format_t
 * @param[in] flags: buffer flag array @mem_usage_t
 * @param[in] stride: Image horizontal span array
 * @param[in] vstride: Image vertical span array
 * @param[out] buf_group: graphic buffer group
 * @param[in] bitmap: the bitmap for graphic buffer group
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_alloc_graph_buf_group(int32_t *w, int32_t *h, int32_t *format, int64_t *flags, int32_t *stride,
			int32_t *vstride, hb_mem_graphic_buf_group_t * buf_group, uint32_t bitmap);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief import graphic buffer group
 *
 * @param[in] in_group: the in graphic buffer group
 * @param[out] out_group: the import out graphic buffer group
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INSUFFICIENT_MEM": out of memory
 * @retval "HB_MEM_ERR_TOO_MANY_FD": The number of file handles exceeds the maximum
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_import_graph_buf_group(hb_mem_graphic_buf_group_t *in_group, hb_mem_graphic_buf_group_t *out_group);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get graphic buffer group information with fd
 *
 * @param[in] fd: the file description
 * @param[out] buf_group: the graphic buffer group
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_FD": File handle exception
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_get_graph_buf_group(int32_t fd, hb_mem_graphic_buf_group_t *buf_group);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get graphic buffer group information with vaddr
 *
 * @param[in] virt_addr: the virt_addr in graphic buffer group
 * @param[out] buf_group: the graphic buffer group
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 * @retval "HB_MEM_ERR_INVALID_VADDR": The virtual address is abnormal. There is an abnormal virtual address
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_get_graph_buf_group_with_vaddr(uint64_t virt_addr, hb_mem_graphic_buf_group_t *buf_group);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief increase graphic buffer group consume count
 *
 * @param[in] buf_group: the graphic buffer group
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_inc_graph_buf_group_consume_cnt(hb_mem_graphic_buf_group_t * buf_group);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief get buffer type and buffer structure with vaddr
 *
 * @param[in] virt_addr: the virtual address
 * @param[out] type: the buffer type
 * @param[out] com_buf: the common buffer structure
 * @param[out] graph_buf: the graphic buffer structure
 * @param[out] graph_group: the graphic buffer group structure
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_VADDR": invalid vaddr
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_get_buf_and_type_with_vaddr(uint64_t virt_addr, hb_mem_buffer_type_t *type, hb_mem_common_buf_t *com_buf,
					hb_mem_graphic_buf_t *graph_buf, hb_mem_graphic_buf_group_t *graph_group);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief increase the buffer user consume count with fd, not support graphic buffer group,
 * pool buffer and share pool buffer.
 *
 * @param[in] hb_fd: the file description
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_FD": invalid fd
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_inc_user_consume_cnt(int32_t hb_fd);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief decrease the buffer user consume count with fd, not support graphic buffer group,
 * pool buffer and share pool buffer.
 *
 * @param[in] hb_fd: the file description
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_FD": invalid fd
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_dec_user_consume_cnt(int32_t hb_fd);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief increase the buffer user consume count with fd, not support graphic buffer group,
 * pool buffer and share pool buffer.
 *
 * @param[in] virt_addr: the virtual address 
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_VADDR": invalid vaddr
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_inc_user_consume_cnt_with_vaddr(uint64_t virt_addr);

/**
 * @NO{S21E04C02I}
 * @ASIL{B}
 * @brief decrease the buffer user consume count with fd, not support graphic buffer group,
 * pool buffer and share pool buffer.
 *
 * @param[in] virt_addr: the virtual address 
 *
 * @retval "0": succeed
 * @retval "HB_MEM_ERR_MODULE_NOT_FOUND": The memory module is not open
 * @retval "HB_MEM_ERR_INVALID_VADDR": invalid vaddr
 * @retval "HB_MEM_ERR_INVALID_PARAMS": invalid parameter
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: J6
 * @compatibility SW: 0.1.1
 *
 * @callgraph
 * @callergraph
 * @design
*/
int32_t hb_mem_dec_user_consume_cnt_with_vaddr(uint64_t virt_addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* HB_MEM_MGR_H */
