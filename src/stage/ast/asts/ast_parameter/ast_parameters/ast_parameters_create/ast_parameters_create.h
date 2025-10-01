#ifndef _STAGE_AST_ASTS_AST_PARAMETER_AST_PARAMETERS_AST_PARAMETERS_CREATE_AST_PARAMETERS_CREATE_H_
#define _STAGE_AST_ASTS_AST_PARAMETER_AST_PARAMETERS_AST_PARAMETERS_CREATE_AST_PARAMETERS_CREATE_H_

#include <stddef.h>  // for size_t

// base
#include <base.h>

// array
#include <utility/array/array_create/array_create.h>
#include <utility/array/type.h>

// ast
#include <stage/ast/type.h>

ast_t* ast_parameters_create(array_t* values, size_t value_count);

#endif  // _STAGE_AST_ASTS_AST_PARAMETER_AST_PARAMETERS_AST_PARAMETERS_CREATE_AST_PARAMETERS_CREATE_H_
