#include <src/stage/ast/asts/ast_layout/ast_layout_log/ast_layout_log.h>

void ast_layout_log(ast_layout_t* ast_layout)
{
    if (!extern_decl) {
        log_info("Extern Declaration is NULL\n");
        return;
    }

    log_info("Extern Declaration...\n");
}
