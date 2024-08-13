#ifndef aGENERATOR_H_
#define _GENERATOR_H_

#include <stddef.h>

#include "memory.h"
#include "string.h"
#include "file.h"
#include "ast.h"

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

#endif
