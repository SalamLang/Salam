#include <stage/ast/asts/ast_program/ast_program_log/ast_program_log.h>

void ast_program_log(ast_program_t* program)
{
    if (!program) {
        log_info("Program is NULL");
        return;
    }

    log_info("Program: TODO");
}
