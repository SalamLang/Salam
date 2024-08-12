#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

/**
 * 
 * @function memory_allocate
 * @brief Allocate memory and panic if allocation fails
 * @param {size_t} size
 * @returns {void*}
 * 
 */
void* memory_allocate(size_t size);

/**
 * 
 * @function memory_callocate
 * @brief Allocate memory and panic if allocation fails
 * @param {size_t} count
 * @param {size_t} size
 * @returns {void*}
 * 
 */
void* memory_callocate(size_t count, size_t size);

/**
 * 
 * @function memory_free
 * @brief Free memory and panic if pointer is NULL
 * @param {void*} ptr
 * @returns {void}
 * 
 */
void memory_free(void* ptr);

// memory related functions
/**
 * 
 * @function memory_rellacate
 * @brief Reallocate memory and panic if allocation fails
 * @param {void*} ptr
 * @param {size_t} size
 * @returns {void*}
 * 
 */
void* memory_reallocate(void* ptr, size_t size);

/**
 * 
 * @function memory_zero
 * @brief Zero out memory
 * @param {void*} ptr
 * @param {size_t} size
 * @returns {void}
 * 
 */
void memory_zero(void* ptr, size_t size);

/**
 * 
 * @function memory_copy
 * @brief Copy memory
 * @param {void*} dest - Destination
 * @param {const void*} src - Source
 * @param {size_t} size - Size in bytes
 * @returns {void}
 * 
 */
void memory_copy(void* dest, const void* src, size_t size);

/**
 * 
 * @function memory_compare
 * @brief Compare memory
 * @param {void*} ptr1
 * @param {void*} ptr2
 * @param {size_t} size
 * @returns {int}
 * 
 */
int memory_compare(void* ptr1, void* ptr2, size_t size);

/**
 * 
 * @function memory_set
 * @brief Set memory
 * @param {void*} ptr
 * @param {int} value
 * @param {size_t} size
 * @returns {void}
 * 
 */
void memory_set(void* ptr, int value, size_t size);

/**
 * 
 * @function memory_swap
 * @brief Swap memory
 * @param {void*} ptr1
 * @param {void*} ptr2
 * @param {size_t} size
 * @returns {void}
 * 
 */
void memory_swap(void* ptr1, void* ptr2, size_t size);

/**
 * 
 * @function memory_reverse
 * @brief Reverse memory
 * @param {void*} ptr
 * @param {size_t} size
 * @returns {void}
 * 
 */
void memory_reverse(void* ptr, size_t size);

#endif
