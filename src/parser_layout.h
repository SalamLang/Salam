#ifndef _PARSER_LAYOUT_H_
#define _PARSER_LAYOUT_H_

#include <stdbool.h>

#include "ast.h"
#include "base.h"
#include "lexer.h"
#include "parser_layout.h"
#include "validator.h"

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
void parser_parse_layout_block(ast_layout_block_t *block, lexer_t *lexer);

/**
 *
 * @function parser_parse_layout
 * @brief Parse the layout
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_layout(lexer_t *lexer);

/**
 *
 * @function parser_layout
 * @brief Parse the layout
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_layout(lexer_t *lexer);

/**
 *
 * @function parser_parse_layout_block_children
 * @brief Parse the block children
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {lexer_t*} lexer - Lexer
 * @params {char*} name - Name of the attribute
 * @params {token_t*} last_name - Last token
 * @returns {void}
 *
 */
void parser_parse_layout_block_children(ast_layout_block_t *block,
                                        lexer_t *lexer, char *name,
                                        token_t *last_name);

/**
 *
 * @function parser_parse_layout_block_attribute
 * @brief Parse the block attribute
 * @params {bool} onlyStyle - Only style
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {hashmap_t*} normal - Normal hashmap
 * @params {lexer_t*} lexer - Lexer
 * @params {char*} name - Name of the attribute
 * @params {token_t*} last_name - Last token
 * @returns {void}
 *
 */
void parser_parse_layout_block_attribute(bool onlyStyle,
                                         ast_layout_block_t *block,
                                         hashmap_t *normal, lexer_t *lexer,
                                         char *name, token_t *last_name);

/**
 *
 * @function parser_parse_layout_node
 * @brief Parsing layout node
 * @params {lexer_t*} lexer - Lexer
 * @params {char*} name - Name of the node
 * @params {token_t*} last_name - Last token
 * @returns {ast_layout_node_t*} - AST layout node
 *
 */
ast_layout_node_t *parser_parse_layout_node(lexer_t *lexer, char *name,
                                            token_t *last_name);

/**
 *
 * @function parser_parse_layout_value
 * @brief Parse the layout value
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST Layout attribute value
 *
 */
ast_value_t *parser_parse_layout_value(lexer_t *lexer);

/**
 *
 * @function parser_parse_layout_block_style_state
 * @brief Parse the block style state
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {lexer_t*} lexer - Lexer
 * @params {char*} name - Name of the attribute
 * @params {token_t*} last_name - Last token
 * @returns {void}
 *
 */
void parser_parse_layout_block_style_state(ast_layout_block_t *block,
                                           lexer_t *lexer, char *name,
                                           token_t *last_name);

/**
 *
 * @function parser_parse_layout_name
 * @brief Parse the layout name
 * @params {lexer_t*} lexer - Lexer
 * @params {token_t**} last_name - Last token
 * @returns {string_t*} - String
 *
 */
string_t *parser_parse_layout_name(lexer_t *lexer, token_t **last_name);

/**
 *
 * @function parser_parse_layout_values
 * @brief Parse the layout values
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 *
 */
array_value_t *parser_parse_layout_values(lexer_t *lexer);

#endif
