#ifndef _UTILITY_MEMORY_MEMORY_ALLOCATION_MEMORY_ALLOCATION_H_
#define _UTILITY_MEMORY_MEMORY_ALLOCATION_MEMORY_ALLOCATION_H_

#include <stdlib.h> // for size_t
#include <stdio.h> // for malloc, NULL

#include <utility/log/log_fatal/log_fatal.h>

void* memory_allocation(size_t siz);

#endif // _UTILITY_MEMORY_MEMORY_ALLOCATION_MEMORY_ALLOCATION_H_
