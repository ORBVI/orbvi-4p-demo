#include "x5_camera.hpp"
#include "hb_camera_interface.h"
#include "hb_mem_mgr.h"
#include "hbn_api.h"
#include "isp_cfg.h"
#include "vin_cfg.h"
#include "vse_cfg.h"
#include <cstdio>

namespace orbvi::nv12 {
namespace {
constexpr unsigned width = 1088, height = 1280, raw10 = 0x2b;
int check(int status, const char *operation, unsigned port) {
    if (status) std::fprintf(stderr, "camera%u: %s failed (%d)\n", port, operation, status);
    return status;
}
}

struct X5Camera::State {
    unsigned port, output_fps, output_width, output_height;
    camera_handle_t sensor{};
    hbn_vnode_handle_t vin{}, isp{}, vse{};
    hbn_vflow_handle_t flow{};
    bool sensor_open = false, vin_open = false, isp_open = false, vse_open = false;
    bool flow_open = false, started = false, leased = false;
    mipi_config_t mipi{};
    camera_config_t config{};
    hbn_vnode_image_t image{};
};

X5Camera::X5Camera(unsigned port, unsigned output_fps, unsigned output_width, unsigned output_height)
    : state_(new State{}) {
    state_->port = port; state_->output_fps = output_fps;
    state_->output_width = output_width; state_->output_height = output_height;
}
X5Camera::~X5Camera() { close(); }

int X5Camera::prepare() {
    auto *c = state_.get();
#define STEP(call) do { if (check((call), #call, c->port)) return -1; } while (0)
    c->mipi.rx_enable = 1;
    auto &rx = c->mipi.rx_attr;
    rx.lane = 1; rx.datatype = raw10; rx.fps = 60;
    rx.mclk = 1; rx.mipiclk = 1200;
    rx.width = width; rx.height = height;
    rx.linelenth = 1400; rx.framelenth = 1500;
    rx.settle = 20; rx.channel_num = 1;
    std::snprintf(c->config.name, sizeof(c->config.name), "%s", "sc132gs");
    c->config.addr = c->port % 2 ? 0x32 : 0x30;
    c->config.sensor_mode = 1;
    c->config.fps = 60; c->config.width = width; c->config.height = height;
    c->config.format = raw10; c->config.gpio_enable_bit = 1;
    c->config.mipi_cfg = &c->mipi;
    std::snprintf(c->config.calib_lname, sizeof(c->config.calib_lname), "%s", "disable");
    vin_node_attr_t vin{};
    vin.cim_attr.mipi_rx = c->port;
    vin.cim_attr.ipi_channel = 1;
    vin.cim_attr.func.enable_frame_id = 1;
    vin_ichn_attr_t vin_in{};
    vin_in.width = width; vin_in.height = height; vin_in.format = raw10;
    vin_ochn_attr_t vin_out{};
    vin_out.ddr_en = 1; vin_out.ochn_attr_type = VIN_BASIC_ATTR;
    vin_out.vin_basic_attr.format = raw10;
    vin_out.vin_basic_attr.wstride = width * 2;
    vin_attr_ex_t vin_extra{};
    vin_extra.vin_attr_ex_mask = 0x80;
    vin_extra.ex_attr_type = static_cast<decltype(vin_extra.ex_attr_type)>(7);
    vin_extra.mclk_ex_attr.mclk_freq = 24000000;
    isp_attr_t isp{};
    isp.input_mode = DDR_MODE;
    isp.sensor_mode = ISP_NORMAL_M;
    isp.crop.w = width; isp.crop.h = height;
    isp_ichn_attr_t isp_in{};
    isp_in.width = width; isp_in.height = height;
    isp_in.fmt = FRM_FMT_RAW; isp_in.bit_width = 10;
    isp_ochn_attr_t isp_out{};
    isp_out.ddr_en = CAM_TRUE; isp_out.fmt = FRM_FMT_NV12; isp_out.bit_width = 8;
    vse_attr_t vse{};
    vse_ichn_attr_t vse_in{};
    vse_in.width = width; vse_in.height = height;
    vse_in.fmt = FRM_FMT_NV12; vse_in.bit_width = 8;
    vse_ochn_attr_t vse_out{};
    vse_out.chn_en = CAM_TRUE;
    vse_out.roi.w = width; vse_out.roi.h = height;
    vse_out.target_w = c->output_width; vse_out.target_h = c->output_height;
    vse_out.fmt = FRM_FMT_NV12; vse_out.bit_width = 8;
    vse_out.fps.src = 60; vse_out.fps.dst = c->output_fps;
    hbn_buf_alloc_attr_t buffers{};
    buffers.buffers_num = 6; buffers.is_contig = 1;
    buffers.flags = HB_MEM_USAGE_CPU_READ_OFTEN | HB_MEM_USAGE_CPU_WRITE_OFTEN | HB_MEM_USAGE_CACHED;

    STEP(hbn_camera_create(&c->config, &c->sensor)); c->sensor_open = true;
    STEP(hbn_vnode_open(HB_VIN, c->port, AUTO_ALLOC_ID, &c->vin)); c->vin_open = true;
    STEP(hbn_vnode_set_attr(c->vin, &vin));
    STEP(hbn_vnode_set_ichn_attr(c->vin, 0, &vin_in));
    STEP(hbn_vnode_set_ochn_attr(c->vin, 0, &vin_out));
    STEP(hbn_vnode_set_attr_ex(c->vin, &vin_extra));
    STEP(hbn_vnode_set_ochn_buf_attr(c->vin, 0, &buffers));
    STEP(hbn_vnode_open(HB_ISP, 0, AUTO_ALLOC_ID, &c->isp)); c->isp_open = true;
    STEP(hbn_vnode_set_attr(c->isp, &isp));
    STEP(hbn_vnode_set_ichn_attr(c->isp, 0, &isp_in));
    STEP(hbn_vnode_set_ochn_attr(c->isp, 0, &isp_out));
    STEP(hbn_vnode_set_ochn_buf_attr(c->isp, 0, &buffers));
    STEP(hbn_vnode_open(HB_VSE, 0, AUTO_ALLOC_ID, &c->vse)); c->vse_open = true;
    STEP(hbn_vnode_set_attr(c->vse, &vse));
    STEP(hbn_vnode_set_ichn_attr(c->vse, 0, &vse_in));
    STEP(hbn_vnode_set_ochn_attr(c->vse, 0, &vse_out));
    STEP(hbn_vnode_set_ochn_buf_attr(c->vse, 0, &buffers));
    STEP(hbn_vflow_create(&c->flow)); c->flow_open = true;
    STEP(hbn_vflow_add_vnode(c->flow, c->vin));
    STEP(hbn_vflow_add_vnode(c->flow, c->isp));
    STEP(hbn_vflow_add_vnode(c->flow, c->vse));
    STEP(hbn_vflow_bind_vnode(c->flow, c->vin, 0, c->isp, 0));
    STEP(hbn_vflow_bind_vnode(c->flow, c->isp, 0, c->vse, 0));
    STEP(hbn_camera_attach_to_vin(c->sensor, c->vin));
    return 0;
}

int X5Camera::start() {
    auto *c = state_.get();
    STEP(hbn_vflow_start(c->flow));
    c->started = true;
    return 0;
}

int X5Camera::acquire(Frame &frame, unsigned timeout_ms) {
    auto *c = state_.get();
    if (c->leased) return -1;
    int result = hbn_vnode_getframe(c->vse, 0, timeout_ms, &c->image);
    if (result) return result;
    c->leased = true;
    auto &buffer = c->image.buffer;
    if (buffer.plane_cnt < 2 || !buffer.virt_addr[0] || !buffer.virt_addr[1] ||
        buffer.width <= 0 || buffer.height <= 0 || buffer.stride <= 0 || buffer.vstride <= 0) {
        std::fprintf(stderr, "camera%u: VSE returned invalid NV12 planes\n", c->port);
        release();
        return -1;
    }
    if (check(hb_mem_invalidate_buf_with_vaddr(reinterpret_cast<uint64_t>(buffer.virt_addr[0]), buffer.size[0]),
              "invalidate Y", c->port) ||
        check(hb_mem_invalidate_buf_with_vaddr(reinterpret_cast<uint64_t>(buffer.virt_addr[1]), buffer.size[1]),
              "invalidate UV", c->port)) {
        release();
        return -1;
    }
    frame = {buffer.virt_addr[0], buffer.virt_addr[1],
             static_cast<size_t>(buffer.size[0]), static_cast<size_t>(buffer.size[1]),
             c->image.info.frame_id, static_cast<uint32_t>(buffer.width),
             static_cast<uint32_t>(buffer.height), static_cast<uint32_t>(buffer.stride),
             static_cast<uint32_t>(buffer.vstride), c->image.info.timestamps,
             c->image.info.sys_timestamps};
    return 0;
}

int X5Camera::release() {
    auto *c = state_.get();
    if (!c->leased) return 0;
    c->leased = false;
    return check(hbn_vnode_releaseframe(c->vse, 0, &c->image), "releaseframe", c->port);
}

int X5Camera::close() {
    auto *c = state_.get();
    int result = release();
    if (c->started && check(hbn_vflow_stop(c->flow), "stop flow", c->port)) result = -1;
    if (c->vse_open) hbn_vnode_close(c->vse);
    if (c->isp_open) hbn_vnode_close(c->isp);
    if (c->vin_open) hbn_vnode_close(c->vin);
    if (c->sensor_open && check(hbn_camera_destroy(c->sensor), "destroy camera", c->port)) result = -1;
    if (c->flow_open) hbn_vflow_destroy(c->flow);
    c->started = c->vse_open = c->isp_open = c->vin_open = c->sensor_open = c->flow_open = false;
    return result;
}
#undef STEP
}
