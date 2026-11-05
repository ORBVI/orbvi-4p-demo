#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace orbvi::camera {
struct FrameMetadata {
    uint8_t camera_id;
    uint32_t frame_id;
    uint64_t timestamp_ns;
};

inline std::vector<uint8_t> frame_metadata_sei(const FrameMetadata &metadata) {
    constexpr std::array<uint8_t, 16> uuid{
        0x4f, 0x52, 0x42, 0x56, 0x49, 0x2d, 0x34, 0x50,
        0xa5, 0x68, 0x49, 0x26, 0xb2, 0x84, 0x7d, 0x91};
    std::vector<uint8_t> raw{5, 30};
    raw.insert(raw.end(), uuid.begin(), uuid.end());
    raw.push_back(1);
    raw.push_back(metadata.camera_id);
    for (int shift = 24; shift >= 0; shift -= 8)
        raw.push_back(static_cast<uint8_t>(metadata.frame_id >> shift));
    for (int shift = 56; shift >= 0; shift -= 8)
        raw.push_back(static_cast<uint8_t>(metadata.timestamp_ns >> shift));
    raw.push_back(0x80);
    std::vector<uint8_t> result{0, 0, 0, 1, 6};
    unsigned zeros = 0;
    for (uint8_t byte : raw) {
        if (zeros >= 2 && byte <= 3) { result.push_back(3); zeros = 0; }
        result.push_back(byte);
        zeros = byte == 0 ? zeros + 1 : 0;
    }
    return result;
}
}
