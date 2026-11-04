#include "camera_options.hpp"
#include "encoder.hpp"
#include "process_group.hpp"
#include "rtsp_server.hpp"
#include "x5_camera.hpp"
#include "../version.hpp"
#include "hb_mem_mgr.h"
#include <csignal>
#include <cerrno>
#include <cstdio>
#include <algorithm>
#include <string>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <limits>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace {
volatile sig_atomic_t stopped = 0;
void stop(int) { stopped = 1; }
int pulse_trigger(int fd) {
    if (fd < 0) return 0;
    const char high = '1', low = '0';
    if (lseek(fd, 0, SEEK_SET) < 0 || write(fd, &high, 1) != 1) return -1;
    timespec delay{0, 1000000};
    while (nanosleep(&delay, &delay) && errno == EINTR) {}
    return lseek(fd, 0, SEEK_SET) < 0 || write(fd, &low, 1) != 1 ? -1 : 0;
}
}

int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::printf("cam_demo %s\n", orbvi::kDemoVersion);
        return 0;
    }
    using namespace orbvi;
    camera::Options options;
    int parsed = camera::parse_options(argc, argv, options);
    if (parsed) { camera::print_usage(); return parsed == 1 ? 0 : 2; }
    if (options.camera_id < 0) {
        std::fputs("internal cam_demo requires --camera-id; use the packaged launcher for four cameras\n", stderr);
        return 2;
    }
    struct sigaction action{}; action.sa_handler = stop; sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr); sigaction(SIGTERM, &action, nullptr);
    if (hb_mem_module_open()) return 1;
    int result = 1;
    int group_fd = -1;
    camera::SharedFrameGroup *group = nullptr;
    if (!options.group_shm.empty()) {
        group_fd = shm_open(options.group_shm.c_str(), O_RDWR, 0);
        if (group_fd < 0) { std::perror("shm_open frame group"); hb_mem_module_close(); return 1; }
        void *mapping = mmap(nullptr, sizeof(*group), PROT_READ | PROT_WRITE, MAP_SHARED, group_fd, 0);
        if (mapping == MAP_FAILED) { std::perror("mmap frame group"); close(group_fd); hb_mem_module_close(); return 1; }
        group = static_cast<camera::SharedFrameGroup *>(mapping);
    }
    if (!(options.camera_mask & (1u << static_cast<unsigned>(options.camera_id)))) {
        hb_mem_module_close();
        return 0;
    }
    const unsigned source_width = options.height;
    const unsigned source_height = options.width;
    std::string camera_record_directory;
    std::ofstream timestamp_index;
    std::ofstream h264_spool;
    if (!options.record_h264_directory.empty())
        options.record_h264_path = (std::filesystem::path(options.record_h264_directory) /
            ("camera" + std::to_string(options.camera_id) + ".h264spool")).string();
    if (!options.record_h264_path.empty()) {
        h264_spool.open(options.record_h264_path, std::ios::binary | std::ios::trunc);
        if (!h264_spool) { std::fputs("cannot create H.264 spool\n", stderr); hb_mem_module_close(); return 1; }
    }
    if (!options.record_mp4_directory.empty()) {
        const std::filesystem::path base(options.record_mp4_directory);
        camera_record_directory = (base / ("camera" + std::to_string(options.camera_id))).string();
        std::error_code error;
        std::filesystem::create_directories(camera_record_directory, error);
        if (error) { std::fprintf(stderr, "cannot create MP4 directory: %s\n", error.message().c_str()); hb_mem_module_close(); return 1; }
        timestamp_index.open(base / ("camera" + std::to_string(options.camera_id) + "_timestamps.csv"));
        if (!timestamp_index) { std::fputs("cannot create MP4 timestamp index\n", stderr); hb_mem_module_close(); return 1; }
        timestamp_index << "timestamp_ns,encoded_bytes\n";
    }
    nv12::X5Camera source(static_cast<unsigned>(options.camera_id), options.fps,
                          source_width, source_height, options.external_sync, 0, false, options.read_exposure);
    camera::RtspServer server(options.rtsp_base_port + static_cast<unsigned>(options.camera_id),
                              options.url, options.codec, camera_record_directory);
    camera::Options encoder_options = options;
    encoder_options.rotate = (90 + options.rotate) % 360; // Sensor is mounted portrait; API rotation is relative to standard view.
    uint64_t spool_sequence = 0;
    camera::FrameMetadata pending_metadata{};
    bool metadata_pending = false;
    camera::Encoder encoder(source_width, source_height, encoder_options,
        [&](const void *data, size_t size, uint64_t timestamp) {
            if (options.external_sync && (!metadata_pending || !timestamp || timestamp != pending_metadata.timestamp_ns)) {
                std::fprintf(stderr, "camera%d: encoder PTS does not match the captured frame\n", options.camera_id);
                return -1;
            }
            const int status = server.push(data, size, timestamp, options.external_sync ? &pending_metadata : nullptr);
            metadata_pending = false;
            if (status) return status;
            if (timestamp_index.is_open()) {
                timestamp_index << timestamp << ',' << size << '\n';
                if (!timestamp_index) return -1;
            }
            if (h264_spool.is_open() && (!options.record_frame_skip || spool_sequence % 2 == 0)) {
                if (size > std::numeric_limits<uint32_t>::max()) return -1;
                timespec now{}; if (clock_gettime(CLOCK_REALTIME, &now)) return -1;
                const uint64_t host_ns = static_cast<uint64_t>(now.tv_sec) * 1000000000ULL + now.tv_nsec;
                const uint32_t bytes = static_cast<uint32_t>(size);
                h264_spool.write(reinterpret_cast<const char *>(&timestamp), sizeof(timestamp));
                h264_spool.write(reinterpret_cast<const char *>(&host_ns), sizeof(host_ns));
                h264_spool.write(reinterpret_cast<const char *>(&bytes), sizeof(bytes));
                h264_spool.write(static_cast<const char *>(data), bytes);
                if (!h264_spool) return -1;
            }
            ++spool_sequence;
            return 0;
        });
    if (!source.prepare() && !server.open() && !encoder.open() &&
        (!options.external_sync || (group && camera::wait_for_start(*group, options.camera_id))) && !source.start()) {
        result = 0;
        uint64_t frames = 0, acquired_frames = 0, source_gaps = 0, group_drops = 0;
        uint32_t previous_id = 0;
        bool have_previous_id = false;
        while (!stopped) {
            nv12::Frame frame;
            int acquired = source.acquire(frame, options.frame_timeout_ms);
            if (acquired) {
                if (stopped) break; // SIGINT/SIGTERM may interrupt a pending BSP frame wait.
                std::fprintf(stderr, "camera%d: frame timeout (%d)\n", options.camera_id, acquired);
                result = 1;
                break;
            }
            ++acquired_frames;
            if (have_previous_id) {
                const uint32_t advance = frame.id - previous_id;
                if (advance > 1 && advance < 0x80000000u) source_gaps += advance - 1;
                else if (advance != 1) std::fprintf(stderr, "camera%d: frame id discontinuity %u -> %u\n",
                                                    options.camera_id, previous_id, frame.id);
            }
            previous_id = frame.id; have_previous_id = true;
            bool grouped = true;
            if (group && !options.external_sync) {
                bool fresh = true;
                while (true) {
                    bool accepted_leader = false;
                    const camera::GroupDecision decision = camera::wait_for_group(
                        *group, static_cast<unsigned>(options.camera_id), frame.timestamp_ns, fresh,
                        &accepted_leader);
                    if (accepted_leader && pulse_trigger(options.trigger_fd)) {
                        std::fputs("GPIO417 pulse failed\n", stderr); result = 1; grouped = false; break;
                    }
                    fresh = false;
                    if (decision == camera::GroupDecision::Hold) continue;
                    if (decision == camera::GroupDecision::Drop) {
                        if (source.release()) result = 1;
                        ++group_drops;
                        grouped = false; break;
                    }
                    if (decision == camera::GroupDecision::Stop) {
                        source.release(); stopped = 1; grouped = false; break;
                    }
                    break;
                }
            }
            if (!grouped) { if (result || stopped) break; continue; }
            camera::Nv12View view{static_cast<const unsigned char *>(frame.y),
                                  static_cast<const unsigned char *>(frame.uv), frame.width,
                                  frame.height, frame.stride, frame.vstride};
            if (options.external_sync) {
                pending_metadata = {static_cast<uint8_t>(options.camera_id), frame.id, frame.timestamp_ns};
                metadata_pending = true;
            }
            if (encoder.encode(view, frame.timestamp_ns)) result = 1;
            if (options.external_sync && metadata_pending) result = 1;
            if (source.release()) result = 1;
            if (result) break;
            ++frames;
            if (options.diagnostics && frames % std::max(1u, options.fps * options.diag_interval_ms / 1000) == 0)
                std::printf("camera%d: acquired=%llu encoded=%llu source_gaps=%llu group_drops=%llu frame_id=%u timestamp_ns=%llu\n", options.camera_id,
                            static_cast<unsigned long long>(acquired_frames),
                            static_cast<unsigned long long>(frames),
                            static_cast<unsigned long long>(source_gaps),
                            static_cast<unsigned long long>(group_drops), frame.id,
                            static_cast<unsigned long long>(frame.timestamp_ns));
        }
        if (options.external_sync)
            std::printf("camera%d: final acquired=%llu encoded=%llu source_gaps=%llu group_drops=%llu\n",
                        options.camera_id, static_cast<unsigned long long>(acquired_frames),
                        static_cast<unsigned long long>(frames), static_cast<unsigned long long>(source_gaps),
                        static_cast<unsigned long long>(group_drops));
    }
    if (group) camera::stop_group(*group);
    if (source.close()) result = 1;
    if (encoder.close()) result = 1;
    if (server.close()) result = 1;
    if (group) munmap(group, sizeof(*group));
    if (group_fd >= 0) close(group_fd);
    if (hb_mem_module_close()) result = 1;
    return result;
}
