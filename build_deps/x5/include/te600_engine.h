/*
* Copyright 2016 The OpenSSL Project Authors. All Rights Reserved.
*
* Licensed under the OpenSSL license (the "License").  You may not use
* this file except in compliance with the License.  You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://www.openssl.org/source/license.html
*/

#ifndef __TE600_ENGINE_H__
# define __TE600_ENGINE_H__

void ENGINE_load_te600(void);

/**
 * @description: Get both the git commit hash and version of libte600_engine.
 * @return: A pointer to a static string containing both git commit hash and lib version.
 */
extern const char* hb_libte600_engine_get_version_info(void);

#endif // __TE600_ENGINE_H__
