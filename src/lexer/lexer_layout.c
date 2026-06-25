#include "core/prelude.h"
#include "lexer/lexer_internal.h"

static void layout_close_block(lx_t *L)
{
    L->layout_depth--;
    if (L->layout_depth <= 0) {
        L->layout_mode = false;
        LOG_D(L->log, PH_LEXER, "exiting layout mode");
    }
}

static void layout_emit_punct(lx_t *L, token_kind_t k, const char *lexeme)
{
    src_pos_t cb = LX_POS(L);
    lx_adv(L);
    lx_emit(L, k, &cb, lexeme);
}

static void lx_layout_skip_inline_ws(lx_t *L)
{
    for (;;) {
        char c = lx_peek(L);
        if (c == ' ' || c == '\t' || c == '\r') { lx_adv(L); continue; }
        if (lx_try_line_comment(L)) continue;   
        break;
    }
}

static void layout_scan_quoted(lx_t *L)
{
    if (lx_peek2(L,1) == '"' && lx_peek2(L,2) == '"') lx_scan_triple(L);
    else                                              lx_scan_string(L);
}

static void layout_scan_func_value(lx_t *L)
{
    int depth = 0;
    for (;;) {
        lx_layout_skip_inline_ws(L);
        char d = lx_peek(L);
        if (d == '\0' || d == '\n') {
            src_pos_t eb = LX_POS(L);
            lx_error(L, &eb, "unterminated function value in layout");
            return;
        }
        if (d == '(') { depth++; lx_scan_op(L); continue; }
        if (d == ')') { depth--; lx_scan_op(L); if (depth <= 0) return; continue; }
        if (d == '"') { layout_scan_quoted(L); continue; }
        if (lx_at_digit(L, 0))    { lx_scan_number(L); continue; }
        if (lx_is_ident_start(d)) { lx_scan_ident(L);  continue; }
        lx_scan_op(L);                  
    }
}

static void lx_layout_value(lx_t *L)
{
    lx_layout_skip_inline_ws(L);
    char c = lx_peek(L);
    if (c == '\0' || c == '\n') {
        src_pos_t b = LX_POS(L);
        lx_error(L, &b, "layout attribute requires a value");
        return;
    }
    if (c == '"') { layout_scan_quoted(L);  return; }
    if (c == '`') { lx_scan_raw_string(L);  return; }
    if (lx_at_digit(L, 0)) { lx_scan_number(L); return; }
    if (lx_is_ident_start(c)) {
        lx_scan_ident(L);                          
        if (lx_peek(L) == '(') layout_scan_func_value(L);
        return;
    }
    lx_scan_op(L);                                 
}

static bool layout_try_end_inline(lx_t *L, const src_pos_t *b)
{
    const char *end_kw = langpack_end_spelling(L->pack);
    size_t klen = strlen(end_kw);
    const char *matched_kw = NULL;
    size_t matched_len = 0;
    if (strncmp(L->s + L->pos, end_kw, klen) == 0) {
        char after = lx_peek2(L, klen);
        if (after == ' ' || after == '\t' || after == '\r' || after == '\n' || after == '\0') {
            matched_kw = end_kw;
            matched_len = klen;
        }
    }
    if (!matched_kw && strcmp(end_kw, "end") != 0) {
        char c0 = lx_peek(L), c1 = lx_peek2(L,1), c2 = lx_peek2(L,2), c3 = lx_peek2(L,3);
        if (c0=='e' && c1=='n' && c2=='d' &&
            (c3==' '||c3=='\t'||c3=='\r'||c3=='\n'||c3=='\0')) {
            matched_kw = "end";
            matched_len = 3;
        }
    }
    if (!matched_kw) return false;
    for (size_t i = 0; i < matched_len; i++) lx_adv(L);
    lx_emit(L, TK_KW_END, b, matched_kw);
    layout_close_block(L);
    return true;
}

static void layout_report_bad_name(lx_t *L, const src_pos_t *b,
                                   const char *name, size_t namelen)
{
    size_t fw = 0;
    while (fw < namelen && name[fw] != ' ' && name[fw] != '\t') fw++;
    if (fw == 3 &&
        (name[0]=='E'||name[0]=='e') &&
        (name[1]=='N'||name[1]=='n') &&
        (name[2]=='D'||name[2]=='d'))
        lx_error(L, b, "'end' keyword must be lowercase");
    else
        lx_error(L, b, "expected ':' or '=' after layout name");
}

static void lx_layout_name(lx_t *L)
{
    src_pos_t b = LX_POS(L);
    if (layout_try_end_inline(L, &b)) return;
    size_t start = L->pos;
    while (!lx_end(L)) {
        char c = lx_peek(L);
        if (c == ':' || c == '=' || c == '\n') break;
        lx_adv(L);
    }
    size_t end = lx_trim_ws_end(L->s, start, L->pos);   
    size_t namelen = end - start;
    const char *name = arena_strndup(L->a, L->s + start, namelen);
    char term = lx_peek(L);
    if (namelen == 0) {                                 
        if      (term == ':') layout_emit_punct(L, TK_COLON, ":");
        else if (term == '=') { layout_emit_punct(L, TK_ASSIGN, "="); L->layout_sub = LX_SUB_VALUE; }
        return;
    }
    if ((term == '\n' || term == '\0') &&
        langpack_lookup_keyword(L->pack, name) == TK_KW_END) {
        lx_emit(L, TK_KW_END, &b, name);
        layout_close_block(L);
        return;
    }
    if (term == ':') {
        lx_emit(L, TK_LAYOUT_ELEMENT, &b, name);
        layout_emit_punct(L, TK_COLON, ":");
        L->layout_depth++;
        L->layout_sub = LX_SUB_NAME;
        return;
    }
    if (term == '=') {
        lx_emit(L, TK_LAYOUT_ATTR_NAME, &b, name);
        layout_emit_punct(L, TK_ASSIGN, "=");
        L->layout_sub = LX_SUB_VALUE;
        return;
    }
    layout_report_bad_name(L, &b, name, namelen);
    lx_emit(L, TK_LAYOUT_ELEMENT, &b, name);
}

void lx_layout_step(lx_t *L)
{
    lx_layout_skip_inline_ws(L);
    char c = lx_peek(L);
    if (c == '\0') {
        
        if (!lx_end(L)) lx_adv(L);
        return;
    }
    if (c == '\n') {                            
        src_pos_t b = LX_POS(L);
        lx_adv(L);
        lx_emit(L, TK_STMT_END, &b, "\\n");
        L->layout_sub = LX_SUB_NAME;
        return;
    }
    if (L->layout_sub == LX_SUB_VALUE) {
        lx_layout_value(L);
        L->layout_sub = LX_SUB_NAME;
        return;
    }
    lx_layout_name(L);
}
