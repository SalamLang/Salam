#include "array.h"

#include "string_buffer.h"

/**
 *
 * @function array_create
 * @brief Create a new array
 * @params {size_t} element_capacity - Size of each element
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_t*} - Pointer to the created array
 *
 */
array_t *array_create(size_t element_capacity, size_t capacity) {
    DEBUG_ME;
    array_t *array = memory_allocate(sizeof(array_t));
    array->length = 0;
    array->capacity = capacity;
    array->element_capacity = element_capacity;
    array->data = memory_allocate(array->element_capacity * array->capacity);

    array->print = cast(void (*)(void *), array_print);
    array->destroy = cast(void (*)(void *), array_destroy);

    return array;
}

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
void array_init(array_t *array, size_t capacity, size_t element_capacity) {
    DEBUG_ME;
    array->length = 0;
    array->capacity = capacity;
    array->element_capacity = element_capacity;
    array->data = memory_allocate(element_capacity * capacity);
}

/**
 *
 * @function array_push
 * @brief Push an element to the array
 * @params {array_t*} array - Array
 * @params {void*} element - Element to add
 * @returns {bool} - Success status
 *
 */
bool array_push(array_t *array, void *element) {
    DEBUG_ME;
    if (array->length >= array->capacity) {
        array->capacity *= 2;
        array->data = memory_reallocate(
            array->data, array->element_capacity * array->capacity);
    }

    array->data[array->length++] = element;

    return true;
}

/**
 *
 * @function array_pop
 * @brief Pop an element from the array
 * @params {array_t*} array - Array
 * @params {void*} element - Buffer to store the popped element
 * @returns {bool} - Success status
 *
 */
bool array_pop(array_t *array, void *element) {
    DEBUG_ME;
    if (array->length == 0) {
        return false;
    }

    array->length--;
    memory_copy(
        element,
        (char *)array->data + (array->capacity * array->element_capacity),
        array->element_capacity);

    return true;
}

/**
 *
 * @function array_resize
 * @brief Resize the array to a new capacity
 * @params {array_t*} array - Array
 * @params {size_t} new_capacity - New capacity
 * @returns {void}
 *
 */
void array_resize(array_t *array, size_t new_capacity) {
    DEBUG_ME;
    array->capacity = new_capacity;
    array->data =
        memory_reallocate(array->data, array->element_capacity * new_capacity);
}

/**
 *
 * @function array_get
 * @brief Get an element from the array
 * @params {array_t*} array - Array
 * @params {size_t} index - Index of the element
 * @returns {void*} - Pointer to the element
 *
 */
void *array_get(array_t *array, size_t index) {
    DEBUG_ME;
    if (index >= array->length) {
        return NULL;
    }

    return array->data[index];
}

/**
 *
 * @function array_destroy
 * @brief Free the array memory
 * @params {array_t*} array - Array
 * @returns {void}
 *
 */
void array_destroy(array_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        memory_destroy(array->data);

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
    }
}

/**
 *
 * @function array_string
 * @brief Convert the array to a string
 * @params {array_t*} array - Array
 * @params {char*} separator - Separator
 * @returns {char*} - String
 *
 */
char *array_stringify(array_t *array, char *separator) {
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return string_strdup("");
    }

    string_t *buf = string_create(16);
    for (size_t i = 0; i < array->length; i++) {
        char *item = array_get(array, i);

        if (item != NULL) {
            string_append_str(buf, item);

            if (i < array->length - 1) {
                string_append_str(buf, separator);
            }
        }
    }

    char *buffer = string_strdup(buf->data);
    string_destroy(buf);

    return buffer;
}

/**
 *
 * @function array_print
 * @brief Print the array
 * @params {array_t*} array - Array
 * @returns {void}
 *
 */
void array_print(array_t *array) {
    DEBUG_ME;
    printf("Array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t%p\n", array->data[i]);
    }
}

/**
 *
 * @function array_capacity
 * @brief Get the size of the array
 * @params {array_t*} array - Array
 * @returns {size_t} - Size of the array
 *
 */
size_t array_capacity(array_t *array) {
    DEBUG_ME;
    return array->capacity;
}

/**
 *
 * @function array_length
 * @brief Get the length of the array
 * @params {array_t*} array - Array
 * @returns {size_t} - Length of the array
 *
 */
size_t array_length(array_t *array) {
    DEBUG_ME;
    return array->length;
}
