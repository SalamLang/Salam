#include "parser.h"

/**
 *
 * @function is_begin_block_token
 * @brief Check if the token is a begin block
 * @params {token_t*} token - Token
 * @returns {bool}
 *
 */
bool is_begin_block_token(token_t *token) {
    return token->type == TOKEN_LEFT_BRACE;
}

/**
 *
 * @function is_close_block_token
 * @brief Check if the token is an end block
 * @params {token_t*} token - Token
 * @returns {bool}
 *
 */
bool is_close_block_token(token_t *token) {
    return token->type == TOKEN_RIGHT_BRACE;
}

/**
 *
 * @function expect_open_block
 * @brief Expect an open block
 * @returns {void}
 *
 */
void expect_open_block(lexer_t *lexer) { expect(lexer, TOKEN_TYPE_OPEN_BLOCK); }

/**
 *
 * @function expect_close_block
 * @brief Expect an open block
 * @returns {void}
 *
 */
void expect_close_block(lexer_t *lexer) {
    expect(lexer, TOKEN_TYPE_CLOSE_BLOCK);
}

/**
 *
 * @function match_next_open_block
 * @brief Match the next open block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_next_open_block(lexer_t *lexer) {
    return match_next(lexer, TOKEN_LEFT_BRACE);
}

/**
 *
 * @function match_open_block
 * @brief Match the open block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_open_block(lexer_t *lexer) {
    return match(lexer, TOKEN_TYPE_OPEN_BLOCK);
}

/**
 *
 * @function match_close_block
 * @brief Match the close block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_close_block(lexer_t *lexer) {
    return match(lexer, TOKEN_TYPE_CLOSE_BLOCK);
}

/**
 *
 * @function match_next_close_block
 * @brief Match the next close block
 * @params {lexer_t*} lexer - Lexer
 * @returns {bool}
 *
 */
bool match_next_close_block(lexer_t *lexer) {
    return match_next(lexer, TOKEN_RIGHT_BRACE);
}

/**
 *
 * @function match
 * @brief Match the token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 *
 */
bool match(lexer_t *lexer, token_type_t token_type) {
    DEBUG_ME;
    return PARSER_CURRENT->type == token_type;
}

/**
 *
 * @function expect
 * @brief Expect the token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {void}
 *
 */
void expect(lexer_t *lexer, token_type_t token_type) {
    DEBUG_ME;
    if (PARSER_CURRENT->type != token_type) {
        error_parser(2, "Expected token type %s, got %s at line %d, column %d",
                     token_type_keyword(token_type),
                     token_type_keyword(PARSER_CURRENT->type),
                     PARSER_CURRENT->location.end_line,
                     PARSER_CURRENT->location.end_column);
    }

    PARSER_NEXT;
}

/**
 *
 * @function match_next
 * @brief Match the next token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 *
 */
bool match_next(lexer_t *lexer, token_type_t token_type) {
    DEBUG_ME;
    if (lexer->token_index + 1 >= lexer->tokens->capacity) {
        return false;
    }

    return PARSER_CURRENT_NEXT->type == token_type;
}

/**
 *
 * @function match_prev
 * @brief Match the previous token type
 * @params {lexer_t*} lexer - Lexer
 * @params {token_type_t} token_type - Token type
 * @returns {bool}
 *
 */
bool match_prev(lexer_t *lexer, token_type_t token_type) {
    DEBUG_ME;
    if (lexer->token_index == 0) {
        return false;
    }

    return PARSER_CURRENT_PREV->type == token_type;
}

/**
 *
 * @function parser_parse_value
 * @brief Parse the value
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST Layout value
 *
 */
ast_value_t *parser_parse_value(lexer_t *lexer) {
    DEBUG_ME;
    token_t *token = PARSER_CURRENT;

    PARSER_NEXT;

    if (token->type == TOKEN_IDENTIFIER) {
        // TODO
        error_parser(2, "Identifier '%s' is not defined at line %d, column %d",
                     token->data.string, token->location.end_line,
                     token->location.end_column);
    }

    ast_value_type_t *type =
        ast_value_type_create(AST_TYPE_KIND_STRING, token->location);

    ast_value_t *value = ast_value_create(type, token_value_stringify(token));

    return value;
}

/**
 *
 * @function parser_parse_block
 * @brief Parse the block
 * @params {lexer_t*} lexer - Lexer
 * @params {ast_block_t*} block - AST Block
 * @returns {void}
 *
 */
void parser_parse_block(lexer_t *lexer, ast_block_t *block) {
    DEBUG_ME;

    expect_open_block(lexer);

    while (PARSER_CURRENT->type != TOKEN_TYPE_CLOSE_BLOCK) {
        ast_node_t *node = parser_parse_node(lexer);

        if (node == NULL) {
            error_parser(2, "Expected a node at line %d, column %d, but got %s",
                         PARSER_CURRENT->location.end_line,
                         PARSER_CURRENT->location.end_column,
                         token_type_keyword(PARSER_CURRENT->type));
            continue;
        } else {
            array_push(block->children, node);
        }
    }

    expect_close_block(lexer);
}

