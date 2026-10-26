#include "sensor_timing.hpp"
#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

static unsigned port, opened, closed, exposure_reads;
static int scenario;
extern "C" int __wrap_open(const char *path, int, ...) {
    char expected[32]; std::snprintf(expected, sizeof(expected), "/dev/i2c-%u", port < 2 ? 6 : 4);
    assert(!std::strcmp(path, expected)); ++opened; return 42;
}
extern "C" int __wrap_close(int fd) { assert(fd == 42); ++closed; return 0; }
extern "C" int __wrap_ioctl(int fd, unsigned long request, ...) {
    assert(fd == 42 && request == I2C_RDWR);
    va_list args; va_start(args, request); auto *data = va_arg(args, i2c_rdwr_ioctl_data *); va_end(args);
    // Every operation is a register-address write followed by a read. No register data writes.
    assert(data->nmsgs == 2 && data->msgs[0].len == 2);
    assert(data->msgs[0].addr == (port % 2 ? 0x32 : 0x30));
    assert(data->msgs[1].flags == I2C_M_RD && data->msgs[1].len == 1);
    const auto &msg = data->msgs[0]; unsigned reg = (unsigned(msg.buf[0]) << 8) | msg.buf[1];
    unsigned result = 0;
    if (reg == 0x3107) result = 1;
    else if (reg == 0x3108) result = scenario == 1 ? 0x33 : 0x32;
    else {
        assert(reg >= 0x3e00 && reg <= 0x3e02);
        if (scenario == 2) { errno = EIO; return -1; }
        unsigned index = exposure_reads / 3;
        unsigned code = 0xf3461; // Full 20-bit exposure code, including fractional rows.
        if (scenario == 3) code += index % 2;
        if (scenario == 4 && index == 0) ++code;
        result = (code >> ((0x3e02 - reg) * 8)) & 0xff;
        ++exposure_reads;
    }
    data->msgs[1].buf[0] = static_cast<unsigned char>(result); return 2;
}
int main() {
    for (port = 0; port < 4; ++port) {
        for (scenario = 0; scenario <= 4; ++scenario) {
            opened = closed = exposure_reads = 0; unsigned code = 123;
            int rc = orbvi::read_sensor_exposure(port, code);
            assert(opened == 1 && closed == 1);
            if (scenario == 1) { assert(rc == -ENODEV && exposure_reads == 0 && code == 123); }
            else if (scenario == 2) assert(rc == -EIO && code == 123);
            else if (scenario == 3) assert(rc == -EAGAIN && exposure_reads == 18 && code == 123);
            else { assert(rc == 0 && code == 0xf3461); assert(exposure_reads == (scenario == 4 ? 12u : 6u)); }
        }
    }
    opened = closed = 0; unsigned code = 0;
    assert(orbvi::read_sensor_exposure(4, code) == -EINVAL && !opened && !closed);
}
