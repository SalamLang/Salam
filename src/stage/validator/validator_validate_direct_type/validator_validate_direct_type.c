#include "validator_validate_direct_type.h"

void validator_validate_direct_type(ast_type_t type)
{
    DEBUG_ME;
    if (type.element_type != NULL) {
        validator_validate(type.element_type);
    }

    if (type.key_type != NULL) {
        validator_validate(type.key_type);
    }

    if (type.value_type != NULL) {
        validator_validate(type.value_type);
    }

    for (size_t i = 0; i < type.tuple_elements->size; i++) {
        ast_t* element = array_get(type.tuple_elements, i);
        if (element != NULL) {
            validator_validate(element);
        }
    }

    switch (type.type) {
        case AST_KIND_TYPE_INT:
            break;

        case AST_KIND_TYPE_FLOAT:
            break;

        case AST_KIND_TYPE_SIZE:
            break;

        case AST_KIND_TYPE_STRING:
            break;

        case AST_KIND_TYPE_BOOL:
            break;

        case AST_KIND_TYPE_ARRAY:
            break;

        case AST_KIND_TYPE_MAP:
            break;

        case AST_KIND_TYPE_SET:
            break;

        case AST_KIND_TYPE_TUPLE:
            break;

        case AST_KIND_TYPE_VOID:
            break;

        case AST_KIND_TYPE_ANY:
            break;

        case AST_KIND_TYPE_USER_DEFINED:
            break;

        default:
            log_error("Unknown type type: %d\n", type.type);
            break;
    }
}
