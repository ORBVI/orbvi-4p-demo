#pragma once
#include "frame_set.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace orbvi::reference_images {
// Validate source plane extents before reading from a leased BSP frame.
bool valid_source(const nv12::Frame &frame);
bool copy_plane_rows(const std::uint8_t *source, std::size_t source_bytes,
                     std::size_t source_stride, std::uint8_t *target,
                     std::size_t target_bytes, std::size_t target_stride,
                     std::size_t width, std::size_t rows);
bool pack_nv12(const nv12::Frame &frame, std::vector<std::uint8_t> &output);
bool copy_codec_nv12(const nv12::Frame &frame, std::uint8_t *y, std::size_t y_bytes,
                     std::size_t y_stride, std::uint8_t *uv, std::size_t uv_bytes,
                     std::size_t uv_stride);
}
