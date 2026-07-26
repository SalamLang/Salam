/*
 * Salam Programming Language (2024–2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

#ifndef SALAM_MINIFY_MINIFY_H
#define SALAM_MINIFY_MINIFY_H

#include "core/prelude.h"
#include "core/arena.h"

const char *minify_js(arena_t *a, const char *src);

const char *minify_css(arena_t *a, const char *src);

#endif /* SALAM_MINIFY_MINIFY_H */
