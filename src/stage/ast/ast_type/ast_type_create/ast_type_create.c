#include "ast_type_create.h"

ast_t* ast_type_create(ast_type_type_t type, bool is_pointer, bool is_reference, bool is_const, char *name, ast_t* element_type, ast_t* key_type, ast_t* value_type, array_t* tuple_elements)
{
    DEBUG_ME;
    ast_t* ast = memory_allocation(sizeof(ast_t));
    ast->base.type = AST_TYPE_TYPE;
    ast->raw.type_value.type = type;
    ast->raw.type_value.is_pointer = is_pointer;
    ast->raw.type_value.is_reference = is_reference;
    ast->raw.type_value.is_const = is_const;
    ast->raw.type_value.name = name;

    ast->raw.type_value.element_type = element_type;
    ast->raw.type_value.key_type = key_type;
    ast->raw.type_value.value_type = value_type;
    ast->raw.type_value.tuple_elements = tuple_elements;

    return ast;
}
