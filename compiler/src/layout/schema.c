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
#include "core/sal_format.h"
#include "layout/schema.h"
#include "layout/registry.h"
#include "core/arena.h"
#include "core/sb.h"
#include "ast/ast.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "langpack/langpack.h"
#include "logger/logger.h"
#include "i18n/i18n.h"
#if defined(_WIN32)
#  include <io.h>
#else
#  include <dirent.h>
#endif

#define SCHEMA_MAX_ELEMS 256

static const char *or_null(const char *s) { return (s && s[0]) ? s : NULL; }

typedef struct { const char *canon, *lang; } word_reg_t;

typedef struct { const char *group, *canon, *lang; } value_reg_t;

static layout_elem_type_t kind_of(const char *k)
{
    if (!k) return LE_MOTHER;
    
    if (!strcmp(k, "Single")        || !strcmp(k, "single"))        return LE_SINGLE;
    if (!strcmp(k, "Single2Mother") || !strcmp(k, "single2mother")) return LE_SINGLE2MOTHER;
    if (!strcmp(k, "Mother2Single") || !strcmp(k, "mother2single")) return LE_MOTHER2SINGLE;
    return LE_MOTHER;
}

static const char *field_atom(const ast_node_t *lit, const char *field)
{
    { size_t i = 0; for (; i < lit->list.len; i++) {
        ast_node_t *fi = (ast_node_t *)lit->list.data[i];   
        if (!fi->name || strcmp(fi->name, field) != 0 || !fi->a) continue;
        if (fi->a->kind == AST_LITERAL && fi->a->value.kind == TV_STRING) return fi->a->value.as.s;
        if (fi->a->kind == AST_MEMBER && fi->a->name) return fi->a->name;   
        return NULL;
    } }
    return NULL;
}

static const char *field_csv(arena_t *a, const ast_node_t *lit, const char *field)
{
    { size_t i = 0; for (; i < lit->list.len; i++) {
        ast_node_t *fi = (ast_node_t *)lit->list.data[i];
        if (!fi->name || strcmp(fi->name, field) != 0 || !fi->a) continue;
        if (fi->a->kind == AST_LITERAL && fi->a->value.kind == TV_STRING) {
            const char *s = fi->a->value.as.s;
            return (s && s[0]) ? s : NULL;
        }
        if (fi->a->kind == AST_ARRAY_LIT) {
            sb_t b; sb_init(&b);
            { size_t j = 0; for (; j < fi->a->list.len; j++) {
                ast_node_t *e = (ast_node_t *)fi->a->list.data[j];
                if (e->kind == AST_LITERAL && e->value.kind == TV_STRING) {
                    if (b.len) sb_putc(&b, ',');
                    sb_puts(&b, e->value.as.s);
                }
            } }
            const char *r = b.len ? arena_strdup(a, sb_cstr(&b)) : NULL;
            sb_free(&b);
            return r;
        }
        return NULL;
    } }
    return NULL;
}

static const char *alias_of(const ast_node_t *def, const char *lang)
{
    { size_t i = 0; for (; i + 1 < def->aliases.len; i += 2)
        if (strcmp((const char *)def->aliases.data[i], lang) == 0)
            return (const char *)def->aliases.data[i + 1]; }
    return NULL;
}

static void alias_foreach(const ast_node_t *def, const char *lang,
                          void (*fn)(const char *, void *), void *ctx)
{
    { size_t i = 0; for (; i + 1 < def->aliases.len; i += 2)
        if (strcmp((const char *)def->aliases.data[i], lang) == 0)
            fn((const char *)def->aliases.data[i + 1], ctx); }
}

static void register_word(const char *spelling, void *ctx)
{
    word_reg_t *r = (word_reg_t *)ctx;
    i18n_register_layout_word(r->lang, spelling, r->canon);
}

static void register_name_spellings(const ast_node_t *d, const char *canon)
{
    word_reg_t en = { canon, "en" }; alias_foreach(d, "en", register_word, &en);
    word_reg_t fa = { canon, "fa" }; alias_foreach(d, "fa", register_word, &fa);
}

