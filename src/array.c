#include "base.h"
#include "array.h"

/**
 * 
 * @function array_create
 * @brief Create a new array
 * @params {size_t} element_capacity - Size of each element
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_t*} - Pointer to the created array
 * 
 */
array_t* array_create(size_t element_capacity, size_t capacity)
{
    DEBUG_ME;
    array_t* array = memory_allocate(sizeof(array_t));
    array->length = 0;
    array->capacity = capacity;
    array->element_capacity = element_capacity;
    array->data = memory_allocate(array->element_capacity * array->capacity);

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
 * @params {size_t} element_capacity - Size of each element
 * @returns {void}
 * 
 */
void array_init(array_t* array, size_t capacity, size_t element_capacity)
{
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
bool array_push(array_t* array, void* element)
{
    DEBUG_ME;
    if (array->length >= array->capacity) {
        array->capacity *= 2;
        array->data = memory_reallocate(array->data, array->element_capacity * array->capacity);
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
bool array_pop(array_t* array, void* element)
{
    DEBUG_ME;
    if (array->length == 0) {
        return false;
    }

    array->length--;
    memory_copy(element, (char*)array->data + (array->capacity * array->element_capacity), array->element_capacity);
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
    DEBUG_ME;
    array->capacity = new_capacity;
    array->data = memory_reallocate(array->data, array->element_capacity * new_capacity);
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
void array_destroy(array_t* array)
{
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
 * @params {char*} sepertor - Separator
 * @returns {char*} - String
 * 
 */
char* array_string(array_t* array, char* sepertor)
{
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return strdup("");
    }

    string_t* str = string_create(16);
    for (size_t i = 0; i < array->length; i++) {
        char* item = array_get(array, i);

        if (item != NULL) {
            string_append_str(str, item);
            
            if (i < array->length - 1) {
                string_append_str(str, sepertor);
            }
        }
    }

    char* buffer = strdup(str->data);
    string_destroy(str);

    return buffer;
}

/**
 * 
 * @function array_string_token
 * @brief Convert the token array to a string
 * @params {array_t*} array - Array
 * @params {char*} sepertor - Separator
 * @returns {char*} - String
 * 
 */
char* array_string_token(array_t* array, char* sepertor)
{
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return strdup("");
    }

    string_t* str = string_create(16);
    for (size_t i = 0; i < array->length; i++) {
        token_t* token = array_get(array, i);
        string_append_str(str, token_value(token));

        if (i < array->length - 1) {
            string_append_str(str, sepertor);
        }
    }

    char* buffer = strdup(str->data);
    string_destroy(str);

    return buffer;
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
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            if (free_fn != NULL) {
                for (size_t i = 0; i < array->length; i++) {
                    if (array->data[i] != NULL) {
                        free_fn(array->data[i]);
                    }
                }
            }
            
            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

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
    DEBUG_ME;
    printf("Array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
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
    DEBUG_ME;
    printf("Token array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        token_t* token = array_get(array, i);

        token->print(token);
    }
}

/**
 * 
 * @function array_node_destroy
 * @brief Free the node array memory
 * @params {array_node_t*} array - Node array
 * @returns {void}
 * 
 */
void array_node_destroy(array_node_t* array)
{
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_node_t* node = array_get(array, i);
                
                if (node != NULL) {
                    node->destroy(node);
                }
            }

            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
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
    DEBUG_ME;
    printf("Node array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
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
    DEBUG_ME;
    printf("Attribute array: %zu\n", array->length);

    if (array->length == 0) {
		printf("Array is empty\n");
        return;
    }

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_layout_attribute_t* attribute = array_get(array, i);

        attribute->print(attribute);
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
size_t array_capacity(array_t* array)
{
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
size_t array_length(array_t* array)
{
    DEBUG_ME;
    return array->length;
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
    DEBUG_ME;
    if (array != NULL) {
        for (size_t i = 0; i < array->length; i++) {
            token_t* token = array_get(array, i);
            
            if (token != NULL) {
                token->destroy(token);
            }
        }

        array_destroy(array);
    }
}

/**
 * 
 * @function array_layout_node_destroy
 * @brief Free the node array memory
 * @params {array_node_t*} array - Node array
 * @returns {void}
 * 
 */
void array_layout_node_destroy(array_node_layout_t* array)
{
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_layout_node_t* node = array_get(array, i);

                if (node != NULL) {
                    node->destroy(node);
                }
            }

            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
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
void array_layout_node_print(array_node_layout_t* array)
{
    DEBUG_ME;
    printf("Node array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_layout_node_t* node = cast(ast_layout_node_t*, array_get(cast(array_t*, array), i));
        if (node != NULL) {
            node->print(node);
        }
        else {
            printf("NULL\n");
        }
    }
}

/**
 * 
 * @function array_function_parameter_print
 * @brief Print the function parameter array
 * @params {array_function_parameter_t*} array - Function parameter array
 * @returns {void}
 * 
 */
void array_function_parameter_print(array_function_parameter_t* array)
{
    DEBUG_ME;
    printf("Function parameter array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_function_parameter_t* parameter = array_get(array, i);

        parameter->print(parameter);
    }
}

/**
 * 
 * @function array_function_parameter_destroy
 * @brief Free the function parameter array memory
 * @params {array_function_parameter_t*} array - Function parameter array
 * @returns {void}
 * 
 */
void array_function_parameter_destroy(array_function_parameter_t* array)
{
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_function_parameter_t* parameter = array_get(array, i);
                
                if (parameter != NULL) {
                    parameter->destroy(parameter);
                }
            }

            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
    }
}

/**
 * 
 * @function array_function_destroy
 * @brief Free the function array memory
 * @params {array_function_t*} array - Function array
 * @returns {void}
 */
void array_function_destroy(array_function_t* array)
{
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_function_t* function = array_get(array, i);
                
                if (function != NULL) {
                    function->destroy(function);
                }
            }

            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
    }
}

/**
 * 
 * @function array_function_print
 * @brief Print the function array
 * @params {array_function_t*} array - Function array
 * @returns {void}
 * 
 */
void array_function_print(array_function_t* array)
{
    DEBUG_ME;
    printf("Function array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_function_t* function = array_get(array, i);

        function->print(function);
    }
}

/**
 * 
 * @function array_if_destroy
 * @brief Free the if array memory
 * @params {array_if_t*} array - If array
 * @returns {void}
 * 
 */
void array_if_destroy(array_if_t* array)
{
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_if_t* if_statement = array_get(array, i);
                
                if (if_statement != NULL) {
                    if_statement->destroy(if_statement);
                }
            }

            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
    }
}

/**
 * 
 * @function array_if_print
 * @brief Print the if array
 * @params {array_if_t*} array - If array
 * @returns {void}
 * 
 */
void array_if_print(array_if_t* array)
{
    DEBUG_ME;
    printf("If array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_if_t* if_statement = array_get(array, i);

        if_statement->print(if_statement);
    }
}

/**
 * 
 * @function array_value_print
 * @brief Print the value array
 * @params {array_value_t*} array - Value array
 * @returns {void}
 * 
 */
void array_value_print(array_value_t* array)
{
    DEBUG_ME;
    printf("Value array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_value_t* value = array_get(array, i);

        value->print(value);
    }
}

/**
 * 
 * @function array_value_destroy
 * @brief Free the value array memory
 * @params {array_value_t*} array - Value array
 * @returns {void}
 * 
 */
void array_value_destroy(array_value_t* array)
{
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_value_t* value = array_get(array, i);
                
                if (value != NULL) {
                    value->destroy(value);
                }
            }

            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
    }
}

