#ifndef _STAGE_AST_AST_JSON_AST_JSON_H_
#define _STAGE_AST_AST_JSON_AST_JSON_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_append_char/buffer_append_char.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/type.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/ast_type/ast_type_direct_json/ast_type_direct_json.h>
#include <stage/ast/type.h>

char* ast_json(ast_t* ast);

#endif  // _STAGE_AST_AST_JSON_AST_JSON_H_
