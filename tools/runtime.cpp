#include "runtime.hpp"
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <poll.h>
#include <set>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <ctime>
namespace orbvi::validation {
volatile sig_atomic_t interrupted = 0;
void check_interrupted() { require(!interrupted, "Interrupted by signal " + std::to_string(interrupted)); }
void sleep_until(int64_t deadline) {
    timespec time{time_t(deadline/1000000000), long(deadline%1000000000)};
    int code;
    do { code=clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&time,nullptr); } while(code==EINTR && !interrupted);
    require(code==0 || (code==EINTR && interrupted), "clock_nanosleep failed");
}
static void quoted(std::ostream &out, const std::string &text) {
    const char hex[]="0123456789abcdef";
    out << '"';
    for (unsigned char c : text) {
        switch(c) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default: if(c<32) out << "\\u00" << hex[c>>4] << hex[c&15]; else out << c;
        }
    }
    out << '"';
}
void Json::write(std::ostream &out) const {
    std::visit([&](const auto &x) {
        using T=std::decay_t<decltype(x)>;
        if constexpr(std::is_same_v<T,std::nullptr_t>) out << "null";
        else if constexpr(std::is_same_v<T,bool>) out << (x ? "true" : "false");
        else if constexpr(std::is_same_v<T,std::string>) quoted(out,x);
        else if constexpr(std::is_same_v<T,Object>) {
            out << '{'; bool first=true;
            for(const auto &item:x) { if(!first) out << ','; first=false; quoted(out,item.first); out << ':'; item.second.write(out); }
            out << '}';
        } else if constexpr(std::is_same_v<T,Array>) {
            out << '['; bool first=true;
            for(const auto &item:x) { if(!first) out << ','; first=false; item.write(out); }
            out << ']';
        } else if constexpr(std::is_same_v<T,double>) { require(std::isfinite(x),"Non-finite JSON number"); out << std::setprecision(17) << x; }
        else out << x;
    },value);
}
Json to_json(const Row &row) { Json j; for(const auto &x:row) j[x.first]=x.second; return j; }
Json to_json(const Stats &row) { Json j; for(const auto &x:row) j[x.first]=x.second; return j; }
Json to_json(const Sampling &r) {
    return Json::Object{{"samples",r.samples},{"duration_s",r.duration},{"sample_span_hz",r.hz},
        {"measured_hz",r.hz},{"window_count_hz",r.samples/r.duration},{"first_sample_ns",r.first},
        {"last_sample_ns",r.last},{"min_interval_ns",r.min_gap},{"median_interval_ns",r.median},{"max_interval_ns",r.max_gap}};
}
std::string read_text(const fs::path &path) {
    std::ifstream in(path,std::ios::binary);
    require(bool(in),"Cannot read "+path.string());
    std::string text((std::istreambuf_iterator<char>(in)),{});
    require(!in.bad(),"Read failed: "+path.string());
    while(!text.empty() && (text.back()=='\0' || text.back()=='\n' || text.back()=='\r' || text.back()==' ')) text.pop_back();
    return text;
}
void write_text(const fs::path &path, const std::string &text) {
    std::ofstream out(path,std::ios::binary);
    require(bool(out),"Cannot write "+path.string());
    out << text; out.close(); require(!out.fail(),"Write/close failed: "+path.string());
}
void write_json(const fs::path &path, const Json &json) {
    std::ofstream out(path); require(bool(out),"Cannot write "+path.string());
    json.write(out); out << '\n'; out.close(); require(!out.fail(),"Write/close failed: "+path.string());
}
fs::path temp_directory(const fs::path &base, const std::string &prefix) {
    std::string pattern=(base/(prefix+"XXXXXX")).string();
    require(mkdtemp(pattern.data())!=nullptr,"Cannot create output directory under "+base.string());
    return pattern;
}
void check_board() { require(read_text("/proc/device-tree/model")=="HS ORBVI 4P V1.0","Expected HS ORBVI 4P V1.0 device tree"); }
Process::~Process() { try {close();} catch(const std::exception &e){std::cerr << "Process cleanup: " << e.what() << '\n';} }
void Process::start(const std::vector<std::string> &args, const fs::path &log, const std::string &preload) {
    require(pid_==-1 && !args.empty(),"Invalid process start");
    int fd=::open(log.c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_CLOEXEC,0644);
    require(fd>=0,"Cannot open child log: "+log.string());
    std::vector<char *> argv; for(const auto &arg:args) argv.push_back(const_cast<char *>(arg.c_str())); argv.push_back(nullptr);
    pid_=fork();
    if(pid_==0) {
        std::signal(SIGINT,SIG_DFL); std::signal(SIGTERM,SIG_DFL);
        if(dup2(fd,STDOUT_FILENO)<0 || dup2(fd,STDERR_FILENO)<0) _exit(126);
        ::close(fd);
        // Match run.sh also when the caller supplies the C++ binary directly.
        const char *existing=getenv("LD_LIBRARY_PATH");
        std::string libraries="/usr/hobot/lib:/usr/hobot/lib/sensor";
        if(existing && *existing) libraries+=std::string(":")+existing;
        if(setenv("LD_LIBRARY_PATH",libraries.c_str(),1)!=0) _exit(126);
        if(!preload.empty() && setenv("LD_PRELOAD",preload.c_str(),1)!=0) _exit(126);
        execv(argv[0],argv.data()); _exit(127);
    }
    ::close(fd); require(pid_>0,"fork failed"); result_=-1;
}
int Process::poll() {
    if(pid_==-1) return result_;
    int status=0; int result;
    do {result=waitpid(pid_,&status,WNOHANG);} while(result<0 && errno==EINTR);
    require(result>=0,"waitpid failed");
    if(result>0) {result_=WIFEXITED(status) ? WEXITSTATUS(status) : 128+WTERMSIG(status); pid_=-1;}
    return result_;
}
void Process::signal(int sig) { if(pid_>0 && kill(pid_,sig)<0) require(errno==ESRCH,"kill failed"); }
int Process::wait(int64_t timeout, const std::function<void()> &tick) {
    auto deadline=now_ns()+timeout;
    while(poll()==-1) {
        require(now_ns()<deadline,"Child timeout");
        if(tick) tick(); else sleep_until(now_ns()+1000000);
    }
    return result_;
}
void Process::close() {
    if(pid_<=0 || poll()!=-1) return;
    signal(SIGINT);
    try {wait(10000000000LL);} catch(...) {signal(SIGKILL); wait(2000000000);}
}
static std::vector<std::string> split(const std::string &line) {
    std::vector<std::string> cells; size_t start=0;
    for(;;) {auto end=line.find(',',start); cells.push_back(line.substr(start,end-start)); if(end==std::string::npos) return cells; start=end+1;}
}
std::vector<CsvRow> read_csv(const fs::path &path) {
    std::ifstream input(path); require(bool(input),"Missing CSV: "+path.string());
    std::string line; require(bool(std::getline(input,line)),"Empty CSV"); auto names=split(line);
    require(std::set<std::string>(names.begin(),names.end()).size()==names.size(),"Duplicate CSV columns");
    std::vector<CsvRow> rows;
    while(std::getline(input,line)) {
        auto values=split(line); require(values.size()==names.size(),"Malformed CSV row");
        CsvRow row; for(size_t i=0;i<names.size();++i) row[names[i]]=values[i]; rows.push_back(std::move(row));
    }
    require(!input.bad(),"CSV read failure"); return rows;
}
void verify_raw(const fs::path &root, const CsvRow &row) {
    fs::path relative=row.at("file");
    require(!relative.empty() && relative.filename()==relative,"Invalid RAW filename");
    auto path=root/relative;
    require(fs::file_size(path)==std::stoull(row.at("bytes")),"RAW size mismatch: "+path.string());
    std::ifstream input(path,std::ios::binary); require(bool(input),"Cannot read RAW");
    char data[65536]; int first=-1; bool varied=false;
    while(input.read(data,sizeof(data)) || input.gcount()) {
        for(std::streamsize i=0;i<input.gcount();++i) {
            int value=static_cast<unsigned char>(data[i]); if(first<0) first=value; else varied |= value!=first;
        }
    }
    require(!input.bad() && varied,"Constant or unreadable RAW buffer: "+path.string());
}
Iio::~Iio() { for(const auto &e:close()) std::cerr << "IIO cleanup: " << e << '\n'; }
void Iio::open(const fs::path &sys, const fs::path &dev) {
    require(devices.empty(),"IIO already opened");
    for(const auto &found:find_imu_devices(sys)) {
        Device d; d.path=found.path; d.id=found.id; d.name=found.name;
        d.accel=found.accel; d.gyro=found.gyro;
        require(read_text(d.path/"buffer/enable")=="0",d.name+" buffer is busy");
        auto fsync=d.path/"fsync_supported";
        Json config=Json::Object{{"name",d.name},{"accel",d.accel},{"gyro",d.gyro}};
        const auto source=d.path/"timestamp_source";
        const auto clock=d.path/"current_timestamp_clock";
        config["timestamp_source"]=fs::exists(source) ? read_text(source) : "unspecified";
        config["timestamp_clock"]=fs::exists(clock) ? read_text(clock) : "unspecified";
        config["fsync_supported"]=fs::exists(fsync) ? read_text(fsync) : "unspecified";
        configuration[d.id]=config;
        if(fs::exists(fsync)) require(read_text(fsync)=="1",
            "FSYNC verification unavailable: "+d.id+" ("+d.name+") reports fsync_supported="+read_text(fsync));
        if(fs::exists(clock)) require(read_text(clock)=="monotonic",
            "IMU timestamp clock must be monotonic for camera association: "+d.id);
        std::vector<ScanEntry> entries;
        for(const auto &p:fs::directory_iterator(d.path/"scan_elements")) {
            auto name=p.path().filename().string(); if(name.size()<3 || name.substr(name.size()-3)!="_en") continue;
            auto field=name.substr(0,name.size()-3);
            entries.push_back({field,read_text(d.path/"scan_elements"/(field+"_type")),std::stoi(read_text(d.path/"scan_elements"/(field+"_index")))});
            d.settings[p.path()]=read_text(p.path());
        }
        d.layout=make_layout(entries);
        for(const auto *required:{"in_timestamp","in_count0_fsync_event","in_count5_fsync_timestamp_ns"})
            require(std::any_of(entries.begin(),entries.end(),[&](const auto &e){return e.name==required;}),"Missing IIO field: "+std::string(required));
        bool have_rate=false;
        for(const auto &role:{std::pair{d.accel,std::string("in_accel")},std::pair{d.gyro,std::string("in_anglvel")}}) {
            if(!role.first) continue;
            bool gyro=role.second=="in_anglvel";
            auto frequency=d.path/"sampling_frequency";
            if(!fs::exists(frequency)) frequency=d.path/(role.second+"_sampling_frequency");
            double rate=std::stod(read_text(frequency));
            require(std::isfinite(rate) && rate>=800 && rate<=2000,"IMU sampling frequency must be 800..2000 Hz: "+d.name);
            require(!have_rate || rate==d.nominal_hz,"IMU axes have different sampling frequencies: "+d.name);
            d.nominal_hz=rate; have_rate=true;
            double first_scale=0, first_range=0;
            for(const auto *axis:{"x","y","z"}) {
                auto name=role.second+"_"+axis;
                auto field=std::find_if(d.layout.fields.begin(),d.layout.fields.end(),[&](const auto &item){return item.name==name;});
                require(field!=d.layout.fields.end() && field->signed_value,"Missing signed IIO axis: "+name);
                double scale=iio_axis_value(d.path,role.second,axis,"scale",NAN);
                double range=scale*std::ldexp(1.0,int(field->bits)-1);
                double expected=gyro ? 2000.0*std::acos(-1.0)/180.0 : 16.0*9.80665;
                require(scale>0 && std::isfinite(range) && range>=expected*0.999 && range<=expected*4.001,
                        "Expected at least +/-16 g / +/-2000 dps IIO range: "+name);
                if(std::string(axis)=="x") {first_scale=scale; first_range=range;}
            }
            config[gyro ? "gyro_scale_si" : "accel_scale_si"]=first_scale;
            config[gyro ? "gyro_range_rps" : "accel_range_mps2"]=first_range;
            if(d.accel != d.gyro) {
                config["scale_si"]=first_scale;
            }
        }
        config["nominal_hz"]=d.nominal_hz;
        if(!devices.empty()) require(devices.front().nominal_hz==d.nominal_hz,"IMU devices have different sampling frequencies");
        for(const auto *setting:{"buffer/length","buffer/enable","buffer/watermark"}) {
            auto path=d.path/setting; if(fs::exists(path)) d.settings[path]=read_text(path);
        }
        configuration[d.id]=std::move(config);
        devices.push_back(std::move(d));
    }
    for(auto &d:devices) {
        d.saved=true;
        write_text(d.path/"buffer/length","256");
        if(fs::exists(d.path/"buffer/watermark")) write_text(d.path/"buffer/watermark","1");
        for(const auto &field:d.layout.fields) write_text(d.path/"scan_elements"/(field.name+"_en"),"1");
        d.fd=::open((dev/d.id).c_str(),O_RDONLY|O_NONBLOCK|O_CLOEXEC); require(d.fd>=0,"Cannot open "+d.id);
    }
    for(auto &d:devices) write_text(d.path/"buffer/enable","1");
}
void Iio::drain(int timeout_ms) {
    check_interrupted();
    std::vector<pollfd> fds; std::vector<Device *> selected;
    for(auto &d:devices) if(d.fd>=0 && !d.paused) {fds.push_back({d.fd,POLLIN,0}); selected.push_back(&d);}
    int result=::poll(fds.data(),fds.size(),timeout_ms);
    if(result<0 && errno==EINTR) {check_interrupted(); return;}
    require(result>=0,"IIO poll failed");
    for(size_t i=0;i<fds.size();++i) {
        require(!(fds[i].revents&(POLLERR|POLLHUP|POLLNVAL)),"IIO descriptor error");
        if(!(fds[i].revents&POLLIN)) continue;
        auto &d=*selected[i]; uint8_t bytes[65536]; auto count=::read(d.fd,bytes,sizeof(bytes));
        if(count<0 && (errno==EAGAIN || errno==EINTR)) continue;
        require(count>0,"IIO read failed or unexpected EOF: "+d.id);
        d.pending.insert(d.pending.end(),bytes,bytes+count);
        size_t complete=d.pending.size()/d.layout.size*d.layout.size;
        for(size_t pos=0;pos<complete;pos+=d.layout.size) d.rows.push_back(d.layout.decode(d.pending.data()+pos));
        d.pending.erase(d.pending.begin(),d.pending.begin()+complete);
    }
}
void Iio::disable() {
    std::vector<std::pair<std::string,std::function<void()>>> actions;
    for(auto &d:devices) if(d.saved) actions.push_back({"disable "+d.id,[path=d.path]{write_text(path/"buffer/enable","0");}});
    auto errors=cleanup_all(actions); require(errors.empty(),errors.empty() ? "" : errors.front());
}
std::vector<std::string> Iio::close() {
    std::vector<std::pair<std::string,std::function<void()>>> actions;
    for(auto &d:devices) if(d.saved) actions.push_back({"disable "+d.id,[path=d.path]{write_text(path/"buffer/enable","0");}});
    for(auto &d:devices) {
        if(d.fd>=0) {int fd=d.fd; d.fd=-1; actions.push_back({"close "+d.id,[fd]{require(::close(fd)==0,"close failed");}});}
        if(!d.saved) continue;
        std::vector<std::pair<fs::path,std::string>> settings(d.settings.begin(),d.settings.end());
        auto order=[](const fs::path &p){return p.filename()=="enable" ? 2 : p.filename()=="watermark" ? 1 : 0;};
        std::stable_sort(settings.begin(),settings.end(),[&](const auto &a,const auto &b){return order(a.first)<order(b.first);});
        for(const auto &s:settings) actions.push_back({"restore "+s.first.string(),[s]{write_text(s.first,s.second);}});
        d.saved=false;
    }
    return cleanup_all(actions);
}
Iio::Device &Iio::device_for(Role role) {
    for(auto &d:devices) if(role==Role::accel ? d.accel : d.gyro) return d;
    throw std::runtime_error("Missing IIO device for requested role");
}
Json Iio::samples() const {
    Json result=Json::array();
    for(const auto &d:devices) for(const auto &row:d.rows) {auto j=to_json(row); j["device"]=d.id; result.push(std::move(j));}
    return result;
}
Stats snapshot(const Iio &imu) {
    fs::path stats;
    for(const auto &device:imu.devices) {
        fs::path current=fs::canonical(device.path).parent_path(), found;
        while(!current.empty()) {
            if(fs::is_directory(current/"sync_stats")) { found=current/"sync_stats"; break; }
            if(current==current.root_path()) break;
            current=current.parent_path();
        }
        require(!found.empty(),"IMU driver does not provide sync_stats: "+device.id);
        require(stats.empty() || fs::equivalent(stats,found),"IMU devices do not share sync_stats");
        stats=found;
    }
    require(!stats.empty(),"No IMU device for sync_stats");
    Stats result; for(const auto &p:fs::directory_iterator(stats)) result[p.path().filename().string()]=read_text(p.path()); return result;
}
void check_counters(const Stats &before, const Stats &after, bool drops) {
    for(const auto *key:{"timestamp_reset_count","spi_error_count","fifo_overflow_count","fifo_lost_packet_count","fifo_parse_error_count"})
        require(before.at(key)==after.at(key),"Counter changed: "+std::string(key));
    if(drops) require(before.at("dropped_sample_count")==after.at("dropped_sample_count"),"Dropped IMU samples");
}
}
