#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <csignal>

namespace orbvi::nv12 {
// Y and UV belong to the camera until release(); Sink::write must not retain them.
struct Frame {
    const void *y = nullptr;
    const void *uv = nullptr;
    size_t y_bytes = 0;
    size_t uv_bytes = 0;
    uint32_t id = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t stride = 0;
    uint32_t vstride = 0;
    uint64_t timestamp_ns = 0;
    uint64_t system_timestamp_ns = 0;
};

struct FrameSet {
    std::array<Frame, 4> frames{};
    uint64_t timestamp_ns = 0;
    uint64_t max_skew_ns = 0;
};

class Source {
public:
    virtual ~Source() = default;
    virtual int prepare() = 0;
    virtual int start() = 0;
    virtual int acquire(Frame &frame, unsigned timeout_ms) = 0;
    virtual int release() = 0;
    virtual int close() = 0;
};

class Sink {
public:
    virtual ~Sink() = default;
    virtual int open() = 0;
    virtual int write(unsigned index, const FrameSet &set) = 0;
    virtual int close() = 0;
};

class Session {
public:
    Session(std::array<Source *, 4> cameras, Sink &sink, const volatile sig_atomic_t &interrupted,
            unsigned groups, uint64_t max_skew_ns, unsigned timeout_ms);
    int run();
private:
    std::array<Source *, 4> cameras_;
    Sink &sink_;
    const volatile sig_atomic_t &interrupted_;
    unsigned groups_;
    uint64_t max_skew_ns_;
    unsigned timeout_ms_;
    bool used_ = false;
};
}
