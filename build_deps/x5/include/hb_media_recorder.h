/***
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2019 -2023, Horizon Robotics Co., Ltd.
 *                   All rights reserved.
 ***/
#ifndef HB_MEDIA_RECORDER_H
#define HB_MEDIA_RECORDER_H

#include "hb_media_codec.h"
#include "hb_media_muxer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Define the media recorder state.
 **/
typedef enum _media_recorder_state {
    MEDIA_RECORDER_STATE_NONE = -1,
    MEDIA_RECORDER_STATE_UNINITIALIZED,
    MEDIA_RECORDER_STATE_INITIALIZED,
    MEDIA_RECORDER_STATE_CONFIGURED,
    MEDIA_RECORDER_STATE_PREPARED,
    MEDIA_RECORDER_STATE_STARTED,
    MEDIA_RECORDER_STATE_ERROR,
    MEDIA_RECORDER_STATE_TOTAL
} media_recorder_state_t;

/**
 * Define the audio source.
 **/
typedef enum _mr_audio_source {
    /* The default audio source is MIC */
    MR_AUDIO_SOURCE_DEFAULT = 0,
    MR_AUDIO_SOURCE_MIC,
    MR_AUDIO_SOURCE_TOTAL
} mr_audio_source_t;

/**
 * Define the video source.
 **/
typedef enum _mr_video_source {
    /* The default video source is camera */
    MR_VIDEO_SOURCE_DEFAULT = 0,
    MR_VIDEO_SOURCE_CAMERA,
    MR_VIDEO_SOURCE_TOTAL
} mr_video_source_t;

/**
 * Define the event type of media recorder.
 **/
typedef enum _mr_event_type {
    MR_EVENT_ERROR = 1,
    MR_EVENT_INFO  = 2,
    MR_EVENT_TOTAL,
} mr_event_type_t;

/**
 * Define the error type of media recorder error event.
 **/
typedef enum _mr_error {
    MR_ERROR_UNKNOWN                   = -1,

    MR_ERROR_GENERAL                   = 100,
    MR_ERROR_AUDIO_CODEC               = 101,
    MR_ERROR_VIDEO_CODEC               = 102,
    MR_ERROR_MEDIA_MUXER               = 103,
} mr_error_t;

/**
 * Define the information type of media recorder information event.
 **/
typedef enum _mr_info {
    MR_INFO_UNKNOWN                   = -1,

    MR_INFO_MAX_DURATION_REACHED      = 200,
    MR_INFO_MAX_FILESIZE_REACHED      = 201,
} mr_info_t;

/**
 * Define the listener function .
 **/
typedef void (*media_recorder_listener)(hb_s32 event_type,
                 hb_s32 event, hb_s32 message, void *user);

/**
 * Define the parameters of video encoder.
 **/
typedef struct _mr_video_encoder_params {
    /**
     * Specify the video source.
     * Values[@see mr_video_source_t]
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: MR_VIDEO_SOURCE_DEFAULT
     */
    mr_video_source_t mr_video_source;

    /**
     * Codec ID. Only support H264 and H265.
     * Values[MEDIA_CODEC_ID_H264,MEDIA_CODEC_ID_H265]
     *
     * - Note:
     * - Default: MEDIA_CODEC_ID_NONE
     */
    media_codec_id_t codec_id;

    /**
     * Rotate counterclockwise incoming pictures before starting
     * the ecode process.
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: MC_CCW_0;
     */
    mc_rotate_degree_t rot_degree;

    /**
     * Mirror incoming pictures before starting the ecode process.
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: DIRECTION_NONE;
     */
    mc_mirror_direction_t mir_direction;

    /**
     * The target frame rate of the encoded data in fps.
     * Values[0,240]fps
     *
     * - Note: It's unchangable parameter in the same sequence.
     * - Default: 30
     */
    hb_u32 frame_rate;

    /**
     * The target average bitrate of the encoded data in kbps.
     * Values[0,700000]kps
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: 192
     */
    hb_u32 bit_rate;

    /**
     * A period of intra picture in GOP size.
     * Values[0,2047]
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: 30
     */ 
    hb_s32 intra_period;
} mr_video_encoder_params_t;

/**
 * Define the ex-parameters of video encoder.
 **/