/**
 * 
 * @function array_block_destroy
 * @brief Free the block array memory
 * @params {array_block_t*} array - Block array
 * @returns {void}
 * 
 */
void array_block_destroy(array_block_t* array)
{
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_block_t* block = array_get(array, i);
                
                if (block != NULL) {
                    block->destroy(block);
                }
            }

            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
    }
}

/**
 * 
 * @function array_block_print
 * @brief Print the block array
 * @params {array_block_t*} array - Block array
 * @returns {void}
 * 
 */
void array_block_print(array_block_t* array)
{
    DEBUG_ME;
    if (array == NULL) {
        printf("Block array is NULL\n");
        return;
    }
    
    printf("Block array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_block_t* block = array_get(array, i);

        block->print(block);
    }
}

/**
 * 
 * @function array_value_create
 * @brief Create a new value array
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_value_t*} - Pointer to the created array
 * 
 */
array_value_t* array_value_create(size_t capacity)
{
    DEBUG_ME;
    array_value_t* array = array_create(sizeof(ast_value_t*), capacity);

    array->print = cast(void (*)(void*), array_value_print);
    array->destroy = cast(void (*)(void*), array_value_destroy);
    
    return array;
}

/**
 * 
 * @function array_layout_attribute_value_create
 * @brief Create a new attribute value array
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_layout_attribute_value_t*} - Pointer to the created array
 * 
 */
