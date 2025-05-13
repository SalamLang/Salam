#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_destroy(ast_t* node)
{
    // if (!node) return;

    switch (node->base.type) {
        case AST_TYPE_VAR_DECL:
            ast_var_decl_direct_destroy(node->var_decl);
            break;

        case AST_TYPE_FUNCTION_DECL:
            ast_function_decl_direct_destroy(node->function_decl);
            break;

        case AST_TYPE_BLOCK:
            ast_block_direct_destroy(node->block);
            break;

        case AST_TYPE_PROGRAM:
            ast_program_direct_destroy(node->program);
            break;

        default:
            break;
    }
    memory_destroy(node);
}
