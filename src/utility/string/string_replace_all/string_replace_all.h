#ifndef _SRC_UTILITY_STRING_STRING_REPLACE_ALL_STRING_REPLACE_ALL_H_
#define _SRC_UTILITY_STRING_STRING_REPLACE_ALL_STRING_REPLACE_ALL_H_

#include <stddef.h> // for size_t
#include <string.h> // for strcpy

// base
#include <base.h>

// string
#include <utility/string/string_length/string_length.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>

char *string_replace_all(const char *str, const char *old_substr, const char *new_substr);

#endif // _SRC_UTILITY_STRING_STRING_REPLACE_ALL_STRING_REPLACE_ALL_H_
