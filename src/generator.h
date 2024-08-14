#ifndef aGENERATOR_H_
#define _GENERATOR_H_

#include <stddef.h>
#include <string.h>

#include "memory.h"
#include "string.h"
#include "file.h"
#include "ast.h"
#include "validator.h"

typedef struct {
	ast_t* ast;

	string_t* html;
	string_t* css;
	string_t* js;
} generator_t;

/**
 *
 * @function generator_create
 * @brief Create a generator
 * @params {ast_t*} ast - AST tree
 * @returns {generator_t*}
 *
 */
generator_t* generator_create(ast_t* ast);

/**
 *
 * @function generator_destroy
 * @brief Destroy a generator
 * @params {generator_t*} generator
 * @returns {void}
 *
 */
void generator_destroy(generator_t* generator);

/**
 *
 * @function generator_debug
 * @brief Debug the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_debug(generator_t* generator);

/**
 *
 * @function generator_save
 * @brief Save the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_save(generator_t* generator);

/**
 *
 * @function generator_code
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code(generator_t* generator);

/**
 *
 * @function generator_code_layout_html
 * @brief Generate the HTML code for the layout block
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} html - HTML string
 * @returns {void}
 *
 */
void generator_code_layout_html(ast_layout_block_t* layout_block, string_t* html);

/**
 *
 * @function generator_code_layout_attributes
 * @brief Generate the HTML code for the layout block attributes
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_attributes(ast_layout_block_t* block);

/**
 *
 * @function generator_code_layout_block
 * @brief Generate the HTML code for the layout block
 * @params {generator_t*} generator - Generator
 * @params {array_t*} children - Children
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_block(generator_t* generator, array_t* children);

/**
 * 
 * @function generator_code_layout_body
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} body - Body
 * @returns {void}
 * 
 */
void generator_code_layout_body(generator_t* generator, ast_layout_block_t* layout_block, string_t* body);

/**
 * 
 * @function generator_code_layout_style_name
 * @brief Convert AST layout attribute type to CSS attribute name
 * @params {ast_layout_attribute_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 * 
 */
char* generator_code_layout_style_name(ast_layout_attribute_type_t type);

/**
 * 
 * @function generator_code_layout_style_value
 * @brief Convert AST layout attribute values to CSS attribute values
 * @params {ast_layout_attribute_t*} attribute - Layout Attribute
 * @params {ast_layout_node_type_t} parent_node_type - Parent Node Type
 * @returns {char*} values_str - Values
 * 
 */
char* generator_code_layout_style_value(ast_layout_attribute_t* attribute, ast_layout_node_type_t parent_node_type);

#endif
