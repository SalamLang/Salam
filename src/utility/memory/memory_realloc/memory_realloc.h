#ifndef _UTILITY_MEMORY_MEMORY_REALLOC_MEMORY_REALLOC_H_
#define _UTILITY_MEMORY_MEMORY_REALLOC_MEMORY_REALLOC_H_

#include <stddef.h>  // for size_t
#include <stdlib.h> // for realloc

#include <utility/log/log_fatal/log_fatal.h>

void* memory_realloc(void* ptr, size_t new_size);

#endif // _UTILITY_MEMORY_MEMORY_REALLOC_MEMORY_REALLOC_H_
