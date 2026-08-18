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

#include "codegen/codegen_internal.h"
#include "core/sal_format.h"

void cg_kv(const char *ts, char *kbuf, char *vbuf, size_t cap)
{
    kbuf[0] = vbuf[0] = 0;
    const char *lt = strchr(ts, '<');
    if (!lt) return;
    const char *comma = strchr(lt, ',');
    const char *gt = strrchr(ts, '>');
    if (!comma || !gt) return;
    const char *ks = lt + 1;
    while (*ks == ' ')
        ks++;
    const char *ke = comma;
    while (ke > ks && ke[-1] == ' ')
        ke--;
    const char *vs = comma + 1;
    while (*vs == ' ')
        vs++;
    const char *ve = gt;
    while (ve > vs && ve[-1] == ' ')
        ve--;
    size_t kl = (size_t)(ke - ks);
    if (kl >= cap) kl = cap - 1;
    memcpy(kbuf, ks, kl);
    kbuf[kl] = 0;
    size_t vl = (size_t)(ve - vs);
    if (vl >= cap) vl = cap - 1;
    memcpy(vbuf, vs, vl);
    vbuf[vl] = 0;
}

void cg_put_ident_byte(sb_t *b, unsigned char c)
{
    static const char hex[] = "0123456789abcdef";
    if (isalnum(c))
        sb_putc(b, (char)c);
    else if (c == '_')
        sb_puts(b, "__");
    else {
        char h[4];
        h[0] = '_';
        h[1] = hex[(c >> 4) & 0xf];
        h[2] = hex[c & 0xf];
        h[3] = '\0';
        sb_puts(b, h);
    }
}

static bool cg_is_c_keyword(const char *s)
{
    static const char *const kw[] = {
        "auto",
        "break",
        "case",
        "char",
        "const",
        "continue",
        "default",
        "do",
        "double",
        "else",
        "enum",
        "extern",
        "float",
        "for",
        "goto",
        "if",
        "inline",
        "int",
        "long",
        "register",
        "restrict",
        "return",
        "short",
        "signed",
        "sizeof",
        "static",
        "struct",
        "switch",
        "typedef",
        "union",
        "unsigned",
        "void",
        "volatile",
        "while",
        "asm",
        "bool",
        "true",
        "false",
        "_Bool",
        "_Complex",
        "_Imaginary",
        "_Alignas",
        "_Alignof",
        "_Atomic",
        "_Generic",
        "_Noreturn",
        "_Static_assert",
        "_Thread_local",
    };
    size_t i = 0;
    for (; i < sizeof(kw) / sizeof(kw[0]); i++)
        if (strcmp(s, kw[i]) == 0) return true;
    return false;
}

const char *cg_cident(cg_t *cg, const char *name)
{
    if (!name || !name[0]) return "_";
    bool ok = !isdigit((unsigned char)name[0]);
    {
        const unsigned char *p = (const unsigned char *)name;
        for (; ok && *p; p++)
            if (!(isalnum(*p))) ok = false;
    }
    if (ok) return cg_is_c_keyword(name) ? cg_fmt(cg, "%s_", name) : name;
    sb_t b;
    sb_init(&b);
    if (isdigit((unsigned char)name[0])) sb_putc(&b, '_');
    {
        const unsigned char *p = (const unsigned char *)name;
        /*
         * A LEADING underscore is copied through as one, not doubled. C
         * reserves every identifier starting with "__" for the
         * implementation, and gcc really does use that space: on x86-64 it
         * predefines __k8 as an architecture macro, so Salam's `_k8` (the
         * ordinary "declared but unused" spelling) became C's `__k8` and
         * expanded to `1` - "expected identifier before numeric constant",
         * which took out compiler/tests_port/interp_test.salam entirely.
         * Interior underscores still double, so the mangling stays
         * one-to-one: `_a_b` -> `_a__b`, while `a_b` -> `a__b`.
         */
        if (*p == '_') {
            sb_putc(&b, '_');
            p++;
        }
        for (; *p; p++)
            cg_put_ident_byte(&b, *p);
    }
    const char *r = arena_strdup(cg->a, sb_cstr(&b));
    sb_free(&b);
    /* Copying the leading underscore through as one lets a name land exactly
     * on a C keyword that the doubling used to step over: `_Bool` came out as
     * `__Bool` before, and as plain `_Bool` now - "two or more data types in
     * declaration specifiers". The suffix is the same escape the all-alnum
     * path above already applies to `int`, `while` and friends. */
    return cg_is_c_keyword(r) ? cg_fmt(cg, "%s_", r) : r;
}

