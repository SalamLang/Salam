#ifndef _VALIDATOR_H_
#define _VALIDATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "base.h"
#include "ast.h"
#include "hashmap.h"
#include "parser.h"

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
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {ast_layout_attribute_type_t} attribute_key_type - Attribute key type
 * @params {ast_layout_attribute_t*} attribute - AST layout attribute
 * @returns {bool} - True if the token belongs to the AST layout node, false otherwise
 * 
 */
bool token_belongs_to_ast_layout_node(ast_layout_block_t* block, ast_layout_attribute_type_t attribute_key_type, ast_layout_attribute_t* attribute);

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

#endif
