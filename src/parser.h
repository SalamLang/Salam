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

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>

#include "ast.h"
#include "base.h"
#include "lexer.h"
#include "validator.h"

#define PARSER_NEXT lexer->token_index++
#define PARSER_PREV lexer->token_index--

#define PARSER_CURRENT ((token_t *)lexer->tokens->data[lexer->token_index])
#define PARSER_CURRENT_NEXT \
    ((token_t *)lexer->tokens->data[lexer->token_index + 1])
#define PARSER_CURRENT_PREV \
    ((token_t *)lexer->tokens->data[lexer->token_index + 1])

#include "parser_layout.h"

/**
 *
 * @function match
 * @brief Match the token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 *
 */
bool match(lexer_t *lexer, token_type_t token_type);

/**
 *
 * @function match_next
 * @brief Match the next token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 *
 */
bool match_next(lexer_t *lexer, token_type_t token_type);

/**
 *
 * @function match_prev
 * @brief Match the previous token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 *
 */
bool match_prev(lexer_t *lexer, token_type_t token_type);

/**
 *
 * @function expect
 * @brief Expect the token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {void}
 *
 */
void expect(lexer_t *lexer, token_type_t token_type);

/**
 *
 * @function parser_parse_block
 * @brief Parse the block
 * @params {lexer_t*} lexer - Lexer
 * @params {ast_block_t*} block - AST Block
 * @returns {ast_block_t*} - AST block node
 *
 */
void parser_parse_block(lexer_t *lexer, ast_block_t *block);

/**
 *
 * @function parser_parse_node
 * @brief Parse the node
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_node(lexer_t *lexer);

/**
 *
 * @function parser_parse
 * @brief Parse the tokens
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_t*} - AST
 *
 */
ast_t *parser_parse(lexer_t *lexer);

/**
 *
 * @function parser_parse_function
 * @brief Parse the function
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_function(lexer_t *lexer);

/**
 *
 * @function parser_parse_value
 * @brief Parse the value
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST Layout value
 *
 */
ast_value_t *parser_parse_value(lexer_t *lexer);

/**
 *
 * @function parser_parse_if
 * @brief Parse the if
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_if(lexer_t *lexer);

/**
 *
 * @function parser_parse_expression
 * @brief Parse the expression
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST value
 *
 */
ast_value_t *parser_parse_expression(lexer_t *lexer);

/**
 *
 * @function parser_parse_return
 * @brief Parse the return
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_return(lexer_t *lexer);

/**
 *
 * @function parser_parse_expressions
 * @brief Parse the expressions
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 *
 */
array_value_t *parser_parse_expressions(lexer_t *lexer);

/**
 *
 * @function parser_parse_expressions_maybe
 * @brief Parse the expressions maybe
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 *
 */
array_value_t *parser_parse_expressions_maybe(lexer_t *lexer);

/**
 *
 * @function is_begin_block_token
 * @brief Check if the token is a begin block
 * @params {token_t*} token - Token
 * @returns {bool}
 *
 */
bool is_begin_block_token(token_t *token);

/**
 *
 * @function is_end_block_token
 * @brief Check if the token is an end block
 * @params {token_t*} token - Token
 * @returns {bool}
 *
 */
bool is_end_block_token(token_t *token);

/**
 *
 * @function expect_open_block
 * @brief Expect an open block
 * @returns {void}
 *
 */
void expect_open_block(lexer_t *lexer);

/**
 *
 * @function match_next_open_block
 * @brief Match the next open block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_next_open_block(lexer_t *lexer);

/**
 *
 * @function match_next_close_block
 * @brief Match the next close block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_next_close_block(lexer_t *lexer);

/**
 *
 * @function match_open_block
 * @brief Match the open block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_open_block(lexer_t *lexer);

/**
 *
 * @function match_close_block
 * @brief Match the close block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_close_block(lexer_t *lexer);

/**
 *
 * @function expect_close_block
 * @brief Expect an open block
 * @returns {void}
 *
 */
void expect_close_block(lexer_t *lexer);

#endif