static layout_attr_dest_t dest_of(const char *d)
{
    if (!d) return LA_HTML;
    if (!strcmp(d, "content")) return LA_CONTENT;
    if (!strcmp(d, "class"))   return LA_CLASS;
    if (!strcmp(d, "style"))   return LA_STYLE;
    if (!strcmp(d, "dir"))     return LA_DIR;
    if (!strcmp(d, "lang"))    return LA_LANG;
    if (!strcmp(d, "source"))  return LA_SOURCE;
    if (!strcmp(d, "css"))     return LA_CSS;
    if (!strcmp(d, "meta"))    return LA_META;
    return LA_HTML;
}

static layout_value_type_t vtype_of(const char *t)
{
    if (!t) return VT_STRING;
    if (!strcmp(t, "int"))   return VT_INT;
    if (!strcmp(t, "float")) return VT_FLOAT;
    if (!strcmp(t, "bool"))  return VT_BOOL;
    if (!strcmp(t, "email")) return VT_EMAIL;
    if (!strcmp(t, "url"))   return VT_URL;
    if (!strcmp(t, "color")) return VT_COLOR;
    if (!strcmp(t, "dir"))   return VT_DIR;
    if (!strcmp(t, "lang"))  return VT_LANG;
    if (!strcmp(t, "enum"))  return VT_ENUM;
    return VT_STRING;
}

static void load_file(arena_t *a, logger_t *log, langpack_t *pack,
                      const char *path, layout_elem_def_t *defs, size_t *n)
{
    source_file_t *src = source_load(a, path);
    if (!src) return;
    token_stream_t *toks = NULL; lexer_run(a, log, pack, src, &toks);
    ast_node_t *prog = NULL;     parser_run(a, log, toks, &prog);
    if (!prog) return;
    { size_t i = 0; for (; i < prog->list.len && *n < SCHEMA_MAX_ELEMS; i++) {
        ast_node_t *d = (ast_node_t *)prog->list.data[i];
        if (d->kind != AST_CONST_DECL || !d->a || d->a->kind != AST_STRUCT_LIT) continue;
        const ast_node_t *lit = d->a;
        
        const char *name = field_atom(lit, "name");
        if (!name || !name[0]) name = alias_of(d, "en");
        const char *gen  = field_atom(lit, "generated_name");
        if (!gen) gen = field_atom(lit, "tag");
        if (!name || !name[0]) continue;
        layout_elem_def_t *e = &defs[(*n)++];
        e->name     = name;
        e->tag      = or_null(gen);
        e->type     = kind_of(field_atom(lit, "kind"));   
        e->parents  = field_csv(a, lit, "parents");
        if (!e->parents) e->parents = field_csv(a, lit, "parentElements");
        e->required = field_csv(a, lit, "required");
        register_name_spellings(d, name);   
    } }
}

static void load_attr_file(arena_t *a, logger_t *log, langpack_t *pack,
                           const char *path, layout_attr_def_t *defs, size_t *n)
{
    source_file_t *src = source_load(a, path);
    if (!src) return;
    token_stream_t *toks = NULL; lexer_run(a, log, pack, src, &toks);
    ast_node_t *prog = NULL;     parser_run(a, log, toks, &prog);
    if (!prog) return;
    { size_t i = 0; for (; i < prog->list.len && *n < SCHEMA_MAX_ELEMS; i++) {
        ast_node_t *d = (ast_node_t *)prog->list.data[i];
        fprintf(stderr, "DBGL kind=%d aliases=%zu name=%s\n", (int)d->kind, d->aliases.len, d->name?d->name:"(null)");
        if (d->kind != AST_CONST_DECL || !d->a || d->a->kind != AST_STRUCT_LIT) continue;
        const ast_node_t *lit = d->a;
        const char *name = field_atom(lit, "name");
        if (!name || !name[0]) name = alias_of(d, "en");
        fprintf(stderr, "DBGL   -> resolved name=%s dest_field=%s\n", name?name:"(null)", field_atom(lit,"destination")?field_atom(lit,"destination"):"(null)");
        if (!name || !name[0]) continue;
        layout_attr_def_t *e = &defs[(*n)++];
        e->name    = name;
        e->out     = or_null(field_atom(lit, "generated_name"));
        e->dest    = dest_of(field_atom(lit, "destination"));
        e->vtype   = vtype_of(field_atom(lit, "value_type"));
        e->allowed = field_csv(a, lit, "allowed");
        register_name_spellings(d, name);   
    } }
}

static void register_value_spelling(const char *spelling, void *ctx)
{
    value_reg_t *r = (value_reg_t *)ctx;
    layout_registry_add_value(r->group, spelling, r->canon);
    i18n_register_layout_value(r->lang, spelling, r->canon);
}

