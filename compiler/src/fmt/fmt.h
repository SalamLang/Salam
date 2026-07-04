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

#ifndef SALAM_FMT_FMT_H
#define SALAM_FMT_FMT_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "token/token.h"

typedef struct {
    bool tabs;
    int width;
} fmt_style_t;

fmt_style_t fmt_style_default(void);

void fmt_tokens(const token_stream_t *toks, const fmt_style_t *style, sb_t *out);

bool fmt_source(arena_t *a, logger_t *log, const langpack_t *pack,
                const source_file_t *src, const fmt_style_t *style, sb_t *out);

#endif /* SALAM_FMT_FMT_H */
