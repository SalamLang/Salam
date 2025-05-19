#include "generator_c_ident.h"

#define GENERATOR_C_IDENT_SIZE 2

buffer_t* generator_c_ident(generator_t* generator)
{
    DEBUG_ME;
    if (generator->ident == 0) return NULL;

    buffer_t* temp = buffer_create(3);

    for (size_t i = 0; i < generator->ident; i++) {
        for (size_t j = 0; j < GENERATOR_C_IDENT_SIZE; j++) {
            buffer_append_char(temp, ' ');
        }
    }

    return temp;
}
