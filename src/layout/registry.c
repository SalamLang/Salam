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
#include "layout/registry.h"
#include "i18n/i18n.h"

static const layout_elem_def_t *g_elems = NULL;
static size_t g_elems_n = 0;
void layout_registry_set_elements(const layout_elem_def_t *defs, size_t n)
{
    g_elems = defs; g_elems_n = n;
}

const layout_elem_def_t *layout_elem_lookup(const char *name)
{
    { size_t i = 0; for (; i < g_elems_n; i++)
        if (strcmp(g_elems[i].name, name) == 0) return &g_elems[i]; }
    return NULL;
}

static const layout_attr_def_t *g_attrs = NULL;
static size_t g_attrs_n = 0;
void layout_registry_set_attributes(const layout_attr_def_t *defs, size_t n)
{
    g_attrs = defs; g_attrs_n = n;
}

const layout_attr_def_t *layout_attr_lookup(const char *name)
{
    { size_t i = 0; for (; i < g_attrs_n; i++)
        if (strcmp(g_attrs[i].name, name) == 0) return &g_attrs[i]; }
    return NULL;
}
#define LAYOUT_DYN_VAL_MAX 512
static struct { const char *group, *spelling, *canon; } g_vals[LAYOUT_DYN_VAL_MAX];
static int g_vals_n = 0;
void layout_registry_add_value(const char *group, const char *spelling, const char *canonical)
{
    if (!group || !spelling || !canonical || g_vals_n >= LAYOUT_DYN_VAL_MAX) return;
    g_vals[g_vals_n].group = group; g_vals[g_vals_n].spelling = spelling;
    g_vals[g_vals_n].canon = canonical; g_vals_n++;
}

static const char *dyn_value(const char *group, const char *v)
{
    { int i = 0; for (; i < g_vals_n; i++)
        if (strcmp(g_vals[i].group, group) == 0 && strcmp(g_vals[i].spelling, v) == 0)
            return g_vals[i].canon; }
    return NULL;
}

static bool csv_has(const char *csv, const char *v)
{
    if (!csv || !v) return false;
    size_t vl = strlen(v);
    { const char *p = csv; for (; *p; ) {
        const char *c = strchr(p, ',');
        size_t len = c ? (size_t)(c - p) : strlen(p);
        if (len == vl && strncmp(p, v, vl) == 0) return true;
        if (!c) break;
        p = c + 1;
    } }
    return false;
}

static bool all_digits(const char *v) {
    if (!*v) return false;
    for (; *v; v++) if (*v < '0' || *v > '9') return false;
    return true;
}

static bool is_int(const char *v) {
    if (*v == '+' || *v == '-') v++;
    return all_digits(v);
}

static bool is_float(const char *v) {
    if (*v == '+' || *v == '-') v++;
    bool dig = false, dot = false;
    for (; *v; v++) {
        if (*v >= '0' && *v <= '9') dig = true;
        else if (*v == '.' && !dot) dot = true;
        else return false;
    }
    return dig;
}

static bool is_email(const char *v) {
    const char *at = strchr(v, '@');
    if (!at || at == v) return false;
    const char *dot = strchr(at + 1, '.');
    return dot && dot[1] != '\0';
}

static bool is_url(const char *v) {
    return strncmp(v, "http://", 7) == 0 || strncmp(v, "https://", 8) == 0 ||
           strncmp(v, "mailto:", 7) == 0 || v[0] == '/' || v[0] == '#' || v[0] == '.';
}

const char *layout_attr_value_map(const layout_attr_def_t *ad, const char *value)
{
    if (!ad || !value) return value;
    const char *v = i18n_layout_value(value);   
    switch (ad->vtype) {
        case VT_COLOR: { const char *c = layout_map_color(v); return c ? c : v; }
        case VT_DIR:   { const char *c = layout_map_dir(v);   return c ? c : v; }
        case VT_LANG:  { const char *c = layout_map_lang(v);  return c ? c : v; }
        case VT_ENUM:  { const char *c = dyn_value(ad->allowed, v); return c ? c : v; }
        default:       return value;   
    }
}

const char *layout_value_type_name(layout_value_type_t vt)
{
    switch (vt) {
        case VT_STRING: return i18n_tr("a string");
        case VT_INT:    return i18n_tr("an integer");
        case VT_FLOAT:  return i18n_tr("a number");
        case VT_BOOL:   return i18n_tr("'true' or 'false'");
        case VT_EMAIL:  return i18n_tr("an email address");
        case VT_URL:    return i18n_tr("a URL");
        case VT_COLOR:  return i18n_tr("a color");
        case VT_DIR:    return i18n_tr("a text direction");
        case VT_LANG:   return i18n_tr("a language");
        case VT_ENUM:   return i18n_tr("one of the allowed values");
    }
    return i18n_tr("a value");
}

bool layout_attr_value_ok(const layout_attr_def_t *ad, const char *value)
{
    if (!ad || !value) return true;
    const char *v = i18n_layout_value(value);   
    switch (ad->vtype) {
        case VT_STRING: return true;
        case VT_INT:    return is_int(v);
        case VT_FLOAT:  return is_float(v);
        case VT_BOOL:   return !strcmp(v,"true") || !strcmp(v,"false");
        case VT_EMAIL:  return is_email(v);
        case VT_URL:    return is_url(v);
        case VT_COLOR:  return layout_map_color(v) != NULL;
        case VT_DIR:    return layout_map_dir(v)   != NULL;
        case VT_LANG:   return layout_map_lang(v)  != NULL;
        
        case VT_ENUM:   return csv_has(ad->allowed, v) || dyn_value(ad->allowed, v) != NULL;
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
