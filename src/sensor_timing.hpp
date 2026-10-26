#pragma once
#include <cstdint>

namespace orbvi {
struct SensorTiming {
    unsigned hts = 0, vts = 0, exposure_code = 0;
    unsigned analog_gain_code = 0, digital_gain_code = 0;
    unsigned trigger = 0, trigger_mode = 0;
};
// ORBVI 4P connector wiring; call after SDK sensor initialization, before start.
int read_sensor_timing(unsigned port, SensorTiming &timing);
int configure_sensor_vts(unsigned port, unsigned vts);
// SC132GS datasheet V3.1 table 2-2: 0x3231 bit 5 selects readout triggering.
// Call before stream start. No exposure/AE writes.
int configure_sensor_trigger_readout(unsigned port);
}
