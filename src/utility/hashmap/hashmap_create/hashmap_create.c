#include <utility/hashmap/hashmap_create/hashmap_create.h>

hashmap_t* hashmap_create(size_t capacity, void (*free_value)(void*))
{
    hashmap_t* map = memory_allocation(sizeof(hashmap_t));
    map->capacity = capacity;
    map->size = 0;
    map->free_value = free_value;
    map->buckets = memory_calloc(capacity, sizeof(hashmap_entry_t*));
    if (!map->buckets) {
        memory_free(map);
        return NULL;
    }

    return map;
}
