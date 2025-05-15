#ifndef _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_BLOCK_PARSER_PARSE_BLOCK_H_
#define _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_BLOCK_PARSER_PARSE_BLOCK_H_

// base
#include <base.h>

// log
#include <utility/log/log_info/log_info.h>

// array
#include <utility/array/type.h>
#include <utility/array/array_append/array_append.h>
#include <utility/array/array_create/array_create.h>

// token
#include <stage/token/type.h>
#include <stage/token/token_name/token_name.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_expect_get/parser_expect_get.h>
#include <stage/parser/parser_next/parser_next.h>
#include <stage/parser/parser_prev/parser_prev.h>
#include <stage/parser/parser_has/parser_has.h>
#include <stage/parser/parser_expect/parser_expect.h>
#include <stage/parser/parser_operator_expect/parser_operator_expect.h>
#include <stage/parser/parser_eat/parser_eat.h>
#include <stage/parser/parser_skip/parser_skip.h>
#include <stage/parser/parser_parse_statement/parser_parse_statement.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/asts/ast_block/ast_block_create/ast_block_create.h>

ast_t* parser_parse_block(parser_t* parser);

#endif // _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_BLOCK_PARSER_PARSE_BLOCK_H_
