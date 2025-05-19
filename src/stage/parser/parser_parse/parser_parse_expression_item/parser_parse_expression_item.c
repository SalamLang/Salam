#include "parser_parse_expression_item.h"

ast_t* parser_parse_expression_item(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    // TODO: only supprting literal value for now...
    ast_t* ast_literal = parser_parse_expression_literal(parser);

    ast_t* ast = ast_expression_item_create(AST_EXPRESSION_TYPE_LITERAL, NULL);
    ast->raw.expression_item_value.raw.literal = ast_literal->raw.expression_literal_value;
    return ast;
}
