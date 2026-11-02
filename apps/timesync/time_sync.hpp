#pragma once
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace orbvi::timesync {
using Steady = std::chrono::steady_clock;
struct Edge { uint32_t sequence; int64_t realtime_ns; Steady::time_point received; };
struct Match { uint32_t sequence; int64_t utc_seconds; int64_t edge_realtime_ns; unsigned consecutive; };
enum class Protocol { AUTO, ZDA, RMC };
std::optional<Protocol> parse_protocol(const std::string &name);
const char *protocol_name(Protocol protocol);
std::optional<int64_t> parse_zda(const std::string &line);
std::optional<int64_t> parse_rmc(const std::string &line);
std::optional<int64_t> parse_sentence(const std::string &line, Protocol protocol = Protocol::AUTO);
// Return the current UTC estimate, extrapolating from the kernel's PPS CLOCK_REALTIME timestamp.
std::optional<int64_t> target_now_ns(const Match &match, int64_t realtime_now_ns);
class Pairer {
public:
    std::optional<Match> on_pps(Edge edge);
    std::optional<Match> on_time(int64_t utc_seconds, Steady::time_point received);
    void tick(Steady::time_point now);
    unsigned consecutive() const { return consecutive_; }
private:
    std::optional<Match> match(int64_t utc_seconds, Steady::time_point received);
    std::optional<Edge> pending_edge_;
    std::optional<std::pair<int64_t, Steady::time_point>> early_time_;
    uint32_t last_sequence_ = 0;
    int64_t last_utc_ = 0;
    unsigned consecutive_ = 0;
    Steady::time_point last_received_{};
};
}
