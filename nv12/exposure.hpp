#pragma once
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>

namespace orbvi::nv12 {
struct ExposureReading {
    int status = 0; // ISP exposure query status.
    double duration_s = 0; // ISP-reported, separate from the register snapshot.
    uint32_t frame_id = 0;
    uint64_t timestamp_ns = 0;
    int sensor_status = -1;
    unsigned sensor_code = 0;
    int line_rate_status = -1;
    uint32_t lines_per_second = 0;
};

// Optional read-only snapshots. Neither registers nor ISP metadata establish
// physical integration edges or the exposure applied to the current image.
class ExposureReporter {
public:
    ExposureReporter(unsigned camera = 0, bool enabled = false)
        : camera_(camera), enabled_(enabled) {}
    bool enabled() const { return enabled_; }

    template <typename Read>
    void poll(uint64_t now_ns, uint32_t image_id, uint64_t image_timestamp_ns,
              Read read, FILE *output = stdout) {
        if (!enabled_ || (sampled_ && now_ns - last_read_ns_ < interval_ns)) return;
        sampled_ = true;
        last_read_ns_ = now_ns;
        const ExposureReading value = read();
        char duration[48] = "unavailable", isp_duration[48] = "unavailable";
        const bool sensor_valid = !value.sensor_status && value.sensor_code <= 0xfffff;
        const bool line_rate_valid = !value.line_rate_status && value.lines_per_second;
        const char *status = !sensor_valid ? "sensor_read_failed" :
                             !line_rate_valid ? "line_rate_unavailable" : "ok";
        if (sensor_valid && line_rate_valid)
            std::snprintf(duration, sizeof(duration), "%.3f",
                          double(value.sensor_code) * 1e6 / (16.0 * value.lines_per_second));
        const long double isp_ns = static_cast<long double>(value.duration_s) * 1e9L;
        const bool isp_valid = !value.status && std::isfinite(value.duration_s) && isp_ns >= 1 &&
            isp_ns <= static_cast<long double>(std::numeric_limits<int64_t>::max());
        if (isp_valid)
            std::snprintf(isp_duration, sizeof(isp_duration), "%.3f", value.duration_s * 1e6);
        std::fprintf(output, "camera%u: exposure_status=%s exposure_duration_us=%s "
                     "duration_source=sensor_registers sensor_rc=%d exposure_code=0x%05x "
                     "exposure_lines=%.4f lines_per_second=%u line_rate_rc=%d "
                     "line_rate_source=isp_reported isp_exposure_us=%s isp_rc=%d "
                     "sample_scope=register_snapshot\n",
                     camera_, status, duration, value.sensor_status, value.sensor_code,
                     double(value.sensor_code) / 16.0, value.lines_per_second,
                     value.line_rate_status, isp_duration, value.status);

        // Keep the previous ISP-based estimate explicitly separate: the register
        // snapshot has no frame ID and cannot be attached to this ISP timestamp.
        char start[32] = "unavailable";
        const char *start_status = value.status ? "isp_query_failed" :
                                   !isp_valid ? "invalid_isp_duration" : "timestamp_unavailable";
        if (isp_valid) {
            const auto rounded_ns = static_cast<uint64_t>(isp_ns + 0.5L);
            if (value.timestamp_ns && value.timestamp_ns >= rounded_ns) {
                std::snprintf(start, sizeof(start), "%llu",
                              static_cast<unsigned long long>(value.timestamp_ns - rounded_ns));
                start_status = "unverified_estimate";
            }
        }
        std::fprintf(output, "camera%u: exposure_start_ns=unavailable "
                     "exposure_start_estimated_ns=%s start_status=%s "
                     "start_model=isp_timestamp_minus_isp_duration clock=isp_frame_timestamp "
                     "isp_timestamp_ns=%llu isp_frame_id=%u image_frame_id=%u "
                     "isp_frame_match=%u sensor_frame_match=unverified\n",
                     camera_, start, start_status,
                     static_cast<unsigned long long>(value.timestamp_ns), value.frame_id, image_id,
                     unsigned(!value.status && value.timestamp_ns && value.frame_id == image_id &&
                              value.timestamp_ns == image_timestamp_ns));
        std::fflush(output);
    }

private:
    static constexpr uint64_t interval_ns = 1000000000ULL;
    unsigned camera_;
    bool enabled_;
    bool sampled_ = false;
    uint64_t last_read_ns_ = 0;
};
}
