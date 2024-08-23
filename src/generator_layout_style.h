#ifndef _GENERATOR_LAYOUT_STYLE_H_
#define _GENERATOR_LAYOUT_STYLE_H_

#include <stddef.h>

#include "base.h"
#include "memory.h"
#include "string.h"
#include "generator.h"
#include "ast.h"
#include "ast_layout.h"
#include "ast_layout_style.h"
#include "string.h"

/**
 *
 * @function generator_code_layout_style_value
 * @brief Convert AST layout attribute values to CSS attribute values
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New Styles
 * @params {ast_layout_attribute_t*} attribute - Layout Attribute
 * @returns {void}
 *
 */
void generator_code_layout_style_value(hashmap_t* styles, hashmap_t* new_styles, ast_layout_attribute_t* attribute);

/**
 *
 * @function generator_code_layout_attribute_style_state_type_to_enduser_name
 * @brief Convert style attribute state type to enduser name
 * @params {ast_layout_attribute_style_state_type} type - Style Attribute State Type
 * @returns {char*} name - Name
 *
 */
char* generator_code_layout_attribute_style_state_type_to_enduser_name(ast_layout_attribute_style_state_type type);

/**
 *
 * @function generator_code_layout_attribute_style_state_type_to_name
 * @brief Convert style attribute state type to name
 * @params {ast_layout_attribute_style_state_type} type - Style Attribute State Type
 * @returns {char*} name - Name
 *
 */
char* generator_code_layout_attribute_style_state_type_to_name(ast_layout_attribute_style_state_type type);

/**
 *
 * @function generator_code_layout_pseudo_elements
 * @brief Generate the CSS code for the layout block pseudo elements
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @params {size_t*} css_attributes_length - CSS attributes length
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_pseudo_elements(generator_t* generator, ast_layout_block_t* block, size_t* css_attributes_length);

/**
 *
 * @function generator_code_layout_attribute_style_state_enduser_name_to_type
 * @brief Convert style attribute state enduser name to type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_style_state_type} type - Type
 *
 */
ast_layout_attribute_style_state_type generator_code_layout_attribute_style_state_enduser_name_to_type(char* name);

/**
 *
 * @function generator_code_layout_attribute_style_state_name_to_type
 * @brief Convert style attribute state name to type
 * @params {char*} name - Name
 * @returns {ast_layout_attribute_style_state_type} type - Type
 *
 */
ast_layout_attribute_style_state_type generator_code_layout_attribute_style_state_name_to_type(char* name);

/**
 *
 * @function generator_code_layout_attribute_style_state_type_to_generated_name
 * @brief Convert style attribute state type to generated name
 * @params {ast_layout_attribute_style_state_type} type - Style Attribute State Type
 * @returns {char*} name - Name
 *
 */
char* generator_code_layout_attribute_style_state_type_to_generated_name(ast_layout_attribute_style_state_type type);

#endif