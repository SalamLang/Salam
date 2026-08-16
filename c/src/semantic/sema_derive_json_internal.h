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

#ifndef SALAM_SEMANTIC_SEMA_DERIVE_JSON_INTERNAL_H
#define SALAM_SEMANTIC_SEMA_DERIVE_JSON_INTERNAL_H

#include "core/prelude.h"
#include "core/sb.h"
#include "semantic/sema_derive_internal.h"
#include "semantic/sema_derive_json.h"

/* Locals the generated bodies use. Underscore-prefixed so they never collide
 * with a field, a package name or anything the unused-name rules care about. */
#define JV_PARAM "_v"  /* encoder: the value being written        */
#define JV_BUF "_sb"   /* encoder: the builder                    */
#define JD_TEXT "_s"   /* decoder: the whole document             */
#define JD_POS "_p"    /* decoder: the cursor                     */
#define JD_START "_i"  /* decoder: where this value began         */
#define JD_OUT "_o"    /* decoder: the destination, by reference  */
#define JD_ERR "_e"    /* decoder: the message, by reference      */
#define JD_ERRP "_ep"  /* decoder: the position, by reference     */
#define JD_STRICT "_st"

#define JIND "    "

/*
 * What a `@json` marker on a field says. `@json "wire" "omitempty"`: the
 * first value renames the field, the rest are options. `"-"` drops the field
 * from both directions, `""` keeps the field's own name.
 */
typedef struct {
    const char *name;
    bool skip;
    bool omitempty;
    bool optional;
    bool as_string;
} json_attr_t;

/*
 * Read the marker off a field symbol. Returns false and reports when a value
 * is not one of the options, so a typo is a compile error rather than a
 * setting that silently does nothing.
 */
bool jd_attr_of(sema_t *s, symbol_t *field, json_attr_t *out, const src_span_t *span);

/* A name no other line in the same generated function uses. */
typedef struct {
    int n;
} jd_ctx_t;

const char *jd_fresh(sema_t *s, jd_ctx_t *c, const char *stem);

/* Arena-allocated printf, for building generated fragments. */
const char *jd_fmt(sema_t *s, const char *f, ...);

/* `"..."` as it has to be written inside generated Salam source. */
const char *jd_quote(sema_t *s, const char *text);

/* True when the type is one the codec has a direct spelling for and the
 * caller does not have to derive a helper function. */
bool jd_is_scalar(const type_t *t);

/* u8/u16/u32/u64/size: the widths that decode through _du and have to reject
 * a negative number rather than wrap it. */
bool jd_is_unsigned(const type_t *t);

/* The bounds a narrow integer field is checked against, or false when the
 * type is i64/u64 and the check would be a no-op. */
bool jd_int_bounds(const type_t *t, long long *lo, long long *hi);

/* The decoder's single failure path, and its value dispatch. Shared between
 * sema_derive_json_dec.c and the container bodies split out of it. */
void jd_fail(sb_t *b, sema_t *s, const char *ind, const char *msg, const char *pos);
bool jd_dec_value(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, const char *lval,
                  const char *ind, const json_attr_t *at, scope_t *home,
                  const src_span_t *span);
bool jd_dec_body(sema_t *s, sb_t *b, jd_ctx_t *c, type_t *t, scope_t *home,
                 const src_span_t *span);

/* The `func ...` text, or NULL when some part of the type has no JSON form.
 * Both may derive further functions before returning. */
const char *jd_enc_source(sema_t *s, type_t *t, const char *fname, scope_t *home,
                          const src_span_t *span);
const char *jd_dec_source(sema_t *s, type_t *t, const char *fname, scope_t *home,
                          const src_span_t *span);

#endif /* SALAM_SEMANTIC_SEMA_DERIVE_JSON_INTERNAL_H */
