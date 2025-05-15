#ifndef _STAGE_PARSER_PARSER_SKIP_PARSER_SKIP_H_
#define _STAGE_PARSER_PARSER_SKIP_PARSER_SKIP_H_

#include <stdbool.h> // for bool, true, false

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_next/parser_next.h>
// token
#include <stage/token/type.h>

bool parser_skip(parser_t* parser, token_type_t type);

#endif // _STAGE_PARSER_PARSER_SKIP_PARSER_SKIP_H_
