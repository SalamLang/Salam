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
#include "core/arena.h"
#include "core/sb.h"
#include "layout/registry.h"
#include "i18n/i18n.h"

static const layout_elem_def_t *g_elems = NULL;
static size_t g_elems_n = 0;
static const layout_attr_def_t *g_attrs = NULL;
static size_t g_attrs_n = 0;
static int g_vals_n = 0;

#define LAYOUT_DYN_VAL_MAX 512

static struct {
    const char *group, *spelling, *canon;
} g_vals[LAYOUT_DYN_VAL_MAX];

void layout_registry_set_elements(const layout_elem_def_t *defs, size_t n)
{
    g_elems = defs;
    g_elems_n = n;
}

const layout_elem_def_t *layout_elem_lookup(const char *name)
{
    {
        size_t i = 0;
        for (; i < g_elems_n; i++)
            if (strcmp(g_elems[i].name, name) == 0) return &g_elems[i];
    }
    return NULL;
}

void layout_registry_set_attributes(const layout_attr_def_t *defs, size_t n)
{
    g_attrs = defs;
    g_attrs_n = n;
}

const layout_attr_def_t *layout_attr_lookup(const char *name)
{
    {
        size_t i = 0;
        for (; i < g_attrs_n; i++)
            if (strcmp(g_attrs[i].name, name) == 0) return &g_attrs[i];
    }
    return NULL;
}

void layout_registry_add_value(const char *group, const char *spelling,
                               const char *canonical)
{
    if (!group || !spelling || !canonical || g_vals_n >= LAYOUT_DYN_VAL_MAX) return;
    g_vals[g_vals_n].group = group;
    g_vals[g_vals_n].spelling = spelling;
    g_vals[g_vals_n].canon = canonical;
    g_vals_n++;
}

static const char *dyn_value(const char *group, const char *v)
{
    {
        int i = 0;
        for (; i < g_vals_n; i++)
            if (strcmp(g_vals[i].group, group) == 0 && strcmp(g_vals[i].spelling, v) == 0)
                return g_vals[i].canon;
    }
    return NULL;
}

const char *layout_map_unit(const char *v)
{
    return v ? dyn_value("unit", v) : NULL;
}

static bool is_ws(char c)
{
    return c == ' ' || c == '\t';
}

static bool scan_number(const char *v, const char **out_end)
{
    const char *p = v;
    if (*p == '+' || *p == '-') p++;
    bool dig = false;
    while (*p >= '0' && *p <= '9') {
        p++;
        dig = true;
    }
    if (*p == '.') {
        const char *q = p + 1;
        while (*q >= '0' && *q <= '9')
            q++;
        if (q > p + 1) p = q;
    }
    if (!dig) return false;
    *out_end = p;
    return true;
}

static size_t normalize_unit_phrase(const char *s, size_t len, char *out, size_t cap)
{
    const char *q = s;
    const char *qend = s + len;
    size_t oi = 0;
    bool prev_space = false;
    while (q < qend && oi < cap - 1) {
        if ((qend - q) >= 3 && (unsigned char)q[0] == 0xE2 &&
            (unsigned char)q[1] == 0x80 && (unsigned char)q[2] == 0x8C) {
            if (!prev_space) {
                out[oi++] = ' ';
                prev_space = true;
            }
            q += 3;
            continue;
        }
        if ((qend - q) >= 2 && (unsigned char)q[0] == 0xD9 &&
            ((unsigned char)q[1] == 0x8A || (unsigned char)q[1] == 0x83)) {
            if (oi + 2 >= cap - 1) break;
            if ((unsigned char)q[1] == 0x8A) {
                out[oi++] = (char)0xDB;
                out[oi++] = (char)0x8C;
            } else {
                out[oi++] = (char)0xDA;
                out[oi++] = (char)0xA9;
            }
            prev_space = false;
            q += 2;
            continue;
        }
        if (is_ws(*q)) {
            if (!prev_space) {
                out[oi++] = ' ';
                prev_space = true;
            }
            q++;
            continue;
        }
        char c = *q;
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        out[oi++] = c;
        prev_space = false;
        q++;
    }
    out[oi] = '\0';
    return oi;
}

