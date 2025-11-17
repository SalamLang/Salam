#ifndef _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_CREATE_AST_PROGRAM_CREATE_H_
#define _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_CREATE_AST_PROGRAM_CREATE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// array
#include <utility/array/type.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/asts/ast_layout/ast_layout_create/ast_layout_create.h>

ast_t* ast_program_create(array_t* variable_declarations, array_t* function_declarations, size_t variable_declaration_count, size_t function_declaration_count);

#endif // _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_CREATE_AST_PROGRAM_CREATE_H_
