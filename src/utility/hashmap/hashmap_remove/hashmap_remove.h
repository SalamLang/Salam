#ifndef _UTILITY_HASHMAP_HASHMAP_REMOVE_HASHMAP_REMOVE_H_
#define _UTILITY_HASHMAP_HASHMAP_REMOVE_HASHMAP_REMOVE_H_

// hashmap
#include <utility/hashmap/type.h>
// memory
#include <utility/memory/memory_destroy/memory_destroy.h>
// string
#include <utility/string/string_compare/string_compare.h>
#include <utility/string/string_hash/string_hash.h>

void hashmap_remove(hashmap_t* map, const char* key);

#endif  // _UTILITY_HASHMAP_HASHMAP_REMOVE_HASHMAP_REMOVE_H_
