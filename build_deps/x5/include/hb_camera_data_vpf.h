/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

/**
 * @file hb_camera_data_vpf.h
 *
 * @NO{S10E02C07}
 * @ASIL{B}
 */

#ifndef __HB_CAMERA_DATA_VPF_H__
#define __HB_CAMERA_DATA_VPF_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CAMERA_NOVPF
#include "vin_cfg.h"

#else /* CAMERA_NOVPF */
/* the ipi channel number and index */
#define MIPIHOST_CHANNEL_NUM (4)
#define MIPIHOST_CHANNEL_0   (0)
#define MIPIHOST_CHANNEL_1   (1)
#define MIPIHOST_CHANNEL_2   (2)
#define MIPIHOST_CHANNEL_3   (3)

/**
 * struct mipi_host_cfg_s - define the configuration to init mipi host
 * @NO{S10E03C01}
 *
 * @phy:	the phy type of host: dphy/cphy, Range: 0:1.
 * @lane:	number of mipi dphy data lane or cphy trio, Range: 1:4.
 * @datatype:	datatype of mipi data packages, Range: 0x00:0x3F.
 * @fps:	frames per second of mipi data to recive.
 * @mclk:	the clock of sensor which output from soc, set 0/1 to disable/enable
 *		only, set <=24 to ignore, set other to enable with freq=mclk*100KHz.
 * @mipiclk:	total bitrate of mipi host with all data lane, Unit: Mbps.
 * @width:	width of image resolution to recive, Unit: pixel.
 * @height:	height of image resolution to recive, Unit: line.
 * @linelenth:	total width lenth of image line to recive, Unit: pixel.
 * @framelenth:	total height lenth of image frame to recive, Unit: line.
 * @settle:	settle time of mipi host dphy, Range: 1:127.
 * @ppi_pg:	configuration of ppi pattern generator, 0 to disable:
 *			b0: set 1 only to enable ppi pg with default config;
 *			b1: set 0 to vertical mode, set 1 to horiontal mode;
 *			b2: set 0 to normal data tyep, set 1 to emb type;
 *			b3-4: vc index setting of ppi pg;
 * @hsaTime:	time of horizontal synchronism active for ipi, 0 as default.
 * @hbpTime:	time of horizontal back period for ipi, 0 as default.
 * @hsdTime:	time of horizontal sync porch delay period for ipi, 0 as default.
 * @channle_num:ipi channel number to be used, Range: 0:4.
 * @channle_sel:vc index of each ipi channel to recive, Range: 0:3.
 *
 */
typedef struct mipi_host_cfg_s {
	/* type must be: uint16_t */
	uint16_t phy;
	uint16_t lane;
	uint16_t datatype;
	uint16_t fps;
	uint16_t mclk;
	uint16_t mipiclk;
	uint16_t width;
	uint16_t height;
	uint16_t linelenth;
	uint16_t framelenth;
	uint16_t settle;
	uint16_t ppi_pg;
	uint16_t hsaTime;
	uint16_t hbpTime;
	uint16_t hsdTime;
	uint16_t channel_num;
	uint16_t channel_sel[MIPIHOST_CHANNEL_NUM];
} mipi_host_cfg_t;
#define MIPI_HOST_CFG_NUM       ((uint32_t)(sizeof(mipi_host_cfg_t)/sizeof(uint16_t)))

/* config name strings, see: struct mipi_host_cfg_s */
#define MIPI_HOST_CFG_STRINGS { \
	"phy", \
	"lane", \
	"datatype", \
	"fps", \
	"mclk", \
	"mipiclk", \
	"width", \
	"height", \
	"linelenth", \
	"framelenth", \
	"settle", \
	"ppi_pg", \
	"hsaTime", \
	"hbpTime", \
	"hsdTime", \
	"channel_num", \
	"channel_sel0", \
	"channel_sel1", \
	"channel_sel2", \
	"channel_sel3", \
}

