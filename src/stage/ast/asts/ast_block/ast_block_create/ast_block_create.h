#ifndef _SRC_STAGE_AST_ASTS_AST_BLOCK_AST_BLOCK_CREATE_AST_BLOCK_CREATE_H_
#define _SRC_STAGE_AST_ASTS_AST_BLOCK_AST_BLOCK_CREATE_AST_BLOCK_CREATE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// array
#include <utility/array/type.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/asts/ast_block/ast_block_destroy/ast_block_destroy.h>
#include <stage/ast/asts/ast_block/ast_block_direct_destroy/ast_block_direct_destroy.h>

ast_t* ast_block_create(array_t* statements, size_t statement_count);

#endif // _SRC_STAGE_AST_ASTS_AST_BLOCK_AST_BLOCK_CREATE_AST_BLOCK_CREATE_H_
