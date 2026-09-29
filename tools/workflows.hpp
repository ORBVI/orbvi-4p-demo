#pragma once
#include "runtime.hpp"
#include <array>
namespace orbvi::validation {
struct Phase { std::string name; int64_t start=0, end=0; std::vector<int64_t> pulses; };
Json to_json(const Phase &);
Json analyze_phase(const Phase &, const std::array<std::vector<CsvRow>,4> &, const Iio &);
int run_sync(const std::string &runner, bool raw, unsigned seconds, const fs::path &base);
int run_backpressure(const fs::path &output);
int run_board(const std::string &runner);
int run_shutdown(const std::string &runner, const std::string &injection);
}
