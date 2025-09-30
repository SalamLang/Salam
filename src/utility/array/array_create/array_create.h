#ifndef _UTILITY_ARRAY_ARRAY_CREATE_ARRAY_CREATE_H_
#define _UTILITY_ARRAY_ARRAY_CREATE_ARRAY_CREATE_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>
#include <utility/memory/memory_allocation_soft/memory_allocation_soft.h>
#include <utility/memory/memory_destroy/memory_destroy.h>

// array
#include <utility/array/type.h>

array_t* array_create(array_destroy_t destroyer);

#endif  // _UTILITY_ARRAY_ARRAY_CREATE_ARRAY_CREATE_H_
