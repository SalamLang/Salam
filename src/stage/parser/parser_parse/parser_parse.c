#include <stage/parser/parser_parse/parser_parse.h>

ast_t* parser_parse(parser_t* parser)
{
    size_t declaration_count = 0;
    array_t* declarations = array_create((array_destroy_t)ast_destroy);

    while (PARSER_HAS_CURRENT) {
        log_info("Parse node -> %s\n", token_name(PARSER_CURRENT->type));
        ast_t* ast = parser_parse_node(parser);
        if (!ast) {
            continue;
        }
        array_append(declarations, ast);
    }

    ast_t* ast = ast_program_create(declarations, declaration_count);
    return ast;
}
