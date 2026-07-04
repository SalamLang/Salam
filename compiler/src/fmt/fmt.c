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
#include "fmt/fmt.h"
#include "fmt/fmt_internal.h"
#include "lexer/lexer.h"

#define FMT_MAX_BRACKET 256
#define FMT_MAX_BLOCK 256

typedef struct {
    const fmt_style_t *st;
    sb_t *out;
    int indent;
    int bracket;
    int angle;
    int bracket_indent;
    bool ml_stack[FMT_MAX_BRACKET];
    int ml_top;
    uint32_t block_line[FMT_MAX_BLOCK];
    int block_top;
    bool line_has_content;
    bool force_break;
    bool no_space_next;
    bool prev_gt_generic;
    uint32_t open_colon_line;
    const token_t *prev;
    bool prev_unary;
    uint32_t prev_end_line;
} fmt_ctx_t;

fmt_style_t fmt_style_default(void)
{
    fmt_style_t s;
    s.tabs = false;
    s.width = FMT_INDENT_WIDTH;
    return s;
}

static void fmt_emit_indent(sb_t *out, const fmt_style_t *st, int indent)
{
    if (indent < 0) indent = 0;
    if (st->tabs) {
        int s = 0;
        for (; s < indent; s++)
            sb_putc(out, '\t');
    } else {
        int s = 0;
        for (; s < indent * st->width; s++)
            sb_putc(out, ' ');
    }
}

static bool fmt_bracket_multiline(const token_stream_t *toks, size_t open_idx)
{
    const token_t *o = token_stream_at(toks, open_idx);
    size_t n = token_stream_count(toks);
    int depth = 0;
    size_t i = open_idx;
    for (; i < n; i++) {
        const token_t *t = token_stream_at(toks, i);
        if (t->kind == TK_EOF) break;
        if (fmt_is_open(t->kind))
            depth++;
        else if (fmt_is_close(t->kind)) {
            if (--depth == 0) return t->span.end.line > o->span.begin.line;
        }
    }
    return false;
}

/*
 * Decide whether a `<` at `lt_idx` opens a generic argument list (e.g.
 * `Vector<str>`, `HashMap<K, V>`) rather than being a less-than comparison
 * (`i < 2`). A generic closes with a matching `>` over type-like tokens only;
 * a comparison contains literals/operators or never closes before the
 * expression ends. Without this, an unspaced comparison such as `i<2` would be
 * mistaken for an open generic, leaving `angle` stuck > 0 and disabling block
 * indentation for the rest of the file.
 */
static bool fmt_angle_is_generic(const token_stream_t *toks, size_t lt_idx)
{
    size_t n = token_stream_count(toks);
    int depth = 0;
    int steps = 0;
    size_t i = lt_idx;
    for (; i < n && steps < 256; i++, steps++) {
        token_kind_t k = token_stream_at(toks, i)->kind;
        switch (k) {
        case TK_LT:
            depth++;
            break;
        case TK_GT:
            if (--depth == 0) return true;
            break;
        case TK_IDENT:
        case TK_COMMA:
        case TK_DOT:
        case TK_STAR:
        case TK_AMP:
        case TK_LBRACKET:
        case TK_RBRACKET:
            break;
        default:
            return false;
        }
    }
    return false;
}

static void fmt_step_stmt_end(fmt_ctx_t *c, const token_t *t, bool cur_ml)
{
    if (c->bracket > 0) {
        if (cur_ml) c->force_break = true;
        return;
    }
    if (c->line_has_content && t->lexeme && t->lexeme[0] == ';') {
        sb_puts(c->out, "; ");
        c->no_space_next = true;
        c->prev = t;
        c->prev_end_line = t->span.end.line;
        return;
    }
    if (c->line_has_content) {
        sb_putc(c->out, '\n');
        c->line_has_content = false;
    }
    c->force_break = false;
    c->open_colon_line = 0;
}

static void fmt_step_break_before(fmt_ctx_t *c, const token_t *t, token_kind_t k,
                                  bool cur_ml)
{
    if (c->bracket > 0 && fmt_is_close(k) && cur_ml) {
        if (c->bracket_indent > 0) c->bracket_indent--;
        if (c->line_has_content) {
            sb_putc(c->out, '\n');
            c->line_has_content = false;
        }
        c->force_break = false;
    }
    if (c->force_break && c->line_has_content) {
        sb_putc(c->out, '\n');
        c->line_has_content = false;
        c->force_break = false;
    }
    if (c->open_colon_line && c->line_has_content &&
        t->span.begin.line > c->open_colon_line) {
        sb_putc(c->out, '\n');
        c->line_has_content = false;
    }
    c->open_colon_line = 0;
}

