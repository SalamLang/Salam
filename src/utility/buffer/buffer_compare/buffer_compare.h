#ifndef _UTILITY_BUFFER_BUFFER_COMPARE_BUFFER_COMPARE_H_
#define _UTILITY_BUFFER_BUFFER_COMPARE_BUFFER_COMPARE_H_

#include <string.h> // for strcmp
#include <stdbool.h> // for bool

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>

bool buffer_compare(buffer_t *str1, buffer_t *str2);

#endif // _UTILITY_BUFFER_BUFFER_COMPARE_BUFFER_COMPARE_H_
