#include "parser.h"

#define PARSER_CURRENT ((token_t*)lexer->tokens->data[lexer->token_index])
#define PARSER_NEXT lexer->token_index++
#define PARSER_PREV lexer->token_index--
#define PARSER_CURRENT_NEXT ((token_t*)lexer->tokens->data[lexer->token_index + 1])
#define PARSER_CURRENT_PREV ((token_t*)lexer->tokens->data[lexer->token_index + 1])

/**
 * 
 * @function match
 * @brief Match the token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match(lexer_t* lexer, token_type_t token_type)
{
    return PARSER_CURRENT->type == token_type;
}

/**
 * 
 * @function match_next
 * @brief Match the next token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_next(lexer_t* lexer, token_type_t token_type)
{
    if (lexer->token_index + 1 >= lexer->tokens->size) {
        return false;
    }
    return PARSER_CURRENT_NEXT->type == token_type;
}

/**
 * 
 * @function match_prev
 * @brief Match the previous token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {bool}
 * 
 */
bool match_prev(lexer_t* lexer, token_type_t token_type)
{
    if (lexer->token_index - 1 < 0) {
        return false;
    }
    return PARSER_CURRENT_PREV->type == token_type;
}

/**
 * 
 * @function parser_layout
 * @brief Parse the layout
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_layout(lexer_t* lexer)
{
    ast_node_t* node = ast_node_create(AST_NODE_TYPE_ERROR, PARSER_CURRENT->location);

    PARSER_NEXT;

    return node;
}

/**
 * 
 * @function parser_parse_node
 * @brief Parse the node
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_node(lexer_t* lexer)
{
    if (match(lexer, TOKEN_LAYOUT)) {
        return parser_layout(lexer);
    }

    return NULL;
}

/**
 * 
 * @function parser_parse
 * @brief Parse the tokens
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_t*} - AST
 * 
 */
ast_t* parser_parse(lexer_t* lexer)
{
    ast_t* ast = ast_create();

    while (lexer->token_index < lexer->tokens->size) {
        ast_node_t* node = parser_parse_node(lexer);
        array_push(ast->layout, node);
    }

    return ast;
}
