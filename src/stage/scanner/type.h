#ifndef _STAGE_SCANNER_TYPE_H_
#define _STAGE_SCANNER_TYPE_H_

#include <utility/array/type.h>

typedef struct scanner_t
{
    char* source;                // Pointer to the source code
    int line;                    // Current line number
    int column;                  // Current column number
    int index;                   // Current index in the source code
    array_t* tokens;             // Array of tokens
} scanner_t;

#endif
