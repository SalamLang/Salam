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
 * `println p` on a struct, an array, a Vector or a HashMap.
 *
 * printf has no spelling for an aggregate, so the value is turned into a
 * string first: this pass writes a `func _strof_T(_v: T): str` that appends
 * every field or element to a buffer, installs it in the program being
 * analysed, and rewrites the print argument into a call to it. A field that
 * is itself an aggregate gets its own function, derived on the way through,
 * so a Vector of structs nests without any of this knowing how deep it goes.
 *
 * The function is generated as Salam source and put through the lexer and
 * parser, rather than assembled as AST by hand. Two reasons: what a printed
 * value looks like is then readable in one place instead of spread over node
 * constructors, and the result is an ordinary synthetic top-level
 * declaration - exactly what a generic instantiation is - so the C, LLVM, JS
 * and interpreter backends emit it without knowing this pass exists.
 */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "core/sb.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema_derive_internal.h"
#include "source/source.h"

typedef struct {
    scope_t *scope;
    const char *name;
    ast_node_t *decl;
} derived_t;

/* Identifier characters for a type's key: `Vector<str>` -> `Vector_str`,
 * `Point[3]` -> `Point_a3`, `geo.Point` -> `geo_Point`. Only has to be
 * injective enough that two different types never collide. */
static void key_append(sb_t *b, const char *ts)
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

static const char *derive_name(sema_t *s, type_t *t)
{
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "_strof_");
    key_append(&b, type_to_string(s->tc, t));
    const char *r = arena_strdup(s->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

/* The name if this scope already has (or is midway through) deriving it. The
 * registry is the authority rather than a scope lookup: a user function that
 * happens to be called `_strof_Point` must not be mistaken for one of ours. */
static derived_t *registry_find(sema_t *s, const char *name)
{
    size_t i = 0;
    for (; i < s->derived.len; i++) {
        derived_t *d = (derived_t *)s->derived.data[i];
        if (d->scope == s->global && strcmp(d->name, name) == 0) return d;
    }
    return NULL;
}

bool sema_is_derived_decl(sema_t *s, ast_node_t *d)
{
    size_t i = 0;
    for (; i < s->derived.len; i++)
        if (((derived_t *)s->derived.data[i])->decl == d) return true;
    return false;
}

static derived_t *registry_claim(sema_t *s, const char *name)
{
    derived_t *d = (derived_t *)arena_alloc(s->a, sizeof *d);
    d->scope = s->global;
    d->name = name;
    d->decl = NULL;
    vec_push(s->a, &s->derived, d);
    return d;
}

static void registry_drop(sema_t *s, derived_t *ent)
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

static ast_node_t *parse_derived(sema_t *s, const char *text, const char *fname)
{
    source_file_t sf;
    sf.path = arena_strdup(s->a, fname);
    sf.text = (char *)CONST_CAST(text);
    sf.len = strlen(text);
    logger_add_diag_source(s->log, sf.path, sf.text, sf.len);
    const langpack_t *pack = langpack_load("en");
    token_stream_t *toks = NULL;
    if (!lexer_run(s->a, s->log, pack, &sf, &toks)) return NULL;
    ast_node_t *prog = NULL;
    if (!parser_run(s->a, s->log, toks, &prog)) return NULL;
    return prog;
}

/* Register one parsed `func` as a synthetic top-level of the program being
 * analysed: AST into the program list, symbol into the global scope,
 * declaration onto the pending queue so its body is checked with everything
 * else instantiation produces. */
static void install_fn(sema_t *s, ast_node_t *fn)
{
    fn->synthetic = true;
    fn->origin_lang = "en";
    vec_push(s->a, &s->program->list, fn);
    symbol_t *fsym = get_or_make_func(s, s->global, fn->name, SYM_FUNC);
    if (!fsym->home) fsym->home = s->global;
    fsym->used = true;
    vec_push(s->a, &fsym->overloads, build_sig(s, fn, NULL));
    vec_push(s->a, &s->pending, fn);
}

/* Signature types resolve against the global scope in English: a local named
 * after the struct must not shadow it, and `Vector` must not be held to the
 * Farsi spelling rule that applies to what the user typed. */
static ast_node_t *install_all(sema_t *s, ast_node_t *prog, const char *want)
{
    scope_t *save_cur = s->cur;
    const char *save_lang = s->lang;
    ast_node_t *found = NULL;
    s->cur = s->global;
    s->lang = "en";
    {
        size_t i = 0;
        for (; i < prog->list.len; i++) {
            ast_node_t *d = (ast_node_t *)prog->list.data[i];
            if (d->kind != AST_FUNC_DEF || !d->name) continue;
            install_fn(s, d);
            if (strcmp(d->name, want) == 0) found = d;
        }
    }
    s->cur = save_cur;
    s->lang = save_lang;
    return found;
}

const char *sema_derive_stringify(sema_t *s, type_t *t, const src_span_t *span)
{
    if (!t || !s->program) return NULL;
    const char *fname = derive_name(s, t);
    if (registry_find(s, fname)) return fname;
    if (scope_lookup_local(s->global, fname)) return NULL;

    /* Claimed before the body is generated: a struct holding a Vector of
     * itself asks for this same function again from inside derive_fn_source,
     * and has to be told it already exists rather than recurse forever. */
    derived_t *ent = registry_claim(s, fname);
    const char *text = derive_fn_source(s, t, fname, span);
    if (!text) {
        registry_drop(s, ent);
        return NULL;
    }
    ast_node_t *prog = parse_derived(s, text, "<derived>");
    ast_node_t *fn = prog ? install_all(s, prog, fname) : NULL;
    if (!fn) {
        registry_drop(s, ent);
        return NULL;
    }
    ent->decl = fn;
    return fname;
}
