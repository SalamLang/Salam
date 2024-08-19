#ifndef _VALIDATOR_H_
#define _VALIDATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "base.h"
#include "ast.h"
#include "hashmap.h"
#include "parser.h"
#include "generator.h"

typedef struct {
    const char* input;
    const char* output;
} ast_layout_attribute_style_pair_t;

/**
 * 
 * @var valid_layout_attributes
 * @brief Valid layout attributes
 * @type {ast_layout_attribute_type_t[]}
 */
extern ast_layout_attribute_type_t valid_layout_attributes[];

/**
 * 
 * @var valid_layout_attributes_length
 * @brief Valid layout attributes length
 * @type {size_t}
 */
extern size_t valid_layout_attributes_length;

/**
 * 
 * @function validate_layout_block
 * @brief Validate and modify the layout block
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 * 
 */
void validate_layout_block(ast_layout_block_t* block);

/**
 * 
 * @function is_attribute_type_in_array
 * @brief Check if the attribute type is in the array
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @params {ast_layout_attribute_type_t*} array - Array of attribute types
 * @params {size_t} array_size - Array size
 * @returns {bool} - True if the attribute type is in the array, false otherwise
 * 
 */
bool is_attribute_type_in_array(ast_layout_attribute_type_t type, ast_layout_attribute_type_t* array, size_t array_size);

/**
 * 
 * @function token_belongs_to_ast_layout_node
 * @brief Check if the token belongs to the AST layout node
 * @params {ast_layout_attribute_type_t} attribute_key_type - Attribute key type
 * @params {ast_layout_attribute_t*} attribute - AST layout attribute
 * @returns {bool} - True if the token belongs to the AST layout node, false otherwise
 * 
 */
bool token_belongs_to_ast_layout_node(ast_layout_attribute_type_t attribute_key_type, ast_layout_attribute_t* attribute);

/**
 * 
 * @function validate_layout_mainbody
 * @brief Validate the layout mainbody
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 * 
 */
void validate_layout_mainbody(ast_layout_block_t* block);

/**
 * 
 * @function is_layout_node_a_single_tag
 * @brief Check if the layout node is a single tag
 * @params {ast_layout_node_type_t} type - Layout node type
 * @returns {bool} - True if the layout node is a single tag, false otherwise
 * 
 */
bool is_layout_node_a_single_tag(ast_layout_node_type_t type);

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
 * @function is_style_attribute
 * @brief Check if the attribute type is a CSS attribute
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 * 
 */
bool is_style_attribute(ast_layout_attribute_type_t type);

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
bool validate_style_value(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute);

/**
 * 
 * @function normalise_css_size
 * @brief Normalise the CSS size
 * @params {char*} attribute_value - Attribute value
 * @returns {char*} - Normalised CSS size
 * 
 */
char* normalise_css_size(char* attribute_value);

/**
 * 
 * @function has_css_size_prefix
 * @brief Check if the CSS value has a size prefix
 * @params {char*} css_value - CSS value
 * @params {char**} css_output_value - CSS output value
 * @returns {bool} - True if the CSS value has a size prefix, false otherwise
 * 
 */
bool has_css_size_prefix(char* css_value, char** css_output_value);

/**
 * 
 * @function attribute_css_size_value
 * @brief Convert the attribute value to a CSS size value
 * @params {char*} attribute_value - Attribute value
 * @returns {char*} - CSS size value
 * 
 */
char* attribute_css_size_value(char* attribute_value);

/**
 * 
 * @function validate_style_value_sizes
 * @brief Validate the style value sizes
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_sizes(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute);

/**
 * 
 * @function validate_style_value_color
 * @brief Validate the style value color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_color(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2);

/**
 * 
 * @function validate_style_value_size
 * @brief Validate the style value size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_size(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2);

/**
 * 
 * @function validate_style_value_string
 * @brief Validate the style value string
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_string(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2);

/**
 * 
 * @function validate_style_value_percentage
 * @brief Validate the style value percentage
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_percentage(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2);

/**
 * 
 * @function validate_style_value_integer
 * @brief Validate the style value integer
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 - Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 - Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 * 
 */
bool validate_style_value_integer(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute, const ast_layout_attribute_style_pair_t* allowed_values1, const ast_layout_attribute_style_pair_t* allowed_values2);

#endif
