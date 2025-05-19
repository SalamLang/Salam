// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_append/buffer_append.h>

// generator
#include <stage/generator/type.h>

// ast
#include <stage/ast/type.h>

buffer_t* generator_c_variable_decl(generator_t* generator, ast_t* ast);
