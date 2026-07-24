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
    bool paren_is_lambda[FMT_MAX_BRACKET];
    bool prev_close_was_lambda;
    int ml_top;
    uint32_t block_line[FMT_MAX_BLOCK];
    bool in_match_arms[FMT_MAX_BLOCK];
    int block_top;
    bool line_has_content;
    bool force_break;
    bool no_space_next;
    bool prev_gt_generic;
    int q_open[FMT_MAX_BRACKET + 1];
    token_kind_t stmt_head;
    bool match_pending;
    uint32_t open_colon_line;
    const token_t *prev;
    bool prev_unary;
    bool prev_after_dot;
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
        case TK_SHR:
            /* '>>' closes two generic levels at once (e.g. 'A<B<C>>'). */
            depth -= 2;
            if (depth <= 0) return true;
            break;
        case TK_IDENT:
        case TK_COMMA:
        case TK_DOT:
        case TK_STAR:
        case TK_AMP:
        case TK_LBRACKET:
        case TK_RBRACKET:
        case TK_COLON:
            break;
        default:
            return false;
        }
    }
    return false;
}

/* Every lambda parameter is mandatorily typed ('name: Type'), so a '('
 * only opens a lambda header if its contents fully match that shape. Used
 * to tell a lambda's parameter list apart from an unrelated grouped
 * expression that simply happens to sit in front of a ':' (e.g. an
 * 'if (a) || (b):' condition). */
static bool fmt_lambda_header_at(const token_stream_t *toks, size_t open_idx)
{
    size_t n = token_stream_count(toks);
    size_t i = open_idx + 1;
    if (i < n && token_stream_at(toks, i)->kind == TK_RPAREN) return true;
    for (;;) {
        if (i >= n || token_stream_at(toks, i)->kind != TK_IDENT) return false;
        i++;
        if (i >= n || token_stream_at(toks, i)->kind != TK_COLON) return false;
        i++;
        int depth = 0;
        for (;;) {
            if (i >= n) return false;
            token_kind_t k = token_stream_at(toks, i)->kind;
            if (k == TK_EOF) return false;
            if (k == TK_LT && fmt_angle_is_generic(toks, i)) {
                depth++;
            } else if (k == TK_GT && depth > 0) {
                depth--;
            } else if (k == TK_SHR && depth > 0) {
                depth -= 2;
                if (depth < 0) depth = 0;
            } else if (fmt_is_open(k)) {
                depth++;
            } else if (fmt_is_close(k)) {
                if (depth == 0) return k == TK_RPAREN;
                depth--;
            } else if (k == TK_COMMA && depth == 0) {
                i++;
                break;
            }
            i++;
        }
    }
}

static bool fmt_head_modifier(token_kind_t k)
{
    return k == TK_KW_PUB || k == TK_KW_INLINE || k == TK_KW_NOINLINE ||
           k == TK_KW_PURE || k == TK_KW_NORET || k == TK_KW_DEPRECATED;
}

static bool fmt_is_overload_symbol(token_kind_t k)
{
    switch (k) {
    case TK_GT:
    case TK_PLUS:
    case TK_MINUS:
    case TK_STAR:
    case TK_SLASH:
    case TK_PERCENT:
    case TK_POWER:
    case TK_EQ:
    case TK_NE:
    case TK_LT:
    case TK_LE:
    case TK_GE:
    case TK_NOT:
        return true;
    default:
        return false;
    }
}

static bool fmt_head_annotates(token_kind_t k)
{
    return k == TK_IDENT || k == TK_KW_MUT || k == TK_KW_CONST || k == TK_KW_THIS;
}

static bool fmt_type_token(token_kind_t k)
{
    switch (k) {
    case TK_IDENT:
    case TK_DOT:
    case TK_STAR:
    case TK_AMP:
    case TK_LT:
    case TK_GT:
    case TK_SHR:
    case TK_LBRACKET:
    case TK_RBRACKET:
    case TK_LPAREN:
    case TK_RPAREN:
    case TK_COMMA:
    case TK_INT:
    case TK_KW_FUNC:
        return true;
    default:
        return false;
    }
}

static bool fmt_colon_is_annotation(const token_stream_t *toks, size_t colon_idx)
{
    size_t n = token_stream_count(toks);
    uint32_t line = token_stream_at(toks, colon_idx)->span.end.line;
    bool saw_type = false;
    bool is_func_type = false;
    int steps = 0;
    size_t j = colon_idx + 1;
    for (; j < n && steps < 64; j++, steps++) {
        const token_t *t = token_stream_at(toks, j);
        if (t->span.begin.line > line) return saw_type;
        if (t->kind == TK_COLON || t->kind == TK_STMT_END || t->kind == TK_EOF)
            return saw_type;
        if (t->kind == TK_KW_FUNC && !saw_type) is_func_type = true;
        /* salam's type grammar has no use for '(' except a function-pointer
         * type's own parameter list ('func(int): bool'); anywhere else a
         * '(' means this is actually a call expression (e.g. 'f(a)' or
         * 'x.f(a)', like a same-line if-branch body: 'if x: f(a, b)') and
         * this colon isn't a type annotation at all. */
        if (t->kind == TK_LPAREN && !is_func_type) return false;
        if (!fmt_type_token(t->kind)) return false;
        saw_type = true;
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
        c->stmt_head = TK_EOF;
        return;
    }
    if (c->line_has_content) {
        sb_putc(c->out, '\n');
        c->line_has_content = false;
    }
    c->force_break = false;
    c->open_colon_line = 0;
    c->q_open[0] = 0;
    c->stmt_head = TK_EOF;
    c->match_pending = false;
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
    if (c->bracket == 0 && c->prev_gt_generic && c->line_has_content &&
        t->span.begin.line > c->prev_end_line) {
        sb_putc(c->out, '\n');
        c->line_has_content = false;
        c->force_break = false;
        c->q_open[0] = 0;
        c->stmt_head = TK_EOF;
        c->match_pending = false;
    }
}

