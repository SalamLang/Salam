#include "parser_parse_expression_item.h"

ast_t* parser_parse_expression_item(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    // TODO: only supprting literal value for now...
    ast_t* ast_literal = parser_parse_expression_literal(parser);

    return ast_literal;
}
