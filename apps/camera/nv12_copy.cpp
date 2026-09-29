#include "encoder.hpp"
#include <cstring>

namespace orbvi::camera {
bool copy_nv12(const Nv12View &source, unsigned char *target, size_t capacity) {
    if (!source.y || !source.uv || !target || !source.width || !source.height ||
        (source.width & 1) || (source.height & 1) || source.stride < source.width ||
        source.vstride < source.height || capacity < size_t(source.width) * source.height * 3 / 2)
        return false;
    for (unsigned row = 0; row < source.height; ++row)
        std::memcpy(target + size_t(row) * source.width, source.y + size_t(row) * source.stride, source.width);
    unsigned char *chroma = target + size_t(source.width) * source.height;
    for (unsigned row = 0; row < source.height / 2; ++row)
        std::memcpy(chroma + size_t(row) * source.width, source.uv + size_t(row) * source.stride, source.width);
    return true;
}
}
