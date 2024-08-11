#include "base.h"
#include "array.h"

/**
 * 
 * @function array_create
 * @brief Create a new array
 * @param {size_t} element_size - Size of each element
 * @param {size_t} capacity - Initial capacity of the array
 * @returns {array_t*} - Pointer to the created array
 * 
 */
array_t* array_create(size_t element_size, size_t capacity)
{
    array_t* array = memory_allocate(sizeof(array_t));
    array->size = 0;
    array->capacity = capacity < 0 ? 1 : capacity;
    array->element_size = element_size;
    array->data = memory_allocate(array->element_size * array->capacity);
    return array;
}

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
void array_init(array_t* array, size_t capacity, size_t element_size)
{
    array->size = 0;
    array->capacity = capacity;
    array->element_size = element_size;
    array->data = memory_allocate(element_size * capacity);
}

/**
 * 
 * @function array_push
 * @brief Push an element to the array
 * @param {array_t*} array - Array
 * @param {void*} element - Element to add
 * @returns {bool} - Success status
 * 
 */
bool array_push(array_t* array, void* element)
{
    if (array->size >= array->capacity) {
        array->capacity *= 2;
        array->data = memory_reallocate(array->data, array->element_size * array->capacity);
    }

    array->data[array->size++] = element;

    return true;
}

/**
 * 
 * @function array_pop
 * @brief Pop an element from the array
 * @param {array_t*} array - Array
 * @param {void*} element - Buffer to store the popped element
 * @returns {bool} - Success status
 * 
 */
bool array_pop(array_t* array, void* element)
{
    if (array->size == 0) {
        return false;
    }

    array->size--;
    memory_copy(element, (char*)array->data + (array->size * array->element_size), array->element_size);
    return true;
}

/**
 * 
 * @function array_resize
 * @brief Resize the array to a new capacity
 * @param {array_t*} array - Array
 * @param {size_t} new_capacity - New capacity
 * @returns {void}
 * 
 */
void array_resize(array_t* array, size_t new_capacity)
{
    array->capacity = new_capacity;
    array->data = memory_reallocate(array->data, array->element_size * new_capacity);
}

/**
 * 
 * @function array_get
 * @brief Get an element from the array
 * @param {array_t*} array - Array
 * @param {size_t} index - Index of the element
 * @returns {void*} - Pointer to the element
 * 
 */
void* array_get(array_t* array, size_t index)
{
    if (index >= array->size) {
        return NULL;
    }

    return array->data[index];
}

/**
 * 
 * @function array_free
 * @brief Free the array memory
 * @param {array_t*} array - Array
 * @returns {void}
 * 
 */
void array_free(array_t* array)
{
    memory_free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
    array->element_size = 0;
    free(array);
}

/**
 * 
 * @function array_token_free
 * @brief Free the token array memory
 * @param {array_token_t*} array - Token array
 * @returns {void}
 * 
 */
void array_token_free(array_token_t* array)
{
    token_t** token_array = (token_t**) array->data;

    for (size_t i = 0; i < array->size; i++) {
        token_t* token_item = token_array[i];
        token_destroy(token_item);
    }

    array_free(array);
}

/**
 * 
 * @function array_token_print
 * @brief Print the token array
 * @param {array_token_t*} array - Token array
 * @returns {void}
 * 
 */
void array_token_print(array_token_t* array)
{
    printf("Token array: %zu\n", array->size);

    for (size_t i = 0; i < array->size; i++) {
        printf("\t");
        token_t* token = array_get(array, i);
        token->print(token);
    }
}
