#include "imu_reader.hpp"
#include <cassert>
#include <fstream>
#include <sstream>

using namespace orbvi::imu;
int main() {
    Options options;
    const char *basic[]{"imu_reader_demo"};
    assert(parse_options(1, basic, options) == 0);
    assert(options.sample_rate_hz == 1000 && options.print_rate_hz == 10 && options.count == 0);
    assert(options.udp_dest.empty());
    assert(options.csv_path.empty());
    // std::ofstream::operator bool reflects stream errors, not whether it
    // has an open file. The capture loop must use is_open() for --csv.
    std::ofstream unopened_csv;
    assert(static_cast<bool>(unopened_csv));
    assert(!unopened_csv.is_open());
    const char *configured[]{"demo", "--sample-rate-hz", "2000", "--print-rate-hz", "50",
                              "--count", "10000", "--print-metrics", "--csv", "/tmp/imu.csv"};
    assert(parse_options(10, configured, options) == 0);
    assert(options.sample_rate_hz == 2000 && options.print_rate_hz == 50);
    assert(options.count == 10000 && options.print_metrics);
    assert(options.csv_path == "/tmp/imu.csv");
    assert(options.udp_dest.empty());
    const char *udp[]{"demo", "--udp-dest", "192.168.1.20:12000"};
    assert(parse_options(3, udp, options) == 0);
    assert(options.udp_dest == "192.168.1.20:12000");
    const char *bad_udp[]{"demo", "--udp-dest", "192.168.1.20"};
    assert(parse_options(3, bad_udp, options) == 2);
    const char *bad_port[]{"demo", "--udp-dest", "192.168.1.20:0"};
    assert(parse_options(3, bad_port, options) == 2);
    const char *bad_ipv6[]{"demo", "--udp-dest", "[::1]:12000"};
    assert(parse_options(3, bad_ipv6, options) == 2);
    const char *bad[]{"demo", "--sample-rate-hz", "30"};
    assert(parse_options(3, bad, options) == 2);

    Sample sample{};
    sample.timestamp_ns = 123;
    sample.accel = {1.0, 2.0, 3.0}; sample.gyro = {4.0, 5.0, 6.0};
    sample.accel_timestamp_ns = 123;
    sample.gyro_timestamp_ns = 123;
    std::ostringstream output;
    print_sample(output, sample);
    assert(output.str() == "ts_ns=123 accel_mps2=[1,2,3] gyro_rps=[4,5,6]\n");
}
