#include "hashmap.h"

#include <string.h>

/**
 * 
 * @function hash_function
 * @brief Hash function
 * @param {const char*} key
 * @returns {unsigned long}
 * 
 */
unsigned long hash_function(const char* str)
{
	unsigned long hash = 5381;
	int c;

	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

/**
 * 
 * @function hashmap_create
 * @brief Create a new hashmap
 * @param {size_t} size
 * @returns {hashmap_t*}
 * 
 */
hashmap_t* hashmap_create(size_t size)
{
	hashmap_t *map = memory_allocate(sizeof(hashmap_t));

	map->size = size;
	map->length = 0;
	map->data = (hashmap_entry_t**) memory_callocate(map->size, sizeof(hashmap_entry_t*));

	map->print = cast(void (*)(void*), hashmap_print);
	map->free = cast(void (*)(void*), hashmap_destroy);

	return map;
}

/**
 * 
 * @function hashmap_put
 * @brief Put a key-value pair in the hashmap
 * @param {hashmap_t*} map
 * @param {const char*} key
 * @param {void*} value
 * @returns {void}
 * 
 */
void hashmap_put(hashmap_t *map, const char *key, void *value)
{
    hashmap_put_custom(map, key, value, free);
}

/**
 * 
 * @function hashmap_put_custom
 * @brief Put a key-value pair in the hashmap
 * @param {hashmap_t*} map
 * @param {const char*} key
 * @param {void*} value
 * @param {void (*free_fn)(void*)} free_fn
 * @returns {void}
 * 
 */
void hashmap_put_custom(hashmap_t *map, const char *key, void *value, void (*free_fn)(void*))
{
	unsigned long hash = hash_function(key);

	size_t index = hash % map->size;
	hashmap_entry_t *entry = map->data[index];

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (free_fn != NULL) free_fn(entry->value);
			entry->value = NULL;

			entry->value = value;

			return;
		}

		entry = entry->next;
	}

	hashmap_entry_t *new_entry = memory_allocate(sizeof(hashmap_entry_t));

	new_entry->key = strdup(key);
	new_entry->value = value;
	new_entry->next = map->data[index];

	map->data[index] = new_entry;

	map->length++;

	if ((float)map->length / map->size >= 0.75) {
		size_t new_length = map->size * 2;
		hashmap_entry_t **new_data = (hashmap_entry_t**) calloc(new_length, sizeof(hashmap_entry_t*));
		for (size_t i = 0; i < map->size; i++) {
			hashmap_entry_t *entry = map->data[i];

			while (entry) {
				hashmap_entry_t *next = entry->next;
				unsigned long new_index = hash_function(entry->key) % new_length;
				entry->next = new_data[new_index];
				new_data[new_index] = entry;
				entry = next;
			}
		}

		free(map->data);
		map->data = new_data;
		map->size = new_length;
	}
}

/**
 * 
 * @function hashmap_get
 * @brief Get a value from the hashmap
 * @param {hashmap_t*} map
 * @param {const char*} key
 * @returns {void*}
 * 
 */
void* hashmap_get(hashmap_t *map, const char *key)
{
	unsigned long hash = hash_function(key);
	size_t index = hash % map->size;
	hashmap_entry_t *entry = map->data[index];

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) return entry->value;

		entry = entry->next;
	}

	return NULL;
}

/**
 * 
 * @function hashmap_remove
 * @brief Remove a key-value pair from the hashmap
 * @param {hashmap_t*} map
 * @param {const char*} key
 * @returns {void*}
 */
void* hashmap_remove(hashmap_t *map, const char *key)
{
	unsigned long hash = hash_function(key);

	size_t index = hash % map->size;
	hashmap_entry_t *entry = map->data[index];
	hashmap_entry_t *prev = NULL;

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (prev == NULL) map->data[index] = entry->next;
			else prev->next = entry->next;

			void *value = entry->value;

            // TODO: Do we need to free the value or not?

			free(entry->key);

			free(entry);

			map->length--;

			return value;
		}
		prev = entry;
		entry = entry->next;
	}

	return NULL;
}

/**
 * 
 * @function hashmap_destroy
 * @brief Free the hashmap memory
 * @param {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy(hashmap_t *map)
{
    hashmap_destroy_custom(map, free);
}

/**
 * 
 * @function hashmap_destroy_custom
 * @brief Free the hashmap memory
 * @param {hashmap_t*} map
 * @param {void (*free_fn)(void*)} free_fn
 * @returns {void}
 * 
 */
void hashmap_destroy_custom(hashmap_t *map, void (*free_fn)(void*))
{
    if (map == NULL) return;

    if (map->data != NULL) {
        for (size_t i = 0; i < map->size; i++) {
            hashmap_entry_t *entry = map->data[i];

            while (entry) {
                hashmap_entry_t *next = entry->next;

                free(entry->key);
                entry->key = NULL;

                if (free_fn != NULL) free_fn(entry->value);
                entry->value = NULL;

                free(entry);

                entry = next;
            }
        }

        free(map->data);
        map->data = NULL;
    }

    free(map);
    map = NULL;
}

/**
 * 
 * @function hashmap_print
 * @brief Print the hashmap
 * @param {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_print(hashmap_t *map)
{
	printf("Hashmap Size: %zu\n", map->length);
	// printf("Hashmap Capacity: %zu\n", map->size);
	printf("Hashmap Contents:\n");

	for (size_t i = 0; i < map->size; i++) {
		hashmap_entry_t *entry = map->data[i];

		while (entry) {
			printf("[%zu] Key: %s, Value: %p\n", i, entry->key, entry->value);
			entry = entry->next;
		}
	}
}

/**
 * 
 * @function hashmap_print_custom
 * @brief Print the hashmap with a custom print function
 * @param {hashmap_t*} map
 * @param {void (*print_fn)(void*)} print_fn
 * @returns {void}
 * 
 */
void hashmap_print_custom(hashmap_t* map, void (*print_fn)(void*))
{
    printf("Hashmap array: %zu\n", map->length);
	if (map->length == 0) {
		printf("Hashmap is empty\n");
		return;
	}

	for (size_t i = 0; i < map->size; i++) {
		hashmap_entry_t *entry = map->data[i];

		while (entry) {
			printf("[%zu] Key: %s, Value: ", i, entry->key);
			print_fn(entry->value);
			printf("\n");

			entry = entry->next;
		}
	}
}
