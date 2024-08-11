#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "memory.h"

typedef struct {
    void** data;
    size_t size;
    size_t capacity;
    size_t element_size;
} array_t;

typedef array_t array_token_t;

#include "lexer.h"

/**
 * 
 * @function array_create
 * @brief Create a new array
 * @param {size_t} element_size - Size of each element
 * @param {size_t} capacity - Initial capacity of the array
 * @returns {array_t*} - Pointer to the created array
 * 
 */
array_t* array_create(size_t element_size, size_t capacity);

/**
 * 
 * @function array_init
 * @brief Create a new array
 * @param {array_t*} array - Array to initialize
 * @param {size_t} capacity - Initial capacity of the array
 * @param {size_t} element_size - Size of each element
 * @returns {void}
 * 
 */
void array_init(array_t* array, size_t capacity, size_t element_size);

/**
 * 
 * @function array_push
 * @brief Push an element to the array
 * @param {array_t*} array - Array
 * @param {void*} element - Element to add
 * @returns {bool} - Success status
 * 
 */
bool array_push(array_t* array, void* element);

/**
 * 
 * @function array_pop
 * @brief Pop an element from the array
 * @param {array_t*} array - Array
 * @param {void*} element - Buffer to store the popped element
 * @returns {bool} - Success status
 * 
 */
bool array_pop(array_t* array, void* element);

/**
 * 
 * @function array_resize
 * @brief Resize the array to a new capacity
 * @param {array_t*} array - Array
 * @param {size_t} new_capacity - New capacity
 * @returns {void}
 * 
 */
void array_resize(array_t* array, size_t new_capacity);

/**
 * 
 * @function array_get
 * @brief Get an element from the array
 * @param {array_t*} array - Array
 * @param {size_t} index - Index of the element
 * @returns {void*} - Pointer to the element
 * 
 */
void* array_get(array_t* array, size_t index);

/**
 * 
 * @function array_free
 * @brief Free the array memory
 * @param {array_t*} array - Array
 * @returns {void}
 * 
 */
void array_free(array_t* array);

/**
 * 
 * @function array_token_free
 * @brief Free the token array memory
 * @param {array_token_t*} array - Token array
 * @returns {void}
 * 
 */
void array_token_free(array_token_t* array);

#endif