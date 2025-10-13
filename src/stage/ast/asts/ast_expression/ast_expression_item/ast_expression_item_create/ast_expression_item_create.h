#ifndef _SRC_STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_ITEM_AST_EXPRESSION_ITEM_CREATE_AST_EXPRESSION_ITEM_CREATE_H_
#define _SRC_STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_ITEM_AST_EXPRESSION_ITEM_CREATE_AST_EXPRESSION_ITEM_CREATE_H_

// base
#include <base.h>

// parser
#include <stage/parser/type.h>

// ast
#include <stage/ast/type.h>

// value
#include <stage/value/type.h>

ast_t* ast_expression_item_create(ast_expression_type_t type, ast_type_t* runtime_type);

#endif // _SRC_STAGE_AST_ASTS_AST_EXPRESSION_AST_EXPRESSION_ITEM_AST_EXPRESSION_ITEM_CREATE_AST_EXPRESSION_ITEM_CREATE_H_
