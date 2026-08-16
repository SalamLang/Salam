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

/*
 * The Salam source of a derived JSON encoder. See sema_derive_json.c for why
 * this is generated text rather than hand-built AST.
 *
 * What a value encodes to:
 *
 *   struct        {"x":10,"y":15}
 *   vector/array  [1,2,3]
 *   hashmap       {"a":1}          (string or integer keys)
 *   enum          "Red"            (the member's name, its number if unnamed)
 *   pointer       null, or the pointee
 *   char          "x"              (a one-character string)
 *   f64 inf/nan   null             (JSON cannot spell either)
 *
 * A struct that defines `to_json(): str` gets that called instead, so a type
 * with a wire format of its own - a date, a money amount, an id - can present
 * itself however it likes.
 */

#include "core/prelude.h"
#include "semantic/sema_derive_core.h"
#include "semantic/sema_derive_json_internal.h"

static bool enc_value(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *expr,
                      const char *ind, const json_attr_t *at, scope_t *home,
                      const src_span_t *span);

static void add(sb_t *b, const char *ind, const char *arg)
{
    sb_printf(b, "%s_dadd(" JV_BUF ", %s)\n", ind, arg);
}

bool jd_is_unsigned(const type_t *t)
{
    switch (t->kind) {
    case TY_U8:
    case TY_U16:
    case TY_U32:
    case TY_U64:
    case TY_SIZE:
        return true;
    default:
        return false;
    }
}

/* The scalar conversion for `t`, as a call on `expr`, or NULL when the type
 * is not one the runtime layer has a direct spelling for. */
static const char *scalar_call(sema_t *s, type_t *t, const char *expr)
{
    if (t->kind == TY_STR || t->kind == TY_UCHAR) return jd_fmt(s, "_dqs(%s)", expr);
    if (t->kind == TY_BOOL) return jd_fmt(s, "_db(%s)", expr);
    if (t->kind == TY_CHAR) return jd_fmt(s, "_dch((%s) as int)", expr);
    if (type_is_float(t)) return jd_fmt(s, "_df((%s) as f64)", expr);
    if (type_is_integer(t)) {
        if (jd_is_unsigned(t)) return jd_fmt(s, "_du((%s) as u64)", expr);
        return jd_fmt(s, "_di((%s) as i64)", expr);
    }
    return NULL;
}

/* `@json "" "string"`: the scalar goes out as a JSON string. Wide 64-bit ids
 * that a JavaScript client would round off are the usual reason. */
static const char *maybe_stringify(sema_t *s, type_t *t, const char *call,
                                   const json_attr_t *at)
{
    if (!at || !at->as_string) return call;
    if (t->kind == TY_STR || t->kind == TY_UCHAR) return call;
    return jd_fmt(s, "_dqs(%s)", call);
}

static bool enc_enum(sema_t *s, sb_t *b, type_t *t, const char *expr, const char *ind)
{
    symbol_t *sym = (symbol_t *)t->decl;
    bool first = true;
    size_t i = 0;
    if (!sym || !sym->members) return false;
    for (; i < sym->members->symbols.len; i++) {
        symbol_t *m = (symbol_t *)sym->members->symbols.data[i];
        if (m->kind != SYM_ENUM_MEMBER) continue;
        sb_printf(b, "%s%s (%s) as i64 == %lld:\n", ind, first ? "if" : "else", expr,
                  (long long)m->enum_value);
        add(b, jd_fmt(s, "%s" JIND, ind), jd_quote(s, jd_quote(s, m->name)));
        first = false;
    }
    if (first) return false;
    /* A value outside the declared members still has to produce valid JSON;
     * its number is the only lossless thing left to write. */
    sb_printf(b, "%selse:\n", ind);
    add(b, jd_fmt(s, "%s" JIND, ind), jd_fmt(s, "_di((%s) as i64)", expr));
    sb_printf(b, "%send\n", ind);
    return true;
}

static bool enc_ptr(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *expr,
                    const char *ind, scope_t *home, const src_span_t *span)
{
    const char *deep = jd_fmt(s, "%s" JIND, ind);
    sb_printf(b, "%sif (%s) == null:\n", ind, expr);
    add(b, deep, "\"null\"");
    sb_printf(b, "%selse:\n", ind);
    if (!enc_value(s, b, c, t->pointee, jd_fmt(s, "(%s)[0]", expr), deep, NULL, home,
                   span))
        return false;
    sb_printf(b, "%send\n", ind);
    return true;
}

