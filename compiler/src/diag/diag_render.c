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

#include "core/prelude.h"
#include "diag/diag_render.h"
#include "diag/diag_internal.h"

bool diag_style_from_string(const char *s, diag_style_t *out)
{
    if (!s) return false;
#if defined(_WIN32)
#  define STR_CI _stricmp
#else
#  define STR_CI strcasecmp
#endif
    if (STR_CI(s, "rust") == 0) {
        *out = DIAG_STYLE_RUST;
        return true;
    }
    if (STR_CI(s, "gcc") == 0) {
        *out = DIAG_STYLE_GCC;
        return true;
    }
    if (STR_CI(s, "clang") == 0) {
        *out = DIAG_STYLE_CLANG;
        return true;
    }
    return false;
#undef STR_CI
}

bool diag_format_from_string(const char *s, diag_format_t *out)
{
    if (!s) return false;
#if defined(_WIN32)
#  define STR_CI _stricmp
#else
#  define STR_CI strcasecmp
#endif
    if (STR_CI(s, "human") == 0) {
        *out = DIAG_FORMAT_HUMAN;
        return true;
    }
    if (STR_CI(s, "json") == 0) {
        *out = DIAG_FORMAT_JSON;
        return true;
    }
    if (STR_CI(s, "xml") == 0) {
        *out = DIAG_FORMAT_XML;
        return true;
    }
    return false;
#undef STR_CI
}

void diag_render(FILE *sink, const char *level, int code, bool is_error, const char *file,
                 const src_span_t *span, const char *src_text, size_t src_len,
                 const char *msg, const char *help, diag_style_t style,
                 diag_format_t format, bool color)
{
    if (!sink) return;
    if (!level) level = "error";
    if (!msg) msg = "";
    switch (format) {
    case DIAG_FORMAT_JSON:
        diag_render_json(sink, level, code, is_error, file, span, msg, help);
        return;
    case DIAG_FORMAT_XML:
        diag_render_xml(sink, level, code, is_error, file, span, msg, help);
        return;
    default:
        break;
    }

    switch (style) {
    case DIAG_STYLE_GCC:
        diag_render_gcc(sink, level, code, is_error, file, span, src_text, src_len, msg,
                        help, color);
        break;
    case DIAG_STYLE_CLANG:
        diag_render_clang(sink, level, code, is_error, file, span, src_text, src_len, msg,
                          help, color);
        break;
    default:
        diag_render_rust(sink, level, code, is_error, file, span, src_text, src_len, msg,
                         help, color);
        break;
    }
}
