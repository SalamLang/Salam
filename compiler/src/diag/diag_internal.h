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

#ifndef SALAM_DIAG_INTERNAL_H
#define SALAM_DIAG_INTERNAL_H

#include "core/prelude.h"
#include "core/span.h"

#define A_RESET  "\033[0m"

#define A_BOLD   "\033[1m"

#define A_RED    "\033[1;31m"   /* bold red    - error   */

#define A_YELLOW "\033[1;33m"   /* bold yellow - warning */

#define A_CYAN   "\033[1;36m"   /* bold cyan   - note    */

#define A_GREEN  "\033[1;32m"   /* bold green  - help    */

#define A_BLUE   "\033[34m"     /* blue        - gutter  */

#define A_BBLUE  "\033[1;34m"   /* bold blue   - line numbers */

void diag_render_rust(FILE *sink,
                      const char *level, int code, bool is_error,
                      const char *file, const src_span_t *span,
                      const char *src_text, size_t src_len,
                      const char *body, const char *help, bool color);

void diag_render_gcc(FILE *sink,
                     const char *level, int code, bool is_error,
                     const char *file, const src_span_t *span,
                     const char *src_text, size_t src_len,
                     const char *body, const char *help, bool color);

void diag_render_clang(FILE *sink,
                       const char *level, int code, bool is_error,
                       const char *file, const src_span_t *span,
                       const char *src_text, size_t src_len,
                       const char *body, const char *help, bool color);

void diag_render_json(FILE *sink,
                      const char *level, int code, bool is_error,
                      const char *file, const src_span_t *span,
                      const char *body, const char *help);

void diag_render_xml(FILE *sink,
                     const char *level, int code, bool is_error,
                     const char *file, const src_span_t *span,
                     const char *body, const char *help);

#endif /* SALAM_DIAG_INTERNAL_H */
