#ifndef _STAGE_SCANNER_TYPE_H_
#define _STAGE_SCANNER_TYPE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// array
#include <utility/array/type.h>
#include <utility/array/array_append/array_append.h>

// token
#include <stage/token/type.h>

typedef struct scanner_t
{
    char* source;                   // Pointer to the source code
    char* directory;                // Directory of the source code
    char* file_name;                // File name of the source code

    size_t line;                    // Current line number
    size_t column;                  // Current column number
    size_t index;                   // Current index in the source code
    size_t length;                  // Length of the source code

    array_t* tokens;                // Array of tokens
} scanner_t;





#define SCANNER_CURRENT_LOCATION \
{ \
    .begin_line = scanner->line, \
    .begin_column = scanner->column, \
    .begin_index = scanner->index, \
    .end_line = scanner->line, \
    .end_column = scanner->column, \
    .end_index = scanner->index, \
    .length = 1, \
}

#endif // _STAGE_SCANNER_TYPE_H_
