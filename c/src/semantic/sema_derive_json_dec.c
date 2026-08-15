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
 * The Salam source of a derived JSON decoder.
 *
 * Shape: `func _jsondec_T(_s, _i, _o &: T, _e &: str, _ep &: int, _st: bool):
 * int`, returning the position just past the value it read or -1 with the
 * message and offset filled in. Every nested decoder has the same shape, so a
 * failure twelve levels down needs no unwinding: the first _derr wins and
 * every frame above it returns -1.
 *
 * `_st` is strictness, threaded through rather than baked in, so one
 * Unmarshal has one policy for the whole document:
 *
 *   strict    unknown key, missing key and repeated key are errors
 *   lenient   unknown keys are skipped, missing keys keep the field's
 *             default, a repeated key wins
 *
 * A type mismatch is an error in both: a decoder that quietly turns "3" into
 * 3, or 1.9 into 1, hides the schema drift it exists to catch.
 */

#include "core/prelude.h"
#include "semantic/sema_derive_core.h"
#include "semantic/sema_derive_json_internal.h"

static bool dec_value(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *lval,
                      const char *ind, const json_attr_t *at, scope_t *home,
                      const src_span_t *span);

static void fail(sb_t *b, sema_t *s, const char *ind, const char *msg, const char *pos)
{
    sb_printf(b, "%sret _derr(" JD_ERR ", " JD_ERRP ", %s, %s)\n", ind,
              jd_quote(s, msg), pos);
}

bool jd_int_bounds(const type_t *t, long long *lo, long long *hi)
{
    switch (t->kind) {
    case TY_I8:
        *lo = -128;
        *hi = 127;
        return true;
    case TY_I16:
        *lo = -32768;
        *hi = 32767;
        return true;
    case TY_I32:
        *lo = -2147483647LL - 1;
        *hi = 2147483647LL;
        return true;
    case TY_U8:
        *lo = 0;
        *hi = 255;
        return true;
    case TY_U16:
        *lo = 0;
        *hi = 65535;
        return true;
    case TY_U32:
        *lo = 0;
        *hi = 4294967295LL;
        return true;
    default:
        return false;
    }
}

/* --------------------------------------------------------------- scalars */

/*
 * One scalar: read it, check it landed, store it. Every reader reports
 * failure the same way (an endpos of -1), so the shape here does not vary
 * with the type - only the call and the cast do.
 */
static bool dec_scalar(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *lval,
                       const char *ind, const json_attr_t *at)
{
    const char *q = jd_fresh(s, c, "q");
    const char *spelling = derive_type_spelling(s, t);
    const char *read;
    long long lo = 0, hi = 0;
    bool quoted = at && at->as_string && t->kind != TY_STR && t->kind != TY_UCHAR;

    if (!spelling) return false;
    sb_printf(b, "%smut %s := 0\n", ind, q);

    /* `@json "" "string"` on the way in: the value arrives as a JSON string
     * and its contents are what the number or the bool is spelled in. */
    if (quoted) {
        const char *raw = jd_fresh(s, c, "r");
        sb_printf(b, "%s%s := _dgstr(" JD_TEXT ", " JD_POS ", %s)\n", ind, raw, q);
        sb_printf(b, "%sif %s < 0:\n", ind, q);
        fail(b, s, jd_fmt(s, "%s" JIND, ind), "expected a quoted value", JD_POS);
        sb_printf(b, "%send\n", ind);
        sb_printf(b, "%smut %s2 := 0\n", ind, q);
        if (t->kind == TY_BOOL)
            sb_printf(b, "%s%s = _dgbool(%s, 0, %s2)\n", ind, lval, raw, q);
        else if (type_is_float(t))
            sb_printf(b, "%s%s = _dgflt(%s, 0, %s2) as %s\n", ind, lval, raw, q, spelling);
        else if (jd_is_unsigned(t))
            sb_printf(b, "%s%s = _dguint(%s, 0, %s2) as %s\n", ind, lval, raw, q,
                      spelling);
        else
            sb_printf(b, "%s%s = _dgint(%s, 0, %s2) as %s\n", ind, lval, raw, q, spelling);
        sb_printf(b, "%sif %s2 < 0:\n", ind, q);
        fail(b, s, jd_fmt(s, "%s" JIND, ind), "quoted value is not well formed", JD_POS);
        sb_printf(b, "%send\n", ind);
        sb_printf(b, "%s" JD_POS " = %s\n", ind, q);
        return true;
    }

    if (t->kind == TY_STR || t->kind == TY_UCHAR)
        read = jd_fmt(s, "_dgstr(" JD_TEXT ", " JD_POS ", %s)", q);
    else if (t->kind == TY_BOOL)
        read = jd_fmt(s, "_dgbool(" JD_TEXT ", " JD_POS ", %s)", q);
    else if (t->kind == TY_CHAR)
        read = jd_fmt(s, "_dgch(" JD_TEXT ", " JD_POS ", %s) as %s", q, spelling);
    else if (type_is_float(t))
        read = jd_fmt(s, "_dgflt(" JD_TEXT ", " JD_POS ", %s) as %s", q, spelling);
    else if (jd_is_unsigned(t))
        read = jd_fmt(s, "_dguint(" JD_TEXT ", " JD_POS ", %s) as %s", q, spelling);
    else if (jd_int_bounds(t, &lo, &hi))
        read = jd_fmt(s, "_drange(_dgint(" JD_TEXT ", " JD_POS ", %s), %lld, %lld, %s) as %s",
                      q, lo, hi, q, spelling);
    else if (type_is_integer(t))
        read = jd_fmt(s, "_dgint(" JD_TEXT ", " JD_POS ", %s) as %s", q, spelling);
    else
        return false;

    if (jd_is_unsigned(t) && jd_int_bounds(t, &lo, &hi))
        read = jd_fmt(s, "_drange(_dguint(" JD_TEXT ", " JD_POS ", %s) as i64, %lld, %lld, "
                         "%s) as %s",
                      q, lo, hi, q, spelling);

    sb_printf(b, "%s%s = %s\n", ind, lval, read);
    sb_printf(b, "%sif %s < 0:\n", ind, q);
    fail(b, s, jd_fmt(s, "%s" JIND, ind), "value does not match the expected type",
         JD_POS);
    sb_printf(b, "%send\n", ind);
    sb_printf(b, "%s" JD_POS " = %s\n", ind, q);
    return true;
}

