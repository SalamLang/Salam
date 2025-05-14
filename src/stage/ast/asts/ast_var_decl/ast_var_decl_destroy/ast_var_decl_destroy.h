#ifndef _STAGE_AST_ASTS_AST_VAR_DECL_AST_VAR_DECL_DESTROY_AST_VAR_DECL_DESTROY_H_
#define _STAGE_AST_ASTS_AST_VAR_DECL_AST_VAR_DECL_DESTROY_AST_VAR_DECL_DESTROY_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_var_decl_destroy(ast_var_decl_t* var_decl);

#endif