static const char *base_ctype(const char *base)
{
    struct {
        const char *s, *c;
    } m[] = {
        {"void", "void"},       {"bool", "bool"},         {"char", "char"},
        {"str", "const char*"}, {"uchar", "const char*"}, {"i8", "int8_t"},
        {"i16", "int16_t"},     {"i32", "int32_t"},       {"i64", "int64_t"},
        {"u8", "uint8_t"},      {"u16", "uint16_t"},      {"u32", "uint32_t"},
        {"u64", "uint64_t"},    {"size", "size_t"},       {"f32", "float"},
        {"f64", "double"},      {"File", "salam_file"},
    };
    {
        size_t i = 0;
        for (; i < sizeof(m) / sizeof(m[0]); i++)
            if (strcmp(base, m[i].s) == 0) return m[i].c;
    }
    return base;
}

/*
 * A lambda, closure or function value. All of them lower to the `void*` that
 * carries the closure environment, which matters beyond the C spelling: a
 * callee takes that pointer as a plain `void*` and may write through it (a
 * counter closure updates the state it captured), so such a binding must not
 * be const-qualified even when the Salam name is immutable.
 */
bool type_is_callable(const char *ts)
{
    return ts && (!strncmp(ts, "func(", 5) || !strncmp(ts, "externfunc(", 11));
}

void parse_typestr_ex(const char *ts, char *base, size_t cap, bool *ptr, vec_t *dims,
                      arena_t *a, int *elem_ptr, int *ptr_depth)
{
    *ptr = false;
    if (elem_ptr) *elem_ptr = 0;
    if (ptr_depth) *ptr_depth = 0;
    if (dims) vec_init(dims);
    size_t len = strlen(ts);

    /* Stars nest: Vector<Edge*> lays its data pointer out as "Edge**", so the
     * whole trailing run is counted rather than just the last one. */
    while (len && ts[len - 1] == '*') {
        *ptr = true;
        if (ptr_depth) (*ptr_depth)++;
        len--;
    }

    const char *lb = memchr(ts, '[', len);
    size_t blen = lb ? (size_t)(lb - ts) : len;
    /*
     * An array OF pointers spells its stars between the element name and
     * the first '[' - "Edge*[6]" is six Edge*, not a pointer to anything.
     * They have to come off `base` here: everything downstream feeds the
     * base through cg_cident, which would hex-escape the '*' into a type
     * name ("Edge_2a") that was never declared. The count travels out via
     * elem_ptr so callers can put the stars back on the element type.
     */
    while (blen && ts[blen - 1] == '*') {
        blen--;
        if (elem_ptr) (*elem_ptr)++;
    }
    if (lb && dims) {
        const char *p = lb;
        while (p < ts + len && *p == '[') {
            const char *rb = strchr(p, ']');
            if (!rb) break;
            size_t n = 0;
            {
                const char *q = p + 1;
                for (; q < rb; q++)
                    if (*q >= '0' && *q <= '9') n = n * 10 + (size_t)(*q - '0');
            }
            size_t *slot = (size_t *)arena_alloc(a, sizeof(size_t));
            *slot = n;
            vec_push(a, dims, slot);
            p = rb + 1;
        }
    }
    if (blen >= cap) blen = cap - 1;
    memcpy(base, ts, blen);
    base[blen] = 0;
}

void parse_typestr(const char *ts, char *base, size_t cap, bool *ptr, vec_t *dims,
                   arena_t *a)
{
    parse_typestr_ex(ts, base, cap, ptr, dims, a, NULL, NULL);
}

