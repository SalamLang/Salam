#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct array_t {
    void** data;
    size_t length;
    size_t capacity;
    size_t element_capacity;

    void (*print)(void* node);
    void (*destroy)(void* node);
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
typedef array_t array_layout_attribute_value_t;

#include "memory.h"
#include "ast.h"
#include "lexer.h"
#include "string.h"

/**
 * 
 * @function array_create
 * @brief Create a new array
 * @params {size_t} element_capacity - Size of each element
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_t*} - Pointer to the created array
 * 
 */
array_t* array_create(size_t element_capacity, size_t capacity);

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
void array_init(array_t* array, size_t capacity, size_t element_capacity);

/**
 * 
 * @function array_push
 * @brief Push an element to the array
 * @params {array_t*} array - Array
 * @params {void*} element - Element to add
 * @returns {bool} - Success status
 * 
 */
bool array_push(array_t* array, void* element);

/**
 * 
 * @function array_pop
 * @brief Pop an element from the array
 * @params {array_t*} array - Array
 * @params {void*} element - Buffer to store the popped element
 * @returns {bool} - Success status
 * 
 */
bool array_pop(array_t* array, void* element);

/**
 * 
 * @function array_resize
 * @brief Resize the array to a new capacity
 * @params {array_t*} array - Array
 * @params {size_t} new_capacity - New capacity
 * @returns {void}
 * 
 */
void array_resize(array_t* array, size_t new_capacity);

/**
 * 
 * @function array_get
 * @brief Get an element from the array
 * @params {array_t*} array - Array
 * @params {size_t} index - Index of the element
 * @returns {void*} - Pointer to the element
 * 
 */
void* array_get(array_t* array, size_t index);

/**
 * 
 * @function array_destroy
 * @brief Free the array memory
 * @params {array_t*} array - Array
 * @returns {void}
 * 
 */
void array_destroy(array_t* array);

/**
 * 
 * @function array_destroy_custom
 * @brief Free the array memory
 * @params {array_t*} array - Array
 * @params {void (*free_fn)(void*)} free_fn - Custom free function
 * @returns {void}
 * 
 */
void array_destroy_custom(array_t* array, void (*free_fn)(void*));

/**
 * 
 * @function array_token_print
 * @brief Print the token array
 * @params {array_token_t*} array - Token array
 * @returns {void}
 * 
 */
void array_token_print(array_token_t* array);

/**
 * 
 * @function array_node_destroy
 * @brief Free the node array memory
 * @params {array_node_t*} array - Node array
 * @returns {void}
 * 
 */
void array_node_destroy(array_node_t* array);

/**
 * 
 * @function array_node_print
 * @brief Print the node array
 * @params {array_node_t*} array - Node array
 * @returns {void}
 * 
 */
void array_node_print(array_node_t* array);

/**
 * 
 * @function array_token_destroy
 * @brief Free the token array memory
 * @params {array_token_t*} array - Token array
 * @returns {void}
 * 
 */
void array_token_destroy(array_token_t* array);

/**
 * 
 * @function array_capacity
 * @brief Get the size of the array
 * @params {array_t*} array - Array
 * @returns {size_t} - Size of the array
 * 
 */
size_t array_capacity(array_t* array);

/**
 * 
 * @function array_length
 * @brief Get the length of the array
 * @params {array_t*} array - Array
 * @returns {size_t} - Length of the array
 * 
 */
size_t array_length(array_t* array);

/**
 * 
 * @function array_layout_attribute_print
 * @brief Print the attribute array
 * @params {array_layout_attribute_t*} array - Attribute array
 * @returns {void}
 * 
 */
void array_layout_attribute_print(array_layout_attribute_t* array);

/**
 * 
 * @function array_print
 * @brief Print the array
 * @params {array_t*} array - Array
 * @returns {void}
 * 
 */
void array_print(array_t* array);

/**
 * 
 * @function array_string
 * @brief Convert the array to a string
 * @params {array_t*} array - Array
 * @params {char*} sepertor - Separator
 * @returns {char*} - String
 * 
 */
char* array_string(array_t* array, char* sepertor);

/**
 * 
 * @function array_string_token
 * @brief Convert the token array to a string
 * @params {array_t*} array - Array
 * @params {char*} sepertor - Separator
 * @returns {char*} - String
 * 
 */
