#ifndef _STAGE_PARSER_PARSER_EAT_PARSER_EAT_H_
#define _STAGE_PARSER_PARSER_EAT_PARSER_EAT_H_

// base
#include <base.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// parser
#include <stage/parser/type.h>
#include <stage/parser/parser_next/parser_next.h>

// token
#include <stage/token/type.h>

token_t* parser_eat(parser_t* parser);

#endif // _STAGE_PARSER_PARSER_EAT_PARSER_EAT_H_
