#ifndef _SRC_UTILITY_STRING_STRING_TRIM_STRING_TRIM_H_
#define _SRC_UTILITY_STRING_STRING_TRIM_STRING_TRIM_H_

#include <ctype.h> // for

// base
#include <base.h>

// string
#include <utility/string/string_length/string_length.h>
#include <utility/string/string_ltrim/string_ltrim.h>
#include <utility/string/string_rtrim/string_rtrim.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>
#include <utility/memory/memory_copy/memory_copy.h>
#include <utility/memory/memory_destroy/memory_destroy.h>

char* string_trim(const char* input);

#endif // _SRC_UTILITY_STRING_STRING_TRIM_STRING_TRIM_H_
