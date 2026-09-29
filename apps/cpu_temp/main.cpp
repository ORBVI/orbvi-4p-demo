#include "cpu_temp.hpp"

int main(int argc, char **argv) {
    orbvi::cpu_temp::Options options;
    const int parsed = orbvi::cpu_temp::parse_options(argc, argv, options);
    if (parsed) {
        orbvi::cpu_temp::print_usage();
        return parsed == 1 ? 0 : 2;
    }
    return orbvi::cpu_temp::run(options);
}
