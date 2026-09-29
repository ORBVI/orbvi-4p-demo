// Host regression with fake mk_api: verify the asynchronous registration race
// without a board, camera, or writing MP4 files.
#include "rtsp_server.hpp"
#include "mk_mediakit.h"
#include <cassert>
#include <chrono>
#include <cstdio>

using namespace orbvi::camera;
using Clock = std::chrono::steady_clock;
namespace {
int marker;
int starts = 0, stops = 0, inputs = 0;
bool never_register = false;
Clock::time_point registered_at;
template <typename T> T handle() { return reinterpret_cast<T>(&marker); }
void reset(bool fail = false) {
    starts = stops = inputs = 0;
    never_register = fail;
    registered_at = Clock::now();
}
}
extern "C" {
extern const int MKCodecH264 = 0;
extern const int MKCodecH265 = 1;
void mk_env_init(const mk_config *) {}
uint16_t mk_rtsp_server_start(uint16_t port, int) { return port; }
void mk_stop_all_server() {}
mk_ini mk_ini_create() { return handle<mk_ini>(); }
void mk_ini_set_option_int(mk_ini, const char *, int) {}
void mk_ini_release(mk_ini) {}
mk_media mk_media_create2(const char *, const char *, const char *, float, mk_ini) { return handle<mk_media>(); }
void mk_media_release(mk_media) {}
mk_track mk_track_create(int, codec_args *) { return handle<mk_track>(); }
void mk_track_unref(mk_track) {}
void mk_media_init_track(mk_media, mk_track) {}
void mk_media_init_complete(mk_media) {}
mk_frame mk_frame_create(int, uint64_t, uint64_t, const char *, size_t,
                         on_mk_frame_data_release, void *) { return handle<mk_frame>(); }
void mk_frame_unref(mk_frame) {}
int mk_media_input_frame(mk_media, mk_frame) {
    if (++inputs == 1) registered_at = Clock::now() + std::chrono::milliseconds(25);
    return 1;
}
int mk_recorder_start(int type, const char *, const char *, const char *, const char *, size_t) {
    assert(type == 1);
    ++starts;
    return !never_register && Clock::now() >= registered_at;
}
int mk_recorder_stop(int type, const char *, const char *, const char *) {
    assert(type == 1);
    ++stops;
    return 1;
}
}
int main() {
    constexpr char frame[] = "encoded";
    reset();
    {
        RtspServer server(554, "/live/PRR", Codec::H264);
        assert(server.open() == 0);
        assert(server.push(frame, sizeof(frame), 123) == 0);
        assert(server.close() == 0);
        assert(inputs == 1 && starts == 0 && stops == 0);
    }
    reset();
    {
        RtspServer server(555, "/live/PRR", Codec::H264, "/tmp/record");
        assert(server.open() == 0);
        const auto start = Clock::now();
        assert(server.push(frame, sizeof(frame), 123) == 0);
        assert(Clock::now() - start < std::chrono::milliseconds(500));
        assert(starts >= 2 && inputs == 1);
        assert(server.push(frame, sizeof(frame), 456) == 0);
        assert(stops == 0 && starts >= 2 && inputs == 2);
        const int successful_starts = starts;
        assert(server.close() == 0);
        assert(starts == successful_starts && stops == 1);
    }
    reset(true);
    {
        RtspServer server(556, "/live/PRR", Codec::H264, "/tmp/record");
        assert(server.open() == 0);
        const auto start = Clock::now();
        assert(server.push(frame, sizeof(frame), 123) == -1);
        const auto elapsed = Clock::now() - start;
        assert(elapsed >= std::chrono::milliseconds(490));
        assert(elapsed < std::chrono::seconds(2));
        assert(starts > 1 && inputs == 1);
        const int failed_starts = starts;
        assert(server.push(frame, sizeof(frame), 456) == -1);
        assert(inputs == 1 && starts == failed_starts);
        assert(server.close() == 0);
        assert(stops == 0);
    }
    reset();
    {
        RtspServer server(557, "/live/PRR", Codec::H265, "/tmp/record");
        assert(server.open() == 0);
        assert(server.push(frame, sizeof(frame), 123) == -1);
        assert(inputs == 0 && starts == 0 && stops == 0);
        assert(server.close() == 0);
    }
    std::puts("RTSP recorder race tests passed");
}
