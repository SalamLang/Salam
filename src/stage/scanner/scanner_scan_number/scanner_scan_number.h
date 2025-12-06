#ifndef _STAGE_SCANNER_SCANNER_SCAN_NUMBER_SCANNER_SCAN_NUMBER_H_
#define _STAGE_SCANNER_SCANNER_SCAN_NUMBER_SCANNER_SCAN_NUMBER_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// utf8
#include <utility/utf8/utf8_char_decode/utf8_char_decode.h>
#include <utility/utf8/utf8_is_digit/utf8_is_digit.h>

// buffer
#include <utility/buffer/buffer_append_char/buffer_append_char.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/type.h>

// string
#include <utility/string/string_compare/string_compare.h>
#include <utility/string/string_convert_utf8_to_english_digit/string_convert_utf8_to_english_digit.h>
#include <utility/string/string_duplicate/string_duplicate.h>

// scanner
#include <stage/scanner/type.h>

// token
#include <stage/token/token_create/token_create.h>
#include <stage/token/type.h>

// value
#include <stage/value/type.h>
#include <stage/value/value_create/value_create.h>

void scanner_scan_number(scanner_t* scanner, char* uc);

#endif  // _STAGE_SCANNER_SCANNER_SCAN_NUMBER_SCANNER_SCAN_NUMBER_H_
