#include "nv12_planes.hpp"
#include "protocol.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <sys/file.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ORBVI_REFERENCE_LOCK_PATH "/tmp/orbvi-reference-board-host-test.lock"
#define main reference_board_main
#include "../apps/reference_images/main.cpp"
#undef main

namespace r = orbvi::reference_images;
namespace {
std::atomic<unsigned> opens{0}, prepares{0}, acquires{0}, encodes{0};
int unused_port() {
    Fd sock(::socket(AF_INET, SOCK_STREAM, 0)); assert(sock.fd >= 0);
    sockaddr_in addr{}; addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(::bind(sock.fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0);
    socklen_t len = sizeof(addr);
    assert(::getsockname(sock.fd, reinterpret_cast<sockaddr *>(&addr), &len) == 0);
    return ntohs(addr.sin_port);
}
int connect_to(int port) {
    for (unsigned i = 0; i < 100; ++i) {
        const int fd = ::socket(AF_INET, SOCK_STREAM, 0); assert(fd >= 0);
        sockaddr_in addr{}; addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(port));
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::connect(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == 0) return fd;
        ::close(fd);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    assert(false); return -1;
}
void session_tests() {
    Options o; o.jpeg = true; o.mask = 1; o.count = 2;
    o.port = static_cast<unsigned>(unused_port()); o.wait_ms = 400;
    int result = 0;
    std::thread invalid([&] { result = run_camera(0, o); });
    int peer = connect_to(o.port);
    std::array<uint8_t, r::kControlSize> control{};
    assert(r::encode_start(0, control)); control[7] = 1;
    assert(::send(peer, control.data(), control.size(), 0) == static_cast<ssize_t>(control.size()));
    ::close(peer); invalid.join();
    assert(result == 1 && prepares == 0 && opens == 0 && encodes == 0);
    o.port = static_cast<unsigned>(unused_port());
    std::thread disconnected([&] { result = run_camera(0, o); });
    peer = connect_to(o.port);
    control[7] = 0;
    assert(::send(peer, control.data(), control.size(), 0) == static_cast<ssize_t>(control.size()));
    for (unsigned i = 0; i < 300 && acquires == 0; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    assert(acquires == 1);
    ::shutdown(peer, SHUT_RDWR); ::close(peer);
    disconnected.join();
    assert(result == 1 && prepares == 1 && opens == 0 && encodes == 0);
    // The binary itself enforces the lock, before opening the memory module.
    Fd lock(::open(kOwnerLock, O_CREAT | O_RDWR | O_CLOEXEC, 0600));
    assert(lock.fd >= 0 && ::flock(lock.fd, LOCK_EX | LOCK_NB) == 0);
    char arg0[] = "reference_images_demo", arg1[] = "--mode", arg2[] = "jpeg";
    char *args[] = {arg0, arg1, arg2};
    assert(reference_board_main(3, args) == 1);
    assert(opens == 0);
    ::flock(lock.fd, LOCK_UN);
    ::unlink(kOwnerLock);
    // A slow peer cannot extend a START deadline with intermittent bytes.
    int sockets[2]; assert(::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
    const auto begin = Clock::now();
    std::thread trickle([&] {
        for (int i = 0; i < 4; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(35));
            char byte = 'x'; (void)::send(sockets[1], &byte, 1, MSG_NOSIGNAL);
        }
    });
    char bytes[8]{};
    assert(!read_exact(sockets[0], bytes, sizeof(bytes), Clock::now() + std::chrono::milliseconds(80)));
    assert(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - begin).count() < 130);
    trickle.join(); ::close(sockets[0]); ::close(sockets[1]);
}
} // namespace

namespace orbvi::nv12 {
struct X5Camera::State {};
X5Camera::X5Camera(unsigned, unsigned, unsigned, unsigned) : state_(new State) {}
X5Camera::~X5Camera() = default;
int X5Camera::prepare() { ++prepares; return 0; }
int X5Camera::start() { return 0; }
int X5Camera::acquire(Frame &frame, unsigned) {
    ++acquires;
    std::this_thread::sleep_for(std::chrono::milliseconds(70));
    static std::array<uint8_t, 16> y{};
    static std::array<uint8_t, 8> uv{};
    frame.y = y.data(); frame.uv = uv.data();
    frame.y_bytes = y.size(); frame.uv_bytes = uv.size();
    frame.width = frame.height = 4; frame.stride = 4; frame.vstride = 4;
    frame.timestamp_ns = 42;
    return 0;
}
int X5Camera::release() { return 0; }
int X5Camera::close() { return 0; }
}
namespace orbvi::reference_images {
struct JpegCodec::State {};
JpegCodec::JpegCodec(unsigned) : state_(new State) {}
JpegCodec::~JpegCodec() = default;
bool JpegCodec::open(unsigned, unsigned) { return ++opens, true; }
bool JpegCodec::encode(const nv12::Frame &, std::vector<uint8_t> &) { ++encodes; return false; }
bool JpegCodec::close() { return true; }
}
extern "C" {
int32_t hb_mem_module_open(void) { ++opens; return 0; }
int32_t hb_mem_module_close(void) { return 0; }
}
int main() {
    std::array<std::uint8_t, 24> y{};
    std::array<std::uint8_t, 12> uv{};
    // 4x4 NV12, source stride 6, padded rows contain sentinels.
    y.fill(0xee); uv.fill(0xee);
    for (unsigned row = 0; row < 4; ++row)
        for (unsigned col = 0; col < 4; ++col) y[row * 6 + col] = row * 4 + col;
    for (unsigned row = 0; row < 2; ++row)
        for (unsigned col = 0; col < 4; ++col) uv[row * 6 + col] = 16 + row * 4 + col;
    orbvi::nv12::Frame f;
    f.y = y.data(); f.uv = uv.data(); f.y_bytes = y.size(); f.uv_bytes = uv.size();
    f.width = 4; f.height = 4; f.stride = 6; f.vstride = 4; f.timestamp_ns = 123;
    std::vector<std::uint8_t> tight;
    assert(r::pack_nv12(f, tight) && tight.size() == 24);
    for (unsigned i = 0; i < tight.size(); ++i) assert(tight[i] == i);
    // Codec targets are independent allocations with independent row strides.
    std::array<std::uint8_t, 32> target_y{};
    std::array<std::uint8_t, 14> target_uv{};
    target_y.fill(0xaa); target_uv.fill(0xbb);
    assert(r::copy_codec_nv12(f, target_y.data(), target_y.size(), 8,
                              target_uv.data(), target_uv.size(), 7));
    for (unsigned row = 0; row < 4; ++row)
        for (unsigned col = 0; col < 8; ++col)
            assert(target_y[row * 8 + col] == (col < 4 ? row * 4 + col : 0xaa));
    for (unsigned row = 0; row < 2; ++row)
        for (unsigned col = 0; col < 7; ++col)
            assert(target_uv[row * 7 + col] == (col < 4 ? 16 + row * 4 + col : 0xbb));
    // Check another codec allocation where Y and UV both have 8-byte row strides.
    std::array<std::uint8_t, 16> target_uv_same_stride{};
    target_uv_same_stride.fill(0xcc);
    assert(r::copy_codec_nv12(f, target_y.data(), target_y.size(), 8,
                              target_uv_same_stride.data(), target_uv_same_stride.size(), 8));
    for (unsigned row = 0; row < 2; ++row)
        for (unsigned col = 0; col < 8; ++col)
            assert(target_uv_same_stride[row * 8 + col] ==
                   (col < 4 ? 16 + row * 4 + col : 0xcc));
    assert(!r::copy_codec_nv12(f, target_y.data(), 27, 8, target_uv.data(), 14, 7));
    assert(!r::copy_codec_nv12(f, target_y.data(), 32, 8, target_uv.data(), 10, 7));
    f.uv_bytes = 9;
    assert(!r::pack_nv12(f, tight));
    f.uv_bytes = uv.size();
    r::FrameHeader h;
    h.kind = r::Kind::Nv12; h.camera = 2; h.sequence = 1; h.timestamp_ns = f.timestamp_ns;
    h.width = f.width; h.height = f.height; h.stride = f.width;
    h.payload_size = tight.size(); h.crc32 = r::crc32_iso_hdlc(tight.data(), tight.size());
    std::array<std::uint8_t, r::kFrameHeaderSize> wire{};
    assert(r::encode_frame_header(h, wire));
    r::FrameHeader decoded; std::string error;
    assert(r::decode_frame_header(wire.data(), wire.size(), decoded, error));
    assert(r::validate_payload(decoded, tight.data(), tight.size(), error));
    tight[0] ^= 1;
    assert(!r::validate_payload(decoded, tight.data(), tight.size(), error));
    std::array<std::uint8_t, r::kControlSize> start{};
    assert(r::encode_start(2, start));
    r::Control ctl;
    assert(r::decode_start(start.data(), start.size(), ctl, error) && ctl.camera == 2);
    start[7] = 1;
    assert(!r::decode_start(start.data(), start.size(), ctl, error));
    h.kind = r::Kind::Jpeg; h.stride = 0; h.payload_size = 5;
    assert(r::encode_frame_header(h, wire));
    h.stride = 4; assert(!r::encode_frame_header(h, wire));
    session_tests();
}
