#ifndef _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_EXPRESSION_ITEM_PARSER_PARSE_EXPRESSION_ITEM_H_
#define _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_EXPRESSION_ITEM_PARSER_PARSE_EXPRESSION_ITEM_H_

// base
#include <base.h>

// value
#include <stage/value/type.h>
#include <stage/value/value_create/value_create.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/asts/ast_expression/ast_expression_literal/ast_expression_literal_create/ast_expression_literal_create.h>
#include <stage/ast/asts/ast_expression/ast_expression_item/ast_expression_item_create_from/ast_expression_item_create_from.h>

ast_t* parser_parse_expression_item(parser_t* parser);

#endif // _SRC_STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_EXPRESSION_ITEM_PARSER_PARSE_EXPRESSION_ITEM_H_
