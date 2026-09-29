#pragma once
#include "frame_set.hpp"
#include <memory>

namespace orbvi::nv12 {
class X5Camera final : public Source {
public:
    explicit X5Camera(unsigned port, unsigned output_fps = 60,
                      unsigned output_width = 1088, unsigned output_height = 1280);
    ~X5Camera() override;
    int prepare() override;
    int start() override;
    int acquire(Frame &frame, unsigned timeout_ms) override;
    int release() override;
    int close() override;
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
