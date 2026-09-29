#pragma once
#include "capture_session.hpp"
#include <cstdio>
#include <string>

namespace orbvi {
class FileSink final : public FrameSink {
public:
    FileSink(std::string directory, bool discard);
    ~FileSink() override;
    FileSink(const FileSink &) = delete;
    FileSink &operator=(const FileSink &) = delete;
    int open(unsigned port) override;
    int write(unsigned index, const FrameView &frame) override;
    int close() override;
private:
    std::string directory_;
    bool discard_;
    unsigned port_ = 0;
    FILE *csv_ = nullptr;
};
}
