#ifndef _UTILITY_MEMORY_MEMORY_CALLOC_MEMORY_CALLOC_H_
#define _UTILITY_MEMORY_MEMORY_CALLOC_MEMORY_CALLOC_H_

#include <stddef.h> // for size_t
#include <stdlib.h> // for calloc, NULL

#include <utility/log/log_fatal/log_fatal.h>

void* memory_calloc(size_t count, size_t size);

#endif // _UTILITY_MEMORY_MEMORY_CALLOC_MEMORY_CALLOC_H_
