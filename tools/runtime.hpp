#pragma once
#include "validation.hpp"
#include <filesystem>
#include <memory>
#include <variant>
#include <type_traits>
#include <ostream>
#include <csignal>
namespace orbvi::validation {
namespace fs = std::filesystem;
struct Json {
    using Object = std::map<std::string, Json>;
    using Array = std::vector<Json>;
    std::variant<std::nullptr_t, bool, int64_t, double, std::string, Object, Array> value = Object{};
    Json() = default;
    Json(bool x): value(x) {}
    template<class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T,bool>, int> = 0>
    Json(T x): value(int64_t(x)) {}
    Json(double x): value(x) {}
    Json(const char *x): value(std::string(x)) {}
    Json(std::string x): value(std::move(x)) {}
    Json(Object x): value(std::move(x)) {}
    static Json array() { Json j; j.value = Array{}; return j; }
    Json &operator[](const std::string &key) { return std::get<Object>(value)[key]; }
    void push(const Json &x) { std::get<Array>(value).push_back(x); }
    void write(std::ostream &) const;
};
Json to_json(const Row &row);
Json to_json(const Sampling &result);
std::string read_text(const fs::path &);
void write_text(const fs::path &, const std::string &);
void write_json(const fs::path &, const Json &);
fs::path temp_directory(const fs::path &, const std::string &);
void check_board();
void check_interrupted();
void sleep_until(int64_t deadline);
extern volatile sig_atomic_t interrupted;
class Process {
    int pid_ = -1, result_ = -1;
public:
    Process() = default;
    Process(const Process &) = delete;
    Process &operator=(const Process &) = delete;
    ~Process();
    void start(const std::vector<std::string> &, const fs::path &log, const std::string &preload = "");
    int poll();
    void signal(int);
    int wait(int64_t timeout, const std::function<void()> &tick = {});
    void close();
};
using CsvRow = std::map<std::string, std::string>;
std::vector<CsvRow> read_csv(const fs::path &);
void verify_raw(const fs::path &, const CsvRow &);
class Iio {
public:
    struct Device {
        fs::path path;
        std::string name, id;
        int fd = -1;
        bool paused = false, saved = false;
        Layout layout;
        std::map<fs::path, std::string> settings;
        std::vector<uint8_t> pending;
        std::vector<Row> rows;
    };
    std::vector<Device> devices;
    Json configuration;
    Iio() = default;
    Iio(const Iio &) = delete;
    Iio &operator=(const Iio &) = delete;
    ~Iio();
    void open(const fs::path &sys = "/sys/bus/iio/devices", const fs::path &dev = "/dev");
    void drain(int timeout_ms);
    void disable();
    std::vector<std::string> close();
    Device &device(const std::string &name);
    Json samples() const;
};
using Stats = std::map<std::string, std::string>;
Stats snapshot();
Json to_json(const Stats &);
void check_counters(const Stats &, const Stats &, bool include_drops = true);
}
