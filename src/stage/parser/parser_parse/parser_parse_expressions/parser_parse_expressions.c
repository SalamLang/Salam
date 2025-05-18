#include "parser_parse_expressions.h"

ast_t* parser_parse_expressions(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    ast_t* ast = ast_expressions_create();

    // TODO: handling multiple expression values...
    ast_t* expression = parser_parse_expression_item(parser);
    array_append(ast->raw.expressions_value.values, expression);
    ast->raw.expressions_value.value_count = ast->raw.expressions_value.values->size;

    return ast;
}
