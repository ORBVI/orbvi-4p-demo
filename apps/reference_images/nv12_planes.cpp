#include "nv12_planes.hpp"
#include "protocol.hpp"
#include <cstring>
#include <limits>

namespace orbvi::reference_images {
namespace {
bool extent(std::size_t bytes, std::size_t stride, std::size_t width, std::size_t rows) {
    return width && rows && stride >= width &&
           (rows - 1) <= (std::numeric_limits<std::size_t>::max() - width) / stride &&
           bytes >= (rows - 1) * stride + width;
}
}
bool valid_source(const nv12::Frame &frame) {
    std::uint64_t size = 0;
    return frame.y && frame.uv && valid_nv12_size(frame.width, frame.height, size) &&
           frame.vstride >= frame.height &&
           extent(frame.y_bytes, frame.stride, frame.width, frame.height) &&
           extent(frame.uv_bytes, frame.stride, frame.width, frame.height / 2);
}
bool copy_plane_rows(const std::uint8_t *source, std::size_t source_bytes,
                     std::size_t source_stride, std::uint8_t *target,
                     std::size_t target_bytes, std::size_t target_stride,
                     std::size_t width, std::size_t rows) {
    if (!source || !target || !extent(source_bytes, source_stride, width, rows) ||
        !extent(target_bytes, target_stride, width, rows)) return false;
    for (std::size_t row = 0; row < rows; ++row)
        std::memcpy(target + row * target_stride, source + row * source_stride, width);
    return true;
}
bool pack_nv12(const nv12::Frame &frame, std::vector<std::uint8_t> &output) {
    if (!valid_source(frame)) return false;
    std::uint64_t size = 0;
    valid_nv12_size(frame.width, frame.height, size);
    if (size > std::numeric_limits<std::uint32_t>::max()) return false;
    output.resize(static_cast<std::size_t>(size));
    const auto ysize = std::size_t(frame.width) * frame.height;
    return copy_codec_nv12(frame, output.data(), ysize, frame.width,
                           output.data() + ysize, output.size() - ysize, frame.width);
}
bool copy_codec_nv12(const nv12::Frame &frame, std::uint8_t *y, std::size_t y_bytes,
                     std::size_t y_stride, std::uint8_t *uv, std::size_t uv_bytes,
                     std::size_t uv_stride) {
    return valid_source(frame) &&
        copy_plane_rows(static_cast<const std::uint8_t *>(frame.y), frame.y_bytes, frame.stride,
                        y, y_bytes, y_stride, frame.width, frame.height) &&
        copy_plane_rows(static_cast<const std::uint8_t *>(frame.uv), frame.uv_bytes, frame.stride,
                        uv, uv_bytes, uv_stride, frame.width, frame.height / 2);
}
}
