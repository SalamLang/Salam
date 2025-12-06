#ifndef _STAGE_AST_ASTS_AST_ATTRIBUTE_AST_ATTRIBUTE_ITEM_AST_ATTRIBUTE_ITEM_JSON_AST_ATTRIBUTE_ITEM_JSON_H_
#define _STAGE_AST_ASTS_AST_ATTRIBUTE_AST_ATTRIBUTE_ITEM_AST_ATTRIBUTE_ITEM_JSON_AST_ATTRIBUTE_ITEM_JSON_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// buffer
#include <utility/buffer/buffer_append_char/buffer_append_char.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/type.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>
#include <utility/string/string_escaping/string_escaping.h>

// log
#include <utility/log/log_info/log_info.h>

// ast
#include <stage/ast/ast_json/ast_json.h>
#include <stage/ast/asts/ast_extern/ast_extern_type_name/ast_extern_type_name.h>
#include <stage/ast/type.h>

char* ast_attribute_item_json(ast_attribute_item_t* attribute_item);

#endif  // _STAGE_AST_ASTS_AST_ATTRIBUTE_AST_ATTRIBUTE_ITEM_AST_ATTRIBUTE_ITEM_JSON_AST_ATTRIBUTE_ITEM_JSON_H_
