#include "memory.h"

/**
 * 
 * @function memory_allocate
 * @brief Allocate memory and panic if allocation fails
 * @param {size_t} size
 * @returns {void*}
 * 
 */
void* memory_allocate(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL) {
        panic("Failed to allocate memory");
    }

    return ptr;
}

/**
 * 
 * @function memory_callocate
 * @brief Allocate memory and panic if allocation fails
 * @param {size_t} count
 * @param {size_t} size
 * @returns {void*}
 * 
 */
void* memory_callocate(size_t count, size_t size)
{
    void* ptr = calloc(count, size);
    if (ptr == NULL) {
        panic("Failed to allocate memory");
    }

    return ptr;
}

/**
 * 
 * @function memory_destroy
 * @brief Free memory and panic if pointer is NULL
 * @param {void*} ptr
 * @returns {void}
 * 
 */
void memory_destroy(void* ptr)
{
    if (ptr == NULL) {
        panic("Failed to free memory");
    }

    free(ptr);
}

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
void* memory_reallocate(void* ptr, size_t size)
{
    void* new_ptr = realloc(ptr, size);
    if (new_ptr == NULL) {
        panic("Failed to reallocate memory");
    }

    return new_ptr;
}

/**
 * 
 * @function memory_zero
 * @brief Zero out memory
 * @param {void*} ptr
 * @param {size_t} size
 * @returns {void}
 * 
 */
void memory_zero(void* ptr, size_t size)
{
    memset(ptr, 0, size);
}

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
void memory_copy(void* dest, const void* src, size_t size)
{
    memcpy(dest, src, size);
}

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
int memory_compare(void* ptr1, void* ptr2, size_t size)
{
    return memcmp(ptr1, ptr2, size);
}

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
void memory_set(void* ptr, int value, size_t size)
{
    memset(ptr, value, size);
}

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
void memory_swap(void* ptr1, void* ptr2, size_t size)
{
    void* temp = memory_allocate(size);
    memory_copy(temp, ptr1, size);
    memory_copy(ptr1, ptr2, size);
    memory_copy(ptr2, temp, size);
    memory_destroy(temp);
}

/**
 * 
 * @function memory_reverse
 * @brief Reverse memory
 * @param {void*} ptr
 * @param {size_t} size
 * @returns {void}
 * 
 */
void memory_reverse(void* ptr, size_t size)
{
    size_t i = 0;
    size_t j = size - 1;

    while (i < j) {
        memory_swap(ptr + i, ptr + j, 1);
        i++;
        j--;
    }
}

/**
 * 
 * @function memory_duplicate
 * @brief Duplicate memory
 * @param {void*} ptr
 * @param {size_t} size
 * @returns {void*}
 * 
 */
void* memory_duplicate(void* ptr, size_t size)
{
    void* new_ptr = memory_allocate(size);
    memory_copy(new_ptr, ptr, size);

    return new_ptr;
}
