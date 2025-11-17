#include <stage/ast/asts/ast_layout/ast_layout_log/ast_layout_log.h>

void ast_layout_log(ast_layout_t* ast_layout)
{
    if (!ast_layout) {
        log_info("AST Layout is NULL\n");
        return;
    }

    log_info("AST Layout...\n");
}
