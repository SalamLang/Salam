#include <utility/hashmap/hashmap_remove/hashmap_remove.h>

void hashmap_remove(hashmap_t* map, const char* key)
{
    unsigned long idx = string_hash(key) % map->capacity;
    hashmap_entry_t* curr = map->buckets[idx];
    hashmap_entry_t* prev = NULL;

    while (curr) {
        if (string_compare(curr->key, key) == 0) {
            if (prev) prev->next = curr->next;
            else map->buckets[idx] = curr->next;

            if (map->free_value) map->free_value(curr->value);
            memory_destroy(curr->key);
            memory_destroy(curr);
            map->size--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}
