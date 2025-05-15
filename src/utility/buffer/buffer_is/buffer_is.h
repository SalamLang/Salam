#ifndef _UTILITY_BUFFER_BUFFER_IS_BUFFER_IS_H_
#define _UTILITY_BUFFER_BUFFER_IS_BUFFER_IS_H_

#include <stdbool.h> // for bool
#include <string.h> // for strcmp

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>

// string
#include <utility/string/string_length/string_length.h>

bool buffer_is(buffer_t *str1, const char *str2);

#endif // _UTILITY_BUFFER_BUFFER_IS_BUFFER_IS_H_
