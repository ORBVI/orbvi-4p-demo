#include "time_sync.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>
using namespace orbvi::timesync;

static std::string rmc(const std::string &body) {
    unsigned checksum = 0;
    for (unsigned char ch : body) checksum ^= ch;
    char suffix[8]; std::snprintf(suffix, sizeof(suffix), "*%02X", checksum);
    return "$" + body + suffix;
}
static std::string sentence(const std::string &time, const std::string &date) {
    return rmc("GPRMC," + time + ",V,,,,,,," + date + ",,,N");
}
int main() {
    const auto good = sentence("000000.00", "010125");
    assert(good == "$GPRMC,000000.00,V,,,,,,,010125,,,N*7A");
    auto start = parse_rmc(good);
    assert(start && *start == 1735689600LL);
    assert(parse_rmc(good + "\r\n") == start);
    assert(parse_rmc(sentence("000000.00", "010100")) == 946684800LL);
    assert(parse_rmc(sentence("235959.00", "311299")) == 4102444799LL);
    assert(parse_rmc(sentence("235959.00", "290224")) == 1709251199LL);
    assert(!parse_rmc(sentence("235959.00", "290225")));
    assert(parse_rmc(sentence("000000.00", "290200"))); // 2000 is a leap year
    assert(!parse_rmc(sentence("000000.00", "290201"))); // 2001 is not
    for (const auto &date : {"000125", "320125", "310425", "010025", "011325", "0101a5", "01012", "0101250"})
        assert(!parse_rmc(sentence("000000.00", date)));
    for (const auto &time : {"240000.00", "236000.00", "235960.00", "000000.01", "000000.0", "000000.000", "a00000.00", "00000a.00", "000000x00"})
        assert(!parse_rmc(sentence(time, "010125")));
    assert(!parse_rmc(rmc("GPZDA,000000.00,01,01,2025,00,00")));
    assert(!parse_rmc(rmc("GNRMC,000000.00,V,,,,,,,010125,,,N")));
    // Exhaustively mutate each non-time/date field in the defined 12-field order.
    const std::array<std::string, 12> fields{{"000000.00", "V", "", "", "", "", "", "", "010125", "", "", "N"}};
    const std::array<std::string, 12> wrong{{"", "A", "4916.45", "N", "12311.12", "W", "0.0", "0.0", "", "0.0", "E", "A"}};
    for (size_t change = 0; change < fields.size(); ++change) {
        auto changed = fields;
        changed[change] = wrong[change];
        std::string body = "GPRMC";
        for (const auto &field : changed) body += "," + field;
        assert(!parse_rmc(rmc(body)));
    }
    assert(!parse_rmc(rmc("GPRMC,000000.00,V,,,,,,010125,,,N"))); // missing empty course
    assert(!parse_rmc(rmc("GPRMC,000000.00,V,,,,,,,,010125,,,N"))); // extra empty field
    assert(!parse_rmc(rmc("GPRMC,000000.00,V,,,,,,,010125,,,N,")));
    assert(!parse_rmc(rmc("GPRMC,000000.00,V,,,,,,,010125,,N,"))); // shifted mode
    assert(!parse_rmc(good.substr(1)));
    assert(!parse_rmc(good.substr(0, good.size() - 1)));
    auto bad_checksum = good; bad_checksum.back() = bad_checksum.back() == '0' ? '1' : '0';
    assert(!parse_rmc(bad_checksum));
    bad_checksum = good; bad_checksum.back() = 'G';
    assert(!parse_rmc(bad_checksum));
    assert(!parse_rmc("$GPRMC,000000.00,V,,,,,,,010125,,,N*00"));
    assert(!parse_rmc(good + "junk"));
    assert(!parse_rmc(good + "\r"));
    assert(!parse_rmc(good + "\n"));
    assert(!parse_rmc(good + "\r\njunk"));
    assert(!parse_rmc(good + "\r\n\r\n"));
    assert(!parse_rmc(good + std::string(100, 'x')));
    auto t = Steady::time_point(std::chrono::seconds(20));
    Pairer p;
    assert(!p.on_rmc(*start, t)); // orphan UART before the first observed PPS
    assert(!p.on_pps({10, 100000000000LL, t}));
    auto first = p.on_rmc(*start, t + std::chrono::milliseconds(120));
    assert(first && first->consecutive == 1 && first->sequence == 10);
    assert(target_now_ns(*first, 100500000000LL) == *start * 1000000000LL + 500000000LL);
    assert(!target_now_ns(*first, 103000000000LL));
    assert(!p.on_pps({11, 101000000000LL, t + std::chrono::seconds(1)}));
    auto second = p.on_rmc(*start + 1, t + std::chrono::milliseconds(1110));
    assert(second && second->consecutive == 2);
    assert(!p.on_pps({12, 102000000000LL, t + std::chrono::seconds(2)}));
    auto locked = p.on_rmc(*start + 2, t + std::chrono::milliseconds(2110));
    assert(locked && locked->consecutive == 3);
    p.tick(t + std::chrono::seconds(5));
    assert(p.consecutive() == 0); // PPS/serial timeout loses lock
    assert(!p.on_pps({15, 105000000000LL, t + std::chrono::seconds(5)}));
    auto after_loss = p.on_rmc(*start + 5, t + std::chrono::milliseconds(5010));
    assert(after_loss && after_loss->consecutive == 1);
    auto late = p.on_pps({16, 106000000000LL, t + std::chrono::seconds(6)});
    assert(!late);
    assert(!p.on_rmc(*start + 6, t + std::chrono::milliseconds(6900))); // too late
    auto next = p.on_pps({17, 107000000000LL, t + std::chrono::seconds(7)});
    assert(!next);
    auto restart = p.on_rmc(*start + 7, t + std::chrono::milliseconds(7080));
    assert(restart && restart->consecutive == 1);
    Pairer reordered;
    assert(!reordered.on_pps({5, 5000000000LL, t}));
    assert(reordered.on_rmc(*start, t + std::chrono::milliseconds(20)));
    assert(!reordered.on_rmc(*start + 1, t + std::chrono::milliseconds(1020)));
    auto recovered = reordered.on_pps({6, 6000000000LL, t + std::chrono::milliseconds(1040)});
    assert(recovered && recovered->consecutive == 2);
    // A mismatched UTC second must drop lock even with contiguous PPS sequence numbers.
    Pairer mismatch;
    assert(!mismatch.on_pps({20, 20000000000LL, t}));
    assert(mismatch.on_rmc(*start, t + std::chrono::milliseconds(100))->consecutive == 1);
    assert(!mismatch.on_pps({21, 21000000000LL, t + std::chrono::seconds(1)}));
    assert(mismatch.on_rmc(*start + 2, t + std::chrono::milliseconds(1100))->consecutive == 1);
    assert(!mismatch.on_pps({23, 23000000000LL, t + std::chrono::seconds(2)}));
    assert(mismatch.on_rmc(*start + 3, t + std::chrono::milliseconds(2100))->consecutive == 1);
    std::puts("PASS: RMC no-fix fields/date/checksum, PPS match/reorder/timeout/mismatch, UTC interpolation");
}
