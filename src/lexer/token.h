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

#include "../common/base.h" // DEBUG_ME

typedef struct {
    char *type;  // Token type (e.g., keyword, identifier, etc.)
    char *value; // Token value (e.g., "if", "x", etc.)
} token_t;

typedef enum {
    LANGUAGE_PERSIAN,
    LANGUAGE_ENGLISH,
} language_t;

typedef enum {
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

    TOKEN_ERROR,
    TOKEN_EOF,
} token_type_t;

typedef struct {
    token_type_t type;
    const char *localized;
} token_keyword_t;

typedef struct {
    language_t language;
    token_keyword_t *keywords;
} language_map_t;

void free_token(token_t *token);

#endif
