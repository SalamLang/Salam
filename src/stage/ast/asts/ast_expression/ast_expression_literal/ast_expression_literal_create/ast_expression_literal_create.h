#ifndef _STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_LITERAL_AST_EXPRESSION_LITERAL_CREATE_AST_EXPRESSION_LITERAL_CREATE_H_
#define _STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_LITERAL_AST_EXPRESSION_LITERAL_CREATE_AST_EXPRESSION_LITERAL_CREATE_H_

// base
#include <base.h>

// parser
#include <stage/parser/type.h>

// ast
#include <stage/ast/type.h>

// value
#include <stage/value/type.h>

ast_t* ast_expression_literal_create(value_t* value, ast_type_t* runtime_type);

#endif  // _STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_LITERAL_AST_EXPRESSION_LITERAL_CREATE_AST_EXPRESSION_LITERAL_CREATE_H_
