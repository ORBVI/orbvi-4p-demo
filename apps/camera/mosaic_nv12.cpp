#include "mosaic_nv12.hpp"

namespace orbvi::mosaic {
namespace {
bool valid(const std::array<Nv12Frame, kCameraCount> &sources,
           uint8_t *destination, size_t capacity) {
    if (!destination || capacity < kOutputBytes) return false;
    for (const auto &source : sources) {
        if (!source.y || !source.uv || source.width != kInputWidth ||
            source.height != kInputHeight || source.stride < source.width ||
            source.vstride < source.height ||
            source.y_bytes < size_t(source.height - 1) * source.stride + source.width ||
            source.uv_bytes < size_t(source.height / 2 - 1) * source.stride + source.width)
            return false;
    }
    return true;
}

void compose_tile(unsigned camera, const Nv12Frame &source, uint8_t *destination) {
    auto *output_uv = destination + size_t(kOutputWidth) * kOutputHeight;
    // A small source tile stays in cache while we transpose/rotate it into the
    // output canvas. This avoids rereading whole portrait rows for every x.
    constexpr unsigned block = 32;
    const unsigned tile_x = (camera % 2) * kInputHeight;
    const unsigned tile_y = (camera / 2) * kInputWidth;
    for (unsigned xb = 0; xb < kInputWidth; xb += block) {
        for (unsigned yb = 0; yb < kInputHeight; yb += block) {
            for (unsigned x = xb; x < xb + block; ++x) {
                auto *row = destination + size_t(tile_y + x) * kOutputWidth + tile_x;
                for (unsigned y = yb; y < yb + block; ++y)
                    row[kInputHeight - 1 - y] = source.y[size_t(y) * source.stride + x];
            }
        }
    }
    for (unsigned xb = 0; xb < kInputWidth / 2; xb += block) {
        for (unsigned yb = 0; yb < kInputHeight / 2; yb += block) {
            for (unsigned x = xb; x < xb + block; ++x) {
                auto *row = output_uv + size_t(tile_y / 2 + x) * kOutputWidth + tile_x;
                for (unsigned y = yb; y < yb + block; ++y) {
                    const size_t from = size_t(y) * source.stride + 2 * x;
                    const unsigned to = 2 * (kInputHeight / 2 - 1 - y);
                    row[to] = source.uv[from];
                    row[to + 1] = source.uv[from + 1];
                }
            }
        }
    }
}
}

bool compose(const std::array<Nv12Frame, kCameraCount> &sources,
             uint8_t *destination, size_t capacity) {
    if (!valid(sources, destination, capacity)) return false;
    for (unsigned camera = 0; camera < kCameraCount; ++camera)
        compose_tile(camera, sources[camera], destination);
    return true;
}

ParallelComposer::ParallelComposer() {
    try {
        for (unsigned camera = 1; camera < kCameraCount; ++camera)
            workers_[camera - 1] = std::thread(&ParallelComposer::worker, this, camera);
    } catch (...) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
        }
        start_.notify_all();
        for (auto &thread : workers_) if (thread.joinable()) thread.join();
        throw;
    }
}

ParallelComposer::~ParallelComposer() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }
    start_.notify_all();
    for (auto &thread : workers_) if (thread.joinable()) thread.join();
}

void ParallelComposer::worker(unsigned camera) {
    unsigned seen = 0;
    for (;;) {
        Nv12Frame source;
        uint8_t *destination;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            start_.wait(lock, [&] { return stopping_ || generation_ != seen; });
            if (stopping_) return;
            seen = generation_;
            source = sources_[camera];
            destination = destination_;
        }
        compose_tile(camera, source, destination);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++completed_;
        }
        done_.notify_one();
    }
}

bool ParallelComposer::compose(const std::array<Nv12Frame, kCameraCount> &sources,
                               uint8_t *destination, size_t capacity) {
    std::lock_guard<std::mutex> call(call_mutex_);
    if (!valid(sources, destination, capacity)) return false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sources_ = sources;
        destination_ = destination;
        completed_ = 0;
        ++generation_;
    }
    start_.notify_all();
    compose_tile(0, sources[0], destination);
    std::unique_lock<std::mutex> lock(mutex_);
    done_.wait(lock, [&] { return completed_ == kCameraCount - 1; });
    return true;
}
}
