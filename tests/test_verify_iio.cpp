#include "runtime.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace orbvi::validation;

template<class F> void rejects(F fn) {
    bool caught=false;
    try { fn(); } catch(const std::exception &) { caught=true; }
    assert(caught);
}

void write(const fs::path &path,const std::string &value) {
    std::ofstream output(path);
    assert(bool(output));
    output << value;
}

std::string number(double value) {
    std::ostringstream output;
    output << std::setprecision(16) << value;
    return output.str();
}

int device(const fs::path &root,int index,bool accel,bool gyro,bool fsync,double multiplier=1.0) {
    auto id="iio:device"+std::to_string(index);
    auto path=root/"sys"/id;
    fs::create_directories(path/"scan_elements");
    fs::create_directories(path/"buffer");
    fs::create_directories(root/"dev");
    write(path/"name","generic-imu-"+std::to_string(index));
    write(path/"buffer/enable","0");
    write(path/"buffer/length","77");
    write(path/"sampling_frequency","1000");
    if(accel) write(path/"in_accel_scale",number(multiplier*16.0*9.80665/std::ldexp(1.0,19)));
    if(gyro) write(path/"in_anglvel_scale",number(multiplier*2000.0*std::acos(-1.0)/180.0/std::ldexp(1.0,19)));
    std::vector<ScanEntry> entries;
    for(const auto &role:{std::pair{accel,std::string("in_accel_")},std::pair{gyro,std::string("in_anglvel_")}}) {
        if(!role.first) continue;
        for(const auto *axis:{"x","y","z"}) entries.push_back({role.second+axis,"le:s20/32>>4",int(entries.size())});
    }
    if(fsync) {
        entries.push_back({"in_count0_fsync_event","le:s64/64>>0",int(entries.size())});
        entries.push_back({"in_count5_fsync_timestamp_ns","le:s64/64>>0",int(entries.size())});
    }
    entries.push_back({"in_timestamp","le:s64/64>>0",int(entries.size())});
    for(const auto &entry:entries) {
        write(path/"scan_elements"/(entry.name+"_en"),"0");
        write(path/"scan_elements"/(entry.name+"_index"),std::to_string(entry.index));
        write(path/"scan_elements"/(entry.name+"_type"),entry.type);
    }
    assert(mkfifo((root/"dev"/id).c_str(),0600)==0);
    int writer=::open((root/"dev"/id).c_str(),O_RDWR|O_NONBLOCK|O_CLOEXEC);
    assert(writer>=0);
    return writer;
}

int main() {
    char pattern[]="/tmp/orbvi-verify-iio-XXXXXX";
    const char *created=mkdtemp(pattern);
    assert(created);
    fs::path root(created);
    auto combined=root/"combined";
    int writer=device(combined,0,true,true,true);
    fs::create_directory(combined/"sys"/"sync_stats");
    write(combined/"sys"/"sync_stats"/"fifo_overflow_count","0");
    Iio imu;
    imu.open(combined/"sys",combined/"dev");
    assert(imu.devices.size()==1);
    assert(&imu.device_for(Iio::Role::accel)==&imu.device_for(Iio::Role::gyro));
    assert(snapshot(imu).at("fifo_overflow_count")=="0");
    assert(imu.close().empty());
    ::close(writer);
    auto variable=root/"variable";
    writer=device(variable,0,true,true,true,2.0);
    write(variable/"sys/iio:device0/sampling_frequency","800");
    write(variable/"sys/iio:device0/fsync_supported","1");
    Iio flexible;
    flexible.open(variable/"sys",variable/"dev");
    assert(flexible.devices[0].nominal_hz==800);
    assert(flexible.close().empty());
    ::close(writer);
    auto unsupported=root/"unsupported-fsync";
    writer=device(unsupported,0,true,true,true);
    write(unsupported/"sys/iio:device0/fsync_supported","0");
    Iio no_fsync_capability;
    rejects([&]{no_fsync_capability.open(unsupported/"sys",unsupported/"dev");});
    assert(read_text(unsupported/"sys/iio:device0/buffer/length")=="77");
    ::close(writer);
    auto missing=root/"missing-fsync";
    writer=device(missing,0,true,true,false);
    Iio no_fsync;
    rejects([&]{no_fsync.open(missing/"sys",missing/"dev");});
    assert(read_text(missing/"sys/iio:device0/buffer/length")=="77");
    ::close(writer);
    auto wrong=root/"wrong-range";
    writer=device(wrong,0,true,true,true,0.5);
    Iio wrong_range;
    rejects([&]{wrong_range.open(wrong/"sys",wrong/"dev");});
    assert(read_text(wrong/"sys/iio:device0/buffer/enable")=="0");
    ::close(writer);
}
