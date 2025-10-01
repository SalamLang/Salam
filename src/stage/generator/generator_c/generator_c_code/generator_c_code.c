#include "generator_c_code.h"

char* generator_c_code(generator_t* generator) {
    DEBUG_ME;

    // TODO: handling import, package, etc...
    buffer_t* program = generator_c_program(generator, generator->ast);

    char* result = string_duplicate(program->data);
    buffer_destroy(program);
    return result;
}
