#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_destroy(ast_t* ast)
{
    // if (!ast) return;
    ast->base.destroyer(ast);

    switch (ast->base.type) {
        case AST_TYPE_VAR_DECL:
            ast_var_decl_direct_destroy(ast->var_decl);
            break;

        case AST_TYPE_FUNCTION_DECL:
            ast_function_decl_direct_destroy(ast->function_decl);
            break;

        case AST_TYPE_BLOCK:
            ast_block_direct_destroy(ast->block);
            break;

        case AST_TYPE_PROGRAM:
            ast_program_direct_destroy(ast->program);
            break;

        default:
            break;
    }
    memory_destroy(ast);
}
