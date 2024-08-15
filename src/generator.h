#ifndef aGENERATOR_H_
#define _GENERATOR_H_

#include <stddef.h>
#include <string.h>

#include "memory.h"
#include "string.h"
#include "file.h"
#include "ast.h"
#include "validator.h"

typedef struct generator_identifier_t {
	char *current;
} generator_identifier_t;

typedef struct generator_t {
	ast_t* ast;

	string_t* html;
	string_t* css;
	string_t* js;

	bool inlineCSS;
	bool inlineJS;

	generator_identifier_t* identifier;
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
 * @params {generator_t} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_attributes(generator_t* generator, ast_layout_block_t* block);

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

/**
 * 
 * @function generator_code_head
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} head - Head
 * @returns {void}
 * 
 */
void generator_code_head(ast_layout_block_t* layout_block, string_t* head);

/**
 * 
 * @function generator_code_head_item
 * @params {ast_layout_attribute_t*} attribute - Attribute
 * @params {string_t*} head - Head
 * @returns {void}
 * 
 */
void generator_code_head_item(ast_layout_attribute_t* attribute, string_t* head);

/**
 * 
 * @function generator_identifier_init
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 * 
 */
void generator_identifier_init(generator_identifier_t* gen);

/**
 * 
 * @function generator_identifier_get
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {char*} identifier - Identifier
 * 
 */
char* generator_identifier_get(generator_identifier_t* gen);

/**
 * 
 * @function generator_identifier_destroy
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 * 
 */
void generator_identifier_destroy(generator_identifier_t* gen);

/**
 * 
 * @function generator_code_functions
 * @brief Generate the code for the functions
 * @params {generator_t*} generator - Generator
 * @returns {void}
 * 
 */
void generator_code_functions(generator_t* generator);

/**
 * 
 * @function generator_code_function
 * @brief Generate the code for the function
 * @params {generator_t*} generator - Generator
 * @params {ast_function_t*} function - Function
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_function(generator_t* generator, ast_function_t* function);

/**
 * 
 * @function generator_code_block
 * @brief Generate the code for the block
 * @params {generator_t*} generator - Generator
 * @params {ast_block_t*} block - Block
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_block(generator_t* generator, ast_block_t* block);

/**
 * 
 * @function generator_code_node
 * @brief Generate the code for the node
 * @params {generator_t*} generator - Generator
 * @params {ast_node_t*} node - Node
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_node(generator_t* generator, ast_node_t* node);

/**
 * 
 * @function generator_code_if
 * @brief Generate the code for the if clause
 * @params {generator_t*} generator - Generator
 * @params {ast_if_t*} ifclause - If clause
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_if(generator_t* generator, ast_if_t* ifclause);

/**
 * 
 * @function generator_code_value
 * @brief Generate the code for the value
 * @params {generator_t*} generator - Generator
 * @params {ast_value_t*} value - Value
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_value(generator_t* generator, ast_value_t* value);

/**
 * 
 * @function generator_code_type
 * @brief Generate the code for the type
 * @params {generator_t*} generator - Generator
 * @params {ast_value_type_t*} type - Type
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_type(generator_t* generator, ast_value_type_t* type);

/**
 * 
 * @function generator_code_return
 * @brief Generate the code for the return
 * @params {generator_t*} generator - Generator
 * @params {ast_return_t*} returns - Returns
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_return(generator_t* generator, ast_return_t* returns);

/**
 * 
 * @function generator_code_values
 * @brief Generate the code for the values
 * @params {generator_t*} generator - Generator
 * @params {array_t*} values - Values
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_values(generator_t* generator, array_t* values);

/**
 * 
 * @function generator_code_print
 * @brief Generate the code for the print
 * @params {generator_t*} generator - Generator
 * @params {ast_print_t*} print - Print
 * @returns {string_t*} code - Code
 * 
 */
string_t* generator_code_print(generator_t* generator, ast_print_t* print);

/**
 * 
 * @function generator_code_layout_node_type
 * @brief Convert AST layout node type to HTML node name
 * @params {ast_layout_node_type_t} type - Layout Node Type
 * @returns {char*} name - Name
 * 
 */
char* generator_code_layout_node_type(ast_layout_node_type_t type);

#endif
