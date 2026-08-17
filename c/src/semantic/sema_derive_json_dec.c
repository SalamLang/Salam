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

void jd_fail(sb_t *b, sema_t *s, const char *ind, const char *msg, const char *pos)
{
    sb_printf(b, "%sret _derr(" JD_ERR ", " JD_ERRP ", %s, %s)\n", ind, jd_quote(s, msg),
              pos);
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
        jd_fail(b, s, jd_fmt(s, "%s" JIND, ind), "expected a quoted value", JD_POS);
        sb_printf(b, "%send\n", ind);
        sb_printf(b, "%smut %s2 := 0\n", ind, q);
        if (t->kind == TY_BOOL)
            sb_printf(b, "%s%s = _dgbool(%s, 0, %s2)\n", ind, lval, raw, q);
        else if (type_is_float(t))
            sb_printf(b, "%s%s = _dgflt(%s, 0, %s2) as %s\n", ind, lval, raw, q,
                      spelling);
        else if (jd_is_unsigned(t))
            sb_printf(b, "%s%s = _dguint(%s, 0, %s2) as %s\n", ind, lval, raw, q,
                      spelling);
        else
            sb_printf(b, "%s%s = _dgint(%s, 0, %s2) as %s\n", ind, lval, raw, q,
                      spelling);
        sb_printf(b, "%sif %s2 < 0:\n", ind, q);
        jd_fail(b, s, jd_fmt(s, "%s" JIND, ind), "quoted value is not well formed",
                JD_POS);
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
        read = jd_fmt(
            s, "_drange(_dgint(" JD_TEXT ", " JD_POS ", %s), %lld, %lld, %s) as %s", q,
            lo, hi, q, spelling);
    else if (type_is_integer(t))
        read = jd_fmt(s, "_dgint(" JD_TEXT ", " JD_POS ", %s) as %s", q, spelling);
    else
        return false;

    if (jd_is_unsigned(t) && jd_int_bounds(t, &lo, &hi))
        read = jd_fmt(s,
                      "_drange(_dguint(" JD_TEXT ", " JD_POS ", %s) as i64, %lld, %lld, "
                      "%s) as %s",
                      q, lo, hi, q, spelling);

    sb_printf(b, "%s%s = %s\n", ind, lval, read);
    sb_printf(b, "%sif %s < 0:\n", ind, q);
    jd_fail(b, s, jd_fmt(s, "%s" JIND, ind), "value does not match the expected type",
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
    jd_fail(b, s, deep, "expected a string naming an enum member", JD_POS);
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
    jd_fail(b, s, deep, "no enum member with that name", JD_POS);
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
    sb_printf(b,
              "%s%s := %s(" JD_TEXT ", " JD_POS ", %s, " JD_ERR ", " JD_ERRP
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
    if (!jd_dec_value(s, b, c, t->pointee, jd_fmt(s, "(%s)[0]", lval), deep, NULL, home,
                      span))
        return false;
    sb_printf(b, "%send\n", ind);
    return true;
}

bool jd_dec_value(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *lval,
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
