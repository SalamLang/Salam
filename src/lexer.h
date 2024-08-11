#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdio.h>

#include "array.h"

typedef struct {
    size_t index;
    size_t length;

    size_t start_line;
    size_t start_column;

    size_t end_line;
    size_t end_column;
} location_t;

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER_INT,
    TOKEN_NUMBER_FLOAT,
    TOKEN_BOOLEAN,
    TOKEN_ERROR
} token_type_t;

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
    char* (*name)(void*);
    char* (*value)(void*);
} token_t;

typedef struct {
    const char* file_path; // NULL if source is REPL
    char *source;
    size_t index;
    size_t line;
    size_t column;
    array_t* tokens;
} lexer_t;

/**
 * 
 * @function location_print
 * @brief Print a location
 * @param {location_t} location - Location
 * @returns {void}
 * 
 */
void location_print(location_t location);

/**
 * 
 * @function token_create
 * @brief Creating a new token
 * @param {token_type_t} type - Token type
 * @param {location_t} location - Token location
 * @returns {token_t*}
 * 
 */
token_t* token_create(token_type_t type, location_t location);

/**
 * 
 * @function token_name
 * @brief Get the name of a token
 * @param {token_t} token -
 * @returns {char*}
 * 
 */
char* token_name(token_t* token);

/**
 * 
 * @function token_value
 * @brief Get the value of a token 
 * @param {token_t*} token -
 * @returns {char*}
 * 
 */
char* token_value(token_t* token);

/**
 * 
 * @function token_destroy
 * @brief Destroying a token
 * @param {token_t*} token - Token
 * @returns {void}
 * 
 */
void token_destroy(token_t* token);

/**
 * 
 * @function token_print
 * @brief Print a token
 * @param {token_t*} token - Token
 * @returns {void}
 * 
 */
void token_print(token_t* token);

/**
 * 
 * @function lexer_create
 * @brief Creating a new lexer state
 * @param {const char*} file_path - File path
 * @param {char*} source - Source code
 * @returns {lexer_t*}
 * 
 */
lexer_t* lexer_create(const char* file_path, char* source);

/**
 * 
 * @function lexer_destroy
 * @brief Destroying a lexer state
 * @param {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_destroy(lexer_t* lexer);

/**
 * 
 * @function lexer_lex
 * @brief Lexing the source code
 * @param {lexer_t*} lexer - Lexer state
 * @returns {void}
 * 
 */
void lexer_lex(lexer_t* lexer);

#endif