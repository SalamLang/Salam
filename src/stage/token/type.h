#ifndef _STAGE_TOKEN_TYPE_H_
#define _STAGE_TOKEN_TYPE_H_

#include <stddef.h> // for size_t

typedef enum token_type_t {
    TOKEN_TYPE_UNKNOWN = 0,
    TOKEN_TYPE_EOF,

    TOKEN_TYPE_OPERATOR,

    TOKEN_TYPE_IDENTIFIER,

    // keyword
    TOKEN_TYPE_KEYWORD_IF,
    TOKEN_TYPE_KEYWORD_ELSE,
    TOKEN_TYPE_KEYWORD_FOR,
    TOKEN_TYPE_KEYWORD_WHILE,
    TOKEN_TYPE_KEYWORD_RET,
    TOKEN_TYPE_KEYWORD_FN,

    // value
    TOKEN_TYPE_VALUE_STRING,
    TOKEN_TYPE_VALUE_NUMBER_INT,
    TOKEN_TYPE_VALUE_NUMBER_FLOAT,
} token_type_t;

typedef struct token_location_t
{
    size_t begin_line;                    // Line number where the token was found
    size_t begin_column;                  // Column number where the token was found
    size_t begin_index;                   // Index of the token in the source code

    size_t end_line;                      // Line number where the token ends
    size_t end_column;                    // Column number where the token ends
    size_t end_index;                     // Index of the token in the source code

    size_t length;                        // Length of the token in the source code
} token_location_t;

typedef struct token_t
{
    char* value;                 // Pointer to the token value
    token_location_t location;   // Location of the token in the source code
    token_type_t type;           // Type of the token
} token_t;

#endif
