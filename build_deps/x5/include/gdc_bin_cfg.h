/***************************************************************************
 *                      COPYRIGHT NOTICE
 *             Copyright(C) 2024, D-Robotics Co., Ltd.
 *                     All rights reserved.
 ***************************************************************************/

#ifndef HB_GDC_DATA_INFO_H_
#define HB_GDC_DATA_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/time.h>

/**
 * @enum frame_format
 * frame format  @NO{S09E06C01U}
 */
typedef enum frame_format {
	FMT_UNKNOWN,
	FMT_LUMINANCE,
	FMT_PLANAR_444,
	FMT_PLANAR_420,
	FMT_SEMIPLANAR_420,
	FMT_GDC_MAX
} frame_format_t;

//---------------------------------------------------------
/**
 * ECC binary generation
 */
typedef enum {
	ECC_DISABLED, // ECC configuration is disabled
	ECC_ENABLED, // ONLY ECC configuration will be generated
	ECC_EMBEDDED // ECC configuration is ebmedded in the transformation configuration
} ecc_gen_t;
//---------------------------------------------------------

//---------------------------------------------------------
/**
 * GDC Rectangle definition
 */

/**
 * @struct rect_s
 * @brief GDC Rectangle definition
 * @NO{S09E06C01U}
 */
typedef struct rect_s {
	int32_t x; /**< Start x coordinate */
	int32_t y; /**< Start y coordinate */
	int32_t w; /**< width */
	int32_t h; /**< height */
} rect_t;
//---------------------------------------------------------
/**
 * GDC Types of supported transformations
 */

/**
 * @enum gdc_transformation
 * GDC Types of supported transformations  @NO{S09E06C01U}
 */
typedef enum gdc_transformation {
	PANORAMIC,
	CYLINDRICAL,
	STEREOGRAPHIC,
	UNIVERSAL,
	CUSTOM,
	AFFINE,
	DEWARP_KEYSTONE
} transformation_t;
//---------------------------------------------------------

//---------------------------------------------------------
/**
 * GDC Resolution definition
 */
/**
 * @struct resolution_s
 * @brief GDC Resolution definition
 * @NO{S09E06C01U}
 */
typedef struct resolution_s {
	uint32_t w; /**< width in pixels */
	uint32_t h; /**< height in pixels */
} resolution_t;
//---------------------------------------------------------
/**
 * Grid point
 */
/**
 * @struct point_s
 * @brief GDC Grid point
 * @NO{S09E06C01U}
 */
typedef struct point_s {
	double x; /**< x coordinate */
	double y; /**< y coordinate */
} point_t;

//---------------------------------------------------------
/**
 * Custom transformation structure
 */
/**
 * @struct custom_tranformation_s
 * @brief GDC custom transformation structure
 * @NO{S09E06C01U}
 */
typedef struct custom_tranformation_s {
	uint8_t full_tile_calc; /**< full tile calculation flag */
	uint16_t tile_incr_x; /**< tile increase in x  */
	uint16_t tile_incr_y; /**< tile increase in y */
	int32_t w; /**< Number or points in horizontal direction in the custom transformation grid */
	int32_t h; /**< Number or points in vertical direction in the custom transformation grid */
	double centerx; /**< center along x axis */
	double centery; /**< center along y axis */
	point_t *points; /**< array with points, number of elements = w*h */
} custom_tranformation_t;

//---------------------------------------------------------
/**
 * Window definition and transformation parameters
 * For parameters meaning read GDC guide
 */
/**
 * @struct window_s
 * @brief Window definition and transformation parameters.For parameters meaning read GDC guide
 * @NO{S09E06C01U}
 */
typedef struct window_s {
	rect_t out_r; /**< Output window position and size */
	transformation_t transform; /**< Used transformation */
	rect_t input_roi_r; /**< input roi */
	int32_t pan; /**< Target shift in horizontal direction from centre of the output image in pixels */
	int32_t tilt; /**< Target shift in vertical direction from centre of the output image in pixels */
	double zoom; /**< Target zoom dimensionless coefficient (must not be bigger than zero) */
	double strength; /**< Dimensionless non-negative parameter defining the strength of transformation along X axis */
	double strengthY; /**< Dimensionless non-negative parameter defining the strength of transformation along X axis */
	double angle; /**< Angle of main projection axis rotation around itself in degrees */
	double elevation; /**< Angle in degrees which specify the main projection axis */
	double azimuth; /**< Angle in degrees which specify the main projection axis, counted clockwise from North direction (positive to East)*/
	int32_t keep_ratio; /**<  Keep the same stretching strength in both horizontal and vertical directions */
	double FOV_h; /**< Size of output field of view in vertical dimension in degrees */
	double FOV_w; /**< Size of output field of view in horizontal dimension in degrees */
	double cylindricity_y; /**< Level of cylindricity for target projection shape in vertical direction */
	double cylindricity_x; /**< Level of cylindricity for target projection shape in horizontal direction */
	char custom_file[128]; /**< File name of the file containing custom transformation description */
	custom_tranformation_t custom; /**< Parsed custom transformation structure */
	double trapezoid_left_angle; /**< Left Acute angle in degrees between trapezoid base and leg */
	double trapezoid_right_angle; /**< Right Acute angle in degrees between trapezoid base and leg */
	uint8_t check_compute; /**< Perform fixed point and floating point result comparisons */
} window_t;
//---------------------------------------------------------
/**
 * Common parameters
 */

/**
 * @struct param_s
 * @brief GDC common parameters
 * @NO{S09E06C01U}
 */
typedef struct param_s {
	frame_format_t format; /**< frame format */
	resolution_t in; /**< input frame resolution */
	resolution_t out; /**< output frame resolution */
	int32_t x_offset; /**< center offset for input x coordinate */
	int32_t y_offset; /**< center offset for input y coordinate */
	int32_t diameter; /**< diameter of equivalent 180 field of view in pixels */
	double fov; /**< field of view */
} param_t;

//---------------------------------------------------------
/**
 * Grid parameters
 */
typedef struct {
	int32_t step_x; // step in pixels for X coordinate
	int32_t step_y; // step in pixels for Y coordinate
	uint64_t array_length; // length of allocated memory for array to fill points
	point_t *array; // array with points
} grid_t;
//---------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif // HB_GDC_DATA_INFO_H_
