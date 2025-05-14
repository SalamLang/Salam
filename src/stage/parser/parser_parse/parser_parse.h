#ifndef _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_H_
#define _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_H_

// base
#include <base.h>

// array
#include <utility/array/type.h>
#include <utility/array/array_create/array_create.h>

// parser
#include <stage/parser/type.h>

// scanner
#include <stage/scanner/type.h>

// token
#include <stage/token/type.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/asts/ast_program/ast_program_create/ast_program_create.h>

ast_t* parser_parse(parser_t* parser);

#endif
