#ifndef HB_POWERCTL_OUT_VERSION_H
#define HB_POWERCTL_OUT_VERSION_H

/**
 * @description: Get both the git commit hash and version of libpowerctl.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libpowerctl_get_version_info(void);

#endif // HB_POWERCTL_OUT_VERSION_H
