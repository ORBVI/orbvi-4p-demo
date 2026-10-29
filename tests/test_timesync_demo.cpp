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
    assert(parse_protocol("AUTO") == Protocol::AUTO);
    assert(parse_protocol("ZDA") == Protocol::ZDA);
    assert(parse_protocol("RMC") == Protocol::RMC);
    for (const auto &name : {"", "zda", "rmc", "auto", "ZDA|RMC"}) assert(!parse_protocol(name));
    assert(std::string(protocol_name(Protocol::AUTO)) == "AUTO");
    assert(std::string(protocol_name(Protocol::ZDA)) == "ZDA");
    assert(std::string(protocol_name(Protocol::RMC)) == "RMC");
    const auto zda = rmc("GPZDA,000000.00,01,01,2025,00,00");
    assert(zda == "$GPZDA,000000.00,01,01,2025,00,00*63");
    assert(parse_zda(zda) == start && parse_zda(zda + "\r\n") == start);
    assert(parse_sentence(zda) == start && parse_sentence(good) == start);
    assert(!parse_sentence(zda + "junk") && !parse_sentence(good + "junk"));
    assert(!parse_sentence(rmc("GPZDA,000000.00,29,02,2025,00,00")));
    assert(parse_sentence(zda, Protocol::ZDA) == start);
    assert(parse_sentence(good, Protocol::RMC) == start);
    assert(!parse_sentence(zda, Protocol::RMC) && !parse_sentence(good, Protocol::ZDA));
    assert(parse_zda(rmc("GPZDA,000000.00,01,01,2000,00,00")) == 946684800LL);
    assert(parse_zda(rmc("GPZDA,235959.00,31,12,2099,00,00")) == 4102444799LL);
    assert(parse_zda(rmc("GPZDA,235959.00,29,02,2024,00,00")) == 1709251199LL);
    assert(parse_zda(rmc("GPZDA,000000.00,29,02,2000,00,00")));
    for (const auto &body : {
        "GPZDA,000000.00,29,02,2025,00,00", "GPZDA,000000.00,31,04,2025,00,00",
        "GPZDA,000000.00,00,01,2025,00,00", "GPZDA,000000.00,01,13,2025,00,00",
        "GPZDA,000000.00,01,00,2025,00,00", "GPZDA,000000.00,32,01,2025,00,00",
        "GPZDA,000000.00,01,01,1999,00,00", "GPZDA,000000.00,01,01,2100,00,00",
        "GPZDA,000000.00,1,01,2025,00,00", "GPZDA,000000.00,01,1,2025,00,00",
        "GPZDA,000000.00,01,01,025,00,00", "GPZDA,000000.00,01,01,20a5,00,00",
        "GPZDA,000000.00,01,01,2025,08,00", "GPZDA,000000.00,01,01,2025,00,30",
        "GPZDA,000000.00,01,01,2025,,", "GPZDA,000000.00,01,01,2025,00,00,",
        "GPZDA,000000.00,01,01,2025,00", "GNZDA,000000.00,01,01,2025,00,00"})
        assert(!parse_zda(rmc(body)));
    for (const auto &time : {"240000.00", "236000.00", "235960.00", "000000.01", "000000.0", "000000.000", "a00000.00"})
        assert(!parse_zda(rmc(std::string("GPZDA,") + time + ",01,01,2025,00,00")));
    for (const auto &bad : {"", "$", "$GPZDA,*00", "$GPZDA,000000.00,01,01,2025,00,00*00"}) assert(!parse_zda(bad));
    for (const auto &tail : {"junk", "\r", "\n", "\r\njunk", "\r\n\r\n"}) assert(!parse_zda(zda + tail));
    assert(!parse_zda(zda.substr(1)) && !parse_zda(zda.substr(0, zda.size() - 1)));
    // Both selected wire formats must drive the same PPS lock and midnight rollover.
    for (const auto protocol : {Protocol::ZDA, Protocol::RMC}) {
        Pairer paired;
        const auto base = Steady::time_point(std::chrono::seconds(10));
        for (int i = 0; i < 3; ++i) {
            const std::string wire = protocol == Protocol::ZDA
                ? rmc(i == 0 ? "GPZDA,235959.00,31,12,2024,00,00" :
                      "GPZDA,00000" + std::to_string(i - 1) + ".00,01,01,2025,00,00")
                : sentence(i == 0 ? "235959.00" : "00000" + std::to_string(i - 1) + ".00", i == 0 ? "311224" : "010125");
            assert(!paired.on_pps({static_cast<uint32_t>(i + 1), i * 1000000000LL, base + std::chrono::seconds(i)}));
            auto matched = paired.on_time(*parse_sentence(wire, protocol), base + std::chrono::seconds(i) + std::chrono::milliseconds(20));
            assert(matched && matched->consecutive == static_cast<unsigned>(i + 1) && matched->utc_seconds == *start - 1 + i);
        }
    }
    // The default receiver must accept either sender without reconfiguration.
    // Repeated same-second sentences do not create a second matched PPS.
    Pairer compatible;
    const auto epoch = Steady::time_point(std::chrono::seconds(10));
    for (int i = 0; i < 4; ++i) {
        const auto received = epoch + std::chrono::seconds(i);
        assert(!compatible.on_pps({static_cast<uint32_t>(i + 1), i * 1000000000LL, received}));
        const auto wire = i % 2 == 0
            ? rmc("GPZDA,00000" + std::to_string(i) + ".00,01,01,2025,00,00")
            : sentence("00000" + std::to_string(i) + ".00", "010125");
        auto matched = compatible.on_time(*parse_sentence(wire), received + std::chrono::milliseconds(20));
        assert(matched && matched->utc_seconds == *start + i && matched->consecutive == static_cast<unsigned>(i + 1));
        assert(!compatible.on_time(*parse_sentence(wire), received + std::chrono::milliseconds(30)));
        assert(compatible.consecutive() == static_cast<unsigned>(i + 1));
    }
    auto t = Steady::time_point(std::chrono::seconds(20));
    Pairer p;
    assert(!p.on_time(*start, t)); // orphan UART before the first observed PPS
    assert(!p.on_pps({10, 100000000000LL, t}));
    auto first = p.on_time(*start, t + std::chrono::milliseconds(120));
    assert(first && first->consecutive == 1 && first->sequence == 10);
    assert(target_now_ns(*first, 100500000000LL) == *start * 1000000000LL + 500000000LL);
    assert(!target_now_ns(*first, 103000000000LL));
    assert(!p.on_pps({11, 101000000000LL, t + std::chrono::seconds(1)}));
    auto second = p.on_time(*start + 1, t + std::chrono::milliseconds(1110));
    assert(second && second->consecutive == 2);
    assert(!p.on_pps({12, 102000000000LL, t + std::chrono::seconds(2)}));
    auto locked = p.on_time(*start + 2, t + std::chrono::milliseconds(2110));
    assert(locked && locked->consecutive == 3);
    p.tick(t + std::chrono::seconds(5));
    assert(p.consecutive() == 0); // PPS/serial timeout loses lock
    assert(!p.on_pps({15, 105000000000LL, t + std::chrono::seconds(5)}));
    auto after_loss = p.on_time(*start + 5, t + std::chrono::milliseconds(5010));
    assert(after_loss && after_loss->consecutive == 1);
    auto late = p.on_pps({16, 106000000000LL, t + std::chrono::seconds(6)});
    assert(!late);
    assert(!p.on_time(*start + 6, t + std::chrono::milliseconds(6900))); // too late
    auto next = p.on_pps({17, 107000000000LL, t + std::chrono::seconds(7)});
    assert(!next);
    auto restart = p.on_time(*start + 7, t + std::chrono::milliseconds(7080));
    assert(restart && restart->consecutive == 1);
    Pairer reordered;
    assert(!reordered.on_pps({5, 5000000000LL, t}));
    assert(reordered.on_time(*start, t + std::chrono::milliseconds(20)));
    assert(!reordered.on_time(*start + 1, t + std::chrono::milliseconds(1020)));
    auto recovered = reordered.on_pps({6, 6000000000LL, t + std::chrono::milliseconds(1040)});
    assert(recovered && recovered->consecutive == 2);
    // A mismatched UTC second must drop lock even with contiguous PPS sequence numbers.
    Pairer mismatch;
    assert(!mismatch.on_pps({20, 20000000000LL, t}));
    assert(mismatch.on_time(*start, t + std::chrono::milliseconds(100))->consecutive == 1);
    assert(!mismatch.on_pps({21, 21000000000LL, t + std::chrono::seconds(1)}));
    assert(mismatch.on_time(*start + 2, t + std::chrono::milliseconds(1100))->consecutive == 1);
    assert(!mismatch.on_pps({23, 23000000000LL, t + std::chrono::seconds(2)}));
    assert(mismatch.on_time(*start + 3, t + std::chrono::milliseconds(2100))->consecutive == 1);
    std::puts("PASS: ZDA/RMC selection, date/checksum/no-fix fields, PPS match/reorder/timeout/mismatch, UTC interpolation");
}
