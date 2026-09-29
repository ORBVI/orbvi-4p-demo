/*************************************************************************
 *                     COPYRIGHT NOTICE
 *            Copyright 2021 Horizon Robotics, Inc.
 *                   All rights reserved.
 *************************************************************************/
/**
 * @file hb_mem_err.h
 *
 * @NO{S21E04C02I}
 * @ASIL{B}
 */
#ifndef HB_MEM_ERR_H
#define HB_MEM_ERR_H

#include <errno.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* error handling */
#if EDOM > 0
/**
 * Returns a negative error code from a POSIX error code,
 * to return from library functions.
 */
#define HB_MEM_ERR(e) (-(e))

/**
 * Returns a POSIX error code from a library function error return value.
 */
#define HB_MEM_UNERR(e) (-(e))
#else
/**
 * Some platforms have E* and errno already negated.
 */
#define HB_MEM_ERR(e) (e)
#define HB_MEM_UNERR(e) (e)
#endif

#define HB_MEM_ERR_MAX_STR_SIZE 256

/* Unknown Error */
#define HB_MEM_ERR_UNKNOWN				(-16777215) /**< unknown error number, 0xFF000001.*/
#define HB_MEM_ERR_INVALID_PARAMS			(-16777214) /**< invalidate parameter, 0xFF000002.*/
#define HB_MEM_ERR_INVALID_FD				(-16777213) /**< invalidate fd, 0xFF000003.*/
#define HB_MEM_ERR_INVALID_VADDR			(-16777212) /**< invalidate vaddr, 0xFF000004.*/
#define HB_MEM_ERR_INSUFFICIENT_MEM			(-16777211) /**< insufficent memory, 0xFF000005.*/
#define HB_MEM_ERR_TOO_MANY_FD				(-16777210) /**< Too many opened fd, 0xFF000006.*/
#define HB_MEM_ERR_TIMEOUT				(-16777209) /**< Timeout to wait operation finishing, 0xFF000007.*/
#define HB_MEM_ERR_MODULE_NOT_FOUND			(-16777208) /**< Memory module is not opened, 0xFF000008.*/
#define HB_MEM_ERR_MODULE_OPEN_FAIL			(-16777207) /**< Fail to open memory module, 0xFF000009.*/
#define HB_MEM_ERR_QUEUE_NOT_FOUND			(-16777206) /**< Buffer queue is not created 0xFF00000A.*/
#define HB_MEM_ERR_QUEUE_DESTROYED			(-16777205) /**< Buffer queue is destroyed 0xFF00000B.*/
#define HB_MEM_ERR_QUEUE_WRONG_SLOT			(-16777204) /**< Buffer queue slot is wrong 0xFF00000C.*/
#define HB_MEM_ERR_QUEUE_NO_AVAILABLE_SLOT		(-16777203) /**< No availabel buffer queue slot 0xFF00000D.*/
#define HB_MEM_ERR_QUEUE_ALREADY_EXIST			(-16777202) /**< Buffer queue has been created 0xFF00000E.*/
#define HB_MEM_ERR_POOL_NOT_FOUND			(-16777201) /**< Memory pool is not created 0xFF00000F.*/
#define HB_MEM_ERR_POOL_DESTROYED			(-16777200) /**< Memory pool is destroyed 0xFF000010.*/
#define HB_MEM_ERR_POOL_BUSY				(-16777199) /**< Memory pool is busy 0xFF000011.*/
#define HB_MEM_ERR_WAIT_SHARE_FAILURE			(-16777198) /**< Fail to wait share client count 0xFF000012.*/
#define HB_MEM_ERR_NOT_ALLOW				(-16777197) /**< operation is not allow 0xFF000013.*/
#define HB_MEM_ERR_CHECK_VER_FAIL			(-16777196) /**< Fail to check version.*/
#define HB_MEM_ERR_REGISTER_FAIL			(-16777195) /**< Fail to register graphic buffer group.*/
#define HB_MEM_ERR_INVALID_GROUPID			(-16777194) /**< invalidate groupid*/

/**
 * Query a description of the HB_MEM_ERR code.
 *
 * @param[in]  err_num      error code
 * @param[out] err_buf      buffer to which description is written
 * @param[in]  errbuf_size  the size in bytes of errbuf
 * @return 0 on success, a negative value if a description for errnum
 * cannot be found
 */
extern int32_t hb_mem_strerror(int32_t err_num, char * err_buf,
				size_t errbuf_size);

/**
 * Fill the provided buffer with a string containing an error string
 * corresponding to the HB_MEM_ERR code errnum.
 *
 * @param[in]  err_num         error code to describe
 * @param[out] err_buf         a buffer
 * @param[in]  errbuf_size     size in bytes of errbuf
 * @return the buffer in input, filled with the error description
 * @see hb_mem_strerror()
 */
static inline char * hb_mem_make_error_string(int32_t err_num,
						char * err_buf,
						size_t errbuf_size) {
	(void)hb_mem_strerror(err_num, err_buf, errbuf_size);
	return err_buf;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* HB_MEM_ERR_H */
