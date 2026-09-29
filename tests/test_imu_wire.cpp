#include "imu_reader.hpp"
#include "imu_wire.hpp"

#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using namespace orbvi::imu;

namespace {
const std::uint64_t kSequence = 0x0102030405060708ULL;
const std::uint64_t kAccelTimestamp = 0x1112131415161718ULL;
const std::uint64_t kGyroTimestamp = 0x2122232425262728ULL;
const std::uint64_t kSendTimestamp = 0x3132333435363738ULL;

Sample golden_sample() {
    Sample sample;
    sample.timestamp_ns = static_cast<std::int64_t>(kAccelTimestamp);
    sample.accel = {1.0, -2.5, 3.25};
    sample.gyro = {-4.0, 5.5, -6.75};
    sample.accel_timestamp_ns = static_cast<std::int64_t>(kAccelTimestamp);
    sample.gyro_timestamp_ns = static_cast<std::int64_t>(kGyroTimestamp);
    return sample;
}

const std::array<std::uint8_t, kImuDatagramSize> kGoldenPacket{
    0x4f, 0x49, 0x4d, 0x55, 0x01, 0x01, 0x00, 0x48,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x00, 0x00, 0x00, 0x00,
    0x3f, 0x80, 0x00, 0x00, 0xc0, 0x20, 0x00, 0x00,
    0x40, 0x50, 0x00, 0x00, 0xc0, 0x80, 0x00, 0x00,
    0x40, 0xb0, 0x00, 0x00, 0xc0, 0xd8, 0x00, 0x00,
    0xf3, 0x60, 0x80, 0x31,
};

class FailingTransport final : public DatagramTransport {
public:
    ssize_t send(const std::uint8_t *, std::size_t) override {
        errno = EAGAIN;
        return -1;
    }
};

class CountingTransport final : public DatagramTransport {
public:
    int calls = 0;

    ssize_t send(const std::uint8_t *, std::size_t size) override {
        ++calls;
        return static_cast<ssize_t>(size);
    }
};
}

int main() {
    UdpEndpoint endpoint;
    std::string error;
    assert(parse_udp_endpoint("127.0.0.1:1", endpoint, &error));
    assert(endpoint.port == 1);
    in_addr loopback{};
    assert(inet_pton(AF_INET, "127.0.0.1", &loopback) == 1);
    assert(endpoint.address == loopback.s_addr);

    const char *const invalid_endpoints[]{
        "", "127.0.0.1", "127.0.0.1:0", "127.0.0.1:65536", "127.0.0.1:+1",
        "127.0.0.1:1x", "127.0.0.1:1:2", "localhost:1", "[::1]:1", "1.2.3.999:1",
    };
    for (const char *text : invalid_endpoints) {
        assert(!parse_udp_endpoint(text, endpoint, &error));
    }

    const char *const non_unicast_endpoints[]{
        "0.0.0.0:1", "224.0.0.1:1", "239.255.255.255:1",
        "240.0.0.1:1", "255.255.255.255:1",
    };
    for (const char *text : non_unicast_endpoints) {
        assert(!parse_udp_endpoint(text, endpoint, &error));
    }
    assert(parse_udp_endpoint("127.0.0.1:1", endpoint, &error));

    const std::uint8_t check[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    assert(crc32_iso_hdlc(check, sizeof(check)) == 0xcbf43926u);
    const std::uint8_t empty = 0;
    assert(crc32_iso_hdlc(&empty, 0) == 0u);

    const Sample sample = golden_sample();
    std::array<std::uint8_t, kImuDatagramSize> packet{};
    assert(encode_imu_datagram(sample, kSequence, kSendTimestamp, 0, packet));
    assert(packet == kGoldenPacket);

    Sample invalid = sample;
    invalid.accel[0] = std::numeric_limits<double>::quiet_NaN();
    assert(!encode_imu_datagram(invalid, kSequence, kSendTimestamp, 0, packet));
    invalid = sample;
    invalid.gyro[2] = std::numeric_limits<double>::infinity();
    assert(!encode_imu_datagram(invalid, kSequence, kSendTimestamp, 0, packet));
    invalid = sample;
    invalid.accel[1] = std::numeric_limits<double>::max();
    assert(!encode_imu_datagram(invalid, kSequence, kSendTimestamp, 0, packet));
    invalid = sample;
    invalid.accel_timestamp_ns = -1;
    assert(!encode_imu_datagram(invalid, kSequence, kSendTimestamp, 0, packet));
    invalid = sample;
    invalid.timestamp_ns = -1;
    assert(!encode_imu_datagram(invalid, kSequence, kSendTimestamp, 0, packet));

    const int receiver = ::socket(AF_INET, SOCK_DGRAM, 0);
    assert(receiver >= 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(::bind(receiver, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == 0);
    socklen_t address_size = sizeof(address);
    assert(::getsockname(receiver, reinterpret_cast<sockaddr *>(&address), &address_size) == 0);

    UdpEndpoint local_endpoint;
    assert(parse_udp_endpoint("127.0.0.1:" + std::to_string(ntohs(address.sin_port)),
                              local_endpoint, &error));
    UdpPublisher publisher(local_endpoint);
    publisher.publish(sample, kSendTimestamp);
    const UdpMetrics &delivery_metrics = publisher.metrics();
    assert(delivery_metrics.samples == 1);
    assert(delivery_metrics.sent == 1);
    assert(delivery_metrics.dropped == 0);

    pollfd descriptor{receiver, POLLIN, 0};
    assert(::poll(&descriptor, 1, 1000) == 1);
    std::array<std::uint8_t, 128> received{};
    const ssize_t received_size = ::recv(receiver, received.data(), received.size(), 0);
    assert(received_size == static_cast<ssize_t>(kImuDatagramSize));
    std::array<std::uint8_t, kImuDatagramSize> expected{};
    assert(encode_imu_datagram(sample, 0, kSendTimestamp, 0, expected));
    assert(std::memcmp(received.data(), expected.data(), expected.size()) == 0);
    ::close(receiver);

    UdpPublisher failing(std::make_unique<FailingTransport>());
    failing.publish(sample, kSendTimestamp);
    failing.publish(sample, kSendTimestamp);
    const UdpMetrics &failed_metrics = failing.metrics();
    assert(failed_metrics.samples == 2);
    assert(failed_metrics.sent == 0);
    assert(failed_metrics.dropped == 2);

    auto counting = std::make_unique<CountingTransport>();
    CountingTransport *counter = counting.get();
    UdpPublisher invalid_publisher(std::move(counting));
    invalid_publisher.publish(invalid, kSendTimestamp);
    assert(counter->calls == 0);
    assert(invalid_publisher.metrics().samples == 1);
    assert(invalid_publisher.metrics().sent == 0);
    assert(invalid_publisher.metrics().dropped == 1);
}
