#pragma once
#include "camera_options.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

namespace orbvi::camera {
struct Nv12View {
    const unsigned char *y;
    const unsigned char *uv;
    unsigned width, height, stride, vstride;
};
bool copy_nv12(const Nv12View &source, unsigned char *target, size_t capacity);

class Encoder {
public:
    using Output = std::function<int(const void *, size_t, uint64_t)>;
    Encoder(unsigned width, unsigned height, const Options &options, Output output);
    ~Encoder();
    int open();
    int encode(const Nv12View &frame, uint64_t timestamp_ns);
    int close();
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
