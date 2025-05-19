#include "generator_create.h"

generator_t* generator_create(ast_t* ast)
{
    DEBUG_ME;
    generator_t* generator = memory_allocation(sizeof(generator_t));
    generator->ast = ast;
    generator->package = NULL;
    return generator;
}
