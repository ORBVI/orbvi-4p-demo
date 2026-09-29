#include "jpeg_codec.hpp"
#include "nv12_planes.hpp"
#include "protocol.hpp"
#include "x5_camera.hpp"
#include "hb_mem_mgr.h"
#include <arpa/inet.h>
#include <poll.h>
#include <sys/file.h>
#include <fcntl.h>
#include <chrono>
#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include <atomic>
#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <thread>
#include <vector>

namespace ref = orbvi::reference_images;
namespace {
volatile sig_atomic_t interrupted = 0;
void stop(int) { interrupted = 1; }
struct Options {
    bool jpeg = false;
    unsigned mask = 15, count = 10, port = 5610, wait_ms = 10000;
};
bool number(const char *text, unsigned &value) {
    if (!text || !*text || *text == '-') return false;
    char *end = nullptr;
    errno = 0;
    const unsigned long parsed = std::strtoul(text, &end, 0);
    if (errno || *end || parsed > UINT_MAX) return false;
    value = static_cast<unsigned>(parsed);
    return true;
}
bool parse(int argc, char **argv, Options &o) {
    bool mode = false;
    for (int i = 1; i < argc; ++i) {
        if (i + 1 == argc) return false;
        const std::string key = argv[i++];
        if (key == "--mode") {
            if (mode) return false;
            mode = true;
            if (!std::strcmp(argv[i], "jpeg")) o.jpeg = true;
            else if (std::strcmp(argv[i], "nv12")) return false;
        } else if (key == "--camera-mask") {
            if (!number(argv[i], o.mask)) return false;
        } else if (key == "--frames-per-camera") {
            if (!number(argv[i], o.count)) return false;
        } else if (key == "--base-port") {
            if (!number(argv[i], o.port)) return false;
        } else if (key == "--wait-timeout-ms") {
            if (!number(argv[i], o.wait_ms)) return false;
        } else return false;
    }
    return mode && o.mask && !(o.mask & ~15u) && o.count && o.wait_ms &&
           o.port >= 1 && o.port <= 65532;
}
#ifndef ORBVI_REFERENCE_LOCK_PATH
#define ORBVI_REFERENCE_LOCK_PATH "/run/lock/orbvi-reference-camera-owner.lock"
#endif
constexpr const char *kOwnerLock = ORBVI_REFERENCE_LOCK_PATH;
struct Fd {
    int fd = -1;
    explicit Fd(int value = -1) : fd(value) {}
    ~Fd() { if (fd >= 0) ::close(fd); }
    Fd(const Fd &) = delete;
    Fd &operator=(const Fd &) = delete;
};
// One monotonic deadline per operation: a trickling peer cannot keep a camera alive.
using Clock = std::chrono::steady_clock;
using Deadline = Clock::time_point;
bool ready(int fd, short events, Deadline deadline) {
    while (!interrupted) {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now()).count();
        if (remaining <= 0) return false;
        pollfd p{fd, events, 0};
        const int r = poll(&p, 1, static_cast<int>(remaining < 100 ? remaining : 100));
        if (r > 0) return (p.revents & events) && !(p.revents & (POLLERR | POLLHUP | POLLNVAL));
        if (r < 0 && errno != EINTR) return false;
    }
    return false;
}
bool read_exact(int fd, void *data, std::size_t length, Deadline deadline) {
    auto *p = static_cast<std::uint8_t *>(data);
    while (length && ready(fd, POLLIN, deadline)) {
        const ssize_t n = recv(fd, p, length, MSG_DONTWAIT);
        if (n > 0) { p += n; length -= static_cast<std::size_t>(n); }
        else if (n == 0) return false;
        else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) return false;
    }
    return length == 0;
}
bool connected(int fd) {
    char c;
    const ssize_t n = recv(fd, &c, 1, MSG_PEEK | MSG_DONTWAIT);
    return n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK);
}
bool send_exact(int fd, const void *data, std::size_t length, Deadline deadline) {
    const auto *p = static_cast<const std::uint8_t *>(data);
    while (length && ready(fd, POLLOUT, deadline)) {
        const ssize_t n = send(fd, p, length, MSG_NOSIGNAL | MSG_DONTWAIT);
        if (n > 0) { p += n; length -= static_cast<std::size_t>(n); }
        else if (n == 0) return false;
        else if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) return false;
    }
    return length == 0;
}
int run_camera(unsigned camera, const Options &o) {
    Fd listener(socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0));
    if (listener.fd < 0) { std::perror("socket"); return 1; }
    int reuse = 1;
    setsockopt(listener.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<std::uint16_t>(o.port + camera));
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener.fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) ||
        listen(listener.fd, 1)) { std::perror("bind/listen"); return 1; }
    std::fprintf(stderr, "camera%u: awaiting %s START on port %u\n", camera,
                 o.jpeg ? "jpeg" : "nv12", o.port + camera);
    if (!ready(listener.fd, POLLIN, Clock::now() + std::chrono::milliseconds(o.wait_ms))) {
        std::fprintf(stderr, "camera%u: START wait timed out\n", camera); return 1;
    }
    Fd peer(accept4(listener.fd, nullptr, nullptr, SOCK_CLOEXEC));
    if (peer.fd < 0) { std::perror("accept"); return 1; }
    std::array<std::uint8_t, ref::kControlSize> control{};
    ref::Control parsed;
    std::string error;
    if (!read_exact(peer.fd, control.data(), control.size(),
                    Clock::now() + std::chrono::milliseconds(o.wait_ms)) ||
        !ref::decode_start(control.data(), control.size(), parsed, error) || parsed.camera != camera) {
        std::fprintf(stderr, "camera%u: invalid START (%s)\n", camera, error.c_str()); return 1;
    }
    // Camera and codec are not created until a valid START; JPEG never copies/encodes idle frames.
    orbvi::nv12::X5Camera source(camera);
    ref::JpegCodec codec(camera);
    int result = 1;
    unsigned delivered = 0;
    bool codec_opened = false;
    if (source.prepare()) goto done;
    if (source.start()) goto done;
    while (!interrupted && delivered < o.count) {
        if (!connected(peer.fd)) { std::fprintf(stderr, "camera%u: subscriber disconnected\n", camera); break; }
        orbvi::nv12::Frame frame;
        const int acquire_rc = source.acquire(frame, 1000);
        if (acquire_rc) {
            std::fprintf(stderr, "camera%u: acquire failed rc=%d (0x%08x)\n", camera,
                         acquire_rc, static_cast<unsigned>(acquire_rc)); break;
        }
        bool ok = ref::valid_source(frame);
        if (!ok) std::fprintf(stderr,
            "camera%u: invalid source NV12 %ux%u stride=%u vstride=%u y_bytes=%zu uv_bytes=%zu\n",
            camera, frame.width, frame.height, frame.stride, frame.vstride,
            frame.y_bytes, frame.uv_bytes);
        std::vector<std::uint8_t> payload;
        if (ok && o.jpeg && !connected(peer.fd)) {
            std::fprintf(stderr, "camera%u: disconnected after acquire; no JPEG work\n", camera);
            ok = false;
        }
        if (ok) {
            if (o.jpeg) {
                if (!codec_opened) {
                    ok = codec.open(frame.width, frame.height);
                    codec_opened = ok;
                }
                if (ok && connected(peer.fd)) ok = codec.encode(frame, payload);
                else {
                    if (ok) std::fprintf(stderr, "camera%u: disconnected before JPEG encode\n", camera);
                    ok = false;
                }
            } else ok = ref::pack_nv12(frame, payload);
        }
        if (payload.size() > std::numeric_limits<std::uint32_t>::max()) ok = false;
        const int release_rc = source.release();
        if (release_rc) {
            std::fprintf(stderr, "camera%u: releaseframe failed rc=%d (0x%08x)\n", camera,
                         release_rc, static_cast<unsigned>(release_rc));
            ok = false;
        }
        if (!ok) { std::fprintf(stderr, "camera%u: capture/encode/release failed\n", camera); break; }
        ref::FrameHeader h;
        h.kind = o.jpeg ? ref::Kind::Jpeg : ref::Kind::Nv12;
        h.camera = camera; h.sequence = delivered + 1;
        h.timestamp_ns = frame.timestamp_ns;
        h.width = frame.width; h.height = frame.height;
        h.stride = o.jpeg ? 0 : frame.width;
        h.payload_size = static_cast<std::uint32_t>(payload.size());
        h.crc32 = ref::crc32_iso_hdlc(payload.data(), payload.size());
        std::array<std::uint8_t, ref::kFrameHeaderSize> bytes{};
        const auto send_deadline = Clock::now() + std::chrono::milliseconds(o.wait_ms);
        if (!connected(peer.fd) || !ref::encode_frame_header(h, bytes) ||
            !send_exact(peer.fd, bytes.data(), bytes.size(), send_deadline) ||
            !send_exact(peer.fd, payload.data(), payload.size(), send_deadline)) {
            std::fprintf(stderr, "camera%u: send failed\n", camera); break;
        }
        ++delivered;
    }
    if (delivered == o.count) result = 0;
