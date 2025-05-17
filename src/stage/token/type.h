#ifndef _STAGE_TOKEN_TYPE_H_
#define _STAGE_TOKEN_TYPE_H_

// enum token_type_t;
// enum token_operator_type_t;
// struct token_t;
// struct token_location_t;

// typedef enum token_type_t token_type_t;
// typedef enum token_operator_type_t token_operator_type_t;
// typedef struct token_t token_t;
// typedef struct token_location_t token_location_t;

#include <stddef.h> // for size_t

// base
// #include <base.h>
// TODO

// value
#include <stage/value/type.h>

typedef enum token_operator_type_t {
    TOKEN_OPERATOR_TYPE_UNKNOWN = 1000,
    TOKEN_OPERATOR_TYPE_BRACE_OPEN, // {
    TOKEN_OPERATOR_TYPE_BRACE_CLOSE, // }
    TOKEN_OPERATOR_TYPE_PAREN_OPEN, // (
    TOKEN_OPERATOR_TYPE_PAREN_CLOSE, // )
    TOKEN_OPERATOR_TYPE_BRACKET_OPEN, // [
    TOKEN_OPERATOR_TYPE_BRACKET_CLOSE, // ]
    TOKEN_OPERATOR_TYPE_COMMA, // ,
    TOKEN_OPERATOR_TYPE_SEMICOLON, // ;
    TOKEN_OPERATOR_TYPE_COLON, // :
    TOKEN_OPERATOR_TYPE_DOT, // .
    TOKEN_OPERATOR_TYPE_DOT_DOT, // ..
    TOKEN_OPERATOR_TYPE_DOT_DOT_DOT, // ...
    TOKEN_OPERATOR_TYPE_ARROW, // ->
    TOKEN_OPERATOR_TYPE_HASH, // #
    TOKEN_OPERATOR_TYPE_ASSIGN, // =
    TOKEN_OPERATOR_TYPE_PLUS, // +
    TOKEN_OPERATOR_TYPE_MINUS, // -
    TOKEN_OPERATOR_TYPE_ASTERISK, // *
    TOKEN_OPERATOR_TYPE_SLASH, // /
    TOKEN_OPERATOR_TYPE_PERCENT, // %
    TOKEN_OPERATOR_TYPE_CARET, // ^
    TOKEN_OPERATOR_TYPE_AMPERSAND, // &
    TOKEN_OPERATOR_TYPE_PIPE, // |
    TOKEN_OPERATOR_TYPE_TILDE, // ~
    TOKEN_OPERATOR_TYPE_EXCLAMATION, // !
    TOKEN_OPERATOR_TYPE_QUESTION, // ?
    TOKEN_OPERATOR_TYPE_NULL_COALESCING, // ??
    TOKEN_OPERATOR_TYPE_NULLISH_COALESCING, // ?:
    TOKEN_OPERATOR_TYPE_LT, // <
    TOKEN_OPERATOR_TYPE_GT, // >
    TOKEN_OPERATOR_TYPE_LT_EQ, // <=
    TOKEN_OPERATOR_TYPE_GT_EQ, // >=
    TOKEN_OPERATOR_TYPE_EQ, // ==
    TOKEN_OPERATOR_TYPE_NOT_EQ, // !=
    TOKEN_OPERATOR_TYPE_AND, // &&
    TOKEN_OPERATOR_TYPE_OR, // ||
    TOKEN_OPERATOR_TYPE_INC, // ++
    TOKEN_OPERATOR_TYPE_DEC, // --
} token_operator_type_t;

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
    TOKEN_TYPE_KEYWORD_END,
    TOKEN_TYPE_KEYWORD_IMPORT,
    TOKEN_TYPE_KEYWORD_PACKAGE,
    TOKEN_TYPE_KEYWORD_STRUCT,
    TOKEN_TYPE_KEYWORD_ENUM,
    TOKEN_TYPE_KEYWORD_UNION,
    TOKEN_TYPE_KEYWORD_TYPE,
    TOKEN_TYPE_KEYWORD_RAW,

    // kind
    TOKEN_TYPE_KIND_INT,
    TOKEN_TYPE_KIND_FLOAT,
    TOKEN_TYPE_KIND_STRING,
    TOKEN_TYPE_KIND_BOOL,
    TOKEN_TYPE_KIND_ARRAY,
    TOKEN_TYPE_KIND_MAP,
    TOKEN_TYPE_KIND_SET,
    TOKEN_TYPE_KIND_TUPLE,
    TOKEN_TYPE_KIND_VOID,
    TOKEN_TYPE_KIND_ANY,

    // value
    TOKEN_TYPE_VALUE_STRING,
    TOKEN_TYPE_VALUE_NUMBER_INT,
    TOKEN_TYPE_VALUE_NUMBER_FLOAT,
    TOKEN_TYPE_VALUE_TRUE,
    TOKEN_TYPE_VALUE_FALSE,
    TOKEN_TYPE_VALUE_NULL,
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
    char* source;                         // Pointer to the token value
    token_location_t location;            // Location of the token in the source code
    token_type_t type;                    // Type of the token
    token_operator_type_t operator_type;  // Operator type of the token
    value_t* value;                       // Pointer to the value of the token
} token_t;

#endif // _STAGE_TOKEN_TYPE_H_
