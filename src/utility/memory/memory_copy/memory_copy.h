#ifndef _UTILITY_MEMORY_MEMORY_COPY_MEMORY_COPY_H_
#define _UTILITY_MEMORY_MEMORY_COPY_MEMORY_COPY_H_

#include <stddef.h>  // for size_t
#include <stdlib.h>  // for NULL
#include <string.h>  // for memcpy

// base
#include <base.h>

void* memory_copy(void* destination, const void* source, size_t size);

#endif  // _UTILITY_MEMORY_MEMORY_COPY_MEMORY_COPY_H_
