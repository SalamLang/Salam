#include "ast_type_exact_create.h"

ast_type_t* ast_type_exact_create(ast_type_type_t type, bool is_pointer, bool is_reference, bool is_const, char *name, ast_t* element_type, ast_t* key_type, ast_t* value_type, array_t* tuple_elements)
{
    DEBUG_ME;
    ast_type_t* ast_type = memory_allocation(sizeof(ast_type_t));
    ast_type.type = type;
    ast_type.is_pointer = is_pointer;
    ast_type.is_reference = is_reference;
    ast_type.is_const = is_const;
    ast_type.name = name;

    ast_type.element_type = element_type;
    ast_type.key_type = key_type;
    ast_type.value_type = value_type;
    ast_type.tuple_elements = tuple_elements;

    return ast_type;
}
