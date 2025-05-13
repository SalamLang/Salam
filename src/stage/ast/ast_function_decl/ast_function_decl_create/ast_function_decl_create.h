#include <stddef.h> // for size_t

// base
#include <base.h>

// memory
#include <utility/memory/memory_calloc/memory_calloc.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_alloc/ast_alloc.h>

ast_t* ast_create_function_decl(const char* name, char** params, size_t param_count, ast_t* body);
