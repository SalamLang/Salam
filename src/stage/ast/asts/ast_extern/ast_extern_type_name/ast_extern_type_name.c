#include <stage/ast/asts/ast_extern/ast_extern_type_name/ast_extern_type_name.h>

char* ast_extern_type_name(ast_extern_decl_type_t type)
{
    switch (type)
    {
        case AST_EXTERN_TYPE_FUNCTION: return "EXTERN_FUNCTION";
        case AST_EXTERN_TYPE_VARIABLE: return "EXTERN_VARIABLE";
    }
    return "EXTERN_TYPE_UNKNOWN";
}
