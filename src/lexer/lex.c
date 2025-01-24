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

#include "lex.h"

#define LEXER_CURRENT (lexer->source[lexer->index])
#define LEXER_CURRENT_PREV (lexer->source[lexer->index - 1])
#define LEXER_CURRENT_NEXT (lexer->source[lexer->index + 1])

#define LEXER_NEXT lexer->index++
#define LEXER_PREV lexer->index--

#define LEXER_NEXT_LINE lexer->line++
#define LEXER_NEXT_COLUMN lexer->column++

#define LEXER_ZERO_COLUMN lexer->column = 0

#define LEXER_CURRENT_UTF8 (utf8_decode(lexer->source, &lexer->index))
#define LEXER_CURRENT_PREV_UTF8 (utf8_decode(lexer->source, &lexer->index - 1))
#define LEXER_CURRENT_NEXT_UTF8 (utf8_decode(lexer->source, &lexer->index + 1))

#define LEXER_NEXT_UTF8 \
    lexer->index += utf8_char_length(lexer->source[lexer->index])
#define LEXER_PREV_UTF8 \
    lexer->index -= utf8_char_length(lexer->source[lexer->index])

#define LEXER_PUSH_TOKEN(TOKEN) array_token_push(lexer->tokens, TOKEN)

/**
 *
 * @function token_char_type
 * @brief Get the type of a character
 * @params {char} c - Character
 * @returns {token_type_t}
 *
 */
token_type_t token_char_type(char c) {
    
}

/**
 *
 * @function lexer_lex_identifier
 * @brief Lexing an identifier
 * @params {lexer_t*} lexer - Lexer state
 * @params {char*} uc - UTF8 character
 * @returns {void}
 *
 */
void lexer_lex_identifier(lexer_t *lexer, char *uc) {
    DEBUG_ME;
    string_t *value = string_create(25);

    string_append_str(value, uc);

    while (LEXER_CURRENT != '\0') {
        size_t num_bytes;
        uc = char_utf8_decode(lexer->source, &lexer->index, &num_bytes);

        if (!is_utf8_alpha(uc)) {
            lexer->index -= num_bytes;

            memory_destroy(uc);

            break;
        }

        string_append_str(value, uc);

        memory_destroy(uc);
    }

    token_type_t type = type_keyword(value->data);
    token_t *token = token_create(
        type, (location_t){lexer->index, 1, lexer->line, lexer->column,
                           lexer->line, lexer->column});
    token->data_type = TOKEN_IDENTIFIER;
    token->data.string = string_arabic2persian(value->data);

    string_destroy(value);

    LEXER_PUSH_TOKEN(token);
}

/**
 *
 * @function lexer_lex_number
 * @brief Lexing a number
 * @params {lexer_t*} lexer - Lexer state
 * @params {char*} uc - UTF8 character
 * @returns {void}
 *
 */
void lexer_lex_number(lexer_t *lexer, char *uc) {
    DEBUG_ME;
    string_t *value = string_create(25);
    string_append_char(value, convert_utf8_to_english_digit(uc));

    bool is_float = false;

    while (LEXER_CURRENT != '\0') {
        // size_t num_bytes;
        uc = char_utf8_decode(lexer->source, &lexer->index, NULL);
        if (strcmp(uc, "\0") == 0) {
            memory_destroy(uc);

            break;
        }

        size_t is_dot = strcmp(uc, ".");

        if (!is_utf8_digit(uc) && is_dot != 0) {
            memory_destroy(uc);

            break;
        }

        if (is_dot == 0) {
            if (is_float) {
                memory_destroy(uc);

                break;
            }

            is_float = true;
        }

        string_append_char(value, convert_utf8_to_english_digit(uc));

        memory_destroy(uc);
    }

    token_type_t type = is_float ? TOKEN_NUMBER_FLOAT : TOKEN_NUMBER_INT;
    token_t *token = token_create(
        type, (location_t){lexer->index, 1, lexer->line, lexer->column,
                           lexer->line, lexer->column});
    token->data_type = type;

    if (is_float) {
        token->data.number_float = atof(value->data);
    } else {
        token->data.number_int = atoi(value->data);
    }

    string_destroy(value);

    LEXER_PUSH_TOKEN(token);
}

/**
 *
 * @function type_keyword
 * @brief Check if a string is a keyword then return the token type
 * @params {const char*} string - String
 * @returns {bool}
 *
 */
token_type_t type_keyword(const char *string) {
    DEBUG_ME;
    // for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
    //     // TODO: keywords[i].length == length
    //     if (strcmp(string, keywords[i].keyword) == 0) {
    //         return keywords[i].type;
    //     }
    // }

    return TOKEN_IDENTIFIER;
}

/**
 *
 * @function lexer_lex_string
 * @brief Lexing a string
 * @params {lexer_t*} lexer - Lexer state
 * @params {int} type - String type (0 = english strings, 1= persian strings, 2=
 * second persian strings)
 * @returns {void}
 *
 */
