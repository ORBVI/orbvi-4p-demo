#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace orbvi::reference_images {

constexpr std::size_t kControlSize = 8;
constexpr std::size_t kFrameHeaderSize = 48;
constexpr std::uint8_t kProtocolVersion = 1;
constexpr std::uint8_t kStartCommand = 1;

enum class Kind : std::uint8_t {
    Nv12 = 1,
    Jpeg = 2,
};

struct Control {
    std::uint8_t version = kProtocolVersion;
    std::uint8_t command = kStartCommand;
    std::uint8_t camera = 0;
    std::uint8_t reserved = 0;
};

struct FrameHeader {
    std::uint8_t version = kProtocolVersion;
    Kind kind = Kind::Nv12;
    std::uint8_t camera = 0;
    std::uint8_t reserved = 0;
    std::uint16_t header_size = static_cast<std::uint16_t>(kFrameHeaderSize);
    std::uint16_t reserved2 = 0;
    std::uint64_t sequence = 0;
    std::uint64_t timestamp_ns = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t stride = 0;
    std::uint32_t payload_size = 0;
    std::uint32_t crc32 = 0;
};

std::uint32_t crc32_iso_hdlc(const std::uint8_t *data, std::size_t size);

bool encode_start(std::uint8_t camera, std::array<std::uint8_t, kControlSize> &output);
bool decode_start(const std::uint8_t *data, std::size_t size, Control &output, std::string &error);

bool encode_frame_header(const FrameHeader &header,
                         std::array<std::uint8_t, kFrameHeaderSize> &output);
bool decode_frame_header(const std::uint8_t *data, std::size_t size,
                         FrameHeader &output, std::string &error);

bool valid_nv12_size(std::uint32_t width, std::uint32_t height, std::uint64_t &size);
bool jpeg_has_complete_signature(const std::uint8_t *data, std::size_t size);
bool jpeg_probe_dimensions(const std::uint8_t *data, std::size_t size,
                           std::uint32_t &width, std::uint32_t &height);
bool validate_payload(const FrameHeader &header, const std::uint8_t *data,
                      std::size_t size, std::string &error);

const char *kind_name(Kind kind);

}  // namespace orbvi::reference_images
