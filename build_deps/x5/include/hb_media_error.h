/***
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2019 -2023, Horizon Robotics Co., Ltd.
 *                   All rights reserved.
 ***/
#ifndef HB_MEDIA_ERROR_H
#define HB_MEDIA_ERROR_H

#include <errno.h>
#include <stddef.h>
#include "hb_media_basic_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* error handling */
#if EDOM > 0
/**
 * Returns a negative error code from a POSIX error code,
 * to return from library functions.
 */
#define HB_MEDIA_ERR(e) (-(e))

/**
 * Returns a POSIX error code from a library function error return value.
 */
#define HB_MEDIA_UNERR(e) (-(e))
#else
/**
 * Some platforms have E* and errno already negated.
 */
#define HB_MEDIA_ERR(e) (e)
#define HB_MEDIA_UNERR(e) (e)
#endif

/* Unknown Error */
#define HB_MEDIA_ERR_UNKNOWN                   (-268435455) //0xF0000001
/* CODEC not found */
#define HB_MEDIA_ERR_CODEC_NOT_FOUND           (-268435454) //0xF0000002
/* Failed to open codec device */
#define HB_MEDIA_ERR_CODEC_OPEN_FAIL           (-268435453) //0xF0000003
/* Timeout to operate codec device */
#define HB_MEDIA_ERR_CODEC_RESPONSE_TIMEOUT    (-268435452) //0xF0000004
/* Failed to initialize codec device */
#define HB_MEDIA_ERR_CODEC_INIT_FAIL           (-268435451) //0xF0000005
/* Operation is not allowed */
#define HB_MEDIA_ERR_OPERATION_NOT_ALLOWED     (-268435450) //0xF0000006
/* Insufficient resource */
#define HB_MEDIA_ERR_INSUFFICIENT_RES          (-268435449) //0xF0000007
/* No free instance left */
#define HB_MEDIA_ERR_NO_FREE_INSTANCE          (-268435448) //0xF0000008
/* Invalid parameters */
#define HB_MEDIA_ERR_INVALID_PARAMS            (-268435447) //0xF0000009
/* Invalid instance */
#define HB_MEDIA_ERR_INVALID_INSTANCE          (-268435446) //0xF000000A
/* Invalid buffer */
#define HB_MEDIA_ERR_INVALID_BUFFER            (-268435445) //0xF000000B
/* Invalid command */
#define HB_MEDIA_ERR_INVALID_COMMAND           (-268435444) //0xF000000C
/* Wait timeout */
#define HB_MEDIA_ERR_WAIT_TIMEOUT              (-268435443) //0xF000000D
/* file cannot be operated successfully */
#define HB_MEDIA_ERR_FILE_OPERATION_FAILURE    (-268435442) //0xF000000E
/* fail to set parameters */
#define HB_MEDIA_ERR_PARAMS_SET_FAILURE        (-268435441) //0xF000000F
/* fail to get parameters */
#define HB_MEDIA_ERR_PARAMS_GET_FAILURE        (-268435440) //0xF0000010
/* audio encoding/decoding failed */
#define HB_MEDIA_ERR_CODING_FAILED             (-268435439) //0xF0000011
/* audio output buffer full*/
#define HB_MEDIA_ERR_OUTPUT_BUF_FULL           (-268435438) //0xF0000012
/* Unsupported feature*/
#define HB_MEDIA_ERR_UNSUPPORTED_FEATURE       (-268435437) //0xF0000013
/* Invalid priority */
#define HB_MEDIA_ERR_INVALID_PRIORITY          (-268435436) //0xF0000014

#define HB_ERR_MAX_STRING_SIZE 64

/**
 * @NO{S07E01C01I}
 * @ASIL{QM}
 * @brief Query a description of the HB_MEDIA_ERR code.
 *
 * @param[in] err_num: error code to describe
 * @param[out] err_buf: buffer to which description is written
 * @param[in] errbuf_size: the size in bytes of errbuf
 *
 * @retval =0: Succeed
 * @retval <0: Failed
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: v1.2.3
 *
 * @callgraph
 * @callergraph
 * @design
 */
extern hb_s32 hb_mm_strerror(hb_s32 err_num, hb_string err_buf,
				size_t errbuf_size);

 /**
 * @NO{S07E01C01I}
 * @ASIL{QM}
 * @brief Fill the provided buffer with a string containing an error string
 * corresponding to the HB_MEDIA_ERR code errnum.
 *
 * @param[in] err_num: error code
 * @param[out] err_buf: buffer to which description is written
 * @param[in] errbuf_size: the size in bytes of errbuf
 *
 * @retval !=NULL: get error buffer Succeed
 * @retval =NULL: get error buffer failed
 *
 * @data_read None
 * @data_updated None
 * @compatibility HW: XJ3/J5/J6
 * @compatibility SW: v1.2.3
 *
 * @callgraph
 * @callergraph
 * @design
 */
static inline hb_string hb_mm_make_error_string(hb_s32 err_num,
						hb_string err_buf,
						size_t errbuf_size) {
	hb_mm_strerror(err_num, err_buf, errbuf_size);
	return err_buf;
}
/**
 * Convenience macro, the return value should be used only directly in
 * function arguments but never stand-alone.
 */
#define hb_mm_err2str(errnum) \
	hb_mm_make_error_string(errnum, (char[HB_ERR_MAX_STRING_SIZE]){0},\
	HB_ERR_MAX_STRING_SIZE)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* HB_MEDIA_ERROR_H */
