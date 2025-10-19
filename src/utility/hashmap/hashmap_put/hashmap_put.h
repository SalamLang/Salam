#ifndef _SRC_UTILITY_HASHMAP_HASHMAP_PUT_HASHMAP_PUT_H_
#define _SRC_UTILITY_HASHMAP_HASHMAP_PUT_HASHMAP_PUT_H_

// hashmap
#include <utility/hashmap/type.h>
// memory
#include <utility/memory/memory_allocation/memory_allocation.h>
// string
#include <utility/string/string_duplicate/string_duplicate.h>
#include <utility/string/string_compare/string_compare.h>
#include <utility/string/string_hash/string_hash.h>

void hashmap_put(hashmap_t* map, const char* key, void* value);

#endif // _SRC_UTILITY_HASHMAP_HASHMAP_PUT_HASHMAP_PUT_H_
