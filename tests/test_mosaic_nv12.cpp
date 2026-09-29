#include "mosaic_nv12.hpp"
#include <array>
#include <cassert>
#include <vector>

using namespace orbvi::mosaic;
int main() {
    // Different pixels and padded source strides catch UV ordering, tile
    // boundaries, 90-degree rotation and accidental reads of stride padding.
    constexpr unsigned stride = kInputWidth + 32;
    std::array<std::vector<uint8_t>, kCameraCount> y, uv;
    std::array<Nv12Frame, kCameraCount> views{};
    for (unsigned camera = 0; camera < kCameraCount; ++camera) {
        y[camera].assign(size_t(stride) * kInputHeight, 0xff);
        uv[camera].assign(size_t(stride) * kInputHeight / 2, 0xff);
        for (unsigned row = 0; row < kInputHeight; ++row)
            for (unsigned col = 0; col < kInputWidth; ++col)
                y[camera][size_t(row) * stride + col] = uint8_t((camera * 23 + row * 7 + col * 3) % 251);
        for (unsigned row = 0; row < kInputHeight / 2; ++row)
            for (unsigned col = 0; col < kInputWidth; ++col)
                uv[camera][size_t(row) * stride + col] = uint8_t((camera * 37 + row * 11 + col * 5) % 251);
        views[camera] = {y[camera].data(), uv[camera].data(), y[camera].size(), uv[camera].size(),
                         kInputWidth, kInputHeight, stride, kInputHeight};
    }
    std::vector<uint8_t> output(kOutputBytes + 1, 0x77);
    assert(compose(views, output.data(), kOutputBytes));
    for (unsigned camera = 0; camera < kCameraCount; ++camera) {
        const unsigned tile_x = (camera % 2) * kInputHeight;
        const unsigned tile_y = (camera / 2) * kInputWidth;
        for (unsigned x = 0; x < kInputWidth; ++x)
            for (unsigned row = 0; row < kInputHeight; ++row)
                assert(output[size_t(tile_y + x) * kOutputWidth + tile_x + kInputHeight - 1 - row] ==
                       y[camera][size_t(row) * stride + x]);
        for (unsigned x = 0; x < kInputWidth / 2; ++x)
            for (unsigned row = 0; row < kInputHeight / 2; ++row)
                for (unsigned uv_byte = 0; uv_byte < 2; ++uv_byte)
                    assert(output[size_t(kOutputWidth) * kOutputHeight +
                                  size_t(tile_y / 2 + x) * kOutputWidth + tile_x +
                                  2 * (kInputHeight / 2 - 1 - row) + uv_byte] ==
                           uv[camera][size_t(row) * stride + 2 * x + uv_byte]);
    }
    // Independent tiles must produce exactly the same bytes as serial compose.
    const std::vector<uint8_t> expected(output.begin(), output.begin() + kOutputBytes);
    {
        ParallelComposer parallel;
        for (unsigned iteration = 0; iteration < 12; ++iteration) {
            output.assign(kOutputBytes + 1, 0x77);
            assert(parallel.compose(views, output.data(), kOutputBytes));
            assert(std::equal(expected.begin(), expected.end(), output.begin()));
            assert(output[kOutputBytes] == 0x77);
        }
        assert(!parallel.compose(views, output.data(), kOutputBytes - 1));
        views[2].uv_bytes = 1;
        assert(!parallel.compose(views, output.data(), kOutputBytes));
        views[2].uv_bytes = uv[2].size();
    }
    assert(output[kOutputBytes] == 0x77);
    assert(!compose(views, output.data(), kOutputBytes - 1));
    views[2].y_bytes = 1;
    assert(!compose(views, output.data(), kOutputBytes));
}
