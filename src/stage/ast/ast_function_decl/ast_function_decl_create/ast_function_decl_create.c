#include <stage/ast/ast_function_decl_create/ast_function_decl_create.h>

ast_t* ast_create_function_decl(const char* name, char** params, size_t param_count, ast_t* body)
{
    ast_t* node = ast_alloc(AST_TYPE_FUNCTION_DECL, ast_free);
    node->function_decl.name = string_duplicate(name);
    node->function_decl.param_count = param_count;

    if (param_count > 0) {
        node->function_decl.params = memory_calloc(param_count, sizeof(char*));
        for (size_t i = 0; i < param_count; ++i)
            node->function_decl.params[i] = string_duplicate(params[i]);
    }

    node->function_decl.body = body;
    return node;
}