static bool dec_enum(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *lval,
                     const char *ind)
{
    symbol_t *sym = (symbol_t *)t->decl;
    const char *spelling = derive_type_spelling(s, t);
    const char *q = jd_fresh(s, c, "q");
    const char *nm = jd_fresh(s, c, "en");
    const char *deep = jd_fmt(s, "%s" JIND, ind);
    bool first = true;
    size_t i = 0;

    if (!sym || !sym->members || !spelling) return false;
    sb_printf(b, "%smut %s := 0\n", ind, q);
    sb_printf(b, "%s%s := _dgstr(" JD_TEXT ", " JD_POS ", %s)\n", ind, nm, q);
    sb_printf(b, "%sif %s < 0:\n", ind, q);
    fail(b, s, deep, "expected a string naming an enum member", JD_POS);
    sb_printf(b, "%send\n", ind);
    for (; i < sym->members->symbols.len; i++) {
        symbol_t *m = (symbol_t *)sym->members->symbols.data[i];
        if (m->kind != SYM_ENUM_MEMBER) continue;
        sb_printf(b, "%s%s %s == %s:\n", ind, first ? "if" : "else", nm,
                  jd_quote(s, m->name));
        sb_printf(b, "%s%s = %s.%s\n", deep, lval, spelling, m->name);
        first = false;
    }
    if (first) return false;
    sb_printf(b, "%selse:\n", ind);
    fail(b, s, deep, "no enum member with that name", JD_POS);
    sb_printf(b, "%send\n", ind);
    sb_printf(b, "%s" JD_POS " = %s\n", ind, q);
    return true;
}

/* ------------------------------------------------------------ aggregates */

/* Anything with its own derived function: recurse into it and take its
 * cursor. `lval` is passed by reference, so a field twelve levels down is
 * written in place rather than copied back up. */
static bool dec_nested(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *lval,
                       const char *ind, scope_t *home, const src_span_t *span)
{
    const char *fn = sema_derive_json_dec(s, t, home, span);
    const char *q = jd_fresh(s, c, "q");
    if (!fn) return false;
    sb_printf(b, "%s%s := %s(" JD_TEXT ", " JD_POS ", %s, " JD_ERR ", " JD_ERRP
                 ", " JD_STRICT ")\n",
              ind, q, fn, lval);
    sb_printf(b, "%sif %s < 0: ret -1 end\n", ind, q);
    sb_printf(b, "%s" JD_POS " = %s\n", ind, q);
    return true;
}

