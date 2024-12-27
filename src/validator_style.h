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

#ifndef _VALIDATOR_STYLE_H_
#define _VALIDATOR_STYLE_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "array.h"
#include "array_custom.h"
#include "ast.h"
#include "base.h"
#include "generator.h"
#include "hashmap.h"
#include "hashmap_custom.h"
#include "parser.h"
#include "string_buffer.h"
#include "validator.h"

typedef struct ast_layout_attribute_pair_t {
    char *input;
    char *output;
} ast_layout_attribute_pair_t;

/**
 *
 * @function has_font_extension
 * @brief Check if the value has a font extension
 * @params {char*} value - Value
 * @params {char**} out_extension - Output extension
 * @returns {bool} - True if the value has a font extension, false otherwise
 *
 */
bool has_font_extension(char *value, char **out_extension);

/**
 *
 * @function has_css_size_prefix
 * @brief Check if the CSS value has a size prefix
 * @params {char*} css_value - CSS value
 * @params {char**} css_output_value - CSS output value
 * @returns {bool} - True if the CSS value has a size prefix, false otherwise
 *
 */
bool has_css_size_prefix(char *css_value, char **css_output_value);

/**
 *
 * @function normalise_css_size
 * @brief Normalise the CSS size
 * @params {char*} attribute_value - Attribute value
 * @returns {char*} - Normalised CSS size
 *
 */
char *normalise_css_size(char *attribute_value);

/**
 *
 * @function attribute_css_size_value
 * @brief Convert the attribute value to a CSS size value
 * @params {char*} attribute_value - Attribute value
 * @returns {char*} - CSS size value
 *
 */
char *attribute_css_size_value(char *attribute_value);

/**
 *
 * @function is_style_attribute
 * @brief Check if the attribute type is a CSS attribute
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 *
 */
bool is_style_attribute(ast_layout_attribute_type_t type);

/**
 *
 * @function is_attribute_type_a_style
 * @brief Check if the attribute type is a style
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 *
 */
bool is_attribute_type_a_style(ast_layout_attribute_type_t type);

/**
 *
 * @function validate_style_value
 * @brief Validate the style value
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value(hashmap_t *styles, hashmap_t *new_styles,
                          ast_layout_attribute_t *attribute);

/**
 *
 * @function validate_style_value_sizes
 * @brief Validate the style values size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_sizes(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2, bool length_124);

/**
 *
 * @function validate_style_value_color
 * @brief Validate the style value color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_color(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_size
 * @brief Validate the style value size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_size(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_size_color
 * @brief Validate the style value size or color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_size_color(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_string
 * @brief Validate the style value string
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_string(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_percentage
 * @brief Validate the style value percentage
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_percentage(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_integer
 * @brief Validate the style value integer
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_integer(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_sizes_colors
 * @brief Validate the style values color or size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_sizes_colors(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_float
 * @brief Validate the style value float
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_float(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_number
 * @brief Validate the style value number
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 - Allowed
 * values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 - Allowed
 * values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_number(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

/**
 *
 * @function validate_style_value_colors
 * @brief Validate the style values color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_colors(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_pair_t *allowed_values1,
    const ast_layout_attribute_pair_t *allowed_values2);

#endif
