#include "workflows.hpp"
#include <algorithm>
#include <atomic>
#include <fcntl.h>
#include <iostream>
#include <thread>
#include <pthread.h>
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
    int64_t pulse() {
        set('1'); auto stamp=now_ns();
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
    std::vector<int64_t> emitted_;
public:
    Trigger(Gpio &gpio,int64_t start,unsigned seconds,unsigned hz) {
        emitted_.reserve(size_t(seconds)*hz);
        thread_=std::thread([this,&gpio,start,seconds,hz] {
            try {
                sched_param param{}; param.sched_priority=1;
                require(pthread_setschedparam(pthread_self(),SCHED_FIFO,&param)==0,"SCHED_FIFO priority 1 unavailable");
                int64_t previous=0;
                for(unsigned i=0;i<seconds*hz && !cancel_.load();++i) {
                    auto target=start+10000000+int64_t(i)*1000000000/hz;
                    sleep_until(target);
                    if(cancel_.load() || interrupted) break;
                    auto now=now_ns();
                    require(now-target<=2000000,"Trigger scheduling delay exceeds 2 ms; no catch-up pulses");
                    require(!previous || now-previous>=1000000000/hz-2000000,"Trigger interval too short");
                    previous=gpio.pulse(); emitted_.push_back(previous);
                }
            } catch(...) {error_=std::current_exception();}
            done_.store(true,std::memory_order_release);
        });
    }
    ~Trigger() {cancel_.store(true); if(thread_.joinable()) thread_.join();}
    void check() {if(done_.load(std::memory_order_acquire) && error_) std::rethrow_exception(error_);}
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
int run_sync(const std::string &runner,bool raw,unsigned seconds,const fs::path &base) {
    check_board();
    auto root=temp_directory(base,"orbvi-sync-");
    std::cout << "Artifacts: " << root << std::endl;
    Iio imu; Gpio gpio; Process child;
    std::vector<Phase> phases; Stats before,after;
    Json result; result["raw_saved"]=raw;
    std::string error; std::vector<std::string> errors;
    try {
        gpio.open(); imu.open();
        std::vector<std::string> args{runner,"--mode","external","--frames","1000000","--timeout-ms","60000","--output",(root/"capture").string()};
        if(!raw) args.push_back("--discard");
        child.start(args,root/"capture.log");
        wait_ready(child,root/"capture.log",15,[&] {
            gpio.pulse(); auto until=now_ns()+50000000;
            while(now_ns()<until) imu.drain(1);
        });
        phases.push_back(acquire_phase("settle",1000000000,imu,&child));
        before=snapshot();
        phases.push_back(acquire_phase("quiet_before",2000000000,imu,&child));
        phases.push_back(acquire_phase(raw ? "trigger_2hz" : "trigger_15hz",3000000000,imu,&child,&gpio,raw ? 2 : 15));
        phases.push_back(acquire_phase("quiet_middle",2000000000,imu,&child));
        phases.push_back(acquire_phase("trigger_30hz",int64_t(raw ? 1 : seconds)*1000000000,imu,&child,&gpio,30));
        phases.push_back(acquire_phase("quiet_after",2000000000,imu,&child));
        stop_capture([&]{child.signal(SIGINT);},[&]{return child.poll();},[&]{imu.drain(5);},
                     [&]{imu.disable();},[&]{after=snapshot();});
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
        try {diagnostic["stats"]=to_json(snapshot());} catch(const std::exception &e) {diagnostic["stats_error"]=e.what(); errors.push_back(e.what());}
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
        auto warm=phase("warmup_both",1000000000);
        auto before=snapshot(); snapshots["before_stall"]=to_json(before);
        auto &accel=imu.device("icm42688-accel"); auto &gyro=imu.device("icm42688-gyro");
        auto count=accel.rows.size(); accel.paused=true;
        auto stall=phase("accel_unread_gyro_drained",2000000000);
        require(accel.rows.size()==count,"Harness drained accel during stall");
        auto stalled=snapshot(); snapshots["after_stall"]=to_json(stalled);
        accel.paused=false;
        phase("resume_drain_settle",300000000);
        auto recovering=snapshot(); snapshots["before_recovery"]=to_json(recovering);
        auto recovery=phase("recovery_both",1000000000);
        phase("final_drain_margin",50000000);
        auto after=snapshot(); snapshots["after_recovery"]=to_json(after);
        imu.disable();
        auto stable=warm.end-500000000;
        for(const auto &d:imu.devices) {
            checks["baseline"][d.name]=to_json(sampling(d.rows,stable,warm.end));
            checks["recovery"][d.name]=to_json(sampling(d.rows,recovery.start,recovery.end));
        }
        checks["gyro_during_stall"]=to_json(sampling(gyro.rows,stall.start,stall.end));
        checks["gyro_entire_stable_capture"]=to_json(sampling(gyro.rows,stable,recovery.end));
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
