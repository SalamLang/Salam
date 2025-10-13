#ifndef _SRC_STAGE_PARSER_PARSER_OPERATOR_SKIP_PARSER_OPERATOR_SKIP_H_
#define _SRC_STAGE_PARSER_PARSER_OPERATOR_SKIP_PARSER_OPERATOR_SKIP_H_

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

bool parser_operator_skip(parser_t* parser, token_operator_type_t operator_type);

#endif // _SRC_STAGE_PARSER_PARSER_OPERATOR_SKIP_PARSER_OPERATOR_SKIP_H_
