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