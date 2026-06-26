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

#include "interp/interp_internal.h"
#include "core/sal_format.h"
#include <math.h>

const char *afmt(interp_t *I, const char *fmt, ...)
{
    va_list ap, ap2;
    va_start(ap, fmt); SAL_VA_COPY(ap2, ap);
    int n = sal_vsnprintf(NULL, 0, fmt, ap); va_end(ap);
    char *buf = (char *)arena_alloc(I->a, (size_t)n + 1);
    sal_vsnprintf(buf, (size_t)n + 1, fmt, ap2); va_end(ap2);
    return buf;
}

bool to_bool(value_t v)
{
    switch (v.kind) {
        case VAL_BOOL:  return v.as.b;
        case VAL_INT:
        case VAL_CHAR:  return v.as.i != 0;
        case VAL_FLOAT: return v.as.f != 0.0;
        case VAL_NULL:  return false;
        case VAL_STR:   return v.as.s && v.as.s[0] != '\0';
        default:        return true;
    }
}
double to_float(value_t v)
{
    switch (v.kind) {
        case VAL_FLOAT: return v.as.f;
        case VAL_INT:
        case VAL_CHAR:  return (double)v.as.i;
        case VAL_BOOL:  return v.as.b ? 1.0 : 0.0;
        default:        return 0.0;
    }
}

int64_t to_int(value_t v)
{
    switch (v.kind) {
        case VAL_INT:
        case VAL_CHAR:  return v.as.i;
        case VAL_FLOAT: return (int64_t)v.as.f;
        case VAL_BOOL:  return v.as.b ? 1 : 0;
        default:        return 0;
    }
}

static bool is_intlike(value_t v) { return v.kind == VAL_INT || v.kind == VAL_CHAR; }

static bool is_number(value_t v)  { return v.kind == VAL_INT || v.kind == VAL_CHAR || v.kind == VAL_FLOAT; }

static void utf8_put(sb_t *b, int64_t cp)
{
    unsigned c = (unsigned)cp;
    if (c < 0x80) { sb_putc(b, (char)c); }
    else if (c < 0x800) { sb_putc(b, (char)(0xC0 | (c >> 6))); sb_putc(b, (char)(0x80 | (c & 0x3F))); }
    else if (c < 0x10000) { sb_putc(b, (char)(0xE0 | (c >> 12))); sb_putc(b, (char)(0x80 | ((c >> 6) & 0x3F)));
                            sb_putc(b, (char)(0x80 | (c & 0x3F))); }
    else { sb_putc(b, (char)(0xF0 | (c >> 18))); sb_putc(b, (char)(0x80 | ((c >> 12) & 0x3F)));
           sb_putc(b, (char)(0x80 | ((c >> 6) & 0x3F))); sb_putc(b, (char)(0x80 | (c & 0x3F))); }
}

