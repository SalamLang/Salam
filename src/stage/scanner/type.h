#ifndef _STAGE_SCANNER_TYPE_H_
#define _STAGE_SCANNER_TYPE_H_

// base
#include <base.h>

// array
#include <utility/array/type.h>
#include <utility/array/array_append/array_append.h>

typedef struct scanner_t
{
    char* source;                // Pointer to the source code
    char* directory;             // Directory of the source code
    char* file_name;             // File name of the source code

    int line;                    // Current line number
    int column;                  // Current column number
    int index;                   // Current index in the source code
    int length;                  // Length of the source code

    array_t* tokens;             // Array of tokens
} scanner_t;

#define SCANNER_CURRENT (scanner->source[scanner->index])
#define SCANNER_CURRENT_PREV (scanner->source[scanner->index - 1])
#define SCANNER_CURRENT_NEXT (scanner->source[scanner->index + 1])

#define SCANNER_NEXT scanner->index++
#define SCANNER_PREV scanner->index--

#define SCANNER_NEXT_LINE scanner->line++
#define SCANNER_NEXT_COLUMN scanner->column++

#define SCANNER_ZERO_COLUMN scanner->column = 0

#define SCANNER_PUSH_TOKEN(TOKEN) array_append(scanner->tokens, TOKEN)

#endif
