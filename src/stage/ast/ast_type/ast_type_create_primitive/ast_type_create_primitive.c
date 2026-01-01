#include "ast_type_create_primitive.h"

ast_t* ast_type_create_primitive(ast_type_type_t type, bool is_pointer,
                                 bool is_reference, bool is_const) {
    DEBUG_ME;
    return ast_type_create(type, is_pointer, is_reference, is_const, NULL, NULL,
                           NULL, NULL, NULL);
}