const char *to_str(interp_t *I, value_t v)
{
    switch (v.kind) {
        case VAL_STR:   return v.as.s ? v.as.s : "";
        case VAL_INT:   return afmt(I, "%lld", (long long)v.as.i);
        case VAL_CHAR:  { sb_t b; sb_init(&b); utf8_put(&b, v.as.i);
                          const char *r = arena_strdup(I->a, sb_cstr(&b)); sb_free(&b); return r; }
        case VAL_BOOL:  return v.as.b ? "true" : "false";
        case VAL_NULL:  return "null";
        case VAL_FLOAT: {
            char buf[64];
            snprintf(buf, sizeof buf, "%.15g", v.as.f);
            return arena_strdup(I->a, buf);
        }
        case VAL_ARRAY: {
            sb_t b; sb_init(&b); sb_putc(&b, '[');
            { size_t i = 0; for (; i < v.as.arr->len; i++) {
                if (i) sb_puts(&b, ", ");
                sb_puts(&b, to_str(I, v.as.arr->data[i]));
            } }
            sb_putc(&b, ']');
            const char *r = arena_strdup(I->a, sb_cstr(&b)); sb_free(&b); return r;
        }
        case VAL_STRUCT: {
            sb_t b; sb_init(&b);
            sb_puts(&b, v.as.st->type_name); sb_puts(&b, "{");
            { size_t i = 0; for (; i < v.as.st->nfields; i++) {
                if (i) sb_puts(&b, ", ");
                sb_puts(&b, v.as.st->fields[i].name); sb_puts(&b, "=");
                sb_puts(&b, to_str(I, v.as.st->fields[i].val));
            } }
            sb_puts(&b, "}");
            const char *r = arena_strdup(I->a, sb_cstr(&b)); sb_free(&b); return r;
        }
        case VAL_MAP:     return "<map>";
        case VAL_MAPITER: return "<mapiter>";
        case VAL_FUNC:    return "<func>";
        case VAL_MODULE:  return v.as.mod ? afmt(I, "<module %s>", v.as.mod->name) : "<module>";
    }
    return "";
}
sarray_t *array_new(interp_t *I, size_t cap)
{
    sarray_t *a = (sarray_t *)arena_alloc(I->a, sizeof *a);
    a->len = 0; a->cap = cap < 4 ? 4 : cap;
    a->data = (value_t *)arena_alloc(I->a, salam_size_mul(a->cap, sizeof(value_t)));
    return a;
}

void array_push(interp_t *I, sarray_t *a, value_t v)
{
    if (a->len == a->cap) {
        size_t nc = salam_grow_cap(a->cap, a->len + 1, 4);
        value_t *nd = (value_t *)arena_alloc(I->a, salam_size_mul(nc, sizeof(value_t)));
        memcpy(nd, a->data, salam_size_mul(a->len, sizeof(value_t)));
        a->data = nd; a->cap = nc;
    }
    a->data[a->len++] = v;
}
value_t mk_array(interp_t *I, sarray_t *a) { (void)I; value_t v; v.kind = VAL_ARRAY; v.as.arr = a; return v; }
smap_t *map_new(interp_t *I)
{
    smap_t *m = (smap_t *)arena_alloc(I->a, sizeof *m);
    m->count = 0; m->cap = 8;
    m->entries = (smap_entry_t *)arena_alloc(I->a, salam_size_mul(m->cap, sizeof(smap_entry_t)));
    memset(m->entries, 0, salam_size_mul(m->cap, sizeof(smap_entry_t)));
    return m;
}

static bool key_eq(value_t a, value_t b)
{
    if (a.kind != b.kind) return false;
    if (a.kind == VAL_STR)  return strcmp(a.as.s, b.as.s) == 0;
    if (a.kind == VAL_INT)  return a.as.i == b.as.i;
    if (a.kind == VAL_CHAR) return a.as.i == b.as.i;   
    if (a.kind == VAL_BOOL) return a.as.b == b.as.b;
    if (a.kind == VAL_FLOAT) return a.as.f == b.as.f;  
    return false;
}

