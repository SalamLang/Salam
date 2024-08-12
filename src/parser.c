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
 * @function expect
 * @brief Expect the token type
 * @param {lexer_t*} lexer - Lexer
 * @param {token_type_t} token_type - Token type
 * @returns {void}
 * 
 */
void expect(lexer_t* lexer, token_type_t token_type)
{
    if (PARSER_CURRENT->type != token_type) {
        error(2, "Expected token type %s, got %s at line %d, column %d", token_name(token_type), token_name(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
    }

    PARSER_NEXT;
}

/**
 * 
 * @function unknown
 * @brief Unknown token type
 * @param {lexer_t*} lexer - Lexer
 * @returns {void}
 * 
 */
void unknown(lexer_t* lexer)
{
    error(2, "Unexpected token type %s at line %d, column %d", token_name(PARSER_CURRENT->type), PARSER_CURRENT->location.end_line, PARSER_CURRENT->location.end_column);
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
    if (lexer->token_index == 0) {
        return false;
    }
    return PARSER_CURRENT_PREV->type == token_type;
}

/**
 * 
 * @function parser_parse_block
 * @brief Parse the block
 * @param {lexer_t*} lexer - Lexer
 * @param {ast_node_type_t} block_parent_type - Block parent type
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_block(lexer_t* lexer, ast_node_type_t block_parent_type)
{
    ast_node_t* node = ast_node_create(AST_NODE_TYPE_BLOCK, PARSER_CURRENT->location);
    // node->block_parent_type = block_parent_type;

    expect(lexer, TOKEN_LEFT_BRACE);

    // while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
    //     ast_node_t* child = ast_node_create(AST_NODE_TYPE_ERROR, PARSER_CURRENT->location);

    //     if (match(lexer, TOKEN_LAYOUT)) {
    //         child = parser_parse_layout(lexer);
    //     }
    //     else {
    //         unknown(lexer);
    //     }

    //     if (block_parent_type == AST_NODE_TYPE_LAYOUT) {
    //         array_push(node->children, child);
    //     }
    //     else {
    //         array_push(node->children, child);
    //     }
    // }

    expect(lexer, TOKEN_RIGHT_BRACE);

    return node;
}

/**
 * 
 * @function parser_parse_layout_block
 * @brief Parse the block
 * @param {lexer_t*} lexer - Lexer
 * @param {ast_node_type_t} block_parent_type - Block parent type
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_layout_block(lexer_t* lexer, ast_node_type_t block_parent_type)
{
    ast_node_t* node = ast_node_create(AST_NODE_TYPE_LAYOUT_BLOCK, PARSER_CURRENT->location);
    node->layout_block = NULL;

    expect(lexer, TOKEN_LEFT_BRACE);

    while (PARSER_CURRENT->type != TOKEN_RIGHT_BRACE) {
        ast_node_t* child = ast_node_create(AST_NODE_TYPE_ERROR, PARSER_CURRENT->location);

        if (match(lexer, TOKEN_IDENTIFIER)) {

        }            
    }

    expect(lexer, TOKEN_RIGHT_BRACE);

    return node;
}

/**
 * 
 * @function parser_parse_layout
 * @brief Parse the layout
 * @param {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 * 
 */
ast_node_t* parser_parse_layout(lexer_t* lexer)
{
    PARSER_NEXT; // Eat the layout token

    ast_node_t* node = ast_node_create(AST_NODE_TYPE_LAYOUT, PARSER_CURRENT->location);

    if (match(lexer, TOKEN_LEFT_BRACE)) {
        node->layout = ast_node_create
        node->layout->block = parser_parse_layout_block(lexer, AST_NODE_TYPE_LAYOUT);
    }
    else {
        unknown(lexer);
    }


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
        return parser_parse_layout(lexer);
    }
    else {
        unknown(lexer);
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
        printf("Token: %s\n", token_name(PARSER_CURRENT->type));

        ast_node_t* node = parser_parse_node(lexer);
        array_push(ast->layout, node);
    }

    return ast;
}
