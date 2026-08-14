/*
 * Salam Programming Language (2024-2026)
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

/* What a REPL session remembers between turns, and how it turns that back
 * into one compilable program.
 *
 * The prompt is not a file: definitions arrive in whatever order the user
 * thinks of them, statements accumulate one at a time, and a name typed
 * twice means "I changed my mind", not "duplicate symbol". This file holds
 * that model; repl.c drives it and repl_line.c reads the keys. */

#include "driver/repl_internal.h"
#include "core/sb.h"

#define REPL_NAME_MAX 128

void repl_session_init(repl_session_t *s)
{
    memset(s, 0, sizeof *s);
}

void repl_session_free(repl_session_t *s)
{
    {
        size_t i = 0;
        for (; i < s->ndefs; i++) {
            free(s->defs[i].name);
            free(s->defs[i].text);
        }
    }
    free(s->defs);
    {
        size_t i = 0;
        for (; i < s->nstmts; i++)
            free(s->stmts[i]);
    }
    free(s->stmts);
    memset(s, 0, sizeof *s);
}

static const char *skip_ws(const char *p)
{
    while (*p == ' ' || *p == '\t')
        p++;
    return p;
}

/* Identifier bytes, non-ASCII included: fa/ar keywords and identifiers are
 * UTF-8, and every continuation byte has the high bit set. */
static bool ident_byte(char c)
{
    unsigned char u = (unsigned char)c;
    return u >= 0x80 || u == '_' || (u >= '0' && u <= '9') || (u >= 'a' && u <= 'z') ||
           (u >= 'A' && u <= 'Z');
}

static size_t word_len(const char *p)
{
    size_t n = 0;
    while (ident_byte(p[n]))
        n++;
    return n;
}

