#include "sensor_timing.hpp"
#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <map>
#include <vector>
static std::map<unsigned, unsigned> regs;
static std::vector<unsigned> writes;
static unsigned opens, closes;
static int scenario;
extern "C" int __wrap_open(const char *path, int, ...) {
    assert(!std::strcmp(path, "/dev/i2c-6")); ++opens; return 42;
}
extern "C" int __wrap_close(int fd) { assert(fd == 42); ++closes; return 0; }
extern "C" int __wrap_ioctl(int fd, unsigned long request, ...) {
    assert(fd == 42 && request == I2C_RDWR);
    va_list args; va_start(args, request);
    auto *data = va_arg(args, i2c_rdwr_ioctl_data *); va_end(args);
    auto &message = data->msgs[0]; assert(message.addr == 0x30);
    unsigned reg = (unsigned(message.buf[0]) << 8) | message.buf[1];
    if (data->nmsgs == 2) {
        assert(message.len == 2 && data->msgs[1].flags == I2C_M_RD && data->msgs[1].len == 1);
        data->msgs[1].buf[0] = regs[reg]; return 2;
    }
    assert(data->nmsgs == 1 && message.len == 3 && reg == 0x3231);
    if (scenario == 3) { errno = EIO; return -1; }
    writes.push_back(reg); regs[reg] = message.buf[2];
    if (scenario == 4) regs[reg] ^= 0x20;
    if (scenario == 5) regs[0x3e01] ^= 1;
    return 1;
}
int main() {
    for (scenario = 0; scenario < 7; ++scenario) {
        regs = {{0x3107,1},{0x3108,0x32},{0x320c,2},{0x320d,0xee},
                {0x320e,5},{0x320f,0x78},{0x3222,2},{0x3231,0x88},
                {0x3e00,0},{0x3e01,0x32},{0x3e02,0x80},
                {0x3e06,0},{0x3e07,0x80},{0x3e08,0x27},{0x3e09,0x22},
                {0x3260,5},{0x3261,0x36},{0x3228,0},{0x3229,2},{0x3226,8}};
        if (scenario == 1) regs[0x3108] = 0x33;
        if (scenario == 2) regs[0x3222] = 0;
        if (scenario == 6) regs[0x3231] = (regs[0x3231] | 0x20);
        const auto before = regs;
        writes.clear(); opens = closes = 0;
        const int rc = orbvi::configure_sensor_trigger_readout(0);
        assert(rc == ((scenario == 0 || scenario == 6) ? 0 : -1));
        assert(opens == closes);
        if (scenario == 0 || scenario == 6) {
            auto expected = before; expected[0x3231] = (expected[0x3231] | 0x20);
            assert(regs == expected);
        }
        if (scenario == 0) assert((writes == std::vector<unsigned>{0x3231}));
        if (scenario == 1 || scenario == 2 || scenario == 3 || scenario == 6) assert(writes.empty());
    }
}
