#include "capture_session.hpp"
#include <cstdio>
#include <ctime>
#include <exception>
#include <thread>

namespace orbvi {
uint64_t monotonic_ns() {
    timespec t{};
    clock_gettime(CLOCK_MONOTONIC, &t);
    return uint64_t(t.tv_sec) * 1000000000u + uint64_t(t.tv_nsec);
}
CaptureSession::CaptureSession(const options &config, std::array<CameraSource *, 4> cameras,
                               std::array<FrameSink *, 4> sinks, const volatile sig_atomic_t &interrupted)
    : config_(config), cameras_(cameras), sinks_(sinks), interrupted_(interrupted) {}

void CaptureSession::capture(unsigned port) {
    unsigned count = 0;
    uint64_t last = monotonic_ns();
    try {
        while (count < config_.frames && !interrupted_ && !failed_.load()) {
            FrameView frame;
            int result = cameras_[port]->acquire(frame, 100);
            uint64_t now = monotonic_ns();
            if (result) {
                if (now - last < uint64_t(config_.timeout_ms) * 1000000) continue;
                std::fprintf(stderr, "camera%u: no frame for %u ms (SDK %d)\n", port, config_.timeout_ms, result);
                failed_.store(true);
                break;
            }
            frame.received_ns = now;
            int error = 0;
            try { error = sinks_[port]->write(count, frame); }
            catch (...) { cameras_[port]->release(); throw; }
            if (cameras_[port]->release()) error = 1;
            if (error) { failed_.store(true); break; }
            ++count;
            last = now;
        }
    } catch (const std::exception &error) {
        std::fprintf(stderr, "camera%u: %s\n", port, error.what());
        failed_.store(true);
    } catch (...) { failed_.store(true); }
    std::printf("camera%u: captured %u/%u frames\n", port, count, config_.frames);
}

int CaptureSession::run() {
    if (used_) return 1;
    used_ = true;
    std::array<std::thread, 4> workers;
    try {
        // Every slave shares FSYNC: prepare all four before starting any selected flow.
        unsigned mask = config_.external ? 15 : config_.mask;
        for (unsigned i = 0; i < 4 && !interrupted_ && !failed_.load(); ++i) {
            if (!(mask & (1u << i))) continue;
            if (!cameras_[i] || cameras_[i]->prepare(config_.external)) { failed_.store(true); break; }
            if ((config_.mask & (1u << i)) && (!sinks_[i] || sinks_[i]->open(i))) failed_.store(true);
        }
        for (unsigned i = 0; i < 4 && !interrupted_ && !failed_.load(); ++i) {
            if (!(config_.mask & (1u << i))) continue;
            if (cameras_[i]->start()) { failed_.store(true); break; }
            workers[i] = std::thread(&CaptureSession::capture, this, i);
        }
        if (!interrupted_ && !failed_.load()) { std::puts("Capture ready"); std::fflush(stdout); }
    } catch (const std::exception &error) {
        std::fprintf(stderr, "capture startup: %s\n", error.what());
        failed_.store(true);
    } catch (...) { failed_.store(true); }
    for (auto &worker : workers) if (worker.joinable()) worker.join();
    for (int i = 3; i >= 0; --i) {
        if (cameras_[i] && cameras_[i]->close()) failed_.store(true);
        if (sinks_[i] && sinks_[i]->close()) failed_.store(true);
    }
    if (failed_.load()) return 1;
    if (interrupted_) return 130;
    std::puts("Capture complete");
    return 0;
}
}
