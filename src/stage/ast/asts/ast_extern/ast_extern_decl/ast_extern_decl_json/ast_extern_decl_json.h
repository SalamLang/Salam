#ifndef _STAGE_AST_ASTS_AST_EXTERN_AST_EXTERN_DECL_AST_EXTERN_DECL_JSON_AST_EXTERN_DECL_JSON_H_
#define _STAGE_AST_ASTS_AST_EXTERN_AST_EXTERN_DECL_AST_EXTERN_DECL_JSON_AST_EXTERN_DECL_JSON_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_append_char/buffer_append_char.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// log
#include <utility/log/log_info/log_info.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_json/ast_json.h>
#include <stage/ast/asts/ast_extern/ast_extern_type_name/ast_extern_type_name.h>

char* ast_extern_decl_json(ast_extern_decl_t* extern_decl);

#endif
