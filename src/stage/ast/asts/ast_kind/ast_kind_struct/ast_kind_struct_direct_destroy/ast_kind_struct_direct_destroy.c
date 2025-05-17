#include "ast_kind_struct_direct_destroy.h"

void ast_kind_struct_direct_destroy(ast_kind_struct_t kind_struct)
{
    DEBUG_ME;
    if (kind_struct.name != NULL) {
        memory_destroy(kind_struct.name);
    }

    if (kind_struct.parameters != NULL) {
        ast_destroy(kind_struct.parameters);
    }

    if (kind_struct.functions != NULL) {
        ast_destroy(kind_struct.functions);
    }
}
