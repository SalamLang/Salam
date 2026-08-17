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
 * `.name()`, `FromString`, and `Names()` on a user enum: reflection the
 * compiler can't write once and for all (no runtime type info exists), so
 * it's derived per concrete enum, the same treatment sema_derive_json.c
 * gives json.Marshal/Unmarshal - generate Salam source, parse it, install it
 * as a synthetic top-level declaration. Every backend already knows how to
 * emit an `if`/`else` chain, a member comparison, and a Vector push, so none
 * of them need to know this pass exists.
 *
 * `Count()` needs no derived function at all - the call site in sema_call.c
 * folds it straight to an int literal, the member count being known at
 * compile time.
 */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "core/sb.h"
#include "semantic/sema_derive_core.h"
#include "semantic/sema_derive_enum.h"

static const char *derive_one(sema_t *s, type_t *t, const char *prefix,
                              const char *(*body)(sema_t *, type_t *, symbol_t *,
                                                  const char *))
{
    symbol_t *esym;
    const char *fname;
    derived_t *ent;
    const char *text;
    ast_node_t *fn;

    if (!t || t->kind != TY_ENUM || !s->program) return NULL;
    esym = (symbol_t *)t->decl;
    if (!esym || !esym->members || esym->members->symbols.len == 0) return NULL;

    fname = derive_fn_name(s, prefix, t);
    if (derive_claimed(s, fname)) return fname;
    if (scope_lookup_local(s->global, fname)) return NULL;

    ent = derive_claim(s, fname);
    text = body(s, t, esym, fname);
    if (!text) {
        derive_unclaim(s, ent);
        return NULL;
    }
    fn = derive_install_source(s, text, fname, esym->home);
    if (!fn) {
        derive_unclaim(s, ent);
        return NULL;
    }
    derive_set_decl(ent, fn);
    return fname;
}

static const char *name_source(sema_t *s, type_t *t, symbol_t *esym, const char *fname)
{
    const char *en = esym->name;
    sb_t b;
    (void)t;
    sb_init(&b);
    sb_printf(&b, "func %s(_v: %s): str:\n", fname, en);
    {
        size_t i = 0;
        for (; i < esym->members->symbols.len; i++) {
            symbol_t *m = (symbol_t *)esym->members->symbols.data[i];
            if (m->kind != SYM_ENUM_MEMBER) continue;
            sb_printf(&b, "    if _v == %s.%s: ret \"%s\" end\n", en, m->name, m->name);
        }
    }
    /* Unreachable for any value that actually came from this enum - every
     * member is covered above - but the function still needs a fallthrough
     * return to satisfy "does this function always return". */
    sb_puts(&b, "    ret \"\"\nend\n");
    {
        const char *r = arena_strdup(s->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

static const char *fromstring_source(sema_t *s, type_t *t, symbol_t *esym,
                                     const char *fname)
{
    const char *en = esym->name;
    const char *first_member = NULL;
    sb_t b;
    (void)t;
    sb_init(&b);
    sb_printf(&b, "func %s(_s: str, _ok &: bool): %s:\n", fname, en);
    {
        size_t i = 0;
        for (; i < esym->members->symbols.len; i++) {
            symbol_t *m = (symbol_t *)esym->members->symbols.data[i];
            if (m->kind != SYM_ENUM_MEMBER) continue;
            if (!first_member) first_member = m->name;
            sb_printf(&b, "    if _s == \"%s\": _ok = true ret %s.%s end\n", m->name, en,
                      m->name);
        }
    }
    /* No name matched: report failure via _ok and hand back an arbitrary
     * member (the first one declared) so the return type is satisfied - the
     * caller is required to check _ok before trusting the value, exactly
     * like every other ok&:bool-style fallible call in the stdlib. */
    sb_printf(&b, "    _ok = false\n    ret %s.%s\nend\n", en, first_member);
    {
        const char *r = arena_strdup(s->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

static const char *names_source(sema_t *s, type_t *t, symbol_t *esym, const char *fname)
{
    sb_t b;
    (void)t;
    sb_init(&b);
    sb_printf(&b, "func %s(): Vector<str>:\n", fname);
    sb_puts(&b, "    mut _v := Vector {} as Vector<str>\n");
    {
        size_t i = 0;
        for (; i < esym->members->symbols.len; i++) {
            symbol_t *m = (symbol_t *)esym->members->symbols.data[i];
            if (m->kind != SYM_ENUM_MEMBER) continue;
            sb_printf(&b, "    _v.push(\"%s\")\n", m->name);
        }
    }
    sb_puts(&b, "    ret _v\nend\n");
    {
        const char *r = arena_strdup(s->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

const char *sema_derive_enum_name(sema_t *s, type_t *t, const src_span_t *span)
{
    (void)span;
    return derive_one(s, t, "_enum_name_", name_source);
}

const char *sema_derive_enum_fromstring(sema_t *s, type_t *t, const src_span_t *span)
{
    (void)span;
    return derive_one(s, t, "_enum_fromstring_", fromstring_source);
}

const char *sema_derive_enum_names(sema_t *s, type_t *t, const src_span_t *span)
{
    (void)span;
    return derive_one(s, t, "_enum_names_", names_source);
}