static bool fmt_in_match_arms(const fmt_ctx_t *c)
{
    return c->block_top > 0 && c->block_top <= FMT_MAX_BLOCK &&
           c->in_match_arms[c->block_top - 1];
}

static void fmt_step_dedent(fmt_ctx_t *c, const token_t *t, token_kind_t k,
                            const token_stream_t *toks, size_t i)
{
    (void)toks;
    (void)i;
    if (k == TK_KW_ELSE && fmt_in_match_arms(c)) return;
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
        /* A name right after '.' is always a member/method reference, even
         * if it happens to collide with a statement keyword (e.g. the
         * 'repeat' loop keyword doubling as a method name in 's.repeat(n)')
         * - it should never get keyword-style spacing before '('. */
        if (k == TK_LPAREN && c->prev_after_dot) need = false;
        if (k == TK_COLON && c->bracket <= FMT_MAX_BRACKET && c->q_open[c->bracket] > 0)
            need = true;
        if (need) sb_putc(c->out, ' ');
    }
    c->no_space_next = false;
    c->prev_gt_generic = false;
}

static void fmt_step_state_after(fmt_ctx_t *c, const token_t *t, token_kind_t k,
                                 const token_stream_t *toks, size_t i)
{
    if (c->bracket == 0 && k == TK_KW_END)
        c->stmt_head = TK_EOF;
    else if (c->bracket == 0 && k == TK_KW_ELSE)
        c->stmt_head = TK_KW_ELSE;
    if (c->stmt_head == TK_EOF && c->bracket == 0 && k != TK_KW_END && k != TK_COLON &&
        k != TK_COMMENT_LINE && k != TK_COMMENT_BLOCK && !fmt_head_modifier(k))
        c->stmt_head = k;

    if (k == TK_KW_MATCH && c->bracket == 0) c->match_pending = true;

    if (k == TK_QUESTION && c->bracket <= FMT_MAX_BRACKET) c->q_open[c->bracket]++;
    if (k == TK_COLON && c->bracket <= FMT_MAX_BRACKET && c->q_open[c->bracket] > 0) {
        c->q_open[c->bracket]--;
    } else if (c->bracket == 0 && c->angle == 0 && k == TK_COLON) {
        bool opener;
        bool was_match_subject = c->match_pending;
        bool was_in_match_arms = fmt_in_match_arms(c);
        if (c->prev != NULL && c->prev->kind == TK_RPAREN && c->prev_close_was_lambda)
            opener = true;
        else if (was_match_subject)
            opener = true;
        else if (was_in_match_arms)
            opener = true;
        else if (fmt_head_annotates(c->stmt_head))
            opener = false;
        else
            opener = !fmt_colon_is_annotation(toks, i);
        if (opener) {
            c->indent++;
            c->open_colon_line = t->span.end.line;
            if (c->block_top < FMT_MAX_BLOCK) {
                c->block_line[c->block_top] = t->span.begin.line;
                c->in_match_arms[c->block_top] = was_match_subject;
            }
            c->block_top++;
            if (c->prev != NULL && c->prev->kind == TK_KW_ELSE && !was_in_match_arms)
                c->force_break = true;
            c->stmt_head = TK_EOF;
            c->match_pending = false;
        }
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
        bool prev_is_value =
            c->prev != NULL &&
            (fmt_is_value_end(c->prev->kind) || fmt_is_overload_symbol(c->prev->kind));
        bool is_lambda =
            k == TK_LPAREN && !prev_is_value && fmt_lambda_header_at(toks, i);
        if (c->ml_top < FMT_MAX_BRACKET) {
            c->ml_stack[c->ml_top] = ml;
            c->paren_is_lambda[c->ml_top] = is_lambda;
        }
        c->ml_top++;
        c->bracket++;
        if (c->bracket <= FMT_MAX_BRACKET) c->q_open[c->bracket] = 0;
        if (ml) {
            c->bracket_indent++;
            c->force_break = true;
        }
    } else if (fmt_is_close(k)) {
        c->prev_close_was_lambda = k == TK_RPAREN && c->ml_top > 0 &&
                                   c->ml_top <= FMT_MAX_BRACKET &&
                                   c->paren_is_lambda[c->ml_top - 1];
        if (c->ml_top > 0) c->ml_top--;
        if (c->bracket > 0) {
            if (c->bracket <= FMT_MAX_BRACKET) c->q_open[c->bracket] = 0;
            c->bracket--;
        }
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
        fmt_step_dedent(&c, t, k, toks, i);
        fmt_step_leading(&c, t, k);

        if (k == TK_META) sb_putc(out, '@');
        sb_puts(out, t->lexeme ? t->lexeme : "");
        c.line_has_content = true;

        fmt_step_state_after(&c, t, k, toks, i);
        c.prev_after_dot = c.prev != NULL && c.prev->kind == TK_DOT;
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
