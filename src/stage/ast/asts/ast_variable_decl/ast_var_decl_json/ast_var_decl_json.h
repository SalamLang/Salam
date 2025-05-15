#ifndef _STAGE_AST_ASTS_AST_VARIABLE_DECL_AST_VAR_DECL_JSON_AST_VAR_DECL_JSON_H_
#define _STAGE_AST_ASTS_AST_VARIABLE_DECL_AST_VAR_DECL_JSON_AST_VAR_DECL_JSON_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/type.h>

char* ast_variable_decl_json(ast_variable_decl_t* variable_decl);

#endif // _STAGE_AST_ASTS_AST_VARIABLE_DECL_AST_VAR_DECL_JSON_AST_VAR_DECL_JSON_H_