bool cg_is_int_typestr(const char *ts)
{
    static const char *const ints[] = {"i8",  "i16", "i32", "i64",  "u8",
                                       "u16", "u32", "u64", "size", NULL};
    {
        int i = 0;
        for (; ints[i]; i++)
            if (!strcmp(ts, ints[i])) return true;
    }
    return false;
}

static int cg_int_ts_bits(const char *ts)
{
    if (!strcmp(ts, "i8") || !strcmp(ts, "u8")) return 8;
    if (!strcmp(ts, "i16") || !strcmp(ts, "u16")) return 16;
    if (!strcmp(ts, "i32") || !strcmp(ts, "u32")) return 32;
    return 64;
}

/*
 * Common type of an integer binary operation (SALAM-TYPES.md 15): same
 * signedness picks the higher rank, mixed signedness has no common type
 * (semantic analysis rejects it as E021).
 */
const char *cg_common_int_typestr(const char *a, const char *b)
{
    if (!a || !b || !cg_is_int_typestr(a) || !cg_is_int_typestr(b)) return NULL;
    if (!strcmp(a, b)) return a;
    if (cg_is_unsigned_typestr(a) != cg_is_unsigned_typestr(b)) return NULL;
    return cg_int_ts_bits(a) >= cg_int_ts_bits(b) ? a : b;
}

/*
 * The unsigned type of the same width, or `ts` itself when there isn't one.
 *
 * Salam defines +, - and * to wrap modulo 2^n in the operand type (SALAM-TYPES
 * 4.1, 15), but signed overflow is undefined in C, not wrapping: at -O2 gcc is
 * entitled to assume it never happens and fold the branch that checks for it.
 * Emitting the arithmetic in the unsigned twin and converting the result back
 * gives exactly the defined wrap Salam promises - unsigned arithmetic is
 * modular by definition, and the narrowing conversion back is two's complement
 * on every target Salam supports (and mandated outright by C23).
 */
const char *cg_unsigned_twin(const char *ts)
{
    if (!ts) return ts;
    if (!strcmp(ts, "i8")) return "u8";
    if (!strcmp(ts, "i16")) return "u16";
    if (!strcmp(ts, "i32")) return "u32";
    if (!strcmp(ts, "i64")) return "u64";
    return ts;
}

bool cg_is_unsigned_typestr(const char *ts)
{
    static const char *const uns[] = {"u8", "u16", "u32", "u64", "size", NULL};
    if (!ts) return false;
    {
        int i = 0;
        for (; uns[i]; i++)
            if (!strcmp(ts, uns[i])) return true;
    }
    return false;
}

void cg_vec_elem(const char *ts, char *ebuf, size_t cap)
{
    ebuf[0] = 0;
    const char *lt = strchr(ts, '<');
    const char *gt = strrchr(ts, '>');
    if (!lt || !gt || gt <= lt) return;
    const char *es = lt + 1;
    while (*es == ' ')
        es++;
    const char *ee = gt;
    while (ee > es && ee[-1] == ' ')
        ee--;
    size_t el = (size_t)(ee - es);
    if (el >= cap) el = cap - 1;
    memcpy(ebuf, es, el);
    ebuf[el] = 0;
}

static void cg_encode_typestr(sb_t *s, const char *str, bool skip_spaces)
{
    const unsigned char *p = (const unsigned char *)str;
    for (; *p; p++) {
        if (*p == '*')
            sb_puts(s, "_ptr");
        else if (*p == '[')
            sb_puts(s, "_arr");
        else if (*p == ']') {
        } else if (*p == ' ' && skip_spaces) {
        } else
            cg_put_ident_byte(s, *p);
    }
}

static const char *cg_vec_code_str(cg_t *cg, const char *elem)
{
    sb_t s;
    sb_init(&s);
    cg_encode_typestr(&s, elem, true);
    const char *r = arena_strdup(cg->a, sb_cstr(&s));
    sb_free(&s);
    return r;
}

const char *cg_vec_cname(cg_t *cg, const char *ts)
{
    char elem[96];
    cg_vec_elem(ts, elem, sizeof(elem));
    return cg_fmt(cg, "Vector_%s", cg_vec_code_str(cg, elem));
}

