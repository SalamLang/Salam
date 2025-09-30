#ifndef _STAGE_PARSER_PARSER_JSON_PARSER_JSON_H_
#define _STAGE_PARSER_PARSER_JSON_PARSER_JSON_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_append_char/buffer_append_char.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/type.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// parser
#include <stage/parser/type.h>

// token
#include <stage/token/token_json/token_json.h>
#include <stage/token/token_name/token_name.h>
#include <stage/token/type.h>

// value
#include <stage/value/type.h>
#include <stage/value/value_json/value_json.h>

char* parser_json(parser_t* parser);

#endif  // _STAGE_PARSER_PARSER_JSON_PARSER_JSON_H_
