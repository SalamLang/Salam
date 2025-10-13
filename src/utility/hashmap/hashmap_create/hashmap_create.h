#ifndef _SRC_UTILITY_HASHMAP_HASHMAP_CREATE_HASHMAP_CREATE_H_
#define _SRC_UTILITY_HASHMAP_HASHMAP_CREATE_HASHMAP_CREATE_H_

#include <utility/hashmap/type.h>
#include <utility/memory/memory_destroy/memory_destroy.h>
#include <utility/memory/memory_calloc/memory_calloc.h>
#include <utility/memory/memory_allocation/memory_allocation.h>

#include <stdlib.h> // for NULL
#include <stddef.h> // for size_t

hashmap_t* hashmap_create(size_t capacity, void (*free_value)(void*));

#endif // _SRC_UTILITY_HASHMAP_HASHMAP_CREATE_HASHMAP_CREATE_H_