static void fmt_step_dedent(fmt_ctx_t *c, const token_t *t, token_kind_t k)
{
    if (c->bracket == 0 && c->angle == 0 && (k == TK_KW_END || k == TK_KW_ELSE)) {
        if (c->block_top > 0 && c->block_top <= FMT_MAX_BLOCK && c->line_has_content &&
            t->span.begin.line > c->block_line[c->block_top - 1]) {
            sb_putc(c->out, '\n');
            c->line_has_content = false;
            c->force_break = false;
        }
        if (c->block_top > 0) c->block_top--;
        if (c->indent > 0) c->indent--;
    }
}

static void fmt_step_leading(fmt_ctx_t *c, const token_t *t, token_kind_t k)
{
    if (!c->line_has_content) {
        if (c->bracket == 0 && c->prev != NULL &&
            t->span.begin.line > c->prev_end_line + 1)
            sb_putc(c->out, '\n');
        fmt_emit_indent(c->out, c->st, c->indent + c->bracket_indent);
    } else if (!c->no_space_next) {
        bool need = fmt_need_space(c->prev, t, c->prev_unary);
        if (c->prev_gt_generic && (k == TK_LPAREN || k == TK_LBRACKET)) need = false;
        if (need) sb_putc(c->out, ' ');
    }
    c->no_space_next = false;
    c->prev_gt_generic = false;
}

static void fmt_step_state_after(fmt_ctx_t *c, const token_t *t, token_kind_t k,
                                 const token_stream_t *toks, size_t i)
{
    if (c->bracket == 0 && c->angle == 0 && k == TK_COLON) {
        c->indent++;
        c->open_colon_line = t->span.end.line;
        if (c->block_top < FMT_MAX_BLOCK)
            c->block_line[c->block_top] = t->span.begin.line;
        c->block_top++;
        if (c->prev != NULL && c->prev->kind == TK_KW_ELSE) c->force_break = true;
    }

    if (k == TK_LT && c->prev != NULL && fmt_is_value_end(c->prev->kind) &&
        !fmt_gap_in_source(c->prev, t) && fmt_angle_is_generic(toks, i))
        c->angle++;
    else if (k == TK_GT && c->angle > 0) {
        c->angle--;
        c->prev_gt_generic = true;
    }

    if (fmt_is_open(k)) {
        bool ml = fmt_bracket_multiline(toks, i);
        if (c->ml_top < FMT_MAX_BRACKET) c->ml_stack[c->ml_top] = ml;
        c->ml_top++;
        c->bracket++;
        if (ml) {
            c->bracket_indent++;
            c->force_break = true;
        }
    } else if (fmt_is_close(k)) {
        if (c->ml_top > 0) c->ml_top--;
        if (c->bracket > 0) c->bracket--;
    }

    c->prev_unary = fmt_is_prefix(k, c->prev);
    if (k == TK_COMMENT_LINE) c->force_break = true;
}

void fmt_tokens(const token_stream_t *toks, const fmt_style_t *style, sb_t *out)
{
    fmt_style_t def = fmt_style_default();
    fmt_ctx_t c;
    size_t n = token_stream_count(toks);
    size_t i = 0;

    memset(&c, 0, sizeof c);
    c.st = style ? style : &def;
    c.out = out;

    for (; i < n; i++) {
        const token_t *t = token_stream_at(toks, i);
        token_kind_t k = t->kind;
        bool cur_ml =
            c.ml_top > 0 && c.ml_top <= FMT_MAX_BRACKET && c.ml_stack[c.ml_top - 1];
        if (k == TK_EOF) break;
        if (k == TK_STMT_END) {
            fmt_step_stmt_end(&c, t, cur_ml);
            continue;
        }

        fmt_step_break_before(&c, t, k, cur_ml);
        fmt_step_dedent(&c, t, k);
        fmt_step_leading(&c, t, k);

        if (k == TK_META) sb_putc(out, '@');
        sb_puts(out, t->lexeme ? t->lexeme : "");
        c.line_has_content = true;

        fmt_step_state_after(&c, t, k, toks, i);
        c.prev = t;
        c.prev_end_line = t->span.end.line;
    }
    if (c.line_has_content) sb_putc(out, '\n');
}

bool fmt_source(arena_t *a, logger_t *log, const langpack_t *pack,
                const source_file_t *src, const fmt_style_t *style, sb_t *out)
{
    token_stream_t *toks = NULL;

    bool ok = lexer_run_ex(a, log, pack, src, true, &toks);
    if (!ok) return false;
    fmt_tokens(toks, style, out);
    return true;
}
