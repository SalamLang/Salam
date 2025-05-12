// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>

// string
#include <utility/string/string_compare/string_compare.h>

// utf8
#include <utility/utf8/utf8_char_decode/utf8_char_decode.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// token
#include <stage/token/type.h>
#include <stage/token/token_create/token_create.h>

// scanner
#include <stage/scanner/type.h>
#include <stage/scanner/scanner_error/scanner_error.h>

void scanner_scan_string(scanner_t *lexer, int type);