void map_put(interp_t *I, smap_t *m, value_t k, value_t val)
{
    { size_t i = 0; for (; i < m->cap; i++)
        if (m->entries[i].used && key_eq(m->entries[i].key, k)) { m->entries[i].val = val; return; } }
    if (m->count + 1 > m->cap) {     
        size_t nc = salam_grow_cap(m->cap, m->count + 1, 8);
        smap_entry_t *ne = (smap_entry_t *)arena_alloc(I->a, salam_size_mul(nc, sizeof(smap_entry_t)));
        memset(ne, 0, salam_size_mul(nc, sizeof(smap_entry_t)));
        memcpy(ne, m->entries, salam_size_mul(m->cap, sizeof(smap_entry_t)));
        m->entries = ne; m->cap = nc;
    }
    { size_t i = 0; for (; i < m->cap; i++)
        if (!m->entries[i].used) { m->entries[i].key = k; m->entries[i].val = val; m->entries[i].used = true; m->count++; return; } }
}
smap_entry_t *map_find(smap_t *m, value_t k)
{
    { size_t i = 0; for (; i < m->cap; i++)
        if (m->entries[i].used && key_eq(m->entries[i].key, k)) return &m->entries[i]; }
    return NULL;
}
value_t mk_map(smap_t *m)   { value_t v; v.kind = VAL_MAP;  v.as.map = m; return v; }
value_t mk_struct(interp_t *I, const char *name, ast_node_t *def, size_t nfields)
{
    sstruct_t *s = (sstruct_t *)arena_alloc(I->a, sizeof *s);
    s->type_name = name; s->def = def; s->nfields = nfields;
    s->fields = nfields ? (sfield_t *)arena_alloc(I->a, salam_size_mul(nfields, sizeof(sfield_t))) : NULL;
    value_t v; v.kind = VAL_STRUCT; v.as.st = s; return v;
}
value_t mk_closure(interp_t *I, ast_node_t *fn, env_t *env)
{
    sclosure_t *c = (sclosure_t *)arena_alloc(I->a, sizeof *c);
    c->fn = fn; c->env = env;
    value_t v; v.kind = VAL_FUNC; v.as.fn = c; return v;
}

void base_typename(const char *ts, char *out, size_t cap)
{
    out[0] = 0;
    if (!ts) return;
    size_t i = 0;
    for (; ts[i] && i + 1 < cap; i++) {
        char c = ts[i];
        if (c == '<' || c == '[' || c == '*' || c == ' ') break;
        out[i] = c;
    }
    out[i] = 0;
}

bool is_int_typename(const char *b)
{
    static const char *ints[] = { "i8","i16","i32","i64","u8","u16","u32","u64",
                                  "int","byte","char","isize","usize", NULL };
    { int i = 0; for (; ints[i]; i++) if (strcmp(b, ints[i]) == 0) return true; }
    return false;
}

bool is_float_typename(const char *b)
{ return strcmp(b, "f32") == 0 || strcmp(b, "f64") == 0 || strcmp(b, "float") == 0; }

static value_t zero_for_base(const char *base)
{
    if (is_int_typename(base))   return val_int(0);
    if (is_float_typename(base)) return val_float(0.0);
    if (!strcmp(base, "bool"))   return val_bool(false);
    if (!strcmp(base, "str"))    return val_str("");
    return val_null();
}
value_t default_for_type(interp_t *I, const char *ts)
{
    if (!ts) return val_null();
    const char *lb = strchr(ts, '[');
    if (!lb) return val_null();
    long n = 0;
    { const char *p = lb + 1; for (; *p && *p != ']'; p++)
        if (*p >= '0' && *p <= '9') n = n * 10 + (*p - '0'); }
    char base[96]; base_typename(ts, base, sizeof base);
    value_t zero = zero_for_base(base);
    sarray_t *a = array_new(I, n > 0 ? (size_t)n : 4);
    { long i = 0; for (; i < n; i++) array_push(I, a, zero); }
    return mk_array(I, a);
}