static void cg_variant_push_trimmed(const char *start, const char *end,
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

size_t cg_variant_members(const char *ts, char members[][160], size_t max_members)
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
            cg_variant_push_trimmed(start, p, members, &n, max_members);
            start = p + 1;
        }
    }
    cg_variant_push_trimmed(start, gt, members, &n, max_members);
    return n;
}

const char *cg_variant_cname(cg_t *cg, const char *ts)
{
    char members[64][160];
    size_t n = cg_variant_members(ts, members, 64);
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "_Salam_variant");
    {
        size_t i = 0;
        for (; i < n; i++) {
            sb_putc(&b, '_');
            sb_puts(&b, cg_vec_code_str(cg, members[i]));
        }
    }
    {
        const char *r = arena_strdup(cg->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

int cg_variant_tag_of(const char *variant_ts, const char *member_ts)
{
    char members[64][160];
    size_t n = cg_variant_members(variant_ts, members, 64);
    size_t i = 0;
    for (; i < n; i++)
        if (!strcmp(members[i], member_ts)) return (int)i;
    return -1;
}

bool cg_is_slice_ts(const char *ts)
{
    return ts && !strncmp(ts, "slice<", 6);
}

void cg_slice_elem(const char *ts, char *ebuf, size_t cap)
{
    if (!ts) {
        if (cap) ebuf[0] = 0;
        return;
    }
    cg_vec_elem(ts, ebuf, cap);
}

const char *cg_ctype(cg_t *cg, const char *ts)
{
    if (!ts) return "void";
    if (cg_is_slice_ts(ts)) return "salam_slice";
    if (!strncmp(ts, "Variant<", 8)) return cg_variant_cname(cg, ts);
    if (!strncmp(ts, "dyn ", 4)) {
        const char *suf = strpbrk(ts + 4, "[*");
        char iface[96];
        size_t il = suf ? (size_t)(suf - (ts + 4)) : strlen(ts + 4);
        if (il >= sizeof iface) il = sizeof iface - 1;
        memcpy(iface, ts + 4, il);
        iface[il] = 0;
        return cg_fmt(cg, "_Salam_dyn_%s%s", cg_cident(cg, iface), suf ? "*" : "");
    }
    if (type_is_callable(ts)) return "void*";
    char base[96];
    bool ptr;
    int eptr, pdepth;
    vec_t dims;
    parse_typestr_ex(ts, base, sizeof(base), &ptr, &dims, cg->a, &eptr, &pdepth);
    const char *bc = base_ctype(base);

    if (bc == base) bc = arena_strdup(cg->a, cg_cident(cg, base));
    for (; eptr > 0; eptr--)
        bc = cg_fmt(cg, "%s*", bc);
    if (dims.len) return cg_fmt(cg, "%s*", bc);
    for (; pdepth > 0; pdepth--)
        bc = cg_fmt(cg, "%s*", bc);
    return bc;
}

/*
 * The C spelling of an array type with no declarator attached - "int32_t[3]"
 * - which is what a compound literal needs in front of its braces. NULL for
 * anything that is not a plain array of a nameable element type, so callers
 * can fall back to the bare brace form.
 */
const char *cg_array_ctype(cg_t *cg, const char *ts)
{
    char base[96];
    bool ptr;
    int eptr, pdepth;
    vec_t dims;
    if (!ts || !*ts || cg_is_slice_ts(ts) || !strncmp(ts, "Variant<", 8) ||
        !strncmp(ts, "dyn ", 4) || !strncmp(ts, "func(", 5))
        return NULL;
    parse_typestr_ex(ts, base, sizeof(base), &ptr, &dims, cg->a, &eptr, &pdepth);
    if (!dims.len || ptr) return NULL;
    {
        const char *bc = base_ctype(base);
        sb_t s;
        size_t i = 0;
        const char *r;
        if (bc == base) bc = arena_strdup(cg->a, cg_cident(cg, base));
        for (; eptr > 0; eptr--)
            bc = cg_fmt(cg, "%s*", bc);
        sb_init(&s);
        sb_puts(&s, bc);
        for (i = 0; i < dims.len; i++) {
            char b[32];
            size_t d = *(size_t *)dims.data[i];
            if (!d) {
                sb_free(&s);
                return NULL;
            }
            sal_snprintf(b, sizeof(b), "[%zu]", d);
            sb_puts(&s, b);
        }
        r = arena_strdup(cg->a, sb_cstr(&s));
        sb_free(&s);
        return r;
    }
}

/*
 * The C name of a module-level global. Functions and types have carried their
 * package since the beginning; globals went out under their bare Salam name,
 * so two modules that each declared `const COLS` produced two `COLS` objects
 * in one flat C namespace and the link failed with "multiple definition" -
 * even when neither was `pub`, and even for std's own `excel.FormatUnknown`
 * against `image.FormatUnknown`, which is why no program could import both.
 *
 * `pkg` is the package the global was declared in, not necessarily the module
 * being emitted: a generic body instantiated in another module still reads
 * its home package's globals, and must name them the same way that package
 * defined them.
 */
const char *cg_global_cname(cg_t *cg, const char *pkg, const char *name)
{
    return cg_fmt(cg, "_Salam_g_%s_%s", cg_cident(cg, (pkg && *pkg) ? pkg : "main"),
                  cg_cident(cg, name));
}

const char *cg_decl(cg_t *cg, const char *ts, const char *name)
{
    return cg_decl_cn(cg, ts, cg_cident(cg, name));
}

/* cg_decl over a name that is already a C identifier. A module-level global
 * is spelled with its package baked in (cg_global_cname), and that spelling
 * must not go through cg_cident a second time - the encoder doubles interior
 * underscores, so the definition and the extern would disagree. */
const char *cg_decl_cn(cg_t *cg, const char *ts, const char *name)
{
    if (cg_is_slice_ts(ts)) return cg_fmt(cg, "salam_slice %s", name);
    if (!strncmp(ts, "Variant<", 8))
        return cg_fmt(cg, "%s %s", cg_variant_cname(cg, ts), name);
    if (!strncmp(ts, "dyn ", 4)) {
        const char *suf = strpbrk(ts + 4, "[*");
        char iface[96];
        size_t il = suf ? (size_t)(suf - (ts + 4)) : strlen(ts + 4);
        if (il >= sizeof iface) il = sizeof iface - 1;
        memcpy(iface, ts + 4, il);
        iface[il] = 0;
        const char *star = (suf && *suf == '*') ? "*" : "";
        const char *dims = (suf && *suf == '[') ? suf : "";
        return cg_fmt(cg, "_Salam_dyn_%s%s %s%s", cg_cident(cg, iface), star, name, dims);
    }
    if (!strncmp(ts, "func(", 5) || !strncmp(ts, "externfunc(", 11))
        return cg_fmt(cg, "void* %s", name);
    char base[96];
    bool ptr;
    int eptr, pdepth;
    vec_t dims;
    parse_typestr_ex(ts, base, sizeof(base), &ptr, &dims, cg->a, &eptr, &pdepth);
    const char *bc = base_ctype(base);
    if (bc == base) bc = arena_strdup(cg->a, cg_cident(cg, base));
    for (; eptr > 0; eptr--)
        bc = cg_fmt(cg, "%s*", bc);
    if (dims.len) {
        sb_t s;
        sb_init(&s);
        sb_puts(&s, bc);
        sb_putc(&s, ' ');
        sb_puts(&s, name);
        {
            size_t i = 0;
            for (; i < dims.len; i++) {
                char b[32];
                sal_snprintf(b, sizeof(b), "[%zu]", *(size_t *)dims.data[i]);
                sb_puts(&s, b);
            }
        }
        const char *r = arena_strdup(cg->a, sb_cstr(&s));
        sb_free(&s);
        return r;
    }
    for (; pdepth > 0; pdepth--)
        bc = cg_fmt(cg, "%s*", bc);
    return cg_fmt(cg, "%s %s", bc, name);
}

static const char *type_code(cg_t *cg, type_t *t)
{
    const char *ts = type_to_string(cg->sem->tc, t);
    sb_t s;
    sb_init(&s);
    cg_encode_typestr(&s, ts, false);
    const char *r = arena_strdup(cg->a, sb_cstr(&s));
    sb_free(&s);
    return r;
}

/*
 * --export=Fn:CName overrides, keyed by the DEFINING file's own package.
 * Keying on the name alone would rename EVERY same-named free function in
 * every module the build transitively pulls in - std/llvm's Emit() and
 * sal_web's Emit() would both answer to one --export=Emit:foo.
 *
 * The strings are borrowed, not copied: the driver passes argv slices and
 * pointers out of the build arena, both of which outlive codegen. The table
 * is file-scope for the same reason dce.c's is - it is set up by the driver
 * before any module is emitted and read from deep inside the mangler.
 */
typedef struct {
    const char *pkg;
    const char *from;
    const char *to;
} cg_export_ovr_t;

/* Matches the CLI's own cap on --export entries, so cli_parse's "too many"
   error is the only place a limit is ever reported to the user. */
#define CG_MAX_EXPORT_OVERRIDES 1024

static cg_export_ovr_t g_cg_exports[CG_MAX_EXPORT_OVERRIDES];
static int g_cg_nexports;

void cg_reset_export_overrides(void)
{
    g_cg_nexports = 0;
}

void cg_add_export_override(const char *pkg, const char *fn, const char *cname)
{
    if (!pkg || !fn || !cname) return;
    if (g_cg_nexports >= CG_MAX_EXPORT_OVERRIDES) return;
    g_cg_exports[g_cg_nexports].pkg = pkg;
    g_cg_exports[g_cg_nexports].from = fn;
    g_cg_exports[g_cg_nexports].to = cname;
    g_cg_nexports++;
}

/* NULL when (pkg, fn) has no registered override. */
static const char *cg_export_override(const char *pkg, const char *fn)
{
    int i = 0;
    for (; i < g_cg_nexports; i++)
        if (!strcmp(g_cg_exports[i].pkg, pkg) && !strcmp(g_cg_exports[i].from, fn))
            return g_cg_exports[i].to;
    return NULL;
}

const char *cg_mangle_in(cg_t *cg, const char *pkg, const char *struct_name,
                         const char *fn, vec_t *params)
{
    /* Free functions only: a method's C name carries its receiver type, and
     * an override that dropped that would collide across structs. */
    if (!struct_name) {
        const char *ovr = cg_export_override(pkg ? pkg : "main", fn);
        if (ovr) return ovr;
    }
    sb_t s;
    sb_init(&s);
    sb_puts(&s, "_Salam_");
    sb_puts(&s, cg_cident(cg, pkg ? pkg : "main"));
    if (struct_name) {
        sb_puts(&s, "_S_");
        sb_puts(&s, cg_cident(cg, struct_name));
    }
    sb_putc(&s, '_');
    sb_puts(&s, cg_cident(cg, fn));
    {
        size_t i = 0;
        for (; i < params->len; i++) {
            sb_putc(&s, '_');
            sb_puts(&s, type_code(cg, (type_t *)params->data[i]));
        }
    }
    const char *r = arena_strdup(cg->a, sb_cstr(&s));
    sb_free(&s);
    return r;
}

const char *cg_mangle(cg_t *cg, const char *struct_name, const char *fn, vec_t *params)
{
    return cg_mangle_in(cg, cg->pkg, struct_name, fn, params);
}

const char *cg_mangle_method(cg_t *cg, const char *sname, symbol_t *owner, const char *fn,
                             vec_t *params)
{
    bool inst = (owner && owner->generic_base) || (owner == NULL && sname);
    if (inst) {
        const char *sn = sname ? sname : (owner ? owner->name : NULL);
        return cg_mangle_in(cg, "g", sn, fn, params);
    }
    const char *sn = owner ? owner->name : sname;
    const char *pkg = (owner && owner->pkgname) ? owner->pkgname : cg->pkg;
    return cg_mangle_in(cg, pkg, sn, fn, params);
}

const char *cg_mangle_ti(cg_t *cg, const char *typestr, const char *fn, vec_t *params)
{
    return cg_mangle_in(cg, "ti", typestr, fn, params);
}

const char *func_ret_of(const char *ts)
{
    if (!ts || strncmp(ts, "func(", 5) != 0) return "void";
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
    return *p ? p : "void";
}

/* 'externfunc(...)' counterparts of func_ret_of/func_cast_params, for raw C
 * function-pointer types (see AST_TYPE.is_extern in sema_type.c). */
const char *raw_ret_of(const char *ts)
{
    if (!ts || strncmp(ts, "externfunc(", 11) != 0) return "void";
    const char *p = ts + 10;
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
    return *p ? p : "void";
}

const char *raw_cast_params(cg_t *cg, const char *ts)
{
    if (!ts || strncmp(ts, "externfunc(", 11) != 0) return "(void)";
    const char *s = ts + 11;
    sb_t out;
    sb_init(&out);
    sb_putc(&out, '(');
    int depth = 0;
    const char *start = s;
    bool any = false;
    {
        const char *p = s;
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
                if (len) {
                    char buf[128];
                    if (len >= sizeof buf) len = sizeof buf - 1;
                    memcpy(buf, start, len);
                    buf[len] = 0;
                    if (any) sb_puts(&out, ", ");
                    sb_puts(&out, cg_ctype(cg, buf));
                    any = true;
                }
                if (*p == ')' || *p == '\0') break;
                start = p + 1;
            }
        }
    }
    if (!any) sb_puts(&out, "void");
    sb_putc(&out, ')');
    const char *r = arena_strdup(cg->a, sb_cstr(&out));
    sb_free(&out);
    return r;
}

