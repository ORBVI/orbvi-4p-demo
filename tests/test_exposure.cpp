#include "exposure.hpp"
#include <cassert>
#include <limits>
#include <string>

using namespace orbvi::nv12;
static std::string contents(FILE *file) {
    std::rewind(file);
    std::string result;
    char line[2048];
    while (std::fgets(line, sizeof(line), file)) result += line;
    return result;
}
static std::string sample(const ExposureReading &value) {
    FILE *file = std::tmpfile(); assert(file);
    ExposureReporter reporter(0, true);
    reporter.poll(0, value.frame_id, value.timestamp_ns, [&] { return value; }, file);
    const auto text = contents(file); std::fclose(file); return text;
}
int main() {
    FILE *file = std::tmpfile(); assert(file);
    unsigned calls = 0;
    ExposureReading value{0, 0.01, 101, 2000000000ULL, 0, 0x03460, 0, 84000};
    auto read = [&] { ++calls; return value; };
    ExposureReporter disabled(0);
    disabled.poll(0, 101, value.timestamp_ns, read, file);
    disabled.poll(10000000000ULL, 101, value.timestamp_ns, read, file);
    assert(calls == 0 && contents(file).empty());
    ExposureReporter enabled(2, true);
    enabled.poll(0, 100, 1900000000ULL, read, file);
    assert(calls == 1);
    auto text = contents(file);
    assert(text.find("camera2: exposure_status=ok exposure_duration_us=9976.190") != std::string::npos);
    assert(text.find("duration_source=sensor_registers") != std::string::npos);
    assert(text.find("exposure_code=0x03460 exposure_lines=838.0000") != std::string::npos);
    assert(text.find("isp_exposure_us=10000.000") != std::string::npos);
    // Preserve the ISP-based estimate; never attach the register snapshot to its frame.
    assert(text.find("exposure_start_ns=unavailable exposure_start_estimated_ns=1990000000") != std::string::npos);
    assert(text.find("start_status=unverified_estimate") != std::string::npos);
    assert(text.find("isp_frame_id=101 image_frame_id=100 isp_frame_match=0 sensor_frame_match=unverified") != std::string::npos);
    enabled.poll(999999999ULL, 101, value.timestamp_ns, read, file);
    assert(calls == 1);
    enabled.poll(1000000000ULL, 101, value.timestamp_ns, read, file);
    assert(calls == 2 && contents(file).find("isp_frame_match=1") != std::string::npos);
    enabled.poll(1000000001ULL, 101, value.timestamp_ns, read, file);
    assert(calls == 2);
    value.status = -12;
    text = sample(value);
    assert(text.find("exposure_status=ok exposure_duration_us=9976.190") != std::string::npos);
    assert(text.find("isp_exposure_us=unavailable isp_rc=-12") != std::string::npos);
    assert(text.find("start_status=isp_query_failed") != std::string::npos);
    value.status = 0;
    value.sensor_status = -11;
    text = sample(value);
    assert(text.find("exposure_status=sensor_read_failed exposure_duration_us=unavailable") != std::string::npos);
    assert(text.find("isp_exposure_us=10000.000") != std::string::npos);
    value.sensor_status = 0;
    value.lines_per_second = 0;
    assert(sample(value).find("exposure_status=line_rate_unavailable exposure_duration_us=unavailable") != std::string::npos);
    value.lines_per_second = 84000;
    value.line_rate_status = -5;
    assert(sample(value).find("exposure_status=line_rate_unavailable") != std::string::npos);
    value.line_rate_status = 0;
    value.timestamp_ns = 0;
    text = sample(value);
    assert(text.find("start_status=timestamp_unavailable") != std::string::npos);
    assert(text.find("isp_frame_match=0") != std::string::npos);
    value.timestamp_ns = 1;
    text = sample(value);
    assert(text.find("exposure_start_estimated_ns=unavailable") != std::string::npos);
    value.sensor_code = 0x3461; // Preserve fractional 1/16-line resolution.
    assert(sample(value).find("exposure_lines=838.0625") != std::string::npos);
    for (double bad : {0.0, -1.0, std::numeric_limits<double>::quiet_NaN(),
                       std::numeric_limits<double>::infinity(), 1e20}) {
        value.duration_s = bad;
        text = sample(value);
        assert(text.find("start_status=invalid_isp_duration") != std::string::npos);
        assert(text.find("isp_exposure_us=unavailable") != std::string::npos);
        assert(text.find("exposure_status=ok") != std::string::npos);
    }
    value = {0, .001, 102, 3000000000ULL, 0, 0x260, 0, 84000};
    enabled.poll(2000000000ULL, 102, value.timestamp_ns, read, file);
    text = contents(file);
    assert(text.find("exposure_duration_us=452.381") != std::string::npos);
    assert(text.find("exposure_start_estimated_ns=2999000000") != std::string::npos);
    std::fclose(file);
}
