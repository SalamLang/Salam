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

#ifndef SALAM_CORE_SPAN_H
#define SALAM_CORE_SPAN_H

#include "core/prelude.h"

typedef struct {
    uint32_t line;
    uint32_t col;
    uint32_t index;
} src_pos_t;

typedef struct {
    src_pos_t begin;
    src_pos_t end;
    uint32_t  length;
} src_span_t;

#endif /* SALAM_CORE_SPAN_H */
