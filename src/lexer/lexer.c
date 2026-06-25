#include "core/prelude.h"
#include "lexer/lexer.h"
#include "lexer/lexer_internal.h"
#include "i18n/i18n.h"

static void lx_init(lx_t *L, arena_t *a, logger_t *log, const langpack_t *pack,
                    const source_file_t *src, bool keep_comments)
{
    memset(L, 0, sizeof(*L));
    L->s = src->text;
    L->n = src->len;
    L->line = 1;
    L->col = 1;
    L->file = src->path;
    L->last = TK_EOF;
    L->layout_sub = LX_SUB_NAME;
    L->keep_comments = keep_comments;
    L->a = a;
    L->log = log;
    L->pack = pack;
    L->out = token_stream_new(a, src->path);
}

static void lx_scan_one(lx_t *L)
{
    char c = lx_peek(L);
    if (lx_at_digit(L, 0))                                       lx_scan_number(L);

    else if (c == '.' && lx_at_digit(L, 1))                      lx_scan_number(L);
    else if (c == 'u' && lx_peek2(L, 1) == '\'')                  lx_scan_utf8_char(L);
    else if (lx_is_ident_start(c))                               lx_scan_ident(L);
    else if (c == '"') {
        if (lx_peek2(L,1) == '"' && lx_peek2(L,2) == '"')       lx_scan_triple(L);
        else                                                    lx_scan_string(L);
    }
    else if (c == '\'')                                          lx_scan_char(L);
    else if (c == '`')                                          lx_scan_raw_string(L);
    else if (c == '@' && lx_is_ident_start(lx_peek2(L,1)))       lx_scan_meta(L);
    else                                                        lx_scan_op(L);
}

static void lx_emit_eof(lx_t *L)
{
    token_t t;
    t.kind = TK_EOF;
    t.lexeme = "";
    t.layout_mode = false;
    t.value.kind = TV_NONE;
    t.span.begin = LX_POS(L); t.span.end = t.span.begin; t.span.length = 0;
    token_stream_push(L->a, L->out, &t);
}

bool lexer_run(arena_t *a, logger_t *log, const langpack_t *pack,
               const source_file_t *src, token_stream_t **out)
{
    return lexer_run_ex(a, log, pack, src, false, out);
}

bool lexer_run_ex(arena_t *a, logger_t *log, const langpack_t *pack,
                  const source_file_t *src, bool keep_comments,
                  token_stream_t **out)
{
    lx_t L;
    lx_init(&L, a, log, pack, src, keep_comments);
    LOG_D(log, PH_LEXER, "lexing %s (%zu bytes, lang=%s)",
          src->path, src->len, langpack_code(pack));
    for (;;) {
        if (L.layout_mode) {
            if (lx_end(&L)) break;
            lx_layout_step(&L);
            continue;
        }
        lx_skip_trivia(&L);
        if (lx_end(&L)) break;
        lx_scan_one(&L);
    }
    if (L.layout_mode)
        LOG_W(log, PH_LEXER, "unclosed layout block at end of file");
    lx_newline_terminator(&L);
    lx_emit_eof(&L);
    LOG_D(log, PH_LEXER, "lexing complete: %zu tokens%s",
          token_stream_count(L.out), L.had_error ? i18n_tr(" (with errors)") : "");
    *out = L.out;
    return !L.had_error;
}
