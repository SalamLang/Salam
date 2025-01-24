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

#ifndef _LEXER_LANGUAGE_H_
#define _LEXER_LANGUAGE_H_

#include <stdlib.h> // NULL

#include "token.h"
#include "token_keyword.h"

typedef enum {
    LANGUAGE_PERSIAN,
    LANGUAGE_ENGLISH,
} language_t;

typedef struct {
    language_t language;
    token_keyword_t *keywords;
} language_map_t;

extern token_keyword_t en_keywords[];
extern token_keyword_t fa_keywords[];
extern language_map_t language_maps[];

#endif
