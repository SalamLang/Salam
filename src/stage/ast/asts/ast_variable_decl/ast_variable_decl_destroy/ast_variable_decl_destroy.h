#ifndef _STAGE_AST_ASTS_AST_VARIABLE_DECL_AST_VAR_DECL_DESTROY_AST_VAR_DECL_DESTROY_H_
#define _STAGE_AST_ASTS_AST_VARIABLE_DECL_AST_VAR_DECL_DESTROY_AST_VAR_DECL_DESTROY_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// ast
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/type.h>

void ast_variable_decl_destroy(ast_variable_decl_t* variable_decl);

#endif  // _STAGE_AST_ASTS_AST_VARIABLE_DECL_AST_VAR_DECL_DESTROY_AST_VAR_DECL_DESTROY_H_
