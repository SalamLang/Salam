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

#include "array_token.h"

void array_token_init(array_token_t *array) {
    DEBUG_ME;
    array->tokens = NULL;
    array->size = 0;
    array->capacity = 0;
}

void array_token_free(array_token_t *array) {
    DEBUG_ME;
    for (size_t i = 0; i < array->size; ++i) {
        free_token(array->tokens[i]);
    }
    free(array->tokens);
    array->tokens = NULL;
    array->size = 0;
    array->capacity = 0;
}

static void resize_array_if_needed(array_token_t *array) {
    DEBUG_ME;
    if (array->size < array->capacity) return;

    size_t new_capacity = (array->capacity == 0) ? 4 : array->capacity * 2;
    token_t **new_tokens = realloc(array->tokens, new_capacity * sizeof(token_t *));
    if (!new_tokens) {
        fprintf(stderr, "Error: Failed to allocate memory for tokens.\n");
        exit(EXIT_FAILURE);
    }
    array->tokens = new_tokens;
    array->capacity = new_capacity;
}

void array_token_add(array_token_t *array, token_t *token) {
    DEBUG_ME;
    resize_array_if_needed(array);
    array->tokens[array->size++] = token;
}

token_t *array_token_get(array_token_t *array, size_t index) {
    DEBUG_ME;
    if (index >= array->size) {
        fprintf(stderr, "Error: Index out of bounds.\n");
        exit(EXIT_FAILURE);
    }
    return array->tokens[index];
}

static void shift_tokens_left(array_token_t *array, size_t index) {
    DEBUG_ME;
    for (size_t i = index; i < array->size - 1; ++i) {
        array->tokens[i] = array->tokens[i + 1];
    }
}

void array_token_remove(array_token_t *array, size_t index) {
    DEBUG_ME;
    if (index >= array->size) {
        fprintf(stderr, "Error: Index out of bounds.\n");
        exit(EXIT_FAILURE);
    }

    free_token(array->tokens[index]);
    shift_tokens_left(array, index);
    array->size--;

    if (array->size == 0) {
        free(array->tokens);
        array->tokens = NULL;
        array->capacity = 0;
    }
}
