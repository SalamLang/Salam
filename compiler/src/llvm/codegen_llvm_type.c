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

#include "llvm/codegen_llvm_internal.h"
#include "core/sal_format.h"

int ll_int_bits(const char *ts)
{
    if (!ts) return 32;
    if (!strcmp(ts, "bool")) return 1;
    if (!strcmp(ts, "char") || !strcmp(ts, "i8") || !strcmp(ts, "u8")) return 8;
    if (!strcmp(ts, "i16") || !strcmp(ts, "u16")) return 16;
    if (!strcmp(ts, "i32") || !strcmp(ts, "u32")) return 32;
    if (!strcmp(ts, "i64") || !strcmp(ts, "u64")) return 64;
    return 32;
}

int ll_target_ptr_bits(const char *triple)
{
    if (!triple || !triple[0]) return (int)(sizeof(void *) * 8);
    static const char *w64[] = {"x86_64", "amd64",       "aarch64", "arm64",  "riscv64",
                                "wasm64", "powerpc64",   "ppc64",   "mips64", "sparc64",
                                "s390x",  "loongarch64", NULL};
    static const char *w32[] = {
        "i386",  "i486",   "i586", "i686",    "wasm32",  "armv6", "armv7", "armv5",
        "thumb", "mipsel", "mips", "riscv32", "powerpc", "ppc",   "arm",   NULL};
    {
        int i = 0;
        for (; w64[i]; i++)
            if (strstr(triple, w64[i])) return 64;
    }
    {
        int i = 0;
        for (; w32[i]; i++)
            if (strstr(triple, w32[i])) return 32;
    }
    return (int)(sizeof(void *) * 8);
}

const char *ll_usize_to_i32(ll_t *ll, const char *ref)
{
    if (ll->ptr_bits <= 32) return ref;
    const char *r = ll_new_tmp(ll);
    ll_emit(ll, "%s = trunc %s %s to i32", r, ll->usize, ref);
    return r;
}

bool ll_is_int(const char *ts)
{
    if (!ts) return false;
    static const char *ints[] = {"bool", "char", "i8",  "i16", "i32", "i64",
                                 "u8",   "u16",  "u32", "u64", NULL};
    {
        int i = 0;
        for (; ints[i]; i++)
            if (!strcmp(ts, ints[i])) return true;
    }
    return false;
}

bool ll_is_signed(const char *ts)
{
    if (!ts) return true;
    if (ts[0] == 'u') return false;
    if (!strcmp(ts, "bool")) return false;
    if (!strcmp(ts, "char")) return false;
    return true;
}

long ll_array_dim(const char *ts)
{
    if (!ts) return 0;
    const char *lb = strchr(ts, '[');
    if (!lb) return 0;
    long n = 0;
    {
        const char *p = lb + 1;
        for (; *p && *p != ']'; p++)
            if (*p >= '0' && *p <= '9') n = n * 10 + (*p - '0');
    }
    return n;
}

const char *ll_array_elem(ll_t *ll, const char *ts)
{
    const char *lb = strchr(ts, '[');
    if (!lb) return ts;
    const char *rb = strchr(lb, ']');
    if (!rb) return ts;
    return ll_fmt(ll, "%.*s%s", (int)(lb - ts), ts, rb + 1);
}

bool ll_is_slice_ts(const char *ts)
{
    return ts && !strncmp(ts, "slice<", 6);
}

const char *ll_slice_elem(ll_t *ll, const char *ts)
{
    if (!ts) return "i32";
    const char *lt = strchr(ts, '<');
    const char *gt = strrchr(ts, '>');
    if (!lt || !gt || gt <= lt) return "i32";
    return arena_strndup(ll->a, lt + 1, (size_t)(gt - lt - 1));
}

