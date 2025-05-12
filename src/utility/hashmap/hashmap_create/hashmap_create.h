#include <utility/hashmap/type.h>
#include <utility/memory/memory_free/memory_free.h>
#include <utility/memory/memory_calloc/memory_calloc.h>
#include <utility/memory/memory_allocation/memory_allocation.h>

#include <stdlib.h> // for NULL
#include <stddef.h> // for size_t

hashmap_t* hashmap_create(size_t capacity, void (*free_value)(void*));
