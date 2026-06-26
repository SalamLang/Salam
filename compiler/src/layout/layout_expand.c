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
#include "core/numstr.h"
#include "layout/layout_expand.h"
#include "core/vec.h"
#include "core/sb.h"
#include "source/source.h"
#include "preproc/preproc.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"   

#define LX_EXPAND_MAX_DEPTH 64
typedef struct {
    const char *name;
    ast_node_t *def;
    const char *dir;
    bool        mother;   
} comp_t;
typedef struct { const char *path; ast_node_t *prog; const char *dir; } loaded_t;
typedef struct {
    arena_t          *a;
    logger_t         *log;
    const langpack_t *pack;
    vec_t             comps;    
    vec_t             loaded;   
    size_t            errors;
} expand_t;
typedef struct {
    vec_t names;   
    vec_t vals;    
    vec_t content; 
} frame_t;

#define EXERR(ex, ...) do { LOG_E((ex)->log, PH_DRIVER, __VA_ARGS__); (ex)->errors++; } while (0)
static bool name_eq(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static bool is_include_attr(const char *n) { return name_eq(n, "include") || name_eq(n, "بگنجان"); }

static bool is_content_word(const char *n) { return name_eq(n, "content") || name_eq(n, "محتوا"); }

static const char *dir_of(arena_t *a, const char *path)
{
    const char *slash = NULL;
    { const char *p = path; for (; *p; p++)
        if (*p == '/' || *p == '\\') slash = p; }
    if (!slash) return "";
    return arena_strndup(a, path, (size_t)(slash - path));
}

static ast_node_t *mk_str(expand_t *ex, const char *s, const src_span_t *span)
{
    ast_node_t *n = ast_new(ex->a, AST_LITERAL, span);
    n->op = TK_STRING;
    n->name = s;
    n->value.kind = TV_STRING;
    n->value.as.s = s;
    return n;
}

static const char *node_text(expand_t *ex, ast_node_t *v)
{
    if (!v) return "";
    if (v->kind == AST_IDENTIFIER) return v->name ? v->name : "";
    if (v->kind == AST_LITERAL) {
        char buf[32];
        switch (v->op) {
            case TK_STRING: case TK_TRIPLE_STRING: return v->value.as.s ? v->value.as.s : "";
            case TK_INT:    sal_u64toa((uint64_t)v->value.as.i, buf);
                            return arena_strdup(ex->a, buf);
            case TK_FLOAT:  snprintf(buf, sizeof(buf), "%g", v->value.as.f);
                            return arena_strdup(ex->a, buf);
            case TK_KW_TRUE:  return "true";
            case TK_KW_FALSE: return "false";
            default: return v->name ? v->name : "";
        }
    }
    return "";
}

static ast_node_t *frame_lookup(frame_t *f, const char *name, bool *found)
{
    *found = false;
    if (!f) return NULL;
    { size_t i = 0; for (; i < f->names.len; i++)
        if (name_eq((const char *)f->names.data[i], name)) {
            *found = true;
            return (ast_node_t *)f->vals.data[i];
        } }
    return NULL;
}

static void frame_set(expand_t *ex, frame_t *f, const char *name, ast_node_t *val)
{
    { size_t i = 0; for (; i < f->names.len; i++)
        if (name_eq((const char *)f->names.data[i], name)) { f->vals.data[i] = val; return; } }
    vec_push(ex->a, &f->names, CONST_CAST(name));
    vec_push(ex->a, &f->vals, val);
}

static const char *interpolate(expand_t *ex, const char *s, frame_t *frame)
{
    if (!frame || !s || !strchr(s, '{')) return s;
    sb_t out; sb_init(&out);
    bool changed = false;
    { const char *p = s; for (; *p; ) {
        if (*p == '{') {
            const char *close = strchr(p, '}');
            if (close) {
                size_t len = (size_t)(close - (p + 1));
                char *key = arena_strndup(ex->a, p + 1, len);
                bool found = false;
                ast_node_t *bound = frame_lookup(frame, key, &found);
                if (found) {
                    sb_puts(&out, node_text(ex, bound));
                    changed = true;
                    p = close + 1;
                    continue;
                }
            }
        }
        sb_putc(&out, *p);
        p++;
    } }
    const char *r = changed ? arena_strdup(ex->a, sb_cstr(&out)) : s;
    sb_free(&out);
    return r;
}

static ast_node_t *subst_value(expand_t *ex, ast_node_t *v, frame_t *frame)
{
    if (!v || !frame) return v;
    if (v->kind == AST_IDENTIFIER) {
        bool found = false;
        ast_node_t *bound = frame_lookup(frame, v->name, &found);
        if (found) return bound ? bound : mk_str(ex, "", &v->span);
    }
    if (v->kind == AST_LITERAL &&
        (v->op == TK_STRING || v->op == TK_TRIPLE_STRING) && v->value.as.s) {
        const char *interp = interpolate(ex, v->value.as.s, frame);
        if (interp != v->value.as.s) return mk_str(ex, interp, &v->span);
    }
    return v;
}

static comp_t *find_comp(expand_t *ex, const char *name)
{
    { size_t i = 0; for (; i < ex->comps.len; i++) {
        comp_t *c = (comp_t *)ex->comps.data[i];
        if (name_eq(c->name, name)) return c;
    } }
    return NULL;
}

static bool body_has_content_sink(ast_node_t *node)
{
    { size_t i = 0; for (; i < node->list.len; i++) {
        ast_node_t *c = (ast_node_t *)node->list.data[i];
        if (c->kind != AST_LAYOUT_ELEMENT) continue;
        if (is_content_word(c->name)) return true;
        if (body_has_content_sink(c)) return true;
    } }
    return false;
}

static void register_components(expand_t *ex, ast_node_t *program, const char *dir)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind != AST_LAYOUT_COMPONENT) continue;
        if (find_comp(ex, d->name)) {
            LOG_W(ex->log, PH_DRIVER, "duplicate component '%s' ignored", d->name);
            continue;
        }
        
        { size_t j = 0; for (; j < d->list.len; j++) {
            ast_node_t *p = (ast_node_t *)d->list.data[j];
            if (p->kind == AST_PARAM && is_content_word(p->name))
                EXERR(ex, "component '%s': 'content' is reserved and cannot be a parameter", d->name);
        } }
        comp_t *c = (comp_t *)arena_alloc(ex->a, sizeof(*c));
        c->name = d->name; c->def = d; c->dir = dir;
        c->mother = d->a ? body_has_content_sink(d->a) : false;
        vec_push(ex->a, &ex->comps, c);
        LOG_D(ex->log, PH_DRIVER, "registered component '%s' (%s)",
              d->name, c->mother ? "mother" : "single");
    } }
}

