#include <src/stage/ast/asts/ast_layout/ast_layout_create/ast_layout_create.h>

ast_layout_t* ast_layout_create()
{
    DEBUG_ME;

    ast_layout_t* ast_layout = memory_create_zeroed(sizeof(ast_layout_t));

    return ast_layout;
}
