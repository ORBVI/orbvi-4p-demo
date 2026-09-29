#include "bag_session.hpp"
#include "rosbag/rosbag_v2_writer.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace orbvi::sensor {
namespace {
using robobaton_demo::RosbagTime;
using robobaton_demo::RosbagTimeFromNs;
using robobaton_demo::RosbagV2Writer;

const char kCompressedImageDefinition[] =
    "std_msgs/Header header\nstring format\nuint8[] data\n"
    "================================================================================\n"
    "MSG: std_msgs/Header\nuint32 seq\ntime stamp\nstring frame_id\n";
const char kImuDefinition[] =
    "std_msgs/Header header\ngeometry_msgs/Quaternion orientation\n"
    "float64[9] orientation_covariance\ngeometry_msgs/Vector3 angular_velocity\n"
    "float64[9] angular_velocity_covariance\ngeometry_msgs/Vector3 linear_acceleration\n"
    "float64[9] linear_acceleration_covariance\n"
    "================================================================================\n"
    "MSG: std_msgs/Header\nuint32 seq\ntime stamp\nstring frame_id\n"
    "================================================================================\n"
    "MSG: geometry_msgs/Quaternion\nfloat64 x\nfloat64 y\nfloat64 z\nfloat64 w\n"
    "================================================================================\n"
    "MSG: geometry_msgs/Vector3\nfloat64 x\nfloat64 y\nfloat64 z\n";

void append_header(std::vector<uint8_t> &payload, uint32_t sequence, uint64_t timestamp_ns,
                   const std::string &frame_id) {
    robobaton_demo::AppendU32(&payload, sequence);
    robobaton_demo::AppendRosTime(&payload, RosbagTimeFromNs(timestamp_ns));
    robobaton_demo::AppendRosString(&payload, frame_id);
}

struct CameraRecord {
    uint64_t source_timestamp_ns = 0;
    uint64_t host_timestamp_ns = 0;
    std::vector<uint8_t> bytes;
};
class CameraReader {
public:
    explicit CameraReader(const std::string &path) : input_(path, std::ios::binary) {
        if (!input_) throw std::runtime_error("cannot open camera spool " + path);
    }
    bool next(CameraRecord &record) {
        uint32_t size = 0;
        input_.read(reinterpret_cast<char *>(&record.source_timestamp_ns), sizeof(record.source_timestamp_ns));
        if (input_.eof() && input_.gcount() == 0) return false;
        if (!input_) throw std::runtime_error("truncated camera spool timestamp");
        input_.read(reinterpret_cast<char *>(&record.host_timestamp_ns), sizeof(record.host_timestamp_ns));
        input_.read(reinterpret_cast<char *>(&size), sizeof(size));
        if (!input_ || size > 64U * 1024U * 1024U) throw std::runtime_error("invalid camera spool record");
        record.bytes.resize(size);
        input_.read(reinterpret_cast<char *>(record.bytes.data()), size);
        if (!input_) throw std::runtime_error("truncated camera spool payload");
        return true;
    }
private:
    std::ifstream input_;
};

struct ImuRecord {
    uint64_t sensor_timestamp_ns = 0;
    uint64_t host_timestamp_ns = 0;
    std::array<double, 3> accel{}, gyro{};
};
class ImuReader {
public:
    explicit ImuReader(const std::string &path) : input_(path) {
        if (!input_) throw std::runtime_error("cannot open IMU CSV " + path);
        std::string header; std::getline(input_, header);
    }
    bool next(ImuRecord &record) {
        std::string line;
        if (!std::getline(input_, line)) return false;
        std::istringstream stream(line); std::string field;
        std::array<std::string, 8> fields;
        for (auto &item : fields) {
            if (!std::getline(stream, item, ',')) throw std::runtime_error("invalid IMU CSV row");
        }
        if (std::getline(stream, field, ',')) throw std::runtime_error("invalid IMU CSV column count");
        record.sensor_timestamp_ns = std::stoull(fields[0]);
        record.host_timestamp_ns = std::stoull(fields[1]);
        for (unsigned i = 0; i < 3; ++i) record.accel[i] = std::stod(fields[2 + i]);
        for (unsigned i = 0; i < 3; ++i) record.gyro[i] = std::stod(fields[5 + i]);
        return true;
    }
private:
    std::ifstream input_;
};

std::vector<uint8_t> image_payload(uint32_t sequence, const CameraRecord &record, unsigned camera) {
    std::vector<uint8_t> payload;
    payload.reserve(record.bytes.size() + 64);
    append_header(payload, sequence, record.host_timestamp_ns,
                  "camera" + std::to_string(camera) + "_optical_frame");
    robobaton_demo::AppendRosString(&payload, "h264");
    if (record.bytes.size() > std::numeric_limits<uint32_t>::max())
        throw std::runtime_error("H.264 access unit too large");
    robobaton_demo::AppendU32(&payload, static_cast<uint32_t>(record.bytes.size()));
    payload.insert(payload.end(), record.bytes.begin(), record.bytes.end());
    return payload;
}

std::vector<uint8_t> imu_payload(uint32_t sequence, const ImuRecord &record) {
    std::vector<uint8_t> payload;
    append_header(payload, sequence, record.host_timestamp_ns, "orbvi_imu_link");
    for (double value : {0.0, 0.0, 0.0, 1.0}) robobaton_demo::AppendF64(&payload, value);
    for (unsigned i = 0; i < 9; ++i) robobaton_demo::AppendF64(&payload, i == 0 ? -1.0 : 0.0);
    for (double value : record.gyro) robobaton_demo::AppendF64(&payload, value);
    for (unsigned i = 0; i < 9; ++i) robobaton_demo::AppendF64(&payload, 0.0);
    for (double value : record.accel) robobaton_demo::AppendF64(&payload, value);
    for (unsigned i = 0; i < 9; ++i) robobaton_demo::AppendF64(&payload, 0.0);
    return payload;
}
}

