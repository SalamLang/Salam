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

#ifndef _LEXER_LEX_H_
#define _LEXER_LEX_H_

#include "token.h"
#include "lexer.h"
#include "array_token.h"

token_type_t token_char_type(char c);
void lex(lexer_t *lexer);
token_type_t type_keyword(const char *string);
void lexer_lex_number(lexer_t *lexer, char *uc);
void lexer_lex_identifier(lexer_t *lexer, char *uc);
void lexer_lex_string(lexer_t *lexer, int type);

#endif