static const char *binop_method(token_kind_t op)
{
    switch (op) {
        case TK_PLUS:    return "operator_add";
        case TK_MINUS:   return "operator_sub";
        case TK_STAR:    return "operator_mul";
        case TK_SLASH:   return "operator_div";
        case TK_PERCENT: return "operator_mod";
        case TK_POWER:   return "operator_pow";
        case TK_EQ:      return "operator_eq";
        case TK_NE:      return "operator_ne";
        case TK_LT:      return "operator_lt";
        case TK_GT:      return "operator_gt";
        case TK_LE:      return "operator_le";
        case TK_GE:      return "operator_ge";
        default:         return NULL;
    }
}
token_kind_t compound_base(token_kind_t op)
{
    switch (op) {
        case TK_PLUS_EQ:    return TK_PLUS;
        case TK_MINUS_EQ:   return TK_MINUS;
        case TK_STAR_EQ:    return TK_STAR;
        case TK_SLASH_EQ:   return TK_SLASH;
        case TK_PERCENT_EQ: return TK_PERCENT;
        default:            return TK_EOF;
    }
}
value_t try_struct_op(interp_t *I, token_kind_t op, value_t a, value_t b,
                      bool has_b, bool *found)
{
    *found = false;
    if (a.kind != VAL_STRUCT) return val_null();
    const char *mname = binop_method(op);
    if (!mname) return val_null();
    ast_node_t *m = struct_method_arity(a.as.st->def, mname, has_b ? 1 : 0);
    
    if (!m && op == TK_NE) {
        ast_node_t *eq = struct_method_arity(a.as.st->def, "operator_eq", 1);
        if (eq) {
            *found = true;
            value_t r = call_func(I, eq, I->globals, &a, &b, 1);
            return val_bool(!to_bool(r));
        }
    }
    if (!m) return val_null();
    *found = true;
    return call_func(I, m, I->globals, &a, has_b ? &b : NULL, has_b ? 1 : 0);
}
value_t arith(interp_t *I, ast_node_t *n, token_kind_t op, value_t a, value_t b)
{
    
    if (op == TK_PLUS && (a.kind == VAL_STR || b.kind == VAL_STR))
        return val_str(afmt(I, "%s%s", to_str(I, a), to_str(I, b)));
    if (op == TK_POWER) {
        double r = pow(to_float(a), to_float(b));
        if (a.kind == VAL_INT && b.kind == VAL_INT && r == floor(r) && fabs(r) < 9.2e18)
            return val_int((int64_t)r);
        return val_float(r);
    }
    bool both_int = is_intlike(a) && is_intlike(b);
    if (both_int) {
        int64_t x = a.as.i, y = b.as.i;
        
        switch (op) {
            case TK_PLUS:    return val_int((int64_t)((uint64_t)x + (uint64_t)y));
            case TK_MINUS:   return val_int((int64_t)((uint64_t)x - (uint64_t)y));
            case TK_STAR:    return val_int((int64_t)((uint64_t)x * (uint64_t)y));
            case TK_SLASH:   if (y == 0) rt_error(I, n, "division by zero");
                             return val_int((x == INT64_MIN && y == -1) ? INT64_MIN : x / y);
            case TK_PERCENT: if (y == 0) rt_error(I, n, "modulo by zero");
                             return val_int((x == INT64_MIN && y == -1) ? 0 : x % y);
            default: break;
        }
    }
    double x = to_float(a), y = to_float(b);
    switch (op) {
        case TK_PLUS:    return val_float(x + y);
        case TK_MINUS:   return val_float(x - y);
        case TK_STAR:    return val_float(x * y);
        case TK_SLASH:   if (y == 0.0) rt_error(I, n, "division by zero");
                         return val_float(x / y);
        case TK_PERCENT: if (y == 0.0) rt_error(I, n, "modulo by zero");
                         return val_float(fmod(x, y));
        default: break;
    }
    rt_error(I, n, "unsupported arithmetic operator");
    return val_null();
}

int value_cmp(interp_t *I, value_t a, value_t b)   
{
    (void)I;
    if (a.kind == VAL_STR && b.kind == VAL_STR) {
        int c = strcmp(a.as.s, b.as.s);
        return c < 0 ? -1 : c > 0 ? 1 : 0;
    }
    double x = to_float(a), y = to_float(b);
    return x < y ? -1 : x > y ? 1 : 0;
}

bool value_eq(interp_t *I, value_t a, value_t b)
{
    (void)I;
    if (a.kind == VAL_STR && b.kind == VAL_STR) return strcmp(a.as.s, b.as.s) == 0;
    if (a.kind == VAL_NULL || b.kind == VAL_NULL) return a.kind == b.kind;
    if (a.kind == VAL_BOOL || b.kind == VAL_BOOL) return to_bool(a) == to_bool(b);
    if (is_number(a) && is_number(b))             return to_float(a) == to_float(b);
    if (a.kind == VAL_STRUCT && b.kind == VAL_STRUCT) return a.as.st == b.as.st;
    return false;
}
