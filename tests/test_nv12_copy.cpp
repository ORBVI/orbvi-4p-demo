#include "encoder.hpp"
#include <cassert>
#include <vector>

using namespace orbvi::camera;
int main() {
    const unsigned char y[]{1,2,9,9, 3,4,9,9};
    const unsigned char uv[]{5,6,9,9};
    unsigned char output[6]{};
    Nv12View input{y, uv, 2, 2, 4, 2};
    assert(copy_nv12(input, output, sizeof(output)));
    assert(std::vector<unsigned char>(output, output + 6) ==
           std::vector<unsigned char>({1,2,3,4,5,6}));
    assert(!copy_nv12(input, output, 5));
}
