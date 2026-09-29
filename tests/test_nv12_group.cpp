#include "frame_set.hpp"
#include <cassert>
#include <stdexcept>
#include <vector>

using namespace orbvi::nv12;

struct Camera : Source {
    std::vector<Frame> frames;
    size_t next = 0;
    unsigned releases = 0;
    bool leased = false;
    bool fail_stop = false;
    int prepare() override { return 0; }
    int start() override { return 0; }
    int acquire(Frame &frame, unsigned) override {
        if (leased || next >= frames.size()) return -1;
        frame = frames[next++]; leased = true; return 0;
    }
    int release() override { assert(leased); leased = false; ++releases; return 0; }
    int close() override { assert(!leased); return fail_stop ? -1 : 0; }
};

struct Writer : Sink {
    unsigned writes = 0;
    bool fail = false;
    int open() override { return 0; }
    int write(unsigned index, const FrameSet &set) override {
        assert(index == writes++);
        assert(set.timestamp_ns == set.frames[0].timestamp_ns);
        assert(set.max_skew_ns == 300);
        if (fail) throw std::runtime_error("write failure");
        return 0;
    }
    int close() override { return 0; }
};

int main() {
    unsigned char y[4]{}, uv[2]{};
    for (int scenario = 0; scenario < 3; ++scenario) {
        Camera a, b, c, d;
        Camera *cameras[]{&a, &b, &c, &d};
        for (unsigned i = 0; i < 4; ++i) {
            Frame frame{};
            frame.y = y; frame.uv = uv; frame.y_bytes = 4; frame.uv_bytes = 2;
            frame.width = 2; frame.height = 2; frame.stride = 2;
            frame.timestamp_ns = 2000000 + i * 100;
            cameras[i]->frames.push_back(frame);
        }
        if (scenario == 1) {
            a.frames[0].timestamp_ns = 1000000;
            Frame next = a.frames[0]; next.timestamp_ns = 2000000;
            a.frames.push_back(next);
        }
        Writer writer;
        writer.fail = scenario == 2;
        volatile sig_atomic_t interrupted = 0;
        Session session({&a, &b, &c, &d}, writer, interrupted, 1, 1000, 20);
        assert(session.run() == (writer.fail ? 1 : 0));
        assert(writer.writes == 1);
        assert(a.releases == (scenario == 1 ? 2u : 1u));
        for (auto *camera : {&b, &c, &d}) assert(camera->releases == 1);
        assert(session.run() == 1); // A session may not be reused.
    }
}