/**
 * struct mipi_host_param_s - define the runtime parameters of mipi host
 * @NO{S10E03C01}
 *
 * @nocheck:	disable the check operation when init and start, Range: 0:1.
 * @notimeout:	disable the check timeout and wait forever, Range: 0:1.
 * @wait_ms:	the wait time of check operation, Unit: ms.
 * @dbg_value:	enable the debug print show, Range: 0:1.
 * @adv_value:	the config value of IPI adv_feature register.
 * @need_stop_check: disable the stop check operation, Range: 0:1.
 * @stop_check_instart:	do the stop check operation when start, Range: 0:1.
 * @cut_through: enable the cut_through mode for IPI, Range: 0:1.
 * @mem_flush:	enabel the auto flush function for IPI, Range: 0:1.
 * @data_ids_1:	debug monitor set of ids1.
 * @data_ids_2:	debug monotor set of ids2.
 * @data_ids_vc1: debug monitor set of ids_vc1.
 * @data_ids_vc2: debug monitor set of ids_vc2.
 * @ipi_16bit:	the value of IPI bit mode configuration.
 * @ipi_force:	force set the IPI clock freq, Unit: Hz.
 * @ipi_limit:	teh minimum limit of IPI clock freq set, Unit: Hz.
 * @ipi_overst:	enable the ipi reset operation when ipi overflow, Range: 0:1.
 * @pkt2pkt_time: manual set the pkt2pkt time value, Range: 0:255.
 * @snrclk_en:	enable the sensor mclk output.
 * @snrclk_freq: the sensor mclk output clock freq, Unit: Hz.
 * @vcext_en:	enable extern VC function, Range: 0:1.
 * @error_diag:	enable diag report of soft error.
 * @ipi1_dt:	force datatype config of ipi1.
 * @ipi2_dt:	force datatype config of ipi2.
 * @ipi3_dt:	force datatype config of ipi3.
 * @ipi4_dt:	force datatype config of ipi4.
 * @cfg_nocheck: disable config check function.
 * @drop_func:	frame drop when error function select stl/irq: Range: 0:1.
 * @drop_mask:	disable mask of error type for frame drop function.
 * @irq_cnt:	irq count limit set.
 * @irq_debug:	irq debug print level.
 * @stl_dbg:	stl debug print level.
 * @stl_mask:	stl function mask.
 * @stl_pile:	stl pile function for test.
 * @stl_ovif:	ignore frame count for ipi overflow error.
 * @stl_stif:	ignore frame count for start.
 * @fatal_ap:	fatal inject function select: fun/ap.
 *
 */
typedef struct mipi_host_param_s {
	/* type must be: uint32_t */
	uint32_t nocheck;
	uint32_t notimeout;
	uint32_t wait_ms;
	uint32_t dbg_value;
	uint32_t adv_value;
	uint32_t need_stop_check;
	uint32_t stop_check_instart;
	uint32_t cut_through;
	uint32_t mem_flush;
	uint32_t data_ids_1;
	uint32_t data_ids_2;
	uint32_t data_ids_vc1;
	uint32_t data_ids_vc2;
	uint32_t ipi_16bit;
	uint32_t ipi_force;
	uint32_t ipi_limit;
	uint32_t ipi_overst;
	uint32_t pkt2pkt_time;
	uint32_t snrclk_en;
	uint32_t snrclk_freq;
	uint32_t vcext_en;
	uint32_t error_diag;
	uint32_t ipi1_dt;
	uint32_t ipi2_dt;
	uint32_t ipi3_dt;
	uint32_t ipi4_dt;
	uint32_t cfg_nocheck;
	uint32_t drop_func;
	uint32_t drop_mask;
	uint32_t irq_cnt;
	uint32_t irq_debug;
	uint32_t stl_dbg;
	uint32_t stl_mask;
	uint32_t stl_pile;
	uint32_t stl_ovif;
	uint32_t stl_stif;
	uint32_t fatal_ap;
} mipi_host_param_t;

/* run params name strings, see: struct mipi_host_param_s */
#define MIPI_HOST_PARAM_STRINGS { \
	"nocheck", \
	"notimeout", \
	"wait_ms", \
	"dbg_value", \
	"adv_value", \
	"need_stop_check", \
	"stop_check_instart", \
	"cut_through", \
	"mem_flush", \
	"data_ids_1", \
	"data_ids_2", \
	"data_ids_vc1", \
	"data_ids_vc2", \
	"ipi_16bit", \
	"ipi_force", \
	"ipi_limit", \
	"ipi_overst", \
	"pkt2pkt_time", \
	"snrclk_en", \
	"snrclk_freq", \
	"vcext_en", \
	"error_diag", \
	"ipi1_dt", \
	"ipi2_dt", \
	"ipi3_dt", \
	"ipi4_dt", \
	"cfg_nocheck", \
	"drop_func", \
	"drop_mask", \
	"irq_cnt", \
	"irq_debug", \
	"stl_dbg", \
	"stl_mask", \
	"stl_pile", \
	"stl_ovif", \
	"stl_stif", \
	"fatal_ap", \
}
#define MIPI_HOST_PARAMS_NUM    ((uint32_t)(sizeof(struct mipi_host_param_s)/sizeof(uint32_t)))