/*
 * A pointer field: `null` clears it, anything else gets storage of its own
 * and is decoded straight through the pointer. Salam has no address-of
 * operator, so a pointee that outlives this frame has to be allocated rather
 * than pointed at.
 */
static bool dec_ptr(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *lval,
                    const char *ind, scope_t *home, const src_span_t *span)
{
    const char *spelling = derive_type_spelling(s, t->pointee);
    const char *nul = jd_fresh(s, c, "q");
    const char *deep = jd_fmt(s, "%s" JIND, ind);
    if (!spelling) return false;
    sb_printf(b, "%s%s := _dnullat(" JD_TEXT ", " JD_POS ")\n", ind, nul);
    sb_printf(b, "%sif %s > 0:\n", ind, nul);
    sb_printf(b, "%s%s = null\n", deep, lval);
    sb_printf(b, "%s" JD_POS " = %s\n", deep, nul);
    sb_printf(b, "%selse:\n", ind);
    sb_printf(b, "%s%s = _dalloc(sizeof(%s)) as %s*\n", deep, lval, spelling, spelling);
    if (!dec_value(s, b, c, t->pointee, jd_fmt(s, "(%s)[0]", lval), deep, NULL, home,
                   span))
        return false;
    sb_printf(b, "%send\n", ind);
    return true;
}

static bool dec_value(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *lval,
                      const char *ind, const json_attr_t *at, scope_t *home,
                      const src_span_t *span)
{
    if (!t) return false;
    if (jd_is_scalar(t)) return dec_scalar(s, b, c, t, lval, ind, at);
    if (at && at->as_string) return false;
    if (t->kind == TY_ENUM) return dec_enum(s, b, c, t, lval, ind);
    if (t->kind == TY_PTR) return dec_ptr(s, b, c, t, lval, ind, home, span);
    return dec_nested(s, b, c, t, lval, ind, home, span);
}

/* ---------------------------------------------------------------- bodies */

/*
 * The object walk every struct and map decoder shares: open brace, then
 * key/colon/value until the closing brace. `on_key` writes what to do with a
 * decoded key, and is the only part that differs between the two.
 */
typedef bool (*key_body_fn)(sema_t *s, sb_t *b, jd_ctx_t *c, void *ud, const char *keyvar,
                            const char *ind);

static bool dec_object(sema_t *s, sb_t *b, jd_ctx_t *c, key_body_fn on_key, void *ud)
{
    const char *k = jd_fresh(s, c, "k");
    const char *ke = jd_fresh(s, c, "q");

    sb_puts(b, JIND JD_POS " = _jws(" JD_TEXT ", " JD_START ")\n");
    sb_puts(b, JIND "if _dbyte(" JD_TEXT ", " JD_POS ") != 123:\n");
    fail(b, s, JIND JIND, "expected '{'", JD_POS);
    sb_puts(b, JIND "end\n");
    sb_puts(b, JIND JD_POS " = _jws(" JD_TEXT ", " JD_POS " + 1)\n");
    sb_puts(b, JIND "if _dbyte(" JD_TEXT ", " JD_POS ") == 125:\n");
    sb_puts(b, JIND JIND JD_POS " = " JD_POS " + 1\n");
    sb_puts(b, JIND "else:\n");
    sb_puts(b, JIND JIND "until true:\n");
    sb_printf(b, JIND JIND JIND "mut %s := 0\n", ke);
    sb_printf(b, JIND JIND JIND "%s := _dgstr(" JD_TEXT ", " JD_POS ", %s)\n", k, ke);
    sb_printf(b, JIND JIND JIND "if %s < 0:\n", ke);
    fail(b, s, JIND JIND JIND JIND, "expected an object key", JD_POS);
    sb_puts(b, JIND JIND JIND "end\n");
    sb_printf(b, JIND JIND JIND JD_POS " = _jws(" JD_TEXT ", %s)\n", ke);
    sb_puts(b, JIND JIND JIND "if _dbyte(" JD_TEXT ", " JD_POS ") != 58:\n");
    fail(b, s, JIND JIND JIND JIND, "expected ':'", JD_POS);
    sb_puts(b, JIND JIND JIND "end\n");
    sb_puts(b, JIND JIND JIND JD_POS " = _jws(" JD_TEXT ", " JD_POS " + 1)\n");
    if (!on_key(s, b, c, ud, k, JIND JIND JIND)) return false;
    sb_puts(b, JIND JIND JIND JD_POS " = _jws(" JD_TEXT ", " JD_POS ")\n");
    sb_puts(b, JIND JIND JIND "if _dbyte(" JD_TEXT ", " JD_POS ") == 44:\n");
    sb_puts(b, JIND JIND JIND JIND JD_POS " = _jws(" JD_TEXT ", " JD_POS " + 1)\n");
    sb_puts(b, JIND JIND JIND JIND "continue\n");
    sb_puts(b, JIND JIND JIND "end\n");
    sb_puts(b, JIND JIND JIND "if _dbyte(" JD_TEXT ", " JD_POS ") == 125:\n");
    sb_puts(b, JIND JIND JIND JIND JD_POS " = " JD_POS " + 1\n");
    sb_puts(b, JIND JIND JIND JIND "break\n");
    sb_puts(b, JIND JIND JIND "end\n");
    fail(b, s, JIND JIND JIND, "expected ',' or '}'", JD_POS);
    sb_puts(b, JIND JIND "end\n");
    sb_puts(b, JIND "end\n");
    return true;
}

