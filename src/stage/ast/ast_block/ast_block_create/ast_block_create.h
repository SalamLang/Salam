#include <stddef.h> // for size_t

// base
#include <base.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>
#include <stage/ast/ast_destroy/ast_destroy.h>

ast_t* ast_create_block(ast_t** statements, size_t statement_count);
