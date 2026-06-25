#include "core/prelude.h"
#include "diag/diag_internal.h"
#include "i18n/i18n.h"

#define TAB_WIDTH 4

static size_t get_src_line(const char *text, size_t text_len,
                           uint32_t lineno, char *buf, size_t cap)
{
    const char *p   = text;
    const char *end = text + text_len;
    uint32_t    cur = 1;
    while (cur < lineno && p < end) {
        if (*p++ == '\n') cur++;
    }
    if (p >= end || cur != lineno) { buf[0] = '\0'; return 0; }
    const char *start = p;
    while (p < end && *p != '\n' && *p != '\r') p++;
    size_t len = (size_t)(p - start);
    if (len >= cap) len = cap - 1;
    memcpy(buf, start, len);
    buf[len] = '\0';
    return len;
}

static size_t visual_col(const char *line, size_t byte_off)
{
    size_t vis = 0;
    for (size_t i = 0; i < byte_off; i++) {
        if (line[i] == '\t')
            vis += TAB_WIDTH - (vis % TAB_WIDTH);
        else
            vis++;
    }
    return vis;
}

static void write_expanded(FILE *sink, const char *line, size_t len)
{
    size_t col = 0;
    for (size_t i = 0; i < len; i++) {
        if (line[i] == '\t') {
            size_t sp = TAB_WIDTH - (col % TAB_WIDTH);
            for (size_t s = 0; s < sp; s++) fputc(' ', sink);
            col += sp;
        } else {
            fputc((unsigned char)line[i], sink);
            col++;
        }
    }
}

static void write_diag_code(FILE *sink, bool is_error, int code)
{
    if (strcmp(i18n_lang(), "fa") == 0) {
        static const char *const fa_digit[] = {
            "۰", "۱", "۲", "۳", "۴", "۵", "۶", "۷", "۸", "۹"
        };
        int c = ((code % 1000) + 1000) % 1000;   
        fprintf(sink, "[%s", i18n_tr(is_error ? "error" : "warning"));
        fputs(fa_digit[(c / 100) % 10], sink);
        fputs(fa_digit[(c / 10) % 10], sink);
        fputs(fa_digit[c % 10], sink);
        fputc(']', sink);
    } else {
        fprintf(sink, "[%c%03d]", is_error ? 'E' : 'W', code);
    }
}

static const char *level_color(const char *level)
{
    if (strcmp(level, "error")   == 0) return A_RED;
    if (strcmp(level, "warning") == 0) return A_YELLOW;
    if (strcmp(level, "note")    == 0) return A_CYAN;
    if (strcmp(level, "help")    == 0) return A_GREEN;
    return A_BOLD;
}

static void span_visuals(const char *line_buf, size_t line_len,
                         const src_span_t *span,
                         size_t *vis_s, size_t *vis_w)
{
    size_t byte_start = (span->begin.col > 0) ? (size_t)(span->begin.col - 1) : 0;
    if (byte_start > line_len) byte_start = line_len;
    size_t span_bytes = (span->length > 0) ? (size_t)span->length : 1;
    size_t byte_end   = byte_start + span_bytes;
    if (byte_end > line_len) byte_end = line_len;
    if (byte_end <= byte_start) byte_end = byte_start + 1;
    *vis_s = visual_col(line_buf, byte_start);
    size_t ve = visual_col(line_buf, byte_end);
    *vis_w = (ve > *vis_s) ? (ve - *vis_s) : 1;
}

static void write_gutter(FILE *sink, uint32_t lineno, int num_width, bool color)
{
    if (color) fputs(A_BBLUE, sink);
    if (lineno == 0)
        fprintf(sink, "%*s |", num_width, "");
    else
        fprintf(sink, "%*u |", num_width, lineno);
    if (color) fputs(A_RESET, sink);
}

void diag_render_rust(FILE *sink,
                      const char *level, int code, bool is_error,
                      const char *file, const src_span_t *span,
                      const char *src_text, size_t src_len,
                      const char *body, const char *help,
                      bool color)
{
    const char *lc    = color ? level_color(level) : "";
    const char *reset = color ? A_RESET : "";
    const char *blue  = color ? A_BLUE  : "";
    const char *bold  = color ? A_BOLD  : "";
    
    fprintf(sink, "%s%s", lc, i18n_tr(level));
    if (code > 0)
        write_diag_code(sink, is_error, code);
    fprintf(sink, "%s: %s%s%s\n", reset, bold, body, reset);
    if (!file) return;
    
    if (span && span->begin.line > 0)
        fprintf(sink, " %s-->%s %s:%u:%u\n",
                blue, reset, file, span->begin.line, span->begin.col);
    else {
        fprintf(sink, " %s-->%s %s\n", blue, reset, file);
        if (help)
            fprintf(sink, "   %s= %s:%s %s\n",
                    color ? A_GREEN : "", i18n_tr("help"), reset, help);
        return;
    }
    
    char     line_buf[1024];
    uint32_t lineno = span->begin.line;
    
    int num_width = 1;
    { uint32_t t = lineno; while (t >= 10) { t /= 10; num_width++; } }
    
    write_gutter(sink, 0, num_width, color);
    fputc('\n', sink);
    
    size_t line_len = 0;
    if (src_text)
        line_len = get_src_line(src_text, src_len, lineno,
                                line_buf, sizeof(line_buf));
    write_gutter(sink, lineno, num_width, color);
    fputc(' ', sink);
    if (src_text) write_expanded(sink, line_buf, line_len);
    fputc('\n', sink);
    
    write_gutter(sink, 0, num_width, color);
    fputc(' ', sink);
    if (src_text && span->begin.col > 0) {
        size_t vis_s, vis_w;
        span_visuals(line_buf, line_len, span, &vis_s, &vis_w);
        if (color) fputs(lc, sink);
        for (size_t i = 0; i < vis_s; i++) fputc(' ', sink);
        for (size_t i = 0; i < vis_w; i++) fputc('^', sink);
        if (color) fputs(reset, sink);
    }
    fputc('\n', sink);
    
    write_gutter(sink, 0, num_width, color);
    fputc('\n', sink);
    
    if (help) {
        write_gutter(sink, 0, num_width, color);
        fprintf(sink, " %s= %s:%s %s\n",
                color ? A_GREEN : "", i18n_tr("help"), reset, help);
    }
}

