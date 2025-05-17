#ifndef _STAGE_AST_ASTS_AST_EXTERN_AST_EXTERN_FUNCTION_AST_EXTERN_FUNCTION_CREATE_AST_EXTERN_FUNCTION_CREATE_H_
#define _STAGE_AST_ASTS_AST_EXTERN_AST_EXTERN_FUNCTION_AST_EXTERN_FUNCTION_CREATE_AST_EXTERN_FUNCTION_CREATE_H_

// base
#include <base.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>

ast_t* ast_extern_function_create(char* name, char* alias, ast_t* type, ast_t* attributes);

#endif // _STAGE_AST_ASTS_AST_EXTERN_AST_EXTERN_FUNCTION_AST_EXTERN_FUNCTION_CREATE_AST_EXTERN_FUNCTION_CREATE_H_
