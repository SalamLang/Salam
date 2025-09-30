#ifndef _UTILITY_BUFFER_BUFFER_COMPARE_BUFFER_COMPARE_H_
#define _UTILITY_BUFFER_BUFFER_COMPARE_BUFFER_COMPARE_H_

#include <stdbool.h>  // for bool
#include <string.h>   // for strcmp

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>

bool buffer_compare(buffer_t* str1, buffer_t* str2);

#endif  // _UTILITY_BUFFER_BUFFER_COMPARE_BUFFER_COMPARE_H_