static bool enc_value(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *expr,
                      const char *ind, const json_attr_t *at, scope_t *home,
                      const src_span_t *span)
{
    const char *call;
    if (!t) return false;
    call = scalar_call(s, t, expr);
    if (call) {
        add(b, ind, maybe_stringify(s, t, call, at));
        return true;
    }
    if (at && at->as_string) return false;
    if (t->kind == TY_ENUM) return enc_enum(s, b, t, expr, ind);
    if (t->kind == TY_PTR) return enc_ptr(s, b, c, t, expr, ind, home, span);
    {
        const char *fn = sema_derive_json_enc(s, t, home, span);
        if (!fn) return false;
        add(b, ind, jd_fmt(s, "%s(%s)", fn, expr));
        return true;
    }
}

/* --------------------------------------------------------------- omitempty */

/* The test that decides whether an `omitempty` field is written at all, or
 * NULL for a type with no meaningful empty value (a struct, a fixed array) -
 * those are always written. */
static const char *empty_test(sema_t *s, type_t *t, const char *expr)
{
    if (!t) return NULL;
    if (t->kind == TY_STR || t->kind == TY_UCHAR)
        return jd_fmt(s, "(%s).len() > 0", expr);
    if (t->kind == TY_BOOL) return jd_fmt(s, "%s", expr);
    if (t->kind == TY_PTR) return jd_fmt(s, "(%s) != null", expr);
    if (t->kind == TY_ENUM) return jd_fmt(s, "(%s) as i64 != 0", expr);
    if (type_is_float(t)) return jd_fmt(s, "(%s) as f64 != 0.0", expr);
    if (type_is_integer(t) || t->kind == TY_CHAR)
        return jd_fmt(s, "(%s) as i64 != 0", expr);
    if (t->kind == TY_VEC) return jd_fmt(s, "(%s).len() > 0", expr);
    if (t->kind == TY_MAP) return jd_fmt(s, "(%s).size() > 0", expr);
    if (t->kind == TY_STRUCT) {
        symbol_t *sym = (symbol_t *)t->decl;
        type_t *k, *v;
        const char *count;
        if (sym && derive_struct_map(sym, &k, &v)) return jd_fmt(s, "(%s).size() > 0", expr);
        if (sym && derive_struct_seq(sym, &v, &count))
            return jd_fmt(s, "(%s).%s() > 0", expr, count);
    }
    return NULL;
}

/* ------------------------------------------------------------------ bodies */

static bool enc_struct(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, scope_t *home,
                       const src_span_t *span)
{
    symbol_t *sym = (symbol_t *)t->decl;
    size_t nfields = 0;
    size_t i = 0;

    for (; sym->members && i < sym->members->symbols.len; i++)
        if (((symbol_t *)sym->members->symbols.data[i])->kind == SYM_FIELD) nfields++;

    add(b, JIND, "\"{\"");
    if (nfields == 0) {
        add(b, JIND, "\"}\"");
        return true;
    }
    sb_puts(b, JIND "mut _n := 0\n");
    for (i = 0; sym->members && i < sym->members->symbols.len; i++) {
        symbol_t *f = (symbol_t *)sym->members->symbols.data[i];
        json_attr_t at;
        const char *expr;
        const char *ind = JIND;
        const char *test;
        if (f->kind != SYM_FIELD) continue;
        if (!jd_attr_of(s, f, &at, span)) return false;
        if (at.skip) continue;
        expr = jd_fmt(s, JV_PARAM ".%s", f->name);
        test = at.omitempty ? empty_test(s, f->type, expr) : NULL;
        if (test) {
            sb_printf(b, JIND "if %s:\n", test);
            ind = JIND JIND;
        }
        sb_printf(b, "%sif _n > 0: _dadd(" JV_BUF ", \",\") end\n", ind);
        sb_printf(b, "%s_n = _n + 1\n", ind);
        add(b, ind, jd_quote(s, jd_fmt(s, "\"%s\":", at.name)));
        if (!enc_value(s, b, c, f->type, expr, ind, &at, home, span)) {
            /* Naming the field is the whole diagnostic: "cannot encode
             * 'Config'" leaves the reader to find which of its twelve members
             * has no JSON form. */
            SERR(s, 2, f->decl ? &f->decl->span : span,
                 "field '%s' of type '%s' has no JSON form; mark it '@json \"-\"' to "
                 "leave it out",
                 f->name, type_to_string(s->tc, f->type));
            return false;
        }
        if (test) sb_puts(b, JIND "end\n");
    }
    add(b, JIND, "\"}\"");
    return true;
}

