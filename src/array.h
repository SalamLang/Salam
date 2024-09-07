#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct array_t
{
	void **data;
	size_t length;
	size_t capacity;
	size_t element_capacity;

	void (*print)(void *node);
	char *(*stringify)(void *);
	void (*destroy)(void *node);
} array_t;

typedef array_t array_token_t;
typedef array_t array_node_t;
typedef array_t array_node_layout_t;
typedef array_t array_layout_attribute_t;
typedef array_t array_function_parameter_t;
typedef array_t array_function_t;
typedef array_t array_block_t;
typedef array_t array_if_t;
typedef array_t array_value_t;

#include "memory.h"
#include "ast.h"
#include "lexer.h"
#include "string.h"
#include "base.h"

/**
 *
 * @function array_create
 * @brief Create a new array
 * @params {size_t} element_capacity - Size of each element
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_t*} - Pointer to the created array
 *
 */
array_t *array_create(size_t element_capacity, size_t capacity);

/**
 *
 * @function array_init
 * @brief Create a new array
 * @params {array_t*} array - Array to initialize
 * @params {size_t} capacity - Initial capacity of the array
 * @params {size_t} element_capacity - Size of each element
 * @returns {void}
 *
 */
void array_init(array_t *array, size_t capacity, size_t element_capacity);

/**
 *
 * @function array_push
 * @brief Push an element to the array
 * @params {array_t*} array - Array
 * @params {void*} element - Element to add
 * @returns {bool} - Success status
 *
 */
bool array_push(array_t *array, void *element);

/**
 *
 * @function array_pop
 * @brief Pop an element from the array
 * @params {array_t*} array - Array
 * @params {void*} element - Buffer to store the popped element
 * @returns {bool} - Success status
 *
 */
bool array_pop(array_t *array, void *element);

/**
 *
 * @function array_resize
 * @brief Resize the array to a new capacity
 * @params {array_t*} array - Array
 * @params {size_t} new_capacity - New capacity
 * @returns {void}
 *
 */
void array_resize(array_t *array, size_t new_capacity);

/**
 *
 * @function array_get
 * @brief Get an element from the array
 * @params {array_t*} array - Array
 * @params {size_t} index - Index of the element
 * @returns {void*} - Pointer to the element
 *
 */
void *array_get(array_t *array, size_t index);

/**
 *
 * @function array_destroy
 * @brief Free the array memory
 * @params {array_t*} array - Array
 * @returns {void}
 *
 */
void array_destroy(array_t *array);

/**
 *
 * @function array_capacity
 * @brief Get the size of the array
 * @params {array_t*} array - Array
 * @returns {size_t} - Size of the array
 *
 */
size_t array_capacity(array_t *array);

/**
 *
 * @function array_length
 * @brief Get the length of the array
 * @params {array_t*} array - Array
 * @returns {size_t} - Length of the array
 *
 */
size_t array_length(array_t *array);

/**
 *
 * @function array_print
 * @brief Print the array
 * @params {array_t*} array - Array
 * @returns {void}
 *
 */
void array_print(array_t *array);

/**
 *
 * @function array_string
 * @brief Convert the array to a string
 * @params {array_t*} array - Array
 * @params {char*} separator - Separator
 * @returns {char*} - String
 *
 */
char *array_stringify(array_t *array, char *separator);

#endif
