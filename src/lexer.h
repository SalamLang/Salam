#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdio.h>
#include <stdbool.h>

#include "base.h"

typedef struct {
    size_t index;
    size_t length;

    size_t start_line;
    size_t start_column;

    size_t end_line;
    size_t end_column;
} location_t;

typedef enum {
    TOKEN_EOF = -1,
    TOKEN_LEFT_BRACE = '{',
    TOKEN_RIGHT_BRACE = '}',
    TOKEN_LEFT_BRACKET = '[',
    TOKEN_RIGHT_BRACKET = ']',
    TOKEN_COLON = ':',
    TOKEN_COMMA = ',',
    TOKEN_LEFT_PAREN = '(',
    TOKEN_RIGHT_PAREN = ')',

    TOKEN_PLUS = '+',
    TOKEN_MINUS = '-',
    TOKEN_MULTIPLY = '*',
    TOKEN_DIVIDE = '/',
    TOKEN_MOD = '%',
    TOKEN_POWER = '^',
    TOKEN_ASSIGN = '=',
    TOKEN_LESS = '<',
    TOKEN_GREATER = '>',
    TOKEN_NOT = '!',

    TOKEN_NOT_EQUAL = 300,
    TOKEN_EQUAL,
    TOKEN_AND_AND,
    TOKEN_OR_OR,
    TOKEN_AND_BIT,
    TOKEN_OR_BIT,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,

    TOKEN_INCREMENT,
    TOKEN_DECREMENT,

    TOKEN_SHIFT_LEFT,
    TOKEN_SHIFT_RIGHT,
    TOKEN_SHIFT_LEFT_ASSIGN,
    TOKEN_SHIFT_RIGHT_ASSIGN,

    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER_INT,
    TOKEN_NUMBER_FLOAT,
    TOKEN_BOOLEAN,

    // Keywords
    TOKEN_LAYOUT,
    TOKEN_IMPORT,
    TOKEN_FUNCTION,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_BREAK,
    TOKEN_CONTINUE,

    TOKEN_ERROR,
} token_type_t;

typedef struct {
    const char* name;
    token_type_t type;
} keyword_t;

static const keyword_t keywords[] = {
    {"layout", TOKEN_LAYOUT},
    {"import", TOKEN_IMPORT},
    {"function", TOKEN_FUNCTION},
    {"return", TOKEN_RETURN},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"true", TOKEN_BOOLEAN},
    {"false", TOKEN_BOOLEAN},
};

typedef struct {
    token_type_t type;
    location_t location;

    token_type_t data_type;
    union {
        int number_int;
        float number_float;
        char* string;
        bool boolean;
    } data;

    void (*print)(void*);
    void (*destroy)(void*);
    char* (*name)(token_type_t);
    char* (*value)(void*);
} token_t;

#include "array.h"

typedef struct {
    const char* file_path; // NULL if source is REPL
    char *source;
    size_t index;
    size_t line;
    size_t column;
    array_t* tokens;
    size_t token_index; // For parsing purposes
} lexer_t;

/**
 * 
 * @function is_char_digit
 * @brief Check if a character is a digit
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_digit(char c);

/**
 * 
 * @function is_keyword
 * @brief Check if a string is a keyword
 * @params {const char*} string - String
 * @returns {bool}
 * 
 */
token_type_t type_keyword(const char* string);

/**
 * 
 * @function is_char_alpha
 * @brief Check if a character is an alphabet
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_alpha(char c);

/**
 * 
 * @function is_char_alnum
 * @brief Check if a character is an alphabet or a digit
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_alnum(char c);

/**
 * 
 * @function is_char_whitespace
 * @brief Check if a character is a whitespace
 * @params {char} c - Character
 * @returns {bool}
 * 
 */
bool is_char_whitespace(char c);

/**
 * 
 * @function location_print
 * @brief Print a location
 * @params {location_t} location - Location
 * @returns {void}
 * 
 */
void location_print(location_t location);

/**
 * 
 * @function token_create
 * @brief Creating a new token
 * @params {token_type_t} type - Token type
 * @params {location_t} location - Token location
 * @returns {token_t*}
 * 
 */
token_t* token_create(token_type_t type, location_t location);

/**
 * 
 * @function token_copy
 * @brief Copying a token
 * @params {token_t*} token - Token
 * @returns {token_t*}
 * 
 */
token_t* token_copy(token_t* token);

/**
 * 
 * @function token_name
 * @brief Get the name of a token
 * @params {token_type_t} Token type
 * @returns {char*}
 * 
 */
char* token_name(token_type_t type);

/**
 * 
 * @function token_value
 * @brief Get the value of a token 
 * @params {token_t*} token -
 * @returns {char*}
 * 
 */
char* token_value(token_t* token);

/**
 * 
 * @function token_destroy
 * @brief Destroying a token
 * @params {token_t*} token - Token
 * @returns {void}
 * 
 */
void token_destroy(token_t* token);

/**
 * 
 * @function token_print
 * @brief Print a token
 * @params {token_t*} token - Token
 * @returns {void}
 * 
 */
void token_print(token_t* token);

/**
 * 
 * @function lexer_create
 * @brief Creating a new lexer state
 * @params {const char*} file_path - File path
 * @params {char*} source - Source code
 * @returns {lexer_t*}
 * 
 */
lexer_t* lexer_create(const char* file_path, char* source);

/**
 * 
 * @function lexer_destroy
 * @brief Destroying a lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_destroy(lexer_t* lexer);

/**
 * 
 * @function lexer_debug
 * @brief Debugging the lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_debug(lexer_t* lexer);

/**
 * 
 * @function is_keyword
 * @brief Check if a string is a keyword
 * @params {const char*} string - String
 * @returns {bool}
 * 
 */
bool is_keyword(const char* string);

/**
 * 
 * @function lexer_lex
 * @brief Lexing the source code
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex(lexer_t* lexer);

/**
 * 
 * @function lexer_lex_identifier
 * @brief Lexing an identifier
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex_identifier(lexer_t* lexer);

/**
 * 
 * @function lexer_lex_number
 * @brief Lexing a number
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex_number(lexer_t* lexer);

/**
 * 
 * @function lexer_lex_string
 * @brief Lexing a string
 * @params {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex_string(lexer_t* lexer);

#endif