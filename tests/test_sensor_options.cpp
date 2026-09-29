#include "sensor_options.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <unistd.h>

using namespace orbvi::sensor;

int main() {
    const auto path = std::filesystem::temp_directory_path() / "orbvi-sensor-config.yaml";
    {
        std::ofstream out(path);
        out << "camera:\n  width: 720\n  height: 480\n  fps: 50\n  rotate: 90\n"
               "rtsp:\n  bps: 6000\n  codec: h265\n  url: /live/test\n"
               "imu:\n  sample_rate_hz: 500\n  print_rate_hz: 25\n  print_metrics: true\n"
               "  udp_dest: 127.0.0.1:5600\n"
               "save_data:\n  save: false\n  format: mp4\n  save_path: /tmp/session\n  skip: false\n";
    }
    Options options;
    const char *args[]{"sensor_demo", "--fps", "30", "--codec", "h264",
                       "--record-mp4-dir", "/data/run"};
    assert(parse_options(7, args, path, options) == 0);
    assert(options.camera.width == 720 && options.camera.height == 480);
    assert(options.camera.fps == 30 && options.camera.codec == orbvi::camera::Codec::H264);
    assert(options.camera.bitrate_kbps == 6000 && options.camera.url == "/live/test");
    assert(options.imu_sample_rate_hz == 500 && options.imu_print_rate_hz == 25);
    assert(options.imu_print_metrics && options.record_mp4_directory == "/data/run");
    assert(options.imu_udp_dest == "127.0.0.1:5600");
    const char *override_dest[]{"sensor_demo", "--udp-dest", "192.168.1.100:5601"};
    assert(parse_options(3, override_dest, path, options) == 0);
    assert(options.imu_udp_dest == "192.168.1.100:5601");
    const char *bad_udp[]{"sensor_demo", "--udp-dest", "192.168.1.100:0"};
    assert(parse_options(3, bad_udp, path, options) == 2);
    const char *bad_host[]{"sensor_demo", "--udp-dest", "localhost:5600"};
    assert(parse_options(3, bad_host, path, options) == 2);
    const char *multicast[]{"sensor_demo", "--udp-dest", "224.0.0.1:5600"};
    assert(parse_options(3, multicast, path, options) == 2);
    const char *unspecified[]{"sensor_demo", "--udp-dest", "0.0.0.0:5600"};
    assert(parse_options(3, unspecified, path, options) == 2);
    const char *both[]{"sensor_demo", "--record-bag", "/data/a.bag",
                       "--record-mp4-dir", "/data/mp4"};
    assert(parse_options(5, both, path, options) == 2);
    const auto generated = std::filesystem::temp_directory_path() /
        ("orbvi-generated-config-" + std::to_string(getpid())) / "sensor_config.yaml";
    const char *defaults[]{"sensor_demo"};
    assert(parse_options(1, defaults, generated, options) == 0);
    assert(std::filesystem::exists(generated));
    assert(options.imu_udp_dest.empty());
    assert(std::filesystem::remove(generated));
    assert(std::filesystem::remove(generated.parent_path()));
    std::filesystem::remove(path);
}
