#ifndef _STAGE_SCANNER_SCANNER_SCAN_SCANNER_SCAN_H_
#define _STAGE_SCANNER_SCANNER_SCAN_SCANNER_SCAN_H_

// base
#include <base.h>

// array
#include <utility/array/array_append/array_append.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// string
#include <utility/string/string_compare/string_compare.h>
#include <utility/string/string_char_is_alpha/string_char_is_alpha.h>

// utf8
#include <utility/utf8/utf8_char_decode/utf8_char_decode.h>

// token
#include <stage/token/type.h>
#include <stage/token/token_create/token_create.h>
#include <stage/token/token_char_type/token_char_type.h>

// scanner
#include <stage/scanner/type.h>
#include <stage/scanner/scanner_error/scanner_error.h>
#include <stage/scanner/scanner_scan_string/scanner_scan_string.h>
#include <stage/scanner/scanner_scan_number/scanner_scan_number.h>
#include <stage/scanner/scanner_scan_identifier/scanner_scan_identifier.h>
#include <stage/scanner/scanner_scan_comment_inline/scanner_scan_comment_inline.h>
#include <stage/scanner/scanner_scan_comment_multiline/scanner_scan_comment_multiline.h>

void scanner_scan(scanner_t* scanner);

#endif // _STAGE_SCANNER_SCANNER_SCAN_SCANNER_SCAN_H_
