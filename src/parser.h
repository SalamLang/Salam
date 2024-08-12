#ifndef _PARSER_H_
#define _PARSER_H_

#include "ast.h"

/**
 * 
 * @function match
 * @brief Match the token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match(lexer_t* lexer, token_type_t token_type);

/**
 * 
 * @function match_next
 * @brief Match the next token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_next(lexer_t* lexer, token_type_t token_type);

/**
 * 
 * @function match_prev
 * @brief Match the previous token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_prev(lexer_t* lexer, token_type_t token_type);

/**
 * 
 * @function expect
 * @brief Expect the token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {void}
 * 
 */
void expect(lexer_t* lexer, token_type_t token_type);

/**
 * 
 * @function unknown
 * @brief Unknown token type
 * @param {lexer_t*} lexer - Lexer
 * @returns {void}
 * 
 */
void unknown(lexer_t* lexer);

/**
 * 
 * @function ast_layout_create
 * @brief Create a new AST layout node
 * @returns {ast_layout_t*} - AST layout node
 * 
 */
ast_layout_block_t* ast_layout_block_create();

/**
 * 
 * @function parser_parse_block
 * @brief Parse the block
 * @param {lexer_t*} lexer - Lexer
 * @param {ast_block_type_t} type - Block type
 * @param {ast_type_t} block_parent_type - Block parent type
 * @returns {ast_block_t*} - AST block node
 * 
 */
ast_block_t* parser_parse_block(lexer_t* lexer, ast_block_type_t type, ast_type_t block_parent_type);

/**
 * 
 * @function ast_block_create
 * @brief Create a new AST block node
 * @returns {ast_block_t*} - AST block node
 * 
 */
ast_block_t* ast_block_create();

/**
 * 
 * @function ast_layout_node_create
 * @brief Create a new AST layout node
 * @param {ast_layout_node_type_t} type - Type of the layout node
 * @returns {ast_layout_node_t*} - AST layout node
 * 
 */
ast_layout_node_t* ast_layout_node_create(ast_layout_node_type_t type);

/**
 * 
 * @function parser_parse_layout_block
 * @brief Parse the block
 * @param {lexer_t*} lexer - Lexer
 * @param {ast_type_t} block_parent_type - Block parent type
 * @returns {ast_layout_block_t*} - AST layout block node
 * 
 */
ast_layout_block_t* parser_parse_layout_block(lexer_t* lexer, ast_type_t block_parent_type);

/**
 * 
 * @function parser_parse_layout
 * @brief Parse the layout
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_layout(lexer_t* lexer);

/**
 * 
 * @function parser_parse_node
 * @brief Parse the node
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_node(lexer_t* lexer);

/**
 * 
 * @function parser_parse
 * @brief Parse the tokens
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_t*} - AST
 * 
 */
ast_t* parser_parse(lexer_t* lexer);

/**
 * 
 * @function parser_layout
 * @brief Parse the layout
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_layout(lexer_t* lexer);


#endif