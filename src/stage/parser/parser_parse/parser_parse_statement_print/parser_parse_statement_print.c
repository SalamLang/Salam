#include "parser_parse_statement_print.h"

ast_t* parser_parse_statement_print(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    parser_expect(parser, TOKEN_TYPE_KEYWORD_PRINT);

    ast_t* values = parser_parse_expressions(parser);
    size_t value_count = values->raw.expressions_value.value_count;
    return ast_statement_print_create(values, value_count);
}
