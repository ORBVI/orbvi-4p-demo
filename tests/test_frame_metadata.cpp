#include "frame_metadata.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    const orbvi::camera::FrameMetadata input{2, 0x12345678U, 0x0102030405060708ULL};
    const auto sei = orbvi::camera::frame_metadata_sei(input);
    const std::vector<uint8_t> expected{
        0, 0, 0, 1, 6, 5, 30,
        0x4f, 0x52, 0x42, 0x56, 0x49, 0x2d, 0x34, 0x50,
        0xa5, 0x68, 0x49, 0x26, 0xb2, 0x84, 0x7d, 0x91,
        1, 2, 0x12, 0x34, 0x56, 0x78,
        1, 2, 3, 4, 5, 6, 7, 8, 0x80};
    assert(sei == expected);
    const auto escaped = orbvi::camera::frame_metadata_sei({0, 0, 1});
    for (std::size_t index = 5; index + 2 < escaped.size(); ++index)
        assert(!(escaped[index] == 0 && escaped[index + 1] == 0 && escaped[index + 2] <= 2));
}
