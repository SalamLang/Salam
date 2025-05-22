#include "validator_validate_type.h"

void validator_validate_type(ast_type_t* type)
{
    DEBUG_ME;
    if (!type) return;

    validator_validate_type(type->element_type);

    validator_validate_type(type->key_type);
    validator_validate_type(type->value_type);

    for (size_t i = 0; i < type->tuple_elements->size; i++) {
        ast_type_t* element = array_get(type->tuple_elements, i);
        validator_validate_type(element);
    }

    switch (type->kind) {
        case AST_KIND_TYPE_INT:
            break;

        case AST_KIND_TYPE_FLOAT:
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
            log_error("Unknown type kind: %d\n", type->kind);
            break;
    }
}
