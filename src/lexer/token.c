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

token_keyword_t en_keywords[] = {
    {TOKEN_IF, "if"},
    {TOKEN_ELSE, "else"},
    {TOKEN_PRINT, "print"},
    {TOKEN_FN, "fn"},
    {TOKEN_FOR, "for"},
    {TOKEN_WHILE, "while"},
    {TOKEN_REPEAT, "repeat"},
    {TOKEN_DO, "do"},
    {TOKEN_RET, "ret"},
    {TOKEN_BREAK, "break"},
    {TOKEN_CONOTINUE, "continue"},
    {TOKEN_LAYOUT, "layout"},
    {TOKEN_BLOCK_BEGIN, ":"},
    {TOKEN_BLOCK_END, "end"},
    {TOKEN_EOF, NULL},
};

token_keyword_t fa_keywords[] = {
    {TOKEN_IF, "اگر"},
    {TOKEN_ELSE, "وگرنه"},
    {TOKEN_PRINT, "چاپ"},
    {TOKEN_FN, "تابع"},
    {TOKEN_FOR, "برای"},
    {TOKEN_WHILE, "تا"},
    {TOKEN_REPEAT, "تکرار"},
    {TOKEN_DO, "انجام"},
    {TOKEN_RET, "بازگرداندن"},
    {TOKEN_BREAK, "شکستن"},
    {TOKEN_CONOTINUE, "ادامه"},
    {TOKEN_LAYOUT, "چیدمان"},
    {TOKEN_BLOCK_BEGIN, ":"},
    {TOKEN_BLOCK_END, "پایان"},
    {TOKEN_EOF, NULL},
};

language_map_t language_maps[] = {
    {LANGUAGE_ENGLISH, en_keywords},
    {LANGUAGE_PERSIAN, fa_keywords},
    {-1, NULL},
};

void free_token(token_t *token) {
    DEBUG_ME;
    if (!token) return;
    free(token->type);
    free(token->value);
    free(token);
}
