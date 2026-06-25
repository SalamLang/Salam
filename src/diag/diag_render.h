#ifndef SALAM_DIAG_DIAG_RENDER_H
#define SALAM_DIAG_DIAG_RENDER_H

#include "core/prelude.h"
#include "core/span.h"

typedef enum {
    DIAG_STYLE_RUST  = 0, /* Rust-like: multi-line with --> and ^^ (default) */
    DIAG_STYLE_GCC,       /* GCC-like : file:line:col: error: msg + ^ caret  */
    DIAG_STYLE_CLANG,     /* Clang-like: range underline with vertical label  */
} diag_style_t;

typedef enum {
    DIAG_FORMAT_HUMAN = 0, /* ASCII-art (default)            */
    DIAG_FORMAT_JSON,      /* JSON for IDE / tool integration */
    DIAG_FORMAT_XML,       /* XML for IDE / tool integration  */
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