static ast_node_t *find_layout_block(ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_LAYOUT_BLOCK) return d;
    } }
    return NULL;
}

static bool has_component_def(ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++)
        if (((ast_node_t *)program->list.data[i])->kind == AST_LAYOUT_COMPONENT) return true; }
    return false;
}

static loaded_t *find_loaded(expand_t *ex, const char *path)
{
    { size_t i = 0; for (; i < ex->loaded.len; i++) {
        loaded_t *l = (loaded_t *)ex->loaded.data[i];
        if (name_eq(l->path, path)) return l;
    } }
    return NULL;
}

static loaded_t *load_include(expand_t *ex, const char *path)
{
    loaded_t *cached = find_loaded(ex, path);
    if (cached) return cached->prog ? cached : NULL;
    
    loaded_t *l = (loaded_t *)arena_alloc(ex->a, sizeof(*l));
    l->path = arena_strdup(ex->a, path); l->prog = NULL; l->dir = dir_of(ex->a, path);
    vec_push(ex->a, &ex->loaded, l);
    source_file_t *src = source_load(ex->a, path);
    if (!src) { EXERR(ex, "cannot read included layout '%s'", path); return NULL; }
    src = preproc_source(ex->a, ex->log, src, NULL, 0);
    token_stream_t *toks = NULL;
    lexer_run(ex->a, ex->log, ex->pack, src, &toks);
    ast_node_t *prog = NULL;
    parser_run(ex->a, ex->log, toks, &prog);
    if (!prog) { EXERR(ex, "cannot parse included layout '%s'", path); return NULL; }
    l->prog = prog;
    LOG_I(ex->log, PH_DRIVER, "included layout %s", path);
    return l;
}

