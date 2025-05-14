#include <stage/ast/asts/ast_program/ast_program_destroy/ast_program_destroy.h>

void ast_program_destroy(ast_program_t* program)
{
    // if (!program) return;

    array_destroy(program->declarations);
    memory_destroy(program);
}
