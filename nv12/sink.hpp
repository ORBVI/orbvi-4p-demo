#pragma once
#include "frame_set.hpp"
#include <cstdio>
#include <string>

namespace orbvi::nv12 {
class FileSink final : public Sink {
public:
    FileSink(std::string directory, bool discard);
    ~FileSink() override;
    int open() override;
    int write(unsigned index, const FrameSet &set) override;
    int close() override;
private:
    std::string directory_;
    bool discard_;
    FILE *csv_ = nullptr;
};
}
