#include <stage/ast/asts/ast_program/ast_program_direct_destroy/ast_program_direct_destroy.h>

void ast_program_direct_destroy(ast_program_t program)
{
    array_destroy(program.declarations);
}
