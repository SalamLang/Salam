// memory
#include <utility/memory/memory_allocation/memory_allocation.h>
#include <utility/memory/memory_allocation_soft/memory_allocation_soft.h>
#include <utility/memory/memory_destroy/memory_destroy.h>
// array
#include <utility/array/type.h>

array_t* array_create(array_free_func_t freer);
