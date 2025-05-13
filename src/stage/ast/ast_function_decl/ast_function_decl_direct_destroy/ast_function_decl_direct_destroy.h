#ifndef _STAGE_AST_AST_FUNCTION_DECL_AST_FUNCTION_DECL_DIRECT_DESTROY_AST_FUNCTION_DECL_DIRECT_DESTROY_H_
#define _STAGE_AST_AST_FUNCTION_DECL_AST_FUNCTION_DECL_DIRECT_DESTROY_AST_FUNCTION_DECL_DIRECT_DESTROY_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_function_decl_direct_destroy(ast_function_decl_t function_decl);

#endif