static void scan_includes(expand_t *ex, ast_node_t *container, const char *dir, int depth);
static void collect(expand_t *ex, ast_node_t *program, const char *dir, int depth)
{
    register_components(ex, program, dir);
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_LAYOUT_BLOCK)          scan_includes(ex, d, dir, depth);
        else if (d->kind == AST_LAYOUT_COMPONENT) scan_includes(ex, d->a, dir, depth);
    } }
}

static void scan_includes(expand_t *ex, ast_node_t *container, const char *dir, int depth)
{
    if (!container || depth > LX_EXPAND_MAX_DEPTH) return;
    { size_t i = 0; for (; i < container->list.len; i++) {
        ast_node_t *item = (ast_node_t *)container->list.data[i];
        if (item->kind == AST_LAYOUT_ATTR && is_include_attr(item->name)) {
            const char *spec = (item->a && item->a->kind == AST_LITERAL &&
                                item->a->value.kind == TV_STRING) ? item->a->value.as.s : NULL;
            if (!spec) continue;   
            const char *path = salam_resolve_import(ex->a, dir, spec);
            if (!find_loaded(ex, path)) {
                loaded_t *l = load_include(ex, path);
                if (l) collect(ex, l->prog, l->dir, depth + 1);
            }
        } else if (item->kind == AST_LAYOUT_ELEMENT) {
            scan_includes(ex, item, dir, depth);
        }
    } }
}

static void expand_items(expand_t *ex, vec_t *out, ast_node_t *container,
                         frame_t *frame, const char *dir, int depth);
static void expand_item(expand_t *ex, vec_t *out, ast_node_t *item,
                        frame_t *frame, const char *dir, int depth);
static void expand_component(expand_t *ex, vec_t *out, ast_node_t *invocation,
                             comp_t *comp, frame_t *caller, const char *dir, int depth)
{
    if (depth > LX_EXPAND_MAX_DEPTH) {
        EXERR(ex, "component '%s' nested too deeply (cyclic invocation?)", comp->name);
        return;
    }
    frame_t fr; vec_init(&fr.names); vec_init(&fr.vals); vec_init(&fr.content);
    
    { size_t i = 0; for (; i < comp->def->list.len; i++) {
        ast_node_t *param = (ast_node_t *)comp->def->list.data[i];
        if (param->kind == AST_PARAM)
            frame_set(ex, &fr, param->name, param->a);   
    } }
    
    bool got_content = false;
    { size_t i = 0; for (; i < invocation->list.len; i++) {
        ast_node_t *a = (ast_node_t *)invocation->list.data[i];
        if (a->kind == AST_LAYOUT_ATTR) {
            if (is_content_word(a->name)) {
                ast_node_t *txt = ast_new(ex->a, AST_LAYOUT_ATTR, &a->span);
                txt->name = "content";
                txt->a = subst_value(ex, a->a, caller);
                vec_push(ex->a, &fr.content, txt);
                got_content = true;
            } else {
                frame_set(ex, &fr, a->name, subst_value(ex, a->a, caller));
            }
        } else if (a->kind == AST_LAYOUT_ELEMENT) {
            size_t before = fr.content.len;
            expand_item(ex, &fr.content, a, caller, dir, depth);
            if (fr.content.len > before) got_content = true;
        }
    } }
    if (!comp->mother && got_content)
        EXERR(ex, "component '%s' does not accept content (it has no `content:` placeholder)",
              comp->name);
    expand_items(ex, out, comp->def->a, &fr, comp->dir, depth + 1);
}