typedef struct {
    const char *p;
    size_t len;
} len_tok_t;

#define LAYOUT_LEN_MAX_TOKENS 24

static size_t tokenize_ws(const char *v, len_tok_t *toks, size_t maxtoks)
{
    size_t n = 0;
    const char *p = v;
    while (*p && n < maxtoks) {
        while (is_ws(*p))
            p++;
        if (!*p) break;
        const char *start = p;
        while (*p && !is_ws(*p))
            p++;
        toks[n].p = start;
        toks[n].len = (size_t)(p - start);
        n++;
    }
    while (is_ws(*p))
        p++;
    if (n == maxtoks && *p)
        return 0; /* too many words: bail out, caller passes through */
    return n;
}

static const char *lookup_unit_span(const char *s, size_t len)
{
    char norm[128];
    normalize_unit_phrase(s, len, norm, sizeof norm);
    return layout_map_unit(norm);
}

const char *layout_map_length(arena_t *a, const char *v)
{
    if (!v || !*v) return v;
    len_tok_t toks[LAYOUT_LEN_MAX_TOKENS];
    size_t ntok = tokenize_ws(v, toks, LAYOUT_LEN_MAX_TOKENS);
    if (ntok == 0) return v;

    sb_t out;
    sb_init(&out);
    size_t i = 0;
    while (i < ntok) {
        const char *tp = toks[i].p;
        const char *tend = tp + toks[i].len;
        const char *numend;
        if (out.len) sb_putc(&out, ' ');
        if (!scan_number(tp, &numend) || numend > tend) {
            size_t k = 0;
            for (; k < toks[i].len; k++)
                sb_putc(&out, tp[k]);
            i++;
            continue;
        }
        size_t attached_len = (size_t)(tend - numend);
        const char *canon = NULL;
        size_t consumed = 1;
        if (attached_len > 0) {
            canon = lookup_unit_span(numend, attached_len);
        } else if (i + 2 < ntok) {
            char joined[128];
            size_t l1 = toks[i + 1].len, l2 = toks[i + 2].len;
            if (l1 + 1 + l2 < sizeof joined) {
                memcpy(joined, toks[i + 1].p, l1);
                joined[l1] = ' ';
                memcpy(joined + l1 + 1, toks[i + 2].p, l2);
                joined[l1 + 1 + l2] = '\0';
                canon = lookup_unit_span(joined, l1 + 1 + l2);
                if (canon) consumed = 3;
            }
        }
        if (!canon && attached_len == 0 && i + 1 < ntok) {
            canon = lookup_unit_span(toks[i + 1].p, toks[i + 1].len);
            if (canon) consumed = 2;
        }
        if (!canon && attached_len == 0) {
            canon = "px";
            consumed = 1;
        }
        if (!canon) {
            size_t k = 0;
            for (; k < toks[i].len; k++)
                sb_putc(&out, tp[k]);
            i++;
            continue;
        }
        {
            size_t k = 0;
            for (; k < (size_t)(numend - tp); k++)
                sb_putc(&out, tp[k]);
        }
        sb_puts(&out, canon);
        i += consumed;
    }

    const char *r = arena_strdup(a, sb_cstr(&out));
    sb_free(&out);
    return r;
}

static bool csv_has(const char *csv, const char *v)
{
    if (!csv || !v) return false;
    size_t vl = strlen(v);
    {
        const char *p = csv;
        for (; *p;) {
            const char *c = strchr(p, ',');
            size_t len = c ? (size_t)(c - p) : strlen(p);
            if (len == vl && strncmp(p, v, vl) == 0) return true;
            if (!c) break;
            p = c + 1;
        }
    }
    return false;
}

