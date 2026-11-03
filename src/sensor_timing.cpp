#include "sensor_timing.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace orbvi {
namespace {
struct SensorBus {
    int fd = -1;
    unsigned address = 0;
    ~SensorBus() { if (fd >= 0) ::close(fd); }
    int transfer(i2c_msg *messages, unsigned count) {
        i2c_rdwr_ioctl_data data{messages, count};
        int result;
        do { result = ioctl(fd, I2C_RDWR, &data); } while (result < 0 && errno == EINTR);
        if (result == static_cast<int>(count)) return 0;
        if (result >= 0) errno = EIO;
        return -1;
    }
    int read(unsigned reg, unsigned &value) {
        unsigned char bytes[]{static_cast<unsigned char>(reg >> 8), static_cast<unsigned char>(reg)};
        unsigned char result = 0;
        i2c_msg messages[]{
            {static_cast<__u16>(address), 0, 2, bytes},
            {static_cast<__u16>(address), I2C_M_RD, 1, &result}};
        if (transfer(messages, 2)) return -1;
        value = result;
        return 0;
    }
    int pair(unsigned reg, unsigned &value) {
        unsigned high, low;
        if (read(reg, high) || read(reg + 1, low)) return -1;
        value = (high << 8) | low;
        return 0;
    }
    int open(unsigned port) {
        if (port > 3) { errno = EINVAL; return -1; }
        // CAM1/2 share I2C6; CAM3/4 share I2C4. SID selects 0x30/0x32.
        char path[32]; std::snprintf(path, sizeof(path), "/dev/i2c-%u", port < 2 ? 6u : 4u);
        address = port % 2 ? 0x32 : 0x30;
        fd = ::open(path, O_RDWR | O_CLOEXEC);
        unsigned id = 0;
        if (fd < 0 || pair(0x3107, id)) return -1;
        if (id != 0x0132) { errno = ENODEV; return -1; }
        return 0;
    }
    int exposure(unsigned &code) {
        unsigned high, low;
        if (read(0x3e00, high) || pair(0x3e01, low)) return -1;
        code = ((high & 0x0f) << 16) | low;
        return 0;
    }
    int snapshot(SensorTiming &timing) {
        unsigned high = 0, low = 0;
        if (pair(0x320c, timing.hts) || pair(0x320e, timing.vts) ||
            read(0x3e00, high) || pair(0x3e01, low) ||
            pair(0x3e08, timing.analog_gain_code) || pair(0x3e06, timing.digital_gain_code) ||
            read(0x3222, timing.trigger) || read(0x3231, timing.trigger_mode)) return -1;
        timing.exposure_code = ((high & 0x0f) << 16) | low;
        return 0;
    }
};
}
int read_sensor_timing(unsigned port, SensorTiming &timing) {
    SensorBus bus;
    if (bus.open(port) || bus.snapshot(timing)) {
        std::fprintf(stderr, "camera%u: sensor timing read failed (errno=%d)\n", port, errno);
        return -1;
    }
    return 0;
}

int read_sensor_exposure(unsigned port, unsigned &code) {
    SensorBus bus;
    if (bus.open(port)) return -errno;
    for (unsigned attempt = 0; attempt < 3; ++attempt) {
        unsigned before, after;
        if (bus.exposure(before) || bus.exposure(after)) return -errno;
        if (before == after) { code = after; return 0; }
    }
    return -EAGAIN;
}

int configure_sensor_trigger_readout(unsigned port) {
    SensorBus bus;
    SensorTiming before, after;
    if (bus.open(port) || bus.snapshot(before)) return -1;
    if (!(before.trigger & 0x02)) { errno = EINVAL; return -1; }
    const unsigned target = before.trigger_mode | 0x20;
    if (target != before.trigger_mode) {
        unsigned char bytes[]{0x32, 0x31, static_cast<unsigned char>(target)};
        i2c_msg message{static_cast<__u16>(bus.address), 0, sizeof(bytes), bytes};
        if (bus.transfer(&message, 1)) return -1;
    }
    if (bus.snapshot(after)) return -1;
    if (after.trigger_mode != target || after.trigger != before.trigger ||
        after.hts != before.hts || after.vts != before.vts ||
        after.exposure_code != before.exposure_code ||
        after.analog_gain_code != before.analog_gain_code ||
        after.digital_gain_code != before.digital_gain_code) {
        std::fprintf(stderr, "camera%u: trigger-mode verification failed\n", port);
        errno = EIO;
        return -1;
    }
    std::printf("camera%u: trigger_readout reg3231=0x%02x->0x%02x "
                "VTS=%u exposure_code=0x%05x again=0x%04x dgain=0x%04x unchanged\n",
                port, before.trigger_mode, after.trigger_mode, after.vts,
                after.exposure_code, after.analog_gain_code, after.digital_gain_code);
    std::fflush(stdout);
    return 0;
}

int configure_sensor_vts(unsigned port, unsigned vts) {
    if (vts < 1288 || vts > 65535) { errno = EINVAL; return -1; }
    SensorBus bus;
    SensorTiming before, after;
    if (bus.open(port) || bus.snapshot(before)) return -1;
    unsigned char bytes[]{0x32, 0x0e, static_cast<unsigned char>(vts >> 8), static_cast<unsigned char>(vts)};
    i2c_msg message{static_cast<__u16>(bus.address), 0, sizeof(bytes), bytes};
    int result = bus.transfer(&message, 1);
    if (!result) result = bus.snapshot(after);
    if (!result && after.vts != vts) { errno = EIO; result = -1; }
    if (result) {
        std::fprintf(stderr, "camera%u: VTS write/readback failed (errno=%d)\n", port, errno);
        return -1;
    }
    std::printf("camera%u: VTS before=0x%04x after=0x%04x (%u) HTS=0x%04x "
                "exposure_code_before=0x%05x exposure_code_after=0x%05x again=0x%04x dgain=0x%04x\n",
                port, before.vts, after.vts, after.vts, after.hts, before.exposure_code,
                after.exposure_code, after.analog_gain_code, after.digital_gain_code);
    std::fflush(stdout);
    return 0;
}
}
