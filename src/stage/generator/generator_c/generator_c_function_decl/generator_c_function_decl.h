// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_append/buffer_append.h>

// generator
#include <stage/generator/type.h>
#include <stage/generator/generator_c/generator_c_type/generator_c_type.h>

// ast
#include <stage/ast/type.h>

buffer_t* generator_c_function_decl(generator_t* generator, ast_t* ast);
