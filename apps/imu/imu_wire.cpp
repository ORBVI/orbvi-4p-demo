#include "imu_wire.hpp"
#include "imu_reader.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <limits>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace orbvi::imu {
namespace {
constexpr std::uint32_t kCrcPolynomial = 0xedb88320u;

void set_error(std::string *error, const char *text) {
    if (error) *error = text;
}

void put_u16(std::uint8_t *destination, std::uint16_t value) noexcept {
    destination[0] = static_cast<std::uint8_t>(value >> 8);
    destination[1] = static_cast<std::uint8_t>(value);
}

void put_u32(std::uint8_t *destination, std::uint32_t value) noexcept {
    destination[0] = static_cast<std::uint8_t>(value >> 24);
    destination[1] = static_cast<std::uint8_t>(value >> 16);
    destination[2] = static_cast<std::uint8_t>(value >> 8);
    destination[3] = static_cast<std::uint8_t>(value);
}

void put_u64(std::uint8_t *destination, std::uint64_t value) noexcept {
    for (unsigned byte = 0; byte < 8; ++byte) {
        destination[byte] = static_cast<std::uint8_t>(value >> (56 - byte * 8));
    }
}

std::uint32_t float_bits(float value) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

bool finite_float(double value) noexcept {
    if (!std::isfinite(value)) return false;
    const float narrowed = static_cast<float>(value);
    return std::isfinite(narrowed);
}

bool valid_sample(const Sample &sample) noexcept {
    if (sample.timestamp_ns < 0 || sample.accel_timestamp_ns < 0 ||
        sample.gyro_timestamp_ns < 0) {
        return false;
    }
    for (double value : sample.accel) {
        if (!finite_float(value)) return false;
    }
    for (double value : sample.gyro) {
        if (!finite_float(value)) return false;
    }
    return true;
}
}

bool parse_udp_endpoint(const std::string &text, UdpEndpoint &endpoint,
                        std::string *error) {
    endpoint = UdpEndpoint{};
    const std::size_t colon = text.find(':');
    if (colon == std::string::npos || text.find(':', colon + 1) != std::string::npos) {
        set_error(error, "expected numeric IPv4:PORT");
        return false;
    }

    const std::string host = text.substr(0, colon);
    const std::string port_text = text.substr(colon + 1);
    if (host.empty() || port_text.empty()) {
        set_error(error, "expected numeric IPv4:PORT");
        return false;
    }

    in_addr address{};
    if (inet_pton(AF_INET, host.c_str(), &address) != 1) {
        set_error(error, "destination must be a numeric IPv4 address");
        return false;
    }
    const unsigned first_octet = ntohl(address.s_addr) >> 24;
    if (first_octet < 1 || first_octet > 223) {
        set_error(error, "destination first octet must be 1..223");
        return false;
    }

    for (char character : port_text) {
        if (!std::isdigit(static_cast<unsigned char>(character))) {
            set_error(error, "destination port must be 1..65535");
            return false;
        }
    }

    errno = 0;
    char *end = nullptr;
    const unsigned long port = std::strtoul(port_text.c_str(), &end, 10);
    if (errno || end == port_text.c_str() || *end || port < 1 || port > 65535) {
        set_error(error, "destination port must be 1..65535");
        return false;
    }

    endpoint.address = address.s_addr;
    endpoint.port = static_cast<std::uint16_t>(port);
    return true;
}

std::uint32_t crc32_iso_hdlc(const std::uint8_t *data, std::size_t size) noexcept {
    std::uint32_t crc = 0xffffffffu;
    for (std::size_t index = 0; index < size; ++index) {
        crc ^= data[index];
        for (unsigned bit = 0; bit < 8; ++bit) {
            const std::uint32_t mask = 0u - (crc & 1u);
            crc = (crc >> 1) ^ (kCrcPolynomial & mask);
        }
    }
    return crc ^ 0xffffffffu;
}

