#ifndef _UTILITY_BUFFER_BUFFER_APPEND_STR_BUFFER_APPEND_STR_H_
#define _UTILITY_BUFFER_BUFFER_APPEND_STR_BUFFER_APPEND_STR_H_

#include <string.h> // for strcpy

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>

// string
#include <utility/string/string_length/string_length.h>

// memory
#include <utility/memory/memory_realloc/memory_realloc.h>

void buffer_append_str(buffer_t *str, const char *suffix);

#endif // _UTILITY_BUFFER_BUFFER_APPEND_STR_BUFFER_APPEND_STR_H_
