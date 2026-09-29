#include "validation.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ctime>
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
                bits > 0 && bits <= storage && shift == 0, "Unsupported IIO type: " + entry.type);
        size_t width = storage/8;
        result.size = (result.size+width-1)/width*width;
        result.fields.push_back({entry.name, result.size, width, bits, match[1] == "le", match[2] == "s"});
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
double median(std::vector<int64_t> values) {
    require(!values.empty(), "Cannot compute empty median");
    std::sort(values.begin(), values.end());
    size_t mid = values.size()/2;
    return values.size()%2 ? double(values[mid]) : (double(values[mid-1])+double(values[mid]))/2;
}
Sampling sampling(const std::vector<Row> &rows, int64_t start, int64_t end) {
    require(end > start, "Invalid measurement window");
    std::vector<int64_t> stamps, gaps;
    for (const auto &row : rows) {
        auto timestamp = row.at("in_timestamp");
        if (start <= timestamp && timestamp < end) stamps.push_back(timestamp);
    }
    require(stamps.size() >= 2, "Insufficient timestamped IMU samples");
    for (size_t i = 1; i < stamps.size(); ++i) {
        int64_t gap = stamps[i]-stamps[i-1];
        require(gap > 0 && gap < 1800000, "Discontinuous IMU sample timestamps");
        gaps.push_back(gap);
    }
    double duration = double(end-start)/1e9;
    double rate = (stamps.size()-1)*1e9/double(stamps.back()-stamps.front());
    require(rate >= 970 && rate <= 1030, "IMU ODR outside 970..1030 Hz");
    require(std::abs(double(stamps.size())-1000*duration) <= 0.03*1000*duration+3,
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
