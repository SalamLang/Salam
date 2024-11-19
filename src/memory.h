/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 * 
 * Welcome to the Salam Programming Language! Salam is a modern, efficient, 
 * and developer-friendly language created to inspire and empower programmers 
 * of all ages. With its unique features and Persian roots, Salam aims to make 
 * programming more accessible and enjoyable for everyone.
 * 
 * This file is part of the core implementation of Salam, including its runtime 
 * and compiler components. By contributing to or using this codebase, you are 
 * part of a growing community dedicated to innovation and inclusivity in 
 * programming.
 * 
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 * 
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

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
 * @params {size_t} size
 * @returns {void*}
 *
 */
void *memory_allocate(size_t size);

/**
 *
 * @function memory_callocate
 * @brief Allocate memory and panic if allocation fails
 * @params {size_t} count
 * @params {size_t} size
 * @returns {void*}
 *
 */
void *memory_callocate(size_t count, size_t size);

/**
 *
 * @function memory_destroy
 * @brief Free memory and panic if pointer is NULL
 * @params {void*} ptr
 * @returns {void}
 *
 */
void memory_destroy(void *ptr);

/**
 *
 * @function memory_rellacate
 * @brief Reallocate memory and panic if allocation fails
 * @params {void*} ptr
 * @params {size_t} size
 * @returns {void*}
 *
 */
void *memory_reallocate(void *ptr, size_t size);

/**
 *
 * @function memory_zero
 * @brief Zero out memory
 * @params {void*} ptr
 * @params {size_t} size
 * @returns {void}
 *
 */
void memory_zero(void *ptr, size_t size);

/**
 *
 * @function memory_copy
 * @brief Copy memory
 * @params {void*} dest - Destination
 * @params {const void*} src - Source
 * @params {size_t} size - Size in bytes
 * @returns {void}
 *
 */
void memory_copy(void *dest, const void *src, size_t size);

/**
 *
 * @function memory_compare
 * @brief Compare memory
 * @params {void*} ptr1
 * @params {void*} ptr2
 * @params {size_t} size
 * @returns {int}
 *
 */
int memory_compare(void *ptr1, void *ptr2, size_t size);

/**
 *
 * @function memory_set
 * @brief Set memory
 * @params {void*} ptr
 * @params {int} value
 * @params {size_t} size
 * @returns {void}
 *
 */
void memory_set(void *ptr, int value, size_t size);

/**
 *
 * @function memory_swap
 * @brief Swap memory
 * @params {void*} ptr1
 * @params {void*} ptr2
 * @params {size_t} size
 * @returns {void}
 *
 */
void memory_swap(void *ptr1, void *ptr2, size_t size);

/**
 *
 * @function memory_reverse
 * @brief Reverse memory
 * @params {void*} ptr
 * @params {size_t} size
 * @returns {void}
 *
 */
void memory_reverse(void *ptr, size_t size);

/**
 *
 * @function memory_duplicate
 * @brief Duplicate memory
 * @params {void*} ptr
 * @params {size_t} size
 * @returns {void*}
 *
 */
void *memory_duplicate(void *ptr, size_t size);

#endif
