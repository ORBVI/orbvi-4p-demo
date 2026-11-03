#include "validation.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iterator>
#include <limits>
#include <regex>
#include <set>
#include <stdexcept>
namespace orbvi::validation {
void require(bool condition, const std::string &message) {
    if (!condition) throw std::runtime_error(message);
}
int64_t now_ns() {
    timespec time{};
    require(clock_gettime(CLOCK_MONOTONIC, &time) == 0, "clock_gettime failed");
    return int64_t(time.tv_sec)*1000000000 + time.tv_nsec;
}
Layout make_layout(std::vector<ScanEntry> entries) {
    require(!entries.empty(), "Empty IIO scan layout");
    std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b){return a.index < b.index;});
    Layout result;
    size_t alignment = 1;
    std::set<int> indices;
    std::set<std::string> names;
    const std::regex pattern("(le|be):([su])([0-9]+)/([0-9]+)>>([0-9]+)");
    for (const auto &entry : entries) {
        std::smatch match;
        require(entry.index >= 0 && indices.insert(entry.index).second && names.insert(entry.name).second,
                "Duplicate or invalid IIO scan index/name");
        require(std::regex_match(entry.type, match, pattern), "Unsupported IIO type: " + entry.type);
        unsigned bits = std::stoul(match[3]), storage = std::stoul(match[4]), shift = std::stoul(match[5]);
        require((storage == 8 || storage == 16 || storage == 32 || storage == 64) &&
                bits > 0 && shift < storage && bits <= storage - shift, "Unsupported IIO type: " + entry.type);
        size_t width = storage/8;
        result.size = (result.size+width-1)/width*width;
        result.fields.push_back({entry.name, result.size, width, bits, shift, match[1] == "le", match[2] == "s"});
        result.size += width;
        alignment = std::max(alignment, width);
    }
    result.size = (result.size+alignment-1)/alignment*alignment;
    return result;
}
Row Layout::decode(const uint8_t *data) const {
    Row result;
    for (const auto &field : fields) {
        uint64_t value = 0;
        for (size_t i = 0; i < field.width; ++i)
            value |= uint64_t(data[field.offset+i]) << (8*(field.little ? i : field.width-1-i));
        value >>= field.shift;
        if (field.bits < 64) {
            const uint64_t mask = (uint64_t(1)<<field.bits)-1;
            value &= mask;
            if (field.signed_value && (value & (uint64_t(1)<<(field.bits-1)))) value |= ~mask;
        }
        require(field.signed_value || value <= INT64_MAX, "Unsigned IIO value exceeds JSON integer range");
        int64_t signed_value;
        std::memcpy(&signed_value, &value, sizeof(value));
        result[field.name] = signed_value;
    }
    return result;
}
std::vector<IioDevice> find_imu_devices(const std::filesystem::path &root) {
    namespace fs = std::filesystem;
    std::vector<IioDevice> candidates;
    for (const auto &entry : fs::directory_iterator(root)) {
        auto path = entry.path();
        auto id = path.filename().string();
        if (id.rfind("iio:device", 0) != 0 || !fs::is_directory(path / "scan_elements")) continue;
        auto has_axes = [&](const std::string &prefix) {
            for (const auto *axis : {"x", "y", "z"})
                if (!fs::exists(path / "scan_elements" / (prefix + axis + "_en"))) return false;
            return true;
        };
        bool accel = has_axes("in_accel_"), gyro = has_axes("in_anglvel_");
        if (!accel && !gyro) continue;
        std::ifstream name_file(path / "name");
        std::string name;
        require(bool(name_file) && bool(std::getline(name_file, name)), "Cannot read IIO device name: " + id);
        candidates.push_back({path, id, name, accel, gyro});
    }
    const IioDevice *accel = nullptr, *gyro = nullptr;
    for (const auto &device : candidates) {
        if (device.accel) {
            require(!accel, "Ambiguous IIO accelerometer devices");
            accel = &device;
        }
        if (device.gyro) {
            require(!gyro, "Ambiguous IIO gyroscope devices");
            gyro = &device;
        }
    }
    require(accel && gyro, "Missing three-axis IIO accelerometer or gyroscope");
    if (accel->path != gyro->path)
        require(fs::canonical(accel->path).parent_path() == fs::canonical(gyro->path).parent_path(),
                "IIO accelerometer and gyroscope do not share a parent device");
    std::vector<IioDevice> selected{*accel};
    if (accel->path != gyro->path) selected.push_back(*gyro);
    return selected;
}
double iio_axis_value(const std::filesystem::path &device, const std::string &channel,
                      const std::string &axis, const std::string &attribute, double fallback) {
    for (const auto &name : {channel + "_" + axis + "_" + attribute, channel + "_" + attribute}) {
        auto path = device / name;
        if (!std::filesystem::exists(path)) continue;
        std::ifstream input(path);
        double value;
        require(bool(input >> value) && std::isfinite(value), "Invalid IIO attribute: " + path.string());
        input >> std::ws;
        require(input.eof(), "Invalid IIO attribute: " + path.string());
        return value;
    }
    require(std::isfinite(fallback), "Missing IIO " + channel + " " + attribute + ": " + device.string());
    return fallback;
}
unsigned choose_iio_rate(const std::vector<std::filesystem::path> &frequencies, unsigned requested) {
    require(!frequencies.empty(), "Missing IIO sampling frequency");
    std::set<unsigned> common;
    bool initialized = false;
    for (const auto &frequency : frequencies) {
        auto available = std::filesystem::path(frequency.string() + "_available");
        bool has_available = std::filesystem::exists(available);
        if (requested && !has_available) continue;
        std::ifstream input(has_available ? available : frequency);
        require(bool(input), "Cannot read IIO sampling frequencies: " + frequency.string());
        std::set<unsigned> rates;
        double value;
        while (input >> value) {
            require(std::isfinite(value) && value > 0,
                    "Unsupported IIO sampling frequency: " + frequency.string());
            if (value >= 1 && value <= 2000 && std::floor(value) == value)
                rates.insert(static_cast<unsigned>(value));
        }
        require(input.eof() && !rates.empty(), "Invalid IIO sampling frequencies: " + frequency.string());
        if (requested) {
            require(rates.count(requested), "IIO sampling frequency is unavailable: " + std::to_string(requested));
        } else if (!initialized) {
            common = std::move(rates);
            initialized = true;
        } else {
            std::set<unsigned> intersection;
            std::set_intersection(common.begin(), common.end(), rates.begin(), rates.end(),
                                  std::inserter(intersection, intersection.end()));
            common = std::move(intersection);
        }
    }
    if (requested) return requested;
    require(!common.empty(), "No common IIO sampling frequency");
    return *std::min_element(common.begin(), common.end(), [](unsigned left, unsigned right) {
        return std::abs(int(left) - 1000) < std::abs(int(right) - 1000);
    });
}
double median(std::vector<int64_t> values) {
    require(!values.empty(), "Cannot compute empty median");
    std::sort(values.begin(), values.end());
    size_t mid = values.size()/2;
    return values.size()%2 ? double(values[mid]) : (double(values[mid-1])+double(values[mid]))/2;
}
Sampling sampling(const std::vector<Row> &rows, int64_t start, int64_t end, double nominal_hz) {
    require(end > start && std::isfinite(nominal_hz) && nominal_hz >= 800 && nominal_hz <= 2000,
            "Invalid measurement window or IMU sampling frequency");
    std::vector<int64_t> stamps, gaps;
    for (const auto &row : rows) {
        auto timestamp = row.at("in_timestamp");
        if (start <= timestamp && timestamp < end) stamps.push_back(timestamp);
    }
    require(stamps.size() >= 2, "Insufficient timestamped IMU samples");
    for (size_t i = 1; i < stamps.size(); ++i) {
        int64_t gap = stamps[i]-stamps[i-1];
        require(gap > 0 && double(gap) < 1.8e9/nominal_hz, "Discontinuous IMU sample timestamps");
        gaps.push_back(gap);
    }
    double duration = double(end-start)/1e9;
    double rate = (stamps.size()-1)*1e9/double(stamps.back()-stamps.front());
    require(std::abs(rate-nominal_hz) <= 0.03*nominal_hz, "IMU ODR outside nominal rate +/-3%");
    require(std::abs(double(stamps.size())-nominal_hz*duration) <= 0.03*nominal_hz*duration+3,
            "Missing IMU samples over measurement window");
    return {stamps.size(), duration, rate, median(gaps), stamps.front(), stamps.back(),
            *std::min_element(gaps.begin(), gaps.end()), *std::max_element(gaps.begin(), gaps.end())};
}
void stop_capture(const std::function<void()> &signal, const std::function<int()> &poll,
                  const std::function<void()> &drain, const std::function<void()> &disable,
                  const std::function<void()> &snapshot, int64_t timeout) {
    signal();
    const auto deadline = now_ns()+timeout;
    try {
        while (poll() == -1) {
            require(now_ns() < deadline, "Capture shutdown timeout");
            drain();
        }
    } catch (...) {
        try { disable(); } catch (...) {}
        throw;
    }
    disable();
    snapshot();
    require(poll() == 130, "Capture or SDK cleanup failed");
}
std::vector<std::string> cleanup_all(const std::vector<std::pair<std::string, std::function<void()>>> &actions) {
    std::vector<std::string> errors;
    for (const auto &action : actions) {
        try { action.second(); }
        catch (const std::exception &error) { errors.push_back(action.first+": "+error.what()); }
        catch (...) { errors.push_back(action.first+": unknown failure"); }
    }
    return errors;
}
}
