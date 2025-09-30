#ifndef _STAGE_SCANNER_SCANNER_SCAN_STRING_SCANNER_SCAN_STRING_H_
#define _STAGE_SCANNER_SCANNER_SCAN_STRING_SCANNER_SCAN_STRING_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>

// string
#include <utility/string/string_compare/string_compare.h>
#include <utility/string/string_duplicate/string_duplicate.h>

// utf8
#include <utility/utf8/utf8_char_decode/utf8_char_decode.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// token
#include <stage/token/token_create/token_create.h>
#include <stage/token/type.h>

// scanner
#include <stage/scanner/scanner_error/scanner_error.h>
#include <stage/scanner/type.h>

// value
#include <stage/value/type.h>
#include <stage/value/value_create/value_create.h>

void scanner_scan_string(scanner_t* lexer, int type);

#endif  // _STAGE_SCANNER_SCANNER_SCAN_STRING_SCANNER_SCAN_STRING_H_
