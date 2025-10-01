#ifndef _STAGE_AST_ASTS_AST_EXTERN_AST_EXTERN_DECL_AST_EXTERN_DECL_CREATE_AST_EXTERN_DECL_CREATE_H_
#define _STAGE_AST_ASTS_AST_EXTERN_AST_EXTERN_DECL_AST_EXTERN_DECL_CREATE_AST_EXTERN_DECL_CREATE_H_

// base
#include <base.h>

// ast
#include <stage/ast/ast_alloc/ast_alloc.h>
#include <stage/ast/type.h>

ast_t* ast_extern_decl_create(ast_extern_decl_type_t type, ast_t* value);

#endif  // _STAGE_AST_ASTS_AST_EXTERN_AST_EXTERN_DECL_AST_EXTERN_DECL_CREATE_AST_EXTERN_DECL_CREATE_H_