/**
 *
 * @function parser_parse_function
 * @brief Parse the function
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_function(lexer_t *lexer) {
    DEBUG_ME;
    ast_node_t *node =
        ast_node_create(AST_TYPE_FUNCTION, PARSER_CURRENT->location);

    PARSER_NEXT;  // Eat the function token

    token_t *function_name = PARSER_CURRENT;
    expect(lexer, TOKEN_IDENTIFIER);
    node->data.function = ast_function_create(function_name->data.string);

    // Optional ()
    if (match(lexer, TOKEN_LEFT_PAREN)) {
        PARSER_NEXT;  // Eat the left parenthesis token

        expect(lexer, TOKEN_RIGHT_PAREN);
    }

    parser_parse_block(lexer, node->data.function->block);

    return node;
}

/**
 *
 * @function parser_parse_expressions_maybe
 * @brief Parse the expressions maybe
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 *
 */
array_value_t *parser_parse_expressions_maybe(lexer_t *lexer) {
    DEBUG_ME;
    if (match(lexer, TOKEN_NUMBER_FLOAT) || match(lexer, TOKEN_NUMBER_INT) ||
        match(lexer, TOKEN_STRING) || match(lexer, TOKEN_PLUS) ||
        match(lexer, TOKEN_IDENTIFIER) || match(lexer, TOKEN_MINUS)
        // TODO: add more types
    ) {
        return parser_parse_expressions(lexer);
    }

    return array_value_create(1);
}

/**
 *
 * @function parser_parse_expressions
 * @brief Parse the expressions
 * @params {lexer_t*} lexer - Lexer
 * @returns {array_value_t*} - Array of AST values
 *
 */
array_value_t *parser_parse_expressions(lexer_t *lexer) {
    DEBUG_ME;
    array_value_t *values = array_value_create(1);

    ast_value_t *value = parser_parse_expression(lexer);
    array_push(values, value);

    while (match(lexer, TOKEN_COMMA)) {
        PARSER_NEXT;  // Eat the comma token

        ast_value_t *new_value = parser_parse_expression(lexer);
        if (new_value == NULL) {
            error_parser(
                2, "Expected an expression at line %d, column %d, but got %s",
                PARSER_CURRENT->location.end_line,
                PARSER_CURRENT->location.end_column,
                token_type_keyword(PARSER_CURRENT->type));
        }

        array_push(values, new_value);
    }

    return values;
}

/**
 *
 * @function parser_parse_expression
 * @brief Parse the expression
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_value_t*} - AST value
 *
 */
ast_value_t *parser_parse_expression(lexer_t *lexer) {
    DEBUG_ME;
    token_t *token = PARSER_CURRENT;

    ast_value_type_t *type = NULL;
    ast_value_t *value = NULL;

    if (match(lexer, TOKEN_IDENTIFIER)) {
        PARSER_NEXT;

        type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);
        value = ast_value_create(type, token->data.string);

        return value;
    } else if (match(lexer, TOKEN_STRING)) {
        PARSER_NEXT;

        type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);
        value = ast_value_create(type, token->data.string);

        return value;
    } else if (match(lexer, TOKEN_NUMBER_INT)) {
        PARSER_NEXT;

        type = ast_value_type_create(AST_TYPE_KIND_INT, token->location);
        value = ast_value_create(type, NULL);
        value->data.int_value = token->data.number_int;

        return value;
    } else if (match(lexer, TOKEN_NUMBER_FLOAT)) {
        PARSER_NEXT;

        type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);
        value = ast_value_create(type, NULL);
        value->data.float_value = token->data.number_float;

        return value;
    } else if (match(lexer, TOKEN_BOOLEAN)) {
        PARSER_NEXT;

        type = ast_value_type_create(AST_TYPE_KIND_STRING, token->location);
        value = ast_value_create(type, NULL);
        value->data.bool_value = token->data.boolean;

        return value;
    } else {
        error_parser(2,
                     "Expected an expression at line %d, column %d, but got %s",
                     token->location.end_line, token->location.end_column,
                     token_type_keyword(token->type));
    }

    return NULL;
}

