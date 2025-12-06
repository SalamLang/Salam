#include "ast_kind_enum_direct_destroy.h"

void ast_kind_enum_direct_destroy(ast_kind_enum_t kind_enum) {
    DEBUG_ME;
    if (kind_enum.name != NULL) {
        memory_destroy(kind_enum.name);
    }

    if (kind_enum.alias != NULL) {
        memory_destroy(kind_enum.alias);
    }

    if (kind_enum.values != NULL) {
        hashmap_destroy(kind_enum.values);
    }
}