/* the ipi channel number and index */
#define MIPIDEV_CHANNEL_NUM (4)
#define MIPIDEV_CHANNEL_0   (0)
#define MIPIDEV_CHANNEL_1   (1)
#define MIPIDEV_CHANNEL_2   (2)
#define MIPIDEV_CHANNEL_3   (3)

/**
 * struct mipi_dev_cfg_s - define the configuration to init mipi dev
 * @NO{S10E03C01}
 *
 * @lane:		number of mipi dphy data lane, Range: 1:4.
 * @datatype:	datatype of mipi data packages, Range: 0x00:0x3F.
 * @fps:		frames per second of mipi data to send.
 * @mclk:		ignored for mipi dev.
 * @mipiclk:	total bitrate of mipi dev with all data lane, Unit: Mbps.
 * @width:		width of image resolution to send, Unit: pixel.
 * @height:		height of image resolution to send, Unit: line.
 * @linelenth:	total width lenth of image line to send, Unit: pixel.
 * @framelenth:	total height lenth of image frame to send, Unit: line.
 * @settle:		settle time of mipi dev dphy, Range: 1:127.
 * @vpg:		configuration of video pattern generator, 0 to disable:
 *					b0: set 1 only to enable vpg with default config;
 *					b1: set 0 to vertical mode, set 1 to horiontal mode;
 *					b2: set 0 to BER pattern, set 1 to Color bar;
 *					b3-4: vc index setting of vpg;
 * @ipi_lines:	lines setting of ipi channel to send, 0 as height+1.
 * @channle_num:	ipi channel number to be used, Range: 0:4.
 * @channle_sel:	vc index of each ipi channel to recive, Range: 0:3.
 *
 */
typedef struct mipi_dev_cfg_s {
	/* type must be: uint16_t */
	uint16_t lane;
	uint16_t datatype;
	uint16_t fps;
	uint16_t mclk;
	uint16_t mipiclk;
	uint16_t width;
	uint16_t height;
	uint16_t linelenth;
	uint16_t framelenth;
	uint16_t settle;
	uint16_t vpg;
	uint16_t ipi_lines;
	uint16_t channel_num;
	uint16_t channel_sel[MIPIDEV_CHANNEL_NUM];
} mipi_dev_cfg_t;
#define MIPI_DEV_CFG_NUM        ((uint32_t)(sizeof(mipi_dev_cfg_t)/sizeof(uint16_t)))

/* config name strings, see: struct mipi_dev_cfg_s */
#define MIPI_DEV_CFG_STRINGS { \
	"lane", \
	"datatype", \
	"fps", \
	"mclk", \
	"mipiclk", \
	"width", \
	"height", \
	"linelenth", \
	"framelenth", \
	"settle", \
	"vpg", \
	"ipi_lines", \
	"channel_num", \
	"channel_sel0", \
	"channel_sel1", \
	"channel_sel2", \
	"channel_sel3", \
}