array_layout_attribute_value_t* array_layout_attribute_value_create(size_t capacity)
{
    DEBUG_ME;
    array_layout_attribute_value_t* array = array_create(sizeof(ast_layout_attribute_value_t*), capacity);

    array->destroy = cast(void (*)(void*), array_layout_attribute_value_destroy);
    array->print = cast(void (*)(void*), array_layout_attribute_value_print);

    return array;
}

/**
 * 
 * @function array_layout_attribute_value_print
 * @brief Print the attribute value array
 * @params {array_layout_attribute_value_t*} array - Attribute value array
 * @returns {void}
 * 
 */
void array_layout_attribute_value_print(array_layout_attribute_value_t* array)
{
    DEBUG_ME;
    printf("Attribute value array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_layout_attribute_value_t* value = array_get(array, i);
        
        if (value == NULL) {
            printf("NULL\n");
            continue;
        }
        else {
            value->print(value);
        }
    }
}

/**
 * 
 * @function array_layout_attribute_value_destroy
 * @brief Free the attribute value array memory
 * @params {array_layout_attribute_value_t*} array - Attribute value array
 * @returns {void}
 * 
 */
void array_layout_attribute_value_destroy(array_layout_attribute_value_t* array)
{
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_layout_attribute_value_t* value = array_get(array, i);

                if (value != NULL) {
                    value->destroy(value);
                }
            }

            memory_destroy(array->data);
        }

        array->capacity = 0;
        array->length = 0;
        array->element_capacity = 0;

        memory_destroy(array);
    }
}

/**
 * 
 * @function array_layout_attribute_value_string
 * @brief Convert the attribute value array to a string
 * @params {array_layout_attribute_value_t*} array - Array
 * @params {char*} seperator - Separator
 * @returns {char*} - String
 * 
 */
char* array_layout_attribute_value_string(array_layout_attribute_value_t* array, char* seperator)
{
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return strdup("");
    }

    string_t* str = string_create(16);
    for (size_t i = 0; i < array->length; i++) {
        ast_layout_attribute_value_t* value = array_get(array, i);

        if (value != NULL) {
            string_append_str(str, value->data); // TODO
            
            if (seperator != NULL && i < array->length - 1) {
                string_append_str(str, seperator);
            }
        }
    }

    char* buffer = strdup(str->data);
    string_destroy(str);

    return buffer;
}

/**
 * 
 * @function array_layout_attribute_value_first_string
 * @brief Get the first string from the attribute value array
 * @params {array_layout_attribute_value_t*} array - Array
 * @returns {char*} - String
 * 
 */
char* array_layout_attribute_value_first_string(array_layout_attribute_value_t* array)
{
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return strdup("");
    }

    string_t* str = string_create(16);
    for (size_t i = 0; i < array->length; i++) {
        ast_layout_attribute_value_t* value = array_get(array, i);

        if (value != NULL) {
            string_append_str(str, value->data); // TODO
            break;
        }
    }

    char* buffer = strdup(str->data);
    string_destroy(str);

    return buffer;
}
