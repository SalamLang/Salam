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
 * `json.Marshal(v)` and `json.Unmarshal(text, out, err)` on a type the
 * compiler has never seen.
 *
 * Neither can be written once and for all in a language without runtime
 * reflection, so sema writes them per concrete type: this pass generates
 * `_jsonenc_T` and `_jsondec_T` as Salam source, parses them, and installs
 * them as synthetic top-level declarations - the same treatment a generic
 * instantiation gets, so every backend emits them without knowing this pass
 * exists. It is the machinery `println <aggregate>` already uses, sharing one
 * registry with it (sema_derive_core.c).
 *
 * The trigger is `jsonenc`/`jsondec` inside std/encoding/json's own generic
 * wrappers. Because those are monomorphised before their bodies are checked,
 * the type parameter is concrete by the time the call is looked at, and the
 * compiler needs no notion of what a "json package" is - only the two builtin
 * names, handled in sema_call.c the way `sizeof` is.
 *
 * Common helpers live here; the two emitters are in sema_derive_json_enc.c
 * and sema_derive_json_dec.c.
 */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "semantic/sema_derive_core.h"
#include "semantic/sema_derive_json_internal.h"

const char *jd_fmt(sema_t *s, const char *f, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    sal_vsnprintf(buf, sizeof buf, f, ap);
    va_end(ap);
    return arena_strdup(s->a, buf);
}

const char *jd_fresh(sema_t *s, jd_ctx_t *c, const char *stem)
{
    return jd_fmt(s, "_%s%d", stem, c->n++);
}

const char *jd_quote(sema_t *s, const char *text)
{
    sb_t b;
    const char *p = text ? text : "";
    sb_init(&b);
    sb_putc(&b, '"');
    for (; *p; p++) {
        if (*p == '"' || *p == '\\') sb_putc(&b, '\\');
        sb_putc(&b, *p);
    }
    sb_putc(&b, '"');
    {
        const char *r = arena_strdup(s->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

bool jd_is_scalar(const type_t *t)
{
    if (!t) return false;
    return derive_is_plain(t) || t->kind == TY_STR;
}

/* ------------------------------------------------------------- attributes */

static bool attr_option(sema_t *s, json_attr_t *out, const char *opt,
                        const src_span_t *span)
{
    if (strcmp(opt, "omitempty") == 0) {
        out->omitempty = true;
        /* A field the encoder is allowed to leave out has to be one the
         * decoder is allowed not to find, or a strict round trip of the
         * struct's own output would fail. */
        out->optional = true;
        return true;
    }
    if (strcmp(opt, "optional") == 0) {
        out->optional = true;
        return true;
    }
    if (strcmp(opt, "string") == 0) {
        out->as_string = true;
        return true;
    }
    SERR(s, 12, span,
         "unknown '@json' option '%s'; expected 'omitempty', 'optional' or 'string'",
         opt);
    return false;
}

bool jd_attr_of(sema_t *s, symbol_t *field, json_attr_t *out, const src_span_t *span)
{
    const vec_t *al = field->decl ? &field->decl->aliases : NULL;
    size_t i = 0;
    bool first = true;

    out->name = field->name;
    out->skip = false;
    out->omitempty = false;
    out->optional = false;
    out->as_string = false;
    if (!al) return true;

    for (; i + 1 < al->len; i += 2) {
        const char *lang = (const char *)al->data[i];
        const char *val = (const char *)al->data[i + 1];
        if (strcmp(lang, "json") != 0) continue;
        if (first) {
            first = false;
            if (strcmp(val, "-") == 0) {
                out->skip = true;
                return true;
            }
            if (*val) out->name = val;
            continue;
        }
        if (!attr_option(s, out, val, field->decl ? &field->decl->span : span))
            return false;
    }
    return true;
}

/* ----------------------------------------------------------------- driver */

typedef const char *(*jd_source_fn)(sema_t *, type_t *, const char *, scope_t *,
                                    const src_span_t *);

static const char *derive_one(sema_t *s, type_t *t, const char *prefix, jd_source_fn body,
                              scope_t *home, const src_span_t *span)
{
    const char *fname;
    derived_t *ent;
    const char *text;
    ast_node_t *fn;

    if (!t || !s->program) return NULL;
    fname = derive_fn_name(s, prefix, t);
    if (derive_claimed(s, fname)) return fname;
    if (scope_lookup_local(s->global, fname)) return NULL;

    /* Claimed before the body is generated: a struct holding a Vector of
     * itself asks for this same function again from inside the emitter, and
     * has to be told it already exists rather than recurse forever. */
    ent = derive_claim(s, fname);
    text = body(s, t, fname, home, span);
    if (!text) {
        derive_unclaim(s, ent);
        return NULL;
    }
    fn = derive_install_source(s, text, fname, home);
    if (!fn) {
        derive_unclaim(s, ent);
        return NULL;
    }
    derive_set_decl(ent, fn);
    return fname;
}

const char *sema_derive_json_enc(sema_t *s, type_t *t, scope_t *home,
                                 const src_span_t *span)
{
    return derive_one(s, t, "_jsonenc_", jd_enc_source, home, span);
}

const char *sema_derive_json_dec(sema_t *s, type_t *t, scope_t *home,
                                 const src_span_t *span)
{
    return derive_one(s, t, "_jsondec_", jd_dec_source, home, span);
}
