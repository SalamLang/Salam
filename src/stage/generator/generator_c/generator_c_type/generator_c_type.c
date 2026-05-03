#include "generator_c_type.h"

buffer_t* generator_c_type(generator_t* generator, ast_t* ast)
{
    (void)generator; // Suppress unused parameter warning
    DEBUG_ME;
    buffer_t* temp = buffer_create(12);

    if (ast == NULL) {
        buffer_append_str(temp, "void");
        return temp;
    }

    ast_type_t type = ast->raw.type_value;

    switch (type.type)
    {
        case AST_KIND_TYPE_INT:
            buffer_append_str(temp, "int");
            break;

        case AST_KIND_TYPE_FLOAT:
            buffer_append_str(temp, "float");
            break;

        case AST_KIND_TYPE_SIZE:
            buffer_append_str(temp, "size_t");
            break;

        case AST_KIND_TYPE_STRING:
            buffer_append_str(temp, "char*");
            break;

        case AST_KIND_TYPE_BOOL:
            buffer_append_str(temp, "bool");
            break;

        case AST_KIND_TYPE_ARRAY:
            buffer_append_str(temp, "array");
            break;

        case AST_KIND_TYPE_MAP:
            buffer_append_str(temp, "map");
            break;

        case AST_KIND_TYPE_SET:
            buffer_append_str(temp, "set");
            break;

        case AST_KIND_TYPE_TUPLE:
            buffer_append_str(temp, "tuple");
            break;

        case AST_KIND_TYPE_VOID:
            buffer_append_str(temp, "void");
            break;

        case AST_KIND_TYPE_ANY:
            buffer_append_str(temp, "any");
            break;

        default:
            log_fatal("Unknown type in generator_c_type: %d\n", type.type);
            break;
    }

    return temp;
}
