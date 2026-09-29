#include "camera_options.hpp"
#include <cassert>

using namespace orbvi::camera;
int main() {
    Options options;
    const char *basic[]{"cam_demo"};
    assert(parse_options(1, basic, options) == 0);
    assert(options.camera_id == -1 && options.codec == Codec::H264);
    assert(options.bitrate_kbps == 4000 && options.url == "/live/PRR");
    assert(options.width == 1280 && options.height == 1088);
    assert(options.channels == 4 && options.camera_mask == 0xf && options.rtsp_base_port == 554);
    const char *single[]{"cam_demo", "--camera-id", "2", "--codec", "h265", "--bps", "8000",
                         "--url", "/live/PRR", "--frame-timeout-ms", "1500", "--max-skew-ns", "2000000",
                         "--width", "720", "--height", "480", "--rtsp-base-port", "8554"};
    assert(parse_options(19, single, options) == 0);
    assert(options.camera_id == 2 && options.codec == Codec::H265 && options.bitrate_kbps == 8000);
    assert(options.frame_timeout_ms == 1500 && options.max_skew_ns == 2000000);
    assert(options.width == 720 && options.height == 480 && options.rtsp_base_port == 8554);
    const char *mask[]{"cam_demo", "--camera-mask", "5"};
    assert(parse_options(3, mask, options) == 0 && options.camera_mask == 5);
    const char *trigger[]{"cam_demo", "--trigger-mode", "none"};
    assert(parse_options(3, trigger, options) == 0 && options.trigger_mode == "none");
    const char *bad_size[]{"cam_demo", "--width", "800", "--height", "600"};
    assert(parse_options(5, bad_size, options) == 2);
    const char *bad[]{"cam_demo", "--fps", "31"};
    assert(parse_options(3, bad, options) == 2);
    const char *bad_url[]{"cam_demo", "--url", "/PRR"};
    assert(parse_options(3, bad_url, options) == 2);
    const char *rotate[]{"cam_demo", "--rotate", "180", "--fps", "60"};
    assert(parse_options(5, rotate, options) == 2);
}
