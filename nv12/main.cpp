#include "frame_set.hpp"
#include "options.hpp"
#include "sink.hpp"
#include "x5_camera.hpp"
#include "hb_mem_mgr.h"
#include <array>
#include <climits>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sys/stat.h>

namespace {
volatile sig_atomic_t interrupted = 0;
void on_signal(int) { interrupted = 1; }
}

int main(int argc, char **argv) {
    using namespace orbvi::nv12;
    Options options;
    int parsed = parse_options(argc, argv, options);
    if (parsed) { print_usage(); return parsed == 1 ? 0 : 2; }
    if (options.output.size() > PATH_MAX - 100) {
        std::fputs("output path too long\n", stderr);
        return 2;
    }
    FILE *model = fopen("/proc/device-tree/model", "rb");
    char board[80]{};
    if (model) {
        size_t length = fread(board, 1, sizeof(board) - 1, model);
        if (ferror(model)) length = 0;
        board[length] = 0;
        fclose(model);
    }
    if (std::strcmp(board, "HS ORBVI 4P V1.0")) {
        std::fputs("expected HS ORBVI 4P V1.0 device tree\n", stderr);
        return 2;
    }
    if (mkdir(options.output.c_str(), 0755)) {
        perror(options.output.c_str());
        return 1;
    }
    struct sigaction signals{};
    signals.sa_handler = on_signal;
    sigemptyset(&signals.sa_mask);
    sigaction(SIGINT, &signals, nullptr);
    sigaction(SIGTERM, &signals, nullptr);
    if (hb_mem_module_open()) {
        std::fputs("failed to open memory module\n", stderr);
        return 1;
    }
    int result = 1;
    try {
        std::array<std::unique_ptr<X5Camera>, 4> cameras;
        std::array<Source *, 4> sources{};
        for (unsigned port = 0; port < 4; ++port) {
            cameras[port] = std::make_unique<X5Camera>(port);
            sources[port] = cameras[port].get();
        }
        FileSink sink(options.output, options.discard);
        Session session(sources, sink, interrupted, options.groups,
                        options.max_skew_ns, options.timeout_ms);
        result = session.run();
    } catch (const std::exception &error) {
        std::fprintf(stderr, "NV12 application: %s\n", error.what());
    }
    if (hb_mem_module_close()) {
        std::fputs("failed to close memory module\n", stderr);
        return 1;
    }
    return result;
}
