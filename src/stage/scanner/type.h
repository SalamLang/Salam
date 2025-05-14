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

#define SCANNER_CURRENT             (scanner->source[scanner->index])
#define SCANNER_CURRENT_NEXT        (scanner->source[scanner->index + 1])
#define SCANNER_CURRENT_PREV        (scanner->source[scanner->index - 1])

#define SCANNER_HAS_NEXT            (scanner->index + 1 < scanner->length && SCANNER_CURRENT_NEXT != '\0')
#define SCANNER_HAS_PREV            (scanner->index > 0)

#define SCANNER_NEXT scanner->index++
#define SCANNER_PREV scanner->index--

#define SCANNER_NEXT_LINE scanner->line++
#define SCANNER_PREV_LINE scanner->line--

#define SCANNER_NEXT_COLUMN scanner->column++
#define SCANNER_PREV_COLUMN scanner->column--

#define SCANNER_ZERO_COLUMN scanner->column = 0

#define SCANNER_CURRENT_LOCATION (token_location_t) { \
    .begin_line = scanner->line, \
    .begin_column = scanner->column, \
    .begin_index = scanner->index, \
    .end_line = scanner->line, \
    .end_column = scanner->column, \
    .end_index = scanner->index, \
    .length = 1, \
}

#define SCANNER_PUSH_TOKEN(TOKEN) array_append(scanner->tokens, TOKEN)

#endif
