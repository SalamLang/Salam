#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdio.h>
#include <wchar.h>
#include <errno.h>
#include <stdbool.h>

#include "base.h"
#include "file.h"

typedef struct {
    size_t index;
    size_t length;

    size_t start_line;
    size_t start_column;

    size_t end_line;
    size_t end_column;
} location_t;

#define _NUM_ARGS(X100, X99, X98, X97, X96, X95, X94, X93, X92, X91, \
                  X90, X89, X88, X87, X86, X85, X84, X83, X82, X81, \
                  X80, X79, X78, X77, X76, X75, X74, X73, X72, X71, \
                  X70, X69, X68, X67, X66, X65, X64, X63, X62, X61, \
                  X60, X59, X58, X57, X56, X55, X54, X53, X52, X51, \
                  X50, X49, X48, X47, X46, X45, X44, X43, X42, X41, \
                  X40, X39, X38, X37, X36, X35, X34, X33, X32, X31, \
                  X30, X29, X28, X27, X26, X25, X24, X23, X22, X21, \
                  X20, X19, X18, X17, X16, X15, X14, X13, X12, X11, \
                  X10, X9, X8, X7, X6, X5, X4, X3, X2, X1, N, ...) N

#define NUM_ARGS(...) _NUM_ARGS(__VA_ARGS__, 100, 99, 98, 97, 96, 95, \
                                94, 93, 92, 91, 90, 89, 88, 87, 86, 85, \
                                84, 83, 82, 81, 80, 79, 78, 77, 76, 75, \
                                74, 73, 72, 71, 70, 69, 68, 67, 66, 65, \
                                64, 63, 62, 61, 60, 59, 58, 57, 56, 55, \
                                54, 53, 52, 51, 50, 49, 48, 47, 46, 45, \
                                44, 43, 42, 41, 40, 39, 38, 37, 36, 35, \
                                34, 33, 32, 31, 30, 29, 28, 27, 26, 25, \
                                24, 23, 22, 21, 20, 19, 18, 17, 16, 15, \
                                14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define ADD_TOKEN_TEST(...) ADD_TOKEN_CHOOSER(NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define ADD_TOKEN_CHOOSER(N) ADD_TOKEN_##N

typedef enum {
    #undef ADD_TOKEN
    #undef ADD_CHAR_TOKEN
    #undef ADD_KEYWORD
    #undef ADD_KEYWORD_HIDE
    
    #define ADD_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE) TOKEN_TYPE,
    #define ADD_CHAR_TOKEN(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_CHAR) TOKEN_TYPE,
    #define ADD_KEYWORD(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) TOKEN_TYPE,
    #define ADD_KEYWORD_HIDE(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH) 
    
    #include "token.h"
} token_type_t;

typedef struct {
    token_type_t token;
    const char* name; // upper case
    const char* keyword; // end-user input
    int code; // -1 for non-char tokens
} token_name_t;

typedef struct {
    size_t length;
    token_type_t type;
    const char* name; // upper case
    const char* keyword; // end-user input
} keyword_t;

// #include "validator.h"

/**
 * 
 * @variable token_names
 * @brief Token names
 * @type {token_name_t[]}
 * 
 */
extern token_name_t token_names[];

/**
 * 
 * @variable keywords
 * @brief Keywords
 * @type {keyword_t[]}
 * 
 */
extern const keyword_t keywords[];

struct token_t;

typedef struct token_t {
    token_type_t type;
    location_t location;

    token_type_t data_type;
    union {
        int number_int;
        float number_float;
        char* string;
        bool boolean;
    } data;

    void (*print)(struct token_t*);
    void (*destroy)(struct token_t*);
    char* (*name)(token_type_t);
    char* (*stringify)(struct token_t*);
    char* (*value_stringify)(struct token_t*);
} token_t;

#include "array.h"

typedef struct {
    const char* file_path; // NULL if source is REPL
    char* source;
    size_t index;
    size_t line;
    size_t column;
    size_t source_size;
    size_t source_length;
    array_token_t* tokens;
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
 * @function type_keyword
 * @brief Check if a string is a keyword then return the token type
 * @params {const char*} string - String
 * @returns {bool}
 * 
 */
token_type_t type_keyword(const char* string);

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
char* token_value_stringify(token_t* token);

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
 * @params {wchar_t} wc - wide character
 * @params {int} wcl - Wide character length
 * @returns {void}
 * 
 */
void lexer_lex_identifier(lexer_t* lexer, wchar_t* wc, int char_size);

/**
 * 
 * @function lexer_lex_number
 * @brief Lexing a number
 * @params {lexer_t*} lexer - Lexer state
 * @params {wchar_t} wc - wide character
 * @params {int} char_size - Character size
 * @returns {void}
 * 
 */
void lexer_lex_number(lexer_t* lexer, wchar_t wc, int char_size);

/**
 * 
 * @function lexer_lex_string
 * @brief Lexing a string
 * @params {lexer_t*} lexer - Lexer state
 * @params {int} type - String type (0 = english strings, 1= persian strings)
 * @returns {void}
 * 
 */
void lexer_lex_string(lexer_t* lexer, int type);

/**
 * 
 * @function lexer_save
 * @brief Saving the lexer state
 * @params {lexer_t*} lexer - Lexer state
 * @params {const char*} tokens_output - Tokens output file
 * @returns {void}
 * 
 */
void lexer_save(lexer_t* lexer, const char* tokens_output);

/**
 * 
 * @function location_string
 * @brief Get the string representation of a location
 * @params {location_t} location - Location
 * @returns {char*}
 * 
 */
char* location_stringify(location_t location);

/**
 * 
 * @function token_stringify
 * @brief Get the name & value & location of a token as a string
 * @params {token_t*} token - Token
 * @returns {char*} - String representation of the token
 * 
 */
char* token_stringify(token_t* token);

/**
 * 
 * @function token_type_stringify
 * @brief Get the name of a token type
 * @params {token_type_t} type - Token type
 * @returns {char*}
 * 
 */
const char* token_type_stringify(token_type_t type);

/**
 * 
 * @function token_char_type
 * @brief Get the type of a character
 * @params {char} c - Character
 * @returns {token_type_t}
 * 
 */
token_type_t token_char_type(char c);

/**
 * 
 * @function token_type_keyword
 * @brief Get the keyword of a token type
 * @params {token_type_t} Token type
 * @returns {char*}
 * 
 */
char* token_type_keyword(token_type_t type);

/**
 * 
 * @function read_token
 * @brief Reading a token
 * @params {lexer_t*} lexer - Lexer state
 * @params {int*} char_size - Character size
 * @returns {wchar_t}
 * 
 */
wchar_t read_token2(lexer_t* lexer, int* char_size);

/**
 * 
 * @function is_english_digit
 * @brief Check if the character is an English digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is an English digit, false otherwise
 * 
 */
bool is_english_digit(wchar_t ch);

/**
 * 
 * @function is_persian_digit
 * @brief Check if the character is a Persian digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is a Persian digit, false otherwise
 * 
 */
bool is_persian_digit(wchar_t ch);

/**
 * 
 * @function is_arabic_digit
 * @brief Check if the character is an Arabic digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is an Arabic digit, false otherwise
 * 
 */
bool is_arabic_digit(wchar_t ch);

/**
 * 
 * @function string_is_number
 * @brief Check if the string is a number
 * @params {const char*} value - Value
 * @returns {bool} - True if the string is a number, false otherwise
 * 
 */
bool string_is_number(const char* value);

/**
 * 
 * @function string_is_percentage
 * @brief Check if the string is a percentage
 * @params {const char*} value - Value
 * @params {bool} acceptSign - Accept sign
 * @returns {bool} - True if the string is a percentage
 * 
 */
bool string_is_percentage(const char* value, bool acceptSign);

bool is_ident(wchar_t ch);

bool is_number(wchar_t ch);

bool is_alpha(wchar_t ch);

// wchar_t read_token(lexer_t* lexer);
wchar_t read_token(lexer_t* lexer, int* wcl);

#endif