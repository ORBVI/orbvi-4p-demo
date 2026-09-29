#include "capture_session.hpp"
#include <cassert>
#include <vector>
#include <stdexcept>

using namespace orbvi;
struct FakeCamera final : CameraSource {
    unsigned port;
    std::vector<unsigned> &events;
    int prepare_error = 0, close_error = 0, acquire_error = 0;
    unsigned releases = 0;
    FakeCamera(unsigned p, std::vector<unsigned> &e) : port(p), events(e) {}
    int prepare(bool) override { events.push_back(port); return prepare_error; }
    int start() override { events.push_back(10 + port); return 0; }
    int acquire(FrameView &f, unsigned) override { f.data = this; f.bytes = 1; return acquire_error; }
    int release() override { ++releases; return 0; }
    int close() override { events.push_back(20 + port); return close_error; }
};
struct FakeSink final : FrameSink {
    bool throws = false;
    unsigned writes = 0;
    int open(unsigned) override { return 0; }
    int write(unsigned, const FrameView &) override {
        ++writes;
        if (throws) throw std::runtime_error("injected output failure");
        return 0;
    }
    int close() override { return 0; }
};
int main() {
    for (unsigned scenario = 0; scenario < 6; ++scenario) {
        std::vector<unsigned> events;
        FakeCamera a(0, events), b(1, events), c(2, events), d(3, events);
        FakeSink sink;
        options config; config.external = true; config.mask = 1; config.frames = 2; config.timeout_ms = 1;
        volatile sig_atomic_t interrupted = 0;
        if (scenario == 1) c.prepare_error = -1;
        if (scenario == 2) sink.throws = true;
        if (scenario == 3) { interrupted = 1; a.close_error = -1; }
        if (scenario == 4) a.acquire_error = -1;
        if (scenario == 5) interrupted = 1;
        CaptureSession session(config, {&a, &b, &c, &d}, {&sink, nullptr, nullptr, nullptr}, interrupted);
        int result = session.run();
        assert(result == (scenario == 5 ? 130 : scenario ? 1 : 0));
        if (!scenario || scenario == 2) {
            assert((std::vector<unsigned>(events.begin(), events.begin() + 5) == std::vector<unsigned>{0,1,2,3,10}));
            assert(a.releases == (scenario == 2 ? 1 : 2));
        }
        if (scenario == 4) assert(a.releases == 0);
        if (scenario == 1) assert(a.releases == 0);
        assert((std::vector<unsigned>(events.end() - 4, events.end()) == std::vector<unsigned>{23,22,21,20}));
    }
}
