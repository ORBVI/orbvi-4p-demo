#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>
namespace orbvi::validation {
using Row = std::map<std::string, int64_t>;
struct ScanEntry { std::string name, type; int index; };
struct Field { std::string name; size_t offset, width; unsigned bits; bool little, signed_value; };
struct Layout { size_t size = 0; std::vector<Field> fields; Row decode(const uint8_t *) const; };
Layout make_layout(std::vector<ScanEntry>);
struct Sampling { size_t samples; double duration, hz, median; int64_t first, last, min_gap, max_gap; };
Sampling sampling(const std::vector<Row> &, int64_t, int64_t);
void stop_capture(const std::function<void()> &, const std::function<int()> &,
                  const std::function<void()> &, const std::function<void()> &,
                  const std::function<void()> &, int64_t = 10000000000LL);
std::vector<std::string> cleanup_all(const std::vector<std::pair<std::string, std::function<void()>>> &);
int64_t now_ns();
double median(std::vector<int64_t> values);
void require(bool condition, const std::string &message);
}
