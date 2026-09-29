#include "time_sync.hpp"
#include <limits>
#include <vector>

namespace orbvi::timesync {
namespace {
int number(const std::string &s, size_t start, size_t digits) {
    if (start + digits > s.size()) return -1;
    int result = 0;
    for (size_t i = start; i < start + digits; ++i) {
        if (s[i] < '0' || s[i] > '9') return -1;
        result = result * 10 + s[i] - '0';
    }
    return result;
}
bool leap(int y) { return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0); }
int days_in_month(int y, int m) {
    static constexpr int days[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return m == 2 ? days[m] + leap(y) : days[m];
}
// Civil date -> days since 1970-01-01 (Gregorian calendar).
int64_t days_since_epoch(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    int era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = static_cast<unsigned>(y - era * 400);
    unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<int64_t>(era) * 146097 + doe - 719468;
}
int hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
} // namespace
std::optional<int64_t> parse_rmc(const std::string &line) {
    // The UART reader removes CRLF; also accept an intact CRLF sentence for callers.
    const size_t end = line.size() >= 2 && line.compare(line.size() - 2, 2, "\r\n") == 0
                           ? line.size() - 2 : line.size();
    if (end > 96 || end < 10 || line[0] != '$') return std::nullopt;
    const auto star = line.find('*');
    if (star == std::string::npos || star + 3 != end) return std::nullopt;
    const int hi = hex(line[star + 1]), lo = hex(line[star + 2]);
    if (hi < 0 || lo < 0) return std::nullopt;
    unsigned checksum = 0;
    for (size_t i = 1; i < star; ++i) checksum ^= static_cast<unsigned char>(line[i]);
    if (checksum != static_cast<unsigned>(hi * 16 + lo)) return std::nullopt;
    if (line.compare(0, 7, "$GPRMC,") != 0) return std::nullopt;
    std::vector<std::string> fields;
    size_t start = 7; // after "$GPRMC,"
    for (size_t i = start; i <= star; ++i) {
        if (i == star || line[i] == ',') { fields.push_back(line.substr(start, i - start)); start = i + 1; }
    }
    // time,status,lat,NS,lon,EW,speed,course,date,magvar,magvarEW,mode.
    // V and N explicitly mean no fix: this is only a simulated clock/PPS experiment.
    if (fields.size() != 12 || fields[0].size() != 9 || fields[0][6] != '.' ||
        number(fields[0], 7, 2) != 0 || fields[1] != "V" || fields[2] != "" ||
        fields[3] != "" || fields[4] != "" || fields[5] != "" ||
        fields[6] != "" || fields[7] != "" || fields[8].size() != 6 ||
        fields[9] != "" || fields[10] != "" || fields[11] != "N") return std::nullopt;
    const int h = number(fields[0], 0, 2), mi = number(fields[0], 2, 2), sec = number(fields[0], 4, 2);
    const int d = number(fields[8], 0, 2), mo = number(fields[8], 2, 2), yy = number(fields[8], 4, 2);
    if (yy < 0 || mo < 1 || mo > 12 || d < 1 || d > days_in_month(2000 + yy, mo) ||
        h < 0 || h > 23 || mi < 0 || mi > 59 || sec < 0 || sec > 59) return std::nullopt;
    return days_since_epoch(2000 + yy, static_cast<unsigned>(mo), static_cast<unsigned>(d)) * 86400 + h * 3600 + mi * 60 + sec;
}
std::optional<int64_t> target_now_ns(const Match &match, int64_t realtime_now_ns) {
    const auto age = realtime_now_ns - match.edge_realtime_ns;
    if (age < 0 || age > 2000000000LL || match.utc_seconds > std::numeric_limits<int64_t>::max() / 1000000000LL)
        return std::nullopt; // stale PPS or another process stepped the clock
    return match.utc_seconds * 1000000000LL + age;
}
void Pairer::tick(Steady::time_point now) {
    if (last_received_ != Steady::time_point{} && now - last_received_ > std::chrono::milliseconds(2200)) {
        consecutive_ = 0; pending_edge_.reset(); early_rmc_.reset();
        last_received_ = now;
    }
    if (pending_edge_ && now - pending_edge_->received > std::chrono::milliseconds(850)) {
        pending_edge_.reset(); consecutive_ = 0;
    }
    if (early_rmc_ && now - early_rmc_->second > std::chrono::milliseconds(200)) early_rmc_.reset();
}
std::optional<Match> Pairer::on_pps(Edge edge) {
    tick(edge.received);
    if (last_sequence_ && edge.sequence != last_sequence_ + 1) consecutive_ = 0;
    if (last_sequence_ && edge.sequence == last_sequence_) return std::nullopt;
    if (pending_edge_) consecutive_ = 0; // previous pulse had no corresponding RMC
    last_sequence_ = edge.sequence;
    last_received_ = edge.received;
    pending_edge_ = edge;
    if (early_rmc_ && edge.received >= early_rmc_->second &&
        edge.received - early_rmc_->second <= std::chrono::milliseconds(150)) {
        auto queued = *early_rmc_; early_rmc_.reset();
        return match(queued.first, queued.second);
    }
    early_rmc_.reset();
    return std::nullopt;
}
std::optional<Match> Pairer::match(int64_t utc_seconds, Steady::time_point received) {
    if (!pending_edge_ || received + std::chrono::milliseconds(150) < pending_edge_->received ||
        received - pending_edge_->received > std::chrono::milliseconds(850)) return std::nullopt;
    if (consecutive_ && utc_seconds != last_utc_ + 1) consecutive_ = 0;
    ++consecutive_;
    last_utc_ = utc_seconds;
    last_received_ = received;
    Match result{pending_edge_->sequence, utc_seconds, pending_edge_->realtime_ns, consecutive_};
    pending_edge_.reset();
    return result;
}
std::optional<Match> Pairer::on_rmc(int64_t utc_seconds, Steady::time_point received) {
    const bool missed_previous_edge = pending_edge_ && received - pending_edge_->received > std::chrono::milliseconds(850);
    tick(received);
    if (missed_previous_edge) return std::nullopt;
    if (pending_edge_) return match(utc_seconds, received);
    // A UART read can beat PPS_FETCH after the same edge; keep only one brief candidate.
    if (last_sequence_ && !early_rmc_) early_rmc_ = std::make_pair(utc_seconds, received);
    return std::nullopt;
}
} // namespace orbvi::timesync
