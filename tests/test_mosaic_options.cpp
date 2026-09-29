#include "mosaic_options.hpp"
#include <cassert>
using namespace orbvi::mosaic;
int main() {
    Options options;
    const char *basic[]{"mosaic_rtsp_demo"};
    assert(parse_options(1, basic, options) == 0);
    assert(options.port == 558 && options.url == "/live/PRR" && options.fps == 30);
    assert(options.max_skew_ns == 40000000 && options.bitrate_kbps == 8000 && !options.serial_compose);
    const char *custom[]{"mosaic_rtsp_demo", "--fps", "50", "--bps", "12000", "--port", "8558",
                         "--url", "/test/mosaic", "--max-skew-ns", "20000000", "--diagnostics"};
    assert(parse_options(int(sizeof(custom) / sizeof(custom[0])), custom, options) == 0 && options.fps == 50 && options.port == 8558);
    assert(options.bitrate_kbps == 12000 && options.url == "/test/mosaic" && options.diagnostics);
    const char *serial[]{"mosaic_rtsp_demo", "--serial-compose"};
    assert(parse_options(2, serial, options) == 0 && options.serial_compose);
    const char *bad_fps[]{"mosaic_rtsp_demo", "--fps", "31"};
    assert(parse_options(3, bad_fps, options) == 2);
    const char *bad_path[]{"mosaic_rtsp_demo", "--url", "/PRR"};
    assert(parse_options(3, bad_path, options) == 2);
    const char *bad_port[]{"mosaic_rtsp_demo", "--port", "65536"};
    assert(parse_options(3, bad_port, options) == 2);
    const char *help[]{"mosaic_rtsp_demo", "--help"};
    assert(parse_options(2, help, options) == 1);
}
