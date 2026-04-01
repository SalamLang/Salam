#ifndef _STAGE_AST_ASTS_AST_FUNCTION_DECL_AST_FUNCTION_DECL_DESTROY_AST_FUNCTION_DECL_DESTROY_H_
#define _STAGE_AST_ASTS_AST_FUNCTION_DECL_AST_FUNCTION_DECL_DESTROY_AST_FUNCTION_DECL_DESTROY_H_

#include <stddef.h>  // for size_t

// base
#include <base.h>

// array
#include <utility/array/array_destroy/array_destroy.h>
#include <utility/array/type.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// ast
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/type.h>

void ast_function_decl_destroy(ast_function_decl_t* function_decl);

#endif  // _STAGE_AST_ASTS_AST_FUNCTION_DECL_AST_FUNCTION_DECL_DESTROY_AST_FUNCTION_DECL_DESTROY_H_
