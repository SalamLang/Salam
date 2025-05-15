#ifndef _UTILITY_BUFFER_BUFFER_APPEND_STR_BEGIN_BUFFER_APPEND_STR_BEGIN_H_
#define _UTILITY_BUFFER_BUFFER_APPEND_STR_BEGIN_BUFFER_APPEND_STR_BEGIN_H_

#include <string.h> // for memcpy

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>

// memory
#include <utility/memory/memory_move/memory_move.h>
#include <utility/memory/memory_realloc/memory_realloc.h>

void buffer_append_str_begin(buffer_t *str, const char *prefix);

#endif // _UTILITY_BUFFER_BUFFER_APPEND_STR_BEGIN_BUFFER_APPEND_STR_BEGIN_H_
