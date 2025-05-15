#ifndef _STAGE_AST_ASTS_AST_BLOCK_AST_BLOCK_DIRECT_DESTROY_AST_BLOCK_DIRECT_DESTROY_H_
#define _STAGE_AST_ASTS_AST_BLOCK_AST_BLOCK_DIRECT_DESTROY_AST_BLOCK_DIRECT_DESTROY_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// array
#include <utility/array/type.h>
#include <utility/array/array_destroy/array_destroy.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_destroy/ast_destroy.h>

void ast_block_direct_destroy(ast_block_t block);

#endif // _STAGE_AST_ASTS_AST_BLOCK_AST_BLOCK_DIRECT_DESTROY_AST_BLOCK_DIRECT_DESTROY_H_
