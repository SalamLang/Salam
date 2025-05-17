#include "ast_kind_decl_direct_destroy.h"

void ast_kind_decl_direct_destroy(ast_kind_decl_t kind_decl)
{
    DEBUG_ME;
    if (kind_decl.value != NULL) {
        ast_destroy(kind_decl.value);
    }
}
