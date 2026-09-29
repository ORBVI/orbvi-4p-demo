#include "x5_camera.hpp"
#include "hb_camera_interface.h"
#include "hb_mem_mgr.h"
#include "hbn_api.h"
#include "vin_cfg.h"
#include <cstdio>
namespace orbvi {
#define WIDTH 1088
#define HEIGHT 1280
#define RAW10 0x2b
struct X5Camera::State {
    unsigned port;
    camera_handle_t sensor{};
    hbn_vnode_handle_t vin{};
    hbn_vflow_handle_t flow{};
    bool sensor_open = false, vin_open = false, flow_open = false, started = false, leased = false;
    mipi_config_t mipi{};
    camera_config_t config{};
    hbn_vnode_image_t frame{};
};
X5Camera::X5Camera(unsigned port) : state_(new State{}) { state_->port = port; }
X5Camera::~X5Camera() { close(); }
static int checked(int result, const char *call, unsigned port)
{
    if (result) fprintf(stderr,"camera%u: %s failed (%d)\n",port,call,result);
    return result;
}
#define CALL(expr) do { if (checked((expr),#expr,c->port)) return -1; } while (0)

int X5Camera::prepare(bool external)
{
    auto *c = state_.get();
    unsigned fps=external ? 30 : 60;
    c->mipi = {};
    c->mipi.rx_enable = 1;
    auto &rx = c->mipi.rx_attr;
    rx.lane = 1; rx.datatype = RAW10; rx.fps = fps;
    rx.mclk = 1; rx.mipiclk = 1200;
    rx.width = WIDTH; rx.height = HEIGHT;
    rx.linelenth = 1400; rx.framelenth = 1500;
    rx.settle = 20; rx.channel_num = 1;
    c->config = {};
    snprintf(c->config.name, sizeof(c->config.name), "%s", "sc132gs");
    c->config.addr = c->port%2 ? 0x32 : 0x30;
    c->config.sensor_mode = external ? 6 : 1;
    c->config.fps = fps; c->config.width = WIDTH; c->config.height = HEIGHT;
    c->config.format = RAW10; c->config.gpio_enable_bit = 1;
    c->config.mipi_cfg = &c->mipi;
    snprintf(c->config.calib_lname, sizeof(c->config.calib_lname), "%s", "disable");
    vin_node_attr_t node{};
    node.cim_attr.mipi_rx = c->port;
    node.cim_attr.ipi_channel = 1;
    node.cim_attr.func.enable_frame_id = 1;
    // The external source owns FSYNC; LPWM remains disabled.
    vin_ichn_attr_t input{};
    input.width = WIDTH; input.height = HEIGHT; input.format = RAW10;
    vin_ochn_attr_t output{};
    output.ddr_en = 1; output.ochn_attr_type = VIN_BASIC_ATTR;
    output.vin_basic_attr.format = RAW10;
    output.vin_basic_attr.wstride = WIDTH*2;
    vin_attr_ex_t extra{};
    extra.vin_attr_ex_mask = 0x80; extra.mclk_ex_attr.mclk_freq = 24000000;
    hbn_buf_alloc_attr_t buffers{};
    buffers.buffers_num = 6; buffers.is_contig = 1;
    buffers.flags = HB_MEM_USAGE_CPU_READ_OFTEN|HB_MEM_USAGE_CPU_WRITE_OFTEN|HB_MEM_USAGE_CACHED;
    CALL(hbn_camera_create(&c->config,&c->sensor)); c->sensor_open=1;
    CALL(hbn_vnode_open(HB_VIN,c->port,AUTO_ALLOC_ID,&c->vin)); c->vin_open=1;
    CALL(hbn_vnode_set_attr(c->vin,&node));
    CALL(hbn_vnode_set_ichn_attr(c->vin,0,&input));
    CALL(hbn_vnode_set_ochn_attr(c->vin,0,&output));
    for (unsigned i=0; i<VIN_ATTR_EX_INVALID; i++) {
        if (!(extra.vin_attr_ex_mask & (1u<<i))) continue;
        extra.ex_attr_type=static_cast<decltype(extra.ex_attr_type)>(i);
        CALL(hbn_vnode_set_attr_ex(c->vin,&extra));
    }
    CALL(hbn_vnode_set_ochn_buf_attr(c->vin,0,&buffers));
    CALL(hbn_vflow_create(&c->flow)); c->flow_open=1;
    CALL(hbn_vflow_add_vnode(c->flow,c->vin));
    CALL(hbn_camera_attach_to_vin(c->sensor,c->vin));
    return 0;
}

int X5Camera::start() {
    auto *c = state_.get();
    CALL(hbn_vflow_start(c->flow));
    c->started = true;
    return 0;
}
int X5Camera::acquire(FrameView &view, unsigned timeout_ms) {
    auto &c = *state_;
    if (c.leased) return -1;
    int result = hbn_vnode_getframe(c.vin, 0, timeout_ms, &c.frame);
    if (result) return result;
    c.leased = true;
    const auto &f = c.frame;
    view = {f.buffer.virt_addr[0], static_cast<size_t>(f.buffer.size[0]),
            f.info.frame_id, static_cast<uint32_t>(f.buffer.width),
            static_cast<uint32_t>(f.buffer.height), static_cast<uint32_t>(f.buffer.stride),
            f.info.timestamps, f.info.sys_timestamps, 0};
    return 0;
}
int X5Camera::release() {
    auto &c = *state_;
    if (!c.leased) return 0;
    c.leased = false;
    return checked(hbn_vnode_releaseframe(c.vin, 0, &c.frame), "releaseframe", c.port);
}
int X5Camera::close() {
    auto *c = state_.get();
    int error = release();
    if (c->started && checked(hbn_vflow_stop(c->flow), "stop flow", c->port)) error = -1;
    if (c->vin_open) hbn_vnode_close(c->vin);
    if (c->sensor_open && checked(hbn_camera_destroy(c->sensor), "destroy camera", c->port)) error = -1;
    if (c->flow_open) hbn_vflow_destroy(c->flow);
    c->started = c->vin_open = c->sensor_open = c->flow_open = false;
    return error;
}
}