static void copy_trunc(char *dst, size_t cap, const char *src, size_t n)
{
    if (n >= cap) n = cap - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/* The whole first line, trimmed - the fallback name for a block whose
 * subject cannot be read off its opener (`impl X on Y`, an extern block,
 * an import). Retyping it verbatim then replaces it instead of colliding;
 * a different one lives alongside it. */
static void line_as_name(char *name, size_t namesz, const char *line)
{
    const char *p = skip_ws(line);
    size_t n = strcspn(p, "\r\n");
    while (n > 0 && (p[n - 1] == ' ' || p[n - 1] == '\t'))
        n--;
    copy_trunc(name, namesz, p, n);
}

/* A definition's subject: the identifier after the keyword, dots kept so
 * two members of one type stay distinct. */
static void subject_name(char *name, size_t namesz, const char *after_kw)
{
    const char *p = skip_ws(after_kw);
    size_t n = 0;
    while (ident_byte(p[n]) || (p[n] == '.' && ident_byte(p[n + 1])))
        n++;
    copy_trunc(name, namesz, p, n);
}

static token_kind_t word_kind(const langpack_t *pack, const char *p, size_t n)
{
    char buf[REPL_NAME_MAX];
    copy_trunc(buf, sizeof buf, p, n);
    return langpack_lookup_keyword(pack, buf);
}

static bool is_modifier(token_kind_t k)
{
    return k == TK_KW_PUB || k == TK_KW_INLINE || k == TK_KW_NOINLINE ||
           k == TK_KW_PURE || k == TK_KW_NORET || k == TK_KW_DEPRECATED;
}

static bool names_its_subject(token_kind_t k)
{
    return k == TK_KW_FUNC || k == TK_KW_STRUCT || k == TK_KW_ENUM || k == TK_KW_TYPE ||
           k == TK_KW_INTERFACE || k == TK_KW_COMPONENT || k == TK_KW_CONST;
}

static repl_kind_t kind_of_keyword(token_kind_t k)
{
    switch (k) {
    case TK_KW_PACKAGE:
        return RS_PKG;
    case TK_KW_IMPORT:
        return RS_IMPORT;
    case TK_KW_EXTERN:
        return RS_EXTERN;
    case TK_KW_CONST:
        return RS_GLOBAL;
    case TK_KW_STRUCT:
    case TK_KW_ENUM:
    case TK_KW_TYPE:
    case TK_KW_INTERFACE:
    case TK_KW_COMPONENT:
    case TK_KW_IMPL:
        return RS_TYPE;
    case TK_KW_FUNC:
    case TK_KW_OPERATOR:
        return RS_DEF;
    default:
        return RS_STMT;
    }
}

repl_kind_t repl_classify(const langpack_t *pack, const char *line, char *name,
                          size_t namesz)
{
    const char *p = skip_ws(line);
    name[0] = '\0';
    if (p[0] == '/' && (p[1] == '/' || p[1] == '*')) {
        line_as_name(name, namesz, p);
        return RS_DEF;
    }
    for (;;) {
        size_t n = word_len(p);
        token_kind_t k;
        repl_kind_t kind;
        if (n == 0) return RS_STMT;
        k = word_kind(pack, p, n);
        if (is_modifier(k)) {
            p = skip_ws(p + n);
            continue;
        }
        /* `link "..."` has no token of its own - it is parsed off the
         * identifier - so it is matched by spelling, English only. */
        if (k == TK_IDENT && n == 4 && strncmp(p, "link", 4) == 0) {
            line_as_name(name, namesz, line);
            return RS_IMPORT;
        }
        kind = kind_of_keyword(k);
        if (kind == RS_STMT) return RS_STMT;
        if (names_its_subject(k))
            subject_name(name, namesz, p + n);
        else if (k == TK_KW_PACKAGE)
            copy_trunc(name, namesz, "package", 7);
        else
            line_as_name(name, namesz, line);
        if (name[0] == '\0') line_as_name(name, namesz, line);
        return kind;
    }
}

/* An `=` that assigns, as opposed to one inside ==, !=, <=, >=, +=, and
 * friends. Struct literals (`Vec2 { x = 1 }`) count as assignment here,
 * which only costs them the echo. */
static bool has_assignment(const char *p)
{
    const char *q = p;
    for (; *q; q++) {
        char prev = (q == p) ? ' ' : q[-1];
        if (*q != '=' || q[1] == '=') continue;
        if (strchr("=!<>+-*/%&|^:~", prev)) continue;
        return true;
    }
    return false;
}

bool repl_is_expr_line(const langpack_t *pack, const char *line)
{
    const char *p = skip_ws(line);
    size_t n = word_len(p);
    size_t nl = strcspn(p, "\n");
    if (p[nl] == '\n' && skip_ws(p + nl + 1)[0] != '\0') return false;
    /* n == 0 is fine: an expression can open with a literal or a sign
     * (`"a" + "b"`, `-x`, `(1 + 2) * 3`). Only a keyword opener rules it
     * out - that is a statement, whatever follows. */
    if (p[0] == '\0' || (p[0] == '/' && (p[1] == '/' || p[1] == '*'))) return false;
    if (n > 0 && tk_is_keyword(word_kind(pack, p, n))) return false;
    return !has_assignment(p);
}

static void defs_push(repl_session_t *s, repl_kind_t kind, const char *name,
                      const char *text)
{
    if (s->ndefs == s->capdefs) {
        s->capdefs = s->capdefs ? s->capdefs * 2 : 8;
        s->defs = (repl_entry_t *)realloc(s->defs, s->capdefs * sizeof *s->defs);
    }
    s->defs[s->ndefs].kind = kind;
    s->defs[s->ndefs].name = sal_strdup(name);
    s->defs[s->ndefs].text = sal_strdup(text);
    s->ndefs++;
}

/* Does the session still define an entry function after this change? */
static void refresh_has_main(repl_session_t *s, const langpack_t *pack)
{
    const char *entry = langpack_entry(pack);
    size_t i = 0;
    s->has_main = false;
    for (; i < s->ndefs; i++)
        if (s->defs[i].kind == RS_DEF && strcmp(s->defs[i].name, entry) == 0)
            s->has_main = true;
}

void repl_session_add(repl_session_t *s, const langpack_t *pack, repl_kind_t kind,
                      const char *name, const char *text)
{
    size_t i = 0;
    for (; i < s->ndefs; i++) {
        if (strcmp(s->defs[i].name, name) != 0) continue;
        free(s->defs[i].text);
        s->defs[i].kind = kind;
        s->defs[i].text = sal_strdup(text);
        refresh_has_main(s, pack);
        return;
    }
    defs_push(s, kind, name, text);
    refresh_has_main(s, pack);
}

void repl_session_add_stmt(repl_session_t *s, const char *text)
{
    if (s->nstmts == s->capstmts) {
        s->capstmts = s->capstmts ? s->capstmts * 2 : 8;
        s->stmts = (char **)realloc(s->stmts, s->capstmts * sizeof *s->stmts);
    }
    s->stmts[s->nstmts++] = sal_strdup(text);
}

bool repl_session_undo(repl_session_t *s)
{
    if (s->nstmts == 0) return false;
    free(s->stmts[--s->nstmts]);
    return true;
}

static void put_block(sb_t *out, const char *text)
{
    size_t n;
    if (!text || !text[0]) return;
    sb_puts(out, text);
    n = strlen(text);
    if (text[n - 1] != '\n') sb_putc(out, '\n');
}

static bool entry_is_pub(const langpack_t *pack, const char *text)
{
    const char *p = skip_ws(text);
    size_t n = word_len(p);
    return n > 0 && word_kind(pack, p, n) == TK_KW_PUB;
}

/* `pubsel` selects a pass over one bucket: -1 for everything, 0 for the
 * private entries, 1 for the pub ones. Functions are emitted in two
 * passes because a private function may not follow a pub one (E088), and
 * the prompt has no business making the user care. */
static void put_bucket(sb_t *out, const repl_session_t *s, const langpack_t *pack,
                       repl_kind_t kind, const repl_entry_t *cand, int pubsel)
{
    size_t i = 0;
    for (; i < s->ndefs; i++) {
        if (s->defs[i].kind != kind) continue;
        if (pubsel >= 0 && entry_is_pub(pack, s->defs[i].text) != (pubsel == 1)) continue;
        if (cand && cand->name && strcmp(s->defs[i].name, cand->name) == 0) continue;
        put_block(out, s->defs[i].text);
    }
    if (cand && cand->kind == kind &&
        (pubsel < 0 || entry_is_pub(pack, cand->text) == (pubsel == 1)))
        put_block(out, cand->text);
}

char *repl_render(const repl_session_t *s, const langpack_t *pack,
                  const repl_entry_t *cand)
{
    const char *kw_func = langpack_spelling(pack, TK_KW_FUNC);
    sb_t out;
    int k = 0;
    sb_init(&out);
    for (; k < RS_DEF; k++)
        put_bucket(&out, s, pack, (repl_kind_t)k, cand, -1);
    put_bucket(&out, s, pack, RS_DEF, cand, 0);
    put_bucket(&out, s, pack, RS_DEF, cand, 1);
    if (!s->has_main) {
        const char *kw_ret = langpack_spelling(pack, TK_KW_RET);
        bool empty = s->nstmts == 0 && !(cand && cand->kind == RS_STMT);
        size_t i = 0;
        sb_printf(&out, "%s %s:\n", kw_func ? kw_func : "func", langpack_entry(pack));
        for (; i < s->nstmts; i++)
            put_block(&out, s->stmts[i]);
        if (cand && cand->kind == RS_STMT) put_block(&out, cand->text);
        /* A function with an empty body is an error in this language, and
         * a session that is all definitions so far has nothing to put in
         * main yet. `ret 0` is what main returns anyway. */
        if (empty) sb_printf(&out, "%s 0\n", kw_ret ? kw_ret : "ret");
        sb_printf(&out, "%s\n", langpack_end_spelling(pack));
    }
    return out.data ? out.data : sal_strdup("");
}
