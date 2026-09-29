#include "rtsp_server.hpp"
#include "mk_mediakit.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <thread>

namespace orbvi::camera {
struct RtspServer::State {
    unsigned port;
    std::string path;
    std::string app;
    std::string stream;
    Codec codec;
    std::string record_directory;
    bool record_started = false;
    bool record_failed = false;
    mk_media media = nullptr;
    bool initialized = false;
};

RtspServer::RtspServer(unsigned port, std::string path, Codec codec, std::string record_directory)
    : state_(new State{port, std::move(path), {}, {}, codec, std::move(record_directory)}) {}
RtspServer::~RtspServer() { close(); }

int RtspServer::open() {
    auto &state = *state_;
    if (state.initialized || state.path.size() < 2 || state.path[0] != '/') return -1;
    const std::string media_path = state.path.substr(1);
    const size_t separator = media_path.find('/');
    if (separator == std::string::npos || separator == 0 || separator + 1 == media_path.size()) return -1;
    state.app = media_path.substr(0, separator);
    state.stream = media_path.substr(separator + 1);
    mk_config config{};
    config.log_level = 2;
    config.log_mask = LOG_CONSOLE;
    config.thread_num = 1;
    mk_env_init(&config);
    state.initialized = true;
    if (!mk_rtsp_server_start(static_cast<uint16_t>(state.port), 0)) return -1;
    mk_ini ini = mk_ini_create();
    mk_ini_set_option_int(ini, "modify_stamp", 0);
    mk_ini_set_option_int(ini, "enable_hls", 0);
    mk_ini_set_option_int(ini, "enable_mp4", 0);
    state.media = mk_media_create2("__defaultVhost__", state.app.c_str(), state.stream.c_str(), 0, ini);
    mk_ini_release(ini);
    if (!state.media) return -1;
    codec_args arguments{};
    mk_track track = mk_track_create(state.codec == Codec::H264 ? MKCodecH264 : MKCodecH265, &arguments);
    if (!track) return -1;
    mk_media_init_track(state.media, track);
    mk_media_init_complete(state.media);
    mk_track_unref(track);
    std::printf("RTSP ready: port=%u path=%s\n", state.port, state.path.c_str());
    return 0;
}

int RtspServer::push(const void *data, size_t size, uint64_t timestamp_ns) {
    auto &state = *state_;
    if (!state.media || !data || !size || state.record_failed) return -1;
    if (!state.record_directory.empty() && state.codec != Codec::H264) {
        std::fprintf(stderr, "RTSP MP4 recording requires H.264: port=%u path=%s\n",
                     state.port, state.path.c_str());
        state.record_failed = true;
        return -1;
    }
    mk_frame frame = mk_frame_create(state.codec == Codec::H264 ? MKCodecH264 : MKCodecH265,
                                     timestamp_ns / 1000000, timestamp_ns / 1000000,
                                     static_cast<const char *>(data), size, nullptr, nullptr);
    if (!frame) return -1;
    int result = mk_media_input_frame(state.media, frame) == 1 ? 0 : -1;
    mk_frame_unref(frame);
    if (!result && !state.record_directory.empty() && !state.record_started) {
        // Single-track media registration can complete after the first encoded frame.
        // Never silently continue a requested recording without a recorder: wait a
        // bounded time for registration, then fail closed if it cannot start.
        constexpr auto kRegistrationTimeout = std::chrono::milliseconds(500);
        constexpr auto kRetryInterval = std::chrono::milliseconds(10);
        const auto deadline = std::chrono::steady_clock::now() + kRegistrationTimeout;
        unsigned attempts = 0;
        do {
            ++attempts;
            if (mk_recorder_start(1, "__defaultVhost__", state.app.c_str(), state.stream.c_str(),
                                  state.record_directory.c_str(), 0)) {
                state.record_started = true;
                if (attempts > 1) {
                    std::fprintf(stderr, "RTSP MP4 recorder started after %u attempts: port=%u path=%s\n",
                                 attempts, state.port, state.path.c_str());
                }
                break;
            }
            const auto now = std::chrono::steady_clock::now();
            if (now >= deadline) break;
            std::this_thread::sleep_for(std::min(std::chrono::steady_clock::duration(kRetryInterval),
                                                 deadline - now));
        } while (true);
        if (!state.record_started) {
            std::fprintf(stderr, "RTSP MP4 recorder failed after %u attempts (500ms limit): port=%u path=%s\n",
                         attempts, state.port, state.path.c_str());
            state.record_failed = true;
            return -1;
        }
    }
    return result;
}

int RtspServer::close() {
    auto &state = *state_;
    int result = 0;
    if (state.record_started) {
        if (!mk_recorder_stop(1, "__defaultVhost__", state.app.c_str(), state.stream.c_str())) result = -1;
        state.record_started = false;
    }
    state.record_failed = false;
    if (state.media) { mk_media_release(state.media); state.media = nullptr; }
    if (state.initialized) { mk_stop_all_server(); state.initialized = false; }
    return result;
}
}
