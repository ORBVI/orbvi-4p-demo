#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>

namespace orbvi::imu {

struct Sample;

constexpr std::size_t kImuDatagramSize = 72;
constexpr std::uint8_t kImuWireVersion = 1;
constexpr std::uint8_t kImuWireType = 1;

struct UdpEndpoint {
    // address is in network byte order; port is in host byte order.
    std::uint32_t address = 0;
    std::uint16_t port = 0;
};

struct UdpMetrics {
    std::uint64_t samples = 0;
    std::uint64_t sent = 0;
    std::uint64_t dropped = 0;
};

// Parses numeric IPv4:PORT only. No DNS resolution or IPv6 is attempted.
bool parse_udp_endpoint(const std::string &text, UdpEndpoint &endpoint,
                        std::string *error = nullptr);
std::uint32_t crc32_iso_hdlc(const std::uint8_t *data, std::size_t size) noexcept;
bool encode_imu_datagram(const Sample &sample, std::uint64_t sequence,
                         std::uint64_t send_timestamp_ns, std::uint32_t flags,
                         std::array<std::uint8_t, kImuDatagramSize> &packet) noexcept;

class DatagramTransport {
public:
    virtual ~DatagramTransport() = default;
    virtual ssize_t send(const std::uint8_t *data, std::size_t size) = 0;
};

class UdpSender final : public DatagramTransport {
public:
    explicit UdpSender(const UdpEndpoint &endpoint);
    ~UdpSender() override;

    UdpSender(const UdpSender &) = delete;
    UdpSender &operator=(const UdpSender &) = delete;

    ssize_t send(const std::uint8_t *data, std::size_t size) noexcept override;

private:
    int fd_ = -1;
};

class UdpPublisher {
public:
    explicit UdpPublisher(const UdpEndpoint &endpoint);
    explicit UdpPublisher(std::unique_ptr<DatagramTransport> transport);

    void publish(const Sample &sample, std::uint64_t send_timestamp_ns);
    const UdpMetrics &metrics() const noexcept { return metrics_; }

private:
    std::unique_ptr<DatagramTransport> transport_;
    UdpMetrics metrics_;
};

}
