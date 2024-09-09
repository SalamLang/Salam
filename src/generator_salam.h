#ifndef _GENERATOR_SALAM_H_
#define _GENERATOR_SALAM_H_

#include "ast.h"
#include "generator.h"
#include "generator_salam.h"
#include "string_buffer.h"

/**
 *
 * @function generator_salam
 * @brief Generate the Salam code (cleaned and formatted)
 * @params {ast_t*} ast - AST
 * @returns {string_t*}
 *
 */
string_t* generator_salam(ast_t* ast);

/**
 *
 * @function generator_salam_layout
 * @brief Generate the Salam code for the layout
 * @params {ast_layout_t*} layout - Layout
 * @returns {void}
 *
 */
void generator_salam_layout(string_t* salam, ast_layout_t* layout);

/**
 *
 * @function generator_salam_layout_block
 * @brief Generate the Salam code for the layout block
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 *
 */
void generator_salam_layout_block(string_t* salam, ast_layout_block_t* block);

/**
 *
 * @function generator_salam_ident
 * @brief Generate the Salam code for the indentation
 * @params {string_t*} buffer - Buffer
 * @params {size_t} ident - Indentation
 * @returns {void}
 *
 */
void generator_salam_ident(string_t* buffer, size_t ident);

#endif
