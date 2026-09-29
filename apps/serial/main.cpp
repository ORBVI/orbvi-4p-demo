#include "serial_port.hpp"
#include "../version.hpp"
#include <cstdio>
#include <string>
int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::printf("serial_port_demo %s\n", orbvi::kDemoVersion);
        return 0;
    }
    orbvi::serial::Options options;
    int parsed = orbvi::serial::parse_options(argc, argv, options);
    if (parsed) { orbvi::serial::print_usage(); return parsed == 1 ? 0 : 2; }
    return orbvi::serial::run(options, 1);
}
