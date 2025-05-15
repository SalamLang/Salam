#ifndef _STAGE_PARSER_TYPE_H_
#define _STAGE_PARSER_TYPE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// scanner
#include <stage/scanner/type.h>

typedef struct parser_t
{
    const scanner_t* scanner; // Pointer to the scanner
    size_t index;             // Current index in the tokens array
} parser_t;

#endif // _STAGE_PARSER_TYPE_H_
