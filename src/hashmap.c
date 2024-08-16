#include "hashmap.h"

#include <string.h>

/**
 * 
 * @function hash_function
 * @brief Hash function
 * @params {const char*} key
 * @returns {unsigned long}
 * 
 */
unsigned long hash_function(const char* str)
{
    DEBUG_ME;
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
 * @params {size_t} size
 * @returns {hashmap_t*}
 * 
 */
hashmap_t* hashmap_create(size_t capacity)
{
    DEBUG_ME;
	hashmap_t *map = memory_allocate(sizeof(hashmap_t));

	map->capacity = capacity;
	map->length = 0;
	map->data = (hashmap_entry_t**) memory_callocate(map->capacity, sizeof(hashmap_entry_t*));

	map->print = cast(void (*)(void*), hashmap_print);
	map->destroy = cast(void (*)(void*), hashmap_destroy);

	return map;
}

/**
 * 
 * @function hashmap_put
 * @brief Put a key-value pair in the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @params {void*} value
 * @returns {void}
 * 
 */
void hashmap_put(hashmap_t *map, const char *key, void *value)
{
    DEBUG_ME;
	hashmap_put_custom(map, key, value, free);
}

/**
 * 
 * @function hashmap_put_custom
 * @brief Put a key-value pair in the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @params {void*} value
 * @params {void (*free_fn)(void*)} free_fn
 * @returns {void}
 * 
 */
void hashmap_put_custom(hashmap_t *map, const char *key, void *value, void (*free_fn)(void*))
{
    DEBUG_ME;
	unsigned long hash = hash_function(key);

	size_t index = hash % map->capacity;
	hashmap_entry_t *entry = map->data[index];

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (free_fn != NULL) free_fn(entry->value);

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

	if ((float)map->length / map->capacity >= 0.75) {
		size_t new_length = map->capacity * 2;
		hashmap_entry_t **new_data = (hashmap_entry_t**) memory_callocate(new_length, sizeof(hashmap_entry_t*));

		for (size_t i = 0; i < map->capacity; i++) {
			hashmap_entry_t *entry = map->data[i];

			while (entry) {
				hashmap_entry_t *next = entry->next;
				unsigned long new_index = hash_function(entry->key) % new_length;

				entry->next = new_data[new_index];
				new_data[new_index] = entry;
				entry = next;
			}
		}

		if (map->data != NULL) {
			memory_destroy(map->data);
		}

		map->data = new_data;
		map->capacity = new_length;
	}
}

/**
 * 
 * @function hashmap_get
 * @brief Get a value from the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {void*}
 * 
 */
void* hashmap_get(hashmap_t *map, const char *key)
{
    DEBUG_ME;
	unsigned long hash = hash_function(key);
	size_t index = hash % map->capacity;
	hashmap_entry_t *entry = map->data[index];
	
	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) return entry->value;

		entry = entry->next;
	}

	return NULL;
}

/**
 * 
 * @function hashmap_has
 * @brief Check if the hashmap has a key
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {bool}
 * 
 */
bool hashmap_has(hashmap_t *map, const char *key)
{
	DEBUG_ME;
	unsigned long hash = hash_function(key);
	size_t index = hash % map->capacity;
	hashmap_entry_t *entry = map->data[index];
	
	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) return true;

		entry = entry->next;
	}

	return false;
}

/**
 * 
 * @function hashmap_remove
 * @brief Remove a key-value pair from the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {void*}
 */