static bool enc_seq(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *elem, const char *count,
                    const char *at, scope_t *home, const src_span_t *span)
{
    add(b, JIND, "\"[\"");
    sb_puts(b, JIND "mut _i := 0\n");
    sb_printf(b, JIND "until _i < %s:\n", count);
    sb_puts(b, JIND JIND "if _i > 0: _dadd(" JV_BUF ", \",\") end\n");
    if (!enc_value(s, b, c, elem, at, JIND JIND, NULL, home, span)) return false;
    sb_puts(b, JIND JIND "_i = _i + 1\n");
    sb_puts(b, JIND "end\n");
    add(b, JIND, "\"]\"");
    return true;
}

/* An object key is text. A string key is written as it stands; an integer key
 * is written as its digits, which is what every other language's JSON map
 * encoder does and what a decoder can read back. Anything else has no key
 * spelling and the type is not encodable. */
static const char *key_expr(sema_t *s, type_t *key)
{
    if (!key) return NULL;
    if (key->kind == TY_STR || key->kind == TY_UCHAR) return "_dqs(_it.key())";
    if (type_is_integer(key)) return "_dqs(_di(_it.key() as i64))";
    return NULL;
}

static bool enc_map(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *key, type_t *val,
                    scope_t *home, const src_span_t *span)
{
    const char *ke = key_expr(s, key);
    if (!ke) return false;
    add(b, JIND, "\"{\"");
    sb_puts(b, JIND "mut _n := 0\n");
    sb_puts(b, JIND "mut _it := " JV_PARAM ".iter()\n");
    sb_puts(b, JIND "until _it.has_next():\n");
    sb_puts(b, JIND JIND "if _n > 0: _dadd(" JV_BUF ", \",\") end\n");
    add(b, JIND JIND, ke);
    add(b, JIND JIND, "\":\"");
    if (!enc_value(s, b, c, val, "_it.value()", JIND JIND, NULL, home, span)) return false;
    sb_puts(b, JIND JIND "_n = _n + 1\n");
    sb_puts(b, JIND JIND "_it.next()\n");
    sb_puts(b, JIND "end\n");
    add(b, JIND, "\"}\"");
    return true;
}

static bool enc_of_struct(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, scope_t *home,
                          const src_span_t *span)
{
    symbol_t *sym = (symbol_t *)t->decl;
    func_sig_t *sig = derive_method_sig(sym, "to_json", 0);
    type_t *k, *v;
    const char *count;
    if (sig && sig->ret && sig->ret->kind == TY_STR) {
        add(b, JIND, JV_PARAM ".to_json()");
        return true;
    }
    if (derive_struct_map(sym, &k, &v)) return enc_map(s, b, c, k, v, home, span);
    if (derive_struct_seq(sym, &v, &count))
        return enc_seq(s, b, c, v, jd_fmt(s, JV_PARAM ".%s()", count),
                       JV_PARAM ".get(_i)", home, span);
    return enc_struct(s, b, c, t, home, span);
}

static bool enc_body(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, scope_t *home,
                     const src_span_t *span)
{
    switch (t->kind) {
    case TY_STRUCT:
        return enc_of_struct(s, b, c, t, home, span);
    case TY_VEC:
        return enc_seq(s, b, c, t->elem, JV_PARAM ".len()", JV_PARAM ".get(_i)", home,
                       span);
    case TY_ARRAY:
    case TY_SLICE:
        return enc_seq(s, b, c, t->elem, "len(" JV_PARAM ")", JV_PARAM "[_i]", home, span);
    case TY_MAP:
        return enc_map(s, b, c, t->key, t->elem, home, span);
    default:
        /* A scalar at the top: `json.Marshal(42)` is a JSON document. */
        return enc_value(s, b, c, t, JV_PARAM, JIND, NULL, home, span);
    }
}

const char *jd_enc_source(sema_t *s, type_t *t, const char *fname, scope_t *home,
                          const src_span_t *span)
{
    const char *spelling = derive_type_spelling(s, t);
    jd_ctx_t c;
    sb_t b;
    c.n = 0;
    if (!spelling) return NULL;
    sb_init(&b);
    sb_printf(&b, "func %s(" JV_PARAM ": %s): str:\n", fname, spelling);
    sb_puts(&b, JIND "mut " JV_BUF " := _dnew()\n");
    if (!enc_body(s, &b, &c, t, home, span)) {
        sb_free(&b);
        return NULL;
    }
    sb_puts(&b, JIND "ret _dfin(" JV_BUF ")\n");
    sb_puts(&b, "end\n");
    {
        const char *r = arena_strdup(s->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}
