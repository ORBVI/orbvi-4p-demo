#include "protocol.hpp"

#include <cstring>
#include <limits>

namespace orbvi::reference_images {
namespace {

void put_u16(std::uint8_t *output, std::uint16_t value) {
    output[0] = static_cast<std::uint8_t>(value >> 8);
    output[1] = static_cast<std::uint8_t>(value);
}

void put_u32(std::uint8_t *output, std::uint32_t value) {
    output[0] = static_cast<std::uint8_t>(value >> 24);
    output[1] = static_cast<std::uint8_t>(value >> 16);
    output[2] = static_cast<std::uint8_t>(value >> 8);
    output[3] = static_cast<std::uint8_t>(value);
}

void put_u64(std::uint8_t *output, std::uint64_t value) {
    for (unsigned i = 0; i < 8; ++i)
        output[i] = static_cast<std::uint8_t>(value >> (56 - 8 * i));
}

std::uint16_t get_u16(const std::uint8_t *input) {
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(input[0]) << 8) |
                                     static_cast<std::uint16_t>(input[1]));
}

std::uint32_t get_u32(const std::uint8_t *input) {
    return (static_cast<std::uint32_t>(input[0]) << 24) |
           (static_cast<std::uint32_t>(input[1]) << 16) |
           (static_cast<std::uint32_t>(input[2]) << 8) |
           static_cast<std::uint32_t>(input[3]);
}

std::uint64_t get_u64(const std::uint8_t *input) {
    std::uint64_t value = 0;
    for (unsigned i = 0; i < 8; ++i)
        value = (value << 8) | input[i];
    return value;
}

bool is_sof_marker(std::uint8_t marker) {
    switch (marker) {
    case 0xc0: case 0xc1: case 0xc2: case 0xc3:
    case 0xc5: case 0xc6: case 0xc7:
    case 0xc9: case 0xca: case 0xcb:
    case 0xcd: case 0xce: case 0xcf:
        return true;
    default:
        return false;
    }
}

bool is_standalone_marker(std::uint8_t marker) {
    return marker == 0x01 || marker == 0xd8 || marker == 0xd9 ||
           (marker >= 0xd0 && marker <= 0xd7);
}

}  // namespace

