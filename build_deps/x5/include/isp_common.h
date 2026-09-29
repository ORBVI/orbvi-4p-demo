#ifndef __ISP_COMMON_H__
#define __ISP_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_types.h"


#define HBN_ISO_STRENGTH_NUM 20          /**< ISO strength number */
#define HBN_ISP_AUTO_LEVEL_MAX 20

typedef struct hbn_windows_s {
	uint32_t h_offset;	/**< Horizontal start offset */
	uint32_t v_offset;	/**< Vertical start offset */
	uint32_t width;		/**< Width */
	uint32_t height;	/**< Height */
} hbn_windows_t;

/* common define */
typedef enum enum_isp_mode {
	HBN_ISP_MODE_AUTO = 0,
	HBN_ISP_MODE_MANUAL,
	HBN_ISP_MODE_BUTT
} hbn_isp_mode_e;


#ifdef __cplusplus
}
#endif

#endif // __ISP_COMMON_H__