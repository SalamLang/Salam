#include "parser_parse_statement_return.h"

ast_t* parser_parse_statement_return(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    parser_expect(parser, TOKEN_TYPE_KEYWORD_RETURN);

    ast_t* expressions = parser_parse_expressions(parser);
    size_t expression_count = expressions->raw.expressions_value->value_count;

    // ast_t* expression = parser_parse_expression_item(parser);
    // size_t expression_count = 1;
    // array_t* expressions_list = array_create((array_destroy_t)ast_destroy);
    // array_append(expressions_list, expression);
    // ast_t* expressions = ast_expressions_create();
    // array_append(expressions->raw.expressions_value.values, expressions_list);

    ast_t* ast = ast_statement_return_create(expressions, expression_count);
    return ast;
}
