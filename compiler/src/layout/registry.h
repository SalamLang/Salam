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

#ifndef SALAM_LAYOUT_REGISTRY_H
#define SALAM_LAYOUT_REGISTRY_H

#include "core/prelude.h"
#include "core/arena.h"

typedef enum {
    LE_SINGLE,
    LE_MOTHER,
    LE_SINGLE2MOTHER,
    LE_MOTHER2SINGLE
} layout_elem_type_t;

typedef struct {
    const char *name;
    const char *tag;
    layout_elem_type_t type;
    const char *parents;
    const char *required;
} layout_elem_def_t;

const layout_elem_def_t *layout_elem_lookup(const char *name);

void layout_registry_set_elements(const layout_elem_def_t *defs, size_t n);

typedef enum {
    LA_CONTENT,
    LA_CLASS,
    LA_HTML,
    LA_STYLE,
    LA_DIR,
    LA_LANG,
    LA_SOURCE,
    LA_CSS,
    LA_META
} layout_attr_dest_t;

typedef enum {
    VT_STRING,
    VT_INT,
    VT_FLOAT,
    VT_BOOL,
    VT_EMAIL,
    VT_URL,
    VT_COLOR,
    VT_DIR,
    VT_LANG,
    VT_ENUM,
    VT_LENGTH
} layout_value_type_t;

typedef struct {
    const char *name;
    const char *out;
    layout_attr_dest_t dest;
    layout_value_type_t vtype;
    const char *allowed;
} layout_attr_def_t;

const layout_attr_def_t *layout_attr_lookup(const char *name);

void layout_registry_set_attributes(const layout_attr_def_t *defs, size_t n);

void layout_registry_add_value(const char *group, const char *spelling,
                               const char *canonical);

bool layout_attr_value_ok(const layout_attr_def_t *ad, const char *value);

const char *layout_attr_value_map(arena_t *a, const layout_attr_def_t *ad,
                                  const char *value);

const char *layout_value_type_name(layout_value_type_t vt);

const char *layout_map_dir(const char *value);

const char *layout_map_lang(const char *value);

const char *layout_map_color(const char *value);

const char *layout_map_unit(const char *value);

const char *layout_map_length(arena_t *a, const char *value);

#endif /* SALAM_LAYOUT_REGISTRY_H */
