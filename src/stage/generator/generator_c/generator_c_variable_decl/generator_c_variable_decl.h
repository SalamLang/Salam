#ifndef _STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_VARIABLE_DECL_GENERATOR_C_VARIABLE_DECL_H_
#define _STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_VARIABLE_DECL_GENERATOR_C_VARIABLE_DECL_H_

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

#endif // _STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_VARIABLE_DECL_GENERATOR_C_VARIABLE_DECL_H_
