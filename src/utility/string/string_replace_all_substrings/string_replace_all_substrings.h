#ifndef _SRC_UTILITY_STRING_STRING_REPLACE_ALL_SUBSTRINGS_STRING_REPLACE_ALL_SUBSTRINGS_H_
#define _SRC_UTILITY_STRING_STRING_REPLACE_ALL_SUBSTRINGS_STRING_REPLACE_ALL_SUBSTRINGS_H_

#include <string.h> // for strstr, strncpy, strcpy

// base
#include <base.h>

// string
#include <utility/string/string_length/string_length.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>

char *string_replace_all_substrings(const char *str, const char *old_substr, const char *new_substr);

#endif // _SRC_UTILITY_STRING_STRING_REPLACE_ALL_SUBSTRINGS_STRING_REPLACE_ALL_SUBSTRINGS_H_
