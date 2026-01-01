#include "parser_parse_variable_decl.h"

ast_t* parser_parse_variable_decl(parser_t* parser) {
    DEBUG_ME;
    if (!parser) return NULL;
    ast_t* type = parser_parse_type(parser);

    token_t* name = parser_expect_get(parser, TOKEN_TYPE_IDENTIFIER);
    ast_t* value = NULL;

    if (parser_operator_skip(parser, TOKEN_OPERATOR_TYPE_ASSIGN)) {
        value = parser_parse_expression_item(parser);
    }

    ast_t* ast = ast_variable_decl_create(type, name->source, value);
    return ast;
}
