#include "jpeg_codec.hpp"
#include "nv12_planes.hpp"
#include "protocol.hpp"
#include "hb_media_codec.h"
#include "hb_mem_mgr.h"
#include <cstdio>
#include <limits>
#include <cinttypes>

namespace orbvi::reference_images {
struct JpegCodec::State {
    media_codec_context_t context{};
    bool initialized = false, started = false;
    unsigned width = 0, height = 0, camera = 255;
};
JpegCodec::JpegCodec(unsigned camera) : state_(new State{}) { state_->camera = camera; }
JpegCodec::~JpegCodec() { close(); }
namespace {
void codec_error(unsigned camera, const char *stage, int rc) {
    std::fprintf(stderr, "camera%u: JPEG %s failed rc=%d (0x%08x)\n",
                 camera, stage, rc, static_cast<unsigned>(rc));
}
}
bool JpegCodec::open(unsigned width, unsigned height) {
    auto &s = *state_;
    std::uint64_t bytes = 0;
    if (s.initialized || !valid_nv12_size(width, height, bytes) ||
        bytes > std::numeric_limits<std::uint32_t>::max() - 4095u) {
        std::fprintf(stderr, "camera%u: JPEG open invalid dimensions/state %ux%u initialized=%d\n",
                     s.camera, width, height, s.initialized);
        return false;
    }
    auto &ctx = s.context;
    ctx.encoder = 1;
    ctx.codec_id = MEDIA_CODEC_ID_JPEG;
    auto &p = ctx.video_enc_params;
    p.width = width; p.height = height;
    p.pix_fmt = MC_PIXEL_FORMAT_NV12;
    p.frame_buf_count = 3; p.external_frame_buf = 0;
    p.bitstream_buf_count = 3;
    p.bitstream_buf_size = (static_cast<std::uint32_t>(bytes) + 4095u) & ~4095u;
    p.jpeg_enc_config.quality_factor = 50;
    p.jpeg_enc_config.restart_interval = width / 16;
    p.rot_degree = MC_CCW_0;
    p.mir_direction = MC_DIRECTION_NONE;
    p.enable_user_pts = 1;
    const int init_rc = hb_mm_mc_initialize(&ctx);
    if (init_rc) { codec_error(s.camera, "initialize", init_rc); return false; }
    s.initialized = true;
    const int configure_rc = hb_mm_mc_configure(&ctx);
    if (configure_rc) { codec_error(s.camera, "configure", configure_rc); return false; }
    mc_av_codec_startup_params_t startup{};
    const int start_rc = hb_mm_mc_start(&ctx, &startup);
    if (start_rc) { codec_error(s.camera, "start", start_rc); return false; }
    s.started = true; s.width = width; s.height = height;
    return true;
}
bool JpegCodec::encode(const nv12::Frame &frame, std::vector<std::uint8_t> &jpeg) {
    jpeg.clear();
    auto &s = *state_;
    if (!s.started || frame.width != s.width || frame.height != s.height || !valid_source(frame)) {
        std::fprintf(stderr, "camera%u: JPEG encode invalid source/state started=%d source=%ux%u configured=%ux%u\n",
                     s.camera, s.started, frame.width, frame.height, s.width, s.height);
        return false;
    }
    media_codec_buffer_t input{};
    const int dequeue_input_rc = hb_mm_mc_dequeue_input_buffer(&s.context, &input, 1000);
    if (dequeue_input_rc) { codec_error(s.camera, "dequeue input", dequeue_input_rc); return false; }
    auto &buf = input.vframe_buf;
    // BSP sample uses independent vir_ptr[0] and vir_ptr[1]. Never derive UV from Y.
    // The JPEG encoder feeder in this BSP explicitly sets vframe_buf.vstride
    // to JPG_FRAME_BUF::strideC (chroma row bytes). This differs from
    // nv12::Frame::vstride, which is the camera vertical row count.
    // Refuse unknown plane bounds instead of guessing from nominal dimensions.
    const std::size_t ys = buf.stride > 0 ? static_cast<std::size_t>(buf.stride) : 0;
    const std::size_t uvs = buf.vstride > 0 ? static_cast<std::size_t>(buf.vstride) : 0;
    const bool copied = ys && uvs && buf.compSize[0] && buf.compSize[1] &&
        buf.size >= static_cast<std::uint64_t>(frame.width) * frame.height * 3 / 2 &&
        copy_codec_nv12(frame, buf.vir_ptr[0], buf.compSize[0], ys,
                        buf.vir_ptr[1], buf.compSize[1], uvs);
    // No cancel-dequeue API: stop/release the codec on failure rather than
    // queueing an uninitialized/partially-copied input frame.
    if (!copied) {
        std::fprintf(stderr,
                     "camera%u: JPEG input bounds/copy failed source=%ux%u y=%zu uv=%zu stride=%u vstride=%u "
                     "codec_y=%p codec_uv=%p size=%u comp=[%u,%u] stride=%zu chroma_stride=%zu\n",
                     s.camera, frame.width, frame.height, frame.y_bytes, frame.uv_bytes,
                     frame.stride, frame.vstride, static_cast<void *>(buf.vir_ptr[0]),
                     static_cast<void *>(buf.vir_ptr[1]), buf.size, buf.compSize[0], buf.compSize[1], ys, uvs);
        close();
        return false;
    }
    // Codec buffers are cached DMA memory. Flush both independent planes after
    // the CPU copy and before handing ownership to the hardware queue.
    const int flush_y_rc = hb_mem_flush_buf_with_vaddr(
        reinterpret_cast<std::uint64_t>(buf.vir_ptr[0]), buf.compSize[0]);
    if (flush_y_rc) { codec_error(s.camera, "flush Y", flush_y_rc); close(); return false; }
    const int flush_uv_rc = hb_mem_flush_buf_with_vaddr(
        reinterpret_cast<std::uint64_t>(buf.vir_ptr[1]), buf.compSize[1]);
    if (flush_uv_rc) { codec_error(s.camera, "flush UV", flush_uv_rc); close(); return false; }
    {
        input.type = MC_VIDEO_FRAME_BUFFER;
        buf.width = frame.width; buf.height = frame.height;
        buf.pix_fmt = MC_PIXEL_FORMAT_NV12;
        buf.size = frame.width * frame.height * 3 / 2;
        buf.pts = frame.timestamp_ns;
    }
    const int queue_input_rc = hb_mm_mc_queue_input_buffer(&s.context, &input, 1000);
    if (queue_input_rc) {
        codec_error(s.camera, "queue input", queue_input_rc);
        close();
        return false;
    }
    media_codec_buffer_t output{};
    media_codec_output_buffer_info_t info{};
    const int dequeue_output_rc = hb_mm_mc_dequeue_output_buffer(&s.context, &output, &info, 1000);
    if (dequeue_output_rc) {
        codec_error(s.camera, "dequeue output", dequeue_output_rc);
        close(); return false;
    }
    const auto &stream = output.vstream_buf;
    std::uint32_t jpeg_width = 0, jpeg_height = 0;
    // BSP jpeg_component_enc_encoder.c sets output size to JPEG header size +
    // JPU bitstreamSize; its reader copies that to vstream_buf.size. The
    // jpeg_stream_info.frame_size is *input NV12* size, not JPEG byte count.
    // The JPU can report an aligned bitstream end including zero padding.
    // Only remove up to 15 trailing zero bytes, and only if a complete JPEG
    // with the expected SOF and EOI immediately precedes them.
    std::size_t used = stream.size;
    if (stream.vir_ptr && stream.size >= 4 &&
        stream.size <= s.context.video_enc_params.bitstream_buf_size) {
        while (used > 4 && stream.size - used < 15 && stream.vir_ptr[used - 1] == 0)
            --used;
    }
    bool ok = stream.vir_ptr && used >= 4 &&
              stream.size <= s.context.video_enc_params.bitstream_buf_size &&
              jpeg_probe_dimensions(stream.vir_ptr, used, jpeg_width, jpeg_height) &&
              jpeg_width == frame.width && jpeg_height == frame.height;
    if (ok) {
        if (used != stream.size)
            std::fprintf(stderr, "camera%u: JPEG zero padding trimmed=%zu reported=%u jpeg=%zu input_frame_size=%d\n",
                         s.camera, stream.size - used, stream.size, used, info.jpeg_stream_info.frame_size);
        jpeg.assign(stream.vir_ptr, stream.vir_ptr + used);
    }
    if (!ok) {
        const auto *p = stream.size <= s.context.video_enc_params.bitstream_buf_size
                            ? stream.vir_ptr : nullptr;
        std::fprintf(stderr,
                     "camera%u: JPEG output invalid size=%u candidate=%zu capacity=%u input_frame_size=%d "
                     "head=%02x%02x%02x%02x tail=%02x%02x candidate_tail=%02x%02x "
                     "SOF=%ux%u expected=%ux%u\n",
                     s.camera, stream.size, used, s.context.video_enc_params.bitstream_buf_size,
                     info.jpeg_stream_info.frame_size,
                     p && stream.size >= 4 ? p[0] : 0, p && stream.size >= 4 ? p[1] : 0,
                     p && stream.size >= 4 ? p[2] : 0, p && stream.size >= 4 ? p[3] : 0,
                     p && stream.size >= 2 ? p[stream.size - 2] : 0,
                     p && stream.size >= 2 ? p[stream.size - 1] : 0,
                     p && used >= 2 && used <= stream.size ? p[used - 2] : 0,
                     p && used >= 2 && used <= stream.size ? p[used - 1] : 0,
                     jpeg_width, jpeg_height, frame.width, frame.height);
    }
    const int queue_output_rc = hb_mm_mc_queue_output_buffer(&s.context, &output, 1000);
    if (queue_output_rc) { codec_error(s.camera, "queue output", queue_output_rc); ok = false; }
    if (!ok) { jpeg.clear(); close(); }
    return ok;
}
bool JpegCodec::close() {
    auto &s = *state_;
    bool ok = true;
    if (s.started) {
        const int rc = hb_mm_mc_stop(&s.context);
        if (rc) { codec_error(s.camera, "stop", rc); ok = false; }
    }
    s.started = false;
    if (s.initialized) {
        const int rc = hb_mm_mc_release(&s.context);
        if (rc) { codec_error(s.camera, "release", rc); ok = false; }
    }
    s.initialized = false;
    return ok;
}
}
