#include "validation.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace orbvi::validation;
template<class F> void rejects(F fn) {
    bool caught = false;
    try { fn(); } catch (const std::exception &) { caught = true; }
    assert(caught);
}
int main() {
    auto layout = make_layout({{"timestamp", "le:s64/64>>0", 2},
                               {"x", "le:s16/16>>0", 0}, {"y", "be:u16/16>>0", 1}});
    assert(layout.size == 16 && layout.fields[2].offset == 8);
    const uint8_t data[] = {0xfe,0xff,0x12,0x34,0,0,0,0,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01};
    auto row = layout.decode(data);
    assert(row.at("x") == -2 && row.at("y") == 0x1234);
    assert(row.at("timestamp") == 0x0102030405060708LL);
    rejects([] { make_layout({{"x", "le:u7/7>>0", 0}}); });
    rejects([] { make_layout({{"x", "le:s16/16>>1", 0}}); });
    rejects([] { make_layout({{"x", "le:s128/128>>0", 0}}); });
    rejects([] { make_layout({{"x", "le:s16/16>>0", 0}, {"y", "le:s16/16>>0", 0}}); });
    rejects([] { make_layout({}); });
    std::vector<Row> rows;
    for (int64_t i = 0; i < 1000; ++i) rows.push_back({{"in_timestamp", i*1000000}});
    auto rate = sampling(rows, 0, 1000000000);
    assert(rate.samples == 1000 && rate.hz == 1000 && rate.max_gap == 1000000);
    auto missing = rows; missing.erase(missing.begin()+500);
    rejects([&] { sampling(missing, 0, 1000000000); });
    auto reversed = rows; reversed[500]["in_timestamp"] = 499000000;
    rejects([&] { sampling(reversed, 0, 1000000000); });
    rejects([&] { sampling(rows, 0, 2000000000); });
    rejects([&] { sampling({}, 0, 1000000000); });
    for (int code : {130, 1}) {
        int drains = 0;
        std::vector<std::string> events;
        auto run = [&] { stop_capture([&]{events.push_back("signal");},
            [&]{return drains >= 4 ? code : -1;}, [&]{++drains;},
            [&]{events.push_back("disable");}, [&]{events.push_back("snapshot");}); };
        if (code == 130) run(); else rejects(run);
        assert(drains == 4 && events == std::vector<std::string>({"signal", "disable", "snapshot"}));
    }
    bool disabled = false;
    rejects([&] { stop_capture([]{}, []{return -1;}, []{}, [&]{disabled=true;}, []{}, 0); });
    assert(disabled);
    disabled = false;
    rejects([&] { stop_capture([]{}, []{return -1;}, []{throw std::runtime_error("read");},
                               [&]{disabled=true;}, []{}); });
    assert(disabled);
    std::vector<int> cleanup;
    auto errors = cleanup_all({{"disable", [&]{cleanup.push_back(1); throw std::runtime_error("fail");}},
                               {"restore", [&]{cleanup.push_back(2);}},
                               {"diagnostics", [&]{cleanup.push_back(3); throw std::runtime_error("disk");}}});
    assert(cleanup == std::vector<int>({1,2,3}) && errors.size() == 2);
    std::cout << "PASS: IIO layout, sampling continuity, draining shutdown and best-effort cleanup\n";
}
