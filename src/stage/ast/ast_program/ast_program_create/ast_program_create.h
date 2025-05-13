#ifndef _AST_AST_PROGRAM_AST_PROGRAM_CREATE_AST_PROGRAM_CREATE_H_
#define _AST_AST_PROGRAM_AST_PROGRAM_CREATE_AST_PROGRAM_CREATE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>
#include <stage/ast/ast_destroy/ast_destroy.h>

ast_t* ast_program_create(ast_t** declarations, size_t declaration_count);

#endif
