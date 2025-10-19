#ifndef _SRC_STAGE_TOKEN_TOKEN_LOCATION_JSON_TOKEN_LOCATION_JSON_H_
#define _SRC_STAGE_TOKEN_TOKEN_LOCATION_JSON_TOKEN_LOCATION_JSON_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// convert
#include <utility/convert/convert_size2string/convert_size2string.h>

// token
#include <stage/token/type.h>

char* token_location_json(token_location_t location);

#endif // _SRC_STAGE_TOKEN_TOKEN_LOCATION_JSON_TOKEN_LOCATION_JSON_H_
