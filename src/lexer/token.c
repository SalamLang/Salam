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

#include "token.h"

#include "config.h"

/**
 *
 * @function token_create
 * @brief Creating a new token
 * @params {token_type_t} type - Token type
 * @params {location_t} location - Token location
 * @returns {token_t*}
 *
 */
token_t *token_create(token_type_t type, location_t location) {
    DEBUG_ME;
    token_t *token = memory_allocate(sizeof(token_t));
    token->type = type;
    token->location = location;
    token->data_type = TOKEN_ERROR;
    token->name = token_name;
    token->value_stringify = token_value_stringify;
    token->print = token_print;
    token->stringify = token_stringify;
    token->destroy = token_destroy;

    return token;
}

char *token_name(token_t *token) {
    return "soon";
}

char *token_value_stringify(token_t *token) {
    return "soon";
}

/**
 *
 * @function token_destroy
 * @brief Destroying a token
 * @params {token_t*} token - Token
 * @returns {void}
 *
 */
void token_destroy(token_t *token) {
    DEBUG_ME;
    if (token != NULL) {
        if (token->data_type == TOKEN_STRING ||
            token->data_type == TOKEN_IDENTIFIER) {
            if (token->data.string != NULL) {
                memory_destroy(token->data.string);
            }
        }

        memory_destroy(token);
    }
}

/**
 *
 * @function token_stringify
 * @brief Get the name & value & location of a token as a string
 * @params {token_t*} token - Token
 * @returns {char*} - String representation of the token
 *
 */
char *token_stringify(token_t *token) {
    DEBUG_ME;
    const char *type = ""; // token_type_stringify(token->type);
    const char *value = ""; // token_value_stringify(token);
    const char *location = location_stringify(token->location);

    size_t size =
        strlen(type) + strlen(value) + strlen(location) + strlen(":  at ") + 1;
    char *buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    snprintf(buffer, size, "%s: %s at %s", type, value, location);

    return buffer;
}

/**
 *
 * @function token_print
 * @brief Print a token
 * @params {token_t*} token - Token
 * @returns {void}
 *
 */
void token_print(token_t *token) {
    DEBUG_ME;
    printf("Token: ");
    printf("%s\n", token_stringify(token));
}
