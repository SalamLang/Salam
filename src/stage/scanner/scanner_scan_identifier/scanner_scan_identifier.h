#ifndef _SRC_STAGE_SCANNER_SCANNER_SCAN_IDENTIFIER_SCANNER_SCAN_IDENTIFIER_H_
#define _SRC_STAGE_SCANNER_SCANNER_SCAN_IDENTIFIER_SCANNER_SCAN_IDENTIFIER_H_

#include <ctype.h> // for isspace

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>

// utf8
#include <utility/utf8/utf8_is_alpha/utf8_is_alpha.h>
#include <utility/utf8/utf8_char_decode/utf8_char_decode.h>

// token
#include <stage/token/type.h>
#include <stage/token/token_create/token_create.h>
#include <stage/token/token_keyword_type/token_keyword_type.h>
#include <stage/token/token_operator_keyword_type/token_operator_keyword_type.h>

// scanner
#include <stage/scanner/type.h>

void scanner_scan_identifier(scanner_t *scanner, char *uc);

#endif // _SRC_STAGE_SCANNER_SCANNER_SCAN_IDENTIFIER_SCANNER_SCAN_IDENTIFIER_H_
