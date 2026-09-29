#include "sink.hpp"
#include <cinttypes>
#include <utility>

namespace orbvi::nv12 {
FileSink::FileSink(std::string directory, bool discard)
    : directory_(std::move(directory)), discard_(discard) {}
FileSink::~FileSink() { close(); }

int FileSink::open() {
    if (csv_) return -1;
    const std::string path = directory_ + "/frame_sets.csv";
    csv_ = fopen(path.c_str(), "wx");
    if (!csv_) { perror(path.c_str()); return -1; }
    return fputs("group,group_timestamp_ns,max_skew_ns,cam0_id,cam0_timestamp_ns,cam1_id,cam1_timestamp_ns,cam2_id,cam2_timestamp_ns,cam3_id,cam3_timestamp_ns\n", csv_) < 0 || fflush(csv_) ? -1 : 0;
}

int FileSink::write(unsigned index, const FrameSet &set) {
    if (!csv_) return -1;
    for (unsigned port = 0; port < 4; ++port) {
        const Frame &frame = set.frames[port];
        if (!frame.y || !frame.uv || !frame.width || !frame.height ||
            (frame.width & 1) || (frame.height & 1) || frame.stride < frame.width ||
            frame.vstride < frame.height || frame.y_bytes < size_t(frame.stride) * frame.vstride ||
            frame.uv_bytes < size_t(frame.stride) * (frame.vstride / 2)) {
            std::fprintf(stderr, "camera%u: invalid NV12 planes or dimensions\n", port);
            return -1;
        }
    }
    if (!discard_) {
        for (unsigned port = 0; port < 4; ++port) {
            const std::string path = directory_ + "/camera" + std::to_string(port) + "_" +
                [index]() { char name[16]; snprintf(name, sizeof(name), "%06u", index); return std::string(name); }() + ".nv12";
            FILE *out = fopen(path.c_str(), "wbx");
            if (!out) { perror(path.c_str()); return -1; }
            const Frame &frame = set.frames[port];
            bool failed = fwrite(frame.y, 1, frame.y_bytes, out) != frame.y_bytes ||
                          fwrite(frame.uv, 1, frame.uv_bytes, out) != frame.uv_bytes;
            if (fclose(out)) failed = true;
            if (failed) { std::fprintf(stderr, "failed writing %s\n", path.c_str()); return -1; }
        }
    }
    if (fprintf(csv_, "%u,%" PRIu64 ",%" PRIu64, index, set.timestamp_ns, set.max_skew_ns) < 0)
        return -1;
    for (const Frame &frame : set.frames)
        if (fprintf(csv_, ",%u,%" PRIu64, frame.id, frame.timestamp_ns) < 0) return -1;
    return fputc('\n', csv_) == EOF || fflush(csv_) ? -1 : 0;
}

int FileSink::close() {
    if (!csv_) return 0;
    int result = fclose(csv_);
    csv_ = nullptr;
    return result;
}
}