void diag_render_gcc(FILE *sink,
                     const char *level, int code, bool is_error,
                     const char *file, const src_span_t *span,
                     const char *src_text, size_t src_len,
                     const char *body, const char *help,
                     bool color)
{
    const char *lc    = color ? level_color(level) : "";
    const char *reset = color ? A_RESET : "";
    const char *bold  = color ? A_BOLD  : "";
    
    if (file && span && span->begin.line > 0)
        fprintf(sink, "%s%s:%u:%u:%s ", bold, file, span->begin.line, span->begin.col, reset);
    else if (file)
        fprintf(sink, "%s%s:%s ", bold, file, reset);
    fprintf(sink, "%s%s%s: %s", lc, i18n_tr(level), reset, body);
    if (code > 0) {
        fputc(' ', sink);
        write_diag_code(sink, is_error, code);
    }
    fputc('\n', sink);
    if (!src_text || !span || span->begin.line == 0) {
        if (help)
            fprintf(sink, "  %s%s:%s %s\n", color ? A_CYAN : "", i18n_tr("note"), reset, help);
        return;
    }
    
    char   line_buf[1024];
    size_t line_len = get_src_line(src_text, src_len, span->begin.line,
                                   line_buf, sizeof(line_buf));
    fputs("    ", sink);
    write_expanded(sink, line_buf, line_len);
    fputc('\n', sink);
    
    if (span->begin.col > 0) {
        size_t vis_s, vis_w;
        span_visuals(line_buf, line_len, span, &vis_s, &vis_w);
        fputs("    ", sink);
        if (color) fputs(lc, sink);
        for (size_t i = 0; i < vis_s; i++) fputc(' ', sink);
        fputc('^', sink);
        for (size_t i = 1; i < vis_w; i++) fputc('~', sink);
        if (color) fputs(reset, sink);
        fputc('\n', sink);
    }
    if (help)
        fprintf(sink, "  %s%s:%s %s\n", color ? A_CYAN : "", i18n_tr("note"), reset, help);
}

void diag_render_clang(FILE *sink,
                       const char *level, int code, bool is_error,
                       const char *file, const src_span_t *span,
                       const char *src_text, size_t src_len,
                       const char *body, const char *help,
                       bool color)
{
    const char *lc    = color ? level_color(level) : "";
    const char *reset = color ? A_RESET : "";
    const char *bold  = color ? A_BOLD  : "";
    
    if (file && span && span->begin.line > 0)
        fprintf(sink, "%s%s:%u:%u:%s ", bold, file, span->begin.line, span->begin.col, reset);
    else if (file)
        fprintf(sink, "%s%s:%s ", bold, file, reset);
    fprintf(sink, "%s%s%s: %s", lc, i18n_tr(level), reset, body);
    if (code > 0) {
        fputc(' ', sink);
        write_diag_code(sink, is_error, code);
    }
    fputc('\n', sink);
    if (!src_text || !span || span->begin.line == 0) {
        if (help)
            fprintf(sink, "  %s%s:%s %s\n", color ? A_CYAN : "", i18n_tr("note"), reset, help);
        return;
    }
    
    char   line_buf[1024];
    size_t line_len = get_src_line(src_text, src_len, span->begin.line,
                                   line_buf, sizeof(line_buf));
    fputs("    ", sink);
    write_expanded(sink, line_buf, line_len);
    fputc('\n', sink);
    size_t vis_s = 0, vis_w = 1;
    if (span->begin.col > 0)
        span_visuals(line_buf, line_len, span, &vis_s, &vis_w);
    
    fputs("    ", sink);
    if (color) fputs(lc, sink);
    for (size_t i = 0; i < vis_s; i++) fputc(' ', sink);
    fputc('^', sink);
    for (size_t i = 1; i < vis_w; i++) fputc('~', sink);
    if (color) fputs(reset, sink);
    fputc('\n', sink);
    
    fputs("    ", sink);
    if (color) fputs(lc, sink);
    for (size_t i = 0; i < vis_s; i++) fputc(' ', sink);
    fprintf(sink, "| %s", body);
    if (color) fputs(reset, sink);
    fputc('\n', sink);
    
    if (help) {
        fputs("    ", sink);
        for (size_t i = 0; i < vis_s; i++) fputc(' ', sink);
        fprintf(sink, "%s= %s%s\n", color ? A_GREEN : "", reset, help);
    }
}
