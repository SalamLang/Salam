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
 * The Salam source of a derived stringify function. See sema_derive_str.c for
 * why this is text rather than hand-built AST.
 *
 * What a printed value looks like:
 *
 *   struct       Point {x = 10, y = 15}
 *   array/vector [1, 2, 3]
 *   hashmap      {"a": 1, "b": 2}
 *   nested str   "ali"          (quoted, so an empty element is visible)
 *   pointer      null | <ptr>   (never followed - a cycle would not end)
 *
 * A struct that defines `to_str(): str` gets that called instead, so a type
 * can present itself however it likes.
 */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "semantic/sema_derive_internal.h"

#define IND "    "

static const char *fmt(sema_t *s, const char *f, ...)
{
    char buf[1024];
    va_list ap;
    va_start(ap, f);
    sal_vsnprintf(buf, sizeof buf, f, ap);
    va_end(ap);
    return arena_strdup(s->a, buf);
}

static bool is_plain(const type_t *t)
{
    return t->kind == TY_BOOL || t->kind == TY_CHAR || t->kind == TY_UCHAR ||
           type_is_numeric(t);
}

static func_sig_t *method_sig(symbol_t *sym, const char *name, size_t nparams)
{
    symbol_t *m = sym->members ? scope_lookup_local(sym->members, name) : NULL;
    size_t i = 0;
    if (!m || m->kind != SYM_METHOD) return NULL;
    for (; i < m->overloads.len; i++) {
        func_sig_t *sig = (func_sig_t *)m->overloads.data[i];
        if (sig->params.len == nparams) return sig;
    }
    return NULL;
}

static bool user_to_str(symbol_t *sym)
{
    func_sig_t *sig = method_sig(sym, "to_str", 0);
    return sig && sig->ret && sig->ret->kind == TY_STR;
}

/*
 * A struct that behaves like a sequence - `len()`/`size()` plus `get(int)`.
 * Recognised by shape rather than by name so std/collections' Vector, Deque,
 * LinkedList and anything a user writes the same way all print as `[...]`
 * instead of exposing the buffer and capacity behind them.
 */
static bool struct_seq(symbol_t *sym, type_t **elem, const char **count)
{
    static const char *const names[] = {"len", "size"};
    func_sig_t *get = method_sig(sym, "get", 1);
    size_t i = 0;
    if (!get || !get->ret || !type_is_integer((type_t *)get->params.data[0]))
        return false;
    for (; i < 2; i++) {
        func_sig_t *n = method_sig(sym, names[i], 0);
        if (!n || !n->ret || !type_is_integer(n->ret)) continue;
        *elem = get->ret;
        *count = names[i];
        return true;
    }
    return false;
}

/* The same for a keyed container: `iter()` handing back a cursor that answers
 * has_next/key/value/next, which is what std/collections' HashMap offers. */
static bool struct_map(symbol_t *sym, type_t **key, type_t **val)
{
    func_sig_t *it = method_sig(sym, "iter", 0);
    symbol_t *cur = it ? struct_sym_of(it->ret) : NULL;
    func_sig_t *hn = cur ? method_sig(cur, "has_next", 0) : NULL;
    func_sig_t *k = cur ? method_sig(cur, "key", 0) : NULL;
    func_sig_t *v = cur ? method_sig(cur, "value", 0) : NULL;
    if (!hn || !hn->ret || hn->ret->kind != TY_BOOL) return false;
    if (!k || !k->ret || !v || !v->ret || !method_sig(cur, "next", 0)) return false;
    *key = k->ret;
    *val = v->ret;
    return true;
}

/* ---------------------------------------------------------------- spelling */

static const char *pkg_qualified(sema_t *s, symbol_t *sym)
{
    size_t i = 0;
    for (; i < s->global->symbols.len; i++) {
        symbol_t *pk = (symbol_t *)s->global->symbols.data[i];
        if (pk->kind != SYM_PACKAGE || !pk->members || !pk->name) continue;
        if (scope_lookup_local(pk->members, sym->name) != sym) continue;
        if (!sym->is_pub) return NULL;
        return fmt(s, "%s.%s", pk->name, sym->name);
    }
    return NULL;
}

static const char *struct_spelling(sema_t *s, symbol_t *sym)
{
    if (!sym || !sym->name) return NULL;
    if (scope_lookup(s->global, sym->name) == sym) return sym->name;
    return pkg_qualified(s, sym);
}

static const char *array_spelling(sema_t *s, type_t *t)
{
    char dims[128] = "";
    size_t n = 0;
    const type_t *cur = t;
    while (cur && cur->kind == TY_ARRAY) {
        n = sal_catf(dims, sizeof dims, n, "[%zu]", cur->length);
        cur = cur->elem;
    }
    {
        const char *base = derive_type_spelling(s, (type_t *)CONST_CAST(cur));
        return base ? fmt(s, "%s%s", base, dims) : NULL;
    }
}

