#ifndef _UTILITY_STRING_STRING_STARTS_STRING_STARTS_H_
#define _UTILITY_STRING_STRING_STARTS_STRING_STARTS_H_

#include <stdbool.h> // for bool
#include <string.h> // for strncmp

// base
#include <base.h>

// string
#include <utility/string/string_length/string_length.h>

bool string_starts(const char *source, const char *search);

#endif // _UTILITY_STRING_STRING_STARTS_STRING_STARTS_H_
