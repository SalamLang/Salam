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

#ifndef SALAM_SOURCE_SOURCE_H
#define SALAM_SOURCE_SOURCE_H

#include "core/prelude.h"
#include "core/arena.h"

typedef struct {
    const char *path;
    char *text;
    size_t len;
} source_file_t;

source_file_t *source_load(arena_t *a, const char *path);

#endif /* SALAM_SOURCE_SOURCE_H */
