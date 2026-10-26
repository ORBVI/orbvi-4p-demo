#include "mosaic_nv12.hpp"
#include "mosaic_options.hpp"
#include "encoder.hpp"
#include "rtsp_server.hpp"
#include "x5_camera.hpp"
#include "frame_set.hpp"
#include "../version.hpp"
#include "hb_mem_mgr.h"
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <deque>
#include <fcntl.h>
#include <fstream>
#include <limits>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {
volatile sig_atomic_t stopped = 0;
void stop(int) { stopped = 1; }
using Clock = std::chrono::steady_clock;
uint64_t elapsed_ns(Clock::time_point from) {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - from).count());
}
uint64_t monotonic_ns() {
    timespec now{};
    if (clock_gettime(CLOCK_MONOTONIC, &now)) return 0;
    return static_cast<uint64_t>(now.tv_sec) * 1000000000ULL + static_cast<uint64_t>(now.tv_nsec);
}
int sleep_until(uint64_t target_ns) {
    timespec target{static_cast<time_t>(target_ns / 1000000000ULL),
                    static_cast<long>(target_ns % 1000000000ULL)};
    int result;
    do {
        result = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &target, nullptr);
    } while (result == EINTR);
    return result;
}

class GpioTrigger {
public:
    explicit GpioTrigger(unsigned hz) : hz_(hz) {}

    ~GpioTrigger() { close(); }

    int open_gpio() {
        char gpio_dir[64];
        char gpio_value[80];
        std::snprintf(gpio_dir, sizeof(gpio_dir), "/sys/class/gpio/gpio%u", gpio_);
        std::snprintf(gpio_value, sizeof(gpio_value), "%s/value", gpio_dir);
        if (access(gpio_dir, F_OK) == 0) { errno = EBUSY; return -1; }
        if (write_text("/sys/class/gpio/export", std::to_string(gpio_))) return -1;
        exported_ = true;
        if (write_text(std::string(gpio_dir) + "/direction", "low")) return -1;
        fd_ = ::open(gpio_value, O_WRONLY | O_CLOEXEC);
        return fd_ < 0 ? -1 : 0;
    }

    int start() {
        if (fd_ < 0) { errno = EINVAL; return -1; }
        stop_.store(false);
        worker_ = std::thread(&GpioTrigger::run, this);
        return 0;
    }

    void stop() {
        stop_.store(true);
        if (worker_.joinable()) worker_.join();
    }

    uint64_t pulses() const { return pulses_.load(); }
    int failure() const { return failure_.load(); }

private:
    static int write_text(const std::string &path, const std::string &text) {
        const int fd = ::open(path.c_str(), O_WRONLY | O_CLOEXEC);
        if (fd < 0) return -1;
        const ssize_t written = ::write(fd, text.data(), text.size());
        const int saved = errno;
        ::close(fd);
        errno = saved;
        return written == static_cast<ssize_t>(text.size()) ? 0 : -1;
    }

    int set(char value) {
        if (lseek(fd_, 0, SEEK_SET) < 0) return -1;
        return ::write(fd_, &value, 1) == 1 ? 0 : -1;
    }

    void run() {
        sched_param parameter{};
        parameter.sched_priority = 80;
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &parameter))
            std::fputs("mosaic: SCHED_FIFO unavailable for GPIO trigger\n", stderr);
        const uint64_t start = monotonic_ns() + 10000000ULL;
        uint64_t sequence = 0;
        while (!stop_.load()) {
            const uint64_t next = start + sequence * 1000000000ULL / hz_;
            if (const int error = sleep_until(next)) { failure_.store(error); break; }
            if (stop_.load()) break;
            // Fail on excessive delay rather than emit catch-up or short pulses.
            const uint64_t now = monotonic_ns();
            if (now > next + 2000000ULL) { failure_.store(ETIMEDOUT); break; }
            if (set('1')) { failure_.store(errno); break; }
            const int error = sleep_until(monotonic_ns() + 1000000ULL);
            if (set('0')) { failure_.store(errno); break; }
            if (error) { failure_.store(error); break; }
            ++pulses_;
            ++sequence;
        }
        set('0');
    }

    void close() {
        stop();
        if (fd_ >= 0) {
            set('0');
            ::close(fd_);
            fd_ = -1;
        }
        if (exported_) {
            write_text("/sys/class/gpio/unexport", std::to_string(gpio_));
            exported_ = false;
        }
    }

    static constexpr unsigned gpio_ = 417;
    unsigned hz_;
    int fd_ = -1;
    bool exported_ = false;
    std::atomic<bool> stop_{false};
    std::atomic<int> failure_{0};
    std::atomic<uint64_t> pulses_{0};
    std::thread worker_;
};