int write_rosbag(const std::string &bag_path, const std::string &spool_directory,
                 unsigned camera_mask, const std::string &imu_csv_path) {
    RosbagV2Writer writer;
    try {
        writer.Open(bag_path);
        std::array<uint32_t, 4> connections{};
        std::array<std::unique_ptr<CameraReader>, 4> readers;
        std::array<CameraRecord, 4> records;
        std::array<bool, 4> ready{};
        std::array<uint32_t, 4> sequences{};
        for (unsigned camera = 0; camera < 4; ++camera) {
            if (!(camera_mask & (1u << camera))) continue;
            connections[camera] = writer.AddConnection(
                "/camera" + std::to_string(camera) + "/image/compressed",
                "sensor_msgs/CompressedImage", "8f7a12909da2c9d3332d540a0977563f",
                kCompressedImageDefinition);
            readers[camera] = std::make_unique<CameraReader>(
                spool_directory + "/camera" + std::to_string(camera) + ".h264spool");
            ready[camera] = readers[camera]->next(records[camera]);
        }
        const uint32_t imu_connection = writer.AddConnection(
            "/imu/data", "sensor_msgs/Imu", "6a62c6daae103f4ff57a132d6f95cec2", kImuDefinition);
        ImuReader imu_reader(imu_csv_path); ImuRecord imu_record;
        bool imu_ready = imu_reader.next(imu_record); uint32_t imu_sequence = 0;
        while (true) {
            int selected = -1; uint64_t selected_time = std::numeric_limits<uint64_t>::max();
            for (unsigned camera = 0; camera < 4; ++camera) {
                if (ready[camera] && records[camera].host_timestamp_ns < selected_time) {
                    selected = static_cast<int>(camera); selected_time = records[camera].host_timestamp_ns;
                }
            }
            if (imu_ready && imu_record.host_timestamp_ns < selected_time) {
                selected = 4; selected_time = imu_record.host_timestamp_ns;
            }
            if (selected < 0) break;
            if (selected == 4) {
                writer.WriteMessage(imu_connection, RosbagTimeFromNs(selected_time),
                                    imu_payload(imu_sequence++, imu_record));
                imu_ready = imu_reader.next(imu_record);
            } else {
                const unsigned camera = static_cast<unsigned>(selected);
                writer.WriteMessage(connections[camera], RosbagTimeFromNs(selected_time),
                                    image_payload(sequences[camera]++, records[camera], camera));
                ready[camera] = readers[camera]->next(records[camera]);
            }
        }
        const auto finished = writer.Finish(true);
        return finished.cleanup_complete && finished.data_complete ? 0 : 1;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "sensor_demo: rosbag: %s\n", error.what());
        writer.Abort();
        return 1;
    }
}
}
