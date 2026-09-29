#pragma once
#include "camera_options.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace orbvi::camera {
class RtspServer {
public:
    RtspServer(unsigned port, std::string path, Codec codec, std::string record_directory = {});
    ~RtspServer();
    int open();
    int push(const void *data, size_t size, uint64_t timestamp_ns);
    int close();
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
