#include "parser_parse_statement_print.h"

ast_t* parser_parse_statement_print(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    parser_expect(parser, TOKEN_TYPE_KEYWORD_PRINT);

    ast_t* expressions = parser_parse_expressions(parser);
    size_t expression_count = expressions->raw.expressions_value->value_count;

    ast_t* ast = ast_statement_print_create(expressions, expression_count);
    return ast;
}
