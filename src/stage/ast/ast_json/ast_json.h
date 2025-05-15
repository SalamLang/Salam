#ifndef _STAGE_AST_AST_JSON_AST_JSON_H_
#define _STAGE_AST_AST_JSON_AST_JSON_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_append_char/buffer_append_char.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/type.h>

char* ast_json(ast_t* ast);

#endif // _STAGE_AST_AST_JSON_AST_JSON_H_
