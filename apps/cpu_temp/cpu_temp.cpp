#include "cpu_temp.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fcntl.h>
#include <limits>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace orbvi::cpu_temp {
namespace {
namespace fs = std::filesystem;
constexpr unsigned kMaxAttempts = 5;
constexpr unsigned kRetryMs = 100;

bool unsigned_number(const std::string &text, std::uint64_t min, std::uint64_t max,
                     std::uint64_t &result) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](unsigned char c) {
            return c >= '0' && c <= '9';
        })) return false;
    errno = 0;
    char *end = nullptr;
    const unsigned long long number = std::strtoull(text.c_str(), &end, 10);
    if (errno || *end || number < min || number > max) return false;
    result = number;
    return true;
}

bool endpoint(const std::string &text, sockaddr_in &address) {
    const auto colon = text.find(':');
    if (colon == std::string::npos || text.find(':', colon + 1) != std::string::npos)
        return false;
    std::uint64_t port;
    if (!unsigned_number(text.substr(colon + 1), 1, 65535, port)) return false;
    address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<std::uint16_t>(port));
    return inet_pton(AF_INET, text.substr(0, colon).c_str(), &address.sin_addr) == 1;
}

// Nonblocking reads and a fixed maximum length prevent a bad fixture/device from
// hanging this finite demo on a FIFO or an unexpectedly large sysfs value.
bool read_text(const fs::path &path, std::string &text) {
    const int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) return false;
    char buffer[128];
    ssize_t count;
    do { count = read(fd, buffer, sizeof(buffer)); } while (count < 0 && errno == EINTR);
    const int saved_errno = errno;
    close(fd);
    errno = saved_errno;
    if (count <= 0 || count == static_cast<ssize_t>(sizeof(buffer))) return false;
    text.assign(buffer, static_cast<std::size_t>(count));
    // Sysfs exposes short lines. Reject embedded NUL rather than silently truncating.
    return text.find('\0') == std::string::npos;
}

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool millidegrees(const std::string &text, float &celsius) {
    const std::string value = trim(text);
    if (value.empty()) return false;
    std::size_t i = (value[0] == '-') ? 1 : 0;
    if (i == value.size()) return false;
    for (; i < value.size(); ++i)
        if (value[i] < '0' || value[i] > '9') return false;
    errno = 0;
    char *end = nullptr;
    const long long number = std::strtoll(value.c_str(), &end, 10);
    if (errno || *end || number < std::numeric_limits<std::int32_t>::min() ||
        number > std::numeric_limits<std::int32_t>::max()) return false;
    celsius = static_cast<float>(static_cast<double>(number) / 1000.0);
    return std::isfinite(celsius);
}

std::uint64_t monotonic_ns() {
    timespec time{};
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) return 0;
    return static_cast<std::uint64_t>(time.tv_sec) * 1000000000u + time.tv_nsec;
}

void sleep_ms(unsigned milliseconds) {
    timespec duration{static_cast<time_t>(milliseconds / 1000),
                      static_cast<long>(milliseconds % 1000) * 1000000L};
    while (nanosleep(&duration, &duration) < 0 && errno == EINTR) {}
}

void put32(std::uint8_t *at, std::uint32_t value) {
    for (unsigned n = 0; n < 4; ++n) at[n] = static_cast<std::uint8_t>(value >> (24 - 8 * n));
}
void put64(std::uint8_t *at, std::uint64_t value) {
    for (unsigned n = 0; n < 8; ++n) at[n] = static_cast<std::uint8_t>(value >> (56 - 8 * n));
}
} // namespace

int parse_options(int argc, const char *const argv[], Options &options) {
    options = {};
    unsigned seen = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string key(argv[i]);
        if (key == "--help" && argc == 2) return 1;
        if (++i >= argc) return 2;
        const std::string value(argv[i]);
        std::uint64_t number = 0;
        if (key == "--samples" && !(seen & 1) &&
            unsigned_number(value, 1, 100000, number)) {
            options.samples = static_cast<std::uint32_t>(number); seen |= 1;
        } else if (key == "--period-ms" && !(seen & 2) &&
                   unsigned_number(value, 1, 60000, number)) {
            options.period_ms = static_cast<std::uint32_t>(number); seen |= 2;
        } else if (key == "--udp-dest" && !(seen & 4)) {
            sockaddr_in address{};
            if (!endpoint(value, address)) return 2;
            options.udp_dest = value; seen |= 4;
        } else if (key == "--thermal-root" && !(seen & 8) && !value.empty()) {
            options.thermal_root = value; seen |= 8;
        } else return 2;
    }
    return (seen & 7) == 7 ? 0 : 2;
}

void print_usage() {
    std::puts("Usage: cpu_temp_demo --samples N --period-ms T --udp-dest IPv4:PORT "
              "[--thermal-root DIR]\n"
              "  N: 1..100000 successful UDP sends; T: 1..60000 ms between successes\n"
              "  DIR defaults to /sys/class/thermal; only type=thermal-cpu is used");
}

