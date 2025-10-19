#ifndef _SRC_STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_DIRECT_JSON_AST_PROGRAM_DIRECT_JSON_H_
#define _SRC_STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_DIRECT_JSON_AST_PROGRAM_DIRECT_JSON_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>

// convert
#include <utility/convert/convert_size2string/convert_size2string.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/type.h>

char* ast_program_direct_json(ast_program_t program);

#endif // _SRC_STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_DIRECT_JSON_AST_PROGRAM_DIRECT_JSON_H_
