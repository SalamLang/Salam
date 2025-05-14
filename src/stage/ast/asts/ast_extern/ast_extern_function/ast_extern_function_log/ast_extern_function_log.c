#include <stage/ast/asts/ast_extern/ast_extern_function/ast_extern_function_log/ast_extern_function_log.h>

void ast_extern_function_log(ast_extern_function_t* extern_function)
{
    if (!extern_function) {
        log_info("Extern Function is NULL");
        return;
    }

    log_info("Extern Function...");
}
