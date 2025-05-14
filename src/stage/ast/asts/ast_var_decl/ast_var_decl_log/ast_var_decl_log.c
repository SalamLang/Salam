#include <stage/ast/asts/ast_var_decl/ast_var_decl_log/ast_var_decl_log.h>

void ast_var_decl_log(ast_var_decl_t* var_decl)
{
    if (!var_decl) {
        log_info("var_decl is NULL");
        return;
    }

    log_info("var_decl: todo");
}
