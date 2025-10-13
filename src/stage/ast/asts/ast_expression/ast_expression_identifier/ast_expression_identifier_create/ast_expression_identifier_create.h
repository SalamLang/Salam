#ifndef _SRC_STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_IDENTIFIER_AST_EXPRESSION_IDENTIFIER_CREATE_AST_EXPRESSION_IDENTIFIER_CREATE_H_
#define _SRC_STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_IDENTIFIER_AST_EXPRESSION_IDENTIFIER_CREATE_AST_EXPRESSION_IDENTIFIER_CREATE_H_

// base
#include <base.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>

ast_t* ast_expression_identifier_create(char* name, ast_type_t* runtime_type);

#endif // _SRC_STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_IDENTIFIER_AST_EXPRESSION_IDENTIFIER_CREATE_AST_EXPRESSION_IDENTIFIER_CREATE_H_
