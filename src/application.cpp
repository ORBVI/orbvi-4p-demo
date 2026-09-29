#include "application.hpp"
#include "options.hpp"
#include "capture_session.hpp"
#include "x5_camera.hpp"
#include "file_sink.hpp"
#include "hb_mem_mgr.h"
#include <array>
#include <memory>
#include <cstdio>
#include <cstring>
#include <climits>
#include <csignal>
#include <sys/stat.h>
namespace orbvi {
static volatile sig_atomic_t interrupted;
static void on_signal(int) { interrupted = 1; }
struct MemoryModule {
    bool opened = true;
    int close() { if (!opened) return 0; opened = false; return hb_mem_module_close(); }
    ~MemoryModule() { close(); }
};
int run_application(int argc, char **argv)
{
    struct options options;
    int parsed=parse_options(argc,argv,&options);
    if (parsed) { print_usage(); return parsed>0 ? 0 : 2; }
    if (strlen(options.output)>PATH_MAX-100) { fputs("Output path too long\n",stderr); return 2; }
    FILE *model=fopen("/proc/device-tree/model","rb");
    char board[80]={0};
    if (model) {
        size_t length=fread(board,1,sizeof(board)-1,model);
        if (ferror(model)) length=0;
        board[length]=0;
        fclose(model);
    }
    if (strcmp(board,"HS ORBVI 4P V1.0")) { fputs("Expected HS ORBVI 4P V1.0 device tree\n",stderr); return 2; }
    if (mkdir(options.output,0755)) { perror(options.output); return 1; }
    struct sigaction sa{}; sa.sa_handler=on_signal;
    sigemptyset(&sa.sa_mask); sigaction(SIGINT,&sa,NULL); sigaction(SIGTERM,&sa,NULL);
    if (options.external) fputs("External mode: start trigger source before this command.\n",stderr);
    if (hb_mem_module_open()) { fputs("Failed to open memory module\n",stderr); return 1; }
    MemoryModule memory;
    int result = 1;
    try {
        std::array<std::unique_ptr<X5Camera>, 4> cameras;
        std::array<std::unique_ptr<FileSink>, 4> sinks;
        std::array<CameraSource *, 4> sources{};
        std::array<FrameSink *, 4> outputs{};
        for (unsigned i = 0; i < 4; ++i) {
            cameras[i] = std::make_unique<X5Camera>(i);
            sinks[i] = std::make_unique<FileSink>(options.output, options.discard);
            sources[i] = cameras[i].get();
            outputs[i] = sinks[i].get();
        }
        CaptureSession session(options, sources, outputs, interrupted);
        result = session.run();
    } catch (const std::exception &error) {
        fprintf(stderr, "application: %s\n", error.what());
    }
    if (memory.close()) { fputs("Failed to close memory module\n", stderr); return 1; }
    return result;
}
}
