#include "jpeg_codec.hpp"
#include "hb_media_codec.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <vector>

namespace {
std::array<unsigned char, 32> ybuf{};
std::array<unsigned char, 32> uvbuf{};
unsigned queues = 0, stops = 0, releases = 0, flushes = 0, outputs = 0;
bool flush_fail = false, output_fail = false, wrong_size = false, queue_fail = false;
bool short_uv = false;
unsigned zero_padding = 0;
bool damage_eoi = false, wrong_dimensions = false, nonzero_padding = false, oversize_output = false;

}
extern "C" {
int32_t hb_mem_flush_buf_with_vaddr(uint64_t address, uint64_t size) {
    assert((address == reinterpret_cast<uint64_t>(ybuf.data()) && size == ybuf.size()) ||
           (address == reinterpret_cast<uint64_t>(uvbuf.data()) && size == uvbuf.size()));
    ++flushes; return flush_fail ? -1 : 0;
}
hb_s32 hb_mm_mc_initialize(media_codec_context_t *ctx) {
    assert(ctx->codec_id == MEDIA_CODEC_ID_JPEG);
    assert(ctx->video_enc_params.pix_fmt == MC_PIXEL_FORMAT_NV12);
    return 0;
}
hb_s32 hb_mm_mc_configure(media_codec_context_t *) { return 0; }
hb_s32 hb_mm_mc_start(media_codec_context_t *, const mc_av_codec_startup_params_t *) { return 0; }
hb_s32 hb_mm_mc_stop(media_codec_context_t *) { ++stops; return 0; }
hb_s32 hb_mm_mc_release(media_codec_context_t *) { ++releases; return 0; }
hb_s32 hb_mm_mc_dequeue_input_buffer(media_codec_context_t *, media_codec_buffer_t *buf, hb_s32) {
    buf->vframe_buf.vir_ptr[0] = ybuf.data();
    buf->vframe_buf.vir_ptr[1] = uvbuf.data();
    buf->vframe_buf.size = wrong_size ? 4 : 48;
    buf->vframe_buf.stride = 8;
    buf->vframe_buf.vstride = 16; // BSP JPEG feeder reports explicit strideC
    buf->vframe_buf.compSize[0] = ybuf.size();
    buf->vframe_buf.compSize[1] = short_uv ? 8 : uvbuf.size();
    return 0;
}
hb_s32 hb_mm_mc_queue_input_buffer(media_codec_context_t *, media_codec_buffer_t *, hb_s32) {
    ++queues; return queue_fail ? -1 : 0;
}
hb_s32 hb_mm_mc_dequeue_output_buffer(media_codec_context_t *, media_codec_buffer_t *buf,
                                       media_codec_output_buffer_info_t *info, hb_s32) {
    ++outputs;
    if (output_fail) return -1;
    static unsigned char jpeg[] = {
        0xff, 0xd8, 0xff, 0xc0, 0x00, 0x08, 0x08, 0x00, 0x04, 0x00, 0x04, 0x01,
        0xff, 0xda, 0x00, 0x02, 0x13, 0xff, 0x00, 0x8a, 0xff, 0xd9
    };
    static std::array<unsigned char, 64> encoded{};
    std::copy(std::begin(jpeg), std::end(jpeg), encoded.begin());
    if (damage_eoi) encoded[sizeof(jpeg) - 1] = 0x00;
    if (wrong_dimensions) encoded[9] = 0x05;
    assert(sizeof(jpeg) + zero_padding <= encoded.size());
    std::fill(encoded.begin() + sizeof(jpeg), encoded.end(), 0);
    if (nonzero_padding && zero_padding) encoded[sizeof(jpeg)] = 0x7e;
    buf->vstream_buf.vir_ptr = encoded.data();
    buf->vstream_buf.size = oversize_output ? 8192 : sizeof(jpeg) + zero_padding;
    // BSP jpeg_stream_info.frame_size is INPUT frame size, deliberately not
    // equal to either the aligned output size or the JPEG EOI position.
    info->jpeg_stream_info.frame_size = 48;
    return 0;
}
hb_s32 hb_mm_mc_queue_output_buffer(media_codec_context_t *, media_codec_buffer_t *, hb_s32) {
    return 0;
}
}
int main() {
    std::array<unsigned char, 24> source_y{};
    std::array<unsigned char, 12> source_uv{};
    for (unsigned row = 0; row < 4; ++row)
        for (unsigned col = 0; col < 4; ++col) source_y[row * 6 + col] = row * 4 + col;
    for (unsigned row = 0; row < 2; ++row)
        for (unsigned col = 0; col < 4; ++col) source_uv[row * 6 + col] = 16 + row * 4 + col;
    orbvi::nv12::Frame frame;
    frame.y = source_y.data(); frame.uv = source_uv.data();
    frame.y_bytes = source_y.size(); frame.uv_bytes = source_uv.size();
    frame.width = 4; frame.height = 4; frame.stride = 6; frame.vstride = 4;
    frame.timestamp_ns = 345;
    orbvi::reference_images::JpegCodec codec;
    assert(codec.open(4, 4));
    std::vector<std::uint8_t> jpeg;
    assert(codec.encode(frame, jpeg));
    assert(queues == 1 && flushes == 2 && jpeg.size() == 22 && outputs == 1);
    for (unsigned row = 0; row < 2; ++row)
        for (unsigned col = 0; col < 4; ++col)
            assert(uvbuf[row * 16 + col] == 16 + row * 4 + col);
    short_uv = true;
    assert(!codec.encode(frame, jpeg));
    // A failed plane validation must never queue an incomplete JPEG input.
    assert(queues == 1 && stops == 1 && releases == 1);
    assert(jpeg.empty());
    assert(codec.close() && stops == 1 && releases == 1);
    short_uv = false;
    assert(codec.open(4, 4));
    flush_fail = true;
    assert(!codec.encode(frame, jpeg));
    assert(queues == 1 && stops == 2 && releases == 2);
    flush_fail = false;
    assert(codec.open(4, 4));
    output_fail = true;
    assert(!codec.encode(frame, jpeg));
    assert(queues == 2 && stops == 3 && releases == 3);
    assert(codec.close() && stops == 3 && releases == 3);
    assert(codec.open(4, 4));
    wrong_size = true;
    assert(!codec.encode(frame, jpeg));
    assert(queues == 2 && stops == 4 && releases == 4);
    wrong_size = false;
    assert(codec.open(4, 4));
    queue_fail = true;
    assert(!codec.encode(frame, jpeg));
    assert(queues == 3 && stops == 5 && releases == 5);
    assert(codec.close() && stops == 5 && releases == 5);
    queue_fail = false;
    output_fail = false;
    // Stream size is an upper bound with short, zero-only alignment padding.
    // Only copy through EOI; never trust jpeg_stream_info.frame_size as output size.
    // Real X5 single-camera acceptance reported exactly one trailing zero.
    zero_padding = 1;
    assert(codec.open(4, 4));
    assert(codec.encode(frame, jpeg) && jpeg.size() == 22 && jpeg[20] == 0xff && jpeg[21] == 0xd9);
    assert(codec.close());
    zero_padding = 10;
    assert(codec.open(4, 4));
    assert(codec.encode(frame, jpeg) && jpeg.size() == 22 && jpeg[20] == 0xff && jpeg[21] == 0xd9);
    assert(codec.close());
    zero_padding = 15;
    assert(codec.open(4, 4));
    assert(codec.encode(frame, jpeg) && jpeg.size() == 22);
    assert(codec.close());
    zero_padding = 16; // padding beyond the bounded alignment is not accepted
    assert(codec.open(4, 4));
    assert(!codec.encode(frame, jpeg) && jpeg.empty());
    assert(codec.close());
    zero_padding = 10;
    nonzero_padding = true;
    assert(codec.open(4, 4));
    assert(!codec.encode(frame, jpeg) && jpeg.empty());
    assert(codec.close());
    nonzero_padding = false;
    damage_eoi = true;
    assert(codec.open(4, 4));
    assert(!codec.encode(frame, jpeg) && jpeg.empty());
    assert(codec.close());
    damage_eoi = false;
    oversize_output = true;
    assert(codec.open(4, 4));
    assert(!codec.encode(frame, jpeg) && jpeg.empty());
    assert(codec.close());
    oversize_output = false;
    wrong_dimensions = true;
    assert(codec.open(4, 4));
    assert(!codec.encode(frame, jpeg) && jpeg.empty());
    assert(codec.close());
}
