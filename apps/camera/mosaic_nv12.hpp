#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace orbvi::mosaic {
constexpr unsigned kCameraCount = 4;
constexpr unsigned kInputWidth = 1088;
constexpr unsigned kInputHeight = 1280;
constexpr unsigned kOutputWidth = 2560;
constexpr unsigned kOutputHeight = 2176;
constexpr size_t kOutputBytes = size_t(kOutputWidth) * kOutputHeight * 3 / 2;

struct Nv12Frame {
    const uint8_t *y = nullptr;
    const uint8_t *uv = nullptr;
    size_t y_bytes = 0;
    size_t uv_bytes = 0;
    unsigned width = 0, height = 0, stride = 0, vstride = 0;
};

// Rotate each portrait camera clockwise into a landscape tile; order is 0 1 / 2 3.
bool compose(const std::array<Nv12Frame, kCameraCount> &sources,
             uint8_t *destination, size_t capacity);

// The caller and three persistent workers transform independent tiles in parallel.
// compose() blocks until all workers finish, while camera frame leases remain valid.
class ParallelComposer {
public:
    ParallelComposer();
    ~ParallelComposer();
    ParallelComposer(const ParallelComposer &) = delete;
    ParallelComposer &operator=(const ParallelComposer &) = delete;
    bool compose(const std::array<Nv12Frame, kCameraCount> &sources,
                 uint8_t *destination, size_t capacity);
private:
    void worker(unsigned camera);
    std::array<std::thread, kCameraCount - 1> workers_;
    std::mutex call_mutex_, mutex_;
    std::condition_variable start_, done_;
    std::array<Nv12Frame, kCameraCount> sources_{};
    uint8_t *destination_ = nullptr;
    unsigned generation_ = 0, completed_ = 0;
    bool stopping_ = false;
};
}
