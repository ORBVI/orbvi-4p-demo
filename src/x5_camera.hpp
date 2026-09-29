#pragma once
#include "capture_session.hpp"
#include <memory>

namespace orbvi {
// SDK types and SC132GS wiring are private to this backend.
class X5Camera final : public CameraSource {
public:
    explicit X5Camera(unsigned port);
    ~X5Camera() override;
    int prepare(bool external) override;
    int start() override;
    int acquire(FrameView &frame, unsigned timeout_ms) override;
    int release() override;
    int close() override;
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
