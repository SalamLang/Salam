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

#include "llvm/codegen_llvm_internal.h"
#include "core/sal_format.h"

/*
 * A decimal floating constant LLVM's .ll parser will accept.
 *
 * It requires a '.' in every decimal FP literal, and printf's %g drops the
 * point whenever the value is exactly integral - including in exponent form
 * for large magnitudes, where "1e+308" then lexes as an integer with a stray
 * exponent and the module fails to parse ("integer constant must have
 * integer type"). Put the point back, before the exponent when there is one.
 *
 * inf/nan text is left alone: those are not decimal literals and splicing a
 * point into them would only make things worse.
 *
 * Every path returns storage that outlives the call - arena text, or a string
 * literal. Handing `v` straight back would not: ll_literal() formats into a
 * stack buffer and the text it gets is spliced into an instruction emitted
 * further down, by which point that frame is gone. A literal already carrying
 * a '.' took exactly that path, so "0.5" reached the module as whatever had
 * since been written over those bytes ("expected value token" out of the .ll
 * parser, on a line that reads correctly in any build where the frame happened
 * to survive).
 */
const char *ll_fp_text(ll_t *ll, const char *v)
{
    const char *ep;
    char mant[64], expo[64];
    size_t mlen;
    if (!v || !*v) return "0.0";
    if (strchr(v, '.') || strpbrk(v, "nNiI")) return ll_fmt(ll, "%s", v);
    ep = strpbrk(v, "eE");
    if (!ep) return ll_fmt(ll, "%s.0", v);
    mlen = (size_t)(ep - v);
    if (mlen >= sizeof mant || strlen(ep) >= sizeof expo) return ll_fmt(ll, "%s", v);
    memcpy(mant, v, mlen);
    mant[mlen] = '\0';
    sal_snprintf(expo, sizeof expo, "%s", ep);
    return ll_fmt(ll, "%s.0%s", mant, expo);
}

int ll_int_bits(ll_t *ll, const char *ts)
{
    if (!ts) return 32;
    if (!strcmp(ts, "bool")) return 1;
    if (!strcmp(ts, "char") || !strcmp(ts, "i8") || !strcmp(ts, "u8")) return 8;
    if (!strcmp(ts, "i16") || !strcmp(ts, "u16")) return 16;
    if (!strcmp(ts, "i32") || !strcmp(ts, "u32")) return 32;
    if (!strcmp(ts, "i64") || !strcmp(ts, "u64")) return 64;
    if (!strcmp(ts, "size")) return ll->ptr_bits;
    return 32;
}

int ll_target_ptr_bits(const char *triple)
{
    if (!triple || !triple[0]) return (int)(sizeof(void *) * 8);
    static const char *w64[] = {"x86_64", "amd64",       "aarch64", "arm64",  "riscv64",
                                "wasm64", "powerpc64",   "ppc64",   "mips64", "sparc64",
                                "s390x",  "loongarch64", NULL};
    /* Real LLVM backends with a 16-bit pointer datalayout (p:16). Checked
     * before w32 so "avr"/"msp430" triples don't fall into a generic bucket. */
    static const char *w16[] = {"msp430", "avr", NULL};
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
        for (; w16[i]; i++)
            if (strstr(triple, w16[i])) return 16;
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
    static const char *ints[] = {"bool", "char", "i8",  "i16", "i32",  "i64",
                                 "u8",   "u16",  "u32", "u64", "size", NULL};
    {
        int i = 0;
        for (; ints[i]; i++)
            if (!strcmp(ts, ints[i])) return true;
    }
    return false;
}

