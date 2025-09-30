#include <stage/ast/asts/ast_function_decl/ast_function_decl_log/ast_function_decl_log.h>

void ast_function_decl_log(ast_function_decl_t* function_decl) {
    if (!function_decl) {
        log_info("function_decl is NULL\n");
        return;
    }

    log_info("function_decl: todo\n");
}
