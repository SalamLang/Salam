#include <stage/ast/ast_program/ast_program_destroy/ast_program_destroy.h>

void ast_program_destroy(ast_program_t* program)
{
    // if (!program) return;

    for (size_t i = 0; i < program->declaration_count; ++i)
        ast_destroy(program->declarations[i]);
    memory_destroy(program->declarations);
    memory_destroy(program);
}
