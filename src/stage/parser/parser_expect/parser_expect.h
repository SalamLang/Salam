#ifndef _SRC_STAGE_PARSER_PARSER_EXPECT_PARSER_EXPECT_H_
#define _SRC_STAGE_PARSER_PARSER_EXPECT_PARSER_EXPECT_H_

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

void parser_expect(parser_t* parser, token_type_t type);

#endif // _SRC_STAGE_PARSER_PARSER_EXPECT_PARSER_EXPECT_H_
