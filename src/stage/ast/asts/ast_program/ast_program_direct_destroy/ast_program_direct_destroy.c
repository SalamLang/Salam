#include <stage/ast/asts/ast_program/ast_program_direct_destroy/ast_program_direct_destroy.h>

void ast_program_direct_destroy(ast_program_t program)
{
    DEBUG_ME;
    if (program.variable_declarations != NULL) {
        array_destroy(program.variable_declarations);
    }

    if (program.function_declarations != NULL) {
        array_destroy(program.function_declarations);
    }
}
