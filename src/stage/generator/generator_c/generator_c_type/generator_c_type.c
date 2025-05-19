#include "generator_c_type.h"

buffer_t* generator_c_type(generator_t* generator, ast_type_t* ast)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(12);

    if (ast == NULL) {
        buffer_append_str(temp, "void");
        return temp;
    }

    // TODO: handle the type

    return temp;
}