typedef struct _mr_video_encoder_params_ex {
    /**
     * Specify the video source.
     * Values[@see mr_video_source_t]
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: MR_VIDEO_SOURCE_DEFAULT
     */
    mr_video_source_t mr_video_source;

    /**
     * Codec ID. Only support H264 and H265.
     * Values[MEDIA_CODEC_ID_H264,MEDIA_CODEC_ID_H265]
     *
     * - Note:
     * - Default: MEDIA_CODEC_ID_NONE
     */
    media_codec_id_t codec_id;

    /**
     * Codec parameters. Only support H264 and H265.
     *
     * - Note:
     * - Default: MEDIA_CODEC_ID_NONE
     */
    mc_video_codec_enc_params_t video_enc_params;
} mr_video_encoder_params_ex_t;

/**
 * Define the parameters of audio encoder.
 **/
typedef struct _mr_audio_encoder_params {
    /**
     * Specify the audio source.
     * Values[@see mr_audio_source_t]
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: MR_AUDIO_SOURCE_DEFAULT
     */
    mr_audio_source_t mr_audio_source;

    /**
     * Codec ID. Only support H264 and H265.
     * Values[MEDIA_CODEC_ID_AAC]
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: MEDIA_CODEC_ID_NONE
     */
    media_codec_id_t codec_id;

    /**
     * The average bitrate
     * Values[>0]
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: 14
     */
    hb_s32 bit_rate;

    /**
     * Audio sample format.
     * Values[@see mc_audio_sample_format_t]
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: MC_AV_SAMPLE_FMT_NONE
     */
    mc_audio_sample_format_t sample_fmt;

    /**
     * Audio sample rate.
     * Values[@see mc_audio_sample_rate_t]
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: MC_AV_SAMPLE_RATE_NONE
     */
    mc_audio_sample_rate_t sample_rate;

    /**
     * Audio channel layout.
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: 0
     */
    mc_audio_channel_layout_t channel_layout;

    /**
     * Number of audio channels. It's related with channel layout.
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: 0
     */
    hb_s32 channels;
} mr_audio_encoder_params_t;

/**
 * Define the parameters of media recorder context.
 **/
typedef struct _media_recorder_context {
    /**
     * Specify the output data path. For example, data path should be
     * "/data/outputFile.mp4"
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: NULL
     */
    hb_string output_file_name;

    /**
     * Specify the format of the output file.
     * The valid values @see mx_output_format_t.
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: MEDIA_MUXER_OUTPUT_FORMAT_DEFAULT
     */
    mx_output_format_t output_format;

    /**
     * Specify the maximum duration in ms of the output file.
     * Values[>=0], <=0 means no limit
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: 0
     */
    hb_s32 max_file_duration;

    /**
     * Specify the maximum size in bytes of the output file.
     * Values[>=0], <=0 means no limit
     *
     * - Note: It's unchangable parameters in the same sequence.
     * - Default: 0
     */
    hb_s32 max_file_size;

    /**
     * Private data. Users must not modify this value!!!
     * Values[0,31]
     * - Note: 
     * - Default: -1
     */
    hb_s32 instance_index;
} media_recorder_context_t;