static void expand_item(expand_t *ex, vec_t *out, ast_node_t *item,
                        frame_t *frame, const char *dir, int depth)
{
    if (item->kind == AST_LAYOUT_ATTR) {
        if (is_include_attr(item->name)) {
            ast_node_t *val = subst_value(ex, item->a, frame);
            const char *spec = (val && val->kind == AST_LITERAL &&
                                val->value.kind == TV_STRING) ? val->value.as.s : NULL;
            if (!spec) { EXERR(ex, "include path must be a string"); return; }
            const char *path = salam_resolve_import(ex->a, dir, spec);
            loaded_t *l = find_loaded(ex, path);
            if (!l) l = load_include(ex, path);
            if (l && l->prog) {
                ast_node_t *lb = find_layout_block(l->prog);
                if (lb) {
                    expand_items(ex, out, lb, NULL, l->dir, depth + 1);
                } else if (!has_component_def(l->prog)) {
                    
                    LOG_W(ex->log, PH_DRIVER, "included file '%s' has no layout block", path);
                }
                
            }
            return;
        }
        
        ast_node_t *attr = ast_clone(ex->a, item);
        attr->a = subst_value(ex, item->a, frame);
        vec_push(ex->a, out, attr);
        return;
    }
    if (item->kind == AST_LAYOUT_ELEMENT) {
        comp_t *comp = find_comp(ex, item->name);
        if (comp) { expand_component(ex, out, item, comp, frame, dir, depth); return; }
        
        if (frame && is_content_word(item->name)) {
            if (frame->content.len)
                { size_t i = 0; for (; i < frame->content.len; i++)
                    vec_push(ex->a, out, frame->content.data[i]); }
            else
                expand_items(ex, out, item, frame, dir, depth);
            return;
        }
        
        ast_node_t *el = ast_new(ex->a, AST_LAYOUT_ELEMENT, &item->span);
        el->name = item->name;
        { size_t i = 0; for (; i < item->aliases.len; i++)
            vec_push(ex->a, &el->aliases, item->aliases.data[i]); }
        expand_items(ex, &el->list, item, frame, dir, depth);
        vec_push(ex->a, out, el);
        return;
    }
    
    vec_push(ex->a, out, item);
}

static void expand_items(expand_t *ex, vec_t *out, ast_node_t *container,
                         frame_t *frame, const char *dir, int depth)
{
    if (depth > LX_EXPAND_MAX_DEPTH) {
        EXERR(ex, "layout include/component nesting too deep");
        return;
    }
    { size_t i = 0; for (; i < container->list.len; i++)
        expand_item(ex, out, (ast_node_t *)container->list.data[i], frame, dir, depth); }
}

size_t layout_expand(arena_t *a, logger_t *log, const langpack_t *pack,
                     ast_node_t *program, const char *base_dir)
{
    expand_t ex; memset(&ex, 0, sizeof(ex));
    ex.a = a; ex.log = log; ex.pack = pack;
    vec_init(&ex.comps); vec_init(&ex.loaded);
    ast_node_t *lb = find_layout_block(program);
    if (!lb) return 0;   
    collect(&ex, program, base_dir ? base_dir : "", 0);
    vec_t items; vec_init(&items);
    expand_items(&ex, &items, lb, NULL, base_dir ? base_dir : "", 0);
    lb->list = items;    
    LOG_I(log, PH_DRIVER, "layout expand: %zu component(s), %zu included file(s)",
          ex.comps.len, ex.loaded.len);
    return ex.errors;
}