/**
 * struct mipi_dev_param_s - define the runtime parameters of mipi dev
 * @NO{S10E03C01}
 *
 * @nocheck:	disable the check operation when init and start, Range: 0:1.
 * @notimeout:	disable the check timeout and wait forever, Range: 0:1.
 * @wait_ms:	the wait time of check operation, Unit: ms.
 * @dbg_value:	enable the debug print show, Range: 0:1.
 * @power_instart: do the power check when start, Range: 0:1.
 * @hsync_pkt:	enable the hsync packet of IPI, Range: 0:1.
 * @init_retry:	enable retry when init failed , Range: 0:1.
 * @ipi_force:	force set the IPI clock freq, Unit: Hz.
 * @ipi_limit:	teh minimum limit of IPI clock freq set, Unit: Hz.
 * @error_diag:	enable diag report of soft error.
 * @ipi1_dt:	force datatype config of ipi1.
 * @ipi2_dt:	force datatype config of ipi2.
 * @ipi3_dt:	force datatype config of ipi3.
 * @ipi4_dt:	force datatype config of ipi4.
 * @cfg_nocheck: disable config check function.
 * @irq_cnt:	irq count limit set.
 * @irq_debug:	irq debug print level.
 * @stl_dbg:	stl debug print level.
 * @stl_mask:	stl function mask.
 * @stl_pile:	stl pile function for test.
 * @fatal_ap:	fatal inject function select: fun/ap.
 * @txout_param_valid: set the txout param vaild, Range: 0:1.
 * @txout_freq_mode: set the txout freq mode as auto/cal, Range: 0:1.
 * @txout_freq_autolarge_enbale: enable the txout freq auto large, Range: 0:1.
 * @txout_freq_gain_precent: set the txout freq gain precent, Range: 0:100.
 * @txout_freq_force: set the txout freq force, Unit: Hz.
 *
 */
typedef struct mipi_dev_param_s {
	/* type must be: uint32_t */
	uint32_t nocheck;
	uint32_t notimeout;
	uint32_t wait_ms;
	uint32_t dbg_value;
	uint32_t power_instart;
	uint32_t hsync_pkt;
	uint32_t init_retry;
	uint32_t ipi_force;
	uint32_t ipi_limit;
	uint32_t error_diag;
	uint32_t ipi1_dt;
	uint32_t ipi2_dt;
	uint32_t ipi3_dt;
	uint32_t ipi4_dt;
	uint32_t cfg_nocheck;
	uint32_t irq_cnt;
	uint32_t irq_debug;
	uint32_t stl_dbg;
	uint32_t stl_mask;
	uint32_t stl_pile;
	uint32_t fatal_ap;
	uint32_t txout_param_valid;
	uint32_t txout_freq_mode;
	uint32_t txout_freq_autolarge_enbale;
	uint32_t txout_freq_gain_precent;
	uint32_t txout_freq_force;
} mipi_dev_param_t;
#define MIPI_DEV_PARAMS_NUM     ((uint32_t)(sizeof(struct mipi_dev_param_s)/sizeof(uint32_t)))

/* run params name strings, see: struct mipi_host_param_s */
#define MIPI_DEV_PARAM_STRINGS { \
	"nocheck", \
	"notimeout", \
	"wait_ms", \
	"dbg_value", \
	"power_instart", \
	"hsync_pkt", \
	"init_retry", \
	"ipi_force", \
	"ipi_limit", \
	"error_diag", \
	"ipi1_dt", \
	"ipi2_dt", \
	"ipi3_dt", \
	"ipi4_dt", \
	"cfg_nocheck", \
	"irq_cnt", \
	"irq_debug", \
	"stl_dbg", \
	"stl_mask", \
	"stl_pile", \
	"fatal_ap", \
	"txout_param_valid", \
	"txout_freq_mode", \
	"txout_freq_autolarge_enbale", \
	"txout_freq_gain_precent", \
	"txout_freq_force", \
}
#endif /* CAMERA_NOVPF */

/* define for vin fd of vpf */
typedef int64_t vpf_handle_t;

/**
 * @struct mipi_bypass_s
 * the mipi bypass config struct
 * @NO{S10E02C01}
 */
typedef struct mipi_bypass_s {
	int32_t tx_index;		// TX设备选择.
	struct mipi_dev_cfg_s tx_attr;	// TX设备属性.
	uint64_t tx_ex_mask;		// TX增强属性掩码.
	struct mipi_dev_param_s tx_attr_ex; // TX增强属性;
	uint32_t end_flag;	// end flag of config struct.
} mipi_bypass_t;

/**
 * @struct mipi_config_s
 * the mipi config with rx and bypass tx struct
 * @NO{S10E02C01}
 */
typedef struct mipi_config_s {
	int32_t rx_enable;		// RX设备使能.
	struct mipi_host_cfg_s rx_attr;	// RX设备属性.
	uint64_t rx_ex_mask;		// RX增强属性掩码.
	struct mipi_host_param_s rx_attr_ex;// RX增强属性;
	struct mipi_bypass_s *bypass;	// RX->TX bypass使能;
	uint32_t end_flag;	// end flag of config struct.
} mipi_config_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HB_CAMERA_DATA_VPF_H__ */
