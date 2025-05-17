#include <stage/ast/asts/ast_variable_decl/ast_variable_decl_log/ast_variable_decl_log.h>

void ast_variable_decl_log(ast_variable_decl_t* variable_decl)
{
    if (!variable_decl) {
        log_info("variable_decl is NULL");
        return;
    }

    log_info("variable_decl: todo");
}
