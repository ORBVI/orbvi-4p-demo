#pragma once
#include "frame_set.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace orbvi::reference_images {
class JpegCodec {
public:
    explicit JpegCodec(unsigned camera = 255);
    ~JpegCodec();
    JpegCodec(const JpegCodec &) = delete;
    JpegCodec &operator=(const JpegCodec &) = delete;
    bool open(unsigned width, unsigned height);
    bool encode(const nv12::Frame &frame, std::vector<std::uint8_t> &jpeg);
    bool close();
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
