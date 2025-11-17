#include <stage/ast/asts/ast_layout/ast_layout_destroy/ast_layout_destroy.h>

void ast_layout_destroy(ast_layout_t* ast_layout)
{
    DEBUG_ME;

    if (ast_layout->root != NULL) {
        ast_destroy(ast_layout->root);
    }

    memory_destroy(ast_layout);
}
