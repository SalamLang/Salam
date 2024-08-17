#ifndef _PARSER_H_
#define _PARSER_H_

#include "base.h"
#include "ast.h"
#include "lexer.h"
#include "validator.h"

/**
 * 
 * @function match
 * @brief Match the token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match(lexer_t* lexer, token_type_t token_type);

/**
 * 
 * @function match_next
 * @brief Match the next token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_next(lexer_t* lexer, token_type_t token_type);

/**
 * 
 * @function match_prev
 * @brief Match the previous token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_prev(lexer_t* lexer, token_type_t token_type);

/**
 * 
 * @function expect
 * @brief Expect the token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {void}
 * 
 */
void expect(lexer_t* lexer, token_type_t token_type);

/**
 * 
 * @function parser_parse_block
 * @brief Parse the block
 * @params {lexer_t*} lexer - Lexer
 * @params {ast_block_t*} block - AST Block
 * @returns {ast_block_t*} - AST block node
 * 
 */
void parser_parse_block(lexer_t* lexer, ast_block_t* block);

/**
 * 
 * @function parser_parse_layout_block
 * @brief Parse the block
 * @params {ast_layout_block_t*} block - AST layout block
 * @params {lexer_t*} lexer - Lexer
 * @params {ast_type_t} block_parent_type - Block parent type
 * @returns {void}
 * 
 */
void parser_parse_layout_block(ast_layout_block_t* block, lexer_t* lexer);

/**
 * 
 * @function parser_parse_layout
 * @brief Parse the layout
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_layout(lexer_t* lexer);

/**
 * 
 * @function parser_parse_node
 * @brief Parse the node
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_node(lexer_t* lexer);

/**
 * 
 * @function parser_parse
 * @brief Parse the tokens
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_t*} - AST
 * 
 */
ast_t* parser_parse(lexer_t* lexer);

/**
 * 
 * @function parser_layout
 * @brief Parse the layout
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_layout(lexer_t* lexer);

/**
 * 
 * @function parser_parse_layout_block_children
 * @brief Parse the block children
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {lexer_t*} lexer - Lexer
 * @returns {void}
 * 
 */
void parser_parse_layout_block_children(ast_layout_block_t* block, lexer_t* lexer);

/**
 * 
 * @function parser_parse_layout_node
 * @brief Parsing layout node
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_layout_node_t*} - AST layout node
 * 
 */
ast_layout_node_t* parser_parse_layout_node(lexer_t* lexer);

/**
 * 
 * @function parser_parse_function
 * @brief Parse the function
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_function(lexer_t* lexer);

/**
 * 
 * @function parser_parse_value
 * @brief Parse the value
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST Layout value
 * 
 */
ast_value_t* parser_parse_value(lexer_t* lexer);

/**
 * 
 * @function parser_parse_if
 * @brief Parse the if
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_if(lexer_t* lexer);

/**
 * 
 * @function parser_parse_expression
 * @brief Parse the expression
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST value
 * 
 */
ast_value_t* parser_parse_expression(lexer_t* lexer);

/**
 * 
 * @function parser_parse_return
 * @brief Parse the return
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_return(lexer_t* lexer);

/**
 * 
 * @function parser_parse_expressions
 * @brief Parse the expressions
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 * 
 */
array_value_t* parser_parse_expressions(lexer_t* lexer);

/**
 * 
 * @function parser_parse_expressions_maybe
 * @brief Parse the expressions maybe
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 * 
 */
array_value_t* parser_parse_expressions_maybe(lexer_t* lexer);

/**
 * 
 * @function parser_parse_layout_value
 * @brief Parse the layout value
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST Layout attribute value
 * 
 */
ast_value_t* parser_parse_layout_value(lexer_t* lexer);

/**
 * 
 * @function is_begin_block_token
 * @brief Check if the token is a begin block
 * @params {token_t*} token - Token
 * @returns {bool}
 * 
 */
bool is_begin_block_token(token_t* token);

/**
 * 
 * @function is_end_block_token
 * @brief Check if the token is an end block
 * @params {token_t*} token - Token
 * @returns {bool}
 * 
 */
bool is_end_block_token(token_t* token);

/**
 * 
 * @function expect_open_block
 * @brief Expect an open block
 * @returns {void}
 * 
 */
void expect_open_block(lexer_t* lexer);

/**
 * 
 * @function match_next_open_block
 * @brief Match the next open block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 * 
 */
bool match_next_open_block(lexer_t* lexer);

/**
 * 
 * @function match_next_close_block
 * @brief Match the next close block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 * 
 */
bool match_next_close_block(lexer_t* lexer);

#endif