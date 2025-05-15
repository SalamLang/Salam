#ifndef _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_NODE_PARSER_PARSE_NODE_H_
#define _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_NODE_PARSER_PARSE_NODE_H_

// base
#include <base.h>

// log
#include <utility/log/log_info/log_info.h>
#include <utility/log/log_error/log_error.h>

// ast
#include <stage/ast/type.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_next/parser_next.h>
#include <stage/parser/parser_prev/parser_prev.h>
#include <stage/parser/parser_has/parser_has.h>
#include <stage/parser/parser_skip/parser_skip.h>
#include <stage/parser/parser_expect/parser_expect.h>
#include <stage/parser/parser_parse_function_decl/parser_parse_function_decl.h>

// token
#include <stage/token/token_log/token_log.h>
#include <stage/token/token_name/token_name.h>
#include <stage/token/token_log/token_log.h>

ast_t* parser_parse_node(parser_t* parser);

#endif // _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_NODE_PARSER_PARSE_NODE_H_
