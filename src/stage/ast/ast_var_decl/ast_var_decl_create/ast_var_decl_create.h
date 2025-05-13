// base
#include <base.h>

// memory
#include <utility/memory/memory_calloc/memory_calloc.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>
#include <stage/ast/ast_destroy/ast_destroy.h>

ast_t* ast_create_var_decl(const char* name, ast_t* value);
