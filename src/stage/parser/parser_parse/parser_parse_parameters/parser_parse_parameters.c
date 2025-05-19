#include <stage/parser/parser_parse/parser_parse_parameters/parser_parse_parameters.h>

ast_t* parser_parse_parameters(parser_t* parser)
{
    DEBUG_ME;
    if (!parser) return NULL;

    array_t* values = array_create((array_destroy_t)ast_destroy);

    // TODO

    ast_t* ast = ast_parameters_create(values, values->size);
    return ast;
}