static bool all_digits(const char *v)
{
    if (!*v) return false;
    for (; *v; v++)
        if (*v < '0' || *v > '9') return false;
    return true;
}

static bool is_int(const char *v)
{
    if (*v == '+' || *v == '-') v++;
    return all_digits(v);
}

static bool is_float(const char *v)
{
    if (*v == '+' || *v == '-') v++;
    bool dig = false, dot = false;
    for (; *v; v++) {
        if (*v >= '0' && *v <= '9')
            dig = true;
        else if (*v == '.' && !dot)
            dot = true;
        else
            return false;
    }
    return dig;
}

static bool is_email(const char *v)
{
    const char *at = strchr(v, '@');
    if (!at || at == v) return false;
    const char *dot = strchr(at + 1, '.');
    return dot && dot[1] != '\0';
}

static bool is_url(const char *v)
{
    return strncmp(v, "http://", 7) == 0 || strncmp(v, "https://", 8) == 0 ||
           strncmp(v, "mailto:", 7) == 0 || v[0] == '/' || v[0] == '#' || v[0] == '.';
}

const char *layout_attr_value_map(arena_t *a, const layout_attr_def_t *ad,
                                  const char *value)
{
    if (!ad || !value) return value;
    const char *v = i18n_layout_value(value);
    switch (ad->vtype) {
    case VT_COLOR: {
        const char *c = layout_map_color(v);
        return c ? c : v;
    }
    case VT_DIR: {
        const char *c = layout_map_dir(v);
        return c ? c : v;
    }
    case VT_LANG: {
        const char *c = layout_map_lang(v);
        return c ? c : v;
    }
    case VT_ENUM: {
        const char *c = dyn_value(ad->allowed, v);
        return c ? c : v;
    }
    case VT_LENGTH:
        return layout_map_length(a, v);
    default:
        return value;
    }
}

const char *layout_value_type_name(layout_value_type_t vt)
{
    switch (vt) {
    case VT_STRING:
        return i18n_tr("a string");
    case VT_INT:
        return i18n_tr("an integer");
    case VT_FLOAT:
        return i18n_tr("a number");
    case VT_BOOL:
        return i18n_tr("'true' or 'false'");
    case VT_EMAIL:
        return i18n_tr("an email address");
    case VT_URL:
        return i18n_tr("a URL");
    case VT_COLOR:
        return i18n_tr("a color");
    case VT_DIR:
        return i18n_tr("a text direction");
    case VT_LANG:
        return i18n_tr("a language");
    case VT_ENUM:
        return i18n_tr("one of the allowed values");
    case VT_LENGTH:
        return i18n_tr("a length");
    }
    return i18n_tr("a value");
}

bool layout_attr_value_ok(const layout_attr_def_t *ad, const char *value)
{
    if (!ad || !value) return true;
    const char *v = i18n_layout_value(value);
    switch (ad->vtype) {
    case VT_STRING:
        return true;
    case VT_INT:
        return is_int(v);
    case VT_FLOAT:
        return is_float(v);
    case VT_BOOL:
        return !strcmp(v, "true") || !strcmp(v, "false");
    case VT_EMAIL:
        return is_email(v);
    case VT_URL:
        return is_url(v);
    case VT_COLOR:
        return layout_map_color(v) != NULL;
    case VT_DIR:
        return layout_map_dir(v) != NULL;
    case VT_LANG:
        return layout_map_lang(v) != NULL;

    case VT_ENUM:
        return csv_has(ad->allowed, v) || dyn_value(ad->allowed, v) != NULL;
    case VT_LENGTH:
        return true;
    }
    return true;
}

const char *layout_map_dir(const char *v)
{
    return v ? dyn_value("dir", v) : NULL;
}

const char *layout_map_lang(const char *v)
{
    return v ? dyn_value("lang", v) : NULL;
}

const char *layout_map_color(const char *v)
{
    if (!v) return NULL;
    if (v[0] == '#') return v;
    return dyn_value("color", v);
}
