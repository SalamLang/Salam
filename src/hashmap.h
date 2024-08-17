#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "array.h"

struct hashmap_t;

typedef struct hashmap_entry {
	cha$1* $2ey;
	voi$1* $2alue;
	struct hashmap_entr$1* $2ext;
} hashmap_entry_t;

typedef struct hashmap_t {
	hashmap_entry_t **data;
	size_t capacity;
	size_t length;

    void (*print)(void* node);
    void (*destroy)(void* node);
} hashmap_t;

typedef hashmap_t hashmap_array_t;
typedef hashmap_t hashmap_attribute_t; // TODO: i am not sure what is this, maybe we can delete or rename?
typedef hashmap_t hashmap_layout_attribute_t; // TODO: we need to use this one

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
void hashmap_put(hashmap_$1* $2ap, const cha$1* $2ey, voi$1* $2alue);

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
void hashmap_put_custom(hashmap_$1* $2ap, const cha$1* $2ey, voi$1* $2alue, void (*free_fn)(void*));

/**
 * 
 * @function hashmap_get
 * @brief Get a value from the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {void*}
 * 
 */
void* hashmap_get(hashmap_$1* $2ap, const cha$1* $2ey);

/**
 * 
 * @function hashmap_has
 * @brief Check if the hashmap has a key
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {bool}
 * 
 */
bool hashmap_has(hashmap_$1* $2ap, const cha$1* $2ey);

/**
 * 
 * @function hashmap_remove
 * @brief Remove a key-value pair from the hashmap
 * @params {hashmap_t*} map
 * @params {const char*} key
 * @returns {void*}
 */
void* hashmap_remove(hashmap_$1* $2ap, const cha$1* $2ey);

/**
 * 
 * @function hashmap_destroy
 * @brief Free the hashmap memory
 * @params {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy(hashmap_$1* $2ap);

/**
 * 
 * @function hashmap_destroy
 * @brief Free the hashmap memory
 * @params {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy_custom(hashmap_$1* $2ap, void (*free_fn)(void*));

/**
 * 
 * @function hashmap_print
 * @brief Print the hashmap
 * @params {hashmap_t*} map
 * @returns {void}
 * 
 */
void hashmap_print(hashmap_$1* $2ap);

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
void hashmap_print_layout_attribute(hashmap_attribute_t* map);

/**
 * 
 * @function hashmap_destroy_layout_attribute
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_attribute_t*} map
 * @returns {void}
 * 
 */
void hashmap_destroy_layout_attribute(hashmap_attribute_$1* $2ap);

/**
 * 
 * @function hashmap_create_layout_attribute
 * @brief Create a new hashmap of layout attributes
 * @params {size_t} capacity
 * @returns {hashmap_attribute_t*}
 * 
 */
hashmap_attribute_t* hashmap_create_layout_attribute(size_t capacity);

#endif