const char *derive_type_spelling(sema_t *s, type_t *t)
{
    const char *a, *b;
    if (!t) return NULL;
    if (is_plain(t) || t->kind == TY_STR) return type_to_string(s->tc, t);
    switch (t->kind) {
    case TY_ENUM:
    case TY_STRUCT:
        return struct_spelling(s, (symbol_t *)t->decl);
    case TY_PTR:
        a = derive_type_spelling(s, t->pointee);
        return a ? fmt(s, "%s*", a) : NULL;
    case TY_ARRAY:
        return array_spelling(s, t);
    case TY_SLICE:
        a = derive_type_spelling(s, t->elem);
        return a ? fmt(s, "%s[:]", a) : NULL;
    case TY_VEC:
        a = derive_type_spelling(s, t->elem);
        return a ? fmt(s, "Vector<%s>", a) : NULL;
    case TY_MAP:
        a = derive_type_spelling(s, t->key);
        b = derive_type_spelling(s, t->elem);
        return (a && b) ? fmt(s, "HashMap<%s, %s>", a, b) : NULL;
    default:
        return NULL;
    }
}

/* ------------------------------------------------------------- one value */

/*
 * Never fails. A member no function can be derived for - a File handle, a
 * closure, a struct another package keeps private - renders as `<its type>`
 * rather than costing the whole struct its printability: seeing the other
 * nine fields of a ten-field struct beats being told to print them by hand.
 */
void derive_append(sema_t *s, sb_t *b, type_t *t, const char *expr,
                   const src_span_t *span)
{
    if (!t) return;
    if (is_plain(t)) {
        sb_printf(b, IND "%s = %s + (%s)\n", DV_BUF, DV_BUF, expr);
        return;
    }
    if (t->kind == TY_STR) {
        sb_printf(b, IND "%s = %s + \"\\\"\" + (%s) + \"\\\"\"\n", DV_BUF, DV_BUF, expr);
        return;
    }
    if (t->kind == TY_ENUM) {
        sb_printf(b, IND "%s = %s + ((%s) as i64)\n", DV_BUF, DV_BUF, expr);
        return;
    }
    if (t->kind == TY_PTR) {
        sb_printf(b, IND "if (%s) == null: %s = %s + \"null\"\n", expr, DV_BUF, DV_BUF);
        sb_printf(b, IND "else: %s = %s + \"<ptr>\" end\n", DV_BUF, DV_BUF);
        return;
    }
    {
        const char *fn = sema_derive_stringify(s, t, span);
        if (fn)
            sb_printf(b, IND "%s = %s + %s(%s)\n", DV_BUF, DV_BUF, fn, expr);
        else
            sb_printf(b, IND "%s = %s + \"<%s>\"\n", DV_BUF, DV_BUF,
                      type_to_string(s->tc, t));
    }
}

/* ---------------------------------------------------------------- bodies */

/* What the struct calls itself in the printed text. A generic instance is
 * named `Box_i32` internally; it is shown as the `Box<i32>` the programmer
 * wrote instead. */
