#include <stage/parser/parser_parse/parser_parse_all/parser_parse_all.h>

ast_t* parser_parse_all(parser_t* parser)
{
    DEBUG_ME;
    size_t variable_declaration_count = 0;
    array_t* variable_declarations = array_create((array_destroy_t)ast_destroy);
    size_t function_declaration_count = 0;
    array_t* function_declarations = array_create((array_destroy_t)ast_destroy);

    while (PARSER_HAS_CURRENT) {
        log_info("Parse node -> %d\n", PARSER_CURRENT->type);
        log_info("Token name: %s\n", token_name(PARSER_CURRENT->type));
        ast_t* ast = parser_parse_node(parser);
        if (ast == NULL) {
            continue;
        } else {
            if (ast->base.type == AST_TYPE_FUNCTION_DECL) {
                array_append(function_declarations, ast);
                function_declaration_count++;
            } else if (ast->base.type == AST_TYPE_VARIABLE_DECL) {
                array_append(variable_declarations, ast);
                variable_declaration_count++;
            }
        }
    }

    ast_t* ast = ast_program_create(variable_declarations, function_declarations, variable_declaration_count, function_declaration_count);
    return ast;
}
