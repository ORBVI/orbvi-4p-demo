#include "workflows.hpp"
#include "../src/sensor_timing.hpp"
#include <algorithm>
#include <atomic>
#include <fcntl.h>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <fstream>
#include <time.h>
#include <unistd.h>
namespace orbvi::validation {
namespace {
class Gpio {
    int fd_=-1;
    bool exported_=false;
    fs::path path_="/sys/class/gpio/gpio417";
public:
    ~Gpio() {for(const auto &error:close()) std::cerr << error << '\n';}
    void open() {
        require(!fs::exists(path_),"GPIO417 already owned");
        write_text("/sys/class/gpio/export","417"); exported_=true;
        write_text(path_/"direction","low");
        fd_=::open((path_/"value").c_str(),O_WRONLY|O_CLOEXEC); require(fd_>=0,"Cannot open GPIO417 value");
    }
    void set(char value) {
        require(lseek(fd_,0,SEEK_SET)>=0 && ::write(fd_,&value,1)==1,"GPIO417 write failed");
    }
    int64_t pulse(int64_t *raw_stamp=nullptr) {
        set('1');
        if(raw_stamp) {
            timespec ts{};
            require(clock_gettime(CLOCK_MONOTONIC_RAW,&ts)==0,"Trigger clock read failed");
            *raw_stamp=int64_t(ts.tv_sec)*1000000000+ts.tv_nsec;
        }
        auto stamp=now_ns();
        try {sleep_until(stamp+1000000);} catch(...) {try {set('0');} catch(...) {} throw;}
        set('0'); return stamp;
    }
    std::vector<std::string> close() {
        std::vector<std::pair<std::string,std::function<void()>>> actions;
        if(fd_>=0) {
            actions.push_back({"lower GPIO",[this]{set('0');}});
            actions.push_back({"close GPIO",[this]{int fd=fd_; fd_=-1; require(::close(fd)==0,"GPIO close failed");}});
        }
        if(exported_) actions.push_back({"release GPIO",[this]{write_text("/sys/class/gpio/unexport","417"); exported_=false;}});
        return cleanup_all(actions);
    }
};
class Trigger {
    std::atomic_bool cancel_{false},done_{false};
    std::exception_ptr error_;
    std::thread thread_;
    std::vector<int64_t> emitted_,raw_emitted_;
public:
    Trigger(Gpio &gpio,int64_t start,unsigned seconds,unsigned hz,bool reference=false) {
        emitted_.reserve(size_t(seconds)*hz); raw_emitted_.reserve(size_t(seconds)*hz);
        thread_=std::thread([this,&gpio,start,seconds,hz,reference] {
            try {
                if(reference) {
                    cpu_set_t available,selected; CPU_ZERO(&available); CPU_ZERO(&selected);
                    int cpu=-1;
                    if(sched_getaffinity(0,sizeof(available),&available)==0)
                        for(int i=0;i<CPU_SETSIZE;++i) if(CPU_ISSET(i,&available)) cpu=i;
                    if(cpu>=0) {
                        CPU_SET(cpu,&selected);
                        if(pthread_setaffinity_np(pthread_self(),sizeof(selected),&selected)!=0)
                            std::cerr << "Trigger CPU affinity failed; continuing without pinning\n";
                    }
                }
                sched_param param{}; param.sched_priority=reference ? 80 : 1;
                int scheduled=pthread_setschedparam(pthread_self(),SCHED_FIFO,&param);
                if(reference && scheduled) std::cerr << "SCHED_FIFO 80 unavailable; using ordinary scheduling\n";
                else require(scheduled==0,"SCHED_FIFO priority 1 unavailable");
                int64_t previous=0;
                for(unsigned i=0;i<seconds*hz && !cancel_.load();++i) {
                    auto target=start+10000000+int64_t(i)*1000000000/hz;
                    sleep_until(target);
                    if(cancel_.load() || interrupted) break;
                    auto now=now_ns();
                    require(now-target<=2000000,"Trigger scheduling delay exceeds 2 ms; no catch-up pulses");
                    require(!previous || now-previous>=1000000000/hz-2000000,"Trigger interval too short");
                    int64_t raw=0;
                    previous=gpio.pulse(reference ? &raw : nullptr); emitted_.push_back(previous);
                    raw_emitted_.push_back(raw);
                }
            } catch(...) {error_=std::current_exception();}
            done_.store(true,std::memory_order_release);
        });
    }
    ~Trigger() {cancel_.store(true); if(thread_.joinable()) thread_.join();}
    void check() {if(done_.load(std::memory_order_acquire) && error_) std::rethrow_exception(error_);}
    const std::vector<int64_t> &raw_stamps() const {return raw_emitted_;}
    std::vector<int64_t> finish() {if(thread_.joinable()) thread_.join(); check(); return std::move(emitted_);}
};
void append_errors(std::vector<std::string> &to, std::vector<std::string> from) {
    to.insert(to.end(),from.begin(),from.end());
}
Json errors_json(const std::vector<std::string> &errors) {auto j=Json::array(); for(const auto &e:errors) j.push(e); return j;}
void wait_ready(Process &child,const fs::path &log,int seconds,const std::function<void()> &tick) {
    auto deadline=now_ns()+int64_t(seconds)*1000000000;
    while(read_text(log).find("Capture ready")==std::string::npos) {
        check_interrupted(); require(child.poll()==-1,"Capture startup failed; see "+log.string());
        require(now_ns()<deadline,"Capture readiness timeout"); tick();
    }
}
Phase acquire_phase(const std::string &name,int64_t duration,Iio &imu,Process *child=nullptr,Gpio *gpio=nullptr,unsigned hz=0) {
    Phase p; p.name=name; p.start=now_ns()+(hz ? 100000000 : 0);
    int64_t end=p.start+duration, report=p.start+10000000000LL;
    std::unique_ptr<Trigger> trigger;
    if(hz) trigger=std::make_unique<Trigger>(*gpio,p.start,unsigned(duration/1000000000),hz);
    while(now_ns()<end) {
        check_interrupted();
        if(child) require(child->poll()==-1,"Capture exited early; see capture.log");
        if(trigger) trigger->check();
        imu.drain(1);
        if(now_ns()>=report) {std::cout << name << " progress " << (now_ns()-p.start)/1000000000 << " seconds" << std::endl; report+=10000000000LL;}
    }
    if(trigger) {
        p.pulses=trigger->finish(); require(p.pulses.size()==size_t(duration/1000000000)*hz,"Trigger count mismatch");
    }
    p.end=now_ns();
    std::cout << name << " pulses " << p.pulses.size() << std::endl; return p;
}
}
int run_trigger(const std::string &runner,unsigned hz,unsigned vts,unsigned seconds,const fs::path &base,bool nv12) {
    check_board();
    auto root=temp_directory(base,"orbvi-trigger-");
    std::cout << "Artifacts: " << root << std::endl;
    Gpio gpio; Process child;
    Json result; result["fps"]=hz; result["requested_vts"]=vts; result["seconds"]=seconds; result["format"]=nv12 ? "nv12" : "raw10";
    Json cameras=Json::array(),timing=Json::array();
    std::string error; std::vector<std::string> errors;
    try {
        gpio.open();
        std::vector<std::string> args{runner,"--mode","external","--fps",std::to_string(hz),
            "--frames","1000000","--timeout-ms","60000","--discard","--output",(root/"capture").string()};
        if(hz==60) {args.push_back("--trigger-readout"); if(!vts) vts=1400;}
        result["requested_vts"]=vts;
        if(vts) {args.push_back("--vts"); args.push_back(std::to_string(vts));}
        if(nv12) {
            args.push_back("--max-skew-ns"); args.push_back("10000000");
        }
        child.start(args,root/"capture.log");
        wait_ready(child,root/"capture.log",15,[&] {
            gpio.pulse(); sleep_until(now_ns()+50000000);
        });
        sleep_until(now_ns()+500000000);
        int64_t quiet_before=now_ns();
        sleep_until(quiet_before+250000000);
        int64_t start=now_ns()+100000000;
        Trigger trigger(gpio,start,seconds,hz,true);
        while(now_ns()<start+int64_t(seconds)*1000000000+500000000) {
            check_interrupted(); require(child.poll()==-1,"Capture exited early; see capture.log");
            trigger.check(); sleep_until(now_ns()+10000000);
        }
        auto pulses=trigger.finish();
        require(pulses.size()==size_t(hz)*seconds,"Trigger count mismatch");
        std::ofstream pulse_csv(root/"triggers.csv");
        pulse_csv << "sequence,monotonic_ns,monotonic_raw_ns\n";
        for(size_t i=0;i<pulses.size();++i) pulse_csv << i+1 << ',' << pulses[i] << ',' << trigger.raw_stamps()[i] << '\n';
        pulse_csv.close(); require(bool(pulse_csv),"Trigger CSV write failed");
        result["pulses"]=pulses.size();
        double trigger_hz=pulses.size()>1 ? double(pulses.size()-1)*1e9/double(pulses.back()-pulses.front()) : 0;
        result["trigger_hz"]=trigger_hz;
        int64_t quiet_after=now_ns(); sleep_until(quiet_after+250000000);
        bool timing_valid=true;
        for(unsigned port=0;port<4;++port) {
            SensorTiming value;
            require(read_sensor_timing(port,value)==0,"Sensor timing read failed");
            if(vts && value.vts!=vts) timing_valid=false;
            timing.push(Json::Object{{"camera",port},{"hts",value.hts},{"vts",value.vts},
                {"exposure_code",value.exposure_code},{"analog_gain_code",value.analog_gain_code},
                {"digital_gain_code",value.digital_gain_code},{"trigger",value.trigger},{"trigger_mode",value.trigger_mode}});
        }
        child.signal(SIGINT); require(child.wait(15000000000LL)==130,"Capture stop failed");
        bool valid=timing_valid;
        for(unsigned port=0;port<4;++port) {
            auto rows=read_csv(root/"capture"/(nv12 ? "frame_sets.csv" : "camera"+std::to_string(port)+".csv"));
            unsigned count=0,quiet=0,gaps=0; uint64_t first=0,last=0,previous=0;
            for(const auto &row:rows) {
                int64_t received=std::stoll(row.at(nv12 ? "group_timestamp_ns" : "received_monotonic_ns"));
                if((received>=quiet_before && received<start) || received>=quiet_after) ++quiet;
                if(received<start || received>=quiet_after) continue;
                auto id=std::stoull(row.at(nv12 ? "cam"+std::to_string(port)+"_id" : "frame_id"));
                auto stamp=std::stoull(row.at(nv12 ? "cam"+std::to_string(port)+"_timestamp_ns" : "sdk_timestamp"));
                if(count && id!=previous+1) ++gaps;
                if(!count) first=stamp;
                previous=id; last=stamp; ++count;
            }
            double fps=count>1 && last>first ? double(count-1)*1e9/double(last-first) : 0;
            cameras.push(Json::Object{{"camera",port},{"frames",count},{"expected_frames",pulses.size()},
                {"sdk_timestamp_fps",fps},{"frame_id_gaps",gaps},{"quiet_frames",quiet}});
            std::cout << "camera" << port << ": frames=" << count << '/' << pulses.size()
                      << " sdk_fps=" << fps << " gaps=" << gaps << " quiet_frames=" << quiet << std::endl;
            if(count!=pulses.size() || gaps || quiet || fps<double(hz)*0.95) valid=false;
        }
        result["cameras"]=cameras; result["sensor_timing"]=timing;
        require(valid,"External-trigger frame count, cadence, continuity or quiet-period check failed");
    } catch(const std::exception &e) {error=e.what();}
    append_errors(errors,cleanup_all({{"stop capture",[&]{child.close();}}}));
    append_errors(errors,gpio.close());
    result["cameras"]=cameras; result["sensor_timing"]=timing;
    result["status"]=error.empty() && errors.empty() ? "PASS" : "FAIL";
    result["error"]=error; result["cleanup_errors"]=errors_json(errors);
    try {write_json(root/"result.json",result);} catch(const std::exception &e) {errors.push_back(e.what());}
    if(!error.empty()) std::cerr << "FAIL: " << error << '\n';
    for(const auto &e:errors) std::cerr << "Cleanup: " << e << '\n';
    if(!error.empty() || !errors.empty()) return 1;
    std::cout << "PASS: four-camera " << hz << "Hz external trigger, VTS=" << vts << std::endl;
    return 0;
}
int run_sync(const std::string &runner,bool raw,unsigned seconds,const fs::path &base) {
    check_board();
    auto root=temp_directory(base,"orbvi-sync-");
    std::cout << "Artifacts: " << root << std::endl;
    Iio imu; Gpio gpio; Process child;
    std::vector<Phase> phases; Stats before,after;
    Json result; result["raw_saved"]=raw;
    std::string error; std::vector<std::string> errors;
    try {
        imu.open(); gpio.open();
        std::vector<std::string> args{runner,"--mode","external","--frames","1000000","--timeout-ms","60000","--output",(root/"capture").string()};
        if(!raw) args.push_back("--discard");
        child.start(args,root/"capture.log");
        wait_ready(child,root/"capture.log",15,[&] {
            gpio.pulse(); auto until=now_ns()+50000000;
            while(now_ns()<until) imu.drain(1);
        });
        phases.push_back(acquire_phase("settle",1000000000,imu,&child));
        before=snapshot(imu);
        phases.push_back(acquire_phase("quiet_before",2000000000,imu,&child));
        phases.push_back(acquire_phase(raw ? "trigger_2hz" : "trigger_15hz",3000000000,imu,&child,&gpio,raw ? 2 : 15));
        phases.push_back(acquire_phase("quiet_middle",2000000000,imu,&child));
        phases.push_back(acquire_phase("trigger_30hz",int64_t(raw ? 1 : seconds)*1000000000,imu,&child,&gpio,30));
        phases.push_back(acquire_phase("quiet_after",2000000000,imu,&child));
        stop_capture([&]{child.signal(SIGINT);},[&]{return child.poll();},[&]{imu.drain(5);},
                     [&]{imu.disable();},[&]{after=snapshot(imu);});
        imu.drain(0);
        std::array<std::vector<CsvRow>,4> frames;
        for(size_t camera=0;camera<4;++camera) {
            frames[camera]=read_csv(root/"capture"/("camera"+std::to_string(camera)+".csv"));
            if(raw) for(const auto &row:frames[camera]) verify_raw(root/"capture",row);
        }
        Json checks=Json::array(); int64_t pulses=0;
        for(size_t i=1;i<phases.size();++i) {checks.push(analyze_phase(phases[i],frames,imu)); pulses+=phases[i].pulses.size();}
        check_counters(before,after);
        require(std::stoll(after.at("fsync_event_count"))-std::stoll(before.at("fsync_event_count"))==pulses,"FSYNC event counter mismatch");
        result["checks"]=std::move(checks);
    } catch(const std::exception &e) {error=e.what();}
    // Phase-local trigger threads are joined before reaching any of these actions.
    append_errors(errors,cleanup_all({{"disable IMU",[&]{imu.disable();}}, {"stop capture",[&]{child.close();}}}));
    append_errors(errors,gpio.close()); append_errors(errors,imu.close());
    result["configuration"]=imu.configuration; result["before"]=to_json(before); result["after"]=to_json(after);
    append_errors(errors,cleanup_all({{"write diagnostics",[&] {
        Json diagnostic; diagnostic["phases"]=Json::array();
        for(const auto &p:phases) diagnostic["phases"].push(to_json(p));
        diagnostic["imu"]=imu.samples();
        // Preserve samples even if the stats directory becomes unavailable.
        if(imu.devices.empty()) diagnostic["stats_error"]="IMU was not opened";
        else try {diagnostic["stats"]=to_json(snapshot(imu));} catch(const std::exception &e) {diagnostic["stats_error"]=e.what(); errors.push_back(e.what());}
        write_json(root/"diagnostic.json",diagnostic);
    }}}));
    result["status"]=error.empty() && errors.empty() ? "PASS" : "FAIL";
    result["error"]=error; result["cleanup_errors"]=errors_json(errors);
    try {write_json(root/"summary.json",result);} catch(const std::exception &e) {errors.push_back(e.what());}
    if(!error.empty()) std::cerr << error << '\n';
    for(const auto &e:errors) std::cerr << e << '\n';
    if(!error.empty() || !errors.empty()) return 1;
    std::cout << "PASS: four-camera external-trigger counts, quiet periods, matching accel/gyro FSYNC and timestamps\n"; return 0;
}
int run_backpressure(const fs::path &output) {
    check_board();
    Iio imu; Json result,checks,snapshots; std::vector<Phase> phases;
    std::string error; std::vector<std::string> errors;
    try {
        imu.open();
        auto phase=[&](const std::string &name,int64_t duration) {auto p=acquire_phase(name,duration,imu); phases.push_back(p); return p;};
        auto &accel=imu.device_for(Iio::Role::accel); auto &gyro=imu.device_for(Iio::Role::gyro);
        require(&accel!=&gyro,"Backpressure verification requires separate accelerometer and gyroscope buffers");
        auto warm=phase("warmup_both",1000000000);
        auto before=snapshot(imu); snapshots["before_stall"]=to_json(before);
        auto count=accel.rows.size(); accel.paused=true;
        auto stall=phase("accel_unread_gyro_drained",2000000000);
        require(accel.rows.size()==count,"Harness drained accel during stall");
        auto stalled=snapshot(imu); snapshots["after_stall"]=to_json(stalled);
        accel.paused=false;
        phase("resume_drain_settle",300000000);
        auto recovering=snapshot(imu); snapshots["before_recovery"]=to_json(recovering);
        auto recovery=phase("recovery_both",1000000000);
        phase("final_drain_margin",50000000);
        auto after=snapshot(imu); snapshots["after_recovery"]=to_json(after);
        imu.disable();
        auto stable=warm.end-500000000;
        for(const auto &d:imu.devices) {
            checks["baseline"][d.name]=to_json(sampling(d.rows,stable,warm.end,d.nominal_hz));
            checks["recovery"][d.name]=to_json(sampling(d.rows,recovery.start,recovery.end,d.nominal_hz));
        }
        checks["gyro_during_stall"]=to_json(sampling(gyro.rows,stall.start,stall.end,gyro.nominal_hz));
        checks["gyro_entire_stable_capture"]=to_json(sampling(gyro.rows,stable,recovery.end,gyro.nominal_hz));
        int64_t dropped=std::stoll(stalled.at("dropped_sample_count"))-std::stoll(before.at("dropped_sample_count"));
        require(dropped>0,"Accel queue did not demonstrate actual backpressure"); checks["stalled_accel_drop_count"]=dropped;
        for(const auto &item:before) {
            if(!item.second.empty() && std::all_of(item.second.begin(),item.second.end(),[](char c){return c>='0' && c<='9';}))
                checks["counter_delta"][item.first]=std::stoll(after.at(item.first))-std::stoll(item.second);
        }
        check_counters(before,after,false);
        require(recovering.at("dropped_sample_count")==after.at("dropped_sample_count"),"Samples still dropped after recovery");
    } catch(const std::exception &e) {error=e.what();}
    append_errors(errors,imu.close());
    result["status"]=error.empty() && errors.empty() ? "PASS" : "FAIL";
    result["output"]=output.string(); result["error"]=error;
    result["configuration"]=imu.configuration; result["snapshots"]=std::move(snapshots);
    result["checks"]=std::move(checks); result["cleanup_errors"]=errors_json(errors);
    result["phases"]=Json::array(); for(const auto &p:phases) result["phases"].push(to_json(p));
    for(const auto &d:imu.devices) {result["read_sample_counts"][d.name]=d.rows.size(); result["pending_bytes"][d.name]=d.pending.size();}
    try {
        Json payload=result; payload["samples"]=imu.samples(); write_json(output,payload);
    } catch(const std::exception &e) {result["status"]="FAIL"; result["artifact_error"]=e.what(); errors.push_back(e.what());}
    result.write(std::cout); std::cout << '\n'; return error.empty() && errors.empty() ? 0 : 1;
}
int run_board(const std::string &runner) {
    check_board(); auto root=temp_directory("/userdata","orbvi-demo-test-");
    std::cout << "Artifacts: " << root << std::endl;
    auto run=[&](const std::string &name,std::vector<std::string> args,int expected=0) {
        args.insert(args.begin(),{runner,"--output",(root/name).string()});
        Process child; child.start(args,root/(name+".log"));
        require(child.wait(40000000000LL,[]{check_interrupted(); sleep_until(now_ns()+1000000);})==expected,"Unexpected exit: "+name);
        return root/name;
    };
    auto data=run("four",{"--frames","3"});
    for(unsigned camera=0;camera<4;++camera) {
        auto rows=read_csv(data/("camera"+std::to_string(camera)+".csv")); require(rows.size()==3,"RAW frame count mismatch");
        int64_t previous=-1;
        for(const auto &row:rows) {
            verify_raw(data,row); require(row.at("width")=="1088" && row.at("height")=="1280" && std::stoul(row.at("stride"))>=2176,"Unexpected image dimensions");
            auto time=std::stoll(row.at("sdk_timestamp")); require(time>previous,"Non-monotonic camera timestamp"); previous=time;
        }
    }
    data=run("single",{"--camera","2","--frames","30","--discard"});
    require(std::distance(fs::directory_iterator(data),fs::directory_iterator{})==1 && fs::exists(data/"camera2.csv"),"Single camera output mismatch");
    auto single=read_csv(data/"camera2.csv"); require(single.size()==30,"Single camera frame count mismatch");
    for(const auto &row:single) require(row.at("file").empty(),"Discard mode saved RAW");
    run("invalid",{"--frames","-1"},2); run("four",{},1);
    Process child;
    child.start({runner,"--camera","0","--discard","--frames","1000000","--output",(root/"interrupt").string()},root/"interrupt.log");
    wait_ready(child,root/"interrupt.log",20,[]{sleep_until(now_ns()+50000000);});
    child.signal(SIGINT); require(child.wait(15000000000LL)==130,"SIGINT exit mismatch");
    run("restart",{"--frames","2","--discard"});
    std::cout << "PASS: four-camera RAW/CSV, single port, invalid input, no overwrite, SIGINT and restart\n"; return 0;
}
int run_shutdown(const std::string &runner,const std::string &injection) {
    check_board(); auto root=temp_directory("/userdata","orbvi-shutdown-");
    std::cout << "Artifacts: " << root << std::endl;
    for(bool inject:{false,true}) {
        std::string name=inject ? "failed-stop" : "clean"; auto log=root/(name+".log");
        Process child;
        child.start({runner,"--camera","0","--discard","--frames","1000000","--output",(root/name).string()},log,inject ? injection : "");
        wait_ready(child,log,20,[]{sleep_until(now_ns()+50000000);});
        sleep_until(now_ns()+100000000); check_interrupted(); child.signal(SIGINT);
        require(child.wait(15000000000LL)==(inject ? 1 : 130),"Cleanup exit code mismatch: "+name);
        if(inject) require(read_text(log).find("INJECTED: stop flow failure")!=std::string::npos,"Missing stop failure injection");
    }
    std::cout << "PASS: clean signal exit and cleanup failure priority\n"; return 0;
}
}
