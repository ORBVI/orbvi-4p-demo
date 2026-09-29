#include "file_sink.hpp"
#include <cinttypes>
#include <climits>
#include <utility>
namespace orbvi {
FileSink::FileSink(std::string directory, bool discard)
    : directory_(std::move(directory)), discard_(discard) {}
FileSink::~FileSink() { close(); }
int FileSink::open(unsigned port) {
    if (csv_) return -1;
    port_ = port;
    const auto path = directory_ + "/camera" + std::to_string(port) + ".csv";
    csv_ = fopen(path.c_str(), "wx");
    if (!csv_) { perror(path.c_str()); return -1; }
    return fputs("index,frame_id,sdk_timestamp,sdk_sys_timestamp,received_monotonic_ns,width,height,stride,bytes,file\n", csv_) < 0 || fflush(csv_) ? -1 : 0;
}
int FileSink::close() {
    if (!csv_) return 0;
    int result = fclose(csv_);
    csv_ = nullptr;
    return result;
}
int FileSink::write(unsigned index, const FrameView &frame)
{
    if (!csv_) return -1;
    char name[96], path[PATH_MAX];
    snprintf(name,sizeof(name),"camera%u_%06u.raw",port_,index);
    if (!frame.data || !frame.bytes) {
        fprintf(stderr,"camera%u: empty frame buffer\n",port_); return -1;
    }
    if (!discard_) {
        if (snprintf(path,sizeof(path),"%s/%s",directory_.c_str(),name)>=(int)sizeof(path)) return -1;
        FILE *file=fopen(path,"wbx");
        if (!file) { perror(path); return -1; }
        int error=fwrite(frame.data,1,frame.bytes,file)!=frame.bytes;
        if (fclose(file)) error=1;
        if (error) { fprintf(stderr,"Failed writing %s\n",path); return -1; }
    }
    if (fprintf(csv_,"%u,%u,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%u,%u,%u,%" PRIu64 ",%s\n",
        index,frame.id,frame.timestamp,frame.system_timestamp,frame.received_ns,
        frame.width,frame.height,frame.stride,(uint64_t)frame.bytes,
        discard_ ? "" : name)<0 || fflush(csv_)) return -1;
    return 0;
}

}
