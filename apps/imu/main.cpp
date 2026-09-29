#include "imu_reader.hpp"
#include "../version.hpp"
#include <cstdio>
#include <string>
int main(int argc, char **argv) {
    if (argc == 2 && std::string(argv[1]) == "--version") {
        std::printf("imu_reader_demo %s\n", orbvi::kDemoVersion);
        return 0;
    }
    orbvi::imu::Options options;
    int parsed = orbvi::imu::parse_options(argc, argv, options);
    if (parsed) { orbvi::imu::print_usage(); return parsed == 1 ? 0 : 2; }
    return orbvi::imu::run(options);
}