void* hashmap_remove(hashmap_t *map, const char *key)
{
    DEBUG_ME;
	unsigned long hash = hash_function(key);

	size_t index = hash % map->capacity;
	hashmap_entry_t *entry = map->data[index];
	hashmap_entry_t *prev = NULL;

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (prev == NULL) map->data[index] = entry->next;
			else prev->next = entry->next;

			void *value = entry->value;

			// TODO: Do we need to free the value or not?

			if (entry != NULL) {
				if (entry->key != NULL) {
					memory_destroy(entry->key);
				}

				memory_destroy(entry);
			}

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
 * @params {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy(hashmap_t *map)
{
    DEBUG_ME;
	return hashmap_destroy_custom(map, free);
}

/**
 * 
 * @function hashmap_destroy_custom
 * @brief Free the hashmap memory
 * @params {hashmap_t*} map
 * @params {void (*free_fn)(void*)} free_fn
 * @returns {void}
 * 
 */
void hashmap_destroy_custom(hashmap_t *map, void (*free_fn)(void*))
{
    DEBUG_ME;
	printf("hashmap_destroy_custom...\n");
	if (map != NULL) {
		if (map->data != NULL) {
			for (size_t i = 0; i < map->capacity; i++) {
				hashmap_entry_t *entry = map->data[i];

				while (entry) {
					hashmap_entry_t *next = entry->next;

					memory_destroy(entry->key);

					printf("hashmap_destroy_custom 2222...\n");
					free_fn(entry->value);
					printf("hashmap_destroy_custom 3333...\n");

					memory_destroy(entry);

					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 * 
 * @function hashmap_print
 * @brief Print the hashmap
 * @params {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_print(hashmap_t *map)
{
    DEBUG_ME;
	printf("Hashmap Size: %zu\n", map->length);
	// printf("Hashmap Capacity: %zu\n", map->capacity);
	printf("Hashmap Contents:\n");

	for (size_t i = 0; i < map->capacity; i++) {
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
 * @params {hashmap_t*} map
 * @params {void (*print_fn)(void*)} print_fn
 * @returns {void}
 * 
 */
void hashmap_print_custom(hashmap_t* map, void (*print_fn)(void*))
{
    DEBUG_ME;
	printf("Hashmap array: %zu\n", map->length);
	if (map->length == 0) {
		printf("Hashmap is empty\n");
		return;
	}

	for (size_t i = 0; i < map->capacity; i++) {
		hashmap_entry_t *entry = map->data[i];

		while (entry) {
			printf("[%zu] Key: %s, Value: ", i, entry->key);
			print_fn(entry->value);
			printf("\n");

			entry = entry->next;
		}
	}
}

/**
 * 
 * @function hashmap_print_layout_attribute
 * @brief Print the hashmap of layout attributes
 * @params {ast_layout_attribute_t*} map - The hashmap to print
 * @returns {void}
 * 
 */
void hashmap_print_layout_attribute(hashmap_attribute_t* map)
{
    DEBUG_ME;
	printf("Hashmap length: %zu\n", map->length);
	if (map->length == 0) {
		printf("Hashmap is empty\n");
		return;
	}

	for (size_t i = 0; i < map->capacity; i++) {
		hashmap_entry_t *entry = map->data[i];

		while (entry) {
			printf("[%zu] Key: %s, Value: ", i, entry->key);
			ast_layout_attribute_t* layout_attribute = entry->value;

			if (layout_attribute != NULL) layout_attribute->print(layout_attribute);
			else printf("NULL\n");

			entry = entry->next;
		}
	}
}

/**
 * 
 * @function hashmap_destroy_layout_attribute
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy_layout_attribute(hashmap_attribute_t *map)
{
    DEBUG_ME;
	if (map != NULL) {
		if (map->data != NULL) {
			for (size_t i = 0; i < map->capacity; i++) {
				hashmap_entry_t *entry = map->data[i];

				while (entry) {
					hashmap_entry_t *next = entry->next;

					memory_destroy(entry->key);

					ast_layout_attribute_t* layout_attribute = cast(ast_layout_attribute_t*, entry->value);
					if (layout_attribute != NULL) {
						layout_attribute->destroy(layout_attribute);
					}

					memory_destroy(entry);

					entry = next;
				}
			}

			memory_destroy(map->data);
		}

		memory_destroy(map);
	}
}

/**
 * 
 * @function hashmap_create_layout_attribute
 * @brief Create a new hashmap of layout attributes
 * @params {size_t} capacity
 * @returns {hashmap_attribute_t*}
 * 
 */
hashmap_attribute_t* hashmap_create_layout_attribute(size_t capacity)
{
	hashmap_attribute_t* map = cast(struct hashmap_t*, hashmap_create(capacity));

	map->print = cast(void (*)(void*), hashmap_print_layout_attribute);
	map->destroy = cast(void (*)(void*), hashmap_destroy_layout_attribute);

	return map;
}