bool read_cpu_temperature(const std::string &thermal_root, Sample &sample,
                          std::string &error) {
    std::error_code ec;
    fs::directory_iterator entries(thermal_root, ec);
    if (ec) { error = "cannot enumerate thermal root: " + ec.message(); return false; }
    std::vector<fs::path> zones;
    for (fs::directory_iterator end; entries != end; entries.increment(ec)) {
        if (ec) { error = "thermal root enumeration failed: " + ec.message(); return false; }
        const std::string name = entries->path().filename().string();
        const std::string prefix = "thermal_zone";
        if (name.compare(0, prefix.size(), prefix) != 0 || name.size() == prefix.size() ||
            !std::all_of(name.begin() + prefix.size(), name.end(), [](unsigned char c) {
                return c >= '0' && c <= '9';
            })) continue;
        zones.push_back(entries->path());
    }
    if (ec) { error = "thermal root enumeration failed: " + ec.message(); return false; }
    std::sort(zones.begin(), zones.end());
    bool matched = false;
    for (const auto &zone : zones) {
        std::string type;
        if (!read_text(zone / "type", type) || trim(type) != "thermal-cpu") continue;
        matched = true;
        std::string value;
        float temperature;
        if (!read_text(zone / "temp", value) || !millidegrees(value, temperature)) continue;
        const std::uint64_t timestamp = monotonic_ns();
        if (!timestamp) { error = "CLOCK_MONOTONIC failed"; return false; }
        sample = {temperature, timestamp};
        return true;
    }
    error = matched ? "no readable thermal-cpu temperature" : "no thermal-cpu zone";
    return false;
}

std::uint32_t crc32_iso_hdlc(const std::uint8_t *data, std::size_t size) noexcept {
    std::uint32_t crc = 0xffffffffu;
    for (std::size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ ((crc & 1u) ? 0xedb88320u : 0u);
    }
    return crc ^ 0xffffffffu;
}

bool encode_packet(const Sample &sample, std::uint64_t sequence,
                   std::array<std::uint8_t, kPacketSize> &packet) noexcept {
    if (!sample.timestamp_ns || !std::isfinite(sample.celsius)) return false;
    packet = {};
    std::memcpy(packet.data(), "OCPU", 4);
    packet[4] = 1;
    packet[5] = 1;
    put64(packet.data() + 8, sequence);
    put64(packet.data() + 16, sample.timestamp_ns);
    std::uint32_t bits;
    std::memcpy(&bits, &sample.celsius, sizeof(bits));
    put32(packet.data() + 24, bits);
    put32(packet.data() + 28, crc32_iso_hdlc(packet.data(), 28));
    return true;
}

int run(const Options &options) {
    sockaddr_in address{};
    if (!options.samples || !options.period_ms || options.samples > 100000 ||
        options.period_ms > 60000 || !endpoint(options.udp_dest, address) ||
        options.thermal_root.empty()) {
        std::fprintf(stderr, "invalid cpu_temp_demo configuration\n");
        return 2;
    }
    const int fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) { std::perror("cpu_temp_demo socket"); return 1; }
    int result = 0;
    for (std::uint32_t sequence = 0; sequence < options.samples; ++sequence) {
        bool sent = false;
        std::string error;
        for (unsigned attempt = 0; attempt < kMaxAttempts && !sent; ++attempt) {
            Sample sample{};
            if (!read_cpu_temperature(options.thermal_root, sample, error)) {
                if (attempt + 1 < kMaxAttempts) sleep_ms(kRetryMs);
                continue;
            }
            std::array<std::uint8_t, kPacketSize> packet{};
            if (!encode_packet(sample, sequence, packet)) { error = "invalid sample"; break; }
            // Retries reuse the exact same valid reading/timestamp; a failed send
            // cannot result in a synthetic/default temperature packet.
            for (unsigned send_attempt = 0; send_attempt < kMaxAttempts; ++send_attempt) {
                const ssize_t count = sendto(fd, packet.data(), packet.size(), MSG_DONTWAIT,
                                             reinterpret_cast<const sockaddr *>(&address), sizeof(address));
                if (count == static_cast<ssize_t>(packet.size())) { sent = true; break; }
                error = count < 0 ? std::strerror(errno) : "short UDP send";
                if (send_attempt + 1 < kMaxAttempts) {
                    pollfd descriptor{fd, POLLOUT, 0};
                    (void)poll(&descriptor, 1, kRetryMs);
                }
            }
            if (!sent && attempt + 1 < kMaxAttempts) sleep_ms(kRetryMs);
        }
        if (!sent) {
            std::fprintf(stderr, "cpu_temp_demo: sample %u failed after %u attempts: %s\n",
                         sequence, kMaxAttempts, error.c_str());
            result = 1;
            break;
        }
        if (sequence + 1 < options.samples) sleep_ms(options.period_ms);
    }
    close(fd);
    return result;
}

} // namespace orbvi::cpu_temp
