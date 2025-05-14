#ifndef _STAGE_AST_AST_DESTROY_AST_DESTROY_H_
#define _STAGE_AST_AST_DESTROY_AST_DESTROY_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/asts/ast_var_decl/ast_var_decl_direct_destroy/ast_var_decl_direct_destroy.h>
#include <stage/ast/asts/ast_function_decl/ast_function_decl_direct_destroy/ast_function_decl_direct_destroy.h>
#include <stage/ast/asts/ast_block/ast_block_direct_destroy/ast_block_direct_destroy.h>
#include <stage/ast/asts/ast_program/ast_program_direct_destroy/ast_program_direct_destroy.h>
// #include <stage/ast/asts/ast_var_decl/ast_var_decl_destroy/ast_var_decl_destroy.h>
// #include <stage/ast/asts/ast_function_decl/ast_function_decl_destroy/ast_function_decl_destroy.h>
// #include <stage/ast/asts/ast_block/ast_block_destroy/ast_block_destroy.h>
// #include <stage/ast/asts/ast_program/ast_program_destroy/ast_program_destroy.h>

void ast_destroy(ast_t* ast);

#endif
