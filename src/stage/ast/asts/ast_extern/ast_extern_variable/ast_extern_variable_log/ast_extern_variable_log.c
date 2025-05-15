#include <stage/ast/asts/ast_extern/ast_extern_variable/ast_extern_variable_log/ast_extern_variable_log.h>

void ast_extern_variable_log(ast_extern_variable_t* extern_variable)
{
    if (!extern_variable) {
        log_info("Extern Variable is NULL");
        return;
    }

    log_info("Extern Variable...");
}
