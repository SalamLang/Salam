#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_destroy(ast_t* ast)
{
    // if (!ast) return;

    switch (ast->base.type) {
        case AST_TYPE_VAR_DECL:
            ast_var_decl_direct_destroy(ast->raw.var_decl_value);
            break;
        case AST_TYPE_FUNCTION_DECL:
            ast_function_decl_direct_destroy(ast->raw.function_decl_value);
            break;
        case AST_TYPE_BLOCK:
            ast_block_direct_destroy(ast->raw.block_value);
            break;
        case AST_TYPE_PROGRAM:
            ast_program_direct_destroy(ast->raw.program_value);
            break;
        case AST_TYPE_UNKNOWN:
            break;
    }
    memory_destroy(ast);
}
