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

#ifndef _ARRAY_CUSTOM_H_
#define _ARRAY_CUSTOM_H_

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "ast.h"
#include "base.h"
#include "lexer.h"
#include "memory.h"
#include "string_buffer.h"

/**
 *
 * @function array_destroy_custom
 * @brief Free the array memory
 * @params {array_t*} array - Array
 * @params {void (*free_fn)(void*)} free_fn - Custom free function
 * @returns {void}
 *
 */
void array_destroy_custom(array_t *array, void (*free_fn)(void *));

/**
 *
 * @function array_token_print
 * @brief Print the token array
 * @params {array_token_t*} array - Token array
 * @returns {void}
 *
 */
void array_token_print(array_token_t *array);

/**
 *
 * @function array_node_destroy
 * @brief Free the node array memory
 * @params {array_node_t*} array - Node array
 * @returns {void}
 *
 */
void array_node_destroy(array_node_t *array);

/**
 *
 * @function array_node_print
 * @brief Print the node array
 * @params {array_node_t*} array - Node array
 * @returns {void}
 *
 */
void array_node_print(array_node_t *array);

/**
 *
 * @function array_token_destroy
 * @brief Free the token array memory
 * @params {array_token_t*} array - Token array
 * @returns {void}
 *
 */
void array_token_destroy(array_token_t *array);

/**
 *
 * @function array_layout_attribute_print
 * @brief Print the attribute array
 * @params {array_layout_attribute_t*} array - Attribute array
 * @returns {void}
 *
 */
void array_layout_attribute_print(array_layout_attribute_t *array);

/**
 *
 * @function array_string_token
 * @brief Convert the token array to a string
 * @params {array_t*} array - Array
 * @params {char*} separator - Separator
 * @returns {char*} - String
 *
 */
char *array_string_token(array_t *array, char *separator);

/**
 *
 * @function array_layout_attribute_print
 * @brief Print the attribute array
 * @params {array_layout_attribute_t*} array - Attribute array
 * @returns {void}
 *
 */
void array_layout_node_print(array_node_layout_t *node);

/**
 *
 * @function array_layout_node_destroy
 * @brief Free the node array memory
 * @params {array_node_t*} array - Node array
 * @returns {void}
 *
 */
void array_layout_node_destroy(array_node_layout_t *array);

/**
 *
 * @function array_function_parameter_print
 * @brief Print the function parameter array
 * @params {array_function_parameter_t*} array - Function parameter array
 * @returns {void}
 *
 */
void array_function_parameter_print(array_function_parameter_t *array);

/**
 *
 * @function array_function_parameter_destroy
 * @brief Free the function parameter array memory
 * @params {array_function_parameter_t*} array - Function parameter array
 * @returns {void}
 *
 */
void array_function_parameter_destroy(array_function_parameter_t *array);

/**
 *
 * @function array_function_print
 * @brief Print the function array
 * @params {array_function_t*} array - Function array
 * @returns {void}
 *
 */
void array_function_print(array_function_t *array);

/**
 *
 * @function array_function_destroy
 * @brief Free the function array memory
 * @params {array_function_t*} array - Function array
 * @returns {void}
 */
void array_function_destroy(array_function_t *array);

/**
 *
 * @function array_if_destroy
 * @brief Free the if array memory
 * @params {array_if_t*} array - If array
 * @returns {void}
 *
 */
void array_if_destroy(array_if_t *array);

/**
 *
 * @function array_if_print
 * @brief Print the if array
 * @params {array_if_t*} array - If array
 * @returns {void}
 *
 */
void array_if_print(array_if_t *array);

/**
 *
 * @function array_value_print
 * @brief Print the value array
 * @params {array_value_t*} array - Value array
 * @returns {void}
 *
 */
void array_value_print(array_value_t *array);

/**
 *
 * @function array_value_destroy
 * @brief Free the value array memory
 * @params {array_value_t*} array - Value array
 * @returns {void}
 *
 */
void array_value_destroy(array_value_t *array);

/**
 *
 * @function array_block_print
 * @brief Print the block array
 * @params {array_block_t*} array - Block array
 * @returns {void}
 *
 */
void array_block_print(array_block_t *array);

/**
 *
 * @function array_block_destroy
 * @brief Free the block array memory
 * @params {array_block_t*} array - Block array
 * @returns {void}
 *
 */
void array_block_destroy(array_block_t *array);

/**
 *
 * @function array_value_create
 * @brief Create a new value array
 * @params {size_t} capacity - Initial capacity of the array
 * @returns {array_value_t*} - Pointer to the created array
 *
 */
array_value_t *array_value_create(size_t capacity);

/**
 *
 * @function array_value_destroy
 * @brief Free the attribute value array memory
 * @params {array_value_t*} array - Attribute value array
 * @returns {void}
 *
 */
void array_value_destroy(array_value_t *array);

/**
 *
 * @function array_value_print
 * @brief Print the attribute value array
 * @params {array_value_t*} array - Attribute value array
 * @returns {void}
 *
 */
void array_value_print(array_value_t *array);

/**
 *
 * @function array_value_string
 * @brief Convert the attribute value array to a string
 * @params {array_value_t*} array - Array
 * @params {char*} separator - Separator
 * @returns {char*} - String
 *
 */
char *array_value_stringify(array_value_t *array, char *separator);

/**
 *
 * @function array_value_first_string
 * @brief Get the first string from the attribute value array
 * @params {array_value_t*} array - Array
 * @returns {char*} - String
 *
 */
char *array_value_first_stringify(array_value_t *array);

/**
 *
 * @function array_value_copy
 * @brief Copy the attribute value array
 * @params {array_value_t*} values - Array
 * @returns {array_value_t*} - Copied array
 *
 */
array_value_t *array_value_copy(array_value_t *values);

/**
 *
 * @function array_value_string
 * @brief Convert the attribute value array to a string
 * @params {array_value_t*} array - Array
 * @params {char*} separator - Separator
 * @returns {char*} - String
 *
 */
char *array_value_stringify(array_value_t *array, char *separator);

/**
 *
 * @function array_token_stringify
 * @brief Convert the token array to a string
 * @params {array_token_t*} array - Token array
 * @returns {char*} - String
 *
 */
char *array_token_stringify(array_token_t *array);

#endif