bool ll_is_signed(const char *ts)
{
    static const char *const uns[] = {"u8", "u16", "u32", "u64", "size", NULL};
    if (!ts) return true;
    {
        int i = 0;
        for (; uns[i]; i++)
            if (!strcmp(ts, uns[i])) return false;
    }
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

bool ll_is_vec_ts(const char *ts)
{
    return ts && !strncmp(ts, "Vector<", 7);
}

/* Element type of a Vector<...> typestring, matching cg_vec_elem: the text
   between the outermost angle brackets, so nested generics survive. */
const char *ll_vec_elem_ts(ll_t *ll, const char *ts)
{
    if (!ts) return "i32";
    const char *lt = strchr(ts, '<');
    const char *gt = strrchr(ts, '>');
    if (!lt || !gt || gt <= lt) return "i32";
    return arena_strndup(ll->a, lt + 1, (size_t)(gt - lt - 1));
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
    } else if (!strncmp(ts, "dyn ", 4) && !strchr(ts, '[') && !ll_is_ptr_ts(ts)) {
        *out_size = 2 * p;
        *out_align = p;
    } else if (!strncmp(ts, "Variant<", 8)) {
        ll_variant_layout_size(ll, ts, out_size, out_align);
    } else if (ll_is_str(ll, ts) || ll_is_ptr_ts(ts)) {
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
        size_t bytes = (size_t)((ll_int_bits(ll, ts) + 7) / 8);
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

/*
 * Find a struct/enum in `sc` by its *mangled* type name ("rawsock_Socket")
 * rather than by its declared name ("Socket").
 */
static symbol_t *ll_scan_mangled(ll_t *ll, scope_t *sc, const char *name)
{
    size_t i = 0;
    if (!sc) return NULL;
    for (; i < sc->symbols.len; i++) {
        symbol_t *s = (symbol_t *)sc->symbols.data[i];
        const char *ts;
        if (!s || !s->type) continue;
        if (s->kind != SYM_STRUCT && s->kind != SYM_ENUM) continue;
        ts = type_to_string(ll->sem->tc, s->type);
        if (ts && !strcmp(ts, name)) return s;
    }
    return NULL;
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
    /*
     * The split-on-'_' walk above resolves the tail by *declared* name, so
     * it silently picks the wrong type whenever two packages declare the
     * same one - std/net has both a udp.Socket and a rawsock.Socket, and
     * only one of them wins the bare "Socket" in the global scope, leaving
     * the other unreachable ("member 'fd' of non-struct/unknown type
     * 'rawsock_Socket'").
     *
     * Mangled names are unique by construction, so scan for that instead.
     * Package hits are touched, which is also what makes a program that
     * uses only a package's *types* - never its functions, so nothing else
     * would ever trigger ll_touch_pkg - get that package's struct layouts
     * emitted.
     */
    {
        symbol_t *s = ll_scan_mangled(ll, ll->sem->global, name);
        size_t p = 0;
        if (s) return s;
        for (; p < ll->sem->packages.len; p++) {
            symbol_t *pk = (symbol_t *)ll->sem->packages.data[p];
            if (!pk || pk->kind != SYM_PACKAGE) continue;
            s = ll_scan_mangled(ll, pk->members, name);
            if (s) {
                ll_touch_pkg(ll, pk);
                return s;
            }
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

static bool ll_scope_holds(scope_t *sc, symbol_t *want)
{
    size_t i = 0;
    if (!sc) return false;
    for (; i < sc->symbols.len; i++)
        if ((symbol_t *)sc->symbols.data[i] == want) return true;
    return false;
}

/*
 * Which package scope a symbol was declared in. Emitting a callee's body
 * has to happen under *its* package scope, not the caller's: std/net/http
 * has a `send(method, url, headers, body): Response` of its own, and while
 * its Client methods were emitted under the calling module's scope that
 * name resolved to the libc `send()` extern std/net/internal/rawsock
 * declares - four strings went through ptrtoint into a socket call.
 */
scope_t *ll_scope_of(ll_t *ll, symbol_t *want)
{
    size_t p = 0;
    if (!want) return NULL;
    if (ll_scope_holds(ll->sem->global, want)) return ll->sem->global;
    for (; p < ll->sem->packages.len; p++) {
        symbol_t *pk = (symbol_t *)ll->sem->packages.data[p];
        if (!pk || pk->kind != SYM_PACKAGE || !pk->members) continue;
        if (ll_scope_holds(pk->members, want)) return pk->members;
    }
    return NULL;
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

/*
 * Length of the "func(" / "externfunc(" prefix, or 0 when `ts` is neither.
 * Both spellings come out of type_to_string (types.c picks by t->length),
 * and every structural helper below parses them identically - only the
 * *call* lowering differs, since an externfunc is a bare C function
 * pointer with no closure environment.
 */
static size_t ll_fnprefix(const char *ts)
{
    if (!ts) return 0;
    if (!strncmp(ts, "func(", 5)) return 4;
    if (!strncmp(ts, "externfunc(", 11)) return 10;
    return 0;
}

bool ll_is_extern_fn_ts(const char *ts)
{
    return ts && !strncmp(ts, "externfunc(", 11);
}

bool ll_is_any_fn_ts(const char *ts)
{
    return ll_fnprefix(ts) != 0;
}

const char *ll_func_ret(ll_t *ll, const char *ts)
{
    size_t pre = ll_fnprefix(ts);
    if (!pre) return "void";
    const char *p = ts + pre;
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
    size_t pre = ll_fnprefix(ts);
    vec_init(out);
    if (!pre) return;
    const char *start = ts + pre + 1;
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

const char *ll_zero(ll_t *ll, const char *ts)
{
    if (ll_is_str(ll, ts) || ll_is_ptr_ts(ts)) return "null";
    if (ll_is_bool(ts)) return "false";
    if (ll_is_float(ts)) return "0.0";
    if (ll_is_int(ts)) return "0";
    return "zeroinitializer";
}

const char *ll_ty(ll_t *ll, const char *ts)
{
    if (!ts || !strcmp(ts, "void")) return "void";
    /*
     * A `dyn X` interface value is the two-word %dyn fat pointer, but `dyn
     * X*` and `dyn X[N]` are an ordinary pointer and an array OF those - the
     * array case was already excluded, the pointer case was not.
     * Vector<dyn Shape>'s `data: T*` field therefore got LLVM type %dyn, so
     * the buffer was loaded as a struct and handed to Reallocate where a ptr
     * was expected, and no program holding interfaces in a Vector compiled.
     */
    if (!strncmp(ts, "dyn ", 4) && !strchr(ts, '[') && !ll_is_ptr_ts(ts)) return "%dyn";
    if (!strncmp(ts, "Variant<", 8)) {
        ll_ensure_variant_type(ll, ts);
        return ll_variant_cname(ll, ts);
    }
    if (ll_is_str(ll, ts)) return "ptr";
    if (ll_is_ptr_ts(ts)) return "ptr";
    if (ll_is_slice_ts(ts)) return "{ ptr, i64 }";
    if (strchr(ts, '['))
        return ll_fmt(ll, "[%ld x %s]", ll_array_dim(ts),
                      ll_ty(ll, ll_array_elem(ll, ts)));
    if (!strcmp(ts, "f32")) return "float";
    if (!strcmp(ts, "f64")) return "double";
    if (ll_is_int(ts)) return ll_fmt(ll, "i%d", ll_int_bits(ll, ts));
    if (ll_struct_sym(ll, ts)) return ll_struct_ltype(ll, ts);
    {
        symbol_t *esym = ll_enum_sym(ll, ts);
        if (esym) {
            if (esym->enum_val_kind == TV_FLOAT) return "double";
            /* TV_STRING already returned "ptr" above via ll_is_str. */
            return "i32";
        }
    }

    return "ptr";
}

/* An enum with no explicit backing (or an int one) still behaves as i32
 * everywhere in this backend; a float/string-backed one instead behaves as
 * its backing type, per ll_ty/ll_enum_member_value above. */
static bool ll_enum_backed_int(ll_t *ll, const char *ts)
{
    symbol_t *e = ll_enum_sym(ll, ts);
    return e && e->enum_val_kind != TV_STRING && e->enum_val_kind != TV_FLOAT;
}

static bool ll_enum_backed_float(ll_t *ll, const char *ts)
{
    symbol_t *e = ll_enum_sym(ll, ts);
    return e && e->enum_val_kind == TV_FLOAT;
}

const char *ll_conv(ll_t *ll, llv_t v, const char *to_ts)
{
    const char *from = v.ts ? v.ts : "i32";
    if (!strcmp(from, to_ts)) return v.ref;

    if (to_ts && !strncmp(to_ts, "dyn ", 4) && ll_struct_sym(ll, from))
        return ll_box_dyn(ll, v, to_ts + 4);
    /*
     * An int/no-value enum is an i32 everywhere else in this backend (ll_ty
     * returns "i32" for one, and ll_int_bits falls through to 32), but
     * ll_is_int only knows the builtin spellings - so an enum-typed operand
     * matched none of the branches below and fell out of the bottom
     * UNCONVERTED. `println` of a struct with an enum field then emitted
     *   %t5 = load i32, ptr %t4
     *   %t7 = call ptr @salam_ll_i64str(i64 %t5)
     * which is not valid IR at all. The C compiler hid it by falling back to
     * the C backend for the whole file; a self-hosted compiler with
     * in-process LLVM has no such fallback and simply failed the build.
     * A float/string-backed enum instead joins the float/string classes, so
     * it converts (and gets compared) exactly like its backing type.
     */
    bool fi = ll_is_int(from) || ll_enum_backed_int(ll, from),
         ff = ll_is_float(from) || ll_enum_backed_float(ll, from),
         fp = ll_is_str(ll, from) || ll_is_ptr_ts(from);
    bool ti = ll_is_int(to_ts) || ll_enum_backed_int(ll, to_ts),
         tf = ll_is_float(to_ts) || ll_enum_backed_float(ll, to_ts),
         tp = ll_is_str(ll, to_ts) || ll_is_ptr_ts(to_ts);
    const char *lf = ll_ty(ll, from), *lt = ll_ty(ll, to_ts);
    const char *r = ll_new_tmp(ll);
    if (fi && ti) {
        int bf = ll_int_bits(ll, from), bt = ll_int_bits(ll, to_ts);
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

const char *ll_common(ll_t *ll, const char *a, const char *b)
{
    if (ll_is_float(a) || ll_is_float(b)) {
        if (!strcmp(a, "f64") || !strcmp(b, "f64")) return "f64";
        return "f32";
    }
    int ba = ll_int_bits(ll, a), bb = ll_int_bits(ll, b);
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
        /* Truthiness is `x != 0.0`, and NaN != 0.0 is true - the unordered
         * predicate is the one that says so (see ll_cmp_pred). */
        ll_emit(ll, "%s = fcmp une %s %s, 0.0", r, ll_ty(ll, v.ts), v.ref);
    else
        ll_emit(ll, "%s = icmp ne %s %s, 0", r, ll_ty(ll, v.ts), v.ref);
    return r;
}
