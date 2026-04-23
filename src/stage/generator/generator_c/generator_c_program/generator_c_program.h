#ifndef _STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_PROGRAM_GENERATOR_C_PROGRAM_H_
#define _STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_PROGRAM_GENERATOR_C_PROGRAM_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>

// ast
#include <stage/ast/type.h>

// generator
#include <stage/generator/generator_c/generator_c_node/generator_c_node.h>
#include <stage/generator/type.h>

buffer_t* generator_c_program(generator_t* generator, ast_t* ast);

#endif
