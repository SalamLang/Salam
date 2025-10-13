#ifndef _SRC_STAGE_AST_AST_ALLOC_AST_ALLOC_H_
#define _SRC_STAGE_AST_AST_ALLOC_AST_ALLOC_H_

// base
#include <base.h>

// ast
#include <stage/ast/type.h>

// memory
#include <utility/memory/memory_calloc/memory_calloc.h>

ast_t* ast_alloc(ast_node_type_t type);

#endif // _SRC_STAGE_AST_AST_ALLOC_AST_ALLOC_H_
