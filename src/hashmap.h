#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct hashmap_t;

#include "memory.h"
#include "array.h"

typedef struct hashmap_entry {
	char *key;
	void *value;
	struct hashmap_entry *next;
} hashmap_entry_t;

typedef struct {
	hashmap_entry_t **data;
	size_t size;
	size_t length;
} hashmap_t;

typedef hashmap_t hashmap_array_t;
typedef hashmap_t hashmap_attribute_t;

/**
 * 
 * @function hash_function
 * @brief Hash function
 * @param {const char*} key
 * @returns {unsigned long}
 * 
 */
unsigned long hash_function(const char* str);

/**
 * 
 * @function hash_function
 * @brief Hash function
 * @param {const char*} key
 * @returns {unsigned long}
 * 
 */
hashmap_t* hashmap_create();

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
void hashmap_put(hashmap_t *map, const char *key, void *value);

/**
 * 
 * @function hashmap_put_array
 * @brief Put a key-value pair in the hashmap
 * @param {hashmap_t*} map
 * @param {const char*} key
 * @param {void*} value
 * @param {void (*free_fn)(void*)} free_fn
 * @returns {void}
 * 
 */
void hashmap_put_custom(hashmap_t *map, const char *key, void *value, void (*free_fn)(void*));

/**
 * 
 * @function hashmap_get
 * @brief Get a value from the hashmap
 * @param {hashmap_t*} map
 * @param {const char*} key
 * @returns {void*}
 * 
 */
void* hashmap_get(hashmap_t *map, const char *key);

/**
 * 
 * @function hashmap_remove
 * @brief Remove a key-value pair from the hashmap
 * @param {hashmap_t*} map
 * @param {const char*} key
 * @returns {void*}
 */
void* hashmap_remove(hashmap_t *map, const char *key);

/**
 * 
 * @function hashmap_destroy
 * @brief Free the hashmap memory
 * @param {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy(hashmap_t *map);

/**
 * 
 * @function hashmap_destroy
 * @brief Free the hashmap memory
 * @param {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy_custom(hashmap_t *map, void (*free_fn)(void*));

/**
 * 
 * @function hashmap_print
 * @brief Print the hashmap
 * @param {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_print(hashmap_t *map);

#endif
