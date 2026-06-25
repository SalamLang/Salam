#ifndef SALAM_LAYOUT_REGISTRY_H
#define SALAM_LAYOUT_REGISTRY_H
#include "core/prelude.h"

typedef enum {
    LE_SINGLE,         /* void element: <img>, <input>            */
    LE_MOTHER,         /* container: <div>, <p>                   */
    LE_SINGLE2MOTHER,  /* single in Salam, container in HTML: input/textarea */
    LE_MOTHER2SINGLE   /* container in Salam, void in HTML        */
} layout_elem_type_t;
typedef struct {
    const char        *name;     /* Salam element name */
    const char        *tag;      /* HTML tag (NULL = CSS-only / synthetic) */
    layout_elem_type_t type;
    const char        *parents;  /* CSV of strict parents, NULL = recursive */
    const char        *required; /* CSV of required attributes */
} layout_elem_def_t;
const layout_elem_def_t *layout_elem_lookup(const char *name);
void layout_registry_set_elements(const layout_elem_def_t *defs, size_t n);
typedef enum {
    LA_CONTENT,   /* inner text / value         */
    LA_CLASS,     /* class attribute            */
    LA_HTML,      /* plain HTML attribute       */
    LA_STYLE,     /* inline style (raw)         */
    LA_DIR,       /* dir, via dir mapper        */
    LA_LANG,      /* lang, via lang mapper      */
    LA_SOURCE,    /* source -> href/src per element */
    LA_CSS,       /* CSS property               */
    LA_META       /* consumed by an element mapper (e.g. heading size), not emitted */
} layout_attr_dest_t;
typedef enum {
    VT_STRING,   /* any string (no restriction)          */
    VT_INT,      /* integer literal                      */
    VT_FLOAT,    /* number (int or decimal)              */
    VT_BOOL,     /* "true" / "false"                     */
    VT_EMAIL,    /* contains a local@domain.tld shape    */
    VT_URL,      /* http(s):// , / , # , or mailto:      */
    VT_COLOR,    /* named CSS color or #hex (mapped)     */
    VT_DIR,      /* text direction ltr/rtl (mapped)      */
    VT_LANG,     /* language code (mapped)               */
    VT_ENUM      /* one of `allowed` (CSV)               */
} layout_value_type_t;
typedef struct {
    const char         *name;    /* Salam attribute name (canonical English) */
    const char         *out;     /* HTML/CSS output name (NULL = derive)      */
    layout_attr_dest_t  dest;    /* where the value goes in the output        */
    layout_value_type_t vtype;   /* value type for validation/mapping         */
    const char         *allowed; /* CSV of allowed values for VT_ENUM, else NULL */
} layout_attr_def_t;
const layout_attr_def_t *layout_attr_lookup(const char *name);
void layout_registry_set_attributes(const layout_attr_def_t *defs, size_t n);
void layout_registry_add_value(const char *group, const char *spelling, const char *canonical);
bool layout_attr_value_ok(const layout_attr_def_t *ad, const char *value);
const char *layout_attr_value_map(const layout_attr_def_t *ad, const char *value);
const char *layout_value_type_name(layout_value_type_t vt);
const char *layout_map_dir(const char *value);    /* -> "ltr" / "rtl" / NULL   */
const char *layout_map_lang(const char *value);   /* -> "en" / "fa" / … / NULL */
const char *layout_map_color(const char *value);  /* -> color keyword/#hex / NULL */
#endif /* SALAM_LAYOUT_REGISTRY_H */
