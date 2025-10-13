#ifndef _SRC_UTILITY_STRING_STRING_RTRIM_STRING_RTRIM_H_
#define _SRC_UTILITY_STRING_STRING_RTRIM_STRING_RTRIM_H_

#include <ctype.h> // for isspace

// base
#include <base.h>

// string
#include <utility/string/string_length/string_length.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>
#include <utility/memory/memory_copy/memory_copy.h>
#include <utility/memory/memory_destroy/memory_destroy.h>

char* string_rtrim(const char* input);

#endif // _SRC_UTILITY_STRING_STRING_RTRIM_STRING_RTRIM_H_
