// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// buffer
#include <utility/buffer/type.h>
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_append_str/buffer_append_str.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>

// utf8
#include <utility/utf8/utf8_is_alpha/utf8_is_alpha.h>

// token
#include <stage/token/type.h>

// scanner
#include <stage/scanner/type.h>

void scanner_scan_identifier(scanner_t *scanner, char *uc);
