#ifndef _STAGE_PARSER_PARSER_OPERATOR_EXPECT_PARSER_OPERATOR_EXPECT_H_
#define _STAGE_PARSER_PARSER_OPERATOR_EXPECT_PARSER_OPERATOR_EXPECT_H_

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_next/parser_next.h>

// token
#include <stage/token/type.h>
#include <stage/token/token_name/token_name.h>
#include <stage/token/token_operator_name/token_operator_name.h>

void parser_operator_expect(parser_t* parser, token_operator_type_t operator_type);

#endif // _STAGE_PARSER_PARSER_OPERATOR_EXPECT_PARSER_OPERATOR_EXPECT_H_