/**
 * @NO{S07E05C01II}
 * @ASIL{QM}
 * @brief Get the default media recorder context.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
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
extern hb_s32 hb_mm_mr_get_default_context(media_recorder_context_t
				*context);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Initialize the media recorder. If success, MediaRecorder will
 * enter into MEDIA_RECORDER_STATE_INITIALIZED state. And it's invalid to 
 * reinitialize the media recorder if the MediaMuxer's state isn't
 * MEDIA_RECORDER_STATE_UNINITIALIZED.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INSUFFICIENT_RES: Insufficient resources
 * @retval HB_MEDIA_ERR_NO_FREE_INSTANCE: No available instance
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
extern hb_s32 hb_mm_mr_initialize(media_recorder_context_t *context);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Set listener to media recorder. The function should be called before
 * MediaRecorder is started.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 * @param[in] listener: listener function
 * @param[in] userdata: pointer of userdata
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_set_listener(media_recorder_context_t
				*context, media_recorder_listener listener, void *userdata);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Get the video source information.
 *
 * @param[in] context: recorder context
 * @param[out] params: video encoder parameters @see mr_video_encoder_params_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN Unknow: error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_get_mr_video_source(media_recorder_context_t
				*context, mr_video_encoder_params_t *params);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Set the video source information.
 *
 * @param[in] context: recorder context
 * @param[in] params: video encoder parameters @see mr_video_encoder_params_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN Unknow: error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_set_mr_video_source(media_recorder_context_t
				*context, const mr_video_encoder_params_t *params);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Get the extra video source information.
 *
 * @param[in] context: recorder context
 * @param[out] params: video encoder extra parameters @see mr_video_encoder_params_ex_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE invalid instance
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
extern hb_s32 hb_mm_mr_get_mr_video_source_ex(media_recorder_context_t
				*context, mr_video_encoder_params_ex_t *params);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Set the video extra source information.
 *
 * @param[in] context: recorder context
 * @param[in] params: video encoder extra parameters @see mr_video_encoder_params_ex_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_set_mr_video_source_ex(media_recorder_context_t
				*context, const mr_video_encoder_params_ex_t *params);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Get the audio source information.
 *
 * @param[in] context: recorder context
 * @param[out] params: audio encoder parameters @see mr_audio_encoder_params_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_get_mr_audio_source(media_recorder_context_t
				*context, mr_audio_encoder_params_t *params);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Set the audio source information.
 *
 * @param[in] context: recorder context
 * @param[in] params: audio encoder parameters @see mr_audio_encoder_params_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_set_mr_audio_source(media_recorder_context_t
				*context, const mr_audio_encoder_params_t *params);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief It specify the channel information of camera.
 *
 * @param[in] context: recorder context
 * @param[in] pipeline: pipeline
 * @param[in] channel_port_id: IPU channel port id
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_set_camera(media_recorder_context_t *context,
				hb_s32 pipeline, hb_s32 channel_port_id);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief It specify the channel information of VIO pyramid.
 *
 * @param[in] context: recorder context
 * @param[in] pipeline: pipeline
 * @param[in] channel_port_id: Pyramid channel port id
 * @param[in] layer_idx: layer idx
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_set_camera_pym(media_recorder_context_t *context,
				hb_s32 pipeline, hb_s32 channel_port_id, hb_s32 layer_idx);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Configure the media recorder. If success, MediaRecorder
 * will enter into MEDIA_RECORDER_STATE_CONFIGURED state.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INSUFFICIENT_RES: Insufficient resources
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_configure(media_recorder_context_t *context);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Prepare the codecs and muxer. If success, MediaRecorder
 * will enter into MEDIA_RECORDER_STATE_PREPARED state.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INSUFFICIENT_RES: Insufficient resources
 * @retval HB_MEDIA_ERR_NO_FREE_INSTANCE: No available instance
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
extern hb_s32 hb_mm_mr_prepare(media_recorder_context_t *context);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Start the media recorder. If success, MediaRecorder
 * will enter into MEDIA_RECORDER_STATE_STARTED state.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INSUFFICIENT_RES: Insufficient resources
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_start(media_recorder_context_t *context);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Stop the media recorder. If success, MediaRecorder
 * will enter into MEDIA_RECORDER_STATE_INITIALIZE state.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_stop(media_recorder_context_t *context);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Release the media recorder. If success, MediaRecorder
 * will enter into MEDIA_RECORDER_STATE_UNINITIALIZE state.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_OPERATION_NOT_ALLOWED: Disallowed operation
 * @retval HB_MEDIA_ERR_INVALID_INSTANCE: invalid instance
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
extern hb_s32 hb_mm_mr_release(media_recorder_context_t *context);

/**
 * @NO{S07E05C01I}
 * @ASIL{QM}
 * @brief Get the state of media recorder.
 *
 * @param[in] context: recorder context, see media_recorder_context_t
 * @param[out] state: recorder state @see media_recorder_state_t
 *
 * @retval =0: Success
 * @retval HB_MEDIA_ERR_UNKNOWN: Unknow error
 * @retval HB_MEDIA_ERR_INVALID_PARAMS: Invalid parameter
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
extern hb_s32 hb_mm_mr_get_state(media_recorder_context_t *context,
				media_recorder_state_t *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* HB_MEDIA_RECORDER_H */
