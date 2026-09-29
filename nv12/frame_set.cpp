#include "frame_set.hpp"
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <exception>

namespace orbvi::nv12 {
namespace {
uint64_t monotonic_ns() {
    timespec time{};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return uint64_t(time.tv_sec) * 1000000000 + uint64_t(time.tv_nsec);
}
}

Session::Session(std::array<Source *, 4> cameras, Sink &sink,
                 const volatile sig_atomic_t &interrupted, unsigned groups,
                 uint64_t max_skew_ns, unsigned timeout_ms)
    : cameras_(cameras), sink_(sink), interrupted_(interrupted), groups_(groups),
      max_skew_ns_(max_skew_ns), timeout_ms_(timeout_ms) {}

int Session::run() {
    if (used_) return 1;
    used_ = true;
    FrameSet set;
    std::array<bool, 4> leased{};
    int result = 0;
    uint64_t last_group = monotonic_ns();
    try {
        for (auto *camera : cameras_) {
            if (!camera || camera->prepare()) { result = 1; break; }
        }
        if (!result && sink_.open()) result = 1;
        if (!result) {
            for (auto *camera : cameras_) {
                if (camera->start()) { result = 1; break; }
            }
        }
        for (unsigned group = 0; !result && !interrupted_ && group < groups_;) {
            for (unsigned port = 0; port < 4 && !interrupted_; ++port) {
                if (!leased[port] && cameras_[port]->acquire(set.frames[port], 100) == 0)
                    leased[port] = true;
            }
            if (interrupted_) break;
            if (std::any_of(leased.begin(), leased.end(), [](bool held) { return !held; })) {
                if (monotonic_ns() - last_group >= uint64_t(timeout_ms_) * 1000000) {
                    std::fprintf(stderr, "NV12 frame set: no complete group for %u ms\n", timeout_ms_);
                    result = 1;
                }
                continue;
            }
            auto range = std::minmax_element(set.frames.begin(), set.frames.end(),
                [](const Frame &a, const Frame &b) { return a.timestamp_ns < b.timestamp_ns; });
            set.timestamp_ns = range.first->timestamp_ns;
            set.max_skew_ns = range.second->timestamp_ns - set.timestamp_ns;
            if (set.max_skew_ns > max_skew_ns_) {
                unsigned oldest = static_cast<unsigned>(range.first - set.frames.begin());
                if (cameras_[oldest]->release()) result = 1;
                leased[oldest] = false;
                continue;
            }
            if (sink_.write(group, set)) result = 1;
            for (int port = 3; port >= 0; --port) {
                if (cameras_[port]->release()) result = 1;
                leased[port] = false;
            }
            ++group;
            last_group = monotonic_ns();
        }
    } catch (const std::exception &error) {
        std::fprintf(stderr, "NV12 frame set: %s\n", error.what());
        result = 1;
    } catch (...) { result = 1; }
    for (int port = 3; port >= 0; --port) {
        if (leased[port] && cameras_[port]->release()) result = 1;
        if (cameras_[port] && cameras_[port]->close()) result = 1;
    }
    if (sink_.close()) result = 1;
    if (result) return 1;
    return interrupted_ ? 130 : 0;
}
}
