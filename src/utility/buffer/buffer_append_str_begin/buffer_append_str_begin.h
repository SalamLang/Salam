#include <string.h> // for memcpy

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>

// memory
#include <utility/memory/memory_move/memory_move.h>
#include <utility/memory/memory_realloc/memory_realloc.h>

void buffer_append_str_begin(buffer_t *str, const char *prefix);
