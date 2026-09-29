#ifndef  _DROBOT_EFUSE_
#include <stdint.h>
#include <stdbool.h>

enum efuse_type {
	EFUSE_SECURE = 0,
	EFUSE_NONSECURE
};

struct efuse_info {
    enum efuse_type type;
    uint32_t bank;
    uint32_t value;
    bool lock;
};
int drobot_efuse_write(struct efuse_info *efuse);
int drobot_efuse_read(struct efuse_info *efuse);
int drobot_efuse_update_key_hash_and_sec_boot(const char *hash_file);
int drobot_efuse_update_key_hash(const char *hash_file);
int drobot_efuse_update_sec_boot(void);
int drobot_efuse_dump_key_hash_and_sec_boot(uint32_t *hash_data,
							uint32_t hash_size, uint32_t *is_sec_boot);
int32_t drobot_efuse_sec_anti_read(uint32_t *ver);
int32_t drobot_efuse_nosec_anti_read(uint32_t *ver);
int32_t drobot_efuse_sec_anti_write(uint32_t ver);
int32_t drobot_efuse_nosec_anti_write(uint32_t ver);

/**
 * @description: Get both the git commit hash and version of libefuse.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libefuse_get_version_info(void);

#endif
