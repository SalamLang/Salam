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

token_keyword_t keywords[] = {
    // Comparison Operators
    {TOKEN_EQUAL, "=="},
    {TOKEN_NOT_EQUAL, "!="},
    {TOKEN_LESS, "<"},
    {TOKEN_GREATER, ">"},
    {TOKEN_LESS_EQUAL, "<="},
    {TOKEN_GREATER_EQUAL, ">="},
    
    // Logical Operators
    {TOKEN_AND_AND, "&&"},
    {TOKEN_OR_OR, "||"},
    {TOKEN_NOT, "!"},
    
    // Bitwise Operators
    {TOKEN_AND_BIT, "&"},
    {TOKEN_OR_BIT, "|"},
    {TOKEN_XOR_BIT, "^"},
    {TOKEN_SHIFT_LEFT, "<<"},
    {TOKEN_SHIFT_RIGHT, ">>"},
    
    // Assignment Operators
    {TOKEN_ASSIGN, "="},
    {TOKEN_SHIFT_LEFT_ASSIGN, "<<="},
    {TOKEN_SHIFT_RIGHT_ASSIGN, ">>="},
    
    // Parentheses and Braces
    {TOKEN_LEFT_PAREN, "("},
    {TOKEN_RIGHT_PAREN, ")"},
    {TOKEN_LEFT_BRACE, "{"},
    {TOKEN_RIGHT_BRACE, "}"},
    {TOKEN_LEFT_BRACKET, "["},
    {TOKEN_RIGHT_BRACKET, "]"},
    
    // Other Symbols
    {TOKEN_COMMA, ","},
    {TOKEN_COLON, ":"},

    {TOKEN_BLOCK_BEGIN, ":"},
    {TOKEN_EOF, NULL},
};

token_keyword_t en_keywords[] = {
    {TOKEN_IF, "if"},
    {TOKEN_ELSE, "else"},
    {TOKEN_PRINT, "print"},
    {TOKEN_FN, "fn"},
    {TOKEN_REPEAT, "repeat"},
    {TOKEN_RET, "ret"},
    {TOKEN_BREAK, "break"},
    {TOKEN_CONOTINUE, "continue"},
    {TOKEN_LAYOUT, "layout"},
    {TOKEN_BLOCK_END, "end"},
    {TOKEN_EOF, NULL},
};

token_keyword_t fa_keywords[] = {
    {TOKEN_IF, "اگر"},
    {TOKEN_ELSE, "وگرنه"},
    {TOKEN_PRINT, "چاپ"},
    {TOKEN_FN, "تابع"},
    {TOKEN_REPEAT, "تکرار"},
    {TOKEN_RET, "برگشت"},
    {TOKEN_BREAK, "شکستن"},
    {TOKEN_CONOTINUE, "ادامه"},
    {TOKEN_LAYOUT, "صفحه"},
    {TOKEN_BLOCK_END, "پایان"},
    {TOKEN_EOF, NULL},
};

language_map_t language_maps[] = {
    // First is default language
    {LANGUAGE_ENGLISH, en_keywords},
    {LANGUAGE_PERSIAN, fa_keywords},
    {-1, NULL},
};

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

    // token->name = token_name;
    // token->value_stringify = token_value_stringify;

    token->print = token_print;
    token->stringify = token_stringify;
    token->destroy = token_destroy;

    return token;
}

char *token_name(token_t *token) {
    
}

char *token_value_stringify(token_t *token) {
    
}

/**
 *
 * @function token_copy
 * @brief Copying a token
 * @params {token_t*} token - Token
 * @returns {token_t*}
 *
 */
token_t *token_copy(token_t *token) {
    DEBUG_ME;
    token_t *copy = token_create(token->type, token->location);
    copy->data_type = token->data_type;

    switch (token->data_type) {
        case TOKEN_NUMBER_INT:
            copy->data.number_int = token->data.number_int;
            break;

        case TOKEN_NUMBER_FLOAT:
            copy->data.number_float = token->data.number_float;
            break;

        case TOKEN_STRING:
        case TOKEN_IDENTIFIER:
            if (token->data.string != NULL) {
                copy->data.string = string_strdup(token->data.string);
            }
            break;

        case TOKEN_BOOLEAN:
            copy->data.boolean = token->data.boolean;
            break;

        default:
            break;
    }

    return copy;
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
