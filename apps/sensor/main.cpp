#include "sensor_options.hpp"
#include "bag_session.hpp"
#include "../version.hpp"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
volatile sig_atomic_t stop_requested = 0;
void stop(int) { stop_requested = 1; }

pid_t spawn(const std::filesystem::path &program, const std::vector<std::string> &arguments) {
    const pid_t child = fork();
    if (child != 0) return child;
    std::vector<char *> argv;
    argv.reserve(arguments.size() + 2);
    std::string executable = program.string();
    argv.push_back(executable.data());
    for (const auto &argument : arguments) argv.push_back(const_cast<char *>(argument.c_str()));
    argv.push_back(nullptr);
    execv(executable.c_str(), argv.data());
    std::perror(executable.c_str());
    _exit(127);
}

void add(std::vector<std::string> &args, const char *name, const std::string &value) {
    args.emplace_back(name); args.push_back(value);
}
void add(std::vector<std::string> &args, const char *name, unsigned value) {
    add(args, name, std::to_string(value));
}
}

int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::printf("sensor_demo %s\n", orbvi::kDemoVersion);
        return 0;
    }
    std::error_code error;
    const auto executable = std::filesystem::canonical("/proc/self/exe", error);
    if (error) { std::fprintf(stderr, "sensor_demo: %s\n", error.message().c_str()); return 1; }
    const auto root = executable.parent_path().parent_path();
    const char *configured_root = std::getenv("DEMO_DIR");
    const auto config = (configured_root && *configured_root ? std::filesystem::path(configured_root) : root)
                      / "config/sensor_config.yaml";
    orbvi::sensor::Options options;
    const int parsed = orbvi::sensor::parse_options(argc, argv, config.string(), options);
    if (parsed) { orbvi::sensor::print_usage(); return parsed == 1 ? 0 : 2; }
    std::filesystem::path bag_spool;
    if (!options.record_bag_path.empty()) {
        bag_spool = options.record_bag_path + ".spool";
        if (std::filesystem::exists(bag_spool)) {
            std::fputs("sensor_demo: rosbag spool directory already exists\n", stderr); return 1;
        }
        std::filesystem::create_directories(bag_spool, error);
        if (error) { std::fprintf(stderr, "sensor_demo: %s\n", error.message().c_str()); return 1; }
    }
    if (!options.record_mp4_directory.empty()) {
        std::filesystem::create_directories(options.record_mp4_directory, error);
        if (error) { std::fprintf(stderr, "sensor_demo: %s\n", error.message().c_str()); return 1; }
    }

    std::vector<std::string> camera;
    add(camera, "--camera-mask", options.camera.camera_mask);
    add(camera, "--width", options.camera.width); add(camera, "--height", options.camera.height);
    add(camera, "--fps", options.camera.fps); add(camera, "--rotate", options.camera.rotate);
    add(camera, "--bps", options.camera.bitrate_kbps);
    add(camera, "--codec", options.camera.codec == orbvi::camera::Codec::H264 ? "h264" : "h265");
    add(camera, "--url", options.camera.url); add(camera, "--rtsp-base-port", options.camera.rtsp_base_port);
    add(camera, "--frame-timeout-ms", options.camera.frame_timeout_ms);
    add(camera, "--max-skew-ns", std::to_string(options.camera.max_skew_ns));
    add(camera, "--diag-interval-ms", options.camera.diag_interval_ms);
    add(camera, "--trigger-mode", options.camera.trigger_mode);
    if (options.camera.diagnostics) camera.emplace_back("--diagnostics");
    if (!options.record_bag_path.empty()) {
        add(camera, "--record-h264-dir", bag_spool.string());
        add(camera, "--record-frame-skip", options.record_frame_skip);
    }
    if (!options.record_mp4_directory.empty())
        add(camera, "--record-mp4-dir", options.record_mp4_directory);

    std::vector<std::string> imu;
    add(imu, "--sample-rate-hz", options.imu_sample_rate_hz);
    add(imu, "--print-rate-hz", options.imu_print_rate_hz);
    if (options.imu_print_metrics) imu.emplace_back("--print-metrics");
    if (!options.imu_udp_dest.empty()) add(imu, "--udp-dest", options.imu_udp_dest);
    if (!options.record_mp4_directory.empty())
        add(imu, "--csv", (std::filesystem::path(options.record_mp4_directory) / "imu.csv").string());
    if (!options.record_bag_path.empty())
        add(imu, "--csv", (bag_spool / "imu.csv").string());

    struct sigaction action{}; action.sa_handler = stop; sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr); sigaction(SIGTERM, &action, nullptr);
    const pid_t imu_pid = spawn(root / "imu_reader_demo", imu);
    if (imu_pid < 0) { std::perror("fork imu_reader_demo"); return 1; }
    const pid_t camera_pid = spawn(root / "cam_demo", camera);
    if (camera_pid < 0) { std::perror("fork cam_demo"); kill(imu_pid, SIGTERM); waitpid(imu_pid, nullptr, 0); return 1; }

    int result = 0;
    bool imu_running = true, camera_running = true;
    while (imu_running || camera_running) {
        if (stop_requested) {
            if (imu_running) kill(imu_pid, SIGTERM);
            if (camera_running) kill(camera_pid, SIGTERM);
            stop_requested = 0;
        }
        int status = 0;
        const pid_t child = waitpid(-1, &status, 0);
        if (child < 0) {
            if (errno == EINTR) continue;
            result = 1; break;
        }
        if (child == imu_pid) imu_running = false;
        if (child == camera_pid) camera_running = false;
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) result = 1;
        if (imu_running) kill(imu_pid, SIGTERM);
        if (camera_running) kill(camera_pid, SIGTERM);
    }
    if (!result && !options.record_bag_path.empty()) {
        result = orbvi::sensor::write_rosbag(options.record_bag_path, bag_spool.string(),
                                             options.camera.camera_mask,
                                             (bag_spool / "imu.csv").string());
        if (!result) {
            std::filesystem::remove_all(bag_spool, error);
            if (error) { std::fprintf(stderr, "sensor_demo: remove spool: %s\n", error.message().c_str()); result = 1; }
        }
    }
    return result;
}
