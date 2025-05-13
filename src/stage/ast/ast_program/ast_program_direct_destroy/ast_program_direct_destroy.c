#include <stage/ast/ast_program/ast_program_direct_destroy/ast_program_direct_destroy.h>

void ast_program_direct_destroy(ast_program_t program)
{
    for (size_t i = 0; i < program.declaration_count; ++i)
        ast_destroy(program.declarations[i]);
}
