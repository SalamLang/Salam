#ifndef _UTILITY_STRING_STRING_ENDS_STRING_ENDS_H_
#define _UTILITY_STRING_STRING_ENDS_STRING_ENDS_H_

#include <stdbool.h>  // for bool
#include <stddef.h>   // for size_t
#include <string.h>   // for strncmp

// base
#include <base.h>

// string
#include <utility/string/string_length/string_length.h>

bool string_ends(const char* source, const char* search);

#endif  // _UTILITY_STRING_STRING_ENDS_STRING_ENDS_H_