const char *ll_struct_ltype(ll_t *ll, const char *name)
{
    bool ascii = name && name[0] && !isdigit((unsigned char)name[0]);
    {
        const unsigned char *p = (const unsigned char *)name;
        for (; ascii && *p; p++)
            if (!isalnum(*p)) ascii = false;
    }
    if (ascii) return ll_fmt(ll, "%%struct.%s", name);
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "%struct.");
    if (name && isdigit((unsigned char)name[0])) sb_putc(&b, '_');
    {
        const unsigned char *p = (const unsigned char *)name;
        for (; p && *p; p++) {
            if (isalnum(*p))
                sb_putc(&b, (char)*p);
            else if (*p == '_')
                sb_puts(&b, "__");
            else {
                char h[5];
                sal_snprintf(h, sizeof h, "_%02x", *p);
                sb_puts(&b, h);
            }
        }
    }
    const char *r = arena_strdup(ll->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

static void ll_variant_push_trimmed(const char *start, const char *end,
                                    char members[][160], size_t *n, size_t max_members)
{
    while (start < end && *start == ' ')
        start++;
    while (end > start && end[-1] == ' ')
        end--;
    if (*n >= max_members) return;
    {
        size_t l = (size_t)(end - start);
        if (l >= 160) l = 159;
        memcpy(members[*n], start, l);
        members[*n][l] = 0;
        (*n)++;
    }
}

size_t ll_variant_members(const char *ts, char members[][160], size_t max_members)
{
    size_t n = 0;
    const char *lt = strchr(ts, '<');
    const char *gt = strrchr(ts, '>');
    int depth;
    const char *p, *start;
    if (!lt || !gt || gt <= lt) return 0;
    start = lt + 1;
    depth = 0;
    for (p = start; p < gt; p++) {
        if (*p == '<')
            depth++;
        else if (*p == '>')
            depth--;
        else if (*p == ',' && depth == 0) {
            ll_variant_push_trimmed(start, p, members, &n, max_members);
            start = p + 1;
        }
    }
    ll_variant_push_trimmed(start, gt, members, &n, max_members);
    return n;
}

int ll_variant_tag_of(const char *variant_ts, const char *member_ts)
{
    char members[64][160];
    size_t n = ll_variant_members(variant_ts, members, 64);
    size_t i = 0;
    for (; i < n; i++)
        if (!strcmp(members[i], member_ts)) return (int)i;
    return -1;
}

static void ll_ident_byte(sb_t *b, unsigned char c)
{
    if (isalnum(c))
        sb_putc(b, (char)c);
    else if (c == '_')
        sb_puts(b, "__");
    else {
        char h[5];
        sal_snprintf(h, sizeof h, "_%02x", c);
        sb_puts(b, h);
    }
}

static void ll_encode_ts(sb_t *b, const char *ts)
{
    const unsigned char *p = (const unsigned char *)ts;
    for (; *p; p++) {
        if (*p == '*')
            sb_puts(b, "_ptr");
        else if (*p == '[')
            sb_puts(b, "_arr");
        else if (*p == ']' || *p == ' ') {
        } else
            ll_ident_byte(b, *p);
    }
}

const char *ll_variant_cname(ll_t *ll, const char *ts)
{
    char members[64][160];
    size_t n = ll_variant_members(ts, members, 64);
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "%_Salam_variant");
    {
        size_t i = 0;
        for (; i < n; i++) {
            sb_putc(&b, '_');
            ll_encode_ts(&b, members[i]);
        }
    }
    {
        const char *r = arena_strdup(ll->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

static void ll_struct_layout(ll_t *ll, const char *ts, size_t *out_size,
                             size_t *out_align)
{
    symbol_t *ssym = ll_struct_sym(ll, ts);
    size_t off = 0, maxalign = 1;
    if (!ssym) {
        size_t p = (size_t)(ll->ptr_bits / 8);
        *out_size = p;
        *out_align = p;
        return;
    }
    {
        size_t j = 0;
        for (; j < ssym->members->symbols.len; j++) {
            symbol_t *f = (symbol_t *)ssym->members->symbols.data[j];
            size_t fsz, fal;
            if (f->kind != SYM_FIELD) continue;
            ll_type_layout(ll, type_to_string(ll->sem->tc, f->type), &fsz, &fal);
            if (fal < 1) fal = 1;
            if (fal > maxalign) maxalign = fal;
            off = (off + fal - 1) / fal * fal;
            off += fsz;
        }
    }
    off = (off + maxalign - 1) / maxalign * maxalign;
    if (off == 0) off = 1;
    *out_size = off;
    *out_align = maxalign;
}

static void ll_variant_layout_size(ll_t *ll, const char *ts, size_t *out_size,
                                   size_t *out_align)
{
    char members[64][160];
    size_t n = ll_variant_members(ts, members, 64);
    size_t maxsz = 1, maxal = 1, i = 0;
    for (; i < n; i++) {
        size_t sz, al;
        ll_type_layout(ll, members[i], &sz, &al);
        if (sz > maxsz) maxsz = sz;
        if (al > maxal) maxal = al;
    }
    *out_size = maxsz;
    *out_align = maxal;
}

void ll_type_layout(ll_t *ll, const char *ts, size_t *out_size, size_t *out_align)
{
    size_t p = (size_t)(ll->ptr_bits / 8);
    if (!ts || !strcmp(ts, "void")) {
        *out_size = 0;
        *out_align = 1;
    } else if (!strncmp(ts, "dyn ", 4) && !strchr(ts, '[')) {
        *out_size = 2 * p;
        *out_align = p;
    } else if (!strncmp(ts, "Variant<", 8)) {
        ll_variant_layout_size(ll, ts, out_size, out_align);
    } else if (ll_is_str(ts) || ll_is_ptr_ts(ts)) {
        *out_size = p;
        *out_align = p;
    } else if (ll_is_slice_ts(ts)) {
        *out_size = p + 8;
        *out_align = p > 8 ? p : 8;
    } else if (strchr(ts, '[')) {
        long dim = ll_array_dim(ts);
        size_t esz, eal;
        ll_type_layout(ll, ll_array_elem(ll, ts), &esz, &eal);
        *out_size = esz * (size_t)(dim > 0 ? dim : 0);
        *out_align = eal;
    } else if (!strcmp(ts, "f32")) {
        *out_size = 4;
        *out_align = 4;
    } else if (!strcmp(ts, "f64")) {
        *out_size = 8;
        *out_align = 8;
    } else if (!strcmp(ts, "bool") || !strcmp(ts, "char")) {
        *out_size = 1;
        *out_align = 1;
    } else if (ll_is_int(ts)) {
        size_t bytes = (size_t)((ll_int_bits(ts) + 7) / 8);
        *out_size = bytes;
        *out_align = bytes;
    } else if (ll_struct_sym(ll, ts)) {
        ll_struct_layout(ll, ts, out_size, out_align);
    } else if (ll_enum_sym(ll, ts)) {
        *out_size = 4;
        *out_align = 4;
    } else {
        *out_size = p;
        *out_align = p;
    }
}

void ll_ensure_variant_type(ll_t *ll, const char *ts)
{
    const char *cn = ll_variant_cname(ll, ts);
    size_t sz, al, k;
    {
        size_t i = 0;
        for (; i < ll->emitted.len; i++)
            if (!strcmp(cn, (const char *)ll->emitted.data[i])) return;
    }
    vec_push(ll->a, &ll->emitted, CONST_CAST(cn));
    ll_variant_layout_size(ll, ts, &sz, &al);
    k = (sz + 7) / 8;
    if (k < 1) k = 1;
    sb_puts(ll->g, ll_fmt(ll, "%s = type { i32, [%zu x i64] }\n", cn, k));
}

static symbol_t *ll_sym_plain(ll_t *ll, const char *name)
{
    symbol_t *s = scope_lookup(ll->sem->global, name);
    if (!s && ll->pkg_scope) s = scope_lookup_local(ll->pkg_scope, name);

    if (!s) {
        size_t i = 0;
        for (; i < ll->sem->packages.len && !s; i++) {
            symbol_t *pk = (symbol_t *)ll->sem->packages.data[i];
            if (pk && pk->members) s = scope_lookup_local(pk->members, name);
            if (s) ll_touch_pkg(ll, pk);
        }
    }
    return s;
}

static symbol_t *ll_sym_qualified(ll_t *ll, const char *name)
{
    const char *us = strchr(name, '_');
    for (; us; us = strchr(us + 1, '_')) {
        symbol_t *s = ll_sym_plain(ll, us + 1);
        if (s && s->type) {
            const char *ts = type_to_string(ll->sem->tc, s->type);
            if (ts && !strcmp(ts, name)) return s;
        }
    }
    return NULL;
}

symbol_t *ll_sym(ll_t *ll, const char *name)
{
    if (!name) return NULL;
    symbol_t *s = ll_sym_plain(ll, name);
    if (!s && strchr(name, '_')) s = ll_sym_qualified(ll, name);
    return s;
}

symbol_t *ll_struct_sym(ll_t *ll, const char *name)
{
    symbol_t *s = ll_sym(ll, name);
    return (s && s->kind == SYM_STRUCT) ? s : NULL;
}

symbol_t *ll_enum_sym(ll_t *ll, const char *name)
{
    symbol_t *s = ll_sym(ll, name);
    return (s && s->kind == SYM_ENUM) ? s : NULL;
}

int ll_field_index(symbol_t *ssym, const char *field, symbol_t **out_field)
{
    int idx = 0;
    {
        size_t i = 0;
        for (; i < ssym->members->symbols.len; i++) {
            symbol_t *f = (symbol_t *)ssym->members->symbols.data[i];
            if (f->kind != SYM_FIELD) continue;
            if (!strcmp(f->name, field)) {
                if (out_field) *out_field = f;
                return idx;
            }
            idx++;
        }
    }
    return -1;
}

const char *ll_func_ret(ll_t *ll, const char *ts)
{
    if (!ts || strncmp(ts, "func(", 5)) return "void";
    const char *p = ts + 4;
    int d = 0;
    for (; *p; p++) {
        if (*p == '(')
            d++;
        else if (*p == ')') {
            if (--d == 0) {
                p++;
                break;
            }
        }
    }
    while (*p == ' ')
        p++;
    return *p ? arena_strdup(ll->a, p) : "void";
}

void ll_func_params(ll_t *ll, const char *ts, vec_t *out)
{
    vec_init(out);
    if (!ts || strncmp(ts, "func(", 5)) return;
    const char *start = ts + 5;
    int depth = 0;
    {
        const char *p = start;
        for (;; p++) {
            if (*p == '<' || *p == '(')
                depth++;
            else if (*p == '>')
                depth--;
            else if (*p == ')' && depth > 0)
                depth--;
            if ((*p == ',' && depth == 0) || (*p == ')' && depth == 0) || *p == '\0') {
                size_t len = (size_t)(p - start);
                while (len && start[0] == ' ') {
                    start++;
                    len--;
                }
                while (len && start[len - 1] == ' ')
                    len--;
                if (len) vec_push(ll->a, out, (void *)arena_strndup(ll->a, start, len));
                if (*p == ')' || *p == '\0') break;
                start = p + 1;
            }
        }
    }
}

const char *ll_zero(const char *ts)
{
    if (ll_is_str(ts) || ll_is_ptr_ts(ts)) return "null";
    if (ll_is_bool(ts)) return "false";
    if (ll_is_float(ts)) return "0.0";
    if (ll_is_int(ts)) return "0";
    return "zeroinitializer";
}

const char *ll_ty(ll_t *ll, const char *ts)
{
    if (!ts || !strcmp(ts, "void")) return "void";
    if (!strncmp(ts, "dyn ", 4) && !strchr(ts, '[')) return "%dyn";
    if (!strncmp(ts, "Variant<", 8)) {
        ll_ensure_variant_type(ll, ts);
        return ll_variant_cname(ll, ts);
    }
    if (ll_is_str(ts)) return "ptr";
    if (ll_is_ptr_ts(ts)) return "ptr";
    if (ll_is_slice_ts(ts)) return "{ ptr, i64 }";
    if (strchr(ts, '['))
        return ll_fmt(ll, "[%ld x %s]", ll_array_dim(ts),
                      ll_ty(ll, ll_array_elem(ll, ts)));
    if (!strcmp(ts, "f32")) return "float";
    if (!strcmp(ts, "f64")) return "double";
    if (ll_is_int(ts)) return ll_fmt(ll, "i%d", ll_int_bits(ts));
    if (ll_struct_sym(ll, ts)) return ll_struct_ltype(ll, ts);
    if (ll_enum_sym(ll, ts)) return "i32";

    return "ptr";
}

const char *ll_conv(ll_t *ll, llv_t v, const char *to_ts)
{
    const char *from = v.ts ? v.ts : "i32";
    if (!strcmp(from, to_ts)) return v.ref;

    if (to_ts && !strncmp(to_ts, "dyn ", 4) && ll_struct_sym(ll, from))
        return ll_box_dyn(ll, v, to_ts + 4);
    bool fi = ll_is_int(from), ff = ll_is_float(from),
         fp = ll_is_str(from) || ll_is_ptr_ts(from);
    bool ti = ll_is_int(to_ts), tf = ll_is_float(to_ts),
         tp = ll_is_str(to_ts) || ll_is_ptr_ts(to_ts);
    const char *lf = ll_ty(ll, from), *lt = ll_ty(ll, to_ts);
    const char *r = ll_new_tmp(ll);
    if (fi && ti) {
        int bf = ll_int_bits(from), bt = ll_int_bits(to_ts);
        if (bt == bf) return v.ref;
        if (bt < bf)
            ll_emit(ll, "%s = trunc %s %s to %s", r, lf, v.ref, lt);
        else if (ll_is_signed(from))
            ll_emit(ll, "%s = sext %s %s to %s", r, lf, v.ref, lt);
        else
            ll_emit(ll, "%s = zext %s %s to %s", r, lf, v.ref, lt);
        return r;
    }
    if (ff && tf) {
        if (!strcmp(from, "f32") && !strcmp(to_ts, "f64"))
            ll_emit(ll, "%s = fpext float %s to double", r, v.ref);
        else
            ll_emit(ll, "%s = fptrunc double %s to float", r, v.ref);
        return r;
    }
    if (fi && tf) {
        ll_emit(ll, "%s = %s %s %s to %s", r, ll_is_signed(from) ? "sitofp" : "uitofp",
                lf, v.ref, lt);
        return r;
    }
    if (ff && ti) {
        ll_emit(ll, "%s = %s %s %s to %s", r, ll_is_signed(to_ts) ? "fptosi" : "fptoui",
                lf, v.ref, lt);
        return r;
    }
    if (fi && tp) {
        ll_emit(ll, "%s = inttoptr %s %s to ptr", r, lf, v.ref);
        return r;
    }
    if (fp && ti) {
        ll_emit(ll, "%s = ptrtoint ptr %s to %s", r, v.ref, lt);
        return r;
    }
    if (fp && tp) return v.ref;
    return v.ref;
}

const char *ll_common(const char *a, const char *b)
{
    if (ll_is_float(a) || ll_is_float(b)) {
        if (!strcmp(a, "f64") || !strcmp(b, "f64")) return "f64";
        return "f32";
    }
    int ba = ll_int_bits(a), bb = ll_int_bits(b);
    int bits = ba > bb ? ba : bb;
    bool sgn = ll_is_signed(a) && ll_is_signed(b);
    if (bits <= 8) return sgn ? "i8" : "u8";
    if (bits <= 16) return sgn ? "i16" : "u16";
    if (bits <= 32) return sgn ? "i32" : "u32";
    return sgn ? "i64" : "u64";
}

const char *ll_as_i1(ll_t *ll, llv_t v)
{
    if (ll_is_bool(v.ts)) return v.ref;
    const char *r = ll_new_tmp(ll);
    if (ll_is_float(v.ts))
        ll_emit(ll, "%s = fcmp one %s %s, 0.0", r, ll_ty(ll, v.ts), v.ref);
    else
        ll_emit(ll, "%s = icmp ne %s %s, 0", r, ll_ty(ll, v.ts), v.ref);
    return r;
}
