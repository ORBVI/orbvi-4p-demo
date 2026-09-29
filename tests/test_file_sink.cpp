#include "file_sink.hpp"
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <type_traits>

static_assert(!std::is_copy_constructible_v<orbvi::FileSink>);

int main() {
    char directory[] = "/tmp/orbvi-sink-XXXXXX";
    assert(mkdtemp(directory));
    const std::filesystem::path root(directory);
    const unsigned char pixels[] = {1, 3, 5, 7};
    orbvi::FrameView frame{pixels, sizeof(pixels), 42, 2, 1, 4, 100, 200, 300};
    {
        orbvi::FileSink sink(directory, false);
        assert(sink.write(0, frame) != 0);
        assert(sink.open(2) == 0);
        assert(sink.write(0, frame) == 0);
        assert(sink.write(0, frame) != 0); // Existing RAW must not be truncated.
        assert(sink.close() == 0);
        assert(sink.close() == 0);
        assert(sink.open(2) != 0); // Existing CSV must not be truncated.
    }
    std::ifstream raw(root / "camera2_000000.raw", std::ios::binary);
    std::string bytes{std::istreambuf_iterator<char>(raw), {}};
    assert(bytes == std::string(reinterpret_cast<const char *>(pixels), sizeof(pixels)));
    std::ifstream csv(root / "camera2.csv");
    std::string header, row, extra;
    std::getline(csv, header); std::getline(csv, row); std::getline(csv, extra);
    assert(row == "0,42,100,200,300,2,1,4,4,camera2_000000.raw");
    assert(extra.empty());
    {
        orbvi::FileSink sink(directory, true);
        assert(sink.open(0) == 0);
        assert(sink.write(0, frame) == 0);
        frame.data = nullptr;
        assert(sink.write(1, frame) != 0);
        assert(sink.close() == 0);
        assert(!std::filesystem::exists(root / "camera0_000000.raw"));
    }
    std::filesystem::remove_all(root);
}
