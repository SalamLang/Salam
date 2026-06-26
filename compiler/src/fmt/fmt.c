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

fmt_style_t fmt_style_default(void)
{
    fmt_style_t s; s.tabs = false; s.width = FMT_INDENT_WIDTH; return s;
}

static void fmt_emit_indent(sb_t *out, const fmt_style_t *st, int indent)
{
    if (indent < 0) indent = 0;
    if (st->tabs) {
        int s = 0; for (; s < indent; s++) sb_putc(out, '\t');
    } else {
        int s = 0; for (; s < indent * st->width; s++) sb_putc(out, ' ');
    }
}

#define FMT_MAX_BRACKET 256

static bool fmt_bracket_multiline(const token_stream_t *toks, size_t open_idx)
{
    const token_t *o = token_stream_at(toks, open_idx);
    size_t n = token_stream_count(toks);
    int depth = 0;
    size_t i = open_idx;
    for (; i < n; i++) {
        const token_t *t = token_stream_at(toks, i);
        if (t->kind == TK_EOF) break;
        if (fmt_is_open(t->kind)) depth++;
        else if (fmt_is_close(t->kind)) {
            if (--depth == 0)
                return t->span.end.line > o->span.begin.line;
        }
    }
    return false;
}

static bool fmt_next_is_if(const token_stream_t *toks, size_t i)
{
    size_t n = token_stream_count(toks);
    size_t j = i + 1;
    for (; j < n; j++) {
        token_kind_t kk = token_stream_at(toks, j)->kind;
        if (kk == TK_COMMENT_LINE || kk == TK_COMMENT_BLOCK) continue;
        return kk == TK_KW_IF;
    }
    return false;
}

void fmt_tokens(const token_stream_t *toks, const fmt_style_t *style, sb_t *out)
{
    fmt_style_t def = fmt_style_default();
    const fmt_style_t *st = style ? style : &def;
    size_t n = token_stream_count(toks);
    int  indent = 0;
    int  bracket = 0;
    int  angle = 0;
    int  bracket_indent = 0;        /* extra indent from open multi-line brackets */
    bool ml_stack[FMT_MAX_BRACKET]; /* per open bracket: is the group multi-line? */
    int  ml_top = 0;
    bool blk_stack[FMT_MAX_BRACKET];/* per `:`-block: is it a flattened `else: if`? */
    int  blk_top = 0;
    bool line_has_content = false;
    bool force_break = false;
    bool no_space_next = false;
    bool prev_gt_generic = false;   /* previous token was a generic-closing `>` */
    uint32_t open_colon_line = 0;
    const token_t *prev = NULL;
    bool     prev_unary = false;
    uint32_t prev_end_line = 0;
    { size_t i = 0; for (; i < n; i++) {
        const token_t *t = token_stream_at(toks, i);
        token_kind_t   k = t->kind;
        bool cur_ml = ml_top > 0 && ml_top <= FMT_MAX_BRACKET && ml_stack[ml_top - 1];
        if (k == TK_EOF) break;
        if (k == TK_STMT_END) {
            if (bracket > 0) {
                if (cur_ml) force_break = true;
                continue;
            }
            if (line_has_content && t->lexeme && t->lexeme[0] == ';') {
                sb_puts(out, "; ");
                no_space_next = true;
                prev = t; prev_end_line = t->span.end.line;
                continue;
            }
            if (line_has_content) { sb_putc(out, '\n'); line_has_content = false; }
            force_break = false;
            open_colon_line = 0;
            continue;
        }

        if (bracket > 0 && fmt_is_close(k) && cur_ml) {
            if (bracket_indent > 0) bracket_indent--;
            if (line_has_content) { sb_putc(out, '\n'); line_has_content = false; }
            force_break = false;
        }

        if (force_break && line_has_content) {
            sb_putc(out, '\n');
            line_has_content = false;
            force_break = false;
        }

        if (open_colon_line && line_has_content && t->span.begin.line > open_colon_line) {
            sb_putc(out, '\n');
            line_has_content = false;
        }
        open_colon_line = 0;

        if (bracket == 0 && angle == 0 && k == TK_KW_END) {
            bool elif = blk_top > 0 ? blk_stack[--blk_top] : false;
            if (!elif && indent > 0) indent--;
        } else if (bracket == 0 && angle == 0 && k == TK_KW_ELSE) {
            if (blk_top > 0) blk_top--;      /* the if-body block this else ends */
            if (indent > 0) indent--;
        }
        if (!line_has_content) {
            if (bracket == 0 && prev != NULL && t->span.begin.line > prev_end_line + 1)
                sb_putc(out, '\n');
            fmt_emit_indent(out, st, indent + bracket_indent);
        } else if (!no_space_next) {
            bool need = fmt_need_space(prev, t, prev_unary);
            if (prev_gt_generic && (k == TK_LPAREN || k == TK_LBRACKET))
                need = false;
            if (need) sb_putc(out, ' ');
        }
        no_space_next = false;
        prev_gt_generic = false;   /* re-armed below only for a generic `>` */
        if (k == TK_META) sb_putc(out, '@');
        sb_puts(out, t->lexeme ? t->lexeme : "");
        line_has_content = true;

        if (bracket == 0 && angle == 0 && k == TK_COLON) {
            bool elif = prev != NULL && prev->kind == TK_KW_ELSE &&
                        fmt_next_is_if(toks, i);
            if (blk_top < FMT_MAX_BRACKET) blk_stack[blk_top++] = elif;
            if (!elif) indent++;
            open_colon_line = t->span.end.line;
        }

        if (k == TK_LT && prev != NULL && fmt_is_value_end(prev->kind) &&
            !fmt_gap_in_source(prev, t))
            angle++;
        else if (k == TK_GT && angle > 0) {
            angle--;
            prev_gt_generic = true;   /* this `>` closed a generic argument list */
        }
        if (fmt_is_open(k)) {
            bool ml = fmt_bracket_multiline(toks, i);
            if (ml_top < FMT_MAX_BRACKET) ml_stack[ml_top] = ml;
            ml_top++;
            bracket++;
            if (ml) { bracket_indent++; force_break = true; }  /* break after opener */
        } else if (fmt_is_close(k)) {
            if (ml_top > 0) ml_top--;
            if (bracket > 0) bracket--;
        }
        prev_unary = fmt_is_prefix(k, prev);
        if (k == TK_COMMENT_LINE) force_break = true;
        prev = t;
        prev_end_line = t->span.end.line;
    } }
    if (line_has_content) sb_putc(out, '\n');       
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
