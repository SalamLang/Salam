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

#ifndef _LEXER_ARRAY_TOKEN_H_
#define _LEXER_ARRAY_TOKEN_H_

#include <stdio.h> // size_t
#include <stdlib.h> // EXIT_FAILURE

#include "token.h" // token_t
#include "../common/memory.h" // memory_destroy

struct array_token_t;
typedef struct array_token_t {
    token_t **tokens;  // Pointer to an array of token_t pointers
    size_t length;       // Current number of tokens
    size_t capacity;   // Current capacity of the array

    void (*destroy)(struct array_token_t *array);
    void (*print)(struct array_token_t *array);
    void (*stringify)(struct array_token_t *array);
} array_token_t;

void array_token_functions(array_token_t *array);
array_token_t *array_token_init(size_t capacity);
void array_token_destroy(array_token_t *array);
void array_token_push(array_token_t *array, token_t *token);
token_t *array_token_get(array_token_t *array, size_t index);
void array_token_remove(array_token_t *array, size_t index);
void array_token_print(array_token_t *array);
void array_token_stringify(array_token_t *array);

#endif
