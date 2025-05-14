#include <stage/ast/asts/ast_extern/ast_extern_decl/ast_extern_decl_log/ast_extern_decl_log.h>

void ast_extern_decl_log(ast_extern_decl_t* extern_decl)
{
    if (!extern_decl) {
        log_info("Extern Declaration is NULL");
        return;
    }

    log_info("Extern Declaration...");
}
