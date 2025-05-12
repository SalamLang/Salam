#include <utility/hashmap/hashmap_free/hashmap_free.h>

void hashmap_free(hashmap_t* map)
{
    for (size_t i = 0; i < map->capacity; ++i) {
        hashmap_entry_t* curr = map->buckets[i];
        while (curr) {
            hashmap_entry_t* next = curr->next;
            if (map->free_value) map->free_value(curr->value);
            memory_free(curr->key);
            memory_free(curr);
            curr = next;
        }
    }
    memory_free(map->buckets);
    memory_free(map);
}
