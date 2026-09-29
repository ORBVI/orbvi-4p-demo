#ifndef __ISP_TYPES_H__
#define __ISP_TYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include <float.h>

typedef float float32_t;
typedef double float64_t;
typedef long double float128_t;

typedef enum
{
    BOOL_FALSE = 0,
    BOOL_TRUE = (!BOOL_FALSE)
} bool_t;

#endif // __ISP_TYPES_H__