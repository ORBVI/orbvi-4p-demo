#include "sensor_timing.hpp"
#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <map>
#include <vector>

static std::map<unsigned,unsigned> registers;
static std::vector<unsigned> writes;
static unsigned opened,closed;
static int scenario;
extern "C" int __wrap_open(const char *path,int,...) {
    assert(!std::strcmp(path,"/dev/i2c-6")); ++opened; return 42;
}
extern "C" int __wrap_close(int fd) {assert(fd==42); ++closed; return 0;}
extern "C" int __wrap_ioctl(int fd,unsigned long request,...) {
    assert(fd==42 && request==I2C_RDWR);
    va_list args; va_start(args,request); auto *data=va_arg(args,i2c_rdwr_ioctl_data *); va_end(args);
    auto &message=data->msgs[0]; assert(message.addr==0x30);
    unsigned reg=(unsigned(message.buf[0])<<8)|message.buf[1];
    if(data->nmsgs==2) {
        assert(data->msgs[1].flags==I2C_M_RD && data->msgs[1].len==1);
        data->msgs[1].buf[0]=registers[reg]; return 2;
    }
    assert(data->nmsgs==1 && message.len==4 && reg==0x320e);
    if(scenario==3) {errno=EIO; return -1;}
    for(unsigned i=2;i<4;++i) {writes.push_back(reg); registers[reg++]=message.buf[i];}
    if(scenario==2) registers[0x320f]^=1;
    return 1;
}
int main() {
    for(scenario=0;scenario<5;++scenario) {
        registers={{0x3107,1},{0x3108,0x32},{0x320c,2},{0x320d,0xee},
                   {0x320e,0x3f},{0x320f,0xff},{0x3e01,0x32},{0x3e02,0x80}};
        writes.clear(); opened=closed=0;
        if(scenario==1) registers[0x3108]=0x33;
        int result=orbvi::configure_sensor_vts(0,scenario==4 ? 1000 : 1400);
        assert(result==(scenario ? -1 : 0));
        assert(opened==closed);
        if(!scenario) {
            assert((writes==std::vector<unsigned>{0x320e,0x320f}));
            assert(registers[0x320e]==5 && registers[0x320f]==0x78);
            assert(registers[0x3e01]==0x32 && registers[0x3e02]==0x80);
        }
        if(scenario==1 || scenario==3 || scenario==4) assert(writes.empty());
        if(scenario==4) assert(opened==0);
    }
}
