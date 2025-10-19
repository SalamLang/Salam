#ifndef _SRC_STAGE_AST_ASTS_AST_FUNCTION_DECL_AST_FUNCTION_DECL_CREATE_AST_FUNCTION_DECL_CREATE_H_
#define _SRC_STAGE_AST_ASTS_AST_FUNCTION_DECL_AST_FUNCTION_DECL_CREATE_AST_FUNCTION_DECL_CREATE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// memory
#include <utility/memory/memory_calloc/memory_calloc.h>

// array
#include <utility/array/type.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/asts/ast_function_decl/ast_function_decl_destroy/ast_function_decl_destroy.h>
#include <stage/ast/asts/ast_function_decl/ast_function_decl_direct_destroy/ast_function_decl_direct_destroy.h>

ast_t* ast_function_decl_create(const char* name, ast_t* return_type, ast_t* parameters, ast_t* block);

#endif // _SRC_STAGE_AST_ASTS_AST_FUNCTION_DECL_AST_FUNCTION_DECL_CREATE_AST_FUNCTION_DECL_CREATE_H_
