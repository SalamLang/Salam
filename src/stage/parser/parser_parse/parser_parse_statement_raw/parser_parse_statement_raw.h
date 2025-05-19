#ifndef _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_STATEMENT_PARSER_PARSE_STATEMENT_RAW_PARSER_PARSE_STATEMENT_RAW_H_
#define _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_STATEMENT_PARSER_PARSE_STATEMENT_RAW_PARSER_PARSE_STATEMENT_RAW_H_

// base
#include <base.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_expect/parser_expect.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/asts/ast_statement/ast_statement_raw/ast_statement_raw_create/ast_statement_raw_create.h>

ast_t* parser_parse_statement_raw(parser_t* parser);

#endif // _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_STATEMENT_PARSER_PARSE_STATEMENT_RAW_PARSER_PARSE_STATEMENT_RAW_H_
