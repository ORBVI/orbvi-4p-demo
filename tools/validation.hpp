#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>
namespace orbvi::validation {
using Row = std::map<std::string, int64_t>;
struct ScanEntry { std::string name, type; int index; };
struct Field { std::string name; size_t offset, width; unsigned bits, shift; bool little, signed_value; };
struct Layout { size_t size = 0; std::vector<Field> fields; Row decode(const uint8_t *) const; };
Layout make_layout(std::vector<ScanEntry>);
struct IioDevice {
    std::filesystem::path path;
    std::string id, name;
    bool accel = false, gyro = false;
};
std::vector<IioDevice> find_imu_devices(const std::filesystem::path &root);
double iio_axis_value(const std::filesystem::path &device, const std::string &channel,
                      const std::string &axis, const std::string &attribute, double fallback);
unsigned choose_iio_rate(const std::vector<std::filesystem::path> &frequencies, unsigned requested);
struct Sampling { size_t samples; double duration, hz, median; int64_t first, last, min_gap, max_gap; };
Sampling sampling(const std::vector<Row> &, int64_t, int64_t, double nominal_hz = 1000);
void stop_capture(const std::function<void()> &, const std::function<int()> &,
                  const std::function<void()> &, const std::function<void()> &,
                  const std::function<void()> &, int64_t = 10000000000LL);
std::vector<std::string> cleanup_all(const std::vector<std::pair<std::string, std::function<void()>>> &);
int64_t now_ns();
double median(std::vector<int64_t> values);
void require(bool condition, const std::string &message);
}
