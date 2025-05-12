#ifndef _STAGE_TOKEN_TYPE_H_
#define _STAGE_TOKEN_TYPE_H_

typedef enum token_type_t {
    TOKEN_TYPE_UNKNOWN = 0,
    TOKEN_TYPE_EOF,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_NUMBER_INT,
    TOKEN_TYPE_NUMBER_FLOAT,
    TOKEN_TYPE_OPERATOR,
} token_type_t;

typedef struct token_location_t
{
    int begin_line;                    // Line number where the token was found
    int begin_column;                  // Column number where the token was found
    int begin_index;                   // Index of the token in the source code

    int end_line;                      // Line number where the token ends
    int end_column;                    // Column number where the token ends
    int end_index;                     // Index of the token in the source code

    int length;                        // Length of the token in the source code
} token_location_t;

typedef struct token_t
{
    char* value;                 // Pointer to the token value
    token_location_t location;   // Location of the token in the source code
    token_type_t type;           // Type of the token
} token_t;

#endif
