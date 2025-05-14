#include <stage/parser/parser_parse/parser_parse.h>

ast_t* parser_parse(parser_t* parser)
{
    size_t declaration_count = 0;
    array_t* declarations = array_create((array_destroy_t)ast_destroy);

    // TODO

    ast_t* ast = ast_program_create(declarations, declaration_count);
    return ast;
}
