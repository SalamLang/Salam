#ifndef _STAGE_AST_ASTS_AST_ARGUMENT_AST_ARGUMENTS_AST_ARGUMENTS_CREATE_AST_ARGUMENTS_CREATE_H_
#define _STAGE_AST_ASTS_AST_ARGUMENT_AST_ARGUMENTS_AST_ARGUMENTS_CREATE_AST_ARGUMENTS_CREATE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// array
#include <utility/array/type.h>
#include <utility/array/array_create/array_create.h>

// ast
#include <stage/ast/type.h>

ast_t* ast_arguments_create(array_t* values, size_t value_count);

#endif // _STAGE_AST_ASTS_AST_ARGUMENT_AST_ARGUMENTS_AST_ARGUMENTS_CREATE_AST_ARGUMENTS_CREATE_H_