typedef struct {
    type_t *t;
    scope_t *home;
    const src_span_t *span;
    vec_t seen; /* const char*, the `_f<n>` flag per required field */
    vec_t need; /* const char*, the wire name each flag belongs to */
    bool ok;
} struct_ud_t;

static bool struct_key(sema_t *s, sb_t *b, jd_ctx_t *c, void *ud, const char *keyvar,
                       const char *ind)
{
    struct_ud_t *u = (struct_ud_t *)ud;
    symbol_t *sym = (symbol_t *)u->t->decl;
    const char *deep = jd_fmt(s, "%s" JIND, ind);
    bool first = true;
    size_t i = 0;

    for (; sym->members && i < sym->members->symbols.len; i++) {
        symbol_t *f = (symbol_t *)sym->members->symbols.data[i];
        json_attr_t at;
        const char *flag;
        if (f->kind != SYM_FIELD) continue;
        if (!jd_attr_of(s, f, &at, u->span)) return false;
        if (at.skip) continue;
        flag = jd_fresh(s, c, "f");
        vec_push(s->a, &u->seen, CONST_CAST(flag));
        vec_push(s->a, &u->need, CONST_CAST(at.optional ? NULL : at.name));
        sb_printf(b, "%s%s %s == %s:\n", ind, first ? "if" : "else", keyvar,
                  jd_quote(s, at.name));
        sb_printf(b, "%sif %s && " JD_STRICT ":\n", deep, flag);
        fail(b, s, jd_fmt(s, "%s" JIND, deep),
             jd_fmt(s, "duplicate key '%s'", at.name), JD_POS);
        sb_printf(b, "%send\n", deep);
        sb_printf(b, "%s%s = true\n", deep, flag);
        if (!dec_value(s, b, c, f->type, jd_fmt(s, JD_OUT ".%s", f->name), deep, &at,
                       u->home, u->span))
            return false;
        first = false;
    }
    if (first) {
        /* A struct with no decodable field still has to consume the object's
         * contents, or the caller sees a key it cannot place. */
        sb_printf(b, "%sif " JD_STRICT ":\n", ind);
        fail(b, s, deep, "unknown key", JD_POS);
        sb_printf(b, "%send\n", ind);
        sb_printf(b, "%s" JD_POS " = _dskip(" JD_TEXT ", " JD_POS ")\n", ind);
        sb_printf(b, "%sif " JD_POS " < 0:\n", ind);
        fail(b, s, deep, "value is not well formed", JD_ERRP);
        sb_printf(b, "%send\n", ind);
        return true;
    }
    sb_printf(b, "%selse:\n", ind);
    sb_printf(b, "%sif " JD_STRICT ":\n", deep);
    fail(b, s, jd_fmt(s, "%s" JIND, deep), "unknown key", JD_POS);
    sb_printf(b, "%send\n", deep);
    sb_printf(b, "%s" JD_POS " = _dskip(" JD_TEXT ", " JD_POS ")\n", deep);
    sb_printf(b, "%sif " JD_POS " < 0:\n", deep);
    fail(b, s, jd_fmt(s, "%s" JIND, deep), "value is not well formed", JD_START);
    sb_printf(b, "%send\n", deep);
    sb_printf(b, "%send\n", ind);
    return true;
}