/**
 *
 * @function parser_parse_print
 * @brief Parse the print
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_print(lexer_t *lexer) {
    DEBUG_ME;
    ast_node_t *node =
        ast_node_create(AST_TYPE_PRINT, PARSER_CURRENT->location);

    PARSER_NEXT;  // Eat the print token

    array_value_t *values = parser_parse_expressions_maybe(lexer);
    node->data.print = ast_print_create(values);

    return node;
}

/**
 *
 * @function parser_parse_return
 * @brief Parse the return
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_return(lexer_t *lexer) {
    DEBUG_ME;
    ast_node_t *node =
        ast_node_create(AST_TYPE_RETURN, PARSER_CURRENT->location);

    PARSER_NEXT;  // Eat the return token

    array_value_t *values = parser_parse_expressions(lexer);
    node->data.returns = ast_return_create(values);

    return node;
}

/**
 *
 * @function parser_parse_if
 * @brief Parse the if
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_if(lexer_t *lexer) {
    DEBUG_ME;
    ast_node_t *node = ast_node_create(AST_TYPE_IF, PARSER_CURRENT->location);

    PARSER_NEXT;  // Eat the if token

    ast_value_t *condition = parser_parse_expression(lexer);
    node->data.ifclause = ast_if_create(condition);

    parser_parse_block(lexer, node->data.ifclause->block);

    // Optional else and then one or more if or else if
    while (true) {
        if (match(lexer, TOKEN_ELSE)) {
            // else {} and stop
            if (match_next_open_block(lexer)) {
                PARSER_NEXT;  // Eat the else token

                ast_node_t *else_if =
                    ast_node_create(AST_TYPE_ELSE_IF, PARSER_CURRENT->location);

                else_if->data.ifclause = ast_else_create();
                parser_parse_block(lexer, else_if->data.ifclause->block);

                array_push(node->data.ifclause->else_blocks, else_if);

                break;  // Stop the loop, this is the last else
            }
            // else if {}
            else if (match_next(lexer, TOKEN_IF)) {
                PARSER_NEXT;  // Eat the else token

                ast_node_t *else_if =
                    ast_node_create(AST_TYPE_IF, PARSER_CURRENT->location);
                PARSER_NEXT;  // Eat the sub if token

                ast_value_t *condition = parser_parse_expression(lexer);
                else_if->data.ifclause = ast_elseif_create(condition);

                parser_parse_block(lexer, else_if->data.ifclause->block);

                array_push(node->data.ifclause->else_blocks, else_if);
            } else {
                error_parser(2,
                             "Expected a block or an else if at line %d, "
                             "column %d, but got %s",
                             PARSER_CURRENT->location.end_line,
                             PARSER_CURRENT->location.end_column,
                             token_type_keyword(PARSER_CURRENT->type));
            }
        } else {
            break;  // Stop the loop, no else or else if
        }
    }

    return node;
}

/**
 *
 * @function parser_parse_node
 * @brief Parse the node
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_node_t*} - AST node
 *
 */
ast_node_t *parser_parse_node(lexer_t *lexer) {
    DEBUG_ME;
    if (match(lexer, TOKEN_LAYOUT)) {
        return parser_parse_layout(lexer);
    } else if (match(lexer, TOKEN_FUNCTION)) {
        return parser_parse_function(lexer);
    } else if (match(lexer, TOKEN_IF)) {
        return parser_parse_if(lexer);
    } else if (match(lexer, TOKEN_RETURN)) {
        return parser_parse_return(lexer);
    } else if (match(lexer, TOKEN_PRINT)) {
        return parser_parse_print(lexer);
    } else {
        error_parser(2, "Unknown token '%s' as statement at line %d, column %d",
                     token_type_keyword(PARSER_CURRENT->type),
                     PARSER_CURRENT->location.end_line,
                     PARSER_CURRENT->location.end_column);
    }

    return NULL;
}

/**
 *
 * @function parser_parse
 * @brief Parse the tokens
 * @params {lexer_t*} lexer - Lexer
 * @returns {ast_t*} - AST
 *
 */
ast_t *parser_parse(lexer_t *lexer) {
    DEBUG_ME;
    ast_t *ast = ast_create();

    while (lexer->token_index < lexer->tokens->length) {
        if (PARSER_CURRENT->type == TOKEN_EOF) {
            break;
        }

        ast_node_t *node = parser_parse_node(lexer);

        if (node == NULL) {
            error_parser(2, "Expected a node at line %d, column %d, but got %s",
                         PARSER_CURRENT->location.end_line,
                         PARSER_CURRENT->location.end_column,
                         token_type_keyword(PARSER_CURRENT->type));
            continue;
        } else if (node->type == AST_TYPE_LAYOUT) {
            // Free the previous layout if it exists
            if (ast->layout != NULL) {
                ast_layout_destroy(ast->layout);
            }

            // Set the layout
            ast->layout = node->data.layout;

            // Free the node as we don't need it anymore, we just need layout
            // node
            if (node != NULL) {
                memory_destroy(node);
            }
        } else if (node->type == AST_TYPE_FUNCTION) {
            array_push(ast->functions, node->data.function);

            ast_node_destroy_notall(node);
        }
    }

    return ast;
}
