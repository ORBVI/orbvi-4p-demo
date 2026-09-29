/**
 * @file drobot_secure_storage.h
 *
 * @NO{S21E05C02}
 * @ASIL{B}
 */
#ifndef DROBOT_SECURE_STORAGE_H
#define DROBOT_SECURE_STORAGE_H


#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_SECURE_PREFIX				"/storage/secure"	/**< default secure prefix */
#define DEFAULT_SECURE_PREFIX_BAK			"/storage/secure_bak_"	/**< default secure backup prefix */

#ifndef LOG_TAG
#define LOG_TAG "st_log"	/**< ALOGE show */
#endif

#define HBST_OK						0	/**< success return */

#define HBST_ERR_PARAM					-1	/**< error input parameter */
#define HBST_ERR_KEYNAME				-2	/**< illegal file path */
#define HBST_ERR_MALLOC					-3	/**< malloc fail */

#define HBST_ERR_OPEN					-4	/**< open file or object fail */
#define HBST_ERR_FILEOPEN				HBST_ERR_OPEN
#define HBST_ERR_DELETE					-5	/**< delete file or object fail */
#define HBST_ERR_WR					-6	/**< write file or object fail */
#define HBST_ERR_RD					-7	/**< read file or object fail */
#define HBST_ERR_FILERD					HBST_ERR_RD
#define HBST_ERR_NULLOBJ				-8	/**< null object */

/* Storage is provided by the Rich Execution Environment (REE) */
#define SECURE_STORAGE_PRIVATE_REE		1u
/* Storage is the Replay Protected Memory Block partition of an eMMC device */
#define SECURE_STORAGE_PRIVATE_RPMB		0u

#define HB_MAX_PATH_LEN 128u

#define HB_INVALID_KEY_ID      (-1)

int64_t drobot_st_file_load(const char *file_name, char **output, uint32_t *len, uint32_t store_mode);
int64_t drobot_st_file_store(const char *file_name, char *input, uint32_t len, uint32_t store_mode);
int32_t drobot_force_rpmb_key_ready(void);
bool drobot_is_st_file_exist(const char *file_name, uint32_t store_mode);
int32_t drobot_get_device_unlock(bool *state);
int32_t drobot_set_device_unlock(bool state);
int64_t drobot_st_file_delete(const char *file_name, uint32_t store_mode);
int64_t drobot_st_file_rename(const char *old_name, const char *new_name, uint32_t store_mode);
#ifdef __cplusplus
}
#endif

/**
 * @description: Get both the git commit hash and version of libsecure_storage.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libsecure_storage_get_version_info(void);

#endif  //HB_ST_KEY_MANAGER_H