void lexer_lex_string(lexer_t *lexer, int type) {
    DEBUG_ME;
    // Opening quote is already consumed
    string_t *value = string_create(25);

    size_t num_bytes;
    char *uc = char_utf8_decode(lexer->source, &lexer->index, &num_bytes);

    while (strcmp(uc, "\0") != 0 && ((type == 0 && strcmp(uc, "\"") != 0) ||
                                     (type == 1 && strcmp(uc, "»") != 0) ||
                                     (type == 2 && strcmp(uc, "”") != 0))) {
        string_append_str(value, uc);
        memory_destroy(uc);

        uc = char_utf8_decode(lexer->source, &lexer->index, &num_bytes);
    }

    if ((type == 0 && strcmp(uc, "\"") != 0) ||
        (type == 1 && strcmp(uc, "»") != 0) ||
        (type == 2 && strcmp(uc, "”") != 0)) {
        string_destroy(value);

        if (uc != NULL) {
            memory_destroy(uc);
        }

        error_lexer(2, "Unterminated string value at line %zu, column %zu",
                    lexer->line, lexer->column);

        return;
    }

    if (uc != NULL) {
        memory_destroy(uc);
    }

    token_t *token = token_create(
        TOKEN_STRING, (location_t){lexer->index, 1, lexer->line, lexer->column,
                                   lexer->line, lexer->column});
    token->data_type = TOKEN_STRING;
    token->data.string = string_strdup(value->data);

    string_destroy(value);

    LEXER_PUSH_TOKEN(token);
}

void lex(lexer_t *lexer) {
    DEBUG_ME;
    printf("Code:\n%s\n----------\n", lexer->source);

    char c;

    while ((c = LEXER_CURRENT) && c != '\0' &&
           lexer->index < lexer->source_length) {
        printf("c: %c, i: %zu, length: %zu\n", c, lexer->index,
               lexer->source_length);

        size_t num_bytes;
        char *uc = char_utf8_decode(lexer->source, &lexer->index, &num_bytes);

        switch (num_bytes) {
            case 0: {
                if (uc != NULL) {
                    memory_destroy(uc);
                }

                error_lexer(3, "Invalid UTF-8 encoding detected at index %d",
                            lexer->index);
            } break;

            case 1: {
                char ucf = uc[0];

                switch (ucf) {
                    // End of file
                    case '\0':
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        break;

                    // New line
                    case '\n':
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        LEXER_NEXT_LINE;
                        LEXER_ZERO_COLUMN;
                        continue;

                    case '\a':  // Alert
                    case '\b':  // Backspace
                    case '\f':  // Form feed
                    case '\r':  // Carriage return
                    case '\t':  // Horizontal tab
                    case '\v':  // Vertical tab
                    case ' ':   // Space
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        continue;

                    case '{':
                    case '}':
                    case '[':
                    case ']':
                    case ':':
                    case ',':
                    case '(':
                    case ')':
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                    case '%':
                    case '^':
                    case '=':
                    case '<':
                    case '>':
                    case '!':
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        if (LEXER_CURRENT == '/') {
                            LEXER_NEXT;
                            LEXER_NEXT_COLUMN;

                            while (LEXER_CURRENT != '\n' &&
                                   LEXER_CURRENT != '\0') {
                                LEXER_NEXT;
                                LEXER_NEXT_COLUMN;
                            }
                        } else if (LEXER_CURRENT == '*') {
                            LEXER_NEXT;
                            LEXER_NEXT_COLUMN;

                            while (1) {
                                if (LEXER_CURRENT == '\0') {
                                    error_lexer(
                                        2,
                                        "Unterminated multiline comment at "
                                        "line %zu, column %zu",
                                        lexer->line, lexer->column);
                                    break;
                                }

                                if (LEXER_CURRENT == '*') {
                                    LEXER_NEXT;
                                    LEXER_NEXT_COLUMN;

                                    if (LEXER_CURRENT == '/') {
                                        LEXER_NEXT;
                                        LEXER_NEXT_COLUMN;
                                        break;
                                    }
                                } else {
                                    LEXER_NEXT;
                                    LEXER_NEXT_COLUMN;
                                }
                            }
                        } else {
                            token_type_t type = token_char_type(c);
                            token_t *token = token_create(
                                type, (location_t){lexer->index, 1, lexer->line,
                                                   lexer->column, lexer->line,
                                                   lexer->column});

                            LEXER_PUSH_TOKEN(token);
                        }
                        continue;

                    case '"':
                        if (uc != NULL) {
                            memory_destroy(uc);
                        }

                        lexer_lex_string(lexer, 0);
                        continue;

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        lexer_lex_number(lexer, uc);

                        if (uc != NULL) {
                            memory_destroy(uc);
                        }
                        continue;

                    default:
                        if (c == '_' || is_char_alpha(c)) {
                            lexer_lex_identifier(lexer, uc);
                        } else {
                            error_lexer(1,
                                        "Unknown character '%s' at line %zu, "
                                        "column %zu",
                                        uc, lexer->line, lexer->column);
                        }

                        if (uc != NULL) {
                            memory_destroy(uc);
                        }
                        continue;
                }
            } break;

            case 2:
            case 3:
            case 4:
            default: {
                if (strcmp(uc, "«") == 0) {
                    lexer_lex_string(lexer, 1);
                } else if (strcmp(uc, "“") == 0) {
                    lexer_lex_string(lexer, 2);
                } else if (is_utf8_digit(uc)) {
                    lexer_lex_number(lexer, uc);
                } else if (c == '_' || is_utf8_alpha(uc) || is_char_alpha(c)) {
                    lexer_lex_identifier(lexer, uc);
                } else {
                    error_lexer(
                        1, "Unknown character '%s' at line %zu, column %zu", uc,
                        lexer->line, lexer->column);
                }

                if (uc != NULL) {
                    memory_destroy(uc);
                }
                continue;
            } break;
        }
    }
   
    token_t *token = token_create(
        TOKEN_EOF, (location_t){lexer->index, 1, lexer->line, lexer->column,
                                lexer->line, lexer->column});

    LEXER_PUSH_TOKEN(token);
}
