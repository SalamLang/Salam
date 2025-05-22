#include "ast_type_exact_create_primitive.h"

ast_type_t* ast_type_exact_create_primitive(ast_type_type_t type, bool is_pointer, bool is_reference, bool is_const);
{
    DEBUG_ME;
    return ast_type_exact_create(type, is_pointer, is_reference, is_const, NULL, NULL, NULL, NULL, NULL);
}
