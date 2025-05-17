#include "ast_type_direct_destroy.h"

void ast_type_direct_destroy(ast_type_t type)
{
    DEBUG_ME;
    if (type.name != NULL) {
        memory_destroy(type.name);
    }

    if (type.element_type != NULL) {
        ast_type_destroy(type.element_type);
    }

    if (type.key_type != NULL) {
        ast_type_destroy(type.key_type);
    }

    if (type.value_type != NULL) {
        ast_type_destroy(type.value_type);
    }

    if (type.tuple_elements != NULL) {
        array_destroy(type.tuple_elements);
    }
}
