#ifndef __ISP_WDR_H__
#define __ISP_WDR_H__

#ifdef __cplusplus
extern "C" {
#endif


#define HBN_WDR_LIGHT_THR_LOG_SIZE 4   /**< bin number of light threshold log size */
#define HBN_WDR_FLAT_LEVEL_CHANNAL_SIZE 4    /**< Bin number of flat level channal size */
#define HBN_WDR_FLAT_LEVEL_INC_SIZE 17    /**< Bin number of flat level size */
#define HBN_WDR_COLOR_WEIGHT_SIZE 3    /**< Bin number of color weight size */
#define HBN_WDR_RGB_COEF_SIZE 3    /**< Bin number of RGB coefficient size */


/******************************************************************************/
/**
 * @brief    WDR strength configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_strength_attr_s
{
    uint8_t strength;    /**< WDR strength */
    uint8_t high_strength;    /**< WDR high strength */
    uint16_t low_strength;    /**< WDR low strength */
} hbn_isp_wdr_strength_attr_t;

/******************************************************************************/
/**
 * @brief    WDR LTM weight configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_ltm_weight_attr_s
{
    int16_t contrast;    /**< WDR contrast */
    bool_t entropy_enable;    /**< WDR entropy enable */
    uint16_t entropy_base;    /**< WDR factor for all pixel based on image base */
    uint16_t entropy_slope;    /**< WDR factor for all pixel based on image slope */
    uint16_t wdr_luma_thr;    /**< WDR luma threshold */
} hbn_isp_wdr_ltm_weight_attr_t;

/******************************************************************************/
/**
 * @brief    WDR LTM configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_ltm_attr_s
{
    bool_t flat_mode;    /**< WDR flat mode */
    uint8_t flat_evel;    /**< WDR flat level */
    uint8_t flat_level_inc[HBN_WDR_FLAT_LEVEL_CHANNAL_SIZE][HBN_WDR_FLAT_LEVEL_INC_SIZE];    /**< WDR flat level increase */
    uint8_t dark_attention_level;    /**< WDR dark attention level */
} hbn_isp_wdr_ltm_attr_t;

/******************************************************************************/
/**
 * @brief    WDR GTM configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_gtm_weight_config_s
{
    uint16_t fixed_weight;    /**< WDR fixed weight */
} hbn_isp_wdr_gtm_weight_config_t;

/******************************************************************************/
/**
 * @brief    WDR GTM configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_gtm_attr_s
{
    uint8_t wdr_curve_select;    /**< WDR curve select */
    uint8_t log_weight;    /**< WDR log weight */
    float log_anchor_xlog;    /**< WDR anchor X log */
    uint16_t log_anchor_slope;    /**< WDR anchor slope */
    float ratio_actual;    /**< WDR actual ratio */
    float curve2_thr;    /**< WDR curve2 threshold */
    float curve2_losat;    /**< WDR curve2 low saturation */
    float curve2_lofactor;    /**< WDR curve2 low factor */
    float curve2_hifactor;    /**< WDR curve2 high factor */
    uint8_t flat_level_global;    /**< WDR global flat level */
} hbn_isp_wdr_gtm_attr_t;

/******************************************************************************/
/**
 * @brief    WDR degamma configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_degamma_attr_s
{
    float degamma;    /**< WDR degamma */
} hbn_isp_wdr_degamma_attr_t;

/******************************************************************************/
/**
 * @brief    WDR gain limitation configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_gain_limitaion_attr_s
{
    uint16_t max_gain;   /**< WDR max gain */
    uint16_t min_gain;    /**< WDR min gain */
} hbn_isp_wdr_gain_limitaion_attr_t;

/******************************************************************************/
/**
 * @brief    WDR defringe configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_defringe_attr_s
{
    float diff_high;    /**< WDR high difference */
    float diff_low;    /**< WDR low difference */
} hbn_isp_wdr_defringe_attr_t;

/******************************************************************************/
/**
 * @brief    WDR saturation adjustment configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_saturation_adjustment_attr_s
{
    float sat_range;    /**< WDR saturation range */
    uint16_t sat_thr_gain_down;   /**< WDR saturation threshold gain down */
    uint16_t sat_thr_gain_up;   /**< WDR saturation threshold gain up */
} hbn_isp_wdr_saturation_adjustment_attr_t;

