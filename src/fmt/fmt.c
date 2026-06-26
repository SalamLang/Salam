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
    if (st->tabs) {
        int s = 0; for (; s < indent; s++) sb_putc(out, '\t');
    } else {
        int s = 0; for (; s < indent * st->width; s++) sb_putc(out, ' ');
    }
}

void fmt_tokens(const token_stream_t *toks, const fmt_style_t *style, sb_t *out)
{
    fmt_style_t def = fmt_style_default();
    const fmt_style_t *st = style ? style : &def;
    size_t n = token_stream_count(toks);
    int  indent = 0;          
    int  bracket = 0;         
    int  angle = 0;           
    bool line_has_content = false;
    bool force_break = false; 
    bool no_space_next = false;   
    bool in_for_header = false;   
    uint32_t open_colon_line = 0; 
    const token_t *prev = NULL;
    bool     prev_unary = false;
    uint32_t prev_end_line = 0;
    { size_t i = 0; for (; i < n; i++) {
        const token_t *t = token_stream_at(toks, i);
        token_kind_t   k = t->kind;
        if (k == TK_EOF) break;
        if (k == TK_STMT_END) {
            if (bracket > 0) continue;            
            
            if (in_for_header && line_has_content &&
                t->lexeme && t->lexeme[0] == ';') {
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
        
        if (bracket == 0 && angle == 0 && (k == TK_KW_END || k == TK_KW_ELSE)) {
            if (indent > 0) indent--;
        }
        if (!line_has_content) {
            if (prev != NULL && t->span.begin.line > prev_end_line + 1)
                sb_putc(out, '\n');
            fmt_emit_indent(out, st, indent);
        } else if (!no_space_next && fmt_need_space(prev, t, prev_unary)) {
            sb_putc(out, ' ');
        }
        no_space_next = false;
        if (k == TK_KW_FOR && bracket == 0 && angle == 0) in_for_header = true;
        if (k == TK_META) sb_putc(out, '@');       
        sb_puts(out, t->lexeme ? t->lexeme : "");
        line_has_content = true;
        
        if (bracket == 0 && angle == 0 && k == TK_COLON) {
            indent++;
            open_colon_line = t->span.end.line;
            in_for_header = false;        
        }
        
        if (k == TK_LT && prev != NULL && fmt_is_value_end(prev->kind) &&
            !fmt_gap_in_source(prev, t))
            angle++;
        else if (k == TK_GT && angle > 0)
            angle--;
        if (fmt_is_open(k)) bracket++;
        else if (fmt_is_close(k)) { if (bracket > 0) bracket--; }
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
