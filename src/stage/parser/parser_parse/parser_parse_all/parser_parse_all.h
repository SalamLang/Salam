#ifndef _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_ALL_PARSER_PARSE_ALL_H_
#define _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_ALL_PARSER_PARSE_ALL_H_

// base
#include <base.h>

// array
#include <utility/array/type.h>
#include <utility/array/array_create/array_create.h>

// log
#include <utility/log/log_info/log_info.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_parse/parser_parse_node/parser_parse_node.h>

// scanner
#include <stage/scanner/type.h>

// token
#include <stage/token/type.h>
#include <stage/token/token_name/token_name.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/asts/ast_program/ast_program_create/ast_program_create.h>

ast_t* parser_parse_all(parser_t* parser);

#endif // _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_ALL_PARSER_PARSE_ALL_H_
