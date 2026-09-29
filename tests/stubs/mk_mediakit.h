#pragma once
// Test-only subset of the BSP mk_api declarations. Not used for board builds.
// Keeping these local lets the fake recorder regression run without vendor binaries.
#include <cstddef>
#include <cstdint>

extern "C" {
struct mk_media_t;
struct mk_ini_t;
struct mk_track_t;
struct mk_frame_t;
using mk_media = mk_media_t *;
using mk_ini = mk_ini_t *;
using mk_track = mk_track_t *;
using mk_frame = mk_frame_t *;
using on_mk_frame_data_release = void (*)(void *, char *);
struct codec_args { int dummy; };
struct mk_config { int log_level; int log_mask; int thread_num; };
constexpr int LOG_CONSOLE = 1;
extern const int MKCodecH264;
extern const int MKCodecH265;
void mk_env_init(const mk_config *);
uint16_t mk_rtsp_server_start(uint16_t, int);
void mk_stop_all_server();
mk_ini mk_ini_create();
void mk_ini_set_option_int(mk_ini, const char *, int);
void mk_ini_release(mk_ini);
mk_media mk_media_create2(const char *, const char *, const char *, float, mk_ini);
void mk_media_release(mk_media);
mk_track mk_track_create(int, codec_args *);
void mk_track_unref(mk_track);
void mk_media_init_track(mk_media, mk_track);
void mk_media_init_complete(mk_media);
mk_frame mk_frame_create(int, uint64_t, uint64_t, const char *, size_t, on_mk_frame_data_release, void *);
void mk_frame_unref(mk_frame);
int mk_media_input_frame(mk_media, mk_frame);
int mk_recorder_start(int, const char *, const char *, const char *, const char *, size_t);
int mk_recorder_stop(int, const char *, const char *, const char *);
}
