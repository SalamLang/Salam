#include <utility/hashmap/hashmap_put/hashmap_put.h>

void hashmap_put(hashmap_t* map, const char* key, void* value) {
    unsigned long idx = string_hash(key) % map->capacity;
    hashmap_entry_t* curr = map->buckets[idx];

    while (curr) {
        if (string_compare(curr->key, key) == 0) {
            if (map->free_value) map->free_value(curr->value);
            curr->value = value;
            return;
        }
        curr = curr->next;
    }

    hashmap_entry_t* new_entry = memory_allocation(sizeof(hashmap_entry_t));
    new_entry->key = string_duplicate(key);
    new_entry->value = value;
    new_entry->next = map->buckets[idx];
    map->buckets[idx] = new_entry;
    map->size++;
}
