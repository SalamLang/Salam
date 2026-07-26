/*
 * Salam Programming Language (2024–2026)
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

#include "core/prelude.h"
#include "semantic/symbol.h"
#include "core/sal_format.h"

#define SCOPE_INDEX_THRESHOLD 16

scope_t *scope_new(arena_t *a, scope_kind_t kind, scope_t *parent)
{
    scope_t *s = (scope_t *)arena_alloc(a, sizeof(*s));
    memset(s, 0, sizeof(*s));
    s->kind = kind;
    s->parent = parent;
    vec_init(&s->symbols);
    if (parent) {
        s->self_type = parent->self_type;
        s->func = parent->func;
    }
    return s;
}

symbol_t *symbol_new(arena_t *a, sym_kind_t kind, const char *name)
{
    symbol_t *sym = (symbol_t *)arena_alloc(a, sizeof(*sym));
    memset(sym, 0, sizeof(*sym));
    sym->kind = kind;
    sym->name = name;
    vec_init(&sym->overloads);
    return sym;
}

static size_t scope_hash(const char *s)
{
    uint64_t h = 1469598103934665603ULL;
    const unsigned char *p = (const unsigned char *)s;
    for (; *p; p++) {
        h ^= *p;
        h *= 1099511628211ULL;
    }
    return (size_t)h;
}

static void scope_index_insert(scope_t *sc, symbol_t *sym)
{
    size_t mask = sc->hcap - 1;
    size_t i = scope_hash(sym->name) & mask;
    while (sc->hindex[i])
        i = (i + 1) & mask;
    sc->hindex[i] = sym;
}

static void scope_index_build(arena_t *a, scope_t *sc, size_t want)
{
    size_t nc = 32;
    while (nc < want * 2)
        nc *= 2;
    sc->hindex = (void **)arena_alloc(a, salam_size_mul(nc, sizeof(void *)));
    memset(sc->hindex, 0, salam_size_mul(nc, sizeof(void *)));
    sc->hcap = nc;
    {
        size_t i = 0;
        for (; i < sc->symbols.len; i++)
            scope_index_insert(sc, (symbol_t *)sc->symbols.data[i]);
    }
}

symbol_t *scope_lookup_local(scope_t *s, const char *name)
{
    if (s->hindex) {
        size_t mask = s->hcap - 1;
        size_t i = scope_hash(name) & mask;
        for (; s->hindex[i]; i = (i + 1) & mask) {
            symbol_t *sym = (symbol_t *)s->hindex[i];
            if (strcmp(sym->name, name) == 0) return sym;
        }
        return NULL;
    }
    {
        size_t i = 0;
        for (; i < s->symbols.len; i++) {
            symbol_t *sym = (symbol_t *)s->symbols.data[i];
            if (strcmp(sym->name, name) == 0) return sym;
        }
    }
    return NULL;
}

symbol_t *scope_lookup(scope_t *s, const char *name)
{
    return scope_lookup_where(s, name, NULL);
}

symbol_t *scope_lookup_where(scope_t *s, const char *name, scope_t **where)
{
    {
        scope_t *cur = s;
        for (; cur; cur = cur->parent) {
            symbol_t *sym = scope_lookup_local(cur, name);
            if (sym) {
                if (where) *where = cur;
                return sym;
            }

            if (cur->aux) {
                sym = scope_lookup_where(cur->aux, name, where);
                if (sym) return sym;
            }
        }
    }
    return NULL;
}

symbol_t *scope_define(arena_t *a, scope_t *s, symbol_t *sym)
{
    symbol_t *existing = scope_lookup_local(s, sym->name);
    if (existing) return existing;
    vec_push(a, &s->symbols, sym);
    if (s->hindex) {
        if (s->symbols.len * 4 >= s->hcap * 3)
            scope_index_build(a, s, s->symbols.len);
        else
            scope_index_insert(s, sym);
    } else if (s->symbols.len >= SCOPE_INDEX_THRESHOLD) {
        scope_index_build(a, s, s->symbols.len);
    }
    return NULL;
}

static const char *type_code(arena_t *a, const type_t *t)
{
    if (!t) return "v";
    switch (t->kind) {
    case TY_VOID:
        return "v";
    case TY_BOOL:
        return "b";
    case TY_CHAR:
        return "c";
    case TY_STR:
        return "s";
    case TY_UCHAR:
        return "uc";
    case TY_NULL:
        return "n";
    case TY_I8:
        return "i8";
    case TY_I16:
        return "i16";
    case TY_I32:
        return "i32";
    case TY_I64:
        return "i64";
    case TY_U8:
        return "u8";
    case TY_U16:
        return "u16";
    case TY_U32:
        return "u32";
    case TY_U64:
        return "u64";
    case TY_SIZE:
        return "size";
    case TY_F32:
        return "f32";
    case TY_F64:
        return "f64";
    case TY_STRUCT:
    case TY_ENUM:
        return t->name ? t->name : "T";
    case TY_PTR: {
        char buf[96];
        sal_snprintf(buf, sizeof(buf), "P%s", type_code(a, t->pointee));
        return arena_strdup(a, buf);
    }
    case TY_ARRAY: {
        char buf[96];
        sal_snprintf(buf, sizeof(buf), "A%zu%s", t->length, type_code(a, t->elem));
        return arena_strdup(a, buf);
    }
    default:
        return "x";
    }
}

static size_t buf_appendf(char *buf, size_t cap, size_t n, const char *fmt, ...)
{
    if (n >= cap) return cap - 1;
    va_list ap;
    va_start(ap, fmt);
    int w = sal_vsnprintf(buf + n, cap - n, fmt, ap);
    va_end(ap);
    if (w < 0) return n;
    n += (size_t)w;
    return n >= cap ? cap - 1 : n;
}

const char *mangle_func(arena_t *a, const char *struct_name, const char *fn,
                        const vec_t *param_types)
{
    char buf[512];
    size_t n = 0;
    if (struct_name) n = buf_appendf(buf, sizeof(buf), n, "S_%s_", struct_name);
    n = buf_appendf(buf, sizeof(buf), n, "%s__", fn);
    if (param_types && param_types->len) {
        {
            size_t i = 0;
            for (; i < param_types->len && n < sizeof(buf) - 16; i++) {
                const type_t *t = (const type_t *)param_types->data[i];
                n = buf_appendf(buf, sizeof(buf), n, "%s%s", i ? "_" : "",
                                type_code(a, t));
            }
        }
    } else {
        n = buf_appendf(buf, sizeof(buf), n, "void");
    }
    return arena_strdup(a, buf);
}

const char *impl_owner_key(arena_t *a, const char *typestr)
{
    char buf[256];
    sal_snprintf(buf, sizeof(buf), "impl@%s", typestr ? typestr : "");
    return arena_strdup(a, buf);
}
