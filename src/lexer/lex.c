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

// token_t* lex_token()

void lex(lexer_t *lexer) {
    DEBUG_ME;
    printf("Code:\n%s\n----------\n", lexer->source);

    char c;

    while ((c = LEXER_CURRENT) && c != '\0' &&
           lexer->index < lexer->source_length) {
        // printf("c: %c, i: %zu, length: %zu\n", c, lexer->index,
        //        lexer->source_length);
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
