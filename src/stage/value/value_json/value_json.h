// base
#include <base.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_append_char/buffer_append_char.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// convert
#include <utility/convert/convert_size2string/convert_size2string.h>

// value
#include <stage/value/type.h>
#include <stage/value/value_name/value_name.h>

char* value_json(const value_t* value);
