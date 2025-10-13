#ifndef _SRC_STAGE_SCANNER_SCANNER_SET_SCANNER_SET_H_
#define _SRC_STAGE_SCANNER_SCANNER_SET_SCANNER_SET_H_

// base
#include <base.h>

// string
#include <utility/string/string_length/string_length.h>

// scanner
#include <stage/scanner/type.h>

// token
#include <stage/token/type.h>

void scanner_set(
    scanner_t* scanner,
    char* source,
    char* directory,
    char* file_name
);

#endif // _SRC_STAGE_SCANNER_SCANNER_SET_SCANNER_SET_H_
