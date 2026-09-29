#include "encoder.hpp"
#include "hb_media_codec.h"
#include <cstring>
#include <vector>

namespace orbvi::camera {
struct Encoder::State {
    unsigned width, height;
    Options options;
    Output output;
    media_codec_context_t context{};
    bool initialized = false, started = false;
};

Encoder::Encoder(unsigned width, unsigned height, const Options &options, Output output)
    : state_(new State{width, height, options, std::move(output)}) {}
Encoder::~Encoder() { close(); }

int Encoder::open() {
    auto &state = *state_;
    if (state.initialized) return -1;
    auto &context = state.context;
    context.encoder = 1;
    context.codec_id = state.options.codec == Codec::H264 ? MEDIA_CODEC_ID_H264 : MEDIA_CODEC_ID_H265;
    auto &parameters = context.video_enc_params;
    parameters.width = state.width; parameters.height = state.height;
    parameters.pix_fmt = MC_PIXEL_FORMAT_NV12;
    parameters.frame_buf_count = 5; parameters.external_frame_buf = 0;
    parameters.bitstream_buf_count = 8;
    parameters.bitstream_buf_size = (state.width * state.height * 3 / 2 + 1023) & ~1023u;
    parameters.gop_params.gop_preset_idx = 1;
    parameters.gop_params.decoding_refresh_type = 2;
    parameters.rot_degree = static_cast<mc_rotate_degree_t>(state.options.rotate / 90);
    parameters.mir_direction = MC_DIRECTION_NONE;
    parameters.enable_user_pts = 1;
    parameters.rc_params.mode = state.options.codec == Codec::H264 ? MC_AV_RC_MODE_H264CBR : MC_AV_RC_MODE_H265CBR;
    if (hb_mm_mc_get_rate_control_config(&context, &parameters.rc_params)) return -1;
    if (state.options.codec == Codec::H264) {
        auto &rate = parameters.rc_params.h264_cbr_params;
        rate.intra_period = state.options.fps; rate.intra_qp = 30;
        rate.bit_rate = state.options.bitrate_kbps; rate.frame_rate = state.options.fps;
        rate.initial_rc_qp = 20; rate.vbv_buffer_size = 3000;
        rate.mb_level_rc_enalbe = 1; rate.min_qp_I = rate.min_qp_P = rate.min_qp_B = 8;
        rate.max_qp_I = rate.max_qp_P = rate.max_qp_B = 50;
    } else {
        auto &rate = parameters.rc_params.h265_cbr_params;
        rate.intra_period = state.options.fps; rate.intra_qp = 30;
        rate.bit_rate = state.options.bitrate_kbps; rate.frame_rate = state.options.fps;
        rate.initial_rc_qp = 30; rate.vbv_buffer_size = 3000;
        rate.ctu_level_rc_enalbe = 1; rate.min_qp_I = rate.min_qp_P = rate.min_qp_B = 8;
        rate.max_qp_I = rate.max_qp_P = rate.max_qp_B = 50;
    }
    if (hb_mm_mc_initialize(&context)) return -1;
    state.initialized = true;
    if (hb_mm_mc_configure(&context)) return -1;
    mc_av_codec_startup_params_t startup{};
    if (hb_mm_mc_start(&context, &startup)) return -1;
    state.started = true;
    return 0;
}

int Encoder::encode(const Nv12View &frame, uint64_t timestamp_ns) {
    auto &state = *state_;
    if (!state.started || frame.width != state.width || frame.height != state.height) return -1;
    media_codec_buffer_t input{};
    if (hb_mm_mc_dequeue_input_buffer(&state.context, &input, state.options.frame_timeout_ms)) return -1;
    if (!copy_nv12(frame, input.vframe_buf.vir_ptr[0], input.vframe_buf.size)) return -1;
    input.vframe_buf.pts = timestamp_ns;
    if (hb_mm_mc_queue_input_buffer(&state.context, &input, state.options.frame_timeout_ms)) return -1;
    media_codec_buffer_t output{};
    media_codec_output_buffer_info_t info{};
    if (hb_mm_mc_dequeue_output_buffer(&state.context, &output, &info, state.options.frame_timeout_ms)) return -1;
    int result = state.output(output.vstream_buf.vir_ptr, output.vstream_buf.size, output.vstream_buf.pts);
    if (hb_mm_mc_queue_output_buffer(&state.context, &output, state.options.frame_timeout_ms)) result = -1;
    return result;
}

int Encoder::close() {
    auto &state = *state_;
    int result = 0;
    if (state.started && hb_mm_mc_stop(&state.context)) result = -1;
    state.started = false;
    if (state.initialized && hb_mm_mc_release(&state.context)) result = -1;
    state.initialized = false;
    return result;
}
}
