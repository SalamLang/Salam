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

#include "array_custom.h"

/**
 *
 * @function array_string_token
 * @brief Convert the token array to a string
 * @params {array_t*} array - Array
 * @params {char*} separator - Separator
 * @returns {char*} - String
 *
 */
char *array_string_token(array_t *array, char *separator) {
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return NULL;
    }

    string_t *str = string_create(16);
    for (size_t i = 0; i < array->length; i++) {
        token_t *token = array_get(array, i);
        string_append_str(str, token_value_stringify(token));

        if (i < array->length - 1) {
            string_append_str(str, separator);
        }
    }

    char *buffer = string_strdup(str->data);
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
void array_destroy_custom(array_t *array, void (*free_fn)(void *)) {
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
 * @function array_token_print
 * @brief Print the token array
 * @params {array_token_t*} array - Token array
 * @returns {void}
 *
 */
void array_token_print(array_token_t *array) {
    DEBUG_ME;
    printf("Array Token: ");
    printf("%zu\n", array->length);
    
    char *str = array_token_stringify(array);

    printf("%s\n", str);
    memory_destroy(str);
}

/**
 *
 * @function array_token_stringify
 * @brief Convert the token array to a string
 * @params {array_token_t*} array - Token array
 * @params {char*} separator - Separator
 * @returns {char*} - String
 *
 */
char *array_token_stringify(array_token_t *array) {
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return string_strdup("Token array is empty");
    }

    string_t *str = string_create(20);

    for (size_t i = 0; i < array->length; i++) {
        token_t *token = array_get(array, i);
        char *token_str = token->stringify(token);

        string_append_str(str, token_str);

        if (i < array->length - 1) {
            string_append_str(str, ",");
        }
    }

    return string_destroy_and_get(str);
}

/**
 *
 * @function array_node_destroy
 * @brief Free the node array memory
 * @params {array_node_t*} array - Node array
 * @returns {void}
 *
 */
void array_node_destroy(array_node_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_node_t *node = array_get(array, i);

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
void array_node_print(array_node_t *array) {
    DEBUG_ME;
    printf("Node array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_node_t *node = array_get(array, i);
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
void array_layout_attribute_print(array_layout_attribute_t *array) {
    DEBUG_ME;
    printf("Attribute array: ");
    printf("%zu\n", array->length);

    if (array->length == 0) {
        printf("Array is empty\n");
        return;
    }

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_layout_attribute_t *attribute = array_get(array, i);

        attribute->print(attribute);
    }
}

/**
 *
 * @function array_token_destroy
 * @brief Free the token array memory
 * @params {array_token_t*} array - Token array
 * @returns {void}
 *
 */
void array_token_destroy(array_token_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        for (size_t i = 0; i < array->length; i++) {
            token_t *token = array_get(array, i);

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
void array_layout_node_destroy(array_node_layout_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_layout_node_t *node = array_get(array, i);

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
void array_layout_node_print(array_node_layout_t *array) {
    DEBUG_ME;
    printf("Node array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_layout_node_t *node =
            cast(ast_layout_node_t *, array_get(cast(array_t *, array), i));
        if (node != NULL) {
            node->print(node);
        } else {
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
void array_function_parameter_print(array_function_parameter_t *array) {
    DEBUG_ME;
    printf("Function parameter array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_function_parameter_t *parameter = array_get(array, i);

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
void array_function_parameter_destroy(array_function_parameter_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_function_parameter_t *parameter = array_get(array, i);

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
void array_function_destroy(array_function_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_function_t *function = array_get(array, i);

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
void array_function_print(array_function_t *array) {
    DEBUG_ME;
    printf("Function array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_function_t *function = array_get(array, i);

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
void array_if_destroy(array_if_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_if_t *if_statement = array_get(array, i);

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
void array_if_print(array_if_t *array) {
    DEBUG_ME;
    printf("If array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_if_t *if_statement = array_get(array, i);

        if_statement->print(if_statement);
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
void array_value_destroy(array_value_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_value_t *value = array_get(array, i);

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
void array_block_destroy(array_block_t *array) {
    DEBUG_ME;
    if (array != NULL) {
        if (array->data != NULL) {
            for (size_t i = 0; i < array->length; i++) {
                ast_block_t *block = array_get(array, i);

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
void array_block_print(array_block_t *array) {
    DEBUG_ME;
    if (array == NULL) {
        printf("Block array is NULL\n");
        return;
    }

    printf("Block array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_block_t *block = array_get(array, i);

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
array_value_t *array_value_create(size_t capacity) {
    DEBUG_ME;
    array_value_t *array = array_create(sizeof(ast_value_t *), capacity);

    array->print = cast(void (*)(void *), array_value_print);
    array->destroy = cast(void (*)(void *), array_value_destroy);

    return array;
}

/**
 *
 * @function array_value_print
 * @brief Print the value array
 * @params {array_value_t*} array - Value array
 * @returns {void}
 *
 */
void array_value_print(array_value_t *array) {
    DEBUG_ME;
    printf("Value array: %zu\n", array->length);

    for (size_t i = 0; i < array->length; i++) {
        printf("\t");
        ast_value_t *value = array_get(array, i);

        if (value == NULL) {
            printf("NULL\n");
            continue;
        } else {
            value->print(value);
        }
    }
}

/**
 *
 * @function array_value_first_string
 * @brief Get the first string from the value array
 * @params {array_value_t*} array - Array
 * @returns {char*} - String
 *
 */
char *array_value_first_stringify(array_value_t *array) {
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return NULL;
    }

    string_t *str = string_create(16);
    for (size_t i = 0; i < array->length; i++) {
        ast_value_t *value = array_get(array, i);

        if (value != NULL) {
            string_append_str(str, value->get_data(value));
            break;
        }
    }

    char *buffer = string_strdup(str->data);
    string_destroy(str);

    return buffer;
}

/**
 *
 * @function array_value_string
 * @brief Convert the attribute value array to a string
 * @params {array_value_t*} array - Array
 * @params {char*} separator - Separator
 * @returns {char*} - String
 *
 */
char *array_value_stringify(array_value_t *array, char *separator) {
    DEBUG_ME;
    if (array == NULL || array->length == 0) {
        return NULL;
    }

    string_t *str = string_create(16);
    for (size_t i = 0; i < array->length; i++) {
        ast_value_t *value = array_get(array, i);

        if (value != NULL) {
            string_append_str(str, value->get_data(value));

            if (i < array->length - 1) {
                string_append_str(str, separator);
            }
        }
    }

    char *buffer = string_strdup(str->data);
    string_destroy(str);

    return buffer;
}

/**
 *
 * @function array_value_copy
 * @brief Copy the attribute value array
 * @params {array_value_t*} values - Array
 * @returns {array_value_t*} - Copied array
 *
 */
array_value_t *array_value_copy(array_value_t *values) {
    DEBUG_ME;
    array_value_t *copy = array_value_create(values->length);

    for (size_t i = 0; i < values->length; i++) {
        ast_value_t *value = values->data[i];

        array_push(copy, ast_value_copy(value));
    }

    return copy;
}
