#ifndef _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_EXPRESSIONS_PARSER_PARSE_EXPRESSIONS_H_
#define _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_EXPRESSIONS_PARSER_PARSE_EXPRESSIONS_H_

// base
#include <base.h>

// token
#include <stage/token/type.h>

// array
#include <utility/array/array_append/array_append.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/asts/ast_expression/ast_expressions/ast_expressions_create/ast_expressions_create.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_parse/parser_parse_expression_item/parser_parse_expression_item.h>

ast_t* parser_parse_expressions(parser_t* parser);

#endif // _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_EXPRESSIONS_PARSER_PARSE_EXPRESSIONS_H_
