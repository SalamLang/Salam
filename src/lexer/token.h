/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 *
 * Welcome to the Salam Programming Language! Salam is a modern, efficient,
 * and developer-friendly language created to inspire and empower programmers
 * of all ages. With its unique features and Persian roots, Salam aims to make
 * programming more accessible and enjoyable for everyone.
 *
 * This file is part of the core implementation of Salam, including its runtime
 * and compiler components. By contributing to or using this codebase, you are
 * part of a growing community dedicated to innovation and inclusivity in
 * programming.
 *
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 *
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

#ifndef _LEXER_TOKEN_H_
#define _LEXER_TOKEN_H_

#include <stdlib.h> // free
#include <stddef.h> // NULL
#include <stdbool.h> // bool

#include "location.h"
#include "../common/string_buffer.h" // string_strdup
#include "../common/memory.h" // memory_destroy
#include "../common/base.h" // DEBUG_ME

typedef enum {
    // Values
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_BOOLEAN,
    TOKEN_NUMBER_FLOAT,
    TOKEN_NUMBER_INT,
    TOKEN_BOOL_TRUE,
    TOKEN_BOOL_FALSE,

    // Words
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_PRINT,
    TOKEN_FN,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_REPEAT,
    TOKEN_DO,
    TOKEN_RET,
    TOKEN_BREAK,
    TOKEN_CONOTINUE,
    TOKEN_LAYOUT,
    TOKEN_BLOCK_BEGIN,
    TOKEN_BLOCK_END,
    // Operators
    TOKEN_NOT_EQUAL,
    TOKEN_EQUAL,
    TOKEN_AND_AND,
    TOKEN_OR_OR,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_SHIFT_LEFT,
    TOKEN_SHIFT_RIGHT,
    TOKEN_SHIFT_RIGHT_ASSIGN,
    TOKEN_SHIFT_LEFT_ASSIGN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MOD,
    TOKEN_POWER,
    TOKEN_ASSIGN,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_NOT,
    TOKEN_AND_BIT,
    TOKEN_OR_BIT,
    TOKEN_XOR_BIT,
    // Others
    TOKEN_ERROR,
    TOKEN_EOF,
} token_type_t;

struct token_t;

typedef struct token_t {
    token_type_t type;
    location_t location;

    token_type_t data_type;
    union {
        int number_int;
        float number_float;
        char *string;
        bool boolean;
    } data;

    void (*print)(struct token_t *);
    void (*destroy)(struct token_t *);
    char *(*stringify)(struct token_t *);
    char *(*value_stringify)(struct token_t *);
} token_t;

token_t *token_create(token_type_t type, location_t location);
void token_destroy(token_t *token);
char *token_name(token_t *token);
char *token_stringify(token_t *token);
void token_print(token_t *token);

#include "token_type.h"
#include "token_value.h"

#endif
