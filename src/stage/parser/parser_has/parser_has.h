#ifndef _STAGE_PARSER_PARSER_HAS_PARSER_HAS_H_
#define _STAGE_PARSER_PARSER_HAS_PARSER_HAS_H_

#include <stdbool.h> // for bool, true, false

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// parser
#include <stage/parser/type.h>

// token
#include <stage/token/type.h>

bool parser_has(parser_t* parser, token_type_t type);

#endif
