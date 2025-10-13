#ifndef _SRC_STAGE_PARSER_PARSER_EXPECT_GET_PARSER_EXPECT_GET_H_
#define _SRC_STAGE_PARSER_PARSER_EXPECT_GET_PARSER_EXPECT_GET_H_

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_expect/parser_expect.h>

// token
#include <stage/token/type.h>

token_t* parser_expect_get(parser_t* parser, token_type_t type);

#endif // _SRC_STAGE_PARSER_PARSER_EXPECT_GET_PARSER_EXPECT_GET_H_
