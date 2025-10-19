#ifndef _SRC_UTILITY_MEMORY_MEMORY_ALLOCATION_SOFT_MEMORY_ALLOCATION_SOFT_H_
#define _SRC_UTILITY_MEMORY_MEMORY_ALLOCATION_SOFT_MEMORY_ALLOCATION_SOFT_H_

#include <stdlib.h> // for size_t
#include <stdio.h> // for malloc, NULL

// base
#include <base.h>

// log
#include <utility/log/log_error/log_error.h>

void* memory_allocation_soft(size_t siz);

#endif // _SRC_UTILITY_MEMORY_MEMORY_ALLOCATION_SOFT_MEMORY_ALLOCATION_SOFT_H_