class MosaicSink final : public orbvi::nv12::Sink {
public:
    explicit MosaicSink(const orbvi::mosaic::Options &options)
        : options_(options), server_(options.port, options.url, orbvi::camera::Codec::H264),
          encoder_(orbvi::mosaic::kOutputWidth, orbvi::mosaic::kOutputHeight,
                   encoder_options(options), [this](const void *data, size_t size, uint64_t timestamp) {
                       return server_.push(data, size, timestamp);
                   }) {
        if (!options.serial_compose) composer_ = std::make_unique<orbvi::mosaic::ParallelComposer>();
        for (unsigned i = 0; i < buffers_.size(); ++i) {
            buffers_[i].pixels.resize(orbvi::mosaic::kOutputBytes);
            free_.push_back(i);
        }
    }

    int open() override {
        if (server_.open() || encoder_.open()) return -1;
        started_ = Clock::now();
        last_report_ = started_;
        worker_ = std::thread(&MosaicSink::encode_loop, this);
        return 0;
    }

    int write(unsigned, const orbvi::nv12::FrameSet &set) override {
        if (failed_.load()) return -1;
        unsigned slot;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            // Avoid rotating a group that will be superseded before the encoder
            // becomes available. Always admit a new group when the queue is empty.
            const auto estimate = last_encode_ns_.load();
            if (!pending_.empty() && estimate &&
                std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - last_compose_).count() <
                    static_cast<int64_t>(estimate / 2)) {
                ++dropped_before_compose_;
                return 0;
            }
            // One pending frame plus the in-flight frame: latest-frame policy.
            if (!pending_.empty()) {
                slot = pending_.front();
                pending_.pop_front();
                ++dropped_;
            } else if (!free_.empty()) {
                slot = free_.front();
                free_.pop_front();
            } else {
                std::fputs("mosaic: no free output buffer\n", stderr);
                return -1;
            }
        }
        std::array<orbvi::mosaic::Nv12Frame, orbvi::mosaic::kCameraCount> views{};
        for (unsigned camera = 0; camera < views.size(); ++camera) {
            const auto &frame = set.frames[camera];
            views[camera] = {static_cast<const uint8_t *>(frame.y),
                             static_cast<const uint8_t *>(frame.uv), frame.y_bytes,
                             frame.uv_bytes, frame.width, frame.height, frame.stride, frame.vstride};
        }
        const auto began = Clock::now();
        const bool valid = composer_
            ? composer_->compose(views, buffers_[slot].pixels.data(), buffers_[slot].pixels.size())
            : orbvi::mosaic::compose(views, buffers_[slot].pixels.data(), buffers_[slot].pixels.size());
        if (!valid) {
            std::fputs("mosaic: invalid NV12 source or output buffer\n", stderr);
            return -1;
        }
        compose_ns_ += elapsed_ns(began);
        ++composed_;
        last_compose_ = Clock::now();
        buffers_[slot].timestamp_ns = set.timestamp_ns;
        buffers_[slot].skew_ns = set.max_skew_ns;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_.push_back(slot);
        }
        ready_.notify_one();
        return failed_.load() ? -1 : 0;
    }

    int close() override {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closing_ = true;
        }
        ready_.notify_one();
        if (worker_.joinable()) worker_.join();
        int result = failed_.load() ? -1 : 0;
        if (encoder_.close()) result = -1;
        if (server_.close()) result = -1;
        if (options_.diagnostics && composed_.load()) report(true);
        return result;
    }