static void load_value_file(arena_t *a, logger_t *log, langpack_t *pack, const char *path)
{
    source_file_t *src = source_load(a, path);
    if (!src) return;
    token_stream_t *toks = NULL; lexer_run(a, log, pack, src, &toks);
    ast_node_t *prog = NULL;     parser_run(a, log, toks, &prog);
    if (!prog) return;
    { size_t i = 0; for (; i < prog->list.len; i++) {
        ast_node_t *d = (ast_node_t *)prog->list.data[i];
        if (d->kind != AST_CONST_DECL || !d->a || d->a->kind != AST_STRUCT_LIT) continue;
        const ast_node_t *lit = d->a;
        const char *gen   = field_atom(lit, "generated_value");
        const char *group = field_atom(lit, "group");
        if (!gen || !gen[0] || !group || !group[0]) continue;
        layout_registry_add_value(group, gen, gen);   
        value_reg_t en = { group, gen, "*" };  alias_foreach(d, "en", register_value_spelling, &en);
        value_reg_t fa = { group, gen, "fa" }; alias_foreach(d, "fa", register_value_spelling, &fa);
    } }
}

static int list_schema_files(arena_t *a, const char *dir, const char **out, int max)
{
    int n = 0;
#if defined(_WIN32)
    char pat[512]; sal_snprintf(pat, sizeof pat, "%s/*.salam", dir);
    struct _finddata_t fd;
    intptr_t h = _findfirst(pat, &fd);
    if (h == -1) return 0;
    do {
        if (!(fd.attrib & _A_SUBDIR) && n < max) {
            char p[512]; sal_snprintf(p, sizeof p, "%s/%s", dir, fd.name);
            out[n++] = arena_strdup(a, p);
        }
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(dir);
    if (!d) return 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL && n < max) {
        size_t L = strlen(e->d_name);
        if (L > 6 && strcmp(e->d_name + L - 6, ".salam") == 0) {
            char p[512]; sal_snprintf(p, sizeof p, "%s/%s", dir, e->d_name);
            out[n++] = arena_strdup(a, p);
        }
    }
    closedir(d);
#endif
    return n;
}

void layout_schema_init(const char *root)
{
    char base[480];
    if (root && root[0]) sal_snprintf(base, sizeof base, "%s/std/layout", root);
    else                 sal_snprintf(base, sizeof base, "std/layout");
    arena_t  *arena = arena_new(1 << 19);   
    logger_t *log   = logger_new(stderr, LOG_OFF, false);
    langpack_t *pack = langpack_load("en");
    const char *files[SCHEMA_MAX_ELEMS];
    char dir[512];
    
    sal_snprintf(dir, sizeof dir, "%s/elements", base);
    int nf = list_schema_files(arena, dir, files, SCHEMA_MAX_ELEMS);
    if (nf > 0) {
        layout_elem_def_t *defs =
            (layout_elem_def_t *)arena_alloc(arena, sizeof(*defs) * SCHEMA_MAX_ELEMS);
        size_t n = 0;
        { int i = 0; for (; i < nf; i++) load_file(arena, log, pack, files[i], defs, &n); }
        if (n > 0) layout_registry_set_elements(defs, n);
    }
    
    {
        layout_attr_def_t *adefs =
            (layout_attr_def_t *)arena_alloc(arena, sizeof(*adefs) * SCHEMA_MAX_ELEMS);
        size_t an = 0;
        sal_snprintf(dir, sizeof dir, "%s/attributes", base);
        nf = list_schema_files(arena, dir, files, SCHEMA_MAX_ELEMS);
        { int i = 0; for (; i < nf; i++) load_attr_file(arena, log, pack, files[i], adefs, &an); }
        sal_snprintf(dir, sizeof dir, "%s/style", base);
        nf = list_schema_files(arena, dir, files, SCHEMA_MAX_ELEMS);
        { int i = 0; for (; i < nf; i++) load_attr_file(arena, log, pack, files[i], adefs, &an); }
        if (an > 0) layout_registry_set_attributes(adefs, an);
    }
    
    sal_snprintf(dir, sizeof dir, "%s/values", base);
    nf = list_schema_files(arena, dir, files, SCHEMA_MAX_ELEMS);
    { int i = 0; for (; i < nf; i++) load_value_file(arena, log, pack, files[i]); }
    logger_free(log);
    
}
