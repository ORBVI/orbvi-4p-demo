#include "runtime.hpp"
#include <cassert>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
using namespace orbvi::validation;
template<class F> void rejects(F fn) { bool caught=false; try {fn();} catch(const std::exception &){caught=true;} assert(caught); }
int main(int argc, char **argv) {
    if (argc > 1) {
        if (std::string(argv[1]) == "--child") {
            std::signal(SIGINT, [](int){_exit(130);});
            std::ofstream(argv[2]) << "ready";
            for (;;) pause();
        }
        if (std::string(argv[1]) == "--exit") return 7;
    }
    auto root = temp_directory("/tmp", "orbvi-runtime-");
    Json j; j["stamp"] = int64_t(987654321012345678LL); j["escaped"] = "\"\n\\\t";
    std::ostringstream json; j.write(json);
    assert(json.str().find("987654321012345678") != std::string::npos);
    assert(json.str().find("\\\"\\n\\\\\\t") != std::string::npos);
    write_text(root/"csv", "index,file,bytes\n0,,12\n");
    auto rows=read_csv(root/"csv"); assert(rows.size()==1 && rows[0].at("file").empty());
    write_text(root/"bad", "index,file\n0\n"); rejects([&]{read_csv(root/"bad");});
    write_text(root/"image.raw", "abcd");
    verify_raw(root, {{"file","image.raw"},{"bytes","4"}});
    rejects([&]{verify_raw(root, {{"file","image.raw"},{"bytes","5"}});});
    write_text(root/"image.raw", "aaaa");
    rejects([&]{verify_raw(root, {{"file","image.raw"},{"bytes","4"}});});
    Process quick; quick.start({fs::absolute(argv[0]).string(), "--exit"},root/"quick.log");
    assert(quick.wait(2000000000)==7 && quick.poll()==7); quick.close();
    Process child; child.start({fs::absolute(argv[0]).string(), "--child", (root/"ready").string()},root/"child.log");
    auto deadline=now_ns()+2000000000;
    while (!fs::exists(root/"ready")) { assert(now_ns()<deadline); usleep(1000); }
    rejects([&]{child.wait(1000000);});
    child.signal(SIGINT); assert(child.wait(2000000000)==130); child.close();
    // Preflight both sensors before changing either. No real sysfs is touched.
    for (int i=0; i<2; ++i) {
        auto d=root/("iio:device"+std::to_string(i)); fs::create_directories(d/"buffer");
        write_text(d/"name", i ? "icm42688-gyro" : "icm42688-accel");
        write_text(d/"buffer/enable", i ? "1" : "0");
        write_text(d/"buffer/length", "77");
        write_text(d/"sampling_frequency", "1000");
        write_text(d/(i ? "in_anglvel_scale" : "in_accel_scale"), i ? "0.001065264" : "0.004788403");
    }
    Iio busy; rejects([&]{busy.open(root,root);}); busy.close();
    assert(read_text(root/"iio:device0/buffer/length")=="77");
    assert(read_text(root/"iio:device1/buffer/enable")=="1");
    // Feed fragmented scans through real nonblocking FIFOs, then verify restoration.
    write_text(root/"iio:device1/buffer/enable","0");
    fs::create_directory(root/"dev");
    int writers[2];
    for(int i=0;i<2;++i) {
        auto id="iio:device"+std::to_string(i); auto d=root/id;
        fs::create_directory(d/"scan_elements");
        std::vector<std::string> fields{"in_count0_fsync_event","in_count5_fsync_timestamp_ns","in_timestamp"};
        for(size_t n=0;n<fields.size();++n) {
            write_text(d/"scan_elements"/(fields[n]+"_en"),"0");
            write_text(d/"scan_elements"/(fields[n]+"_index"),std::to_string(n));
            write_text(d/"scan_elements"/(fields[n]+"_type"),"le:s64/64>>0");
        }
        write_text(d/"buffer/watermark","5");
        assert(mkfifo((root/"dev"/id).c_str(),0600)==0);
        writers[i]=::open((root/"dev"/id).c_str(),O_RDWR|O_NONBLOCK|O_CLOEXEC); assert(writers[i]>=0);
    }
    Iio streaming; streaming.open(root,root/"dev");
    uint8_t scan[24]={1}; scan[8]=42; scan[16]=99;
    assert(::write(writers[0],scan,7)==7); streaming.drain(0);
    assert(streaming.devices[0].rows.empty() && streaming.devices[0].pending.size()==7);
    assert(::write(writers[0],scan+7,17)==17); streaming.drain(0);
    assert(streaming.devices[0].rows.size()==1 && streaming.devices[0].rows[0].at("in_timestamp")==99);
    streaming.devices[0].paused=true;
    assert(::write(writers[0],scan,24)==24 && ::write(writers[1],scan,24)==24);
    streaming.drain(0);
    assert(streaming.devices[0].rows.size()==1 && streaming.devices[1].rows.size()==1);
    streaming.devices[0].paused=false; streaming.drain(0);
    assert(streaming.devices[0].rows.size()==2);
    assert(streaming.close().empty() && streaming.close().empty());
    for(int i=0;i<2;++i) {
        ::close(writers[i]); auto d=root/("iio:device"+std::to_string(i));
        assert(read_text(d/"buffer/enable")=="0" && read_text(d/"buffer/length")=="77");
        assert(read_text(d/"buffer/watermark")=="5");
        assert(read_text(d/"scan_elements/in_timestamp_en")=="0");
    }
    // A later open failure still restores all earlier partial initialization.
    Iio partial; rejects([&]{partial.open(root,root/"missing-dev");});
    assert(partial.close().empty());
    assert(read_text(root/"iio:device0/buffer/length")=="77");
    fs::remove_all(root);
}
