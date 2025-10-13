#ifndef _SRC_UTILITY_BUFFER_BUFFER_EQUALS_BUFFER_EQUALS_H_
#define _SRC_UTILITY_BUFFER_BUFFER_EQUALS_BUFFER_EQUALS_H_

#include <stdbool.h> // for bool
#include <string.h> // for strcmp

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>

bool buffer_equals(buffer_t *str1, buffer_t *str2);

#endif // _SRC_UTILITY_BUFFER_BUFFER_EQUALS_BUFFER_EQUALS_H_