char* array_string_token(array_t* array, char* sepertor);

/**
 * 
 * @function array_layout_attribute_print
 * @brief Print the attribute array
 * @params {array_layout_attribute_t*} array - Attribute array
 * @returns {void}
 * 
 */
void array_layout_node_print(array_node_layout_t* node);

/**
 * 
 * @function array_layout_node_destroy
 * @brief Free the node array memory
 * @params {array_node_t*} array - Node array
 * @returns {void}
 * 
 */
void array_layout_node_destroy(array_node_layout_t* array);

/**
 * 
 * @function array_function_parameter_print
 * @brief Print the function parameter array
 * @params {array_function_parameter_t*} array - Function parameter array
 * @returns {void}
 * 
 */
void array_function_parameter_print(array_function_parameter_t* array);

/**
 * 
 * @function array_function_parameter_destroy
 * @brief Free the function parameter array memory
 * @params {array_function_parameter_t*} array - Function parameter array
 * @returns {void}
 * 
 */
void array_function_parameter_destroy(array_function_parameter_t* array);

/**
 * 
 * @function array_function_print
 * @brief Print the function array
 * @params {array_function_t*} array - Function array
 * @returns {void}
 * 
 */
void array_function_print(array_function_t* array);

/**
 * 
 * @function array_function_destroy
 * @brief Free the function array memory
 * @params {array_function_t*} array - Function array
 * @returns {void}
 */
void array_function_destroy(array_function_t* array);

/**
 * 
 * @function array_if_destroy
 * @brief Free the if array memory
 * @params {array_if_t*} array - If array
 * @returns {void}
 * 
 */
void array_if_destroy(array_if_t* array);

/**
 * 
 * @function array_if_print
 * @brief Print the if array
 * @params {array_if_t*} array - If array
 * @returns {void}
 * 
 */
void array_if_print(array_if_t* array);

/**
 * 
 * @function array_value_print
 * @brief Print the value array
 * @params {array_value_t*} array - Value array
 * @returns {void}
 * 
 */
void array_value_print(array_value_t* array);

/**
 * 
 * @function array_value_destroy
 * @brief Free the value array memory
 * @params {array_value_t*} array - Value array
 * @returns {void}
 * 
 */
void array_value_destroy(array_value_t* array);

/**
 * 
 * @function array_block_print
 * @brief Print the block array
 * @params {array_block_t*} array - Block array
 * @returns {void}
 * 
 */
void array_block_print(array_block_t* array);

/**
 * 
 * @function array_block_destroy
 * @brief Free the block array memory
 * @params {array_block_t*} array - Block array
 * @returns {void}
 * 
 */
void array_block_destroy(array_block_t* array);

/**
 * 
 * @function array_value_create
 * @brief Create a new value array
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_value_t*} - Pointer to the created array
 * 
 */
array_value_t* array_value_create(size_t capacity);

/**
 * 
 * @function array_layout_attribute_value_destroy
 * @brief Free the attribute value array memory
 * @params {array_layout_attribute_value_t*} array - Attribute value array
 * @returns {void}
 * 
 */
void array_layout_attribute_value_destroy(array_layout_attribute_value_t* array);

/**
 * 
 * @function array_layout_attribute_value_print
 * @brief Print the attribute value array
 * @params {array_layout_attribute_value_t*} array - Attribute value array
 * @returns {void}
 * 
 */
void array_layout_attribute_value_print(array_layout_attribute_value_t* array);

/**
 * 
 * @function array_layout_attribute_value_create
 * @brief Create a new attribute value array
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_layout_attribute_value_t*} - Pointer to the created array
 * 
 */
array_layout_attribute_value_t* array_layout_attribute_value_create(size_t capacity);

/**
 * 
 * @function array_layout_attribute_value_string
 * @brief Convert the attribute value array to a string
 * @params {array_layout_attribute_value_t*} array - Array
 * @params {char*} seperator - Separator
 * @returns {char*} - String
 * 
 */
char* array_layout_attribute_value_string(array_layout_attribute_value_t* array, char* seperator);

/**
 * 
 * @function array_layout_attribute_value_first_string
 * @brief Get the first string from the attribute value array
 * @params {array_layout_attribute_value_t*} array - Array
 * @returns {char*} - String
 * 
 */
char* array_layout_attribute_value_first_string(array_layout_attribute_value_t* array);

#endif
