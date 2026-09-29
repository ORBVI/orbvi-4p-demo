#include "cpu_temp.hpp"

#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <fstream>
#include <limits>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using namespace orbvi::cpu_temp;

namespace {
namespace fs = std::filesystem;

void write_text(const fs::path &path, const std::string &text) {
    std::ofstream output(path);
    assert(output.is_open());
    output << text;
    assert(output.good());
}

fs::path make_fixture(const fs::path &root, const std::string &name,
                      const std::string &type, const std::string &temperature) {
    const fs::path zone = root / name;
    assert(fs::create_directories(zone));
    write_text(zone / "type", type);
    write_text(zone / "temp", temperature);
    return root;
}

std::uint64_t timestamp_ns(const timespec &time) {
    return static_cast<std::uint64_t>(time.tv_sec) * 1000000000ULL +
           static_cast<std::uint64_t>(time.tv_nsec);
}

std::uint32_t read_be32(const std::uint8_t *data) {
    return (static_cast<std::uint32_t>(data[0]) << 24) |
           (static_cast<std::uint32_t>(data[1]) << 16) |
           (static_cast<std::uint32_t>(data[2]) << 8) |
           static_cast<std::uint32_t>(data[3]);
}

std::uint64_t read_be64(const std::uint8_t *data) {
    std::uint64_t value = 0;
    for (unsigned index = 0; index < 8; ++index) {
        value = (value << 8) | data[index];
    }
    return value;
}

float read_float_be(const std::uint8_t *data) {
    const std::uint32_t bits = read_be32(data);
    float value = 0.0f;
    static_assert(sizeof(value) == sizeof(bits), "float32 is required by OCPU/1");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
} // namespace

int main() {
    char fixture_template[] = "/tmp/orbvi-cpu-temp-test-XXXXXX";
    const char *created = ::mkdtemp(fixture_template);
    assert(created != nullptr);
    const fs::path root(created);
    const fs::path good_root = root / "good";
    const fs::path bad_root = root / "bad";
    const fs::path malformed_root = root / "malformed";
    assert(fs::create_directories(good_root));
    assert(fs::create_directories(bad_root));
    assert(fs::create_directories(malformed_root));

    // A prefix match must not select this zone; thermal_zone1 is the exact
    // thermal-cpu fixture and supplies the expected 42.5 C reading.
    make_fixture(good_root, "thermal_zone0", "thermal-cpu-extra\n", "99999\n");
    make_fixture(good_root, "thermal_zone1", "thermal-cpu\n", "42500\n");
    make_fixture(bad_root, "thermal_zone0", "x86_pkg_temp\n", "12345\n");
    make_fixture(bad_root, "thermal_zone1", "thermal-cpu-extra\n", "23456\n");
    make_fixture(malformed_root, "thermal_zone0", "thermal-cpu\n", "not-a-temperature\n");
    make_fixture(malformed_root, "thermal_zone1", "thermal-cpu\n", "999999999999999999999999\n");
    make_fixture(malformed_root, "thermal_zone2", "thermal-cpu\n", "12.3\n");
    make_fixture(malformed_root, "thermal_zone3", "thermal-cpu\n", std::string("1\0garbage\n", 10));

    Options parsed;
    const char *parsed_argv[]{
        "cpu_temp_demo", "--samples", "2", "--period-ms", "1",
        "--udp-dest", "127.0.0.1:12345", "--thermal-root", "/tmp/fixture",
    };
    assert(parse_options(9, parsed_argv, parsed) == 0);
    assert(parsed.samples == 2);
    assert(parsed.period_ms == 1);
    assert(parsed.udp_dest == "127.0.0.1:12345");
    assert(parsed.thermal_root == "/tmp/fixture");

    const char *default_argv[]{
        "cpu_temp_demo", "--samples", "1", "--period-ms", "1",
        "--udp-dest", "127.0.0.1:12345",
    };
    assert(parse_options(7, default_argv, parsed) == 0);
    assert(parsed.thermal_root == "/sys/class/thermal");

    const char *help_argv[]{"cpu_temp_demo", "--help"};
    assert(parse_options(2, help_argv, parsed) == 1);
    const char *missing_argv[]{
        "cpu_temp_demo", "--samples", "1", "--period-ms", "1",
    };
    assert(parse_options(5, missing_argv, parsed) == 2);
    const char *invalid_udp_argv[]{
        "cpu_temp_demo", "--samples", "1", "--period-ms", "1",
        "--udp-dest", "127.0.0.1:0",
    };
    assert(parse_options(7, invalid_udp_argv, parsed) == 2);

    const std::uint8_t check[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    assert(crc32_iso_hdlc(check, sizeof(check)) == 0xcbf43926u);
    const std::uint8_t empty = 0;
    assert(crc32_iso_hdlc(&empty, 0) == 0u);

    const Sample golden_sample{25.5f, 0x1112131415161718ULL};
    const std::array<std::uint8_t, kPacketSize> golden_packet{
        0x4f, 0x43, 0x50, 0x55, 0x01, 0x01, 0x00, 0x00,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x41, 0xcc, 0x00, 0x00, 0x4a, 0xe5, 0x12, 0xf5,
    };
    std::array<std::uint8_t, kPacketSize> packet{};
    assert(encode_packet(golden_sample, 0x0102030405060708ULL, packet));
    assert(packet == golden_packet);

    Sample invalid_sample = golden_sample;
    invalid_sample.timestamp_ns = 0;
    assert(!encode_packet(invalid_sample, 0, packet));
    invalid_sample = golden_sample;
    invalid_sample.celsius = std::numeric_limits<float>::quiet_NaN();
    assert(!encode_packet(invalid_sample, 0, packet));
    invalid_sample = golden_sample;
    invalid_sample.celsius = std::numeric_limits<float>::infinity();
    assert(!encode_packet(invalid_sample, 0, packet));

    Sample sample{};
    std::string error;
    timespec before{};
    timespec after{};
    assert(::clock_gettime(CLOCK_MONOTONIC, &before) == 0);
    assert(read_cpu_temperature(good_root.string(), sample, error));
    assert(::clock_gettime(CLOCK_MONOTONIC, &after) == 0);
    assert(sample.celsius == 42.5f);
    assert(sample.timestamp_ns >= timestamp_ns(before));
    assert(sample.timestamp_ns <= timestamp_ns(after));

    Sample unchanged{123.0f, 456};
    assert(!read_cpu_temperature(bad_root.string(), unchanged, error));
    assert(!read_cpu_temperature(malformed_root.string(), unchanged, error));
    assert(!read_cpu_temperature((root / "nonexistent").string(), unchanged, error));
    assert(unchanged.celsius == 123.0f);
    assert(unchanged.timestamp_ns == 456);

    const int receiver = ::socket(AF_INET, SOCK_DGRAM, 0);
    assert(receiver >= 0);
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(::bind(receiver, reinterpret_cast<sockaddr *>(&local), sizeof(local)) == 0);
    socklen_t local_size = sizeof(local);
    assert(::getsockname(receiver, reinterpret_cast<sockaddr *>(&local), &local_size) == 0);
    const std::string destination =
        "127.0.0.1:" + std::to_string(ntohs(local.sin_port));

    const char *udp_argv[]{
        "cpu_temp_demo", "--samples", "2", "--period-ms", "1",
        "--udp-dest", destination.c_str(), "--thermal-root", good_root.c_str(),
    };
    assert(parse_options(9, udp_argv, parsed) == 0);
    assert(::clock_gettime(CLOCK_MONOTONIC, &before) == 0);
    assert(run(parsed) == 0);
    assert(::clock_gettime(CLOCK_MONOTONIC, &after) == 0);

    for (std::uint64_t sequence = 0; sequence < 2; ++sequence) {
        pollfd descriptor{receiver, POLLIN, 0};
        assert(::poll(&descriptor, 1, 1000) == 1);
        std::array<std::uint8_t, 128> received{};
        const ssize_t received_size = ::recv(receiver, received.data(), received.size(), 0);
        assert(received_size == static_cast<ssize_t>(kPacketSize));
        assert(std::memcmp(received.data(), "OCPU", 4) == 0);
        assert(received[4] == 1 && received[5] == 1);
        assert(received[6] == 0 && received[7] == 0);
        assert(read_be64(received.data() + 8) == sequence);
        const std::uint64_t sent_timestamp = read_be64(received.data() + 16);
        assert(sent_timestamp >= timestamp_ns(before));
        assert(sent_timestamp <= timestamp_ns(after));
        assert(read_float_be(received.data() + 24) == 42.5f);
        assert(read_be32(received.data() + 28) ==
               crc32_iso_hdlc(received.data(), 28));
    }

    Options failed{1, 1, destination, bad_root.string()};
    const int saved_stderr = ::dup(STDERR_FILENO);
    const int null_stderr = ::open("/dev/null", O_WRONLY);
    assert(saved_stderr >= 0 && null_stderr >= 0);
    assert(::dup2(null_stderr, STDERR_FILENO) == STDERR_FILENO);
    const int failed_result = run(failed);
    assert(::dup2(saved_stderr, STDERR_FILENO) == STDERR_FILENO);
    ::close(null_stderr);
    ::close(saved_stderr);
    assert(failed_result == 1);
    pollfd no_packet{receiver, POLLIN, 0};
    assert(::poll(&no_packet, 1, 0) == 0);

    ::close(receiver);
    assert(fs::remove_all(root) > 0);
}
