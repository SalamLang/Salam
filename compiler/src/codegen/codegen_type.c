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
        for (; *p; p++)
            cg_put_ident_byte(&b, *p);
    }
    const char *r = arena_strdup(cg->a, sb_cstr(&b));
    sb_free(&b);
    return r;
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
        {"u64", "uint64_t"},    {"f32", "float"},         {"f64", "double"},
        {"File", "salam_file"},
    };
    {
        size_t i = 0;
        for (; i < sizeof(m) / sizeof(m[0]); i++)
            if (strcmp(base, m[i].s) == 0) return m[i].c;
    }
    return base;
}

void parse_typestr(const char *ts, char *base, size_t cap, bool *ptr, vec_t *dims,
                   arena_t *a)
{
    *ptr = false;
    if (dims) vec_init(dims);
    size_t len = strlen(ts);

    if (len && ts[len - 1] == '*') {
        *ptr = true;
        len--;
    }

    const char *lb = memchr(ts, '[', len);
    size_t blen = lb ? (size_t)(lb - ts) : len;
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

bool cg_is_int_typestr(const char *ts)
{
    static const char *const ints[] = {"i8",  "i16", "i32", "i64", "u8",
                                       "u16", "u32", "u64", NULL};
    {
        int i = 0;
        for (; ints[i]; i++)
            if (!strcmp(ts, ints[i])) return true;
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
    if (!strncmp(ts, "dyn ", 4)) {
        const char *suf = strpbrk(ts + 4, "[*");
        char iface[96];
        size_t il = suf ? (size_t)(suf - (ts + 4)) : strlen(ts + 4);
        if (il >= sizeof iface) il = sizeof iface - 1;
        memcpy(iface, ts + 4, il);
        iface[il] = 0;
        return cg_fmt(cg, "_Salam_dyn_%s%s", cg_cident(cg, iface), suf ? "*" : "");
    }
    if (!strncmp(ts, "func(", 5)) return "void*";
    char base[96];
    bool ptr;
    vec_t dims;
    parse_typestr(ts, base, sizeof(base), &ptr, &dims, cg->a);
    const char *bc = base_ctype(base);

    if (bc == base) bc = arena_strdup(cg->a, cg_cident(cg, base));
    if (dims.len) return cg_fmt(cg, "%s*", bc);
    if (ptr) return cg_fmt(cg, "%s*", bc);
    return bc;
}

const char *cg_decl(cg_t *cg, const char *ts, const char *name)
{
    if (cg_is_slice_ts(ts)) return cg_fmt(cg, "salam_slice %s", cg_cident(cg, name));
    if (!strncmp(ts, "dyn ", 4)) {
        const char *suf = strpbrk(ts + 4, "[*");
        char iface[96];
        size_t il = suf ? (size_t)(suf - (ts + 4)) : strlen(ts + 4);
        if (il >= sizeof iface) il = sizeof iface - 1;
        memcpy(iface, ts + 4, il);
        iface[il] = 0;
        const char *star = (suf && *suf == '*') ? "*" : "";
        const char *dims = (suf && *suf == '[') ? suf : "";
        return cg_fmt(cg, "_Salam_dyn_%s%s %s%s", cg_cident(cg, iface), star,
                      cg_cident(cg, name), dims);
    }
    if (!strncmp(ts, "func(", 5)) return cg_fmt(cg, "void* %s", cg_cident(cg, name));
    char base[96];
    bool ptr;
    vec_t dims;
    parse_typestr(ts, base, sizeof(base), &ptr, &dims, cg->a);
    const char *bc = base_ctype(base);
    if (bc == base) bc = arena_strdup(cg->a, cg_cident(cg, base));
    name = cg_cident(cg, name);
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
    if (ptr) return cg_fmt(cg, "%s* %s", bc, name);
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

const char *cg_mangle_in(cg_t *cg, const char *pkg, const char *struct_name,
                         const char *fn, vec_t *params)
{
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
    const char *sn = sname ? sname : (owner ? owner->name : NULL);
    if (inst) return cg_mangle_in(cg, "g", sn, fn, params);
    return cg_mangle_in(cg, cg->pkg, sn, fn, params);
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
    return t && (t->kind == TY_STRUCT || t->kind == TY_VEC || t->kind == TY_DYN);
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
