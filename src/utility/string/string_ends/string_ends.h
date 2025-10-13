#ifndef _SRC_UTILITY_STRING_STRING_ENDS_STRING_ENDS_H_
#define _SRC_UTILITY_STRING_STRING_ENDS_STRING_ENDS_H_

#include <stddef.h> // for size_t
#include <string.h> // for strncmp
#include <stdbool.h> // for bool

// base
#include <base.h>

// string
#include <utility/string/string_length/string_length.h>

bool string_ends(const char *source, const char *search);

#endif // _SRC_UTILITY_STRING_STRING_ENDS_STRING_ENDS_H_
