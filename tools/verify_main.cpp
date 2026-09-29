#include "workflows.hpp"
#include <iostream>
#include <cstdlib>
using namespace orbvi::validation;
static void usage() {
    std::cout << "ORBVI 4P C++ verification\n"
        "  orbvi-verify sync RUNNER [--raw]\n"
        "  orbvi-verify board RUNNER\n"
        "  orbvi-verify shutdown RUNNER INJECTION.so\n"
        "  orbvi-verify backpressure [--output FILE]\n"
        "RUNNER: absolute/relative path to orbvi-4p-demo or run.sh\n"
        "sync environment: ORBVI_SYNC_SECONDS=1..120 (default 3),\n"
        "                  ORBVI_SYNC_OUTPUT_BASE=/userdata\n"
        "Board functions require an idle ORBVI 4P; sync owns GPIO417.\n";
}
int main(int argc,char **argv) {
    if(argc==2 && (std::string(argv[1])=="--help" || std::string(argv[1])=="-h")) {usage(); return 0;}
    if(argc<2) {usage(); return 2;}
    std::string command=argv[1];
    bool valid=(command=="sync" && (argc==3 || (argc==4 && std::string(argv[3])=="--raw"))) ||
               (command=="board" && argc==3) || (command=="shutdown" && argc==4) ||
               (command=="backpressure" && (argc==2 || (argc==4 && std::string(argv[2])=="--output" && *argv[3])));
    if(!valid) {usage(); return 2;}
    unsigned seconds=3;
    if(command=="sync") {
        const char *setting=getenv("ORBVI_SYNC_SECONDS");
        if(setting) {
            std::string value=setting;
            if(value.empty() || value.size()>3 || value.find_first_not_of("0123456789")!=std::string::npos ||
               (seconds=unsigned(std::stoul(value)))<1 || seconds>120) {
                std::cerr << "ORBVI_SYNC_SECONDS must be in 1..120\n"; return 2;
            }
        }
    }
    struct sigaction action{}; action.sa_handler=[](int sig){interrupted=sig;}; sigemptyset(&action.sa_mask);
    sigaction(SIGINT,&action,nullptr); sigaction(SIGTERM,&action,nullptr);
    try {
        if(command=="backpressure") return run_backpressure(argc==4 ? fs::path(argv[3]) : fs::path("/tmp")/("orbvi-imu-backpressure-"+std::to_string(now_ns())+".json"));
        auto runner=fs::canonical(argv[2]).string();
        if(command=="board") return run_board(runner);
        if(command=="shutdown") return run_shutdown(runner,fs::canonical(argv[3]).string());
        const char *base=getenv("ORBVI_SYNC_OUTPUT_BASE");
        return run_sync(runner,argc==4,seconds,base ? base : "/userdata");
    } catch(const std::exception &error) {std::cerr << "FAIL: " << error.what() << '\n'; return 1;}
}