static bool dec_struct(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, scope_t *home,
                       const src_span_t *span)
{
    struct_ud_t u;
    sb_t body;
    size_t i = 0;
    u.t = t;
    u.home = home;
    u.span = span;
    u.ok = true;
    vec_init(&u.seen);
    vec_init(&u.need);

    /* The flag declarations have to precede the walk that assigns them, but
     * the walk is what discovers how many there are. */
    sb_init(&body);
    if (!dec_object(s, &body, c, struct_key, &u)) {
        sb_free(&body);
        return false;
    }
    for (; i < u.seen.len; i++)
        sb_printf(b, JIND "mut %s := false\n", (const char *)u.seen.data[i]);
    sb_puts(b, sb_cstr(&body));
    sb_free(&body);

    for (i = 0; i < u.need.len; i++) {
        const char *want = (const char *)u.need.data[i];
        if (!want) continue;
        sb_printf(b, JIND "if " JD_STRICT " && %s == false:\n",
                  (const char *)u.seen.data[i]);
        fail(b, s, JIND JIND, jd_fmt(s, "missing key '%s'", want), JD_START);
        sb_puts(b, JIND "end\n");
    }
    return true;
}

typedef struct {
    type_t *val;
    scope_t *home;
    const src_span_t *span;
} map_ud_t;

static bool map_key(sema_t *s, sb_t *b, jd_ctx_t *c, void *ud, const char *keyvar,
                    const char *ind)
{
    map_ud_t *u = (map_ud_t *)ud;
    const char *spelling = derive_type_spelling(s, u->val);
    const char *tmp = jd_fresh(s, c, "t");
    if (!spelling) return false;
    if (jd_is_scalar(u->val) || u->val->kind == TY_ENUM)
        sb_printf(b, "%smut %s := %s\n", ind, tmp,
                  u->val->kind == TY_STR ? "\"\"" : jd_fmt(s, "0 as %s", spelling));
    else
        sb_printf(b, "%smut %s := %s {}\n", ind, tmp, spelling);
    if (!dec_value(s, b, c, u->val, tmp, ind, NULL, u->home, u->span)) return false;
    sb_printf(b, "%s" JD_OUT ".put(%s, %s)\n", ind, keyvar, tmp);
    return true;
}

static bool dec_map(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *key, type_t *val,
                    scope_t *home, const src_span_t *span)
{
    map_ud_t u;
    if (!key || !(key->kind == TY_STR || key->kind == TY_UCHAR)) return false;
    u.val = val;
    u.home = home;
    u.span = span;
    return dec_object(s, b, c, map_key, &u);
}

/*
 * An array: open bracket, values until the close. Elements land in a local
 * and are pushed, because a growable container has no in-place slot to hand
 * out before the element exists.
 */
static bool dec_seq(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *elem, const char *push,
                    scope_t *home, const src_span_t *span)
{
    const char *spelling = derive_type_spelling(s, elem);
    const char *tmp = jd_fresh(s, c, "t");
    if (!spelling) return false;
    sb_puts(b, JIND JD_POS " = _jws(" JD_TEXT ", " JD_START ")\n");
    sb_puts(b, JIND "if _dbyte(" JD_TEXT ", " JD_POS ") != 91:\n");
    fail(b, s, JIND JIND, "expected '['", JD_POS);
    sb_puts(b, JIND "end\n");
    sb_puts(b, JIND JD_POS " = _jws(" JD_TEXT ", " JD_POS " + 1)\n");
    sb_puts(b, JIND "if _dbyte(" JD_TEXT ", " JD_POS ") == 93:\n");
    sb_puts(b, JIND JIND JD_POS " = " JD_POS " + 1\n");
    sb_puts(b, JIND "else:\n");
    sb_puts(b, JIND JIND "until true:\n");
    if (jd_is_scalar(elem) || elem->kind == TY_ENUM)
        sb_printf(b, JIND JIND JIND "mut %s := %s\n", tmp,
                  elem->kind == TY_STR ? "\"\"" : jd_fmt(s, "0 as %s", spelling));
    else if (elem->kind == TY_PTR)
        sb_printf(b, JIND JIND JIND "mut %s := null as %s\n", tmp, spelling);
    else
        sb_printf(b, JIND JIND JIND "mut %s := %s {}\n", tmp, spelling);
    if (!dec_value(s, b, c, elem, tmp, JIND JIND JIND, NULL, home, span)) return false;
    sb_printf(b, JIND JIND JIND "%s(%s)\n", push, tmp);
    sb_puts(b, JIND JIND JIND JD_POS " = _jws(" JD_TEXT ", " JD_POS ")\n");
    sb_puts(b, JIND JIND JIND "if _dbyte(" JD_TEXT ", " JD_POS ") == 44:\n");
    sb_puts(b, JIND JIND JIND JIND JD_POS " = _jws(" JD_TEXT ", " JD_POS " + 1)\n");
    sb_puts(b, JIND JIND JIND JIND "continue\n");
    sb_puts(b, JIND JIND JIND "end\n");
    sb_puts(b, JIND JIND JIND "if _dbyte(" JD_TEXT ", " JD_POS ") == 93:\n");
    sb_puts(b, JIND JIND JIND JIND JD_POS " = " JD_POS " + 1\n");
    sb_puts(b, JIND JIND JIND JIND "break\n");
    sb_puts(b, JIND JIND JIND "end\n");
    fail(b, s, JIND JIND JIND, "expected ',' or ']'", JD_POS);
    sb_puts(b, JIND JIND "end\n");
    sb_puts(b, JIND "end\n");
    return true;
}

