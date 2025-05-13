#include <stage/ast/ast_program_create/ast_program_create.h>

ast_t* ast_program_create(ast_t** declarations, size_t declaration_count)
{
    ast_t* node = ast_alloc(AST_TYPE_PROGRAM, ast_destroy);
    node->program.declarations = declarations;
    node->program.declaration_count = declaration_count;
    return node;
}
