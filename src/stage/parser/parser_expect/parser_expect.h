#ifndef _STAGE_PARSER_PARSER_EXPECT_PARSER_EXPECT_H_
#define _STAGE_PARSER_PARSER_EXPECT_PARSER_EXPECT_H_

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// parser
#include <stage/parser/parser_next/parser_next.h>
#include <stage/parser/type.h>

// token
#include <stage/token/token_name/token_name.h>
#include <stage/token/token_operator_name/token_operator_name.h>
#include <stage/token/type.h>

void parser_expect(parser_t* parser, token_type_t type);

#endif  // _STAGE_PARSER_PARSER_EXPECT_PARSER_EXPECT_H_
