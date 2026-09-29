#pragma once
#include "options.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <csignal>

namespace orbvi {
// Pixels are borrowed from the source until release(); sinks must not retain them.
struct FrameView {
    const void *data = nullptr;
    size_t bytes = 0;
    uint32_t id = 0, width = 0, height = 0, stride = 0;
    uint64_t timestamp = 0, system_timestamp = 0, received_ns = 0;
};
class CameraSource {
public:
    virtual ~CameraSource() = default;
    virtual int prepare(bool external) = 0;
    virtual int start() = 0;
    virtual int acquire(FrameView &frame, unsigned timeout_ms) = 0;
    virtual int release() = 0;
    // Idempotent, including after partial initialization. Must not throw.
    virtual int close() = 0;
};
class FrameSink {
public:
    virtual ~FrameSink() = default;
    virtual int open(unsigned port) = 0;
    virtual int write(unsigned index, const FrameView &frame) = 0;
    virtual int close() = 0;
};
class CaptureSession {
public:
    CaptureSession(const options &config, std::array<CameraSource *, 4> cameras,
                   std::array<FrameSink *, 4> sinks, const volatile sig_atomic_t &interrupted);
    // Single use. Dependencies must outlive run(); joins all workers before returning.
    int run();
private:
    void capture(unsigned port);
    options config_;
    std::array<CameraSource *, 4> cameras_;
    std::array<FrameSink *, 4> sinks_;
    const volatile sig_atomic_t &interrupted_;
    std::atomic_bool failed_{false};
    bool used_ = false;
};
uint64_t monotonic_ns();
}
