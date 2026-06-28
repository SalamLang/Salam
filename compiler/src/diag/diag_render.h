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

#ifndef SALAM_DIAG_DIAG_RENDER_H
#define SALAM_DIAG_DIAG_RENDER_H

#include "core/prelude.h"
#include "core/span.h"

typedef enum {
    DIAG_STYLE_RUST  = 0,
    DIAG_STYLE_GCC,
    DIAG_STYLE_CLANG,
} diag_style_t;

typedef enum {
    DIAG_FORMAT_HUMAN = 0,
    DIAG_FORMAT_JSON,
    DIAG_FORMAT_XML,
} diag_format_t;

bool diag_style_from_string(const char *s, diag_style_t *out);

bool diag_format_from_string(const char *s, diag_format_t *out);

void diag_render(FILE *sink,
                 const char *level,
                 int         code,
                 bool        is_error,
                 const char *file,
                 const src_span_t *span,
                 const char *src_text,
                 size_t      src_len,
                 const char *msg,
                 const char *help,
                 diag_style_t  style,
                 diag_format_t format,
                 bool          color);

#endif /* SALAM_DIAG_DIAG_RENDER_H */
