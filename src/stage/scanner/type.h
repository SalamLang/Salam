#ifndef _STAGE_SCANNER_TYPE_H_
#define _STAGE_SCANNER_TYPE_H_

typedef struct scanner_t
{
    char* source; // Pointer to the source code
    int line;     // Current line number
    int column;   // Current column number
    int index;    // Current index in the source code
} scanner_t;

#endif
