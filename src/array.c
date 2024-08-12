#include "base.h"
#include "array.h"

/**
 * 
 * @function array_create
 * @brief Create a new array
 * @params {size_t} element_size - Size of each element
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_t*} - Pointer to the created array
 * 
 */
array_t* array_create(size_t element_size, size_t capacity)
{
    array_t* array = memory_allocate(sizeof(array_t));
    array->size = 0;
    array->capacity = capacity;
    array->element_size = element_size;
    array->data = memory_allocate(array->element_size * array->capacity);

    array->print = cast(void (*)(void*), array_print);
    array->destroy = cast(void (*)(void*), array_destroy);
    
    return array;
}

/**
 * 
 * @function array_init
 * @brief Create a new array
 * @params {array_t*} array - Array to initialize
 * @params {size_t} capacity - Initial capacity of the array
 * @params {size_t} element_size - Size of each element
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
 * @params {array_t*} array - Array
 * @params {void*} element - Element to add
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
 * @params {array_t*} array - Array
 * @params {void*} element - Buffer to store the popped element
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
 * @params {array_t*} array - Array
 * @params {size_t} new_capacity - New capacity
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
 * @params {array_t*} array - Array
 * @params {size_t} index - Index of the element
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
 * @function array_destroy
 * @brief Free the array memory
 * @params {array_t*} array - Array
 * @returns {void}
 * 
 */
void array_destroy(array_t* array)
{
    if (array != NULL) {
        memory_destroy(array->data);

        array->size = 0;
        array->capacity = 0;
        array->element_size = 0;

        memory_destroy(array);
    }
}

/**
 * 
 * @function array_destroy_custom
 * @brief Free the array memory
 * @params {array_t*} array - Array
 * @params {void (*free_fn)(void*)} free_fn - Custom free function
 * @returns {void}
 * 
 */
void array_destroy_custom(array_t* array, void (*free_fn)(void*))
{
    if (array != NULL) {
        if (array->data != NULL) {
            if (free_fn != NULL) {
                for (size_t i = 0; i < array->size; i++) {
                    if (array->data[i] != NULL) {
                        free_fn(array->data[i]);
                    }
                }
            }
            
            memory_destroy(array->data);
        }

        array->size = 0;
        array->capacity = 0;
        array->element_size = 0;

        memory_destroy(array);
    }
}

/**
 * 
 * @function array_print
 * @brief Print the array
 * @params {array_t*} array - Array
 * @returns {void}
 * 
 */
void array_print(array_t* array)
{
    printf("Array: %zu\n", array->size);

    for (size_t i = 0; i < array->size; i++) {
        printf("\t%p\n", array->data[i]);
    }
}

/**
 * 
 * @function array_token_print
 * @brief Print the token array
 * @params {array_token_t*} array - Token array
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

/**
 * 
 * @function array_node_print
 * @brief Print the node array
 * @params {array_node_t*} array - Node array
 * @returns {void}
 * 
 */
void array_node_print(array_node_t* array)
{
    printf("Node array: %zu\n", array->size);

    for (size_t i = 0; i < array->size; i++) {
        printf("\t");
        ast_node_t* node = array_get(array, i);
        node->print(node);
    }
}

/**
 * 
 * @function array_layout_attribute_print
 * @brief Print the attribute array
 * @params {array_layout_attribute_t*} array - Attribute array
 * @returns {void}
 * 
 */
void array_layout_attribute_print(array_layout_attribute_t* array)
{
    printf("Attribute array: %zu\n", array->size);

    if (array->size == 0) {
		printf("Array is empty\n");
        return;
    }

    for (size_t i = 0; i < array->size; i++) {
        printf("\t");
        ast_layout_attribute_t* attribute = array_get(array, i);

        attribute->print(attribute);
    }
}

/**
 * 
 * @function array_size
 * @brief Get the size of the array
 * @params {array_t*} array - Array
 * @returns {size_t} - Size of the array
 * 
 */
size_t array_size(array_t* array)
{
    return array->size;
}

/**
 * 
 * @function array_token_destroy
 * @brief Free the token array memory
 * @params {array_token_t*} array - Token array
 * @returns {void}
 * 
 */
void array_token_destroy(array_token_t* array)
{
    if (array != NULL) {
        for (size_t i = 0; i < array->size; i++) {
            token_t* token = array_get(array, i);
            
            if (token != NULL) {
                token->destroy(token);
            }
        }

        array_destroy(array);
    }
}

