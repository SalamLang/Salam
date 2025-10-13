#ifndef _SRC_STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_NODE_GENERATOR_C_NODE_H_
#define _SRC_STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_NODE_GENERATOR_C_NODE_H_

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// generator
#include <stage/generator/type.h>

// ast
#include <stage/ast/type.h>

buffer_t* generator_c_node(generator_t* generator, ast_t* ast);

#endif // _SRC_STAGE_GENERATOR_GENERATOR_C_GENERATOR_C_NODE_GENERATOR_C_NODE_H_
