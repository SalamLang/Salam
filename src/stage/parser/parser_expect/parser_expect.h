#ifndef _STAGE_PARSER_PARSER_EXPECT_PARSER_EXPECT_H_
#define _STAGE_PARSER_PARSER_EXPECT_PARSER_EXPECT_H_

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_next/parser_next.h>

// token
#include <stage/token/type.h>

void parser_expect(parser_t* parser, token_type_t type);

#endif