/******************************************************************************/
/**
 * @brief    WDR halo color fading configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_halo_color_fading_attr_s
{
    uint8_t light_sat_lothr;    /**< WDR light saturation low threshold */
    uint8_t light_sat_hithr;    /**< WDR light saturation high threshold */
    float light_red_thr_log[HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR red light threshold log */
    float light_green_thr_log[HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR green light threshold log */
    float light_blue_thr_log[HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR blue light threshold log */
    float light_yellow_thr_log[HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR yellow light threshold log */
    float light_cyan_thr_log[HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR cyan light threshold log */
    float light_magenta_thr_log[HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR magenta light threshold log */
} hbn_isp_wdr_halo_color_fading_attr_t;

/******************************************************************************/
/**
 * @brief    WDR high light configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_high_light_attr_s
{
    float hlc_base_log;    /**< WDR HLC base log */
    uint8_t hlc_slope;    /**< WDR HLC slope */
} hbn_isp_wdr_high_light_attr_t;

/******************************************************************************/
/**
 * @brief    WDR dump configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_damp_attr_s
{
    uint8_t damp_mode;    /**< WDR damp mode */
    uint8_t damp_curve_coef;    /**< WDR curve coefficient */
    uint8_t damp_curve_max;    /**< WDR max damp curve */
    uint8_t damp_curve_min;    /**< WDR min damp curve */
    uint8_t damp_avg_coef;    /**< WDR average coefficient */
    uint8_t damp_avg_max;    /**< WDR max average */
    uint8_t damp_avg_min;    /**< WDR min average */
    uint8_t damp_coef_dec_limit;    /**< WDR damp coefficient decrease limit */
    uint8_t damp_coef_inc_limit;    /**< WDR damp coefficient increase limit */
    uint8_t damp_filter_size;    /**< WDR damp filter size */
    float damp_hithr_log;    /**< WDR damp high threshold log */
    float damp_lothr_log;    /**< WDR damp low threshold log */
} hbn_isp_wdr_damp_attr_t;

/******************************************************************************/
/**
 * @brief    WDR color weight configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_color_weight_attr_s
{
    uint8_t wdr_rgb_coef[HBN_WDR_RGB_COEF_SIZE];    /**< WDR RGB coefficient */
    uint8_t wdr_lightness_weight;    /**< WDR lightness weight */
    uint8_t wdr_color_weight[HBN_WDR_COLOR_WEIGHT_SIZE];    /**< WDR color weight */
} hbn_isp_wdr_color_weight_attr_t;

/******************************************************************************/
/**
 * @brief    WDR auto configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_auto_attr_s
{
    uint8_t auto_level;    /**< WDR auto configuration level */
    float gain[HBN_ISO_STRENGTH_NUM];    /**< WDR gain */
    uint8_t high_strength[HBN_ISO_STRENGTH_NUM];    /**< WDR high strength */
    uint16_t low_strength[HBN_ISO_STRENGTH_NUM];    /**< WDR low strength */
    uint16_t entropy_base[HBN_ISO_STRENGTH_NUM];    /**< WDR factor for all pixel based on image base */
    uint16_t wdr_luma_thr[HBN_ISO_STRENGTH_NUM];    /**< WDR luma threshold */
    uint16_t entropy_slope[HBN_ISO_STRENGTH_NUM];    /**< WDR factor for all pixel based on image slope */
    uint8_t flat_level[HBN_ISO_STRENGTH_NUM];    /**< WDR flat level */
    uint8_t flat_level_global[HBN_ISO_STRENGTH_NUM];    /**< WDR global flat level */
    float sat_range[HBN_ISO_STRENGTH_NUM];    /**< WDR saturation range */
    uint16_t sat_thr_gaindown[HBN_ISO_STRENGTH_NUM];   /**< WDR saturation threshold gain down */
    uint16_t sat_thr_gainup[HBN_ISO_STRENGTH_NUM];   /**< WDR saturation threshold gain up */
    float degamma[HBN_ISO_STRENGTH_NUM];    /**< WDR degamma value */
    float hlc_base_log[HBN_ISO_STRENGTH_NUM];    /**< WDR HLC base log */
    uint8_t light_sat_lothr[HBN_ISO_STRENGTH_NUM];    /**< WDR light saturation low threshold */
    uint8_t light_sat_hithr[HBN_ISO_STRENGTH_NUM];    /**< WDR light saturation high threshold */

    float light_red_thrlog[HBN_ISO_STRENGTH_NUM][HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR red light threshold log */
    float light_green_thrlog[HBN_ISO_STRENGTH_NUM][HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR green light threshold log */
    float light_blue_thrlog[HBN_ISO_STRENGTH_NUM][HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR blue light threshold log */
    float light_yellow_thrlog[HBN_ISO_STRENGTH_NUM][HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR yellow light threshold log */
    float light_cyan_thrlog[HBN_ISO_STRENGTH_NUM][HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR cyan light threshold log */
    float light_magenta_thrlog[HBN_ISO_STRENGTH_NUM][HBN_WDR_LIGHT_THR_LOG_SIZE];    /**< WDR magenta light threshold log */

    int16_t contrast[HBN_ISO_STRENGTH_NUM];    /**< WDR contrast */
    uint16_t fixed_weight[HBN_ISO_STRENGTH_NUM];    /**< WDR fixed weight */
    uint8_t log_weight[HBN_ISO_STRENGTH_NUM];    /**< WDR log weight */
    float log_anchor_xlog[HBN_ISO_STRENGTH_NUM];   /**< WDR anchor X log */
    uint16_t log_anchor_slope[HBN_ISO_STRENGTH_NUM];   /**< WDR anchor slope */
    uint16_t max_gain[HBN_ISO_STRENGTH_NUM];  /**< WDR max gain */
    uint8_t hlc_slope[HBN_ISO_STRENGTH_NUM];   /**< WDR HLC slope */
    bool_t light_en[HBN_ISO_STRENGTH_NUM];  /**< WDR light enable configuration */
    uint8_t wdr_color_weight[HBN_ISO_STRENGTH_NUM][HBN_WDR_COLOR_WEIGHT_SIZE];  /**< WDR color weight */
} hbn_isp_wdr_auto_attr_t;

/******************************************************************************/
/**
 * @brief    WDR current configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_manual_attr_s {
    hbn_isp_wdr_strength_attr_t strength_attr;   /**< WDR strength configuration */
    hbn_isp_wdr_ltm_weight_attr_t ltm_weight_attr;   /**< WDR LTM weight configuration */
    hbn_isp_wdr_ltm_attr_t ltm_attr;   /**< WDR LTM configuration */
    hbn_isp_wdr_gtm_weight_config_t gtm_weight_attr;   /**< WDR GTM weight configuration */
    hbn_isp_wdr_gtm_attr_t gtm_attr;   /**< WDR GTM configuration */
    hbn_isp_wdr_degamma_attr_t degamma_attr;   /**< WDR degamma configuration */
    hbn_isp_wdr_gain_limitaion_attr_t gain_limitation_attr;   /**< WDR gain limitation configuration */
    hbn_isp_wdr_defringe_attr_t defringe_attr;   /**< WDR defringe configuration */
    hbn_isp_wdr_saturation_adjustment_attr_t sat_adjust_attr;   /**< WDR saturation adjustment configuration */
    hbn_isp_wdr_halo_color_fading_attr_t halo_color_fading_attr;   /**< WDR halo color configuration */
    hbn_isp_wdr_high_light_attr_t high_light_attr;   /**< WDR high light configuration */
    bool_t light_en;  /**< WDR light en configuration */
    hbn_isp_wdr_damp_attr_t damp_attr;   /**< WDR damp configuration */
    hbn_isp_wdr_color_weight_attr_t color_weight_attr;   /**< WDR color weight configuration */
}hbn_isp_wdr_manual_attr_t;

/******************************************************************************/
/**
 * @brief    WDR configuration.
 *
 *****************************************************************************/
typedef struct hbn_isp_wdr_attr_s {
    hbn_isp_mode_e mode;      /**< The run mode: 0--manual, 1--auto */
    hbn_isp_wdr_auto_attr_t auto_attr;      /**< WDR auto configuration*/
    hbn_isp_wdr_manual_attr_t manual_attr;   /**< WDR manual configuration*/
} hbn_isp_wdr_attr_t;


#ifdef __cplusplus
}
#endif

#endif // __ISP_WDR_H__
