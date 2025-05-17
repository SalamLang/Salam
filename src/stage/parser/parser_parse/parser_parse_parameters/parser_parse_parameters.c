#include <stage/parser/parser_parse/parser_parse_parameters/parser_parse_parameters.h>

ast_t* parser_parse_parameters(parser_t* parser)
{
    DEBUG_ME;
    // TODO
    array_t* values = array_create(sizeof(ast_t*), 0);
    size_t value_count = 0;
    ast_t* ast = ast_parameters_create(values, value_count);
    return ast;
}
