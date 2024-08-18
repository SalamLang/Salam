#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "array.h"

struct hashmap_t;

typedef struct hashmap_entry {
	char* key;
	void* value;
	struct hashmap_entry_t* next;
} hashmap_entry_t;

typedef struct hashmap_t {
	hashmap_entry_t **data;
	size_t capacity;
	size_t length;

    void (*print)(void* node);
    void (*destroy)(void* node);
} hashmap_t;

typedef hashmap_t hashmap_array_t;
typedef hashmap_t hashmap_layout_attribute_t;
typedef hashmap_t hashmap_layout_attribute_state_style_t;

/**
 *
 * @function hash_function
 * @brief Hash function
 * @params {const char*} key
 * @returns {unsigned long}
 *
 */
unsigned long hash_function(const char* str);

/**
 *
 * @function hashmap_create
 * @brief Create a new hashmap
 * @params {size_t} size
 * @returns {hashmap_t*}
 *
 */
hashmap_t* hashmap_create(size_t size);

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
void hashmap_put(hashmap_t* map, const char* key, void* value);

/**
 *
 * @function hashmap_put_array
 * @brief Put a key-value pair in the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @params {void*} value
 * @params {void (*free_fn)(void*)} free_fn
 * @returns {void}
 *
 */
void hashmap_put_custom(hashmap_t* map, const char* key, void* value, void (*free_fn)(void*));

/**
 *
 * @function hashmap_get
 * @brief Get a value from the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {void*}
 *
 */
void* hashmap_get(hashmap_t* map, const char* key);

/**
 *
 * @function hashmap_has
 * @brief Check if the hashmap has a key
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {bool}
 *
 */
bool hashmap_has(hashmap_t* map, const char* key);

/**
 *
 * @function hashmap_remove
 * @brief Remove a key-value pair from the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {void*}
 */
void* hashmap_remove(hashmap_t* map, const char* key);

/**
 *
 * @function hashmap_destroy
 * @brief Free the hashmap memory
 * @params {hashmap_t*} map
 * @returns {void}
 *
 */
void hashmap_destroy(hashmap_t* map);

/**
 *
 * @function hashmap_destroy
 * @brief Free the hashmap memory
 * @params {hashmap_t*} map
 * @returns {void}
 *
 */
void hashmap_destroy_custom(hashmap_t* map, void (*free_fn)(void*));

/**
 *
 * @function hashmap_print
 * @brief Print the hashmap
 * @params {hashmap_t*} map
 * @returns {void}
 *
 */
void hashmap_print(hashmap_t* map);

/**
 *
 * @function hashmap_print_custom
 * @brief Print the hashmap with a custom print function
 * @params {hashmap_t*} map
 * @params {void (*print_fn)(void*)} print_fn
 * @returns {void}
 *
 */
void hashmap_print_custom(hashmap_t* map, void (*print_fn)(void*));

/**
 *
 * @function hashmap_print_layout_attribute
 * @brief Print the hashmap of layout attributes
 * @params {ast_layout_attribute_t*} map - The hashmap to print
 * @returns {void}
 *
 */
void hashmap_print_layout_attribute(hashmap_layout_attribute_t* map);

/**
 *
 * @function hashmap_destroy_layout_attribute
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_destroy_layout_attribute(hashmap_layout_attribute_t* map);

/**
 *
 * @function hashmap_create_layout_attribute
 * @brief Create a new hashmap of layout attributes
 * @params {size_t} capacity
 * @returns {hashmap_layout_attribute_t*}
 *
 */
hashmap_layout_attribute_t* hashmap_create_layout_attribute(size_t capacity);

/**
 * 
 * @function hashmap_destroy_layout_attribute_style_state
 * @brief Destroy the hashmap of layout style state attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy_layout_attribute_style_state(hashmap_layout_attribute_t* map);

/**
 * 
 * @function hashmap_print_layout_attribute_style_state
 * @brief Print the hashmap of layout style state attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_print_layout_attribute_style_state(hashmap_layout_attribute_t* map);

/**
 * 
 * @function hashmap_create_layout_attribute_style_state
 * @brief Create a new hashmap of layout style state attributes
 * @params {size_t} capacity
 * @returns {hashmap_layout_attribute_state_style_t*}
 * 
 */
hashmap_layout_attribute_state_style_t* hashmap_create_layout_attribute_style_state(size_t capacity);

/**
 * 
 * @function hashmap_layout_attribute_print
 * @brief Print the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_layout_attribute_print(hashmap_layout_attribute_t* map);

/**
 * 
 * @function hashmap_layout_attribute_destroy
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_layout_attribute_destroy(hashmap_layout_attribute_t* map);

#endif
