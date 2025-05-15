#ifndef _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_DESTROY_AST_PROGRAM_DESTROY_H_
#define _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_DESTROY_AST_PROGRAM_DESTROY_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_program_destroy(ast_program_t* program);

#endif // _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_DESTROY_AST_PROGRAM_DESTROY_H_