static const char *func_cast_params(cg_t *cg, const char *ts)
{
    if (!ts || strncmp(ts, "func(", 5) != 0) return "(void)";
    const char *s = ts + 5;
    sb_t out;
    sb_init(&out);
    sb_putc(&out, '(');
    int depth = 0;
    const char *start = s;
    bool any = false;
    {
        const char *p = s;
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
                if (len) {
                    char buf[128];
                    if (len >= sizeof buf) len = sizeof buf - 1;
                    memcpy(buf, start, len);
                    buf[len] = 0;
                    if (any) sb_puts(&out, ", ");
                    sb_puts(&out, cg_ctype(cg, buf));
                    any = true;
                }
                if (*p == ')' || *p == '\0') break;
                start = p + 1;
            }
        }
    }
    if (!any) sb_puts(&out, "void");
    sb_putc(&out, ')');
    const char *r = arena_strdup(cg->a, sb_cstr(&out));
    sb_free(&out);
    return r;
}

const char *func_cast_params_env(cg_t *cg, const char *ts)
{
    const char *base = func_cast_params(cg, ts);
    if (!strcmp(base, "(void)")) return "(void*)";
    return cg_fmt(cg, "(void*, %s", base + 1);
}

bool type_is_byval_agg(const type_t *t)
{
    return t && (t->kind == TY_STRUCT || t->kind == TY_VEC || t->kind == TY_DYN ||
                 t->kind == TY_SLICE || t->kind == TY_VARIANT);
}

long array_size_of(const char *ts)
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

int print_tag(const char *ts)
{
    if (!ts) return 7;
    if (!strcmp(ts, "str")) return 7;
    if (!strcmp(ts, "uchar")) return 7;
    if (!strcmp(ts, "bool")) return 5;
    if (!strcmp(ts, "char")) return 6;
    if (!strcmp(ts, "f32") || !strcmp(ts, "f64")) return 4;
    if (!strcmp(ts, "i64")) return 1;
    if (!strcmp(ts, "u32")) return 2;
    if (!strcmp(ts, "u64")) return 3;
    if (!strcmp(ts, "size")) return 3;
    return 0;
}

const char *prim_suffix(int tag)
{
    switch (tag) {
    case 1:
        return "i64";
    case 2:
        return "u32";
    case 3:
        return "u64";
    case 4:
        return "f64";
    case 5:
        return "bool";
    case 6:
        return "char";
    default:
        return "i32";
    }
}