static bool dec_of_struct(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, scope_t *home,
                          const src_span_t *span)
{
    symbol_t *sym = (symbol_t *)t->decl;
    func_sig_t *from = derive_method_sig(sym, "from_json", 1);
    type_t *k, *v;
    const char *count;

    /* A type that spells itself: hand it the raw text of its own value and
     * let it say whether it liked it. */
    if (from && from->ret && from->ret->kind == TY_BOOL) {
        const char *q = jd_fresh(s, c, "q");
        sb_printf(b, JIND "%s := _dskip(" JD_TEXT ", " JD_START ")\n", q);
        sb_printf(b, JIND "if %s < 0:\n", q);
        fail(b, s, JIND JIND, "value is not well formed", JD_START);
        sb_puts(b, JIND "end\n");
        sb_printf(b, JIND JD_POS " = _jws(" JD_TEXT ", " JD_START ")\n");
        sb_printf(b, JIND "if " JD_OUT ".from_json(" JD_TEXT ".substr(" JD_POS ", %s - "
                         JD_POS ")) == false:\n",
                  q);
        fail(b, s, JIND JIND, "value rejected by from_json", JD_START);
        sb_puts(b, JIND "end\n");
        sb_printf(b, JIND JD_POS " = %s\n", q);
        return true;
    }
    if (derive_struct_map(sym, &k, &v)) {
        map_ud_t u;
        if (!k || !(k->kind == TY_STR || k->kind == TY_UCHAR)) return false;
        u.val = v;
        u.home = home;
        u.span = span;
        if (!derive_method_sig(sym, "put", 2)) return false;
        return dec_object(s, b, c, map_key, &u);
    }
    if (derive_struct_seq(sym, &v, &count)) {
        if (!derive_method_sig(sym, "push", 1)) return false;
        return dec_seq(s, b, c, v, JD_OUT ".push", home, span);
    }
    return dec_struct(s, b, c, t, home, span);
}

static bool dec_body(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, scope_t *home,
                     const src_span_t *span)
{
    switch (t->kind) {
    case TY_STRUCT:
        return dec_of_struct(s, b, c, t, home, span);
    case TY_VEC:
        return dec_seq(s, b, c, t->elem, JD_OUT ".push", home, span);
    case TY_MAP:
        return dec_map(s, b, c, t->key, t->elem, home, span);
    default:
        sb_puts(b, JIND JD_POS " = _jws(" JD_TEXT ", " JD_START ")\n");
        return dec_value(s, b, c, t, JD_OUT, JIND, NULL, home, span);
    }
}

const char *jd_dec_source(sema_t *s, type_t *t, const char *fname, scope_t *home,
                          const src_span_t *span)
{
    const char *spelling = derive_type_spelling(s, t);
    jd_ctx_t c;
    sb_t b;
    c.n = 0;
    if (!spelling) return NULL;
    sb_init(&b);
    sb_printf(&b,
              "func %s(" JD_TEXT ": str, " JD_START ": int, " JD_OUT " &: %s, " JD_ERR
              " &: str, " JD_ERRP " &: int, " JD_STRICT ": bool): int:\n",
              fname, spelling);
    sb_puts(&b, JIND "mut " JD_POS " := " JD_START "\n");
    if (!dec_body(s, &b, &c, t, home, span)) {
        sb_free(&b);
        return NULL;
    }
    sb_puts(&b, JIND "ret " JD_POS "\n");
    sb_puts(&b, "end\n");
    {
        const char *r = arena_strdup(s->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}
