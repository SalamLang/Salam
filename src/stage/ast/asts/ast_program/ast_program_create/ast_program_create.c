#include <stage/ast/asts/ast_program/ast_program_create/ast_program_create.h>

ast_t* ast_program_create(array_t* declarations, size_t declaration_count)
{
    ast_t* node = ast_alloc(AST_TYPE_PROGRAM);
    node->raw.program_value.declarations = declarations;
    node->raw.program_value.declaration_count = declaration_count;
    return node;
}
