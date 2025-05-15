#ifndef _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_JSON_AST_PROGRAM_JSON_H_
#define _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_JSON_AST_PROGRAM_JSON_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// ast
#include <stage/ast/type.h>

char* ast_program_json(ast_program_t* block);

#endif // _STAGE_AST_ASTS_AST_PROGRAM_AST_PROGRAM_JSON_AST_PROGRAM_JSON_H_
