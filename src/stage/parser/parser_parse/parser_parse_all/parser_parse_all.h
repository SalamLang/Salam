#ifndef _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_ALL_PARSER_PARSE_ALL_H_
#define _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_ALL_PARSER_PARSE_ALL_H_

// base
#include <base.h>

// array
#include <utility/array/array_create/array_create.h>
#include <utility/array/type.h>

// log
#include <utility/log/log_info/log_info.h>

// parser
#include <stage/parser/parser_parse/parser_parse_node/parser_parse_node.h>
#include <stage/parser/type.h>

// scanner
#include <stage/scanner/type.h>

// token
#include <stage/token/token_name/token_name.h>
#include <stage/token/type.h>

// ast
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/asts/ast_program/ast_program_create/ast_program_create.h>
#include <stage/ast/type.h>

ast_t* parser_parse_all(parser_t* parser);

#endif  // _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_ALL_PARSER_PARSE_ALL_H_
