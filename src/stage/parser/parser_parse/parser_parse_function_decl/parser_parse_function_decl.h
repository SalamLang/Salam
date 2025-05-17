#ifndef _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_FUNCTION_DECL_PARSER_PARSE_FUNCTION_DECL_H_
#define _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_FUNCTION_DECL_PARSER_PARSE_FUNCTION_DECL_H_

// base
#include <base.h>

// log
#include <utility/log/log_info/log_info.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/asts/ast_function_decl/ast_function_decl_create/ast_function_decl_create.h>

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
#include <stage/parser/parser_eat/parser_eat.h>
#include <stage/parser/parser_skip/parser_skip.h>
#include <stage/parser/parser_parse/parser_parse_block/parser_parse_block.h>
#include <stage/parser/parser_parse/parser_parse_parameters/parser_parse_parameters.h>

ast_t* parser_parse_function_decl(parser_t* parser);

#endif // _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_FUNCTION_DECL_PARSER_PARSE_FUNCTION_DECL_H_
