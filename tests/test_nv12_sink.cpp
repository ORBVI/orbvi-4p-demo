#include "sink.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>

using namespace orbvi::nv12;
int main() {
    char path[] = "/tmp/orbvi-nv12-XXXXXX";
    assert(mkdtemp(path));
    std::filesystem::path root(path);
    const unsigned char y[]{1, 2, 3, 4}, uv[]{5, 6};
    FrameSet set{};
    set.timestamp_ns = 1000;
    set.max_skew_ns = 3;
    for (unsigned i = 0; i < 4; ++i) {
        Frame &f = set.frames[i];
        f.y = y; f.uv = uv; f.y_bytes = 4; f.uv_bytes = 2;
        f.id = i + 10; f.width = 2; f.height = 2;
        f.stride = 2; f.vstride = 2; f.timestamp_ns = 1000 + i;
    }
    FileSink sink(path, false);
    assert(sink.open() == 0);
    assert(sink.write(0, set) == 0);
    assert(sink.write(0, set) != 0); // Never overwrite a published frame.
    assert(sink.close() == 0);
    for (unsigned i = 0; i < 4; ++i) {
        const auto file = root / ("camera" + std::to_string(i) + "_000000.nv12");
        std::ifstream stream(file, std::ios::binary);
        std::string content{std::istreambuf_iterator<char>(stream), {}};
        assert(content == std::string("\1\2\3\4\5\6", 6));
    }
    std::ifstream csv(root / "frame_sets.csv");
    std::string header, row;
    std::getline(csv, header); std::getline(csv, row);
    assert(header == "group,group_timestamp_ns,max_skew_ns,cam0_id,cam0_timestamp_ns,cam1_id,cam1_timestamp_ns,cam2_id,cam2_timestamp_ns,cam3_id,cam3_timestamp_ns");
    assert(row == "0,1000,3,10,1000,11,1001,12,1002,13,1003");
    std::filesystem::remove_all(root);
}
