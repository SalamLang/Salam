// base
#include <base.h>

// ast
#include <stage/ast/type.h>

// memory
#include <utility/memory/memory_calloc/memory_calloc.h>

ast_t* ast_alloc(ast_node_type_t type, ast_destroy_t free_func);