done:
    if (!codec.close()) result = 1;
    if (source.close()) result = 1;
    std::fprintf(stderr, "camera%u: sent=%u/%u (TCP sends, host receipt unverified) %s\n",
                 camera, delivered, o.count, result ? "FAILED" : "OK");
    return result;
}
}
int main(int argc, char **argv) {
    Options o;
    if (!parse(argc, argv, o)) {
        std::fprintf(stderr, "usage: %s --mode nv12|jpeg [--camera-mask 1..15] [--frames-per-camera N] [--base-port 5610] [--wait-timeout-ms 10000]\n", argv[0]);
        return 2;
    }
    struct sigaction action{}; action.sa_handler = stop; sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, nullptr); sigaction(SIGTERM, &action, nullptr);
    // Enforce ownership even for direct invocation; legacy RTSP/UVC do not participate.
    Fd owner(::open(kOwnerLock, O_CREAT | O_RDWR | O_CLOEXEC, 0666));
    if (owner.fd < 0 || ::flock(owner.fd, LOCK_EX | LOCK_NB)) {
        std::perror("camera owner lock"); return 1;
    }
    if (hb_mem_module_open()) { std::fputs("hb_mem_module_open failed\n", stderr); return 1; }
    std::array<int, 4> results{};
    std::vector<std::thread> workers;
    for (unsigned camera = 0; camera < 4; ++camera)
        if (o.mask & (1u << camera)) workers.emplace_back([&, camera] { results[camera] = run_camera(camera, o); });
    for (auto &worker : workers) worker.join();
    int result = hb_mem_module_close() ? 1 : 0;
    for (unsigned camera = 0; camera < 4; ++camera)
        if ((o.mask & (1u << camera)) && results[camera]) result = 1;
    return result;
}
