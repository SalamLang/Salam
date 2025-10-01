#ifndef _STAGE_AST_AST_TYPE_AST_TYPE_EXACT_CREATE_PRIMITIVE_AST_TYPE_EXACT_CREATE_PRIMITIVE_H_
#define _STAGE_AST_AST_TYPE_AST_TYPE_EXACT_CREATE_PRIMITIVE_AST_TYPE_EXACT_CREATE_PRIMITIVE_H_

// base
#include <base.h>

// ast
#include <stage/ast/type.h>

ast_type_t* ast_type_exact_create_primitive(ast_type_type_t type,
                                            bool is_pointer, bool is_reference,
                                            bool is_const);

#endif  // _STAGE_AST_AST_TYPE_AST_TYPE_EXACT_CREATE_PRIMITIVE_AST_TYPE_EXACT_CREATE_PRIMITIVE_H_
