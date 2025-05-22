#ifndef _STAGE_AST_AST_TYPE_AST_TYPE_EXACT_CREATE_AST_TYPE_EXACT_CREATE_H_
#define _STAGE_AST_AST_TYPE_AST_TYPE_EXACT_CREATE_AST_TYPE_EXACT_CREATE_H_

// base
#include <base.h>

// ast
#include <stage/ast/type.h>

ast_type_t* ast_type_exact_create(ast_type_type_t type, bool is_pointer, bool is_reference, bool is_const, char *name, ast_t* element_type, ast_t* key_type, ast_t* value_type, array_t* tuple_elements);

#endif // _STAGE_AST_AST_TYPE_AST_TYPE_EXACT_CREATE_AST_TYPE_EXACT_CREATE_H_
