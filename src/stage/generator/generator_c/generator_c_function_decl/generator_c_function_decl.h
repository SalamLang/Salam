#ifndef _STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_FUNCTION_DECL_GENERATOR_C_FUNCTION_DECL_H_
#define _STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_FUNCTION_DECL_GENERATOR_C_FUNCTION_DECL_H_

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

#endif // _STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_FUNCTION_DECL_GENERATOR_C_FUNCTION_DECL_H_
