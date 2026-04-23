#ifndef _STAGE_TOKEN_TOKEN_JSON_TOKEN_JSON_H_
#define _STAGE_TOKEN_TOKEN_JSON_TOKEN_JSON_H_

// base
#include <base.h>

// log
#include <utility/log/log_info/log_info.h>

// buffer
#include <utility/buffer/buffer_append_char/buffer_append_char.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/type.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// token
#include <stage/token/token_location_json/token_location_json.h>
#include <stage/token/token_name/token_name.h>
#include <stage/token/type.h>

// value
#include <stage/value/type.h>
#include <stage/value/value_json/value_json.h>
#include <stage/value/value_log/value_log.h>

char* token_json(const token_t* token);

#endif  // _STAGE_TOKEN_TOKEN_JSON_TOKEN_JSON_H_
