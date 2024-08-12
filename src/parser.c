#include "parser.h"

#define PARSER_CURRENT ((token_t*)lexer->tokens->data[lexer->token_index])
#define PARSER_NEXT lexer->token_index++
#define PARSER_PREV lexer->token_index--
#define PARSER_CURRENT_NEXT ((token_t*)lexer->tokens->data[lexer->token_index + 1])
#define PARSER_CURRENT_PREV ((token_t*)lexer->tokens->data[lexer->token_index + 1])

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
    ast_node_t* node = ast_node_create(AST_NODE_TYPE_LAYOUT_NODE, PARSER_CURRENT->location);

    PARSER_NEXT;

    return node;
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
        ast_node_t* node = parser_layout(lexer);
        if (node) {
            array_push(ast->layout, node);
        }
    }

    return ast;
}