std::uint32_t crc32_iso_hdlc(const std::uint8_t *data, std::size_t size) {
    std::uint32_t crc = 0xffffffffu;
    for (std::size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return crc ^ 0xffffffffu;
}

bool encode_start(std::uint8_t camera, std::array<std::uint8_t, kControlSize> &output) {
    if (camera > 3) return false;
    output[0] = 'O'; output[1] = 'C'; output[2] = 'T'; output[3] = 'L';
    output[4] = kProtocolVersion;
    output[5] = kStartCommand;
    output[6] = camera;
    output[7] = 0;
    return true;
}

bool decode_start(const std::uint8_t *data, std::size_t size, Control &output, std::string &error) {
    if (!data || size != kControlSize) {
        error = "START must be exactly 8 bytes";
        return false;
    }
    if (std::memcmp(data, "OCTL", 4) != 0) {
        error = "invalid START magic";
        return false;
    }
    Control parsed;
    parsed.version = data[4];
    parsed.command = data[5];
    parsed.camera = data[6];
    parsed.reserved = data[7];
    if (parsed.version != kProtocolVersion) {
        error = "unsupported protocol version";
        return false;
    }
    if (parsed.command != kStartCommand) {
        error = "unsupported control command";
        return false;
    }
    if (parsed.camera > 3) {
        error = "camera index out of range";
        return false;
    }
    if (parsed.reserved != 0) {
        error = "reserved START byte is nonzero";
        return false;
    }
    output = parsed;
    error.clear();
    return true;
}

bool encode_frame_header(const FrameHeader &header,
                         std::array<std::uint8_t, kFrameHeaderSize> &output) {
    const std::uint8_t kind = static_cast<std::uint8_t>(header.kind);
    if (header.version != kProtocolVersion || kind < 1 || kind > 2 ||
        header.camera > 3 || header.reserved != 0 ||
        header.header_size != kFrameHeaderSize || header.reserved2 != 0 ||
        header.sequence == 0 || header.width == 0 || header.height == 0 ||
        header.payload_size == 0) {
        return false;
    }
    if (header.kind == Kind::Nv12) {
        std::uint64_t expected = 0;
        if (header.stride != header.width ||
            !valid_nv12_size(header.width, header.height, expected) ||
            header.payload_size != expected)
            return false;
    } else if (header.stride != 0) {
        return false;
    }

    output.fill(0);
    output[0] = 'O'; output[1] = 'I'; output[2] = 'M'; output[3] = 'G';
    output[4] = header.version;
    output[5] = kind;
    output[6] = header.camera;
    output[7] = header.reserved;
    put_u16(output.data() + 8, header.header_size);
    put_u16(output.data() + 10, header.reserved2);
    put_u64(output.data() + 12, header.sequence);
    put_u64(output.data() + 20, header.timestamp_ns);
    put_u32(output.data() + 28, header.width);
    put_u32(output.data() + 32, header.height);
    put_u32(output.data() + 36, header.stride);
    put_u32(output.data() + 40, header.payload_size);
    put_u32(output.data() + 44, header.crc32);
    return true;
}

bool decode_frame_header(const std::uint8_t *data, std::size_t size,
                         FrameHeader &output, std::string &error) {
    if (!data || size != kFrameHeaderSize) {
        error = "OIMG header must be exactly 48 bytes";
        return false;
    }
    if (std::memcmp(data, "OIMG", 4) != 0) {
        error = "invalid OIMG magic";
        return false;
    }
    FrameHeader parsed;
    parsed.version = data[4];
    parsed.kind = static_cast<Kind>(data[5]);
    parsed.camera = data[6];
    parsed.reserved = data[7];
    parsed.header_size = get_u16(data + 8);
    parsed.reserved2 = get_u16(data + 10);
    parsed.sequence = get_u64(data + 12);
    parsed.timestamp_ns = get_u64(data + 20);
    parsed.width = get_u32(data + 28);
    parsed.height = get_u32(data + 32);
    parsed.stride = get_u32(data + 36);
    parsed.payload_size = get_u32(data + 40);
    parsed.crc32 = get_u32(data + 44);

    const std::uint8_t kind = static_cast<std::uint8_t>(parsed.kind);
    if (parsed.version != kProtocolVersion) {
        error = "unsupported OIMG version";
        return false;
    }
    if (kind != 1 && kind != 2) {
        error = "unsupported OIMG kind";
        return false;
    }
    if (parsed.camera > 3) {
        error = "OIMG camera index out of range";
        return false;
    }
    if (parsed.reserved != 0 || parsed.header_size != kFrameHeaderSize ||
        parsed.reserved2 != 0) {
        error = "invalid OIMG reserved/header fields";
        return false;
    }
    if (parsed.sequence == 0 || parsed.width == 0 || parsed.height == 0 ||
        parsed.payload_size == 0) {
        error = "invalid OIMG sequence/dimensions/payload size";
        return false;
    }
    if (parsed.kind == Kind::Nv12) {
        std::uint64_t expected = 0;
        if (parsed.stride != parsed.width ||
            !valid_nv12_size(parsed.width, parsed.height, expected)) {
            error = "invalid tight NV12 dimensions/stride";
            return false;
        }
        if (parsed.payload_size != expected) {
            error = "NV12 payload size does not match width*height*3/2";
            return false;
        }
    } else if (parsed.stride != 0) {
        error = "JPEG header must use stride=0";
        return false;
    }
    output = parsed;
    error.clear();
    return true;
}

bool valid_nv12_size(std::uint32_t width, std::uint32_t height, std::uint64_t &size) {
    if (width == 0 || height == 0 || (width & 1u) || (height & 1u)) return false;
    const std::uint64_t pixels = static_cast<std::uint64_t>(width) * height;
    if (pixels > (std::numeric_limits<std::uint64_t>::max() / 3u)) return false;
    size = pixels * 3u / 2u;
    return size > 0;
}

bool jpeg_has_complete_signature(const std::uint8_t *data, std::size_t size) {
    return data && size >= 4 && data[0] == 0xff && data[1] == 0xd8 &&
           data[size - 2] == 0xff && data[size - 1] == 0xd9;
}

bool jpeg_probe_dimensions(const std::uint8_t *data, std::size_t size,
                           std::uint32_t &width, std::uint32_t &height) {
    if (!jpeg_has_complete_signature(data, size)) return false;
    std::size_t position = 2;
    while (position < size) {
        while (position < size && data[position] == 0xff) ++position;
        if (position >= size) break;
        const std::uint8_t marker = data[position++];
        if (marker == 0x00 || marker == 0xff) continue;
        if (is_standalone_marker(marker)) {
            if (marker == 0xd9) return false;
            continue;
        }
        if (position + 2 > size) return false;
        const std::uint16_t segment_size = get_u16(data + position);
        if (segment_size < 2 || position + segment_size > size) return false;
        if (is_sof_marker(marker)) {
            // SOF: two length bytes, precision, height BE16, width BE16.
            if (segment_size < 8) return false;
            height = (static_cast<std::uint32_t>(data[position + 3]) << 8) |
                     data[position + 4];
            width = (static_cast<std::uint32_t>(data[position + 5]) << 8) |
                    data[position + 6];
        } else if (marker == 0xda) {
            // Entropy-coded bytes are not length-prefixed JPEG segments. The
            // complete EOI signature was checked above; do not parse scan data
            // as markers (including stuffed 0xff00 or restart markers).
            return width != 0 && height != 0;
        }
        position += segment_size;
    }
    return false;
}

bool validate_payload(const FrameHeader &header, const std::uint8_t *data,
                      std::size_t size, std::string &error) {
    if (!data || size != header.payload_size) {
        error = "payload length does not match OIMG header";
        return false;
    }
    if (crc32_iso_hdlc(data, size) != header.crc32) {
        error = "payload CRC32 mismatch";
        return false;
    }
    if (header.kind == Kind::Nv12) {
        std::uint64_t expected = 0;
        if (!valid_nv12_size(header.width, header.height, expected) ||
            expected != size || header.stride != header.width) {
            error = "NV12 payload is not tight";
            return false;
        }
    } else {
        if (!jpeg_has_complete_signature(data, size)) {
            error = "JPEG payload lacks SOI/EOI markers";
            return false;
        }
        std::uint32_t width = 0, height = 0;
        if (!jpeg_probe_dimensions(data, size, width, height) ||
            width != header.width || height != header.height) {
            error = "JPEG dimensions do not match OIMG header";
            return false;
        }
    }
    error.clear();
    return true;
}

const char *kind_name(Kind kind) {
    return kind == Kind::Jpeg ? "jpeg" : "nv12";
}

}  // namespace orbvi::reference_images