private:
    struct Buffer {
        std::vector<uint8_t> pixels;
        uint64_t timestamp_ns = 0;
        uint64_t skew_ns = 0;
    };
    static orbvi::camera::Options encoder_options(const orbvi::mosaic::Options &options) {
        orbvi::camera::Options output;
        output.fps = options.fps;
        output.bitrate_kbps = options.bitrate_kbps;
        output.frame_timeout_ms = options.frame_timeout_ms;
        output.rotate = 0; // Rotation is applied to each NV12 tile before encoding.
        return output;
    }
    void encode_loop() {
        try {
            for (;;) {
                unsigned slot;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    ready_.wait(lock, [this] { return closing_ || !pending_.empty(); });
                    if (pending_.empty()) break;
                    slot = pending_.front();
                    pending_.pop_front();
                }
                auto &buffer = buffers_[slot];
                const orbvi::camera::Nv12View output{
                    buffer.pixels.data(), buffer.pixels.data() + size_t(orbvi::mosaic::kOutputWidth) * orbvi::mosaic::kOutputHeight,
                    orbvi::mosaic::kOutputWidth, orbvi::mosaic::kOutputHeight,
                    orbvi::mosaic::kOutputWidth, orbvi::mosaic::kOutputHeight};
                const auto began = Clock::now();
                const int result = encoder_.encode(output, buffer.timestamp_ns);
                const auto encode_time = elapsed_ns(began);
                encode_ns_ += encode_time;
                last_encode_ns_ = encode_time;
                if (result) {
                    std::fputs("mosaic: H.264 encode or RTSP push failed\n", stderr);
                    failed_ = true;
                    break;
                }
                last_skew_ns_ = buffer.skew_ns;
                ++sent_;
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    free_.push_back(slot);
                }
                if (options_.diagnostics && elapsed_ns(last_report_) >= 2000000000ULL)
                    report(false);
            }
        } catch (...) {
            std::fputs("mosaic: encoder worker exception\n", stderr);
            failed_ = true;
        }
    }
    void report(bool final) {
        const auto now = Clock::now();
        const uint64_t elapsed = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_report_).count());
        const uint64_t sent = sent_.load();
        const double fps = elapsed ? double(sent - last_sent_) * 1e9 / double(elapsed) : 0.0;
        const uint64_t composed = composed_.load();
        std::printf("mosaic: frames=%llu actual_fps=%.1f composed=%llu dropped_queue=%llu dropped_precompose=%llu "
                    "compose_avg_ms=%.2f encode_push_avg_ms=%.2f skew_ns=%llu%s\n",
                    static_cast<unsigned long long>(sent), fps,
                    static_cast<unsigned long long>(composed),
                    static_cast<unsigned long long>(dropped_.load()),
                    static_cast<unsigned long long>(dropped_before_compose_.load()),
                    composed ? double(compose_ns_.load()) / composed / 1e6 : 0.0,
                    sent ? double(encode_ns_.load()) / sent / 1e6 : 0.0,
                    static_cast<unsigned long long>(last_skew_ns_.load()), final ? " final" : "");
        std::fflush(stdout);
        last_report_ = now;
        last_sent_ = sent;
    }
    orbvi::mosaic::Options options_;
    std::array<Buffer, 3> buffers_;
    std::unique_ptr<orbvi::mosaic::ParallelComposer> composer_;
    orbvi::camera::RtspServer server_;
    orbvi::camera::Encoder encoder_;
    std::mutex mutex_;
    std::condition_variable ready_;
    std::deque<unsigned> free_;
    std::deque<unsigned> pending_;
    std::thread worker_;
    bool closing_ = false;
    std::atomic<bool> failed_{false};
    std::atomic<uint64_t> dropped_{0}, dropped_before_compose_{0}, composed_{0}, sent_{0};
    std::atomic<uint64_t> compose_ns_{0}, encode_ns_{0}, last_encode_ns_{0}, last_skew_ns_{0};
    Clock::time_point last_compose_{};
    Clock::time_point started_{}, last_report_{};
    uint64_t last_sent_ = 0;
};
}

int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::printf("mosaic_rtsp_demo %s\n", orbvi::kDemoVersion);
        return 0;
    }
    orbvi::mosaic::Options options;
    const int parsed = orbvi::mosaic::parse_options(argc, argv, options);
    if (parsed) { orbvi::mosaic::print_usage(); return parsed == 1 ? 0 : 2; }
    std::ifstream model_input("/proc/device-tree/model", std::ios::binary);
    std::string model((std::istreambuf_iterator<char>(model_input)), {});
    while (!model.empty() && model.back() == '\0') model.pop_back();
    if (model != "HS ORBVI 4P V1.0") {
        std::fputs("mosaic: expected HS ORBVI 4P V1.0 device tree\n", stderr);
        return 2;
    }
    struct sigaction action{};
    action.sa_handler = stop;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);
    if (hb_mem_module_open()) { std::fputs("mosaic: hb_mem_module_open failed\n", stderr); return 1; }
    GpioTrigger trigger(options.fps);
    if (options.external && trigger.open_gpio()) {
        std::perror("mosaic: GPIO trigger");
        hb_mem_module_close();
        return 1;
    }
    std::array<std::unique_ptr<orbvi::nv12::X5Camera>, orbvi::mosaic::kCameraCount> cameras;
    std::array<orbvi::nv12::Source *, orbvi::mosaic::kCameraCount> sources{};
    for (unsigned camera = 0; camera < sources.size(); ++camera) {
        cameras[camera] = std::make_unique<orbvi::nv12::X5Camera>(
            camera, options.fps, orbvi::mosaic::kInputWidth, orbvi::mosaic::kInputHeight,
            options.external, options.vts, options.trigger_readout);
        sources[camera] = cameras[camera].get();
    }
    MosaicSink sink(options);
    orbvi::nv12::Session session(sources, sink, stopped, std::numeric_limits<unsigned>::max(),
                                 options.max_skew_ns, options.frame_timeout_ms);
    const int status = session.run(
        [&] { return options.external ? trigger.start() : 0; },
        [&] {
            trigger.stop();
            if (options.external && trigger.failure()) {
                std::fprintf(stderr, "mosaic: GPIO trigger failed errno=%d after %llu pulses\n",
                             trigger.failure(), static_cast<unsigned long long>(trigger.pulses()));
                return -1;
            }
            if (options.external)
                std::fprintf(stderr, "mosaic: GPIO417 trigger stopped after %llu pulses\n",
                             static_cast<unsigned long long>(trigger.pulses()));
            return 0;
        });
    const int memory_status = hb_mem_module_close();
    if (memory_status) std::fputs("mosaic: hb_mem_module_close failed\n", stderr);
    return memory_status ? 1 : (status == 130 && stopped ? 0 : status);
}
