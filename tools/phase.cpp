#include "workflows.hpp"
#include <algorithm>
namespace orbvi::validation {
Json to_json(const Phase &p) {
    Json pulses=Json::array(); for(auto t:p.pulses) pulses.push(t);
    return Json::Object{{"name",p.name},{"start",p.start},{"end",p.end},{"pulses",std::move(pulses)}};
}
Json analyze_phase(const Phase &p, const std::array<std::vector<CsvRow>,4> &frames, const Iio &imu) {
    require(p.end>p.start && imu.devices.size()==2,"Invalid phase or IMU devices");
    std::array<std::vector<int64_t>,4> times;
    Json counts=Json::array(), markers, rates;
    for(size_t camera=0;camera<4;++camera) {
        for(const auto &row:frames[camera]) {
            int64_t time=std::stoll(row.at("sdk_timestamp"));
            if(p.start<=time && time<p.end) times[camera].push_back(time);
        }
        require(times[camera].size()==p.pulses.size(),p.name+": camera "+std::to_string(camera)+" count mismatch");
        for(size_t i=1;i<times[camera].size();++i) require(times[camera][i]>times[camera][i-1],"Non-monotonic camera timestamp");
        counts.push(times[camera].size());
    }
    std::vector<int64_t> reference;
    for(size_t i=0;i<imu.devices.size();++i) {
        const auto &d=imu.devices[i]; std::vector<int64_t> tagged;
        for(const auto &row:d.rows) {
            auto time=row.at("in_count5_fsync_timestamp_ns");
            if(row.at("in_count0_fsync_event") && p.start<=time && time<p.end) tagged.push_back(time);
        }
        require(tagged.size()==p.pulses.size(),p.name+": "+d.id+" FSYNC count mismatch");
        if(i==0) reference=tagged; else require(reference==tagged,"Accel/gyro FSYNC timestamp mismatch");
        markers[d.id]=tagged.size(); rates[d.id]=to_json(sampling(d.rows,p.start,p.end));
    }
    Json result=Json::Object{{"phase",p.name},{"pulses",p.pulses.size()},{"camera_frames",std::move(counts)},
                            {"imu_markers",std::move(markers)},{"imu_sampling",std::move(rates)}};
    if(!p.pulses.empty()) {
        std::vector<int64_t> offsets; int64_t max_skew=0;
        int64_t limit=std::min<int64_t>(40000000,(p.end-p.start)/p.pulses.size());
        for(size_t j=0;j<reference.size();++j) {
            int64_t low=times[0][j], high=low;
            for(const auto &camera:times) {
                auto delta=camera[j]-reference[j];
                require(delta>-1000000 && delta<limit,"Frame/FSYNC association exceeds one trigger period");
                offsets.push_back(delta); low=std::min(low,camera[j]); high=std::max(high,camera[j]);
            }
            max_skew=std::max(max_skew,high-low);
        }
        result["frame_minus_fsync_ns"]=Json::Object{{"min",*std::min_element(offsets.begin(),offsets.end())},
            {"median",median(offsets)},{"max",*std::max_element(offsets.begin(),offsets.end())}};
        result["max_camera_skew_ns"]=max_skew;
    }
    return result;
}
}