static const char *display_name(sema_t *s, symbol_t *sym)
{
    sb_t b;
    size_t i = 0;
    const char *r;
    if (!sym->generic_base || sym->generic_args.len == 0) return sym->name;
    sb_init(&b);
    sb_puts(&b, sym->generic_base);
    sb_putc(&b, '<');
    for (; i < sym->generic_args.len; i++) {
        if (i) sb_puts(&b, ", ");
        sb_puts(&b, type_to_string(s->tc, (type_t *)sym->generic_args.data[i]));
    }
    sb_putc(&b, '>');
    r = arena_strdup(s->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

static void body_struct(sema_t *s, sb_t *b, type_t *t, const src_span_t *span)
{
    symbol_t *sym = (symbol_t *)t->decl;
    bool first = true;
    size_t i = 0;
    sb_printf(b, IND "mut %s := \"%s {\"\n", DV_BUF, display_name(s, sym));
    for (; sym->members && i < sym->members->symbols.len; i++) {
        symbol_t *f = (symbol_t *)sym->members->symbols.data[i];
        if (f->kind != SYM_FIELD) continue;
        sb_printf(b, IND "%s = %s + \"%s%s = \"\n", DV_BUF, DV_BUF, first ? "" : ", ",
                  f->name);
        derive_append(s, b, f->type, fmt(s, DV_PARAM ".%s", f->name), span);
        first = false;
    }
    sb_printf(b, IND "%s = %s + \"}\"\n", DV_BUF, DV_BUF);
}

/* Vector, fixed array and slice all render as `[a, b, c]`; they differ only
 * in how the count and an element are spelled. */
static void body_seq(sema_t *s, sb_t *b, type_t *elem, const char *count, const char *at,
                     const src_span_t *span)
{
    sb_printf(b, IND "mut %s := \"[\"\n", DV_BUF);
    sb_puts(b, IND "mut _i := 0\n");
    sb_printf(b, IND "until _i < %s:\n", count);
    sb_printf(b, IND IND "if _i > 0: %s = %s + \", \" end\n", DV_BUF, DV_BUF);
    sb_puts(b, IND);
    derive_append(s, b, elem, at, span);
    sb_puts(b, IND IND "_i = _i + 1\n");
    sb_puts(b, IND "end\n");
    sb_printf(b, IND "%s = %s + \"]\"\n", DV_BUF, DV_BUF);
}

static void body_map(sema_t *s, sb_t *b, type_t *key, type_t *val, const src_span_t *span)
{
    sb_printf(b, IND "mut %s := \"{\"\n", DV_BUF);
    sb_puts(b, IND "mut _n := 0\n");
    sb_puts(b, IND "mut _it := " DV_PARAM ".iter()\n");
    sb_puts(b, IND "until _it.has_next():\n");
    sb_printf(b, IND IND "if _n > 0: %s = %s + \", \" end\n", DV_BUF, DV_BUF);
    sb_puts(b, IND);
    derive_append(s, b, key, "_it.key()", span);
    sb_printf(b, IND IND "%s = %s + \": \"\n", DV_BUF, DV_BUF);
    sb_puts(b, IND);
    derive_append(s, b, val, "_it.value()", span);
    sb_puts(b, IND IND "_n = _n + 1\n");
    sb_puts(b, IND IND "_it.next()\n");
    sb_puts(b, IND "end\n");
    sb_printf(b, IND "%s = %s + \"}\"\n", DV_BUF, DV_BUF);
}

static void body_of_struct(sema_t *s, sb_t *b, type_t *t, const src_span_t *span)
{
    symbol_t *sym = (symbol_t *)t->decl;
    type_t *k, *v;
    const char *count;
    if (user_to_str(sym)) {
        sb_printf(b, IND "mut %s := " DV_PARAM ".to_str()\n", DV_BUF);
        return;
    }
    if (struct_map(sym, &k, &v)) {
        body_map(s, b, k, v, span);
        return;
    }
    if (struct_seq(sym, &v, &count)) {
        body_seq(s, b, v, fmt(s, DV_PARAM ".%s()", count), DV_PARAM ".get(_i)", span);
        return;
    }
    body_struct(s, b, t, span);
}

static bool body_of(sema_t *s, sb_t *b, type_t *t, const src_span_t *span)
{
    switch (t->kind) {
    case TY_STRUCT:
        body_of_struct(s, b, t, span);
        return true;
    case TY_VEC:
        body_seq(s, b, t->elem, DV_PARAM ".len()", DV_PARAM ".get(_i)", span);
        return true;
    case TY_ARRAY:
    case TY_SLICE:
        body_seq(s, b, t->elem, "len(" DV_PARAM ")", DV_PARAM "[_i]", span);
        return true;
    case TY_MAP:
        body_map(s, b, t->key, t->elem, span);
        return true;
    default:
        return false;
    }
}

/* ------------------------------------------------- shared with the JSON derive */

func_sig_t *derive_method_sig(symbol_t *sym, const char *name, size_t nparams)
{
    return method_sig(sym, name, nparams);
}

bool derive_struct_seq(symbol_t *sym, type_t **elem, const char **count)
{
    return struct_seq(sym, elem, count);
}

bool derive_struct_map(symbol_t *sym, type_t **key, type_t **val)
{
    return struct_map(sym, key, val);
}

bool derive_is_plain(const type_t *t)
{
    return is_plain(t);
}

const char *derive_display_name(sema_t *s, symbol_t *sym)
{
    return display_name(s, sym);
}

const char *derive_fn_source(sema_t *s, type_t *t, const char *fname,
                             const src_span_t *span)
{
    const char *spelling = derive_type_spelling(s, t);
    if (!spelling) return NULL;
    sb_t b;
    sb_init(&b);
    sb_printf(&b, "func %s(" DV_PARAM ": %s): str:\n", fname, spelling);
    if (!body_of(s, &b, t, span)) {
        sb_free(&b);
        return NULL;
    }
    sb_printf(&b, IND "ret %s\n", DV_BUF);
    sb_puts(&b, "end\n");
    {
        const char *r = arena_strdup(s->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}