bool encode_imu_datagram(const Sample &sample, std::uint64_t sequence,
                         std::uint64_t send_timestamp_ns, std::uint32_t flags,
                         std::array<std::uint8_t, kImuDatagramSize> &packet) noexcept {
    packet.fill(0);
    if (!valid_sample(sample)) return false;

    std::array<std::uint8_t, kImuDatagramSize> encoded{};
    std::memcpy(encoded.data(), "OIMU", 4);
    encoded[4] = kImuWireVersion;
    encoded[5] = kImuWireType;
    put_u16(encoded.data() + 6, static_cast<std::uint16_t>(kImuDatagramSize));
    put_u64(encoded.data() + 8, sequence);
    put_u64(encoded.data() + 16,
            static_cast<std::uint64_t>(sample.accel_timestamp_ns));
    put_u64(encoded.data() + 24,
            static_cast<std::uint64_t>(sample.gyro_timestamp_ns));
    put_u64(encoded.data() + 32, send_timestamp_ns);
    put_u32(encoded.data() + 40, flags);

    for (unsigned axis = 0; axis < 3; ++axis) {
        put_u32(encoded.data() + 44 + axis * 4,
                float_bits(static_cast<float>(sample.accel[axis])));
    }
    for (unsigned axis = 0; axis < 3; ++axis) {
        put_u32(encoded.data() + 56 + axis * 4,
                float_bits(static_cast<float>(sample.gyro[axis])));
    }

    put_u32(encoded.data() + 68, crc32_iso_hdlc(encoded.data(), 68));
    packet = encoded;
    return true;
}

UdpSender::UdpSender(const UdpEndpoint &endpoint) {
    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0) {
        throw std::runtime_error(std::string("cannot create UDP socket: ") +
                                 std::strerror(errno));
    }

    if (fcntl(fd_, F_SETFD, FD_CLOEXEC) < 0) {
        const int saved_errno = errno;
        ::close(fd_);
        fd_ = -1;
        throw std::runtime_error(std::string("cannot set UDP close-on-exec: ") +
                                 std::strerror(saved_errno));
    }

    const int flags = fcntl(fd_, F_GETFL, 0);
    if (flags < 0 || fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
        const int saved_errno = errno;
        ::close(fd_);
        fd_ = -1;
        throw std::runtime_error(std::string("cannot set UDP nonblocking mode: ") +
                                 std::strerror(saved_errno));
    }

    sockaddr_in destination{};
    destination.sin_family = AF_INET;
    destination.sin_port = htons(endpoint.port);
    destination.sin_addr.s_addr = endpoint.address;
    if (::connect(fd_, reinterpret_cast<sockaddr *>(&destination),
                  sizeof(destination)) < 0) {
        const int saved_errno = errno;
        ::close(fd_);
        fd_ = -1;
        throw std::runtime_error(std::string("cannot connect UDP socket: ") +
                                 std::strerror(saved_errno));
    }
}

UdpSender::~UdpSender() {
    if (fd_ >= 0) ::close(fd_);
}

ssize_t UdpSender::send(const std::uint8_t *data, std::size_t size) noexcept {
    if (fd_ < 0) {
        errno = EBADF;
        return -1;
    }
    return ::send(fd_, data, size, MSG_DONTWAIT);
}

UdpPublisher::UdpPublisher(const UdpEndpoint &endpoint)
    : transport_(std::make_unique<UdpSender>(endpoint)) {}

UdpPublisher::UdpPublisher(std::unique_ptr<DatagramTransport> transport)
    : transport_(std::move(transport)) {
    if (!transport_) throw std::invalid_argument("UDP transport is null");
}

void UdpPublisher::publish(const Sample &sample, std::uint64_t send_timestamp_ns) {
    const std::uint64_t sequence = metrics_.samples;
    ++metrics_.samples;

    std::array<std::uint8_t, kImuDatagramSize> packet{};
    if (!encode_imu_datagram(sample, sequence, send_timestamp_ns, 0, packet)) {
        ++metrics_.dropped;
        return;
    }

    try {
        const ssize_t result = transport_->send(packet.data(), packet.size());
        if (result == static_cast<ssize_t>(packet.size())) {
            ++metrics_.sent;
        } else {
            ++metrics_.dropped;
        }
    } catch (...) {
        ++metrics_.dropped;
    }
}

}
