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
 * Registry and installation shared by the derive passes. See
 * sema_derive_core.h for what each entry point is for.
 */

#include "core/prelude.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema_derive_core.h"
#include "source/source.h"

struct derived_t {
    scope_t *scope;
    const char *name;
    ast_node_t *decl;
};

bool derive_dump_enabled(void)
{
    static int cached = -1;
    if (cached < 0) {
        const char *v = getenv("SALAM_DERIVE_DUMP");
        cached = (v && *v && strcmp(v, "0") != 0) ? 1 : 0;
    }
    return cached == 1;
}

void derive_key_append(sb_t *b, const char *ts)
{
    const char *p = ts;
    for (; *p; p++) {
        char c = *p;
        if (c == '<' || c == ',' || c == '.' || c == '_')
            sb_putc(b, '_');
        else if (c == '[')
            sb_puts(b, "_a");
        else if (c == '*')
            sb_puts(b, "_p");
        else if (c == '>' || c == ']' || c == ' ')
            continue;
        else
            sb_putc(b, c);
    }
}

const char *derive_fn_name(sema_t *s, const char *prefix, type_t *t)
{
    sb_t b;
    sb_init(&b);
    sb_puts(&b, prefix);
    derive_key_append(&b, type_to_string(s->tc, t));
    {
        const char *r = arena_strdup(s->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

static derived_t *registry_find(sema_t *s, const char *name)
{
    size_t i = 0;
    for (; i < s->derived.len; i++) {
        derived_t *d = (derived_t *)s->derived.data[i];
        if (d->scope == s->global && strcmp(d->name, name) == 0) return d;
    }
    return NULL;
}

bool derive_claimed(sema_t *s, const char *name)
{
    return registry_find(s, name) != NULL;
}

bool sema_is_derived_decl(sema_t *s, ast_node_t *d)
{
    size_t i = 0;
    for (; i < s->derived.len; i++)
        if (((derived_t *)s->derived.data[i])->decl == d) return true;
    return false;
}

derived_t *derive_claim(sema_t *s, const char *name)
{
    derived_t *d = (derived_t *)arena_alloc(s->a, sizeof *d);
    d->scope = s->global;
    d->name = name;
    d->decl = NULL;
    vec_push(s->a, &s->derived, d);
    return d;
}

void derive_unclaim(sema_t *s, derived_t *ent)
{
    size_t i = 0;
    for (; i < s->derived.len; i++) {
        if (s->derived.data[i] != ent) continue;
        {
            size_t j = i + 1;
            for (; j < s->derived.len; j++)
                s->derived.data[j - 1] = s->derived.data[j];
        }
        s->derived.len--;
        return;
    }
}

void derive_set_decl(derived_t *ent, ast_node_t *fn)
{
    ent->decl = fn;
}

const char *derive_pkg_name_of(sema_t *s, scope_t *scope)
{
    size_t i = 0;
    if (!scope || !s->global) return NULL;
    for (; i < s->global->symbols.len; i++) {
        symbol_t *pk = (symbol_t *)s->global->symbols.data[i];
        if (pk->kind != SYM_PACKAGE || pk->members != scope) continue;
        return pk->pkgname ? pk->pkgname : pk->name;
    }
    return NULL;
}

scope_t *derive_home_of(scope_t *from, const char *probe)
{
    scope_t *sc = from;
    for (; sc; sc = sc->parent)
        if (scope_lookup_local(sc, probe)) return sc;
    return NULL;
}

/*
 * A derived body has to see two things at once: the program's own
 * declarations (the struct being encoded) and the runtime layer of the module
 * the codec belongs to (its private _d* helpers). Those live in separate
 * scope trees - a package is analysed on its own and merged in as a symbol,
 * so its scope's ancestors are its own globals, not the program's.
 *
 * The bridge is a scope holding the module's symbols with the program's
 * global scope as its parent, so a lookup finds the helper first and the
 * user's types straight after. One per module per run.
 */
static scope_t *bridge_for(sema_t *s, scope_t *home)
{
    static scope_t *cached_home;
    static scope_t *cached_bridge;
    static scope_t *cached_global;
    scope_t *sc;
    size_t i = 0;

    if (cached_home == home && cached_global == s->global) return cached_bridge;
    sc = scope_new(s->a, SCOPE_BLOCK, s->global);
    sc->label = "<derive>";
    for (; i < home->symbols.len; i++)
        scope_define(s->a, sc, (symbol_t *)home->symbols.data[i]);
    cached_home = home;
    cached_global = s->global;
    cached_bridge = sc;
    return sc;
}

static ast_node_t *parse_derived(sema_t *s, const char *text, const char *fname)
{
    source_file_t sf;
    sf.path = arena_strdup(s->a, fname);
    sf.text = (char *)CONST_CAST(text);
    sf.len = strlen(text);
    logger_add_diag_source(s->log, sf.path, sf.text, sf.len);
    {
        const langpack_t *pack = langpack_load("en");
        token_stream_t *toks = NULL;
        ast_node_t *prog = NULL;
        if (!lexer_run(s->a, s->log, pack, &sf, &toks)) return NULL;
        if (!parser_run(s->a, s->log, toks, &prog)) return NULL;
        return prog;
    }
}

/* Register one parsed `func` as a synthetic top-level of the program being
 * analysed: AST into the program list, symbol into the global scope,
 * declaration onto the pending queue so its body is checked with everything
 * else instantiation produces. */
static void install_fn(sema_t *s, ast_node_t *fn, scope_t *home, const char *pkg)
{
    symbol_t *fsym;
    fn->synthetic = true;
    fn->origin_lang = "en";
    fn->home_pkg = pkg;
    vec_push(s->a, &s->program->list, fn);
    fsym = get_or_make_func(s, s->global, fn->name, SYM_FUNC);
    if (home)
        fsym->home = home;
    else if (!fsym->home)
        fsym->home = s->global;
    fsym->used = true;
    vec_push(s->a, &fsym->overloads, build_sig(s, fn, NULL));
    vec_push(s->a, &s->pending, fn);
}

/* Signature types resolve against the global scope in English: a local named
 * after the struct must not shadow it, and `Vector` must not be held to the
 * Farsi spelling rule that applies to what the user typed. */
ast_node_t *derive_install_source(sema_t *s, const char *text, const char *want,
                                  scope_t *home)
{
    ast_node_t *prog;
    scope_t *save_cur = s->cur;
    const char *save_lang = s->lang;
    ast_node_t *found = NULL;

    if (derive_dump_enabled()) {
        const char *pk = derive_pkg_name_of(s, home);
        fprintf(stderr, "--- derived (home pkg: %s) ---\n%s\n", pk ? pk : "(none)", text);
    }

    prog = parse_derived(s, text, "<derived>");
    if (!prog) return NULL;

    {
        const char *pkg = derive_pkg_name_of(s, home);
        if (home) home = bridge_for(s, home);
        s->cur = home ? home : s->global;
        s->lang = "en";
        {
            size_t i = 0;
            for (; i < prog->list.len; i++) {
                ast_node_t *d = (ast_node_t *)prog->list.data[i];
                if (d->kind != AST_FUNC_DEF || !d->name) continue;
                install_fn(s, d, home, pkg);
                if (strcmp(d->name, want) == 0) found = d;
            }
        }
    }
    s->cur = save_cur;
    s->lang = save_lang;
    return found;
}
