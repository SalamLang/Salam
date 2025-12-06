#ifndef _STAGE_SCANNER_SCANNER_JSON_SCANNER_JSON_H_
#define _STAGE_SCANNER_SCANNER_JSON_SCANNER_JSON_H_

// base
#include <base.h>

// buffer
#include <utility/buffer/buffer_create/buffer_create.h>
#include <utility/buffer/buffer_destroy/buffer_destroy.h>
#include <utility/buffer/type.h>

// array
#include <utility/array/array_get/array_get.h>
#include <utility/array/array_size/array_size.h>
#include <utility/array/type.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// string
#include <utility/string/string_duplicate/string_duplicate.h>
#include <utility/string/string_escaping/string_escaping.h>

// scanner
#include <stage/scanner/type.h>

// token
#include <stage/token/token_json/token_json.h>

char* scanner_json(scanner_t* scanner);

#endif  // _STAGE_SCANNER_SCANNER_JSON_SCANNER_JSON_H_
