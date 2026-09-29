#include "workflows.hpp"
#include <cassert>
using namespace orbvi::validation;
template<class F> void rejects(F fn) {bool caught=false; try {fn();} catch(const std::exception &){caught=true;} assert(caught);}
int main() {
    Phase p{"trigger",0,1000000000,{10000000}};
    Iio imu;
    for(int d=0;d<2;++d) {
        Iio::Device device; device.id=std::to_string(d);
        for(int64_t i=0;i<1000;++i) device.rows.push_back({{"in_timestamp",i*1000000},
            {"in_count0_fsync_event",i==10},{"in_count5_fsync_timestamp_ns",i==10 ? 10000000 : 0}});
        imu.devices.push_back(std::move(device));
    }
    std::array<std::vector<CsvRow>,4> frames;
    for(auto &rows:frames) rows.push_back({{"sdk_timestamp","20000000"}});
    auto result=analyze_phase(p,frames,imu); (void)result;
    frames[0].clear(); rejects([&]{analyze_phase(p,frames,imu);});
    frames[0].push_back({{"sdk_timestamp","60000000"}}); rejects([&]{analyze_phase(p,frames,imu);});
    frames[0][0]["sdk_timestamp"]="20000000";
    imu.devices[0].rows[10]["in_count5_fsync_timestamp_ns"]++;
    rejects([&]{analyze_phase(p,frames,imu);});
    imu.devices[0].rows[10]["in_count5_fsync_timestamp_ns"]--;
    Phase quiet{"quiet",100000000,900000000,{}};
    analyze_phase(quiet,frames,imu);
    frames[0].push_back({{"sdk_timestamp","300000000"}});
    rejects([&]{analyze_phase(quiet,frames,imu);});
}
