#ifndef _UTILITY_HASHMAP_TYPE_H_
#define _UTILITY_HASHMAP_TYPE_H_

#include <stddef.h> // for size_t

typedef struct hashmap_entry_t {
    char* key;
    void* value;
    struct hashmap_entry_t* next;
} hashmap_entry_t;

typedef struct {
    hashmap_entry_t** buckets;
    size_t capacity;
    size_t size;
    void (*free_value)(void*);
} hashmap_t;

#endif // _UTILITY_HASHMAP_TYPE_H_
