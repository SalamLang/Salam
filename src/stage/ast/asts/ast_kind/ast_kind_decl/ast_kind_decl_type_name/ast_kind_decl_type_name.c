#include "ast_kind_decl_type_name.h"

char* ast_kind_decl_type_name(ast_kind_type_t kind_decl_type)
{
    switch (kind_decl_type) {
        case AST_KIND_TYPE_STRUCT: return "STRUCT";
            break;
        case AST_KIND_TYPE_ENUM: return "ENUM";
            break;
        case AST_KIND_TYPE_UNION: return "UNION";
            break;
        default:
            return "KIND_DECL_UNKNOWN";
            break;
    }
}

