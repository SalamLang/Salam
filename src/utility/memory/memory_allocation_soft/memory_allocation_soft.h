#ifndef _UTILITY_MEMORY_MEMORY_ALLOCATION_SOFT_MEMORY_ALLOCATION_SOFT_H_
#define _UTILITY_MEMORY_MEMORY_ALLOCATION_SOFT_MEMORY_ALLOCATION_SOFT_H_

#include <stdio.h>   // for malloc, NULL
#include <stdlib.h>  // for size_t

// base
#include <base.h>

// log
#include <utility/log/log_error/log_error.h>

void* memory_allocation_soft(size_t siz);

#endif  // _UTILITY_MEMORY_MEMORY_ALLOCATION_SOFT_MEMORY_ALLOCATION_SOFT_H_
